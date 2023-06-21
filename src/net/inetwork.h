#pragma once
#include <functional>
#include <memory>
#include <mutex>
#include "nettype.h"
#include "until/inetAddress.h"

class Buffer;

/********************
XXXXCallback 通用事件回调接口
*********************/
typedef std::function<void(const uint64_t)> NewConnectCallback;
void defaultNewConnectCallback(uint64_t);

/*errcode=0连接成功*/
typedef std::function<void (const uint64_t, int32_t errcode)> ConnectCallback;
void defaultConnectCallback(uint64_t, int32_t);

typedef std::function<void (const uint64_t)> DisConnectCallback;
void defaultDisConnectCallback(uint64_t);

typedef std::function<void(const uint64_t,int32_t)> ErrorCallback;
void defaultErrorCallback(uint64_t, int32_t);

/********************
TCP收到消息回调接口
*********************/
typedef std::function<void(const uint64_t, Buffer*)> ReceiveCallback;
void defaultReceiveCallback(const uint64_t, Buffer*);

/********************
UDP收到消息回调接口
*********************/
typedef std::function<void(InetAddress& addr, std::string& message)> ReceiveFromCallback;
void defaultReceiveFromCallback(InetAddress& addr, std::string& message);

/********************
NetObj提供获取连接信息接口
*********************/
class INetObj
{
public:
	INetObj() {}
	virtual ~INetObj() {}

	virtual uint64_t getNetID() = 0;
	/*连接类型*/
	virtual uint32_t getNetMode() = 0;
	/*连接状态*/
	virtual uint32_t getNetStatus() = 0;

	/*连接地址*/
	virtual InetAddress& localAddress() = 0;
	virtual InetAddress& peerAddress() = 0;

	virtual std::mutex& inputMutex() = 0;
	virtual Buffer* inputBuffer() = 0;
	virtual std::mutex& outputMutex() = 0;
	virtual Buffer* outputBuffer() = 0;

	virtual SOCKET fd() = 0;
	virtual int32_t getError() = 0;
};


/********************
NetWrok 核心接口
*********************/
class INetWrok
{
public:
	INetWrok(NetMode mode):
	 m_onNewConnect(defaultNewConnectCallback),
	 m_onConnect(defaultConnectCallback),
	 m_onDisconnect(defaultDisConnectCallback),
	 m_onRecv(defaultReceiveCallback),
	 m_onRecvFrom(defaultReceiveFromCallback),
	 m_onError(defaultErrorCallback)
	{

	};
	virtual ~INetWrok() {};

public:
	/*生命周期函数*/
	virtual bool start() = 0;
	virtual void update() = 0;
	virtual void stop() = 0;

	virtual void setOpenSSL(bool bEnable) = 0;

	/*服务器开启监听*/
	virtual uint64_t linstenTCP(InetAddress& address, TCPServerConfig& config) = 0;
	virtual uint64_t linstenTCP(std::string ip, uint16_t port, TCPServerConfig& config) = 0;
	//virtual uint64_t linsten(InetAddress& address) = 0;
	//virtual uint64_t linsten(std::string ip, uint16_t port) = 0;

	virtual uint64_t bindUDP(InetAddress& address) = 0;
	virtual uint64_t bindUDP(std::string ip, uint16_t port) = 0;

	/*异步连接:
	* 返回连接对象NetID
	*/
	virtual uint64_t asynConnect(InetAddress& address, uint64_t timeout = 10000) = 0;
	virtual uint64_t asynConnect(std::string ip, uint16_t port, uint64_t timeout = 10000) = 0;

	/*
	* 同步连接:
	* 连接成功返回连接对象NetID
	* 连接失败返回0
	*/
	virtual uint64_t connect(InetAddress& address, uint64_t timeout=10000) = 0;
	virtual uint64_t connect(std::string ip, uint16_t port, uint64_t timeout=10000) = 0;


	virtual bool send(uint64_t net_id, const char* data, size_t size) = 0;
	virtual bool sendTo(InetAddress& address, const char* data, size_t size) = 0;

	/* 关闭连接:
	*  关闭过程回调DisConnect
	*/
	virtual void close(uint64_t net_id) = 0;

	/* 主动断开连接
	*  结果不回调DisConnect
	*/
	virtual void disconnect(uint64_t net_id) = 0;
	
	/*获取网络对象
	*/
	virtual std::shared_ptr<INetObj> getNetObj(uint64_t net_id) = 0;

	virtual int getNetMode() = 0;

	/*设置回调函数*/
	void setNewConnectCallback(NewConnectCallback callback) { m_onNewConnect = callback; }
	void setConnectCallback(ConnectCallback callback) { m_onConnect = callback; }
	void setDisConnectCallback(DisConnectCallback callback) { m_onDisconnect = callback; }
	void setRecvCallback(ReceiveCallback callback) { m_onRecv = callback; }
	void setRecvFromCallback(ReceiveFromCallback callback) { m_onRecvFrom = callback; }
	void setErrorCallback(ErrorCallback callback) { m_onError = callback; }

public:
	NewConnectCallback m_onNewConnect;
	ConnectCallback m_onConnect;
	DisConnectCallback m_onDisconnect;
	ReceiveCallback m_onRecv;
	ReceiveFromCallback m_onRecvFrom;
	ErrorCallback m_onError;
};

/********************
获取NetWork对象
********************/
std::shared_ptr<INetWrok> getNetwork(NetMode mode);
