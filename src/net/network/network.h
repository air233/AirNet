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

	/*�������ں���*/
	bool start() override;
	void update() override;
	void stop() override;

	//TODO:����SSL/TLS���� ����չ
	virtual void setOpenSSL(bool bEnable) override;

	/*��������*/
	uint64_t linstenTCP(InetAddress& address, int32_t backlog, TCPServerConfig& config) override;
	uint64_t linstenTCP(std::string ip, uint16_t port, int32_t backlog, TCPServerConfig& config) override;

	uint64_t bindUDP(InetAddress& address) override;
	uint64_t bindUDP(std::string ip, uint16_t port) override;

	/*�첽����*/
	uint64_t asynConnect(InetAddress& address, uint64_t timeout = 10000) override;
	uint64_t asynConnect(std::string ip, uint16_t port, uint64_t timeout = 10000) override;

	/*ͬ������*/
	uint64_t connect(InetAddress& address, uint64_t timeout = 10000) override;
	uint64_t connect(std::string ip, uint16_t port, uint64_t timeout = 10000) override;

	/*TCP��������*/
	bool send(uint64_t net_id, const char* data, size_t size) override;

	/*UDP��������*/
	bool sendTo(InetAddress& address, const char* data, size_t size) override;

	/*�ر�����*/
	void close(uint64_t net_id) override;
	/*�Ͽ�����*/
	void disconnect(uint64_t net_id) override;

	int getNetMode() override;
	std::shared_ptr<INetObj> getNetObj(uint64_t net_id) override;

/*========================���½ӿڲ���¶���û���========================*/
public:
	void rlease();
	Log& getLog() { return m_log; };
	void deleteNetObj(uint64_t net_id);
	uint64_t getNextNetID();
	std::shared_ptr<BaseNetObj> getServerNetObj();
	SOCKET getListenSock();
	//uint64_t processAccept(SOCKET sock,int32_t error);

	/*ͬ���첽���ý��(IO�߳�)*/
	void asyncConnectResult(uint64_t net_id, int32_t err);

	/*�����첽���ӳ�ʱ(���߳�)*/
	void processAsynConnectTimeOut();

	/*NetObj��ɾ�Ĳ�*/
	std::shared_ptr<BaseNetObj> makeNetObj(Network* network, sa_family_t family);
	std::shared_ptr<BaseNetObj> makeNetObj(Network* network, SOCKET sock);
	std::shared_ptr<BaseNetObj> getNetObj2(uint64_t net_id);
	bool insertNetObj(std::shared_ptr<BaseNetObj> netObj,bool addPoll=true);
	void removeNetObj(uint64_t net_id);

private:
	int32_t m_run;

	/*�����ģʽ*/
	NetMode m_mode;

	/*��ǰ�����NetID*/
	std::atomic<uint64_t> m_net_id;
	
	/*����Sock ��������FD��Դʹ�����,AcceptFDһֱ�ɶ�����*/
	SOCKET m_idle_fd;

	/*�������,(���̣߳�IO�߳�)*/
	std::shared_mutex m_net_mutex;
	std::unordered_map<uint64_t, std::shared_ptr<BaseNetObj>> m_net_objs;
	
	/*�����������*/
	std::shared_ptr<BaseNetObj> m_server_obj;
	uint64_t m_server_net_id;
	
	/*�����¼�������*/
	std::shared_ptr<Poll> m_poll;

	/*�첽���Ӷ��� (���̣߳�IO�߳�)*/
	std::mutex m_connect_mutex;
	std::map<uint64_t,ConnectInfo> m_connecting;

	Log m_log;

	/*����չ����*/
	uint8_t m_ssl;
};
