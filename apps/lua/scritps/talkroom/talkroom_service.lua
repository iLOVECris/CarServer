local player_set = {}
local ctype = require("Ctype")
function on_login(s,msg)
	local utag = msg[3]
	local stype = msg[1]
	for i=1,#player_set do
		if player_set[i]==utag then--utag相同
			return
		end
	end
	table.insert(player_set,utag)
	local _msg = {stype,ctype.eRespose,utag,{status = 200}}
	session.send_cmd_msg(s,_msg)
end
function on_sendmsg(s,msg)
	if #player_set<=0 then
		return
	end
	local utag = msg[3]
	local body = msg[4]
	local stype = msg[1]
	for i = 1,#player_set do
		if player_set[i]~=utag then--非自己
			local _msg = {stype,ctype.eRecvMsg,player_set[i],{msg = body.msg}}
			session.send_cmd_msg(s,_msg)
		else
			local self_msg = {stype,ctype.eRespose,utag,{status = 100}}
			session.send_cmd_msg(s,self_msg)
		end
	end

end

function On_ExitTalkRoom(s,msg)
	if #player_set==0 then
		return 
	end
	local utag = msg[3]
	for i = #player_set, 1, -1  do
		if player_set[i]==utag then
			table.remove(player_set,i)
			print("remove "..utag)
		end
	end

end

function on_sendemoji(s,msg)
	if #player_set<=0 then
	return
	end
	local body = msg[4]
	local utag = msg[3]
	local stype = msg[1]
	for i = 1,#player_set do
		if player_set[i]~=utag then--非自己
			local _msg = {stype,ctype.eRecvEmoji,player_set[i],{id = body.id}}
			session.send_cmd_msg(s,_msg)
		else
			local self_msg = {stype,ctype.eRespose,utag,{status = 50}}
			session.send_cmd_msg(s,self_msg)
		end
	end
end

function recv_cmd(s,msg)
	local c_type = msg[2];
	if c_type == ctype.eLoginReq then
	on_login(s,msg)
	elseif c_type == ctype.eSendMsg then
	on_sendmsg(s,msg)
	elseif c_type == ctype.eSendEmoji then
	on_sendemoji(s,msg)
	elseif c_type == ctype.eExitChat then
	On_ExitTalkRoom(s,msg)
	elseif c_type == ctype.eExitDisconnect then
		print("disconnect"..msg[3])
	end
end

function session_disconnect(s,stype)

end

local talkroom_service = {
	on_session_recv_cmd = recv_cmd,
	on_session_disconnect = session_disconnect,
}

local tr_room = {
	stype = 2,
	service = talkroom_service,
}

return tr_room;