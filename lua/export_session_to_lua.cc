#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif // !CPLUSPLUS

#include "tolua++.h"

#ifdef __cplusplus
}
#endif // CPLUSPLUS
#include "lua_wrapper.h"
#include "../../utils//logger.h"
#include "tolua_fix.h"
#include "export_session_to_lua.h"
#include "../../netbus/session.h"
#include "../../netbus/proto_parser.h"

using namespace google::protobuf;

static google::protobuf::Message*
lua_table_to_protobuf(lua_State* L, int stack_index, const char* msg_name)
{
	if (!lua_istable(L, stack_index)) {
		return NULL;
	}

	Message* message = proto_parser::create_message(msg_name);
	if (!message) {
		//log_debug("cant find message  %s from compiled poll \n", msg_name);
		return NULL;
	}

	const Reflection* reflection = message->GetReflection();
	const Descriptor* descriptor = message->GetDescriptor();

	// 遍历table的所有key， 并且与 protobuf结构相比较。如果require的字段没有赋值， 报错！ 如果找不到字段，报错！
	for (int32_t index = 0; index < descriptor->field_count(); ++index) {
		const FieldDescriptor* fd = descriptor->field(index);
		const string& name = fd->name();

		bool isRequired = fd->is_required();
		bool bReapeted = fd->is_repeated();
		lua_pushstring(L, name.c_str());
		lua_rawget(L, stack_index);

		bool isNil = lua_isnil(L, -1);

		if (bReapeted) {
			if (isNil) {
				lua_pop(L, 1);
				continue;
			}
			else {
				bool isTable = lua_istable(L, -1);
				if (!isTable) {
					log_error("cant find required repeated field %s\n", name.c_str());
					proto_parser::release_message(message);
					return NULL;
				}
			}

			lua_pushnil(L);
			for (; lua_next(L, -2) != 0;) {
				switch (fd->cpp_type()) {

				case FieldDescriptor::CPPTYPE_DOUBLE:
				{
					double value = luaL_checknumber(L, -1);
					reflection->AddDouble(message, fd, value);
				}
				break;
				case FieldDescriptor::CPPTYPE_FLOAT:
				{
					float value = luaL_checknumber(L, -1);
					reflection->AddFloat(message, fd, value);
				}
				break;
				case FieldDescriptor::CPPTYPE_INT64:
				{
					int64_t value = luaL_checknumber(L, -1);
					reflection->AddInt64(message, fd, value);
				}
				break;
				case FieldDescriptor::CPPTYPE_UINT64:
				{
					uint64_t value = luaL_checknumber(L, -1);
					reflection->AddUInt64(message, fd, value);
				}
				break;
				case FieldDescriptor::CPPTYPE_ENUM: // 与int32一样处理
				{
					int32_t value = luaL_checknumber(L, -1);
					const EnumDescriptor* enumDescriptor = fd->enum_type();
					const EnumValueDescriptor* valueDescriptor = enumDescriptor->FindValueByNumber(value);
					reflection->AddEnum(message, fd, valueDescriptor);
				}
				break;
				case FieldDescriptor::CPPTYPE_INT32:
				{
					int32_t value = luaL_checknumber(L, -1);
					reflection->AddInt32(message, fd, value);
				}
				break;
				case FieldDescriptor::CPPTYPE_UINT32:
				{
					uint32_t value = luaL_checknumber(L, -1);
					reflection->AddUInt32(message, fd, value);
				}
				break;
				case FieldDescriptor::CPPTYPE_STRING:
				{
					size_t size = 0;
					const char* value = luaL_checklstring(L, -1, &size);
					reflection->AddString(message, fd, std::string(value, size));
				}
				break;
				case FieldDescriptor::CPPTYPE_BOOL:
				{
					bool value = lua_toboolean(L, -1);
					reflection->AddBool(message, fd, value);
				}
				break;
				case FieldDescriptor::CPPTYPE_MESSAGE:
				{
					Message* value = lua_table_to_protobuf(L, lua_gettop(L), fd->message_type()->name().c_str());
					if (!value) {
						log_error("convert to message %s failed whith value %s\n", fd->message_type()->name().c_str(), name.c_str());
						proto_parser::release_message(value);
						return NULL;
					}
					Message* msg = reflection->AddMessage(message, fd);
					msg->CopyFrom(*value);
					proto_parser::release_message(value);
				}
				break;
				default:
					break;
				}

				// remove value， keep the key
				lua_pop(L, 1);
			}
		}
		else {

			if (isRequired) {
				if (isNil) {
					log_error("cant find required field %s\n", name.c_str());
					proto_parser::release_message(message);
					return NULL;
				}
			}
			else {
				if (isNil) {
					lua_pop(L, 1);
					continue;
				}
			}

			switch (fd->cpp_type()) {
			case FieldDescriptor::CPPTYPE_DOUBLE:
			{
				double value = luaL_checknumber(L, -1);
				reflection->SetDouble(message, fd, value);
			}
			break;
			case FieldDescriptor::CPPTYPE_FLOAT:
			{
				float value = luaL_checknumber(L, -1);
				reflection->SetFloat(message, fd, value);
			}
			break;
			case FieldDescriptor::CPPTYPE_INT64:
			{
				int64_t value = luaL_checknumber(L, -1);
				reflection->SetInt64(message, fd, value);
			}
			break;
			case FieldDescriptor::CPPTYPE_UINT64:
			{
				uint64_t value = luaL_checknumber(L, -1);
				reflection->SetUInt64(message, fd, value);
			}
			break;
			case FieldDescriptor::CPPTYPE_ENUM: // 与int32一样处理
			{
				int32_t value = luaL_checknumber(L, -1);
				const EnumDescriptor* enumDescriptor = fd->enum_type();
				const EnumValueDescriptor* valueDescriptor = enumDescriptor->FindValueByNumber(value);
				reflection->SetEnum(message, fd, valueDescriptor);
			}
			break;
			case FieldDescriptor::CPPTYPE_INT32:
			{
				int32_t value = luaL_checknumber(L, -1);
				reflection->SetInt32(message, fd, value);
			}
			break;
			case FieldDescriptor::CPPTYPE_UINT32:
			{
				uint32_t value = luaL_checknumber(L, -1);
				reflection->SetUInt32(message, fd, value);
			}
			break;
			case FieldDescriptor::CPPTYPE_STRING:
			{
				size_t size = 0;
				const char* value = luaL_checklstring(L, -1, &size);
				reflection->SetString(message, fd, std::string(value, size));
			}
			break;
			case FieldDescriptor::CPPTYPE_BOOL:
			{
				bool value = lua_toboolean(L, -1);
				reflection->SetBool(message, fd, value);
			}
			break;
			case FieldDescriptor::CPPTYPE_MESSAGE:
			{
				Message* value = lua_table_to_protobuf(L, lua_gettop(L), fd->message_type()->name().c_str());
				if (!value) {
					log_error("convert to message %s failed whith value %s \n", fd->message_type()->name().c_str(), name.c_str());
					proto_parser::release_message(message);
					return NULL;
				}
				Message* msg = reflection->MutableMessage(message, fd);
				msg->CopyFrom(*value);
				proto_parser::release_message(value);
			}
			break;
			default:
				break;
			}
		}

		// pop value
		lua_pop(L, 1);
	}

	return message;
}
int lua_send_raw_cmd(lua_State* tolua_S)
{
	session* s = (session*)tolua_touserdata(tolua_S, 1, NULL);
	if (s == NULL) {
		log_error("session is null");
		return 0;
	}
	raw_cmd* cmd = (raw_cmd*)tolua_touserdata(tolua_S, 2, NULL);
	s->send_raw_cmd(cmd);
	return 0;
}
int lua_send_cmd_msg(lua_State* tolua_S)
{
	session* s = (session*)tolua_touserdata(tolua_S, 1, NULL);
	if (s == NULL) {
		log_error("session is null");
		return 0;
	}
	
	// stack: 1 s, 2, table,
	if (!lua_istable(tolua_S, 2)) { 
		log_error("param is not table");
		return 0;
	}

	struct cmd_msg msg;

	//int n = luaL_len(tolua_S, 2);
	//if (n != 4)
	//{
	//	log_error("param is not avalibale");
	//	return 0;
	//}

	lua_pushnumber(tolua_S, 1);//数字1压入栈顶
	lua_gettable(tolua_S, 2);//取表的栈顶索引的值压入栈中
	msg.stype = luaL_checkinteger(tolua_S, -1);

	lua_pushnumber(tolua_S, 2);
	lua_gettable(tolua_S, 2);
	msg.ctype = luaL_checkinteger(tolua_S, -1);

	lua_pushnumber(tolua_S, 3);
	lua_gettable(tolua_S, 2);
	msg.utag = luaL_checkinteger(tolua_S, -1);

	lua_pushnumber(tolua_S, 4);
	lua_gettable(tolua_S, 2);

	if (proto_parser::proto_type() == PROTO_JSON) {
		msg.body = (char*)lua_tostring(tolua_S, -1);
		s->send_cmd_msg(&msg);
	}
	else {
		if (!lua_istable(tolua_S, -1)) {
			msg.body = NULL;
			s->send_cmd_msg(&msg);
		}
		else { // protobuf message table
			const char* msg_name = proto_parser::protobuf_cmd_name(msg.ctype);
			msg.body = lua_table_to_protobuf(tolua_S, lua_gettop(tolua_S), msg_name);
			s->send_cmd_msg(&msg);
			if (msg.body != NULL)
			{
				proto_parser::release_message((google::protobuf::Message*)(msg.body));
			}
		}
	}
}

int lua_close_session(lua_State* tolua_S)
{
	session* s = (session*)lua_touserdata(tolua_S, 1);
	if (s == NULL)
	{
		log_error("session is null");
	}
	s->close();
	return 0;
}

int lua_get_address(lua_State* tolua_S)
{
	session* s = (session*)lua_touserdata(tolua_S, 1);
	if (s == NULL)
	{
		log_error("session is null");
		return 0;
	}
	unsigned int port;
	const char* ip = s->get_address(&port);
	lua_pushfstring(tolua_S, ip);
	lua_pushinteger(tolua_S, port);
	return 2;
}
int lua_get_utag(lua_State* tolua_S)
{
	session* s = (session*)lua_touserdata(tolua_S, 1);
	if (s == NULL)
	{
		log_error("session is null");
		return 0;
	}
	int utag;
	utag = s->utag;
	lua_pushinteger(tolua_S, utag);
	return 1;
}
int lua_set_uatg(lua_State* tolua_S)
{
	session* s = (session*)lua_touserdata(tolua_S, 1);
	if (s == NULL)
	{
		log_error("session is null");
		return 0;
	}
	int utag = lua_tointeger(tolua_S, 2);
	s->utag = utag;
	return 0;
}
int lua_get_uid(lua_State* tolua_S)
{
	session* s = (session*)lua_touserdata(tolua_S, 1);
	if (s == NULL)
	{
		log_error("session is null");
		return 0;
	}
	int uid;
	uid = s->uid;
	lua_pushinteger(tolua_S, uid);
	return 1;
}
int lua_set_uid(lua_State* tolua_S)
{
	session* s = (session*)lua_touserdata(tolua_S, 1);
	if (s == NULL)
	{
		log_error("session is null");
		return 0;
	}
	int uid = lua_tointeger(tolua_S, 2);
	s->uid = uid;
	return 0;
}

int lua_getasclient(lua_State* tolua_S)
{
	session* s = (session*)lua_touserdata(tolua_S, 1);
	if (s == NULL)
	{
		log_error("session is null");
		return 0;
	}
	lua_pushinteger(tolua_S, s->as_client);
	return 1;
}

int Register_Session_Module(lua_State* tolua_S)
{
	lua_getglobal(tolua_S, "_G");
	if (lua_istable(tolua_S, -1)) {
		tolua_open(tolua_S);
		tolua_module(tolua_S, "session", 0);
		tolua_beginmodule(tolua_S, "session");

		tolua_function(tolua_S, "send_cmd_msg", lua_send_cmd_msg);
		tolua_function(tolua_S, "send_raw_cmd", lua_send_raw_cmd);
		tolua_function(tolua_S, "close_session", lua_close_session);
		tolua_function(tolua_S, "get_address", lua_get_address);
		tolua_function(tolua_S, "get_utag", lua_get_utag);
		tolua_function(tolua_S, "set_utag", lua_set_uatg);
		tolua_function(tolua_S, "get_uid", lua_get_uid);
		tolua_function(tolua_S, "set_uid", lua_set_uid);
		tolua_function(tolua_S, "as_client", lua_getasclient);
		tolua_endmodule(tolua_S);
	}
	lua_pop(tolua_S, 1);
	return 0;
}