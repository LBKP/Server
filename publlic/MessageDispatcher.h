#pragma once
#include <functional>
#include <map>
#include <memory>
#include <type_traits>

#include <muduo/base/noncopyable.h>
#include <muduo/base/Timestamp.h>

#include <google/protobuf/message.h>

typedef std::shared_ptr<google::protobuf::Message> MessagePtr;

class Callback : muduo::noncopyable
{
public:
	virtual ~Callback() {};
	virtual void onMessage(const muduo::net::TcpConnectionPtr&, 
		const int hash,
		const MessagePtr& message,
		muduo::Timestamp) const = 0;
};

template <typename T>
class CallbackT :public Callback
{
	static_assert(std::is_base_of<google::protobuf::Message, T>::value,
		"T must be a Message");
public:
	typedef std::function<void(const muduo::net::TcpConnectionPtr&,
		const int,
		const std::shared_ptr<T>& message,
		muduo::Timestamp)> ProtobufMessageTCallback;
	CallbackT(const ProtobufMessageTCallback& callback)
		: callback_(callback)
	{
	}

	virtual void onMessage(const muduo::net::TcpConnectionPtr& conn,
		const int hash,
		const MessagePtr& message,
		muduo::Timestamp receiveTime) const
	{
		std::shared_ptr<T> concrete = muduo::down_pointer_cast<T>(message);
		assert(concrete != NULL);
		callback_(conn, hash, concrete, receiveTime);
	}

private:
	ProtobufMessageTCallback callback_;
};

typedef std::function<void(const muduo::net::TcpConnectionPtr conn, const int hash, const MessagePtr& message, muduo::Timestamp)> ProtobufMessageCallback;
class ProtobufDispatcher
{
public:

	explicit ProtobufDispatcher(const ProtobufMessageCallback& defaultCb)
		: defaultCallback_(defaultCb)
	{
	}

	void onProtobufMessage(const muduo::net::TcpConnectionPtr conn,
		const int hash,
		const MessagePtr& message,
		muduo::Timestamp receiveTime) const
	{
		CallbackMap::const_iterator it = callbacks_.find(message->GetDescriptor());
		if (it != callbacks_.end())
		{
			it->second->onMessage(conn, hash, message, receiveTime);
		}
		else
		{
			defaultCallback_(conn, hash, message, receiveTime);
		}
	}

	template<typename T>
	void registerMessageCallback(const typename CallbackT<T>::ProtobufMessageTCallback& callback)
	{
		std::shared_ptr<CallbackT<T> > pd(new CallbackT<T>(callback));
		callbacks_[T::descriptor()] = pd;
	}

private:
	typedef std::map<const google::protobuf::Descriptor*, std::shared_ptr<Callback> > CallbackMap;

	CallbackMap callbacks_;
	ProtobufMessageCallback defaultCallback_;
};