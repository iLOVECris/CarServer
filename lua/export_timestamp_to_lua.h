#ifndef __EXPORT_TIMESTAMP_TO_LUA_H__
#define __EXPORT_TIMESTAMP_TO_LUA_H__

struct lua_State;

int Register_TimeStamp_Module(lua_State* tolua_S);

#endif