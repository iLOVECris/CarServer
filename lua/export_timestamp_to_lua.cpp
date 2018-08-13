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
#include "../../utils/timestamp.h"
#include "export_timestamp_to_lua.h"

int lua_today(lua_State* tolua_S)
{
	unsigned long today = timestamp_today();
	lua_pushinteger(tolua_S,today);
	return 1;
}

int lua_yesterday(lua_State* tolua_S)
{
	unsigned long yesterday = timestamp_yesterday();
	lua_pushinteger(tolua_S, yesterday);
	return 1;
}


int lua_timenow(lua_State* tolua_S)
{
	unsigned long time = timestamp();
	lua_pushinteger(tolua_S, time);
	return 1;
}

int lua_timeweeknow(lua_State* tolua_S)
{
	int week = timestamp2week();
	lua_pushinteger(tolua_S, week);
	return 1;
}

int lua_timeweeks(lua_State* tolua_S)
{
	int weeks = timestamp2weeks();
	lua_pushinteger(tolua_S, weeks);
	return 1;
}

int Register_TimeStamp_Module(lua_State* tolua_S)
{
	lua_getglobal(tolua_S, "_G");
	if (lua_istable(tolua_S, -1)) {
		tolua_open(tolua_S);
		tolua_module(tolua_S, "timestamp", 0);
		tolua_beginmodule(tolua_S, "timestamp");
		tolua_function(tolua_S, "today", lua_today);
		tolua_function(tolua_S, "yesterday", lua_yesterday);
		tolua_function(tolua_S, "timenow", lua_timenow);
		tolua_function(tolua_S, "timeweeknow", lua_timeweeknow);
		tolua_function(tolua_S, "timeweeks", lua_timeweeks);
		tolua_endmodule(tolua_S);
	}
	lua_pop(tolua_S, 1);
	return 0;
}