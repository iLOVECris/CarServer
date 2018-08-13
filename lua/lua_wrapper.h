#ifndef __LUA_WRAPPER_H__
#define __LUA_WRAPPER_H__
#include "lua.hpp"
#include <string>
class lua_wrapper
{

public:
	static void init_lua();
	static void exit();
	static bool Run_Lua_File(std::string& path);
	static void RegisterFunc2Lua(const char* FuncName, int(*lua_func)(lua_State* L));
	static lua_State* lua_state();
	static void add_search_path(const std::string &s);
public:
	static int execute_script_handler(int nHandler, int numArgs);
	static void remove_script_handler(int nHandler);
};



#endif