#include"ThreadCache.h"
#include"CentralCache.h"
#include"SizeClass.h"
void* ThreadCache::Allocate(size_t size)
{
	if(size>MAX_BYTES) throw(ThreadCacheException("Function Alloc: Alloc bytes greater than MAX_BYTES\n"));
	size_t alignSize = SizeClass::RoundUp(size);  // 经过对齐后分配的内存大小
	size_t index = SizeClass::Index(size);		  // 对应哈希桶的下标
	if (!m_freeLists[index].empty())
	{
		return m_freeLists[index].Pop();
	}
	else
	{
		return FetchFromCentralCache(index, alignSize);
	}
}
void ThreadCache::DeAllocate(void* ptr, size_t size)
{
	if (ptr == nullptr) throw(ThreadCacheException("Function DeAllocate: DeAllocate nullptr"));
	if (size>MAX_BYTES) throw(ThreadCacheException("Function DeAllocate: size is greater than MAX_BYTES"));
	//找出对应的自由链表桶,回收内存
	size_t index = SizeClass::Index(size);
	m_freeLists[index].Push(ptr);
	if (m_freeLists[index].size() >= m_freeLists[index].MaxSize())
	{
		ListTooLong(m_freeLists[index], size);
	}
}
void* ThreadCache::FetchFromCentralCache(size_t index, size_t size)
{
	// 慢开始反馈调节算法
	// 1.最开始不会一次向CentralCache批量要太多,因为太多了可能用不完
	// 2.如果不要根据NumMoveSize算出的size,则batch就会不断增长至上限
	// 3.size越大/小,一次向CentralCache申请的内存就越小/大
	size_t batchNum = std::min(m_freeLists[index].MaxSize(), SizeClass::NumMoveSize(size));
	if (batchNum == m_freeLists[index].MaxSize()) m_freeLists[index].MaxSize()++;
	void* start = nullptr, *end = nullptr;
	size_t actualNum = CentralCache::GetInstance()->FetchRangeObj(start, end, batchNum, size);
	if (actualNum < 1) throw(ThreadCacheException("Function FetchFromCentralCache: actualNum < 1"));
	// 将头一个返回给Allocate,剩下的挂载进ThreadCache的m_freeLists
	if (actualNum == 1)
	{
		return start;
	}
	else
	{
		m_freeLists[index].PushRange(NextObj(start), end,actualNum-1);
		return start;
	}
	return nullptr;
}
// 此处size不是要释放的size,而是释放单个对象的size
void ThreadCache::ListTooLong(FreeList& list, size_t size)
{
	void* start = nullptr, * end = nullptr;
	list.PopRange(start, end, list.MaxSize());
	CentralCache::GetInstance()->ReleaseListToSpan(start, size);
}