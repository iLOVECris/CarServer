#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include "uv.h"
#include "base_session.h"
#include "Service.h"
#include "session.h"
//1:  service��ӽӿڣ�ע��ת������, ����ͨ����������ǲ��ý�����������;
//2:  ��дgw_serviceģ��, Ϊÿ��session����ukey������session, ����ǵ�½�˺����uid;
//3:  �ж�session�Ƿ�Ϊ�ͻ��˷����������ݣ������ת����������, ����utag-->���ݰ�����;
//4:  �ж�session�Ƿ�Ϊ������ת���������ݣ�����Ǹ���utag����session, �����ͻ��� ��
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