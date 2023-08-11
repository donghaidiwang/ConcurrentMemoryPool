#ifndef PAGECACHE_H
#define PAGECACHE_H
#include"Common.h"
#include"Span.h"
#include"ObjectPool.h"
#include"PageMap.h"
// ��CentralCacheһ��,���õ���ģʽ
class PageCache
{
private:
	SpanList m_spanLists[NPAGES];
	std::mutex m_pageMtx;
	// std::unordered_map<PAGE_ID, Span*> m_idSpanMap;
	TCMalloc_PageMap2<32 - PAGE_SHIFT> m_idSpanMap;
	ObjectPool<Span> m_spanPool;
	static PageCache m_instance;
	PageCache(){}
	PageCache(const PageCache&) = delete;
	PageCache& operator=(const PageCache&) = delete;
public:
	static PageCache* GetInstance()
	{
		return &m_instance;
	}
	std::mutex* GetMutex();
	Span* NewSpan(size_t k);  // ��ȡһ��kҳ��Span
	Span* MapObjectToSpan(void* obj);  // �ҵ�һ���ڴ���Ӧ�ĸ�Span
	void ReleaseSpanToPageCache(Span* span);  // �ͷſ��е�Span��PageCache,���ϲ����ڵ�Span 
};

#endif // !PAGECACHE_H
