#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<iostream>
#include <string>
using namespace std;

#include "uv.h"
#include "../3rd/crypto/base64_encoder.h"
#include "../3rd/crypto/sha1.h"
#include "../3rd/http_parser/http_parser.h"
#include "base_session.h"
#include "session.h"
#include "ws_protocol.h"
#include "../../utils/cache_allocer.h"
extern struct cache_allocer* wbuf_allocer;
#define STR_SIZE 512

static char *magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";//秘钥
static char *wb_accept = "HTTP/1.1 101 Switching Protocols\r\n"
"Upgrade:websocket\r\n"
"Connection: Upgrade\r\n"
"Sec-WebSocket-Accept: %s\r\n"
"WebSocket-Protocol:chat\r\n\r\n";

static int is_parse_ok = 0;
static int is_sec_key = 0;//检索出seckey
static char sec_key[STR_SIZE];//客户端发送过来报文中的key

int on_header_field(http_parser*p, const char *at, size_t length)
{
	if (strncmp(at, "Sec-WebSocket-Key", length) == 0)
	{
		is_sec_key = 1;
	}
	else
	{
		is_sec_key = 0;
	}
	return 0;
}
int on_header_value(http_parser*p, const char *at, size_t length)
{
	if (is_sec_key == 0)
	{
		return 0;
	}
	memcpy(sec_key, at, length);
	sec_key[length] = 0;
	is_parse_ok = 1;
	//printf("key:%s\n", sec_key);

	is_sec_key = 0;
	return 0;
}

bool ws_protocol::ws_shake_hande(session* s, unsigned char* body, int len)
{
	http_parser_settings setting;
	http_parser parse;
	http_parser_settings_init(&setting);
	http_parser_init(&parse, HTTP_REQUEST);
	setting.on_header_field = on_header_field;
	setting.on_header_value = on_header_value;
	is_parse_ok = 0;
	http_parser_execute(&parse, &setting, (char*)body, len);
	printf("%s\n", body);
	if (is_parse_ok == 1)//解析key成功
	{
		static char magicandkey[STR_SIZE];//加密前的key
		static char Crypt_Magic[STR_SIZE];//加密后的key
		static char send_client[STR_SIZE];//发送到客户端的数据
		sprintf(magicandkey, "%s%s", sec_key, magic);
		int sha1_size = 0;
		crypt_sha1((unsigned char*)magicandkey, strlen(magicandkey), (unsigned char*)&Crypt_Magic, &sha1_size);
		int base64_len = 0;
		char* base_buf = base64_encode((uint8_t*)Crypt_Magic, sha1_size, &base64_len);

		sprintf(send_client, wb_accept, base_buf);
		base64_encode_free(base_buf);//坑 使用第三方C语言库需要使用extern "C"
		s->SendData(send_client, strlen(send_client));
		s->is_shakehand = true;
		s->DataLen = 0;
		return true;
	}
	return false;
}


int ws_protocol::read_ws_header(unsigned char* pkg_data, int pkg_len, int* pkg_size, int* out_header_size)//坑  pkg_data一定要是unsigned char*
{
	int signed_len = 2;
	if (pkg_len < signed_len)
	{
		return 0;
	}
	if (pkg_data[0] != 0x81 && pkg_data[0] != 0x82)//发送过来的数据不是正常格式
	{
		return 0;
	}
	int data_len = pkg_data[1] & 0x7f;
	if (data_len == 126)//后面有两个字节表示长度
	{
		signed_len += 2;
		if (pkg_len < signed_len)
		{
			return 0;
		}
		data_len = pkg_data[2] << 8 | pkg_data[3];
	}
	else if (data_len == 127)//后面有八个字节表示长度
	{
		signed_len += 8;
		if (pkg_len < signed_len)
		{
			return 0;
		}
		int low = pkg_data[5] | pkg_data[4] << 8 | pkg_data[3] << 16 | pkg_data[2] << 24;
		int height = pkg_data[9] | pkg_data[8] << 8 | pkg_data[7] << 16 | pkg_data[6] << 24;

		data_len = low;
	}
	*pkg_size = data_len+signed_len+4;//4个掩码
	*out_header_size = signed_len;
	return 1;
}
void ws_protocol::parser_ws_recv_data(unsigned char* raw_data, unsigned char* mask, int raw_len)
{
	for (int i = 0; i < raw_len; i++)
	{
		raw_data[i] = raw_data[i] ^ mask[i % 4];
	}

	char recv_data[STR_SIZE];
	memcpy(recv_data, raw_data, raw_len);
	recv_data[raw_len] = 0;
}
unsigned char* ws_protocol::package_ws_data(unsigned char* raw_data, int len, int* ws_data_len)
{
	unsigned char* buf = (unsigned char*)cache_alloc(wbuf_allocer,STR_SIZE);
	int signed_len = 2;
	buf[0] = 0x82;//0x81表示字符串  0x82表示二进制
	if (len > 0 && len <= 125)
	{
		buf[1] = len;
	}
	else if (len >125 && len < 65535)
	{
		buf[1] = 126;
		buf[2] = (len&0x0000FF00) >> 8;
		buf[3] = (len & 0x000000FF);
		signed_len += 2;
	}
	else//长度太长  不考虑
	{

	}
	memcpy(buf + signed_len, raw_data, len);
	*ws_data_len = len + signed_len;
	return buf;
}
void ws_protocol::free_package_data(unsigned char* ws_pkg)
{
	cache_free(wbuf_allocer,ws_pkg);
}