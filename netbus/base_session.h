#ifndef __BASE_SESSION_H__
#define __BASE_SESSION_H__


class base_session
{
public :
	int as_client = 0;
	int utag = 0;
	int uid = 0;
	base_session()
	{
		as_client = 0;
		utag = 0;
		uid = 0;
	}
public:
	virtual void close() = 0;
	virtual void SendData(char* body, unsigned int len) = 0;
	virtual const char* get_address(unsigned int* client_port) = 0;
	virtual void send_cmd_msg(struct cmd_msg* msg) = 0;
};


#endif