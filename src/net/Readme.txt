封装socket连接

TCPServer server("0.0.0.0","8888");
server.loop();








GameWorld world;

GameServer server;

EventLoop loop;
loop.Register(server);


world.net = server;

world.log = log;

控制权在框架

network.start();
	监听线程


network.update();
network.stop();


network.connect();
network.listen();
network.send();

network.onError();
network.onConnect();
network.onDisconnect();
network.onRecv(data,size);
network.onMessageDecode();


//读取
network.recv();


//一个待write数据的socket对象集合

set<XX> send_queue;

//发送数据接口
network.send(XX obj, const char* data, size_t size);
{
	if(send_buff.size() == 0)
	{
		send_len = write(obj.fd, data, len);
		if(send_len <= 0)
		{
			if(errno == EWOULDBLOCK)
			{
				obj.buff.pushCString(data,size);

				//添加监听写事件
				poll.addevent(obj.fd,write);
			}

		}
	}
	else
	{
		obj.buff.pushCString(data,size);

		//添加监听写事件
		poll.addevent(obj.fd,write);
	}
}

void workThread()
{
	event = work_poll.wait(events);

	if(event > 0)
	{
		readOrwrite(events);
	}
}

//Decode;


message_queue;


//socket写入
while(true)
{
	char data[1024]={0};
	len = read(socket.fd,data,1024);
	if(len > 0)
	{
		socket.buff.pushCString(data,len);
	}
	else if(len == 0)
	{
		break;
	}
	else
	{
		err = errno;
		// 处理读取错误
        // 可以根据具体需求进行适当的错误处理，如记录日志、断开连接等
        break;
	}
}


//解码
Buff temp_buff;
{
	lock(socket.buff);
	temp_buff.swap(socket.buff);
}

default_decode(temp_buff);

if(temp_buff.size() > 0)
{
	lock(socket.buff);
	socket.buff.insertCString(temp_buff,0,temp_buff.size())
}




//编码
data = default_encode();
network.send(data,data_size);




//socket发送
while(socket.buff.size() > 0)
{
	char data[1024]={0};
	len = socket.peekCString(data,1024);
	send_len = write(socket.fd,data,len);

	if(send_len > 0)
	{
		if(send_len < len)
		{
			socket.buff.insertCString(data,send_len,len-send_len);
			break;
		}
	}
	else
	{
		if(errno != EWOULDBLOCK)
		{
			
		}
	}
}


swapMessage();

msg = getMessage();


void update()
{
 	message = getMessage();

	//处理消息
}




线程池


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

