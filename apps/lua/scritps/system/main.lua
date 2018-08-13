Logger.init("logger/system/", "system", true)
local game_center_db = require("DataBase/game_center_db")
local game_center_redis = require("DataBase/game_center_redis")

--end
-- 初始化协议模块
local proto_type = {
    PROTO_JSON = 0,
    PROTO_BUF = 1,
}

-- 如果是protobuf协议，还要注册一下映射表

if proto_parser.proto_type() == proto_type.PROTO_BUF then 
  local cmd_name_map = require("cmd_name_map")
  if cmd_name_map then 
    proto_parser.register_pf_cmd_map(cmd_name_map)
  end
end
--end
-- 开启网络服务
local server_config = require("server_config")
local stype = require("Stype")

netbus.start_tcp_server(server_config.servers[stype.System].port)

--end

print("start system service success !!!!   "..server_config.servers[stype.System].port)

local system_server = require("system/system_service")
service.register(system_server.stype,system_server.service)






