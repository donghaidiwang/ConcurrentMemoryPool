#include"CentralCache.h"
#include"SizeClass.h"
#include"PageCache.h"
CentralCache CentralCache::m_instance;  // ��̬�����Ķ���
Span* CentralCache::GetOneSpan(SpanList& list, size_t size)
{
	// ����SpanList�ҵ���һ����Ϊ�յ�Span
	Span* it = list.Begin();
	while (it != list.End())
	{
		if (it->m_freelist != nullptr) return it;
		else it = it->m_next;
	}
	// �Ȱ�central cache��Ͱ�����,������������߳��ͷ��ڴ�������,��������
	list.m_mtx.unlock();
	PageCache::GetInstance()->GetMutex()->lock();
	// �ߵ�����˵��list�в����ڷǿյ�Span,������PageCache����һ���µ�Span
	Span* newSpan = PageCache::GetInstance()->NewSpan(SizeClass::NumMovePage(size));
	newSpan->m_isuse = true;
	newSpan->m_objsize = size;
	PageCache::GetInstance()->GetMutex()->unlock();
	// �Ի�ȡ��Span�����зִ�ʱ����ҪͰ��,��Ϊ�����̴߳�ʱ���ʲ�����Span(����CentralCache��)
	// ���»�ȡ��Span�����з�Ϊ���ɴ�СΪsize��С�����freelist��,�ٽ������ӽ�CentralCache��SpanLis		t
	char* start = reinterpret_cast<char*>(newSpan->m_pageId * (1 << PAGE_SHIFT));
	size_t bytes = newSpan->m_n * (1 << PAGE_SHIFT);
	char* end = start + bytes;
	// ������ڴ��г�����������������
	// ����һ�鵱β
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
// ��һ��Span�л�ȡn����СΪsize���ڴ��,start��end��������Ͳ���
size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t n, size_t size)
{
	size_t index = SizeClass::Index(size);
	// Ϊ�˷�ֹ���ThreadCacheͬʱ��CentralCache����ռ�,��SpanList���м���
	m_spanLists[index].m_mtx.lock();
	Span* span = GetOneSpan(m_spanLists[index], size);
	if (span == nullptr || span->m_freelist == nullptr) throw(CentralCacheException("Function GetOneSpan: return a empty Span\n"));
	start = end = span->m_freelist;
	// ��Span����n��size���ڴ�鷵��,��span��һ����n��size��С���ڴ��,��ͳ��ʵ�ʷ��ص��ڴ�����
	// ���ٷ���һ��size��С�Ŀռ�,��n������n��,����n������ʣ��ȫ��
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
		// ͷ���ڴ滹��span
		NextObj(start) = span->m_freelist;
		span->m_freelist = start;
		span->m_usecount--;
		// ��usecountΪ0ʱ,˵��span�зֳ�ȥ�������ڴ�鶼������
		// ��ʱ���԰�span�ٻ��ո�PageCache,PageCahce������ȥ��ǰ��ҳ�ĺϲ�
		if (span->m_usecount == 0)
		{
			m_spanLists->Erase(span);
			span->m_freelist = nullptr;
			span->m_next = nullptr;
			span->m_prev = nullptr;
			// �ͷ�span��PageCacheʱ,ֻ��Ҫʹ��PageCahce��������
			// ��ʱ����ҪCentralCache��Ͱ��,����Ӱ�������߳�������ͷ�
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