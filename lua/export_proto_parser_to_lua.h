#ifndef __EXPORT_PROTO_PARSER_TO_LUA_H__
#define __EXPORT_PROTO_PARSER_TO_LUA_H__

struct lua_State;
int Register_Proto_Parser_Module(lua_State* tolua_S);
int Register_Raw_Msg_Module(lua_State* tolua_S);
#endif
