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

#include "tolua_fix.h"

#include "../datebase/db_wrapper.h"
#include "lua_wrapper.h"
#include "export_mysql_to_lua.h"
#include "../../utils//logger.h"

void on_open_cb(const char* err, void* context, void* udata)
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


int lua_mysql_connect(lua_State* tolua_S)
{
	char* ip = (char*)tolua_tostring(tolua_S, 1, 0);
	int port = (int)tolua_tonumber(tolua_S, 2, 0);
	char* db_name = (char*)tolua_tostring(tolua_S, 3, 0);
	char* uname = (char*)tolua_tostring(tolua_S, 4, 0);
	char* pwd = (char*)tolua_tostring(tolua_S, 5, 0);

	int handler = toluafix_ref_function(tolua_S, 6, 0);
	mysql_wrapper::connect(ip, port, db_name, uname, pwd, on_open_cb, (void*)handler);
	return 0;
}
int lua_mysql_close(lua_State* tolua_S)
{
	void* context = tolua_touserdata(tolua_S, 1, 0);
	if (context) {
		mysql_wrapper::close(context);
	}
	return 0;
}

void push_mysql_row(MYSQL_ROW row, int num)
{
	lua_newtable(lua_wrapper::lua_state());
	int index = 1;
	for (int i = 0; i < num; i++) {
		if (row[i] == NULL) {
			lua_pushnil(lua_wrapper::lua_state());
		}
		else {
			lua_pushstring(lua_wrapper::lua_state(), row[i]);
		}

		lua_rawseti(lua_wrapper::lua_state(), -2, index);
		++index;
	}
}

/*
key = ""
function PrintTable(table, level)
level = level or 1
local indent = ""
for i = 1, level do
indent = indent.."  "
end

if key ~= "" then
print(indent..key.." ".."=".." ".."{")
else
print(indent .. "{")
end

key = ""
for k, v in pairs(table) do
if type(v) == "table" then
key = k
PrintTable(v, level + 1)
else
local content = string.format("%s%s = %s", indent .. "  ", tostring(k), tostring(v))
print(content)
end
end
print(indent .. "}")

end
*/

void on_lua_query_cb(const char* err, MYSQL_RES* result, void* udata)
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
		{ // 把查询得到的结果push成一个表; { {}, {}, {}, ...}
			lua_newtable(lua_wrapper::lua_state());
			int index = 1;
			int num = mysql_num_fields(result);
			MYSQL_ROW row;
			while (row = mysql_fetch_row(result)) 
			{
				push_mysql_row(row, num);
				lua_rawseti(lua_wrapper::lua_state(), -2, index);
				++index;
			}
		}
		else
		{
			lua_pushnil(lua_wrapper::lua_state());
		}
	}

	lua_wrapper::execute_script_handler((int)udata, 2);
	lua_wrapper::remove_script_handler((int)udata);
}

int lua_mysql_query(lua_State* tolua_S)
{
	void* context = tolua_touserdata(tolua_S, 1, 0);
	if (!context) {
		log_debug("context is null");
	}
	char* sql = (char*)tolua_tostring(tolua_S, 2, 0);
	if (sql == NULL) {
		log_debug("sql is null string");
	}
	int handler = toluafix_ref_function(tolua_S, 3, 0);
	if (handler == 0) {
		log_debug("func handler is null");
	}
	mysql_wrapper::query(context, sql, on_lua_query_cb, (void*)handler);

	return 0;
}

int Register_MySQL_Module(lua_State* tolua_S)
{
	lua_getglobal(tolua_S, "_G");
	if (lua_istable(tolua_S, -1)) {
		tolua_open(tolua_S);
		tolua_module(tolua_S, "mysql_wrapper", 0);
		tolua_beginmodule(tolua_S, "mysql_wrapper");

		tolua_function(tolua_S, "connect", lua_mysql_connect);
		tolua_function(tolua_S, "close", lua_mysql_close);
		tolua_function(tolua_S, "query", lua_mysql_query);
		tolua_endmodule(tolua_S);
	}
	lua_pop(tolua_S, 1);
	return 0;
}