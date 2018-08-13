local server_config = require("server_config")
local errcode = require("ErrCode")
local game_db_config = server_config.game_db_config
local MySql_Handler
function Conncet_db()
	mysql_wrapper.connect(game_db_config.ip,game_db_config.port,game_db_config.db_name,game_db_config.user,game_db_config.password,
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

--bonus start

function insert_bonus_info(uid,callback)
	local sql = "insert into login_bonus_data(`uid`, `monday`,`tuesday`,`wednesday`,`thursday`,`friday`,`saturday`,`weekday`,`weeks`)values(%d,0,0,0,0,0,0,0,0)"
	local cmd = string.format(sql,uid)
	mysql_wrapper.query(MySql_Handler,cmd,function(err,result)
		if err~=nil then
			callback("query sql err",nil)
			return
		end
		callback(nil,nil)
	end)
end
 function get_login_bonus_info(uid,callback)
	
	local sql = "select monday, tuesday,wednesday,thursday,friday,saturday,weekday,weeks from login_bonus_data where uid = %d limit 1"
	local cmd = string.format(sql,uid)
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

 function update_login_bonus_info(uid,data,callback)
	local sql = "update login_bonus_data set monday = %d, tuesday = %d,wednesday = %d,thursday = %d,friday = %d,saturday = %d,weekday = %d,weeks = %d where uid = %d"
		local cmd = string.format(sql,data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],uid)
		mysql_wrapper.query(MySql_Handler,cmd,function(err,result)
			callback(err)
		end)

 end

--bonus end

--uinfo start

function getuinfo(uid,callback)
	local sql = "select player_exp, player_grade, player_gold, player_vip, player_status from player_data where uid = %d limit 1"
	local cmd = string.format(sql,uid)
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

function insert_uinfo(uid,callback)
	local sql = "insert into player_data(`uid`,`player_exp`, `player_grade`, `player_gold`, `player_vip`, `player_status`)values(%d,0, 1, 100, 0, 0)"
	local cmd = string.format(sql,uid)
	mysql_wrapper.query(MySql_Handler,cmd,function(err,result)
		if err~=nil then
			callback("query sql err",nil)
			return
		end
		callback(nil,nil)
	end)
end 

function update_player_data(wordkey,uid,num,callback)
	get_player_data(wordkey,uid,function(err,ret)
		if err~=nil then
			callback(err,nil)
			return
		end
		local sql = "update player_data set".." "..wordkey.."=%d where uid = %d"
		local addcount = num+tonumber(ret[1])
		local cmd = string.format(sql,addcount,uid)
		mysql_wrapper.query(MySql_Handler,cmd,function(err,result)
			callback(err)
		end)
	end)
end

function get_player_data(wordkey,uid,callback)
	local sql = "select "..wordkey.." from player_data where uid = %d limit 1"
	local cmd = string.format(sql,uid)
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

--uinfo end

--equip start

 function update_equip_info(uid,wordkey,num,callback)
 	get_equip_info(wordkey,uid,function(err,ret)
 		if err~=nil then
 			callback(err,nil)
 			return
 		end
 		local sql = "update player_bag set "..wordkey.."=%d where uid = %d"
 		local addcount = num+tonumber(ret[1])
		local cmd = string.format(sql,addcount,uid)
		mysql_wrapper.query(MySql_Handler,cmd,function(err,result)
			callback(err)
		end)
 	end)


 end

function insert_equip_info(uid,callback)
	local sql = "insert into player_bag(`uid`)values(%d)"
	local cmd = string.format(sql,uid)
	mysql_wrapper.query(MySql_Handler,cmd,function(err,result)
		if err~=nil then
			callback("query sql err",nil)
			return
		end
		callback(nil,nil)
	end)
end

function get_equip_info(wordkey,uid,callback)
	local sql = "select "..wordkey.." from player_bag where uid = %d limit 1"
	local cmd = string.format(sql,uid)
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

function getallequip(uid,callback)
	local sql = "select* from player_bag where uid = %d limit 1"
	local cmd = string.format(sql,uid)
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

--equip end

local get_guest_uinfo =
{
	get_login_bonus_info = get_login_bonus_info,
	insert_bonus_info = insert_bonus_info,
	update_login_bonus_info = update_login_bonus_info,

	getuinfo = getuinfo,
	insert_uinfo = insert_uinfo,
	get_player_data = get_player_data,
	update_player_data = update_player_data,

	getallequip = getallequip,
	insert_equip_info = insert_equip_info,
	get_equip_info = get_equip_info,
	update_equip_info = update_equip_info,
}

return get_guest_uinfo