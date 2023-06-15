#include "../inetwork.h"
#include "../netobj/basenetobj.h"
#include "../poller/poll.h"
#include "../../common/log/log.h"
#include "../../common/buffer/buffer.h"

#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <set>

#define NETDEBUG getLog().Debug()
#define NETINFO  getLog().Error()
#define NETWARN  getLog().Warn()
#define NETERROR getLog().Error()
#define NETFATAL getLog().Fatal()

class Network : public INetWrok
{
public:
	explicit Network(NetMode mode = TCP);
	~Network();

	/*生命周期函数*/
	bool start() override;
	void update() override;
	void stop() override;

	//TODO:设置SSL/TLS连接
	virtual void setOpenSSL(bool bEnable) override;

	uint64_t linstenTCP(InetAddress& address, TCPServerConfig& config) override;
	uint64_t linstenTCP(std::string ip, uint16_t port, TCPServerConfig& config) override;
	uint64_t linsten(InetAddress& address) override;
	uint64_t linsten(std::string ip, uint16_t port) override;

	uint64_t asynConnect(InetAddress& address, uint64_t timeout = 10000) override;
	uint64_t asynConnect(std::string ip, uint16_t port, uint64_t timeout = 10000) override;

	uint64_t connect(InetAddress& address, uint64_t timeout = 10000) override;
	uint64_t connect(std::string ip, uint16_t port, uint64_t timeout = 10000) override;

	bool send(uint64_t net_id, const char* data, size_t size) override;
	void close(uint64_t net_id) override;

	NetMode getNetMode() override;
	std::shared_ptr<INetObj> getNetObj(uint64_t net_id) override;


/*以下接口不暴露*/
public:
	void rlease();
	std::shared_ptr<BaseNetObj> makeNetObj(Network* network, sa_family_t family);
	Log& getLog() { return m_log; };
	uint64_t getNetID();

private:
	NetMode m_mode;
	int32_t  m_init;
	uint64_t m_net_id;

	/*空置ID*/
	SOCKET m_idle_fd;
	std::unordered_map<uint64_t, std::shared_ptr<BaseNetObj>> m_net_objs;
	std::shared_ptr<BaseNetObj> m_server_obj;
	std::shared_ptr<Poll> m_poll;

	/*连接队列*/
	std::set<uint64_t> m_connecting;
	std::set<uint64_t> m_connected;
	Log m_log;

	/*设置*/
	uint8_t m_ssl;
	uint8_t m_epoll_et;
};
