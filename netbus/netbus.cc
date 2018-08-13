#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<iostream>
using namespace std;
#include "uv.h"
#include "base_session.h"
#include "session.h"
#include "udp_session.h"
#include "netbus.h"
#include "ws_protocol.h"
#include "tcp_protocol.h"
#include "proto_parser.h"
#include "../apps/lua/ProtoGen/game.pb.h"
#include "ServiceManager.h"
#include "../../utils/logger.h"

#define IP_ADDR "0.0.0.0"

netbus g_netbus;

struct UDP_Recv
{
	char* data;
	int data_len;
};

extern "C"
{
	static void Recv_Client_Data(base_session* s, unsigned char* body, int pkg_size)
	{

		raw_cmd raw;
		if (!proto_parser::decode_raw_cmd_msg(body, pkg_size, &raw))
		{
			return;
		}
		if (!ServiceManager::on_recv_cmd_raw_msg((session*)s, &raw)) {
			printf("on_recv_cmd_raw_msg return false\n");
			s->close();
		}

	}
	static void on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
	{		
		session *s = (session*)handle->data;//question 第一个包不是整包，第二个来了恰好满了
		if (s->DataLen < MAXDATALEN)//收的数据大于范围 
		{
			*buf = uv_buf_init(s->RecvData + s->DataLen, MAXDATALEN - s->DataLen);
		}
		else
		{
			if(s->LongData==NULL)
			{
				if (s->socket_type == Socket_Type::WEB_SOCKET) 
				{
					if (s->is_shakehand)
					{
						int pkg_size;
						int head_size;
						ws_protocol::read_ws_header((unsigned char*)s->RecvData, s->DataLen, &pkg_size, &head_size);
						s->Long_Data_Size = pkg_size;
						s->LongData = (char*)malloc(pkg_size);
						memcpy(s->LongData, s->RecvData, s->DataLen);
					}
				}
				else
				{
					int pkg_size;
					int head_size;
					tcp_protocol::read_header((unsigned char*)s->RecvData, s->DataLen, &pkg_size, &head_size);
					s->Long_Data_Size = pkg_size;
					s->LongData = (char*)malloc(pkg_size);
					memcpy(s->LongData, s->RecvData, s->DataLen);
				}
			}
			*buf = uv_buf_init(s->LongData + s->DataLen, s->Long_Data_Size - s->DataLen);
		}
	}
	static void on_close(uv_handle_t* handle)
	{
		printf("close client\n");
		session* s = (session*)handle->data;
		session::destroy(s);
	}
	static void on_shutdown(uv_shutdown_t* req, int status)
	{
		printf("on shutdown\n");
		uv_close((uv_handle_t*)req->handle, on_close);
	}
	static void Recv_Tcp_Data(session* s)
	{
		unsigned char* pkg_data = (unsigned char*)((s->LongData != NULL) ? s->LongData : s->RecvData);
		int pkg_size = 0;
		int header_size = 0;

		while (s->DataLen>0)
		{
			if (tcp_protocol::read_header(pkg_data, s->DataLen, &pkg_size, &header_size))//解析头长度足够
			{
				if (s->DataLen < pkg_size)//接受的数据不足一个包
				{
					break;
				}
				unsigned char* body = (unsigned char*)pkg_data + header_size;
				body[pkg_size - header_size] = 0;
				//s->SendData((char*)body, pkg_size);
				Recv_Client_Data((base_session*)s, body, pkg_size-header_size);
				if (s->DataLen > pkg_size)//多余一个包
				{
					memmove(pkg_data, pkg_data + pkg_size, s->DataLen - pkg_size);
				}
				s->DataLen -= pkg_size;

				if (s->DataLen == 0 && s->LongData != NULL)
				{
					free(s->LongData);
					s->LongData = NULL;
					s->Long_Data_Size = 0;
				}
			}
		}
	}
	static void Recv_Web_Data(session* s)
	{
		unsigned char* pkg_data = (unsigned char*)((s->LongData != NULL) ? s->LongData : s->RecvData);
		int pkg_size = 0;
		int header_size = 0;

		while (s->DataLen>0)
		{
			if (ws_protocol::read_ws_header(pkg_data, s->DataLen, &pkg_size, &header_size))//解析头长度足够
			{
				if (pkg_data[0] == 0x88)//客户端关闭
				{
					s->close();
					break;
				}
				if (s->DataLen < (pkg_size))//接受的数据不足一个包
				{
					break;
				}
				unsigned char* mask = (unsigned char*)pkg_data + header_size;
				unsigned char* body = (unsigned char*)pkg_data + header_size + 4;
				ws_protocol::parser_ws_recv_data(body, mask, pkg_size- header_size-4);
				body[pkg_size - header_size - 4] = 0;
				//printf("get cmd:%s\n", body);
				//s->SendData((char*)body, pkg_size);
				Recv_Client_Data((base_session*)s, body, pkg_size);
				if (s->DataLen > pkg_size)//多余一个包
				{
					memmove(pkg_data, pkg_data + pkg_size, s->DataLen - pkg_size);
				}
				s->DataLen -= pkg_size;

				if (s->DataLen == 0 && s->LongData != NULL) 
				{
					free(s->LongData);
					s->LongData = NULL;
					s->Long_Data_Size = 0;
				}
			}
		}

	}
	static void on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
	{
		session* s = (session*)stream->data;
		if (nread < 0)//断开链接
		{		
			s->close();
			return;
		}
		/*printf("recv len is %d\n", nread);
		buf->base[nread] = 0;
		printf("%s\n", buf->base);*/
		//s->SendData(buf->base, nread);
		s->DataLen += nread;
		if (s->socket_type == Socket_Type::WEB_SOCKET)
		{
			if (s->is_shakehand)//已经握手
			{
				Recv_Web_Data(s);
			}
			else//握手 
			{
				ws_protocol::ws_shake_hande(s,(unsigned char*)buf->base, nread);
			}
		}
		else//tcp
		{
			Recv_Tcp_Data(s);
		}
	}
	static void on_connect(uv_stream_t* server, int status)
	{
		session* s = session::create();
		uv_tcp_t* client_handle = &s->handler;
		memset(client_handle, 0, sizeof(uv_tcp_t));
		uv_tcp_init(uv_default_loop(), client_handle);
		
		uv_accept(server, (uv_stream_t*)client_handle);
		client_handle->data = (void*)s;

		struct sockaddr_in addr;
		int len = sizeof(addr);
		uv_tcp_getpeername(client_handle, (sockaddr*)&addr, &len);
		uv_ip4_name(&addr, s->IP_Addr, IP_LEN);
		s->port = ntohs(addr.sin_port);
		s->socket_type = (int)(server->data);
		log_debug("new client comming %s:%d\n", s->IP_Addr, s->port);

		uv_read_start((uv_stream_t*)client_handle, on_alloc, on_read);

	}

	void on_udp_send(uv_udp_send_t* req, int status)
	{
		if (req != NULL)
		{
			free(req);
			req = NULL;
		}
		if (status == 0) {
			printf("send sucess\n");
		}
	}

	void on_udp_recv(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags)
	{

		printf("recv len is :%d\n", nread);

		buf->base[nread] = 0;
		printf("recv is %s\n", buf->base);

		udp_session udp_s;
		udp_s.udp_handler = handle;
		udp_s.addr = addr;
		uv_ip4_name((struct sockaddr_in*)addr, udp_s.c_address, 32);
		udp_s.c_port = ntohs(((struct sockaddr_in*)addr)->sin_port);
		/*uv_udp_send_t* send_handle = (uv_udp_send_t*)malloc(sizeof(uv_udp_send_t));

		uv_buf_t buffer = uv_buf_init("hello", 5);
		int send_len = uv_udp_send(send_handle, handle, &buffer, 1, addr, on_udp_send);*/
		Recv_Client_Data((base_session*)&udp_s, (unsigned char*)buf->base, buf->len);

	}

	void on_uv_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
	{
		UDP_Recv* recv = (UDP_Recv*)handle->data;
		if (recv->data_len<suggested_size)
		{
			if (recv->data)
			{
				free(recv->data);
				recv->data = NULL;
			}
			recv->data = (char*)malloc(suggested_size);
			recv->data_len = suggested_size;
		}
		buf->base = recv->data;
		buf->len = recv->data_len;
	}
}





netbus* netbus::instacne()
{
	return &g_netbus;
}
void netbus::Start_Tcp_Server(int port)
{
	uv_tcp_t* server_handle = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
	uv_tcp_init(uv_default_loop(), server_handle);

	struct sockaddr_in addr;
	uv_ip4_addr(IP_ADDR, port, &addr);

	if (uv_tcp_bind(server_handle, (struct sockaddr*)&addr, 0) != 0)
	{
		free(server_handle);
		printf("bind error\n");
	}
	if (0 != uv_listen((uv_stream_t*)server_handle, SOMAXCONN, on_connect))
	{
		free(server_handle);
		printf("listen error\n");
	}
	server_handle->data = (void*)Socket_Type::TCP_SOCKET;
}
void netbus::Start_UdpSocket_Server(int port)
{
	uv_udp_t* udp_server = (uv_udp_t*)malloc(sizeof(uv_udp_t));
	uv_loop_t* loop = uv_default_loop();
	uv_loop_init(loop);
	uv_udp_init(loop, udp_server);
	struct sockaddr_in addr;
	uv_ip4_addr(IP_ADDR, port, &addr);

	if (uv_udp_bind(udp_server, (sockaddr*)&addr, 0) != 0)
	{
		free(udp_server);
		printf("bind error\n");
	}
	UDP_Recv* recv = (UDP_Recv*)malloc(sizeof(UDP_Recv));
	memset(recv, 0, sizeof(UDP_Recv));
	udp_server->data = recv;
	uv_udp_recv_start(udp_server, on_uv_alloc, on_udp_recv);
	//uv_run(loop, UV_RUN_DEFAULT);
}
void netbus::Start_WebSocket_Server(int port)
{
	uv_tcp_t* server_handle = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
	uv_tcp_init(uv_default_loop(), server_handle);

	struct sockaddr_in addr;
	uv_ip4_addr(IP_ADDR, port, &addr);

	if (uv_tcp_bind(server_handle, (struct sockaddr*)&addr, 0) != 0)
	{
		free(server_handle);
		printf("bind error\n");
	}
	if (0 != uv_listen((uv_stream_t*)server_handle, SOMAXCONN, on_connect))
	{
		free(server_handle);
		printf("listen error\n");
	}
	server_handle->data = (void*)Socket_Type::WEB_SOCKET;
	
}
void netbus::run()
{
	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}
void netbus::init_cache()
{
	ServiceManager::init();
	session::create_cacher();
}

struct UV_Connect
{
	void(*callback)(int err, session* s, void* udata);
	void*data;
};
static void connect_callback(uv_connect_t* req, int status)
{
	UV_Connect* cb = (UV_Connect*)req->data;
	session* s = (session*)req->handle->data;
	if (status)
	{	
		if(cb->callback)
			cb->callback(1, NULL, cb->data);
		s->close();
		free(req->data);
		free(req);
		return;
	}
	if (cb->callback)
		cb->callback(0, s, cb->data);
	uv_read_start((uv_stream_t*)req->handle, on_alloc, on_read);
	if (req->data)
	{
		free(req->data);
	}
	if (req)
	{
		free(req);
	}
}

void netbus::Tcp_Connect(const char* ip, int port, void(*Connected)(int err, session* s, void* udata), void*udata)
{
	struct sockaddr_in addr;
	int a = uv_ip4_addr(ip, port, &addr);

	session* s = session::create();
	uv_tcp_t* client_handle = &s->handler;
	memset(client_handle, 0, sizeof(uv_tcp_t));
	uv_tcp_init(uv_default_loop(), client_handle);
	client_handle->data = s;
	strcpy(s->IP_Addr, ip);
	s->port = port;
	s->as_client = 1;
	s->socket_type = TCP_SOCKET;

	uv_connect_t* connect_handler = (uv_connect_t*)malloc(sizeof(uv_connect_t));
	UV_Connect* cb = (UV_Connect*)malloc(sizeof(UV_Connect));
	cb->callback = Connected;
	cb->data = udata;
	connect_handler->data = cb;
	int ret = uv_tcp_connect(connect_handler, client_handle, (sockaddr*)&addr, connect_callback);
	if (ret)
	{
		return;//error
	}

}