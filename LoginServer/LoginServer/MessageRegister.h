#pragma once
#include "../../publlic/MessageDispatcher.h"


/****************************************************************
* Author		£ºZhuPei.pur@outlook.com
* Create		£º2018/12/04
* Version		£º1.0.0.1
* * Description	£ºRegist all message
* * History		£º
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


