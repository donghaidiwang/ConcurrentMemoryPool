#include"PageCache.h"
#include<assert.h>
PageCache PageCache::m_instance;  // ��̬�����Ķ���
Span* PageCache::NewSpan(size_t k)
{
	if (k == 0) throw(PageCacheException("Function NewSpan: Alloc pages is 0\n"));
	// ����NPAGESҳ���ڴ治���ؽ�PageCache
	// ������MAX_BYTES>>PAGE_SHIFTҳС��NPAGESҳ��Ҫ���ؽ�PageCache
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
	// �ȼ���k��Ͱ������û��Span
	if (m_spanLists[k].Empty() == false)
	{
		Span* kSpan = m_spanLists[k].PopFront();
		// ����id��Span��ӳ��,����CentralCache����С���ڴ�ʱ,���Ҷ�Ӧ��Span
		for (int i = 0; i < kSpan->m_n; i++)
		{
			// m_idSpanMap[kSpan->m_pageId + i] = kSpan;
			m_idSpanMap.set(kSpan->m_pageId+i, kSpan);
		}
		return kSpan;
	}
	// ���һ�º����Ͱ������û��Span,����оͽ����з�
	for (int i = k + 1; i < NPAGES; i++)
	{
		if (m_spanLists[i].Empty() == false)
		{
			// �˴�ֻ�޸�Span����ʼҳ��,�з�Span������m_freelists��CentralCache::GetOneSpan����
			Span* nSpan = m_spanLists[i].PopFront();
			// Span* kSpan = new Span;
			Span* kSpan = m_spanPool.New();
			kSpan->m_pageId = nSpan->m_pageId;
			kSpan->m_n = k;
			nSpan->m_pageId += k;
			nSpan->m_n -= k;
			m_spanLists[nSpan->m_n].PushFront(nSpan);
			// ����id��Span��ӳ��,����CentralCache����С���ڴ�ʱ,���Ҷ�Ӧ��Span
			for (int i = 0; i < kSpan->m_n; i++)
			{
				// m_idSpanMap[kSpan->m_pageId + i] = kSpan;
				m_idSpanMap.set(kSpan->m_pageId + i, kSpan);
			}
			// ����nSpan��βҳ�ŵ�ӳ��,���㽫Span���ս�PageCacheʱ�Ĳ���
			// m_idSpanMap[nSpan->m_pageId] = nSpan;
			// m_idSpanMap[nSpan->m_pageId+nSpan->m_n-1] = nSpan;
			m_idSpanMap.set(nSpan->m_pageId, nSpan);
			m_idSpanMap.set(nSpan->m_pageId + nSpan->m_n-1, nSpan);
			return kSpan;
		}
	}
	// �����ڴ���Span,�޷������з�,ֱ���������128ҳ��Span
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
	PAGE_ID id = (PAGE_ID)obj >> PAGE_SHIFT;  // ���objС�ڴ��Ӧ��ҳ��
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
	// ��spanǰ���ҳ���Խ��кϲ�,�����ڴ�����Ƭ������
	// ����NPAGES-1ҳ��,ֱ�ӻ�����
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
			// ���prevIdû�ж�Ӧ��Span,�򲻺ϲ�
			if (ret ==nullptr) break;
			// ���prevId��Ӧ��Span����ʹ��,�򲻺ϲ�
			if (ret->m_isuse == true) break;
			// ����ϲ��󳬹���128ҳ,�򲻺ϲ�
			if (ret->m_n + span->m_n > NPAGES - 1) break;
			Span* prevSpan = ret;
			span->m_n += prevSpan->m_n;
			span->m_pageId = prevSpan->m_pageId;
			// ��prevSpan��PageCache��ɾ��
			m_spanLists[prevSpan->m_n].Erase(prevSpan);
			// �˴�delete�ǽ����ڴ������span���ڴ�ռ仹��ȥ
			// �����ǽ�span�����m_freelist���ڴ滹��ȥ
			// delete prevSpan;
			m_spanPool.Delete(prevSpan);
		}
		while (1)
		{
			PAGE_ID nextId = span->m_pageId + span->m_n;
			Span* ret = reinterpret_cast<Span*>(m_idSpanMap.get(nextId));
			// ���nextIdû�ж�Ӧ��Span,�򲻺ϲ�
			if (ret == nullptr) break;
			// ���nextId��Ӧ��Span����ʹ��,�򲻺ϲ�
			if (ret->m_isuse == true) break;
			// ����ϲ��󳬹���128ҳ,�򲻺ϲ�
			if (ret->m_n + span->m_n > NPAGES - 1) break;
			Span* nextSpan = ret;
			span->m_n += nextSpan->m_n;
			// ��prevSpan��PageCache��ɾ��
			m_spanLists[nextSpan->m_n].Erase(nextSpan);
			// �˴�delete�ǽ����ڴ������span���ڴ�ռ仹��ȥ
			// �����ǽ�span�����m_freelist���ڴ滹��ȥ
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