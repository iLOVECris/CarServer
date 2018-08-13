#ifndef __SERVICE_MANAGER_H__
#define __SERVICE_MANAGER_H__
class session;
class Service;


class ServiceManager {
public:
	static bool register_service(int stype, Service* s);
	static bool on_recv_cmd_msg(session* s, struct cmd_msg* msg);
	static bool on_recv_cmd_raw_msg(session* s, struct raw_cmd* msg);
	static void on_session_disconnect(session* s);
	static void init();
};


#endif
