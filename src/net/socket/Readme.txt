封装socket连接

TCPServer server("0.0.0.0","8888");
server.loop();








GameWorld world;

GameServer server;

server.Register("模块1");
server.Register("模块2");
server.Register("模块3");

world.net = server;

world.log = log;

控制权在框架


//读取
char data[1024]={0};
len = read(socket.fd,data,1024);
socket.pushCString(data,len);

//发送
char data[1024]={0};
len = socket.pushCString(data,1024);
send_len = write(socket.fd,data,len);


生命周期函数

Init();
Update();
Uninit();










server.Run();


void Run()
{
	while(run)
	{
		m_poNet->update();
		m_xxxxx->update();
	}
}

m_poNet->send();




连接建立
连接断开
消息到达
消息发送完毕

server.onRecv();
server.onClose();
server.onAccept();
server.onConnect();


server_socket.send();
server_socket.close();
server_socket.connect();

