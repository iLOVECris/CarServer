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
#include "export_service_to_lua.h"
#include "../../netbus/Service.h"
#include "../../netbus//ServiceManager.h"
#include "../../netbus/proto_parser.h"


#include "google/protobuf/message.h"
using namespace google::protobuf;

#define TOLUA_SERVICE_FUNCTION_MAPPING "toluafix_service_function_mapping"
static int service_function_ref_id = 0;

class Lua_Service :public Service
{
public:
	unsigned int session_recv_cmd_handler;
	unsigned int session_disconnect_handler;
	unsigned int session_recv_rawcmd_handler;
public:
	virtual bool on_session_recv_cmd(session* s, struct cmd_msg* msg);
	virtual void on_session_disconnect(session* s,int stpe);
	virtual bool on_session_recv_rawcmd(session*s, struct raw_cmd* msg);
};

void init_service_func_map(lua_State* L)
{
	lua_pushstring(L, TOLUA_SERVICE_FUNCTION_MAPPING);
	lua_newtable(L);
	lua_rawset(L, LUA_REGISTRYINDEX);
}
void push_proto_message_tolua(const Message* message) 
{
	lua_State* state = lua_wrapper::lua_state();
	if (!message) {
		// printf("PushProtobuf2LuaTable failed, message is NULL");
		return;
	}
	const Reflection* reflection = message->GetReflection();

	// 顶层table
	lua_newtable(state);

	const Descriptor* descriptor = message->GetDescriptor();
	for (int32_t index = 0; index < descriptor->field_count(); ++index) {
		const FieldDescriptor* fd = descriptor->field(index);
		const std::string& name = fd->lowercase_name();

		// key
		lua_pushstring(state, name.c_str());

		bool bReapeted = fd->is_repeated();

		if (bReapeted) {
			// repeated这层的table
			lua_newtable(state);
			int size = reflection->FieldSize(*message, fd);
			for (int i = 0; i < size; ++i) {
				char str[32] = { 0 };
				switch (fd->cpp_type()) {
				case FieldDescriptor::CPPTYPE_DOUBLE:
					lua_pushnumber(state, reflection->GetRepeatedDouble(*message, fd, i));
					break;
				case FieldDescriptor::CPPTYPE_FLOAT:
					lua_pushnumber(state, (double)reflection->GetRepeatedFloat(*message, fd, i));
					break;
				case FieldDescriptor::CPPTYPE_INT64:
					sprintf(str, "%lld", (long long)reflection->GetRepeatedInt64(*message, fd, i));
					lua_pushstring(state, str);
					break;
				case FieldDescriptor::CPPTYPE_UINT64:

					sprintf(str, "%llu", (unsigned long long)reflection->GetRepeatedUInt64(*message, fd, i));
					lua_pushstring(state, str);
					break;
				case FieldDescriptor::CPPTYPE_ENUM: // 与int32一样处理
					lua_pushinteger(state, reflection->GetRepeatedEnum(*message, fd, i)->number());
					break;
				case FieldDescriptor::CPPTYPE_INT32:
					lua_pushinteger(state, reflection->GetRepeatedInt32(*message, fd, i));
					break;
				case FieldDescriptor::CPPTYPE_UINT32:
					lua_pushinteger(state, reflection->GetRepeatedUInt32(*message, fd, i));
					break;
				case FieldDescriptor::CPPTYPE_STRING:
				{
					std::string value = reflection->GetRepeatedString(*message, fd, i);
					lua_pushlstring(state, value.c_str(), value.size());
				}
				break;
				case FieldDescriptor::CPPTYPE_BOOL:
					lua_pushboolean(state, reflection->GetRepeatedBool(*message, fd, i));
					break;
				case FieldDescriptor::CPPTYPE_MESSAGE:
					push_proto_message_tolua(&(reflection->GetRepeatedMessage(*message, fd, i)));
					break;
				default:
					break;
				}

				lua_rawseti(state, -2, i + 1); // lua's index start at 1
			}

		}
		else {
			char str[32] = { 0 };
			switch (fd->cpp_type()) {

			case FieldDescriptor::CPPTYPE_DOUBLE:
				lua_pushnumber(state, reflection->GetDouble(*message, fd));
				break;
			case FieldDescriptor::CPPTYPE_FLOAT:
				lua_pushnumber(state, (double)reflection->GetFloat(*message, fd));
				break;
			case FieldDescriptor::CPPTYPE_INT64:

				sprintf(str, "%lld", (long long)reflection->GetInt64(*message, fd));
				lua_pushstring(state, str);
				break;
			case FieldDescriptor::CPPTYPE_UINT64:

				sprintf(str, "%llu", (unsigned long long)reflection->GetUInt64(*message, fd));
				lua_pushstring(state, str);
				break;
			case FieldDescriptor::CPPTYPE_ENUM: // 与int32一样处理
				lua_pushinteger(state, (int)reflection->GetEnum(*message, fd)->number());
				break;
			case FieldDescriptor::CPPTYPE_INT32:
				lua_pushinteger(state, reflection->GetInt32(*message, fd));
				break;
			case FieldDescriptor::CPPTYPE_UINT32:
				lua_pushinteger(state, reflection->GetUInt32(*message, fd));
				break;
			case FieldDescriptor::CPPTYPE_STRING:
			{
				std::string value = reflection->GetString(*message, fd);
				lua_pushlstring(state, value.c_str(), value.size());
			}
			break;
			case FieldDescriptor::CPPTYPE_BOOL:
				lua_pushboolean(state, reflection->GetBool(*message, fd));
				break;
			case FieldDescriptor::CPPTYPE_MESSAGE:
				push_proto_message_tolua(&(reflection->GetMessage(*message, fd)));
				break;
			default:
				break;
			}
		}

		lua_rawset(state, -3);
	}
}
void get_service_function(lua_State* L, int refid)
{
	lua_pushstring(L, TOLUA_SERVICE_FUNCTION_MAPPING);
	lua_rawget(L, LUA_REGISTRYINDEX);                           /* stack: ... refid_fun */
	lua_pushinteger(L, refid);                                  /* stack: ... refid_fun refid */
	lua_rawget(L, -2);                                          /* stack: ... refid_fun fun */
	lua_remove(L, -2);                                          /* stack: ... fun */
}

int exe_function(int numArgs)
{
	int functionIndex = -(numArgs + 1);
	if (!lua_isfunction(lua_wrapper::lua_state(), functionIndex))
	{
		log_error("value at stack [%d] is not function", functionIndex);
		lua_pop(lua_wrapper::lua_state(), numArgs + 1); // remove function and arguments
		return 0;
	}

	int traceback = 0;
	lua_getglobal(lua_wrapper::lua_state(), "__G__TRACKBACK__");                         /* L: ... func arg1 arg2 ... G */
	if (!lua_isfunction(lua_wrapper::lua_state(), -1))
	{
		lua_pop(lua_wrapper::lua_state(), 1);                                            /* L: ... func arg1 arg2 ... */
	}
	else
	{
		lua_insert(lua_wrapper::lua_state(), functionIndex - 1);                         /* L: ... G func arg1 arg2 ... */
		traceback = functionIndex - 1;
	}

	int error = 0;
	error = lua_pcall(lua_wrapper::lua_state(), numArgs, 1, traceback);                  /* L: ... [G] ret */
	if (error)
	{
		if (traceback == 0)
		{
			log_error("[LUA ERROR] %s", lua_tostring(lua_wrapper::lua_state(), -1));        /* L: ... error */
			lua_pop(lua_wrapper::lua_state(), 1); // remove error message from stack
		}
		else                                                            /* L: ... G error */
		{
			lua_pop(lua_wrapper::lua_state(), 2); // remove __G__TRACKBACK__ and error message from stack
		}
		return 0;
	}

	// get return value
	int ret = 0;
	if (lua_isnumber(lua_wrapper::lua_state(), -1))
	{
		ret = (int)lua_tointeger(lua_wrapper::lua_state(), -1);
	}
	else if (lua_isboolean(lua_wrapper::lua_state(), -1))
	{
		ret = (int)lua_toboolean(lua_wrapper::lua_state(), -1);
	}
	// remove return value from stack
	lua_pop(lua_wrapper::lua_state(), 1);                                                /* L: ... [G] */

	if (traceback)
	{
		lua_pop(lua_wrapper::lua_state(), 1); // remove __G__TRACKBACK__ from stack      /* L: ... */
	}

	return ret;
}

bool push_service_function(int nHandler)
{
	get_service_function(lua_wrapper::lua_state(), nHandler);                  /* L: ... func */
	if (!lua_isfunction(lua_wrapper::lua_state(), -1))
	{
		log_error("[LUA ERROR] function refid '%d' does not reference a Lua function", nHandler);
		lua_pop(lua_wrapper::lua_state(), 1);
		return false;
	}
	return true;
}
int execute_service_function(unsigned int nHandler, int numArgs)
{
	int ret = 0;
	if (push_service_function(nHandler))                                /* L: ... arg1 arg2 ... func */
	{
		if (numArgs > 0)
		{
			lua_insert(lua_wrapper::lua_state(), -(numArgs + 1));                        /* L: ... func arg1 arg2 ... */
		}
		ret = exe_function(numArgs);
	}
	lua_settop(lua_wrapper::lua_state(), 0);
	return ret;
}
bool Lua_Service::on_session_recv_cmd(session* s,cmd_msg* msg)
{
	tolua_pushuserdata(lua_wrapper::lua_state(), (void*)s);
	int index = 1;
	lua_newtable(lua_wrapper::lua_state());
	lua_pushinteger(lua_wrapper::lua_state(), msg->stype);//将值压入栈
	lua_rawseti(lua_wrapper::lua_state(), -2, index);//表的index索引值为栈顶元素
	++index;

	lua_pushinteger(lua_wrapper::lua_state(), msg->ctype);
	lua_rawseti(lua_wrapper::lua_state(), -2, index);          /* table[index] = value, L: table */
	++index;

	lua_pushinteger(lua_wrapper::lua_state(), msg->utag);
	lua_rawseti(lua_wrapper::lua_state(), -2, index);          /* table[index] = value, L: table */
	++index;

	if (!msg->body) {
		lua_pushnil(lua_wrapper::lua_state());
		lua_rawseti(lua_wrapper::lua_state(), -2, index);
		++index;
	}
	else {
		if (proto_parser::proto_type() == PROTO_JSON) {
			lua_pushstring(lua_wrapper::lua_state(), (char*)msg->body);
		}
		else { // protobuf
			push_proto_message_tolua((Message*)msg->body);
		}
		lua_rawseti(lua_wrapper::lua_state(), -2, index);          /* table[index] = value, L: table */
		++index;
	}

	int ret = execute_service_function(this->session_recv_cmd_handler, 2);
	return true;
}

void Lua_Service::on_session_disconnect(session* s,int stype)
{
	tolua_pushuserdata(lua_wrapper::lua_state(), (void*)s);
	lua_pushinteger(lua_wrapper::lua_state(), stype);
	execute_service_function(this->session_disconnect_handler, 2);
}

bool Lua_Service::on_session_recv_rawcmd(session*s, struct raw_cmd* msg)
{
	tolua_pushuserdata(lua_wrapper::lua_state(), (void*)s);
	tolua_pushuserdata(lua_wrapper::lua_state(), (void*)msg);
	int ret = execute_service_function(this->session_recv_rawcmd_handler, 2);
	return true;
}
int save_service_function(lua_State* L, int lo, int def)
{
	if (!lua_isfunction(L, lo)) return 0;

	service_function_ref_id++;

	lua_pushstring(L, TOLUA_SERVICE_FUNCTION_MAPPING);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_pushinteger(L, service_function_ref_id);
	lua_pushvalue(L, lo);

	lua_rawset(L, -3);
	lua_pop(L, 1);

	return service_function_ref_id;
}

int lua_register_service(lua_State* tolua_S)
{
	int stype = tolua_tonumber(tolua_S, 1, 0);
	bool ret;
	if (!lua_istable(tolua_S, 2)) {
		log_error("接收的参数错误");
	}
	unsigned int session_recv_cmd_handler = 0;
	unsigned int session_disconnect_handler = 0;
	unsigned int session_recv_rawcmd_handler = 0;
	lua_getfield(tolua_S, 2, "on_session_recv_cmd");//将第二个参数（表）中的函数，压入堆栈
	lua_getfield(tolua_S, 2, "on_session_disconnect");

	session_recv_cmd_handler = save_service_function(tolua_S, 3, 0);
	session_disconnect_handler = save_service_function(tolua_S, 4, 0);
	session_recv_rawcmd_handler = 0;
	if (session_recv_cmd_handler == 0 || session_disconnect_handler == 0)
	{
		log_error("接收函数id错误");
	}
	Lua_Service* service = new Lua_Service();
	service->session_recv_cmd_handler = session_recv_cmd_handler;
	service->session_disconnect_handler = session_disconnect_handler;
	service->session_recv_rawcmd_handler = session_recv_rawcmd_handler;
	ret = ServiceManager::register_service(stype, service);
	lua_pushboolean(tolua_S, ret ? 1 : 0);

	return 1;
}

int lua_register_raw_service(lua_State* tolua_S)
{
	int stype = tolua_tonumber(tolua_S, 1, 0);
	
	bool ret;
	if (!lua_istable(tolua_S, 2)) {
		log_error("接收的参数错误");
	}
	unsigned int session_recv_cmd_handler = 0;
	unsigned int session_disconnect_handler = 0;
	unsigned int session_recv_rawcmd_handler = 0;
	lua_getfield(tolua_S, 2, "on_session_disconnect");
	lua_getfield(tolua_S, 2, "on_session_recv_rawcmd");//将第二个参数（表）中的函数，压入堆栈

	session_recv_cmd_handler = 0;
	session_disconnect_handler = save_service_function(tolua_S, 3, 0);
	session_recv_rawcmd_handler = save_service_function(tolua_S, 4, 0);
	if (session_recv_rawcmd_handler == 0 || session_disconnect_handler == 0)
	{
		log_error("接收函数id错误");
	}
	Lua_Service* service = new Lua_Service();
	service->session_recv_cmd_handler = session_recv_cmd_handler;
	service->session_disconnect_handler = session_disconnect_handler;
	service->session_recv_rawcmd_handler = session_recv_rawcmd_handler;
	service->using_raw_cmd = true;
	ret = ServiceManager::register_service(stype, service);
	lua_pushboolean(tolua_S, ret ? 1 : 0);

	return 1;
}
int Register_Service_Module(lua_State* tolua_S)
{
	init_service_func_map(tolua_S);
	lua_getglobal(tolua_S, "_G");
	if (lua_istable(tolua_S, -1)) {
		tolua_open(tolua_S);
		tolua_module(tolua_S, "service", 0);
		tolua_beginmodule(tolua_S, "service");

		tolua_function(tolua_S, "register", lua_register_service);
		tolua_function(tolua_S, "register_raw", lua_register_raw_service);
		tolua_endmodule(tolua_S); 
	}
	lua_pop(tolua_S, 1);
	return 0;
}