#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<iostream>
using namespace std;
#include "uv.h"
#include "../../netbus/netbus.h"
//#include "pf_cmd_map.h"
#include "../../utils/logger.h"
#include "../../utils/time_list.h"
#include "../../utils/timestamp.h"
#include "../../datebase/db_wrapper.h"
#include "../../datebase/redis_wrapper.h"
#include "../../lua/lua_wrapper.h"
void connect_sercer(int err, session* s, void* udata)
{
	if (err == 1)
	{
		printf("connet server error");
	}
}

int main(int argc, char** argv)
{
	netbus* _netbus = netbus::instacne();
	_netbus->init_cache();
	//_netbus->Tcp_Connect("127.0.0.1", 8001, connect_sercer, NULL);
	lua_wrapper::init_lua();
	if (argc != 3)
	{
		string path = "../../apps/lua/scritps/";
		lua_wrapper::add_search_path(path);
		lua_wrapper::Run_Lua_File(path+"gateway/main.lua");
	}
	else
	{
		string path = argv[1];
		if (*(path.end() - 1) != '/')
		{
			path += '/';
		}
		lua_wrapper::add_search_path(path);
		lua_wrapper::Run_Lua_File(path+argv[2]);
	}

	_netbus->run();
	lua_wrapper::exit(); 

	system("pause");
	return 0;
}