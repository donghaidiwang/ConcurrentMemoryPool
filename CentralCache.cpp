#include"CentralCache.h"
#include"SizeClass.h"
#include"PageCache.h"
CentralCache CentralCache::m_instance;  // 静态变量的定义
Span* CentralCache::GetOneSpan(SpanList& list, size_t size)
{
	// 遍历SpanList找到第一个不为空的Span
	Span* it = list.Begin();
	while (it != list.End())
	{
		if (it->m_freelist != nullptr) return it;
		else it = it->m_next;
	}
	// 先把central cache的桶锁解掉,这样如果其它线程释放内存对象回来,不会阻塞
	list.m_mtx.unlock();
	PageCache::GetInstance()->GetMutex()->lock();
	// 走到这里说明list中不存在非空的Span,必须向PageCache申请一个新的Span
	Span* newSpan = PageCache::GetInstance()->NewSpan(SizeClass::NumMovePage(size));
	newSpan->m_isuse = true;
	newSpan->m_objsize = size;
	PageCache::GetInstance()->GetMutex()->unlock();
	// 对获取的Span进行切分此时不需要桶锁,因为其它线程此时访问不到此Span(不在CentralCache中)
	// 将新获取的Span进行切分为若干大小为size的小块存于freelist中,再将其链接进CentralCache的SpanLis		t
	char* start = reinterpret_cast<char*>(newSpan->m_pageId * (1 << PAGE_SHIFT));
	size_t bytes = newSpan->m_n * (1 << PAGE_SHIFT);
	char* end = start + bytes;
	// 将大块内存切成自由链表链接起来
	// 先切一块当尾
	newSpan->m_freelist = start;
	start += size;
	void* tail = newSpan->m_freelist;
	while (start < end)
	{
		NextObj(tail) = start;
		tail = NextObj(tail);
		start += size;
	}
	NextObj(tail) = nullptr;
	list.m_mtx.lock();
	list.PushFront(newSpan);
	return newSpan;
}
// 在一个Span中获取n个大小为size的内存块,start和end都是输出型参数
size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t n, size_t size)
{
	size_t index = SizeClass::Index(size);
	// 为了防止多个ThreadCache同时向CentralCache申请空间,对SpanList进行加锁
	m_spanLists[index].m_mtx.lock();
	Span* span = GetOneSpan(m_spanLists[index], size);
	if (span == nullptr || span->m_freelist == nullptr) throw(CentralCacheException("Function GetOneSpan: return a empty Span\n"));
	start = end = span->m_freelist;
	// 从Span中切n个size的内存块返回,但span不一定有n个size大小的内存块,故统计实际返回的内存块个数
	// 最少返回一个size大小的空间,够n个返回n个,不够n个返回剩下全部
	size_t actualNum = 1;
	for (int i = 0; i < n - 1 && NextObj(end) != nullptr; i++)
	{
		end = NextObj(end);
		actualNum++;
	}
	/*void* cur = start;
	int i = 0;
	while (cur)
	{
		cur = NextObj(cur);
		i++;
	}
	if (i != n)
	{
		int x = 0;
	}*/
	span->m_freelist = NextObj(end);
	span->m_usecount += actualNum;
	NextObj(end) = nullptr;
	m_spanLists[index].m_mtx.unlock();
	return actualNum;
}
void CentralCache::ReleaseListToSpan(void* start, size_t size)
{
	size_t index = SizeClass::Index(size);
	m_spanLists[index].m_mtx.lock();
	while (start)
	{
		void* next = NextObj(start);
		Span* span = PageCache::GetInstance()->MapObjectToSpan(start);
		// 头插内存还给span
		NextObj(start) = span->m_freelist;
		span->m_freelist = start;
		span->m_usecount--;
		// 当usecount为0时,说明span切分出去的所有内存块都回来了
		// 此时可以把span再回收给PageCache,PageCahce可以再去做前后页的合并
		if (span->m_usecount == 0)
		{
			m_spanLists->Erase(span);
			span->m_freelist = nullptr;
			span->m_next = nullptr;
			span->m_prev = nullptr;
			// 释放span给PageCache时,只需要使用PageCahce的锁即可
			// 此时不需要CentralCache的桶锁,避免影响其它线程申请或释放
			m_spanLists[index].m_mtx.unlock();
			PageCache::GetInstance()->GetMutex()->lock();
			PageCache::GetInstance()->ReleaseSpanToPageCache(span);
			PageCache::GetInstance()->GetMutex()->unlock();
			m_spanLists[index].m_mtx.lock();
		}
		start = next;
	}
	m_spanLists[index].m_mtx.unlock();
}