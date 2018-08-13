local stype = require("Stype")

local Servers = {}

Servers[stype.Auth] = 
{
	s_type = stype.Auth,
	ip = "127.0.0.1",
	port = 8006,
	name = "auth"
}
Servers[stype.TalkRoom] = 
{
	s_type = stype.TalkRoom,
	ip = "127.0.0.1",
	port = 8009,
	name = "talkroom"
}
Servers[stype.System] = 
{
	s_type = stype.System,
	ip = "127.0.0.1",
	port = 8008,
	name = "System"
}

local server_config = 
{
	gateway_tcp_ip = "127.0.0.1",
	gateway_tcp_port = 8001,
	gateway_ws_ip = "127.0.0.1",
	gateway_ws_port = 8002,
	gateway_udp_ip = "127.0.0.1",
	gateway_udp_port = 6666,
	servers = Servers,
	auth_db_config = 
	{
		ip = "127.0.0.1",
		port = 3306,
		db_name = "auth_center",
		user = "root",
		password = "wjj843048045",
	},
	auth_redis_config = 
	{
		ip = "127.0.0.1",
		port = 6379,
		redis_index = 1,
	},
	game_db_config = 
	{
		ip = "127.0.0.1",
		port = 3306,
		db_name = "game_center",
		user = "root",
		password = "wjj843048045",
	},
	game_redis_config = 
	{
		ip = "127.0.0.1",
		port = 6379,
		redis_index = 2,
	},
}
return server_config