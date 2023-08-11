#ifndef PAGECACHE_H
#define PAGECACHE_H
#include"Common.h"
#include"Span.h"
#include"ObjectPool.h"
#include"PageMap.h"
// 与CentralCache一样,采用单例模式
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
	Span* NewSpan(size_t k);  // 获取一个k页的Span
	Span* MapObjectToSpan(void* obj);  // 找到一个内存块对应哪个Span
	void ReleaseSpanToPageCache(Span* span);  // 释放空闲的Span到PageCache,并合并相邻的Span 
};

#endif // !PAGECACHE_H
