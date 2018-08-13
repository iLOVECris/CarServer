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
#include "export_netbus_to_lua.h"
#include "../../netbus/netbus.h"


int lua_start_tcp_server(lua_State* tolua_S)
{
	int argc = lua_gettop(tolua_S);
	if (argc != 1) 
	{
		log_error("param count error");
		return 0;
	}
	int port = luaL_checkinteger(tolua_S,1);
	netbus::instacne()->Start_Tcp_Server(port);
	return 0;
}
void tcp_connected(int err, session* s, void* udata)
{
	if (err)//connect error
	{
		lua_pushinteger(lua_wrapper::lua_state(), err);
		lua_pushnil(lua_wrapper::lua_state());
	}
	else
	{
		lua_pushinteger(lua_wrapper::lua_state(), err);
		tolua_pushuserdata(lua_wrapper::lua_state(), s);
	}

	lua_wrapper::execute_script_handler((int)udata, 2);
	lua_wrapper::remove_script_handler((int)udata); 
}
int lua_tcp_connect(lua_State* tolua_S)
{
	const char* ip = lua_tostring(tolua_S, 1);
	int port = lua_tointeger(tolua_S, 2);

	int Connected_handler = toluafix_ref_function(tolua_S, 3, 0);
	if (Connected_handler == 0) {
		log_debug("func handler is null");
	}
	netbus::instacne()->Tcp_Connect(ip, port, tcp_connected, (void*)Connected_handler);
	return 0;
}
int lua_start_websocket_server(lua_State* tolua_S)
{
	int argc = lua_gettop(tolua_S);
	if (argc != 1)
	{
		log_error("param count error");
		return 0;
	}
	int port = luaL_checkinteger(tolua_S, 1);
	netbus::instacne()->Start_WebSocket_Server(port);
	return 0;
}

int lua_start_udpsocket_server(lua_State* tolua_S)
{
	int argc = lua_gettop(tolua_S);
	if (argc != 1)
	{
		log_error("param count error");
		return 0;
	}
	int port = luaL_checkinteger(tolua_S, 1);
	netbus::instacne()->Start_UdpSocket_Server(port);
	return 0;
}

int Register_Netbus_Module(lua_State* tolua_S)
{
	lua_getglobal(tolua_S, "_G");
	if (lua_istable(tolua_S, -1)) {
		tolua_open(tolua_S);
		tolua_module(tolua_S, "netbus", 0);
		tolua_beginmodule(tolua_S, "netbus");
		tolua_function(tolua_S, "start_tcp_server", lua_start_tcp_server);
		tolua_function(tolua_S, "start_websocket_server", lua_start_websocket_server);
		tolua_function(tolua_S, "start_udpsocket_server", lua_start_udpsocket_server);
		tolua_function(tolua_S, "tcp_connect", lua_tcp_connect);
		tolua_endmodule(tolua_S);
	}
	lua_pop(tolua_S, 1);
	return 0;
}