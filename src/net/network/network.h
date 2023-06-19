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
#include <map>
#include <mutex>
#include <shared_mutex>
#include <atomic>

#define NETDEBUG getLog().Debug()
#define NETINFO  getLog().Error()
#define NETWARN  getLog().Warn()
#define NETERROR getLog().Error()
#define NETFATAL getLog().Fatal()

class Network : public INetWrok
{
public:
	explicit Network(NetMode mode = NetMode::TCP);
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

	int getNetMode() override;
	std::shared_ptr<INetObj> getNetObj(uint64_t net_id) override;

/*以下接口不暴露*/
public:
	void rlease();
	Log& getLog() { return m_log; };
	void deleteNetObj(uint64_t net_id);
	uint64_t getNextNetID();
	std::shared_ptr<BaseNetObj> getServerNetObj();
	SOCKET getListenSock();
	//uint64_t processAccept(SOCKET sock,int32_t error);

	void asyncConnectResult(uint64_t net_id, int32_t err);
	void processAsynConnectTimeOut();

	/*增删改查*/
	std::shared_ptr<BaseNetObj> makeNetObj(Network* network, sa_family_t family);
	std::shared_ptr<BaseNetObj> makeNetObj(Network* network, SOCKET sock);

	std::shared_ptr<BaseNetObj> getNetObj2(uint64_t net_id);
	
	bool insertNetObj(std::shared_ptr<BaseNetObj> netObj,bool addPoll=true);
	void removeNetObj(uint64_t net_id);

private:
	NetMode m_mode;
	std::atomic<uint64_t> m_net_id;
	
	SOCKET m_idle_fd;/*空置Sock*/

	/*网络对象,(主线程，IO线程)*/
	std::shared_mutex m_net_mutex;
	std::unordered_map<uint64_t, std::shared_ptr<BaseNetObj>> m_net_objs;

	std::shared_ptr<BaseNetObj> m_server_obj;
	std::shared_ptr<Poll> m_poll;

	/*异步连接队列 (主线程，IO线程)*/
	std::mutex m_connect_mutex;
	std::map<uint64_t,ConnectInfo> m_connecting;

	//std::queue<uint64_t> m_disconnect;
	Log m_log;

	/*设置*/
	uint8_t m_ssl;
	uint8_t m_epoll_et;
};
