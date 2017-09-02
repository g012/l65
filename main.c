#define LUA_IMPLEMENTATION
#include "lua.h"

extern int luaopen_lpeg(lua_State *L);
extern int luaopen_lfs(lua_State *L);

int main(int argc, char *argv[])
{
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    luaL_require(L, "lpeg", luaopen_lpeg, 1); lua_pop(L, 1);
    luaL_require(L, "lfs", luaopen_lfs, 1); lua_pop(L, 1);
    return 0;
}

