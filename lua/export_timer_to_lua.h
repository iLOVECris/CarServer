#ifndef __EXPORT_TIMER_TO_LUA_H__
#define __EXPORT_TIMER_TO_LUA_H__

struct lua_State;

int Register_Timer_Module(lua_State* tolua_S);

#endif