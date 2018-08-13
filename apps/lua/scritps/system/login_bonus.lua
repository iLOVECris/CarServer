local game_center = require("DataBase/game_center_db")
local stype = require("Stype")
local ctype = require("Ctype")
local ErrCode = require("ErrCode")
local WeekBonus = require("WeekBonus")
local bonustable = require("Config/BonusConfig")

function PackageBonusData(uid,result,callback)
	if result==nil then
		callback(nil)
		return
	end
	local datatable = {}
	for k,v in pairs(WeekBonus) do
		datatable[v] = tonumber(result[v])
	end
	datatable[8] = tonumber(result[8])
	local weeknow = timestamp.timeweeknow()
	local weeks = timestamp.timeweeks()
	if weeknow==0 then
		weeknow=7
	end
	--[[
	WaitSign = 0,//待签到
    CanSign = 1,//可签到
    AgainSign = 2,//可补签
    AlreadySign = 3,//已经签到
	--]]
	local tableLoginBonusData = {}
	if weeks~=tonumber(result[8]) then--不是当前周,清除数据
		for k,v in pairs(datatable) do
			if k<weeknow then
				datatable[k]=2
			elseif k==weeknow then
				datatable[k]=1
			elseif k>weeknow then
				datatable[k] = 0
			end
		end
		for k,v in pairs(WeekBonus) do
			local BonusData = {id = bonustable[v].id,num = bonustable[v].num,status = datatable[v],day = v}
			tableLoginBonusData[v] = BonusData
		end
		game_center.update_login_bonus_info(uid,datatable,function(err)
			if err~=nil then
				callback(nil)
				return
			end
		end)
	else
		if datatable[weeknow]~=3 then--如果今天没签到
			for k,v in pairs(datatable) do
				if k<weeknow then
					if datatable[k]~=3 then
						datatable[k]=2
					end
				elseif k==weeknow then
					if datatable[k]~=3 then
						datatable[k]=1
					end
				elseif k>weeknow then
					datatable[k] = 0
				end
			end
			for k,v in pairs(WeekBonus) do
				local BonusData = {id = bonustable[v].id,num =  bonustable[v].num,status = datatable[v],day = v}
				tableLoginBonusData[v] = BonusData
			end
			game_center.update_login_bonus_info(uid,datatable,function(err)
				if err~=nil then
					callback(nil)
					return
				end
			end)
		end
	end
	callback(tableLoginBonusData)
end

function PackageSignedBonus(result,callback)--打包签到完的签到信息
	local tableLoginBonusData = {}
	for k,v in pairs(WeekBonus) do
		local BonusData = {id = bonustable[v].id,num =  bonustable[v].num,status =  tonumber(result[v]),day = v}
		tableLoginBonusData[v] = BonusData
	end
	callback(tableLoginBonusData)
end

function check_login_bonus(uid,callback)
	game_center.get_login_bonus_info(uid,function(err,result)
		if err~=nil then
			callback(err,nil)
			return
		end
		if result==nil then --查询结果为空，插入
			game_center.insert_bonus_info(uid,function(err,result)
				if err~=nil then
					callback(err,nil)
					return
				end
			end)
		end
		PackageBonusData(uid,result,function(data)
			if data~=nil then
				callback(nil,data)
			else 
				callback("data err",nil)
			end
		end)
	end)
end

function get_login_bonus(uid,signinfo,callback)
	game_center.get_login_bonus_info(uid,function(err,result)
		if err~=nil then
			callback(err,nil)
		end
		if result==nil then --查询结果为空，插入
			print("result==null")
			callback(nil,nil)
			return
		end
		local weeknow = timestamp.timeweeknow()
		if  signinfo.day>weeknow then
			print("signinfo not match day")
			callback(nil,nil);
			return
		end
		if  tonumber(result[signinfo.day])~=signinfo.signtype then--签到类型不匹配
			print("signinfo not match")
			callback(nil,nil)
			return
		end	
		local bonus = bonustable[signinfo.day]
		result[signinfo.day] = 3
		local weeks = timestamp.timeweeks()
		if tonumber(result[8])~=weeks then
			result[8]=weeks
		end
		game_center.update_login_bonus_info(uid,result,function(err)
			if err~=nil then
				callback(err,nil)
			end
		end)
		PackageSignedBonus(result,function(data)
			if data==nil then
				callback(nil,nil)
			end
			local info = {id = bonus.id,num = bonus.num,bonusdata = data}
			callback(nil,info)
		end)
		

	end)

end

local login_bonus = {
	check_login_bonus = check_login_bonus, 
	get_login_bonus = get_login_bonus,
}
return login_bonus