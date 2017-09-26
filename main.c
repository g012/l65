#define LUA_IMPLEMENTATION
#include "lua.h"
#include "scripts.h"

extern int luaopen_lpeg(lua_State *L);
extern int luaopen_lfs(lua_State *L);

static struct script { const char *name; int t;  const char *data; size_t sz; } embedded[] = {
    { "l65cfg", 0, script_l65cfg_lua, sizeof(script_l65cfg_lua) },
    { "vcs", 1, script_vcs_l65, sizeof(script_vcs_l65) },
};

static int getembedded(lua_State *L)
{
    const char *name = lua_tostring(L, 1);
    for (struct script *s = embedded, *e = s + sizeof(embedded) / sizeof(embedded[0]); s != e; ++s)
    {
        if (!strcmp(s->name, name))
        {
            lua_pushlstring(L, s->data, s->sz);
            lua_pushboolean(L, s->t);
            return 2;
        }
    }
    return 0;
}

static int msghandler(lua_State *L)
{
    const char *msg = lua_tostring(L, 1);
    if (msg == NULL)
    {
        if (luaL_callmeta(L, 1, "__tostring") && lua_type(L, -1) == LUA_TSTRING)
            return 1;
        msg = lua_pushfstring(L, "(error object is a %s value)", luaL_typename(L, 1));
    }
    luaL_traceback(L, L, msg, 1);
    return 1;
}

int main(int argc, char *argv[])
{
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    luaL_requiref(L, "lpeg", luaopen_lpeg, 1); lua_pop(L, 1);
    luaL_requiref(L, "lfs", luaopen_lfs, 1); lua_pop(L, 1);

    // preload embedded lua scripts
    luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
    luaL_loadbufferx(L, script_6502_lua, sizeof(script_6502_lua), "6502.lua", "b");
    lua_setfield(L, -2, "6502");
    lua_pop(L, 1);

    // error handler
    lua_pushcfunction(L, msghandler);
    // l65.lua script
    luaL_loadbufferx(L, script_l65_lua, sizeof(script_l65_lua), "l65.lua", "b");
    // arg[] table
    lua_createtable(L, argc-1, 2);
    lua_pushcfunction(L, getembedded); // pass embedded script lookup function as arg[-1]
    lua_rawseti(L, -2, -1);
    for (int i = 0; i < argc; i++) lua_pushstring(L, argv[i]), lua_rawseti(L, -2, i);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "arg");
    // ... arguments
    { int i; for (i = 1; i < argc; ++i) lua_rawgeti(L, -i, i); lua_remove(L, -i); }
    // call l65
    int status = lua_pcall(L, argc-1, 0, -argc-1);
    if (status != LUA_OK)
    {
        const char *msg = lua_tostring(L, -1);
        fprintf(stderr, "%s\n", msg);
        lua_pop(L, 1);
    }
    lua_pop(L, 1); // remove msghandler
    lua_close(L);
    return status;
}

