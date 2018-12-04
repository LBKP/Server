#pragma once
#include "../../publlic/MessageDispatcher.h"


/****************************************************************
* Author		��ZhuPei.pur@outlook.com
* Create		��2018/12/04
* Version		��1.0.0.1
* * Description	��Regist all message
* * History		��
******************************************************************
* Copyright ZhuPei  All rights reserved*
*****************************************************************/

class MessageRegister
{
public:
	MessageRegister();
	~MessageRegister();
private:
	void onUnKnownMessage(const int hash, const MessagePtr& message, muduo::Timestamp);
private:
	ProtobufDispatcher dispatcher_;
};


