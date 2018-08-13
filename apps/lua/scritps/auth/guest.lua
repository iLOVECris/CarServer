local auth_center_db = require("DataBase/auth_center_db")
local auth_center_redis = require("DataBase/auth_center_redis")
local stype = require("Stype")
local ctype = require("Ctype")
local ErrCode = require("ErrCode")
function Login(s,msg)
 	local body = msg[4]
 	local utag = msg[3]
 	local u_key = body.u_key
 	if u_key ~=nil then
 		auth_center_db.getuinfo(u_key,function(err,result)
 			if err~=nil then--查询错误，发送错误码
 				local msg = {errcode = ErrCode.QueryErr}
 				local send_msg = {stype.Auth,ctype.eGuestLoginRes,utag,msg}
 				session.send_cmd_msg(s,send_msg)
 				return
 			end
			if result==nil then --查询结果为空，插入
				auth_center_db.insert_uinfo(u_key,function(err,result)
					if err~=nil then
						local msg = {errcode = ErrCode.QueryErr}
						local send_msg = {stype.Auth,ctype.eGuestLoginRes,utag,msg}
						session.send_cmd_msg(s,send_msg)
						return
					end
					Login(s,msg)
					end)
			return
			end
			local ustatus = tonumber(result[6])
			local u_is_guest = tonumber(result[5])
			local uface = tonumber(result[4])
			local usex = tonumber(result[3])
			local uname = result[2]
			local uid = tonumber(result[1])
			if ustatus ~=0 then--0表示正常状态 1表示封禁
				local msg = {errcode = ErrCode.Freeze}
				local send_msg = {stype.Auth,ctype.eGuestLoginRes,utag,msg}
				session.send_cmd_msg(s,send_msg)
				return
			end
			if u_is_guest ~=1 then --玩家已经不是游客
				local msg = {errcode = ErrCode.NotGuest}
				local send_msg = {stype.Auth,ctype.eGuestLoginRes,utag,msg}
				session.send_cmd_msg(s,send_msg)
				return
			end
			local uinfo = {uid = uid,name = uname,sex = usex,face = uface,is_guest = u_is_guest,status = ustatus}
			local msg = {errcode = 0,info = uinfo}
			local send_msg = {stype.Auth,ctype.eGuestLoginRes,utag,msg}
			auth_center_redis.insert_uinfo(uid,uinfo)
			session.send_cmd_msg(s,send_msg)	
 		end)
 		
 	end
end
function ModifyName(s,msg)
	local uid=  msg[3]
	local body = msg[4]
	local name = body.name
	auth_center_db.modify_uinfo(uid,name,function(err,result)
		if err~=nil then
			local body = {status = ErrCode.ModifyNameErr}
			local msg = {stype.Auth,ctype.eRespose,uid,body}
			session.send_cmd_msg(s,msg)
			return
		end
		local body = {status = ErrCode.ModifyNameOk}
			local msg = {stype.Auth,ctype.eRespose,uid,body}
			session.send_cmd_msg(s,msg)
	end)
	auth_center_redis.modify_uinfo(uid,nil,name)
end

function ModifyFaceIcon(s,msg)	
	local uid=  msg[3]
	local body = msg[4]
	local faceicon = body.playericon
	auth_center_db.modify_faceicon(uid,faceicon,function(err,result)
		if err~=nil then
			local body = {status = ErrCode.ModifyFaceIconErr}
			local msg = {stype.Auth,ctype.eRespose,uid,body}
			session.send_cmd_msg(s,msg)
			return
		end
		local body = {status = ErrCode.ModifyFaceIconOK}
			local msg = {stype.Auth,ctype.eRespose,uid,body}
			session.send_cmd_msg(s,msg)
	end)
	auth_center_redis.modify_uinfo(uid,faceicon,nil)
end

function AccountUpgrade(s,msg)
	local uid=  msg[3]
	local body = msg[4]
	local phonenumber = body.phonenumber
	local password = body.password
	auth_center_db.getuinfobyuid(uid,function(err,ret)
		if err~=nil or ret==nil then
			local body = {status = ErrCode.QueryErr}
			local msg = {stype.Auth,ctype.eAccountUpgradeRes,uid,body}
			session.send_cmd_msg(s,msg)
			return
		end
		if ret~=nil then 
			local is_guest= tonumber(ret[1]);
			if is_guest~=1 then --已经不是游客了
				local body = {status = ErrCode.NotGuest}
				local msg = {stype.Auth,ctype.eAccountUpgradeRes,uid,body}
				session.send_cmd_msg(s,msg)
				return
			end
			auth_center_db.upgradeaccount(uid,phonenumber,password,function(err,ret)
				if err~=nil then
					local body = {status = ErrCode.UpgradeErr}
					local msg = {stype.Auth,ctype.eAccountUpgradeRes,uid,body}
					session.send_cmd_msg(s,msg)
					return
				end
				local body = {status = 0,playeraccount = phonenumber,password = password}
				local msg = {stype.Auth,ctype.eAccountUpgradeRes,uid,body}--升级账号成功
				session.send_cmd_msg(s,msg)
			end)
		end
	end)
end

local guest = {
	Login = Login,
	ModifyName = ModifyName,
	ModifyFaceIcon = ModifyFaceIcon,
	AccountUpgrade = AccountUpgrade, 
}

return guest