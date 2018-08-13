#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include<iostream>
using namespace std;

#include "uv.h"
#include "base_session.h"
#include "session.h"
#include "tcp_protocol.h"
#include "ws_protocol.h"
#include "../../utils/cache_allocer.h"
extern cache_allocer* wbuf_allocer;

bool tcp_protocol::read_header(unsigned char* data, int data_len, int* pkg_size, int* out_header_size){
	int signed_len = 2;
	if (data_len < signed_len)
	{
		return false;
	}
	int size = data[0] | data[1] << 8;
	*pkg_size = size;
	*out_header_size = 2;
	return true;
}
unsigned char* tcp_protocol::package(const unsigned char* raw_data, int len, int* pkg_len)
{
	unsigned char* send_pkg = (unsigned char*)cache_alloc(wbuf_allocer,len + 2);
	send_pkg[0] = ((len+2) & 0x000000ff);
	send_pkg[1] = ((len+2) & 0x0000ff00) >> 8;

	memcpy(send_pkg + 2, raw_data, len);
	*pkg_len = len + 2;
	return send_pkg;
}
void tcp_protocol::release_package(unsigned char* tcp_pkg)
{
	cache_free(wbuf_allocer,tcp_pkg);
}