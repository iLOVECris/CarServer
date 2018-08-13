local game_config = require("server_config")
local stype = require("Stype")
local ctype = require("Ctype")
local session_map = {}
local is_connected = {}
local session_utag = {}--映射表
local session_uid = {}
local ukey = 1
function connecting(stype,ip,port,name)
	netbus.tcp_connect(ip,port,function(err,session)
		is_connected[stype]= false
		if err~=0 then 
			print("connect  "..name.." server error")
			return
		end
		session_map[stype] = session
		print("connect  "..name.." "..stype.." server success")
	end)
end
function ReConnect()
	
	for k,v in pairs(game_config.servers) do
		if session_map[v.s_type]==nil and is_connected[v.s_type]==false then 
			print("connecting ..."..v.name.." server..")		
			is_connected[v.s_type] = true
			connecting(v.s_type,v.ip,v.port,v.name)
		end
	end 

	--connecting(1,"127.0.0.1",8006)
end

function Connect_Init()
	for k,v in pairs(game_config.servers) do
		session_map[v.s_type] = nil
		is_connected[v.s_type] = false
	end 

	timer.schedule_repeat(ReConnect,1000,-1,5000)
end
Connect_Init()
function session_disconnect(s,stype)--会向所有的服务发送断线消息，因此会收到多条
	if session.as_client(s)==1 then--是与服务器断开连接
		for k,v in pairs(session_map) do
			if session_map[k]==s then
				print("session disconnect")
				session_map[k] = nil
				return
			end
		end 
	else--与客户端断开连接
		local utag = session.get_utag(s)
		if session_utag[utag]~=nil and s==session_utag[utag] then
			session_utag[utag] = nil
		end
		local uid = session.get_uid(s)
		if session_uid[uid]~=nil and s==session_uid[uid] then
			session_uid[uid] = nil
		end
		if uid~= 0 then 
			local _msg = {stype,ctype.eExitDisconnect,uid,nil}
			if session_map[stype]~=nil then 
				session.send_cmd_msg(session_map[stype],_msg)
			end
		end
	end

	return
end

function send_to_server(s,raw)
	local s_stype,c_type,tag = RawCmd.readheader(raw)
	if c_type==ctype.eGuestLogin or c_type==ctype.eUserLoginReq then ---如果是登录请求
		local utag = session.get_utag(s)
		if utag==0 then--游客登录
			utag = ukey
			ukey = ukey+1
			session.set_utag(s,utag)
			session_utag[utag] = s
			RawCmd.set_utag(raw,utag)
		else
			session_utag[utag] = s
			RawCmd.set_utag(raw, utag)
		end
	else --其他请求
		local uid= session.get_uid(s)
		if uid==0 or session_uid[uid]==nil then --未登录状态
			return
		end
		RawCmd.set_utag(raw,uid)
	end
	if session_map[s_stype]~=nil then	
		session.send_raw_cmd(session_map[s_stype],raw)
	else
		print("server "..game_config.servers[s_stype].name.." is not connecting")
	end

end
function send_to_client(s,raw)
	local stype,c_type,utag = RawCmd.readheader(raw)
	if c_type == ctype.eGuestLoginRes or c_type == ctype.eUserLoginRes then ---如果是登录返回 utag是utag 其他的是uid
		local body = RawCmd.get_body(raw)
		local info = body.info
		if body.errcode == 0 then--成功登录
			if session_uid[info.uid]~=nil and session_utag[utag]~=session_uid[info.uid] then--已经在线中，顶掉
				local msg = {stype.Auth,ctype.eDropByOther,0,nil}
				session.send_cmd_msg(session_uid[info.uid],msg)
				session.close_session(session_uid[info.uid])
				session_uid[info.uid] = nil;
				return
			end 
			local s = session_utag[utag]
			session.set_uid(s,info.uid)
			session_utag[utag]=nil
			session_uid[info.uid] = s
			body.info.uid = 0
			local login_msg = {stype,c_type,0,body} 
			session.send_cmd_msg(s,login_msg)
			return
		else --登录异常
			RawCmd.set_utag(raw,0)
			session.send_raw_cmd(session_utag[utag],raw)
			return
		end
	end

	if utag~=0 then
		if session_uid[utag]~=nil then 
			session.send_raw_cmd(session_uid[utag],raw)
			if c_type==ctype.eUserLoginOutRes then --注销账号
				session.set_uid(s,0)
				session_uid[utag] = nil
			end
		end
	end
end
function recv_raw_msg(s,raw)
	if session.as_client(s)==0 then --转发到服务器
		send_to_server(s,raw)
	else
		send_to_client(s,raw)--转发到客户端
	end
end
local gateway_service = {
	on_session_disconnect = session_disconnect,
	on_session_recv_rawcmd =recv_raw_msg,
}

return gateway_service