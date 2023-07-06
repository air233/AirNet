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
#define NETINFO  getLog().Info()
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

	//TODO:设置SSL/TLS连接 待拓展
	virtual void setOpenSSL(bool bEnable) override;

	/*监听函数*/
	uint64_t linstenTCP(InetAddress& address, int32_t backlog, TCPServerConfig& config) override;
	uint64_t linstenTCP(std::string ip, uint16_t port, int32_t backlog, TCPServerConfig& config) override;

	uint64_t bindUDP(InetAddress& address) override;
	uint64_t bindUDP(std::string ip, uint16_t port) override;

	/*异步连接*/
	uint64_t asynConnect(InetAddress& address, uint64_t timeout = 10000) override;
	uint64_t asynConnect(std::string ip, uint16_t port, uint64_t timeout = 10000) override;

	/*同步连接*/
	uint64_t connect(InetAddress& address, uint64_t timeout = 10000) override;
	uint64_t connect(std::string ip, uint16_t port, uint64_t timeout = 10000) override;

	/*TCP发送数据*/
	bool send(uint64_t net_id, const char* data, size_t size) override;

	/*UDP发送数据*/
	bool sendTo(InetAddress& address, const char* data, size_t size) override;

	/*关闭连接*/
	void close(uint64_t net_id) override;
	/*断开连接*/
	void disconnect(uint64_t net_id) override;

	int getNetMode() override;
	std::shared_ptr<INetObj> getNetObj(uint64_t net_id) override;

/*========================以下接口不暴露到用户层========================*/
public:
	void rlease();
	Log& getLog() { return m_log; };
	void deleteNetObj(uint64_t net_id);
	uint64_t getNextNetID();
	std::shared_ptr<BaseNetObj> getServerNetObj();
	SOCKET getListenSock();
	//uint64_t processAccept(SOCKET sock,int32_t error);

	/*同步异步调用结果(IO线程)*/
	void asyncConnectResult(uint64_t net_id, int32_t err);

	/*处理异步连接超时(主线程)*/
	void processAsynConnectTimeOut();

	/*NetObj增删改查*/
	std::shared_ptr<BaseNetObj> makeNetObj(Network* network, sa_family_t family);
	std::shared_ptr<BaseNetObj> makeNetObj(Network* network, SOCKET sock);
	std::shared_ptr<BaseNetObj> getNetObj2(uint64_t net_id);
	bool insertNetObj(std::shared_ptr<BaseNetObj> netObj,bool addPoll=true);
	void removeNetObj(uint64_t net_id);

private:
	int32_t m_run;

	/*网络库模式*/
	NetMode m_mode;

	/*当前分配的NetID*/
	std::atomic<uint64_t> m_net_id;
	
	/*空置Sock 用来处理当FD资源使用完后,AcceptFD一直可读问题*/
	SOCKET m_idle_fd;

	/*网络对象,(主线程，IO线程)*/
	std::shared_mutex m_net_mutex;
	std::unordered_map<uint64_t, std::shared_ptr<BaseNetObj>> m_net_objs;
	
	/*网络监听对象*/
	std::shared_ptr<BaseNetObj> m_server_obj;
	uint64_t m_server_net_id;
	
	/*网络事件处理器*/
	std::shared_ptr<Poll> m_poll;

	/*异步连接队列 (主线程，IO线程)*/
	std::mutex m_connect_mutex;
	std::map<uint64_t,ConnectInfo> m_connecting;

	Log m_log;

	/*待拓展设置*/
	uint8_t m_ssl;
};
