local server_config = require("server_config")
local errcode = require("ErrCode")
local auth_db_config = server_config.auth_db_config
local MySql_Handler
function Conncet_db()
	mysql_wrapper.connect(auth_db_config.ip,auth_db_config.port,auth_db_config.db_name,auth_db_config.user,auth_db_config.password,
		function(err,handler)
			if err~=nil then 
				print("connect db falied")
				timer.schedule_once(Conncet_db,2000)
				return
			end
			MySql_Handler = handler
			print("connect db success")
		end)	
end
Conncet_db()
 
function getuinfo(ukey,callback)
	local sql = "select uid, name, sex, uface, is_guest, status from player_info where guest_key = \"%s\" limit 1"
	local cmd = string.format(sql,ukey)
	mysql_wrapper.query(MySql_Handler,cmd,function(err,result)
		if err~=nil then
			callback("query sql err",nil)
			return
		end
		if result==nil then
			callback(nil,nil)
			return
		end
		local ret = result[1]

		callback(nil,ret)
	end)
end

function insert_uinfo(ukey,callback)
	local sql = "insert into player_info(`guest_key`, `name`, `uface`, `sex`, `is_guest`)values(\"%s\", \"%s\", %d, %d, 1)"
	math.randomseed(os.time()) 
	local name = "g" .. math.random(100000, 999999)
	local uface = math.random(1, 6)
	local sex = math.random(0, 1)
	local cmd = string.format(sql,ukey,name,uface,sex)
	mysql_wrapper.query(MySql_Handler,cmd,function(err,result)
		if err~=nil then
			callback("query sql err",nil)
			return
		end
		callback(nil,nil)
	end)
end 
function modifyuinfo(uid,name,callback)
	local sql = "update player_info set name = \"%s\" where uid = %d"
	local cmd = string.format(sql,name,uid)
	mysql_wrapper.query(MySql_Handler,cmd,function(err,result)
		if err~=nil then
			callback("query sql err",nil)
			return
		end
		callback(nil,nil);
	end)

end
function modify_faceicon(uid,faceicon,callback)
	local sql = "update player_info set uface = %d where uid = %d"
	local cmd = string.format(sql,faceicon,uid)
	mysql_wrapper.query(MySql_Handler,cmd,function(err,result)
		if err~=nil then
			callback("query sql err",nil)
			return
		end
		callback(nil,nil);
	end)
end
function getuinfobyuid(uid,callback)
	local sql = "select is_guest from player_info where uid = %d limit 1"
	local cmd = string.format(sql,uid)
	mysql_wrapper.query(MySql_Handler,cmd,function(err,result)
		if err~=nil then
			callback("query sql err",nil)
			return
		end
		if result==nil or #result<=0 then
			callback(nil,nil)
			return
		end
		local ret = result[1]
		callback(nil,ret)
	end)
end
function upgradeaccount(uid,phonenumber,password,callback)
	local sql = "update player_info set is_guest = %d,phone = \"%s\",pwd = \"%s\" where uid = %d"
	local cmd = string.format(sql,0,phonenumber,password,uid)
	mysql_wrapper.query(MySql_Handler,cmd,function(err,result)
		if err~=nil then
			callback("query sql err",nil)
			return
		end
		callback(nil,nil);
	end)
end
function get_uinfo_by_account_password(playeraccount,password,callback)
	local sql = "select uid, name, sex, uface, is_guest,status from player_info where phone = \"%s\" and pwd = \"%s\" limit 1"
	local cmd = string.format(sql,playeraccount,password)
	mysql_wrapper.query(MySql_Handler,cmd,function(err,result)
		if err~=nil then
			callback("query sql err",nil)
			return
		end
		if result==nil then
			callback(nil,nil)
			return
		end
		local ret = result[1]

		callback(nil,ret)
	end)
end

local get_guest_uinfo =
{
	getuinfo = getuinfo,
	insert_uinfo = insert_uinfo,
	modify_uinfo = modifyuinfo,
	modify_faceicon = modify_faceicon,
	getuinfobyuid = getuinfobyuid,
	upgradeaccount = upgradeaccount,
	get_uinfo_byactpsd = get_uinfo_by_account_password,
}

return get_guest_uinfo