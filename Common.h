#ifndef COMMON_H
#define COMMON_H

#include<iostream>
#include<new>
#include<unordered_map>
#include<thread>
#include<algorithm>
#include"MyException.h"

using std::cout;
using std::endl;
#ifdef _WIN32
#define NOMINMAX  // 为了正常使用std::min不与Windows.h里面min的宏冲突
#include<Windows.h>
#else

#endif // _WIN32

#ifdef _WIN64
using PAGE_ID = unsigned long long;
#elif _WIN32
using PAGE_ID = size_t;
#endif // _WIN32

static const int MAX_BYTES = 256 * 1024;  // 一次最大向ThreadCache申请的空间的大小
static const int NFREELISTS = 208;		  // ThreadCache和CentralCache中freeList和spanList的大小
static const int NPAGES = 129;			  // PageCache中spanLists的大小
static const int PAGE_SHIFT = 13;		  // 一页大小为2的13次方-8kb 
void* SystemAlloc(size_t kpage);
void SystemFree(void* ptr);
void*& NextObj(void* obj);

#endif // !COMMON_H
