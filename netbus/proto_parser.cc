#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <map>

#include<iostream>
#include "uv.h"
#include "proto_parser.h"
#include "google/protobuf/message.h"
#include "../apps/lua/ProtoGen/game.pb.h"
#include "../../utils/logger.h";
#include "../../utils/cache_allocer.h"
#include "../../utils/small_alloc.h"

using namespace std;
extern cache_allocer* wbuf_allocer;
#define MAX_CMD_LEN 1024
#define HEADER_LEN 8

static int protocol_type= PROTO_BUF;

static map<int, string>cmd_map;

google::protobuf::Message* proto_parser::create_message(const char* type_name) 
{
	google::protobuf::Message* message = NULL;

	const google::protobuf::Descriptor* descriptor =
		google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(type_name);
	if (descriptor) {
		const google::protobuf::Message* prototype =
			google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
		if (prototype) {
			message = prototype->New();
		}
	}
	return message;
}

void proto_parser::release_message(google::protobuf::Message* m) {
	delete m;
}

void proto_parser::init(int proto_type)
{
	protocol_type = proto_type;
}
void proto_parser::register_pf_cmd_map(map<int, string>&pf_cmd_map)
{
	for (map<int, string>::iterator it = pf_cmd_map.begin(); it != pf_cmd_map.end(); it++)
	{
		cmd_map[it->first] = it->second;
	}
}
int proto_parser::proto_type()
{
	return protocol_type;
}

const char*proto_parser::protobuf_cmd_name(int ctype)
{
	return cmd_map[ctype].c_str();
}

bool proto_parser::decode_cmd_msg(unsigned char* cmd, int cmd_len, struct cmd_msg** out_msg)
{
	if (cmd_len < HEADER_LEN)//读不到头部
	{
		return false;
	}
	cmd_msg* msg = (cmd_msg*)small_alloc(sizeof(cmd_msg));
	msg->stype = cmd[0] | (cmd[1]<<8);
	msg->ctype = cmd[2] | (cmd[3]<<8);
	msg->utag =	 cmd[4] | (cmd[5] << 8) | (cmd[6] << 16) | (cmd[7]<<24);
	msg->body = NULL;
	*out_msg = msg;
	if (cmd_len == HEADER_LEN)
	{
		return true;
	}
	if (protocol_type == PROTO_BUF)
	{
		google::protobuf::Message* cmd_msg = create_message(cmd_map[msg->ctype].c_str());
		if (cmd_msg==NULL)
		{
			free_small_alloc(msg);
			*out_msg = NULL;
			return false;
		}

		if (!cmd_msg->ParseFromArray(cmd + 8, cmd_len - 8))
		{
			free_small_alloc(msg);
			*out_msg = NULL;
			release_message(cmd_msg);
			return false;
		}
		msg->body = cmd_msg;
		/*LoginReq req;
		req.ParseFromArray(cmd + 8, cmd_len - 8);
		printf("%s: %d\n", req.name().c_str(), req.age());*/
	}
	else
	{
		int json_len = cmd_len - HEADER_LEN;
		char* json_str = (char*)cache_alloc(wbuf_allocer,json_len+1);
		memcpy(json_str, cmd + HEADER_LEN, cmd_len - HEADER_LEN);
		json_str[cmd_len - HEADER_LEN] = 0;
		msg->body = json_str;
	}

	*out_msg = msg;
	return true;
}
bool proto_parser::decode_raw_cmd_msg(unsigned char* cmd, int cmd_len, struct raw_cmd* out_msg)
{
	if (cmd_len < HEADER_LEN)//读不到头部
	{
		return false;
	}
	out_msg->stype = cmd[0] | (cmd[1] << 8);
	out_msg->ctype = cmd[2] | (cmd[3] << 8);
	out_msg->utag = cmd[4] | (cmd[5] << 8) | (cmd[6] << 16) | (cmd[7] << 24);
	out_msg->raw_len = cmd_len;
	out_msg->body = cmd;
	return true;
}

void proto_parser::cmd_msg_free(struct cmd_msg* msg)
{
	if (msg->body)
	{
		if (protocol_type == PROTO_BUF)
		{
			release_message((google::protobuf::Message*)msg->body);
		}
		else
		{
			cache_free(wbuf_allocer,msg->body);
		}
	}
	free_small_alloc(msg);
}

unsigned char* proto_parser::encode_msg_to_raw(const struct cmd_msg* msg, int* out_len)
{
	int raw_len = 0;
	unsigned char* raw_data = NULL;
	if (protocol_type == PROTO_JSON)
	{
		int len = 0;
		char* json_str = NULL;
		if (msg->body != NULL)
		{
			json_str = (char*)msg->body;
			len = strlen(json_str);
		}
		raw_data = (unsigned char*)cache_alloc(wbuf_allocer,HEADER_LEN + len);
		if (msg->body != NULL)
		{
			memcpy(raw_data + HEADER_LEN, json_str, len);
		}
		*out_len = (len + HEADER_LEN);
	}
	else if(protocol_type == PROTO_BUF)
	{
		int pf_len = 0;
		google::protobuf::Message* p_m = NULL;
		if (msg->body != NULL)
		{
			p_m = (google::protobuf::Message*)msg->body;
			pf_len = p_m->ByteSize();
		}
		raw_data = (unsigned char*)cache_alloc(wbuf_allocer,HEADER_LEN + pf_len);
		if (msg->body != NULL)
		{
			if (!p_m->SerializePartialToArray(raw_data + HEADER_LEN, pf_len)) {
				free_small_alloc(raw_data);
				return NULL;
			}
		}
		*out_len = (pf_len + HEADER_LEN);
	}
	raw_data[0] = msg->stype & 0x000000ff;
	raw_data[1] = (msg->stype & 0x0000ff00)>>8;
	raw_data[2] = msg->ctype & 0x000000ff;
	raw_data[3] = (msg->ctype & 0x0000ff00)>>8;
	memcpy(raw_data + 4, &msg->utag, 4);
	return raw_data;

}
void proto_parser::msg_raw_free(unsigned char* raw)
{
	cache_free(wbuf_allocer,raw);
}