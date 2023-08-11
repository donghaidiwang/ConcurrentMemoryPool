#define _CRT_SECURE_NO_WARNINGS 1
#include"ObjectPool.h"
#include<vector>
#include<thread>
#include"ConcurrentAlloc.h"
#include"Common.h"
using namespace std;
struct TreeNode
{
	int _val;
	TreeNode* _left;
	TreeNode* _right;
	TreeNode()
		:_val(0)
		, _left(nullptr)
		, _right(nullptr)
	{}
};
void TestObjectPool()
{
	// 申请释放的轮次
	const size_t Rounds = 3;
	// 每轮申请释放多少次
	const size_t N = 10000;
	size_t begin1 = clock();
	std::vector<TreeNode*> v1;
	v1.reserve(N);
	for (size_t j = 0; j < Rounds; ++j)
	{
		for (int i = 0; i < N; ++i)
		{
			v1.push_back(new TreeNode);
		}
		for (int i = 0; i < N; ++i)
		{
			delete v1[i];
		}
		v1.clear();
	}
	size_t end1 = clock();
	ObjectPool<TreeNode> TNPool;
	size_t begin2 = clock();
	std::vector<TreeNode*> v2;
	v2.reserve(N);
	for (size_t j = 0; j < Rounds; ++j)
	{
		for (int i = 0; i < N; ++i)
		{
			v2.push_back(TNPool.New());
		}
		for (int i = 0; i < N; ++i)
		{
			TNPool.Delete(v2[i]);
		}
		v2.clear();
	}
	size_t end2 = clock();
	cout << "new cost time:" << end1 - begin1 << endl;
	cout << "object pool cost time:" << end2 - begin2 << endl;
}
void Alloc1()
{
	for (int i = 0; i < 6; i++)
	{
		void* ptr = ConcurrentAlloc(6);
	}
}
void Alloc2()
{
	for (int i = 0; i < 6; i++)
	{
		void* ptr = ConcurrentAlloc(5);
	}
}
void TestTLS()
{
	try
	{
		std::thread t1(Alloc1);
		t1.join();
		std::thread t2(Alloc2);
		t2.join();
	}
	catch(MyException& ex)
	{
		cout << ex.what() << endl;
	}
	catch (...)
	{
		cout << "Unknown Error" << endl;
	}
}
void TestConcurrentAlloc1()
{
	try
	{
		void* p1 = ConcurrentAlloc(6);
		void* p2 = ConcurrentAlloc(1);
		void* p3 = ConcurrentAlloc(7);
		void* p4 = ConcurrentAlloc(8);
		void* p5 = ConcurrentAlloc(2);
		void* p6 = ConcurrentAlloc(4);
		void* p7 = ConcurrentAlloc(4);
		cout << p1 << endl;
		cout << p2 << endl;
		cout << p3 << endl;
		cout << p4 << endl;
		cout << p5 << endl;
		cout << p6 << endl;
		cout << p7 << endl;
		ConcurrentFree(p1);
		ConcurrentFree(p2);
		ConcurrentFree(p3);
		ConcurrentFree(p4);
		ConcurrentFree(p5);
		ConcurrentFree(p6);
		ConcurrentFree(p7);
	}
	catch (MyException& ex)
	{
		cout << ex.what() << endl;
	}
	catch (...)
	{
		cout << "Unknown Error" << endl;
	}
}
void TestConcurrentAlloc2()
{
	for (int i = 0; i < 1024; i++)
	{
		void* p1=ConcurrentAlloc(6);
		cout << p1 << endl;
	}
	void* p2 = ConcurrentAlloc(6);
	cout << p2 << endl;
}
void MultiThreadAlloc1()
{
	vector<void*> v;
	for (int i = 0; i < 7; i++)
	{
		void* ptr = ConcurrentAlloc(6);
		v.push_back(ptr);
	}
	for (auto e : v)
	{
		ConcurrentFree(e);
	}
}
void MultiThreadAlloc2()
{
	vector<void*> v;
	for (int i = 0; i < 7; i++)
	{
		void* ptr = ConcurrentAlloc(16);
		v.push_back(ptr);
	}
	for (auto e : v)
	{
		ConcurrentFree(e);
	}
}
void TestMultiThread()
{
	try
	{
		std::thread t1(MultiThreadAlloc1);
		std::thread t2(MultiThreadAlloc2);
		t1.join();
		t2.join();
	}
	catch (MyException& ex)
	{
		cout << ex.what() << endl;
	}
	catch (...)
	{
		cout << "Unknown Error" << endl;
	}
}
void BigAlloc()
{
	try
	{
		void* p1 = ConcurrentAlloc(257 * 1024);
		ConcurrentFree(p1);
		void* p2 = ConcurrentAlloc(129 * 8 * 1024);
		ConcurrentFree(p2);
	}
	catch (MyException& ex)
	{
		cout << ex.what() << endl;
	}
	catch (...)
	{
		cout << "Unknown Error" << endl;
	}
}
//int main()
//{	
//	// TestObjectPool();
//	// TestTLS();
//	// TestConcurrentAlloc2();
//	// TestConcurrentAlloc1();
//	// TestMultiThread();
//	BigAlloc();
//	return 0;
//}