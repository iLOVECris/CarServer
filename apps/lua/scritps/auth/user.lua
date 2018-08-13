local auth_center_db = require("DataBase/auth_center_db")
local auth_center_redis  = require("DataBase/auth_center_redis")
local stype = require("Stype")
local ctype = require("Ctype")
local ErrCode = require("ErrCode")
function userloginout(s,msg)
	print("loginout"..msg[3])
	local utag = msg[3]
	local msg = {stype.Auth,ctype.eUserLoginOutRes,utag,{status=0}}
	session.send_cmd_msg(s,msg)
end

function userdisconnect(s,msg)
	print("disconnect"..msg[3])
end
function userlogin(s,msg)
	local body = msg[4]
 	local utag = msg[3]
 	local playeraccount = body.playeraccount
 	local password = body.password
 	if #playeraccount==0 or #password~=32 then --账户数据格式错误
 		local msg = {errcode = ErrCode.PlayerAccountParamErr}
		local send_msg = {stype.Auth,ctype.eUserLoginRes,utag,msg}
		session.send_cmd_msg(s,send_msg)
		return
 	end
 	auth_center_db.get_uinfo_byactpsd(playeraccount,password,function(err,result)
 		if err~=nil then--查询错误，发送错误码
			local msg = {errcode = ErrCode.QueryErr}
			local send_msg = {stype.Auth,ctype.eUserLoginRes,utag,msg}
			session.send_cmd_msg(s,send_msg)
			return
 		end
 		if result==nil then
 			local msg = {errcode = ErrCode.PlayerAccountNoExist}
			local send_msg = {stype.Auth,ctype.eUserLoginRes,utag,msg}
			session.send_cmd_msg(s,send_msg)
			return
 		end
		local ustatus = tonumber(result[6])
		local u_is_guest = tonumber(result[5])
		local uface = tonumber(result[4])
		local usex = tonumber(result[3])
		local uname = result[2]
		local uid = tonumber(result[1])
 		local uinfo = result[1]
		if ustatus ~=0 then--0表示正常状态 1表示封禁
			local msg = {errcode = ErrCode.Freeze}
			local send_msg = {stype.Auth,ctype.eUserLoginRes,utag,msg}
			session.send_cmd_msg(s,send_msg)
			return
		end
		local uinfo = {uid = uid,name = uname,sex = usex,face = uface,is_guest = u_is_guest,status = ustatus}
		local msg = {errcode = 0,info = uinfo}
		local send_msg = {stype.Auth,ctype.eUserLoginRes,utag,msg}
		auth_center_redis.insert_uinfo(uid,uinfo)
		session.send_cmd_msg(s,send_msg)
 	end)

end

local user = 
{
	userlogin = userlogin,
	userdisconnect = userdisconnect,
	userloginout = userloginout,
}
return user