#include"ThreadCache.h"
#include"CentralCache.h"
#include"SizeClass.h"
void* ThreadCache::Allocate(size_t size)
{
	if(size>MAX_BYTES) throw(ThreadCacheException("Function Alloc: Alloc bytes greater than MAX_BYTES\n"));
	size_t alignSize = SizeClass::RoundUp(size);  // ��������������ڴ��С
	size_t index = SizeClass::Index(size);		  // ��Ӧ��ϣͰ���±�
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
	//�ҳ���Ӧ����������Ͱ,�����ڴ�
	size_t index = SizeClass::Index(size);
	m_freeLists[index].Push(ptr);
	if (m_freeLists[index].size() >= m_freeLists[index].MaxSize())
	{
		ListTooLong(m_freeLists[index], size);
	}
}
void* ThreadCache::FetchFromCentralCache(size_t index, size_t size)
{
	// ����ʼ���������㷨
	// 1.�ʼ����һ����CentralCache����Ҫ̫��,��Ϊ̫���˿����ò���
	// 2.�����Ҫ����NumMoveSize�����size,��batch�ͻ᲻������������
	// 3.sizeԽ��/С,һ����CentralCache������ڴ��ԽС/��
	size_t batchNum = std::min(m_freeLists[index].MaxSize(), SizeClass::NumMoveSize(size));
	if (batchNum == m_freeLists[index].MaxSize()) m_freeLists[index].MaxSize()++;
	void* start = nullptr, *end = nullptr;
	size_t actualNum = CentralCache::GetInstance()->FetchRangeObj(start, end, batchNum, size);
	if (actualNum < 1) throw(ThreadCacheException("Function FetchFromCentralCache: actualNum < 1"));
	// ��ͷһ�����ظ�Allocate,ʣ�µĹ��ؽ�ThreadCache��m_freeLists
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
// �˴�size����Ҫ�ͷŵ�size,�����ͷŵ��������size
void ThreadCache::ListTooLong(FreeList& list, size_t size)
{
	void* start = nullptr, * end = nullptr;
	list.PopRange(start, end, list.MaxSize());
	CentralCache::GetInstance()->ReleaseListToSpan(start, size);
}