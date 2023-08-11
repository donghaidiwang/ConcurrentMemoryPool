#ifndef MYEXCEPTION_H
#define MYEXCEPTION_H
#include<string>
class MyException
{
protected:
	std::string _errmsg;
public:
	MyException(const std::string& errmsg)
		:_errmsg(errmsg)
	{}
	virtual const std::string what() const
	{
		return _errmsg;
	}
};
class FreeListException :public MyException
{
public:
	FreeListException(const std::string& errmsg)
		:MyException(errmsg)
	{}
	virtual const std::string what() const
	{
		std::string str = "FreeListExpection: ";
		str += _errmsg;
		return str;
	}
};
class SizeClassException :public MyException
{
public:
	SizeClassException(const std::string& errmsg)
		:MyException(errmsg)
	{}
	virtual const std::string what() const
	{
		std::string str = "SizeClassExpection: ";
		str += _errmsg;
		return str;
	}
};
class ThreadCacheException :public MyException
{
public:
	ThreadCacheException(const std::string& errmsg)
		:MyException(errmsg)
	{}
	virtual const std::string what() const
	{
		std::string str = "ThreadCacheExpection: ";
		str += _errmsg;
		return str;
	}
};
class ConcurrentAllocException :public MyException
{
public:
	ConcurrentAllocException(const std::string& errmsg)
		:MyException(errmsg)
	{}
	virtual const std::string what() const
	{
		std::string str = "ConcurrentAllocExpection: ";
		str += _errmsg;
		return str;
	}
};
class SpanListException :public MyException
{
public:
	SpanListException(const std::string& errmsg)
		:MyException(errmsg)
	{}
	virtual const std::string what() const
	{
		std::string str = "SpanListExpection: ";
		str += _errmsg;
		return str;
	}
};
class CentralCacheException :public MyException
{
public:
	CentralCacheException(const std::string& errmsg)
		:MyException(errmsg)
	{}
	virtual const std::string what() const
	{
		std::string str = "CentralCacheExpection: ";
		str += _errmsg;
		return str;
	}
};
class PageCacheException :public MyException
{
public:
	PageCacheException(const std::string& errmsg)
		:MyException(errmsg)
	{}
	virtual const std::string what() const
	{
		std::string str = "PageCacheExpection: ";
		str += _errmsg;
		return str;
	}
};
#endif
