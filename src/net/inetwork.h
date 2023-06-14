#pragma once
#include <functional>
#include <memory>
#include "nettype.h"
#include "until/inetAddress.h"

class Buffer;

/********************
XXXXCallback �¼��ص��ӿ�
*********************/
typedef std::function<void(const uint64_t)> AcceptCallback;
void defaultAcceptCallback(uint64_t);

typedef std::function<void (const uint64_t)> ConnectCallback;
void defaultConnectCallback(uint64_t);

typedef std::function<void (const uint64_t)> DisConnectCallback;
void defaultDisConnectCallback(uint64_t);

typedef std::function<void (const uint64_t, Buffer*,size_t)> ReceiveCallback;
void defaultReceiveCallback(const uint64_t, Buffer*, size_t);

typedef std::function<void(const uint64_t,int32_t)> ErrorCallback;
void defaultErrorCallback(uint64_t, int32_t);


/********************
NetObj�ṩ��ȡ������Ϣ�ӿ�
*********************/
class INetObj
{
public:
	INetObj() {}
	virtual ~INetObj() {}

	virtual uint64_t getNetID() = 0;
	/*��������*/
	virtual uint32_t getNetMode() = 0;
	/*����״̬*/
	virtual uint32_t getNetStatus() = 0;

	/*���ӵ�ַ*/
	virtual const InetAddress& localAddress() = 0;
	virtual const InetAddress& peerAddress() = 0;

	virtual Buffer* inputBuffer() = 0;
	virtual Buffer* outputBuffer() = 0;

	virtual SOCKET fd() = 0;
	virtual int32_t getError() = 0;
};


/********************
NetWrok ���Ľӿ�
*********************/
class INetWrok
{
public:
	INetWrok(NetMode mode):
	 m_onAccept(defaultAcceptCallback),
	 m_onConnect(defaultConnectCallback),
	 m_onDisconnect(defaultDisConnectCallback),
	 m_onRecv(defaultReceiveCallback),
	 m_onError(defaultErrorCallback)
	{

	};
	virtual ~INetWrok() {};

public:
	virtual bool start() = 0;
	virtual void update() = 0;
	virtual void stop() = 0;

	virtual void setLingerZeroOpt(bool bEnable) = 0;
	virtual void setOpenSSL(bool bEnable) = 0;

	virtual uint64_t linsten(InetAddress& address) = 0;
	virtual uint64_t linsten(std::string ip, std::string port) = 0;

	virtual uint64_t asynConnect(InetAddress& address, uint32_t time) = 0;
	virtual uint64_t connect(InetAddress& address, uint32_t time) = 0;
	virtual uint64_t connect(std::string ip, std::string port, uint32_t time) = 0;

	virtual bool send(uint64_t net_id, const char* data, size_t size) = 0;
	virtual void close(uint64_t net_id) = 0;

	virtual std::shared_ptr<INetObj> getNetObj(uint64_t net_id) = 0;
	virtual NetMode getNetMode() = 0;

	void setAcceptCallback(AcceptCallback callback) { m_onAccept = callback; }
	void setConnectCallback(ConnectCallback callback) { m_onConnect = callback; }
	void setDisConnectCallback(DisConnectCallback callback) { m_onDisconnect = callback; }
	void setRecvCallback(ReceiveCallback callback) { m_onRecv = callback; }
	void setErrorCallback(ErrorCallback callback) { m_onError = callback; }

protected:
	AcceptCallback m_onAccept;
	ConnectCallback m_onConnect;
	DisConnectCallback m_onDisconnect;
	ReceiveCallback m_onRecv;
	ErrorCallback m_onError;
};


/********************
��ȡNetWork����
********************/
/*��ʽ1
INetWrok* getNetwork(NetMode mode);
void rleaseNetwork(INetWrok** network);
*/

/*��ʽ2*/
std::shared_ptr<INetWrok> getNetwork(NetMode mode);