#ifndef __UDP_SESSION_H__
#define __UDP_SESSION_H__

class udp_session : base_session
{
public:
	uv_udp_t* udp_handler;
	char c_address[32];
	int c_port;
	const struct sockaddr* addr;

public:
	virtual void close();
	virtual void SendData(char* body, unsigned int len);
	virtual const char* get_address(unsigned int* client_port);
	virtual void send_cmd_msg(struct cmd_msg* msg);
};

#endif