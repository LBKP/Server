#include "MessageRegister.h"

MessageRegister::MessageRegister()
	:dispatcher_(std::bind(&MessageRegister::onUnKnownMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))
{
	//regeist message

}

MessageRegister::~MessageRegister()
{
}

void MessageRegister::onUnKnownMessage(const int hash, const MessagePtr & message, muduo::Timestamp)
{
}
