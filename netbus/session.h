#ifndef __SESSION_H__
#define __SESSION_H__

#define MAXDATALEN 4096
#define IP_LEN 32
#include "uv.h"
#include "base_session.h"
enum Socket_Type
{
	TCP_SOCKET = 0,
	WEB_SOCKET = 1
};
class session :public base_session
{

public:
	char RecvData[MAXDATALEN];
	unsigned int DataLen;
	char *LongData;
	unsigned int Long_Data_Size;
public:
	char IP_Addr[IP_LEN];
	unsigned int port;
	bool is_shutdown;
public:
	uv_tcp_t handler;
	uv_shutdown_t shutdown_handler;
	int socket_type;
	bool is_shakehand;
public:
	static session* create();
	static void destroy(session* session);
	void init_session();
	void close();
	void SendData(char* body, unsigned int len);
	const char* get_address(unsigned int* client_port);
	static void create_cacher();
	void send_cmd_msg(struct cmd_msg* msg);
	void send_raw_cmd(struct raw_cmd* msg);
};


#endif