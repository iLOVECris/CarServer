#ifndef  __EXPORT_REDIS_TO_LUA_H__
#define __EXPORT_REDIS_TO_LUA_H__


struct lua_State;
int Register_Redis_Module(lua_State* tolua_S);



#endif // ! __EXPORT_REDIS_TO_LUA_H__
