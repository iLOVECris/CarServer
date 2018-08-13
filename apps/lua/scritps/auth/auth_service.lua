local cmd = require("Ctype")
local guest = require("auth/guest")
local user = require("auth/user")
local cmd_function_map = {}
function RegisterCmdFunc(cmd,func)
	cmd_function_map[cmd] = func
end

function RegisterCmdFunction()
	RegisterCmdFunc(cmd.eGuestLogin,guest.Login)
	RegisterCmdFunc(cmd.eModifyName,guest.ModifyName)
	RegisterCmdFunc(cmd.eModifyPlayerIcon,guest.ModifyFaceIcon)
	RegisterCmdFunc(cmd.eAccountUpgradeReq,guest.AccountUpgrade)
	RegisterCmdFunc(cmd.eUserLoginReq,user.userlogin)
	RegisterCmdFunc(cmd.eExitDisconnect,user.userdisconnect)
	RegisterCmdFunc(cmd.eUserLoginOutReq,user.userloginout)
end

RegisterCmdFunction()

function recv_cmd(s,msg)
	local ctype = msg[2]
	cmd_function_map[ctype](s,msg)
end

function session_disconnect(s,stype)
	print("gateway disconnect")
end

local auth_service = {
	on_session_recv_cmd = recv_cmd,
	on_session_disconnect = session_disconnect,
}

local auth = {
	stype = 1,
	service = auth_service,
}

return auth;