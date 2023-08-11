#include"PageCache.h"
#include<assert.h>
PageCache PageCache::m_instance;  // 静态变量的定义
Span* PageCache::NewSpan(size_t k)
{
	if (k == 0) throw(PageCacheException("Function NewSpan: Alloc pages is 0\n"));
	// 大于NPAGES页的内存不挂载进PageCache
	// 但大于MAX_BYTES>>PAGE_SHIFT页小于NPAGES页的要挂载进PageCache
	if (k > NPAGES - 1)
	{
		void* ptr = SystemAlloc(k);
		// Span* span = new Span;
		Span* span = m_spanPool.New();
		span->m_pageId = reinterpret_cast<PAGE_ID>(ptr) >> PAGE_SHIFT;
		// m_idSpanMap[span->m_pageId] = span;
		m_idSpanMap.set(span->m_pageId, span);
		span->m_n = k;
		return span;
	}
	// 先检查第k个桶里面有没有Span
	if (m_spanLists[k].Empty() == false)
	{
		Span* kSpan = m_spanLists[k].PopFront();
		// 建立id和Span的映射,方便CentralCache回收小块内存时,查找对应的Span
		for (int i = 0; i < kSpan->m_n; i++)
		{
			// m_idSpanMap[kSpan->m_pageId + i] = kSpan;
			m_idSpanMap.set(kSpan->m_pageId+i, kSpan);
		}
		return kSpan;
	}
	// 检查一下后面的桶里面有没有Span,如果有就进行切分
	for (int i = k + 1; i < NPAGES; i++)
	{
		if (m_spanLists[i].Empty() == false)
		{
			// 此处只修改Span的起始页号,切分Span并存入m_freelists在CentralCache::GetOneSpan进行
			Span* nSpan = m_spanLists[i].PopFront();
			// Span* kSpan = new Span;
			Span* kSpan = m_spanPool.New();
			kSpan->m_pageId = nSpan->m_pageId;
			kSpan->m_n = k;
			nSpan->m_pageId += k;
			nSpan->m_n -= k;
			m_spanLists[nSpan->m_n].PushFront(nSpan);
			// 建立id和Span的映射,方便CentralCache回收小块内存时,查找对应的Span
			for (int i = 0; i < kSpan->m_n; i++)
			{
				// m_idSpanMap[kSpan->m_pageId + i] = kSpan;
				m_idSpanMap.set(kSpan->m_pageId + i, kSpan);
			}
			// 建立nSpan首尾页号的映射,方便将Span回收进PageCache时的查找
			// m_idSpanMap[nSpan->m_pageId] = nSpan;
			// m_idSpanMap[nSpan->m_pageId+nSpan->m_n-1] = nSpan;
			m_idSpanMap.set(nSpan->m_pageId, nSpan);
			m_idSpanMap.set(nSpan->m_pageId + nSpan->m_n-1, nSpan);
			return kSpan;
		}
	}
	// 不存在大块的Span,无法进行切分,直接向堆申请128页的Span
	// Span* bigSpan = new Span;
	Span* bigSpan = m_spanPool.New();
	void* ptr = SystemAlloc(NPAGES - 1);
	bigSpan->m_pageId = (PAGE_ID)ptr >> PAGE_SHIFT;
	bigSpan->m_n = NPAGES - 1;
	m_spanLists[bigSpan->m_n].PushFront(bigSpan);
	return NewSpan(k);
}
std::mutex* PageCache::GetMutex()
{
	return &m_pageMtx;
}
Span* PageCache::MapObjectToSpan(void* obj)
{
	// std::unique_lock<std::mutex> lock(m_pageMtx);
	PAGE_ID id = (PAGE_ID)obj >> PAGE_SHIFT;  // 算出obj小内存对应的页号
	auto ret = m_idSpanMap.get(id);
	if (ret != nullptr)
	{
		return reinterpret_cast<Span*>(ret);
	}
	else
	{
		throw(PageCacheException("Function MapObjectToSpan: Span not found!\n"));
		// assert(false);
	}
	return nullptr;
}
void PageCache::ReleaseSpanToPageCache(Span* span)
{
	// 对span前后的页尝试进行合并,缓解内存外碎片的问题
	// 大于NPAGES-1页的,直接还给堆
	if (span->m_n > NPAGES - 1)
	{
		void* ptr =reinterpret_cast<void*>(span->m_pageId << PAGE_SHIFT);
		SystemFree(ptr);
		// delete span;
		m_spanPool.Delete(span);
		return;
	}
	else
	{
		while (1)
		{
			PAGE_ID prevId = span->m_pageId - 1;
			Span* ret =reinterpret_cast<Span*>(m_idSpanMap.get(prevId));
			// 如果prevId没有对应的Span,则不合并
			if (ret ==nullptr) break;
			// 如果prevId对应的Span正在使用,则不合并
			if (ret->m_isuse == true) break;
			// 如果合并后超过了128页,则不合并
			if (ret->m_n + span->m_n > NPAGES - 1) break;
			Span* prevSpan = ret;
			span->m_n += prevSpan->m_n;
			span->m_pageId = prevSpan->m_pageId;
			// 将prevSpan从PageCache中删除
			m_spanLists[prevSpan->m_n].Erase(prevSpan);
			// 此处delete是将向内存申请的span的内存空间还回去
			// 而不是将span管理的m_freelist的内存还回去
			// delete prevSpan;
			m_spanPool.Delete(prevSpan);
		}
		while (1)
		{
			PAGE_ID nextId = span->m_pageId + span->m_n;
			Span* ret = reinterpret_cast<Span*>(m_idSpanMap.get(nextId));
			// 如果nextId没有对应的Span,则不合并
			if (ret == nullptr) break;
			// 如果nextId对应的Span正在使用,则不合并
			if (ret->m_isuse == true) break;
			// 如果合并后超过了128页,则不合并
			if (ret->m_n + span->m_n > NPAGES - 1) break;
			Span* nextSpan = ret;
			span->m_n += nextSpan->m_n;
			// 将prevSpan从PageCache中删除
			m_spanLists[nextSpan->m_n].Erase(nextSpan);
			// 此处delete是将向内存申请的span的内存空间还回去
			// 而不是将span管理的m_freelist的内存还回去
			// delete nextSpan;
			m_spanPool.Delete(nextSpan);
		}
		m_spanLists[span->m_n].PushFront(span);
		span->m_isuse = false;
		// m_idSpanMap[span->m_pageId] = span;
		// m_idSpanMap[span->m_pageId + span->m_n - 1] = span;
		m_idSpanMap.set(span->m_pageId, span);
		m_idSpanMap.set(span->m_pageId + span->m_n - 1, span);
	}
}