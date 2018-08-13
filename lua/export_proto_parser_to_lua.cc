#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif // !CPLUSPLUS

#include "tolua++.h"

#ifdef __cplusplus
}
#endif // CPLUSPLUS
#include "lua_wrapper.h"
#include "../../utils//logger.h"
#include "tolua_fix.h"
#include "export_proto_parser_to_lua.h"
#include "../../netbus/proto_parser.h"
using namespace google::protobuf;
extern void push_proto_message_tolua(const Message* message);
int lua_init(lua_State* tolua_S)
{
	int argc = lua_gettop(tolua_S);
	if (argc != 1) 
	{
		log_error("param count error");
		return 0;
	}
	int proto_type = lua_tointeger(tolua_S, 1);
	if (proto_type != PROTO_JSON&&proto_type != PROTO_BUF)
	{
		log_error("proto_type error");
		return 0;
	}
	proto_parser::init(proto_type);
	return 0;
}

int lua_proto_type(lua_State* tolua_S)
{
	lua_pushinteger(tolua_S,proto_parser::proto_type());
	return 1;
}

int lua_register_pf_cmd_map(lua_State* tolua_S)
{
	std::map<int, std::string> cmd_map;
	int n = luaL_len(tolua_S, 1);
	if (n <= 0)
	{
		return 0;
	}
	for (int i = 1; i <= n; i++)
	{
		lua_pushnumber(tolua_S, i);
		lua_gettable(tolua_S, 1);
		const char* name = luaL_checkstring(tolua_S, -1);
		if (name) {
			cmd_map[i] = name;
		}
		lua_pop(tolua_S, 1);
	}  
	proto_parser::register_pf_cmd_map(cmd_map);
	return 0;
}
int lua_get_msg(lua_State* tolua_S)
{
	struct raw_cmd* cmd = (raw_cmd*)lua_touserdata(tolua_S, 1);
	if (cmd == NULL)
	{
		log_error("get raw_cmd error");
	}
	if (cmd->body == NULL)
	{
		lua_pushnil(tolua_S);
	}
	cmd_msg* msg; 
	if (proto_parser::decode_cmd_msg(cmd->body,cmd->raw_len,&msg))
	{
		if (proto_parser::proto_type() == PROTO_JSON)
		{
			lua_pushstring(tolua_S, (const char*)msg->body);
		}
		else
		{
			push_proto_message_tolua((Message*)msg->body);
		}
	}
	
	return 1;
}
int lua_read_raw_header(lua_State* tolua_S)
{
	struct raw_cmd* cmd = (raw_cmd*)lua_touserdata(tolua_S, 1);
	if (cmd == NULL)
	{
		log_error("get raw_cmd error");
	}
	lua_pushinteger(tolua_S, cmd->stype);
	lua_pushinteger(tolua_S, cmd->ctype);
	lua_pushinteger(tolua_S, cmd->utag);
	return 3;
}
int lua_set_utag(lua_State* tolua_S)
{
	struct raw_cmd* cmd = (raw_cmd*)lua_touserdata(tolua_S, 1);
	if (cmd == NULL)
	{
		log_error("get rawcmd error");
	}
	int utag = lua_tointeger(tolua_S, 2);
	unsigned char* utag_header = cmd->body+4;
	utag_header[0] = (utag & 0x000000ff);
	utag_header[1] = ((utag & 0x0000ff00) >> 8);
	utag_header[2] = ((utag & 0x00ff0000) >> 16);
	utag_header[3] = ((utag & 0xff000000) >> 24);
	return 0;
}
int lua_get_sessionutag(lua_State* tolua_S)
{
	struct raw_cmd* cmd = (raw_cmd*)lua_touserdata(tolua_S, 1);
	if (cmd == NULL)
	{
		log_error("get rawcmd error");
	}
	unsigned char* ptr = cmd->body+4;
	int utag = ptr[0] | (ptr[1] << 8) | (ptr[2] << 16) | (ptr[3] << 24);
	lua_pushinteger(tolua_S, utag);
	return 1;
}

int Register_Proto_Parser_Module(lua_State* tolua_S)
{
	lua_getglobal(tolua_S, "_G");
	if (lua_istable(tolua_S, -1)) {
		tolua_open(tolua_S);
		tolua_module(tolua_S, "proto_parser", 0);
		tolua_beginmodule(tolua_S, "proto_parser");
		tolua_function(tolua_S, "init", lua_init);
		tolua_function(tolua_S, "proto_type", lua_proto_type);
		tolua_function(tolua_S, "register_pf_cmd_map", lua_register_pf_cmd_map);
		tolua_endmodule(tolua_S);
	}
	lua_pop(tolua_S, 1);
	return 0;
}
int Register_Raw_Msg_Module(lua_State* tolua_S)
{
	lua_getglobal(tolua_S, "_G");
	if (lua_istable(tolua_S, -1)) {
		tolua_open(tolua_S);
		tolua_module(tolua_S, "RawCmd", 0);
		tolua_beginmodule(tolua_S, "RawCmd");
		tolua_function(tolua_S, "readheader", lua_read_raw_header);
		tolua_function(tolua_S, "set_utag", lua_set_utag);
		tolua_function(tolua_S, "get_utag", lua_get_sessionutag);
		tolua_function(tolua_S, "get_body", lua_get_msg);
		tolua_endmodule(tolua_S);
	}
	lua_pop(tolua_S, 1);
	return 0;
}