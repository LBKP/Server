#pragma once
#include <muduo/base/noncopyable.h>

class Codec : muduo::noncopyable
{
public:
	enum ErrorCode
	{
		kNoError = 0,
		kInvalidLength,
		kInvalidDestination,
		kCheckSumError,
		kInvalidNameLen,
		kParseError,
	};

	Codec();
	~Codec();
};

