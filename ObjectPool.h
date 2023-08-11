#ifndef OBJECTPOOL_H
#define OBJECTPOOL_H

#include"Common.h"

//template<class N>
//class ObjectPool
//{};
template<class T>
class ObjectPool
{
protected:
	char* m_memory = nullptr;  //指向向OS申请的大块内存,即内存池
	void* m_freeList = nullptr;//用户还回来内存时使用的自由链表
	size_t m_remainBytes = 0;  //标识目前内存池的剩余容量
	const int SIZE = 128 * 1024;//内存池每次向OS申请的字节数
	// const int SIZE = 16;//内存池每次向OS申请的页数
public:
	T* New();
	void Delete(T* obj);
};

template<class T>
T* ObjectPool<T>::New()
{
	T* obj = nullptr;
	if (m_freeList != nullptr)
	{
		//头删,将头结点给obj对象使用
		void* next = *static_cast<void**>(m_freeList);
		obj = static_cast<T*>(m_freeList);
		m_freeList = next;
		return obj;
	}
	else
	{
		if (m_remainBytes < sizeof(T)) //如果内存池的大小小于一个T对象的大小,则重新申请
		{
			//m_memory = static_cast<char*>(malloc(SIZE));
			m_memory = static_cast<char*>(SystemAlloc(SIZE>>PAGE_SHIFT));
			m_remainBytes = SIZE;
			if (m_memory == nullptr) throw std::bad_alloc();
		}
		T* obj = reinterpret_cast<T*>(m_memory);
		size_t objSize = sizeof(T) < sizeof(void*) ? sizeof(void*) : sizeof(T); //防止obj对象为char存不下一个指针变量
		m_memory += objSize;
		m_remainBytes -= objSize;
		//定位new,显示调用obj的构造函数进行初始化
		new(obj) T;
		return obj;
	}
}
template<class T>
void ObjectPool<T>::Delete(T* obj)
{
	//显示调用析构函数
	obj->~T();
	//头插
	*reinterpret_cast<void**>(obj) = m_freeList; //为了适配32/64位OS,对void**解引用后(取一级指针的大小)取的是obj前4/8个字节
	m_freeList = obj;
}

#endif // !OBJECTPOOL_H

