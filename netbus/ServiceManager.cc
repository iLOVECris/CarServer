#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "uv.h"
#include "base_session.h"
#include "session.h"
#include "proto_parser.h"
#include "Service.h"
#include "ServiceManager.h"


#define MAX_SERVICE_SIZE 256

static Service* g_ServiceSet[MAX_SERVICE_SIZE];

bool ServiceManager::register_service(int stype, Service* s)
{
	if (stype<0 || stype>MAX_SERVICE_SIZE || g_ServiceSet[stype] != NULL)
	{
		return false;
	}
	g_ServiceSet[stype] = s;
	return true;
}
bool ServiceManager::on_recv_cmd_msg(session* s, struct cmd_msg* msg)
{
	if (g_ServiceSet[msg->stype] == NULL)
	{
		return false;
	}
	g_ServiceSet[msg->stype]->on_session_recv_cmd(s, msg);
	return true;
}
bool ServiceManager::on_recv_cmd_raw_msg(session* s, struct raw_cmd* msg)
{
	if (g_ServiceSet[msg->stype] == NULL)
	{
		return false;
	}
	if (g_ServiceSet[msg->stype]->using_raw_cmd)//网关的服务是直接转发
	{
		g_ServiceSet[msg->stype]->on_session_recv_rawcmd(s, msg);
	}
	else //其他服务的处理
	{
		cmd_msg* cmdmsg = NULL;
		if (!proto_parser::decode_cmd_msg(msg->body, msg->raw_len, &cmdmsg))
		{
			return false;
		}
		if (!ServiceManager::on_recv_cmd_msg((session*)s, cmdmsg)) {
			s->close();
			proto_parser::cmd_msg_free(cmdmsg);
			return false;
		}
		if(cmdmsg!=NULL)
			proto_parser::cmd_msg_free(cmdmsg);
	}
	return true;
}
void ServiceManager::on_session_disconnect(session* s)
{
	for (int index = 0; index < MAX_SERVICE_SIZE; index++)
	{
		if (g_ServiceSet[index] == NULL)
		{
			continue;
		}
		g_ServiceSet[index]->on_session_disconnect(s,index);
	}
}

void ServiceManager::init()
{
	memset(g_ServiceSet, 0, sizeof(g_ServiceSet));
}