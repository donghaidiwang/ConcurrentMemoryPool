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
	char* m_memory = nullptr;  //ָ����OS����Ĵ���ڴ�,���ڴ��
	void* m_freeList = nullptr;//�û��������ڴ�ʱʹ�õ���������
	size_t m_remainBytes = 0;  //��ʶĿǰ�ڴ�ص�ʣ������
	const int SIZE = 128 * 1024;//�ڴ��ÿ����OS������ֽ���
	// const int SIZE = 16;//�ڴ��ÿ����OS�����ҳ��
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
		//ͷɾ,��ͷ����obj����ʹ��
		void* next = *static_cast<void**>(m_freeList);
		obj = static_cast<T*>(m_freeList);
		m_freeList = next;
		return obj;
	}
	else
	{
		if (m_remainBytes < sizeof(T)) //����ڴ�صĴ�СС��һ��T����Ĵ�С,����������
		{
			//m_memory = static_cast<char*>(malloc(SIZE));
			m_memory = static_cast<char*>(SystemAlloc(SIZE>>PAGE_SHIFT));
			m_remainBytes = SIZE;
			if (m_memory == nullptr) throw std::bad_alloc();
		}
		T* obj = reinterpret_cast<T*>(m_memory);
		size_t objSize = sizeof(T) < sizeof(void*) ? sizeof(void*) : sizeof(T); //��ֹobj����Ϊchar�治��һ��ָ�����
		m_memory += objSize;
		m_remainBytes -= objSize;
		//��λnew,��ʾ����obj�Ĺ��캯�����г�ʼ��
		new(obj) T;
		return obj;
	}
}
template<class T>
void ObjectPool<T>::Delete(T* obj)
{
	//��ʾ������������
	obj->~T();
	//ͷ��
	*reinterpret_cast<void**>(obj) = m_freeList; //Ϊ������32/64λOS,��void**�����ú�(ȡһ��ָ��Ĵ�С)ȡ����objǰ4/8���ֽ�
	m_freeList = obj;
}

#endif // !OBJECTPOOL_H

