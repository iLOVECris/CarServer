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
#include "../../utils/time_list.h"
#include "export_timer_to_lua.h"


struct timer_repeat {
	int handler;
	int repeat_count;
};

void on_lua_repeat_timer(void* udata)
{
	timer_repeat* tr = (timer_repeat*)udata;

	lua_wrapper::execute_script_handler(tr->handler, 0);
	if (tr->repeat_count == -1)
	{
		return;
	}
	tr->repeat_count--;
	if (tr->repeat_count <= 0)
	{
		if (tr->handler != 0)
		{
			lua_wrapper::remove_script_handler(tr->handler);
		}
		free(tr);
	}
}

int lua_schedule_once(lua_State* tolua_S)
{
	int handler = toluafix_ref_function(tolua_S, 1, 0);
	if (handler == 0)
	{
		lua_pushnil(tolua_S);
		return 1;
	}

	int after_msec = lua_tointeger(tolua_S, 2, 0);
	if (after_msec <= 0) 
	{
		if (handler != 0) 
		{
			lua_wrapper::remove_script_handler(handler);
		}
		lua_pushnil(tolua_S);
		return 1;
	}

	timer_repeat* tr = (timer_repeat*)malloc(sizeof(timer_repeat));
	tr->handler = handler;
	tr->repeat_count = 1;
	struct timer* t = schedule_once(on_lua_repeat_timer, (void*)tr, after_msec);
	tolua_pushuserdata(tolua_S, t);
	return 1;

}
int lua_cancel_timer(lua_State* tolua_S)
{
	timer* t = (timer*)tolua_touserdata(tolua_S, 1,0);
	if (t == NULL)
	{
		log_error("get timer handler is null");
	}
	struct timer_repeat* tr = (struct timer_repeat*)get_timer_udata(t);
	lua_wrapper::remove_script_handler(tr->handler);
	free(tr);
	cancel_timer(t);
	return 0;
}
int lua_schedule_repeat(lua_State* tolua_S)
{
	int handler = toluafix_ref_function(tolua_S, 1, 0);
	if (handler == 0)
	{
		lua_pushnil(tolua_S);
		return 1;
	}
	int after_msec = lua_tointeger(tolua_S, 2);
	if (after_msec <= 0)
	{
		if (handler != 0)
		{
			lua_wrapper::remove_script_handler(handler);
		}
		lua_pushnil(tolua_S);
		return 1;
	}
	int repeat_count = lua_tointeger(tolua_S, 3);
	int repeat_msec = lua_tointeger(tolua_S, 4);
	if (repeat_msec <= 0)
	{
		if (handler != 0)
		{
			lua_wrapper::remove_script_handler(handler);
		}
		lua_pushnil(tolua_S);
		return 1;
	}

	timer_repeat* timer = (timer_repeat*)malloc(sizeof(timer_repeat));
	timer->handler = handler;
	timer->repeat_count = repeat_count;
	struct timer* t = schedule_repeat(on_lua_repeat_timer, (void*)timer, after_msec, repeat_count, repeat_msec);
	tolua_pushuserdata(tolua_S,t);
	return 1;
}
int Register_Timer_Module(lua_State* tolua_S)
{
	lua_getglobal(tolua_S, "_G");
	if (lua_istable(tolua_S, -1)) {
		tolua_open(tolua_S);
		tolua_module(tolua_S, "timer", 0);
		tolua_beginmodule(tolua_S, "timer");
		tolua_function(tolua_S, "schedule_once", lua_schedule_once);
		tolua_function(tolua_S, "cancel_timer", lua_cancel_timer);
		tolua_function(tolua_S, "schedule_repeat", lua_schedule_repeat);
		tolua_endmodule(tolua_S);
	}
	lua_pop(tolua_S, 1);
	return 0;
}