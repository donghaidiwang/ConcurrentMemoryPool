#ifndef CONCURRENTALLOC_H
#define CONCURRENTALLOC_H
#include"SizeClass.h"
#include"ThreadCache.h"
#include"PageCache.h"

static void* ConcurrentAlloc(size_t size)
{
	if (size > MAX_BYTES)
	{
		size_t alignsize = SizeClass::RoundUp(size);
		size_t kpage = alignsize >> PAGE_SHIFT;
		PageCache::GetInstance()->GetMutex()->lock();
		Span* span=PageCache::GetInstance()->NewSpan(kpage);
		span->m_objsize = size;
		span->m_isuse = true;
		PageCache::GetInstance()->GetMutex()->unlock();
		void* ptr = reinterpret_cast<void*>(span->m_pageId * (1 << PAGE_SHIFT));
		return ptr;
	}
	else
	{
		if (pTLSThreadCache == nullptr)
		{
			static ObjectPool<ThreadCache> tcPool;
			// pTLSThreadCache = new ThreadCache;
			pTLSThreadCache = tcPool.New();
		}
		//std::cout << std::this_thread::get_id() << ": " << pTLSThreadCache << std::endl;
		return pTLSThreadCache->Allocate(size);
	}
}
static void ConcurrentFree(void* ptr)
{
	if (ptr == nullptr) throw(ConcurrentAllocException("Functional ConcurrentFree: Free nullptr"));
	Span* span = PageCache::GetInstance()->MapObjectToSpan(ptr);
	size_t size = span->m_objsize;
	if (size > MAX_BYTES)
	{
		PageCache::GetInstance()->GetMutex()->lock();
		PageCache::GetInstance()->ReleaseSpanToPageCache(span);
		PageCache::GetInstance()->GetMutex()->unlock();
	}
	else pTLSThreadCache->DeAllocate(ptr, size);
}

#endif  //!CONCURRENTALLOC_H
