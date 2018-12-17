#include "MessageRegister.h"

using namespace std::placeholders;

MessageRegister::MessageRegister()
	//:dispatcher_(std::bind(&MessageRegister::onUnKnownMessage, this, _1, _2, _3, _4))
{
	//regeist message

}

MessageRegister::~MessageRegister()
{
}

void MessageRegister::onUnKnownMessage(const muduo::net::TcpConnection& conn, const int hash, const MessagePtr & message, muduo::Timestamp)
{
}
