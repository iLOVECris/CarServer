local server_config = require("server_config")
local errcode = require("ErrCode")
local game_redis_config = server_config.game_redis_config
local Redis_Handler = nil
function Conncet_redis()
	redis_wrapper.connect(game_redis_config.ip,game_redis_config.port,
		function(err,handler)
			if err~=nil then 
				print("connect game center redis falied")
				timer.schedule_once(Conncet_redis,2000)
				return
			end
			Redis_Handler = handler
			print("connect game center redis success")
			local cmd = "select "..game_redis_config.redis_index
			redis_wrapper.query(Redis_Handler,cmd,function(err,result)
				if err==nil then 
					print("select game center redis db success")
				else
					print("select game center redis error")
				end		
			end)
		end)	
end
Conncet_redis()

function getuinfo(uid,callback)
	local cmd = "hgetall game_center_playerinfo_"..uid
	redis_wrapper.query(Redis_Handler,cmd,function(err,result)
		if err~=nil then
			callback("query sql err",nil)
			return
		end
		local uinfo = {}
		uinfo.player_exp = result[2]
		uinfo.player_grade = tonumber(result[4])
		uinfo.player_gold = tonumber(result[6])
		uinfo.player_vip = tonumber(result[8])
		callback(nil,uinfo)
	end)
end

function insert_uinfo(uid,uinfo)
	local tabel_name = "game_center_playerinfo_"..uid
	local cmd = "hmset "..tabel_name..
		" player_exp "..uinfo.player_exp..
		" player_grade "..uinfo.player_grade.. 
		" player_gold "..uinfo.player_gold..
		" player_vip "..uinfo.player_vip
	redis_wrapper.query(Redis_Handler,cmd,function(err,result)
		if err~=nil then
			print("query redis err",nil)
			return
		end
	end)
end 
function modifyuinfo(uid,gold)
	getuinfo(uid,function(err,result)
		if err~=nil then
			return
		end
		if uface~=nil then
			result.player_gold = result.player_gold + gold
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