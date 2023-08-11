#ifndef CENTRALCACHE_H
#define CENTRALCACHE_H

#include"Span.h"
#include"Common.h"
// 因为CentralCache只需要一个，故采用单例模式
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
		return &m_instance;  // static对象无法在别的文件中访问,故在此直接定义
	}
	Span* GetOneSpan(SpanList& list, size_t size);  // 获取一个非空的Span
	size_t FetchRangeObj(void*& start, void*& end, size_t n, size_t size);  // 从CentralCache中获取一定数量的对象给ThreadCache
	void ReleaseListToSpan(void* start, size_t size);  // 将ThreadCache中FreeList中的内存还给CentralCache
};


#endif // !CENTRALCACHE_H
