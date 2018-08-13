Logger.init("logger/talkroom/", "talkroom", true)
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
local server_config = require("server_config")
local stype = require("Stype")
-- 开启网络服务
--netbus.start_udpsocket_server(6666)
netbus.start_tcp_server(server_config.servers[stype.TalkRoom].port)
--netbus.start_websocket_server(8002)

--end


print("start talkroom service success !!!!   "..server_config.servers[stype.TalkRoom].port)
--local echo_server = require("echo_service")
--service.register(echo_server.stype,echo_server.service)
local tr_server = require("talkroom/talkroom_service")
service.register(tr_server.stype,tr_server.service)
--local timer = timer.schedule_once(function ()
  --print("scheduler.schedule called")
--end, 1000)





