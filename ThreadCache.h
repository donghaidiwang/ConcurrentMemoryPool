#ifndef THREADCACHE_H
#define THREADCACHE_H

#include"Common.h"
#include"FreeList.h"

class ThreadCache
{
private:
	FreeList m_freeLists[NFREELISTS];
public:
	void* Allocate(size_t size);
	void DeAllocate(void* ptr, size_t size);
	void* FetchFromCentralCache(size_t index, size_t size);
	void ListTooLong(FreeList& list, size_t size);
};
// TLS thread local storage
static _declspec(thread) ThreadCache* pTLSThreadCache = nullptr;



#endif // !THREADCACHE_H