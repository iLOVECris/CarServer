local playerdata_module = require("system/get_player_data")
local cmd = require("Ctype")
local cmd_function_map = {}
function RegisterCmdFunc(cmd,func)
	cmd_function_map[cmd] = func
end

function disconnect(s,msg)
	print("disconnect"..msg[3])
end

function RegisterCmdFunction()
	RegisterCmdFunc(cmd.eGetPlayerDataReq,playerdata_module.getplayerdata)
	RegisterCmdFunc(cmd.eExitDisconnect,disconnect)
	RegisterCmdFunc(cmd.eGetLoginBonusReq,playerdata_module.getplayerloginbonus)
end

RegisterCmdFunction()

function recv_cmd(s,msg)
	local ctype = msg[2]
	cmd_function_map[ctype](s,msg)
end

function session_disconnect(s,stype)
	print("gateway disconnect")
end

local system_service = {
	on_session_recv_cmd = recv_cmd,
	on_session_disconnect = session_disconnect,
}

local system = {
	stype = 3,
	service = system_service,
}

return system;