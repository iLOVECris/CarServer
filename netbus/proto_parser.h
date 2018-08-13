#ifndef __PROTO_PARSER_H__
#define __PROTO_PARSER_H__
#include <string>
#include <map>
#include "google/protobuf/message.h"
using namespace std;
enum {
	PROTO_JSON = 0,
	PROTO_BUF = 1,
};
struct cmd_msg {
	int stype;
	int ctype;
	unsigned int utag;
	void* body; // JSON str 或者是message;
};
struct raw_cmd
{
	int stype;
	int ctype;
	unsigned int utag;
	int raw_len;//长度
	unsigned char* body; // JSON str 或者是message;
};
class proto_parser {
public:
	static void init(int proto_type);
	static void register_pf_cmd_map(map<int, string>&cmd_map);
	static int proto_type();

	static bool decode_cmd_msg(unsigned char* cmd, int cmd_len, struct cmd_msg** out_msg);
	static bool decode_raw_cmd_msg(unsigned char* cmd, int cmd_len, struct raw_cmd* out_msg);

	static void cmd_msg_free(struct cmd_msg* msg);
	static unsigned char* encode_msg_to_raw(const struct cmd_msg* msg, int* out_len);
	static void msg_raw_free(unsigned char* raw);
	static const char* protobuf_cmd_name(int ctype);
	static google::protobuf::Message* create_message(const char* type_name);
	static void release_message(google::protobuf::Message* m);
};



#endif
