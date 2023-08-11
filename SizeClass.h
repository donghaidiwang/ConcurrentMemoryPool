#ifndef SIZECLASS_H
#define SIZECLASS_H

#include"Common.h"
// 管理对齐和映射等关系
class SizeClass
{
public:
	// 整体控制在最多10%左右的内碎片浪费
	// [1,128] 8byte对齐       freelist[0,16) 
	// [128+1,1024] 16byte对齐   freelist[16,72)
	// [1024+1,8*1024] 128byte对齐   freelist[72,128)
	// [8*1024+1,64*1024] 1024byte对齐     freelist[128,184)
	// [64*1024+1,256*1024] 8*1024byte对齐   freelist[184,208)
	// freelist[0]为一个链表头，链接了若干8字节的空间，freelist[1]链接若干16字节的空间...
	// freelist[16]链接若干128字节的空间，freelist[17]链接若干144字节的空间...
	// freelist[184]链接若干64*1024的空间，freelist[185]链接若干72*1024的空间...
	// 每个freelist直接链接空间大小的差距叫对齐数
	static inline size_t _RoundUp(size_t bytes, size_t align)
	{
		return ((bytes + align - 1) & ~(align - 1));
	}
	// 对齐大小计算
	static inline size_t RoundUp(size_t bytes)
	{
		if (bytes <= 128)
		{
			return _RoundUp(bytes, 8);
		}
		else if (bytes <= 1024)
		{
			return _RoundUp(bytes, 16);
		}
		else if (bytes <= 8 * 1024)
		{
			return _RoundUp(bytes, 128);
		}
		else if (bytes <= 64 * 1024)
		{
			return _RoundUp(bytes, 1024);
		}
		else if (bytes <= 256 * 1024)
		{
			return _RoundUp(bytes, 8 * 1024);
		}
		else
		{
			return _RoundUp(bytes, 1<<PAGE_SHIFT);
		}
		return -1;
	}
	static inline size_t _Index(size_t bytes, size_t align_shift)
	{
		return ((bytes + (1 << align_shift) - 1) >> align_shift) - 1;
	}
	// 计算映射的哪一个自由链表桶
	static inline size_t Index(size_t bytes)
	{
		if (bytes > MAX_BYTES) throw(SizeClassException("Alloc bytes greater than MAX_BYTES\n"));
		// 每个区间有多少个链
		static int group_array[4] = { 16, 56, 56, 56 };
		if (bytes <= 128)
		{
			return _Index(bytes, 3);
		}
		else if (bytes <= 1024)
		{
			return _Index(bytes - 128, 4) + group_array[0];
		}
		else if (bytes <= 8 * 1024)
		{
			return _Index(bytes - 1024, 7) + group_array[1] + group_array[0];
		}
		else if (bytes <= 64 * 1024)
		{
			return _Index(bytes - 8 * 1024, 10) + group_array[2] + group_array[1]
				+ group_array[0];
		}
		else if (bytes <= 256 * 1024)
		{
			return _Index(bytes - 64 * 1024, 13) + group_array[3] +
				group_array[2] + group_array[1] + group_array[0];
		}
		else
		{
			throw(SizeClassException("Unknown Error\n"));
		}
		return -1;
	}
	// 一次从中心缓存获取多少个
	static size_t NumMoveSize(size_t size)
	{
		if (size == 0)
			return 0;
		// [2, 512]，一次批量移动多少个对象的(慢启动)上限值
		// 小对象一次批量上限高
		// 小对象一次批量上限低
		int num = MAX_BYTES / size;
		if (num < 2)
			num = 2;
		if (num > 512)
			num = 512;
		return num;
	}
	// 一次向系统获取几个页
	// 单个对象 8byte
	// ...
	// 单个对象 256KB
	static size_t NumMovePage(size_t size)
	{
		size_t num = NumMoveSize(size);  // 优先满足ThreadCache不足向CentralCache获取时一个Span中含有的小空间最小的个数
		size_t npage = num * size;		 // 总共的大小
		npage >>= PAGE_SHIFT;
		if (npage == 0)
			npage = 1;					 // 最少申请一页
		return npage;
	}
};


#endif // !SIZECLASS_H