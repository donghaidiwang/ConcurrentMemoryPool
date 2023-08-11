#ifndef SPAN_H
#define SPAN_H

#include"Common.h"
#include"ObjectPool.h"
#include<mutex>
struct Span
{
	PAGE_ID m_pageId;  // 管理的大块内存的页号,一页8k,64位会爆size_t，故使用PAGE_ID
	size_t m_n;		   // 页的数量
	Span* m_prev;
	Span* m_next;
	void* m_freelist;   // 切好的小块内存的自由链表
	size_t m_usecount;  // 被分给thread cache使用的小块内存的个数
	size_t m_objsize;   // 被切分的小块内存的大小
	bool m_isuse;		// 是否在被使用
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
//管理Span的带头双向循环链表
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
	std::mutex m_mtx;  // 桶锁
};
#endif // !SPAN_H
