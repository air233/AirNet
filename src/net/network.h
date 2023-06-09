#include <memory>
#include <map>
#include "socket/basesocket.h"
#include "poller/poll.h"
#include "until/inetAddress.h"
class Network
{
public:
	static std::shared_ptr<BaseSocket> makeSocket(SOCKET_TYPE mode);

public:
	explicit Network(SOCKET_TYPE mode = TCP);

	/*方法*/
	bool linsten();
	bool bind();
	bool accept();

	/*生命周期函数*/
	bool start();
	void update();
	void stop();

	uint64_t connect(InetAddress& address);
	uint64_t asynConnect(InetAddress& address);

	bool send(uint64_t net_id, const char* data, size_t size);
	
	/*回调*/
	void onConnet(uint64_t net_id);

	void onDisconnet(uint64_t net_id);

	void onRecv(uint64_t net_id, const char* data, size_t size);

	//返回一个连接
	//xxxx getConnect(uint64_t net_id);

private:
	uint64_t m_net_id;

	int32_t m_init;

	//TODO:一个连接容器
	//map<uint_64,xxxx> m_

	std::shared_ptr<BaseSocket> m_accept;

	std::shared_ptr<Poll> m_poll;

	//异步connect queue
};
