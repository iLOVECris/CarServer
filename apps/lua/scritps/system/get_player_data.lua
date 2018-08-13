local game_center = require("DataBase/game_center_db")
local stype = require("Stype")
local ctype = require("Ctype")
local ErrCode = require("ErrCode")
local login_bonus =require("system/login_bonus")
local game_center_redis = require("DataBase/game_center_redis")
local equipconfig = require("Config/EquipConfig")
local bonusconfig =require("Config/BonusConfig")
function getplayerdata(s,msg)
	local uid = msg[3]
	local utag = msg[3]
	game_center.getuinfo(uid,function(err,result)
		if err~=nil then
			local msg = {errcode = ErrCode.QueryErr}
			local send_msg = {stype.System,ctype.eGetPlayerDataRes,utag,msg}
			session.send_cmd_msg(s,send_msg)
			return
		end
		if result==nil then --查询结果为空，插入
			game_center.insert_uinfo(uid,function(err,result)
			if err~=nil then
				local msg = {errcode = ErrCode.QueryErr}
				local send_msg = {stype.System,ctype.eGetPlayerDataRes,utag,msg}
				session.send_cmd_msg(s,send_msg)
				return
			end
			getplayerdata(s,msg)
			end)
			--生成玩家道具表
			game_center.insert_equip_info(uid,function(err,result)
				if err~=nil then
					print("insert equip info err")
				return
			end
			end)
			return
		end
		local ustatus = tonumber(result[5])
		local uvip = tonumber(result[4])
		local umoney = tonumber(result[3])
		local ugrade = tonumber(result[2])
		local uexp = tonumber(result[1])
		if ustatus ~=0 then--0表示正常状态 1表示封禁
			local msg = {errcode = ErrCode.Freeze}
			local send_msg = {stype.System,ctype.eGetPlayerDataRes,utag,msg}
			session.send_cmd_msg(s,send_msg)
			return
		end
		local uinfo = {player_exp = uexp,player_grade = ugrade,player_gold = umoney,player_vip = uvip}
		game_center_redis.insert_uinfo(uid,uinfo)
		login_bonus.check_login_bonus(uid,function(err,data)
			if err~=nil then
				local msg = {errcode = ErrCode.QueryErr}
				local send_msg = {stype.System,ctype.eGetPlayerDataRes,utag,msg}
				session.send_cmd_msg(s,send_msg)
				return
			end
			local udata = {money = umoney,grade = ugrade,exp = uexp,vip = uvip}
			local msg = {errcode = 0,playerdata = udata,bonusdata = data}
			local send_msg = {stype.System,ctype.eGetPlayerDataRes,utag,msg}
			session.send_cmd_msg(s,send_msg)
		end)


	end)
end

function getplayerloginbonus(s,msg)
	local uid = msg[3]
	local utag = msg[3]
	local signinfo = msg[4]
	login_bonus.get_login_bonus(uid,signinfo,function(err,data)
		if err~=nil then
			local msg = {errcode = ErrCode.QueryErr}
			local send_msg = {stype.System,ctype.eGetLoginBonusRes,utag,msg}
			session.send_cmd_msg(s,send_msg)
			return
		end
		if data==nil then
			local msg = {errcode = ErrCode.GetBonusErr}
			local send_msg = {stype.System,ctype.eGetLoginBonusRes,utag,msg}
			session.send_cmd_msg(s,send_msg)
			return
		end
		local msg = {errcode = 0,id = data.id,num = data.num,bonusdata = data.bonusdata}
		--添加道具到数据库
		local equip = equipconfig[data.id]
		if equip~=nil then
			if equip.type>1 then --道具
				local wordkey = "equip_"..data.id
				game_center.update_equip_info(uid,wordkey,data.num,function(err)
					if err~=nil then
						local msg = {errcode = ErrCode.GetBonusErr}
						local send_msg = {stype.System,ctype.eGetLoginBonusRes,utag,msg}
						session.send_cmd_msg(s,send_msg)
						return
					end
				end)
			else
				game_center.update_player_data("player_gold",uid,data.num,function(err)
					if err~=nil then
						local msg = {errcode = ErrCode.GetBonusErr}
						local send_msg = {stype.System,ctype.eGetLoginBonusRes,utag,msg}
						session.send_cmd_msg(s,send_msg)
						return
					end
				end)
			end
		end
		local send_msg = {stype.System,ctype.eGetLoginBonusRes,utag,msg}
		session.send_cmd_msg(s,send_msg)
		return

	end)

end

local system_info = {
	getplayerdata = getplayerdata,
	getplayerloginbonus = getplayerloginbonus,
}

return system_info--玩家数据