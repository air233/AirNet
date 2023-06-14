#include "../inetwork.h"
#include "../netobj/basenetobj.h"
#include "log/log.h"
#include "buffer/buffer.h"
#include "socket/basesocket.h"
#include "poller/poll.h"

#include <unordered_map>
#include <memory>
#include <string>

class Network : public INetWrok
{
public:
	static std::shared_ptr<BaseSocket> makeSocket(NetMode mode);

public:
	explicit Network(NetMode mode = TCP);
	~Network();

	/*�������ں���*/
	bool start() override;
	void update() override;
	void stop() override;

	/*���ýӿ�*/
	//TODO:����SO_LINGER Only TCP
	virtual void setLingerZeroOpt(bool bEnable) override;
	//TODO:����SSL/TLS����
	virtual void setOpenSSL(bool bEnable) override;

	uint64_t linsten(InetAddress& address) override;
	uint64_t linsten(std::string ip,std::string port) override;

	uint64_t connect(InetAddress& address,uint32_t time) override;
	uint64_t connect(std::string ip, std::string port, uint32_t time) override;
	uint64_t asynConnect(InetAddress& address, uint32_t time) override;

	bool send(uint64_t net_id, const char* data, size_t size) override;
	void close(uint64_t net_id) override;

	std::shared_ptr<INetObj> getNetObj(uint64_t net_id) override;
	NetMode getNetMode() override;

	/*���½ӿڲ���¶*/
	void rlease();
	uint64_t getNetID();

protected:
	/*����������socket*/
	SOCKET createSocket();
	void closeSocket(SOCKET socket);

private:
	NetMode m_mode;
	int32_t  m_init;
	uint64_t m_net_id;
	/*����ID*/
	SOCKET m_idle_fd;
	std::unordered_map<uint64_t, std::shared_ptr<BaseNetObj>> m_netobjs;
	//std::shared_ptr<BaseSocket> m_listen_socket;
	std::shared_ptr<Poll> m_poll;

	Log m_log;
	//TODO:�첽connect queue

	/*����*/
	uint8_t m_linger;
	uint8_t m_ssl;
	uint8_t m_epoll_et;
};
