#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "uv.h"
#include "base_session.h"
#include "udp_session.h"
#include "proto_parser.h"
#include "../../utils/small_alloc.h"

void on_udp_send(uv_udp_send_t* req, int status)
{
	if (req != NULL)
	{
		free_small_alloc(req);
		req = NULL;
	}
	if (status == 0) {
		printf("send sucess\n");
	}
}

void udp_session::close()
{

}
void udp_session::SendData(char* body, unsigned int len)
{
	uv_udp_send_t* send_handle = (uv_udp_send_t*)small_alloc(sizeof(uv_udp_send_t));

	uv_buf_t buffer = uv_buf_init(body, len);
	int send_len = uv_udp_send(send_handle, this->udp_handler, &buffer, 1, addr, on_udp_send);
}
const char* udp_session::get_address(unsigned int* client_port)
{
	*client_port = this->c_port;
	return this->c_address;
}
void udp_session::send_cmd_msg(struct cmd_msg* msg)
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