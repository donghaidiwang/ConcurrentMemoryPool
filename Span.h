#ifndef SPAN_H
#define SPAN_H

#include"Common.h"
#include"ObjectPool.h"
#include<mutex>
struct Span
{
	PAGE_ID m_pageId;  // ����Ĵ���ڴ��ҳ��,һҳ8k,64λ�ᱬsize_t����ʹ��PAGE_ID
	size_t m_n;		   // ҳ������
	Span* m_prev;
	Span* m_next;
	void* m_freelist;   // �кõ�С���ڴ����������
	size_t m_usecount;  // ���ָ�thread cacheʹ�õ�С���ڴ�ĸ���
	size_t m_objsize;   // ���зֵ�С���ڴ�Ĵ�С
	bool m_isuse;		// �Ƿ��ڱ�ʹ��
	Span()
		:m_pageId(0)
		,m_n(0)
		,m_prev(nullptr)
		,m_next(nullptr)
		,m_freelist(nullptr)
		,m_usecount(0)
		,m_objsize(0)
		,m_isuse(false)
	{}
};
//����Span�Ĵ�ͷ˫��ѭ������
class SpanList
{
private:
	Span* m_head;
	ObjectPool<Span> m_head_spanPool;
public:
	SpanList();
	void Insert(Span* pos, Span* newSpan);
	void Erase(Span* pos);
	Span* Begin();
	Span* End();
	bool Empty();
	void PushFront(Span* newSpan);
	Span* PopFront();
	std::mutex m_mtx;  // Ͱ��
};
#endif // !SPAN_H
