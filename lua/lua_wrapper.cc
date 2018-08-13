#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<string>

#ifdef __cplusplus
extern "C"
{
#endif // !CPLUSPLUS

#include "tolua++.h"

#ifdef __cplusplus
}
#endif

#include "lua_wrapper.h"
#include "../utils/logger.h"
#include "export_mysql_to_lua.h"
#include "export_redis_to_lua.h"
#include "tolua_fix.h"
#include "export_service_to_lua.h"
#include "export_session_to_lua.h"
#include "export_timer_to_lua.h"
#include "export_netbus_to_lua.h"
#include "export_proto_parser_to_lua.h"
#include "export_timestamp_to_lua.h"
lua_State* g_lua_State = NULL;

void lua_wrapper::add_search_path(const std::string &s)
{
	char strPath[1024] = { 0 };
	sprintf(strPath, "local path = string.match([[%s]],[[(.*)/[^/]*$]])\n package.path = package.path .. [[;]] .. path .. [[/?.lua;]] .. path .. [[/?/init.lua]]\n", s.c_str());
	luaL_dostring(g_lua_State, strPath);
}

static void do_log_message(void(*log)(const char* file_name, int line_num, const char* msg), const char* msg)
{
	lua_Debug info;
	int depth = 0;
	while (lua_getstack(g_lua_State, depth, &info))
	{

		lua_getinfo(g_lua_State, "S", &info);
		lua_getinfo(g_lua_State, "n", &info);
		lua_getinfo(g_lua_State, "l", &info);

		if (info.source[0] == '@') 
		{
			log(&info.source[1], info.currentline, msg);
			return;
		}

		++depth;
	}
	if (depth == 0)
	{
		log("trunk", 0, msg);
	}
}
void print_err_func(const char* file_name, int line_num, const char* msg)
{
	logger::log(file_name, line_num, Error, msg);
}

void print_debug_func(const char* file_name, int line_num, const char* msg)
{
	logger::log(file_name, line_num, Debug, msg);
}

void print_warning_func(const char* file_name, int line_num, const char* msg)
{
	logger::log(file_name, line_num, Warning, msg);
}

int lua_log_err(lua_State *L)
{
	int nargs = lua_gettop(L);
	std::string t;
	for (int i = 1; i <= nargs; i++)
	{
		if (lua_istable(L, i))
			t += "table";
		else if (lua_isnone(L, i))
			t += "none";
		else if (lua_isnil(L, i))
			t += "nil";
		else if (lua_isboolean(L, i))
		{
			if (lua_toboolean(L, i) != 0)
				t += "true";
			else
				t += "false";
		}
		else if (lua_isfunction(L, i))
			t += "function";
		else if (lua_islightuserdata(L, i))
			t += "lightuserdata";
		else if (lua_isthread(L, i))
			t += "thread";
		else
		{
			const char * str = lua_tostring(L, i);
			if (str)
				t += lua_tostring(L, i);
			else
				t += lua_typename(L, lua_type(L, i));
		}
		if (i != nargs)
			t += "\t";
	}
	do_log_message(print_err_func, t.c_str());
	return 0;
}

int lua_log_warning(lua_State *L)
{
	int nargs = lua_gettop(L);
	std::string t;
	for (int i = 1; i <= nargs; i++)
	{
		if (lua_istable(L, i))
			t += "table";
		else if (lua_isnone(L, i))
			t += "none";
		else if (lua_isnil(L, i))
			t += "nil";
		else if (lua_isboolean(L, i))
		{
			if (lua_toboolean(L, i) != 0)
				t += "true";
			else
				t += "false";
		}
		else if (lua_isfunction(L, i))
			t += "function";
		else if (lua_islightuserdata(L, i))
			t += "lightuserdata";
		else if (lua_isthread(L, i))
			t += "thread";
		else
		{
			const char * str = lua_tostring(L, i);
			if (str)
				t += lua_tostring(L, i);
			else
				t += lua_typename(L, lua_type(L, i));
		}
		if (i != nargs)
			t += "\t";
	}
	do_log_message(print_warning_func, t.c_str());
	return 0;
}

int lua_log_debug(lua_State *L)
{
	int nargs = lua_gettop(L);
	std::string t;
	for (int i = 1; i <= nargs; i++)
	{
		if (lua_istable(L, i))
			t += "table";
		else if (lua_isnone(L, i))
			t += "none";
		else if (lua_isnil(L, i))
			t += "nil";
		else if (lua_isboolean(L, i))
		{
			if (lua_toboolean(L, i) != 0)
				t += "true";
			else
				t += "false";
		}
		else if (lua_isfunction(L, i))
			t += "function";
		else if (lua_islightuserdata(L, i))
			t += "lightuserdata";
		else if (lua_isthread(L, i))
			t += "thread";
		else
		{
			const char * str = lua_tostring(L, i);
			if (str)
				t += lua_tostring(L, i);
			else
				t += lua_typename(L, lua_type(L, i));
		}
		if (i != nargs)
			t += "\t";
	}
	do_log_message(print_debug_func, t.c_str());
	return 0;
}

lua_State* lua_wrapper::lua_state()
{
	return g_lua_State;
}

void lua_wrapper::RegisterFunc2Lua(const char* FuncName, int(*lua_func)(lua_State* L))
{
	lua_pushcfunction(g_lua_State, lua_func);
	lua_setglobal(g_lua_State, FuncName);

}

static int lua_panic(lua_State *L)
{
	const char* msg = luaL_checkstring(L, -1);
	if (msg)
	{
		do_log_message(print_err_func, msg);
	}
	return 0;
}

int logger_path_init(lua_State *tolua_S)
{
	const char* path = lua_tostring(tolua_S, 1);
	const char* prefix = lua_tostring(tolua_S, 2);
	bool std_output = lua_toboolean(tolua_S, 3);

	logger::init(path, prefix, std_output);
	return 0;
}

int lua_add_search_path(lua_State *tolua_S)
{
	const char* path = lua_tostring(tolua_S, 1);
	if (path)
	{
		std::string str_path = path;
		lua_wrapper::add_search_path(str_path);
	}
	return 0;
}

int Register_Log_Module(lua_State *tolua_S)
{
	lua_wrapper::RegisterFunc2Lua("print", lua_log_debug);
	lua_getglobal(tolua_S, "_G");
	if (lua_istable(tolua_S, -1)) {
		tolua_open(tolua_S);
		tolua_module(tolua_S, "Logger", 0);
		tolua_beginmodule(tolua_S, "Logger");

		tolua_function(tolua_S, "log_debug", lua_log_debug);
		tolua_function(tolua_S, "log_warning", lua_log_warning);
		tolua_function(tolua_S, "log_error", lua_log_err);
		tolua_function(tolua_S, "init", logger_path_init);
		//tolua_function(tolua_S, "add_search_path", lua_add_search_path);
		tolua_endmodule(tolua_S);
	}
	lua_pop(tolua_S, 1);
	return 0;
}

void lua_wrapper::init_lua()
{
	g_lua_State = luaL_newstate();
	lua_atpanic(g_lua_State, lua_panic);//·ÀÖ¹±ÀÀ£
	luaL_openlibs(g_lua_State);
	toluafix_open(g_lua_State);
	lua_wrapper::RegisterFunc2Lua("add_search_path", lua_add_search_path);
	Register_Log_Module(g_lua_State);
	Register_Netbus_Module(g_lua_State);
	Register_Proto_Parser_Module(g_lua_State);
	Register_Timer_Module(g_lua_State);
	Register_MySQL_Module(g_lua_State);
	Register_Redis_Module(g_lua_State);
	Register_Service_Module(g_lua_State);
	Register_Session_Module(g_lua_State);
	Register_Raw_Msg_Module(g_lua_State);
	Register_TimeStamp_Module(g_lua_State);
}
void lua_wrapper::exit()
{
	if (g_lua_State != NULL) {
		lua_close(g_lua_State);
		g_lua_State = NULL;
	}
}
bool lua_wrapper::Run_Lua_File(std::string& path)
{
	if (luaL_dofile(g_lua_State, path.c_str())) {
		lua_log_err(g_lua_State);//´òÓ¡´íÎóÈÕÖ¾
		return false;
	}
	return true;
}
static bool pushFunctionByHandler(int nHandler)
{
	toluafix_get_function_by_refid(g_lua_State, nHandler);                  /* L: ... func */
	if (!lua_isfunction(g_lua_State, -1))
	{
		log_error("[LUA ERROR] function refid '%d' does not reference a Lua function", nHandler);
		lua_pop(g_lua_State, 1);
		return false;
	}
	return true;
}

static int executeFunction(int numArgs)
{
	int functionIndex = -(numArgs + 1);
	if (!lua_isfunction(g_lua_State, functionIndex))
	{
		log_error("value at stack [%d] is not function", functionIndex);
		lua_pop(g_lua_State, numArgs + 1); // remove function and arguments
		return 0;
	}

	int traceback = 0;
	lua_getglobal(g_lua_State, "__G__TRACKBACK__");
	if (!lua_isfunction(g_lua_State, -1))
	{
		lua_pop(g_lua_State, 1);
	}
	else
	{
		lua_insert(g_lua_State, functionIndex - 1);
		traceback = functionIndex - 1;
	}

	int error = 0;
	error = lua_pcall(g_lua_State, numArgs, 1, traceback);
	if (error)
	{
		if (traceback == 0)
		{
			log_error("[LUA ERROR] %s", lua_tostring(g_lua_State, -1));
			lua_pop(g_lua_State, 1); // remove error message from stack
		}
		else
		{
			lua_pop(g_lua_State, 2);
		}
		return 0;
	}

	// get return value
	int ret = 0;
	if (lua_isnumber(g_lua_State, -1))
	{
		ret = (int)lua_tointeger(g_lua_State, -1);
	}
	else if (lua_isboolean(g_lua_State, -1))
	{
		ret = (int)lua_toboolean(g_lua_State, -1);
	}
	// remove return value from stack
	lua_pop(g_lua_State, 1);

	if (traceback)
	{
		lua_pop(g_lua_State, 1);
	}

	return ret;
}

int lua_wrapper::execute_script_handler(int nHandler, int numArgs) {
	int ret = 0;
	if (pushFunctionByHandler(nHandler))   
	{
		if (numArgs > 0)
		{
			lua_insert(g_lua_State, -(numArgs + 1));
		}
		ret = executeFunction(numArgs);
	}
	lua_settop(g_lua_State, 0);
	return ret;
}

void lua_wrapper::remove_script_handler(int nHandler)
{
	toluafix_remove_function_by_refid(g_lua_State, nHandler);
}
