#include"Common.h"
#include"FreeList.h"

void FreeList::Push(void* obj)
{
	if (obj == nullptr) throw(FreeListException("Function Push: Push nullptr\n"));
	//Í·²å
	NextObj(obj) = m_freeList;
	m_freeList = obj;
	m_size++;
}
void FreeList::PushRange(void* start, void* end,size_t n)
{
	if (start == nullptr || end==nullptr ) throw(FreeListException("Function PushRange: start or end is nullptr\n"));
	/*void* cur = start;
	int i = 0;
	while (cur)
	{
		cur = NextObj(cur);
		i++;
	}
	if (i != n)
	{
		int x = 0;
	}*/
	NextObj(end) = m_freeList;
	m_freeList = start;
	m_size += n;
}
void FreeList::PopRange(void*& start, void*& end, size_t n)
{
	if (n>m_size) throw(FreeListException("Function PopRange: n is greater than m_size \n"));
	start = end = m_freeList;
	for (int i = 0; i < n-1; i++)
	{
		end = NextObj(end);
	}
	m_freeList = NextObj(end);
	NextObj(end) = nullptr;
	m_size -= n;
}
void* FreeList::Pop()
{
	if (m_freeList == nullptr) throw(FreeListException("Pop is called in a empty FreeList\n"));
	//Í·É¾
	void* obj = m_freeList;
	m_freeList = NextObj(m_freeList);
	m_size--;
	return obj;
}
bool FreeList::empty()
{
	return m_freeList == nullptr;
}
size_t& FreeList::MaxSize()
{
	return m_maxSize;
}
size_t FreeList::size()
{
	return m_size;
}

