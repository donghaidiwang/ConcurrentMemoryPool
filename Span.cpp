#include"Span.h"
SpanList::SpanList()
{
	//static ObjectPool<Span> head_spanPool;
	//m_head = new Span;
	m_head = m_head_spanPool.New();
	m_head->m_next = m_head;
	m_head->m_prev = m_head;
}
void SpanList::Insert(Span* pos, Span* newSpan)
{
	if (pos == nullptr) throw(SpanListException("Function Insert: Insert pos is nullptr\n"));
	if (newSpan == nullptr) throw(SpanListException("Function Insert: Insert obj is nullptr\n"));
	Span* prev = pos->m_prev;
	prev->m_next = newSpan;
	newSpan->m_prev = prev;
	newSpan->m_next = pos;
	pos->m_prev = newSpan;
}
void SpanList::Erase(Span* pos)
{
	if (pos == nullptr) throw(SpanListException("Function Erase: Erase pos is nullptr\n"));
	if (pos == m_head) throw(SpanListException("Function Erase: Erase pos is m_head\n"));
	Span* prev = pos->m_prev;
	Span* next = pos->m_next;
	prev->m_next = next;
	next->m_prev = prev;
}
Span* SpanList::Begin()
{
	return m_head->m_next;
}
Span* SpanList::End()
{
	return m_head;
}
bool SpanList::Empty()
{
	return m_head->m_next == m_head;
}
void SpanList::PushFront(Span* newSpan)
{
	Insert(m_head->m_next, newSpan);
}
Span* SpanList::PopFront()
{
	Span* front = m_head->m_next;
	Erase(front);
	return front;
}