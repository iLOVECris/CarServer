#ifndef __SERVICE_H__
#define __SERVICE_H__

class session;
class Service {
public:
	bool using_raw_cmd;
	Service()
	{
		using_raw_cmd = false;
	}
public:
	virtual bool on_session_recv_cmd(session* s, struct cmd_msg* msg);
	virtual void on_session_disconnect(session* s,int stype);
	virtual bool on_session_recv_rawcmd(session*s, struct raw_cmd* msg);
};


#endif