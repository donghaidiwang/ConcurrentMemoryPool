#ifndef FREELIST_H
#define FREELIST_H

class FreeList
{
private:
	void* m_freeList = nullptr;
	size_t m_maxSize = 1;  // 一次向CentralCache获取的个数
	size_t m_size = 0;
public:
	void Push(void* obj);
	void PushRange(void* start,void* end,size_t n);
	void PopRange(void*& start,void*& end,size_t n);
	void* Pop();
	bool empty();
	size_t& MaxSize();
	size_t size();
};

#endif // FREELIST_H
