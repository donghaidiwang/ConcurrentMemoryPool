#include"Common.h"
void* SystemAlloc(size_t kpage)
{
#ifdef _WIN32
	void* ptr = VirtualAlloc(0, kpage << PAGE_SHIFT, MEM_COMMIT | MEM_RESERVE,
		PAGE_READWRITE);
#else
	// linux��brk mmap��
#endif
	if (ptr == nullptr)
		throw std::bad_alloc();
	return ptr;
}
void SystemFree(void* ptr)
{
#ifdef _WIN32
	VirtualFree(ptr, 0, MEM_RELEASE);
#else
	// sbrk unmmap��
#endif
}
void*& NextObj(void* obj)
{
	return *reinterpret_cast<void**>(obj);
}
