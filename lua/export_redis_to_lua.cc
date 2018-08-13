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
#include "../datebase/redis_wrapper.h"
#include "../../utils//logger.h"
#include "tolua_fix.h"
#include "export_redis_to_lua.h"


void on_open_redis_cb(const char* err, void* context, void* udata)
{
	if (err) {
		lua_pushstring(lua_wrapper::lua_state(), err);
		lua_pushnil(lua_wrapper::lua_state());
	}
	else {
		lua_pushnil(lua_wrapper::lua_state());
		tolua_pushuserdata(lua_wrapper::lua_state(), context);
	}

	lua_wrapper::execute_script_handler((int)udata, 2);
	lua_wrapper::remove_script_handler((int)udata);
}

int lua_redis_connect(lua_State* tolua_S)
{
	char* ip = (char*)tolua_tostring(tolua_S, 1, 0);
	int port = (int)tolua_tonumber(tolua_S, 2, 0);
	
	int handler = toluafix_ref_function(tolua_S, 3, 0);
	redis_wrapper::connect(ip, port, on_open_redis_cb, (void*)handler);
	return 0;
}
int lua_redis_close(lua_State* tolua_S)
{
	void* context = tolua_touserdata(tolua_S, 1, 0);
	if (context) {
		redis_wrapper::CloseRedisConnect(context);
	}
	return 0;
}

void push_result_to_lua(redisReply* result)
{
	switch (result->type) {
	case REDIS_REPLY_STRING:
	case REDIS_REPLY_STATUS:
		lua_pushstring(lua_wrapper::lua_state(), result->str);
		break;
	case REDIS_REPLY_INTEGER:
		lua_pushinteger(lua_wrapper::lua_state(), result->integer);
		break;
	case REDIS_REPLY_NIL:
		lua_pushnil(lua_wrapper::lua_state());
		break;
	case REDIS_REPLY_ARRAY:
		lua_newtable(lua_wrapper::lua_state());
		int index = 1;
		for (int i = 0; i < result->elements; i++) {
			push_result_to_lua(result->element[i]);
			lua_rawseti(lua_wrapper::lua_state(), -2, index);          /* table[index] = value, L: table */
			++index;
		}
		break;
	}
	return;
}

void on_lua_query_cb(const char* err, redisReply* result, void* udata)
{
	if (err)
	{
		lua_pushstring(lua_wrapper::lua_state(), err);
		lua_pushnil(lua_wrapper::lua_state());
	}
	else
	{
		lua_pushnil(lua_wrapper::lua_state());
		if (result)
		{ 		
			push_result_to_lua(result);
		}
		else
		{
			lua_pushnil(lua_wrapper::lua_state());
		}
	}

	lua_wrapper::execute_script_handler((int)udata, 2);
	lua_wrapper::remove_script_handler((int)udata);
}

int lua_redis_query(lua_State* tolua_S)
{
	void* context = tolua_touserdata(tolua_S, 1, 0);
	if (!context) {
		log_debug("context is null");
	}
	char* cmd = (char*)tolua_tostring(tolua_S, 2, 0);
	if (cmd == NULL) {
		log_debug("sql is null string");
	}
	int handler = toluafix_ref_function(tolua_S, 3, 0);
	if (handler == 0) {
		log_debug("func handler is null");
	}

	redis_wrapper::query(context, cmd, on_lua_query_cb, (void*)handler);

	return 0;
}

int Register_Redis_Module(lua_State* tolua_S)
{
	lua_getglobal(tolua_S, "_G");
	if (lua_istable(tolua_S, -1)) {
		tolua_open(tolua_S);
		tolua_module(tolua_S, "redis_wrapper", 0);
		tolua_beginmodule(tolua_S, "redis_wrapper");

		tolua_function(tolua_S, "connect", lua_redis_connect);
		tolua_function(tolua_S, "CloseRedisConnect", lua_redis_close);
		tolua_function(tolua_S, "query", lua_redis_query);
		tolua_endmodule(tolua_S);
	}
	lua_pop(tolua_S, 1);
	return 0;
}