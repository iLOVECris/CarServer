Logger.init("logger/auth/", "auth", true)
--end
require("DataBase/auth_center_db")
require("DataBase/auth_center_redis")--需要在主函数中加载模块，否则会注册服务失败
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

netbus.start_tcp_server(server_config.servers[stype.Auth].port)

--end


print("start auth service success !!!!   "..server_config.servers[stype.Auth].port)
--local echo_server = require("echo_service")
--service.register(echo_server.stype,echo_server.service)
local auth_server = require("auth/auth_service")
service.register(auth_server.stype,auth_server.service)
--local timer = timer.schedule_once(function ()
  --print("scheduler.schedule called")
--end, 1000)





