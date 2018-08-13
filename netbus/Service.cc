#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include "uv.h"
#include "base_session.h"
#include "Service.h"
#include "session.h"
//1:  service添加接口，注册转发服务, 和普通服务的区别是不用解码命令内容;
//2:  编写gw_service模块, 为每个session生成ukey来保存session, 如果是登陆了后就用uid;
//3:  判断session是否为客户端发过来的数据，如果是转发给服务器, 打上utag-->数据包里面;
//4:  判断session是否为服务器转发过来数据，如果是根据utag来找session, 发给客户端 ；
//
bool Service::on_session_recv_cmd(session* s, struct cmd_msg* msg)
{
	return false;
}

void Service::on_session_disconnect(session* s,int stype)
{
	s->close();
}
bool Service::on_session_recv_rawcmd(session*s, struct raw_cmd* msg)
{
	return false;
}