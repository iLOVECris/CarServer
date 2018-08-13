#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<iostream>
using namespace std;

#include "uv.h"
#include "base_session.h"
#include "session.h"
#include "../utils/cache_allocer.h"
#include "ws_protocol.h"
#include "tcp_protocol.h"
#include "proto_parser.h"
#include "ServiceManager.h"

#define SESSION_CAPACITY 6000
#define WRITE_CAPACITY 4096
#define WBUFFER_CAPACITY 1024
#define WBUFFER_SIZE 1024


static cache_allocer* allocer = NULL;
static cache_allocer* write_allocer = NULL;
cache_allocer* wbuf_allocer = NULL;
extern "C"
{
	static void after_write(uv_write_t* req, int status) {
		if (status == 0) {
		}
		cache_free(write_allocer, req);
	}
	static void on_close(uv_handle_t* handle)
	{
		session* s = (session*)handle->data;
		session::destroy(s);
	}
	static void on_shutdown(uv_shutdown_t* req, int status)
	{
		session* s = (session*)req->data;

		uv_close((uv_handle_t*)req->handle, on_close);
	}
}

void session::create_cacher()
{
	if (allocer == NULL)
	{
		allocer = create_cache_allocer(SESSION_CAPACITY, sizeof(session));
	}
	if (write_allocer == NULL)
	{
		write_allocer = create_cache_allocer(WRITE_CAPACITY, sizeof(uv_write_t));
	}
	if (wbuf_allocer == NULL)
	{
		wbuf_allocer = create_cache_allocer(WRITE_CAPACITY, WBUFFER_SIZE);
	}
}

session* session::create()
{
	session* s = (session*)cache_alloc(allocer,sizeof(session));
	s->session::session();
	s->init_session();
	return s;
}
void session::destroy(session* _session)
{
	if (_session != NULL)
	{
		_session->session::~session();
		cache_free(allocer,_session);
	}
}

void session::SendData(char* body, unsigned int len)
{
	if(this->socket_type == Socket_Type::WEB_SOCKET)//发送websocket格式数据
	{
		if (this->is_shakehand)
		{
			uv_write_t* w_req = (uv_write_t*)cache_alloc(write_allocer, sizeof(uv_write_t));
			int datalen = 0;
			unsigned char* c = ws_protocol::package_ws_data((unsigned char*)body, len, &datalen);
			uv_buf_t w_buf = uv_buf_init((char*)c, datalen);
			uv_write(w_req, (uv_stream_t*)&this->handler, &w_buf, 1, after_write);
			ws_protocol::free_package_data(c);
		}
		else//没握手成功,按照TCP格式发送
		{
			uv_write_t* w_req = (uv_write_t*)cache_alloc(write_allocer, sizeof(uv_write_t));
			uv_buf_t w_buf = uv_buf_init(body, len);
			uv_write(w_req, (uv_stream_t*)&this->handler, &w_buf, 1, after_write);
		}
	}
	else//发送TCP格式数据
	{
		uv_write_t* w_req = (uv_write_t*)cache_alloc(write_allocer, sizeof(uv_write_t));
		int datalen = 0;
		unsigned char* c = tcp_protocol::package((unsigned char*)body, len, &datalen);
		uv_buf_t w_buf = uv_buf_init((char*)c, datalen);
		uv_write(w_req, (uv_stream_t*)&this->handler, &w_buf, 1, after_write);
		tcp_protocol::release_package(c);
	}

}
const char* session::get_address(unsigned int* client_port)
{
	client_port = &this->port;
	return this->IP_Addr;
}

void session::init_session()
{
	memset(this->RecvData, 0, sizeof(RecvData));
	memset(this->IP_Addr, 0, sizeof(IP_Addr));
	this->port = 0;
	this->DataLen = 0;
	this->is_shutdown = false;
	this->is_shakehand = false;
	this->LongData = NULL;
	this->Long_Data_Size = 0;
}
void session::close()//服务器主动关闭(与服务器关闭连接)
{
	if (is_shutdown)
	{
		return;
	}
	ServiceManager::on_session_disconnect(this);
	is_shutdown = true;
	memset(&shutdown_handler, 0, sizeof(uv_shutdown_t));
	uv_shutdown(&shutdown_handler, (uv_stream_t*)&handler, on_shutdown);
}

void session::send_cmd_msg(cmd_msg* msg)
{
	unsigned char* encode_data = NULL;
	int out_len = 0;
	encode_data = proto_parser::encode_msg_to_raw(msg, &out_len);
	if (encode_data)
	{
		//LoginReq req;
		//req.ParseFromArray(encode_data + 8, out_len - 8);
		//printf("..1..%s: %d\n", req.name().c_str(), req.age());
		SendData((char*)encode_data, out_len);
		proto_parser::msg_raw_free(encode_data);
	}
}

void session::send_raw_cmd(struct raw_cmd* msg)
{
	SendData((char*)msg->body, msg->raw_len);
}