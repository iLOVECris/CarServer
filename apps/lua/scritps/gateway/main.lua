Logger.init("logger/gateway/", "gateway", true)
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

netbus.start_udpsocket_server(server_config.gateway_udp_port)
netbus.start_tcp_server(server_config.gateway_tcp_port)
netbus.start_websocket_server(server_config.gateway_ws_port)
--netbus.tcp_connect("127.0.0.1",6601,TcpConnected)
--end


print("start gateway service success !!!!")

local gateway_server = require("gateway/gateway_service")
for k,v in pairs(server_config.servers) do
	service.register_raw(k,gateway_server)
end





