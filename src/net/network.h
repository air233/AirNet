#include <memory>
#include <map>
#include "socket/basesocket.h"
#include "poller/poll.h"

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

	bool connect();
	bool asynconnect();
	bool send();
	
	/*回调*/
	void onConnet();
	void onDisconnet();
	void onRecv();

private:
	uint64_t m_net_id;

	int32_t m_init;

	std::shared_ptr<BaseSocket> m_socket;

	std::shared_ptr<Poll> m_poll;

	//异步queue
};
