#pragma once
#include <memory>

#include <muduo/net/Buffer.h>
#include <muduo/net/TcpConnection.h>

#include <google/protobuf/message.h>

typedef std::shared_ptr<google::protobuf::Message> MessagePtr;

// struct ProtobufTransportFormat __attribute__ ((__packed__))
// {
//   int32_t  len;
//   int32_t  starting;
//   int32_t  nameLen;
//   char     typeName[nameLen];
//   char     protobufData[len-nameLen-8];
// }

class ProtobufCodec : muduo::noncopyable
{
public:

	enum ErrorCode : char
	{
		kNoError = 0,
		kInvalidLength,
		kInvalidStarting,
		kInvalidNameLen,
		kUnknownMessageType,
		kParseError,
	};

	typedef std::function<void(const muduo::net::TcpConnectionPtr&,
		const int,
		const MessagePtr&,
		muduo::Timestamp)> ProtobufMessageCallback;

	typedef std::function<void(const muduo::net::TcpConnectionPtr&,
		const int,
		muduo::net::Buffer*,
		muduo::Timestamp,
		ErrorCode)> ErrorCallback;

	explicit ProtobufCodec(const ProtobufMessageCallback& messageCb)
		: messageCallback_(messageCb),
		errorCallback_(defaultErrorCallback)
	{
	}

	ProtobufCodec(const ProtobufMessageCallback& messageCb, const ErrorCallback& errorCb)
		: messageCallback_(messageCb),
		errorCallback_(errorCb)
	{
	}

	void onMessage(const muduo::net::TcpConnectionPtr conn,
		muduo::net::Buffer* buf,
		muduo::Timestamp receiveTime);

	void send(const muduo::net::TcpConnectionPtr& conn,
		const int hash,
		const google::protobuf::Message& message)
	{
		// FIXME: serialize to TcpConnection::outputBuffer()
		muduo::net::Buffer buf;
		fillEmptyBuffer(&buf, hash, message);
		conn->send(&buf);
	}

	static const muduo::string& errorCodeToString(ErrorCode errorCode);
	static void fillEmptyBuffer(muduo::net::Buffer* buf, const int32_t hash, const google::protobuf::Message& message);
	static google::protobuf::Message* createMessage(const std::string& type_name);
	static MessagePtr parse(const char* buf, int len, int32_t& hash, ErrorCode& errorCode);

private:
	static void defaultErrorCallback(const muduo::net::TcpConnectionPtr&,
		int,
		muduo::net::Buffer*,
		muduo::Timestamp,
		ErrorCode);

	ProtobufMessageCallback messageCallback_;
	ErrorCallback errorCallback_;
public:
	const static int kHeaderLen = sizeof(int32_t);
	const static int kSizeofNameLen = sizeof(int8_t);
	const static int kMinMessageLen = kHeaderLen + 2; // nameLen(1) + typeName(>1) + starting(4)
	const static int kMaxMessageLen = 64 * 1024 * 1024; // same as codec_stream.h kDefaultTotalBytesLimit
};