#ifndef __NETBUS_H__
#define __NETBUS_H__
class session;
class netbus
{
public:
	static netbus* instacne();
	void Start_Tcp_Server(int port);
	void Start_WebSocket_Server(int port);
	void Start_UdpSocket_Server(int port);
	void run();
	void Tcp_Connect(const char* ip, int port, void (*callback)(int err, session* s, void* udata), void*data);
	void init_cache();
};
#endif

