#ifndef CENTRALCACHE_H
#define CENTRALCACHE_H

#include"Span.h"
#include"Common.h"
// ��ΪCentralCacheֻ��Ҫһ�����ʲ��õ���ģʽ
class CentralCache
{
private:
	CentralCache(){}
	CentralCache(const CentralCache&) = delete;
	CentralCache& operator=(const CentralCache&) = delete;
	SpanList m_spanLists[NFREELISTS];
	static CentralCache m_instance;
public:
	static CentralCache* GetInstance()
	{
		return &m_instance;  // static�����޷��ڱ���ļ��з���,���ڴ�ֱ�Ӷ���
	}
	Span* GetOneSpan(SpanList& list, size_t size);  // ��ȡһ���ǿյ�Span
	size_t FetchRangeObj(void*& start, void*& end, size_t n, size_t size);  // ��CentralCache�л�ȡһ�������Ķ����ThreadCache
	void ReleaseListToSpan(void* start, size_t size);  // ��ThreadCache��FreeList�е��ڴ滹��CentralCache
};


#endif // !CENTRALCACHE_H
