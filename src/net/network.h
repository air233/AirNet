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

	/*����*/
	bool linsten();
	bool bind();
	bool accept();

	/*�������ں���*/
	bool start();
	void update();
	void stop();

	uint64_t connect(InetAddress& address);
	uint64_t asynConnect(InetAddress& address);

	bool send(uint64_t net_id, const char* data, size_t size);
	
	/*�ص�*/
	void onConnet(uint64_t net_id);

	void onDisconnet(uint64_t net_id);

	void onRecv(uint64_t net_id, const char* data, size_t size);

	//����һ������
	//xxxx getConnect(uint64_t net_id);

private:
	uint64_t m_net_id;

	int32_t m_init;

	//TODO:һ����������
	//map<uint_64,xxxx> m_

	std::shared_ptr<BaseSocket> m_accept;

	std::shared_ptr<Poll> m_poll;

	//�첽connect queue
};
