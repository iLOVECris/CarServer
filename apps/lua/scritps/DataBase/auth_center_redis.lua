local server_config = require("server_config")
local errcode = require("ErrCode")
local auth_redis_config = server_config.auth_redis_config
local Redis_Handler = nil
function Conncet_redis()
	redis_wrapper.connect(auth_redis_config.ip,auth_redis_config.port,
		function(err,handler)
			if err~=nil then 
				print("connect auth center redis falied")
				timer.schedule_once(Conncet_redis,2000)
				return
			end
			Redis_Handler = handler
			print("connect auth center redis success")
			local cmd = "select "..auth_redis_config.redis_index
			redis_wrapper.query(Redis_Handler,cmd,function(err,result)
				if err==nil then 
					print("select auth center redis db success")
				else
					print("select auth center redis error")
				end		
			end)
		end)	
end
Conncet_redis()

function getuinfo(uid,callback)
	local cmd = "hgetall auth_center_playerinfo_"..uid
	redis_wrapper.query(Redis_Handler,cmd,function(err,result)
		if err~=nil then
			callback("query sql err",nil)
			return
		end
		local uinfo = {}
		uinfo.name = result[2]
		uinfo.sex = tonumber(result[4])
		uinfo.face = tonumber(result[6])
		uinfo.is_guest = tonumber(result[8])
		uinfo.status = tonumber(result[10])
		callback(nil,uinfo)
	end)
end

function insert_uinfo(uid,uinfo)
	local tabel_name = "auth_center_playerinfo_"..uid
	local cmd = "hmset "..tabel_name..
		" name "..uinfo.name..
		" sex "..uinfo.sex.. 
		" face "..uinfo.face..
		" is_guest "..uinfo.is_guest..
		" status "..uinfo.status
	redis_wrapper.query(Redis_Handler,cmd,function(err,result)
		if err~=nil then
			print("query redis err",nil)
			return
		end
	end)
end 
function modifyuinfo(uid,uface,uname)
	getuinfo(uid,function(err,result)
		if err~=nil then
			return
		end
		if uface~=nil then
			result.face = uface
		end
		if uname~=nil then
			result.name = uname
		end
		insert_uinfo(uid,result)
	end)

end
local get_guest_uinfo =
{
	getuinfo = getuinfo,
	insert_uinfo = insert_uinfo,
	modify_uinfo = modifyuinfo,
}

return get_guest_uinfo