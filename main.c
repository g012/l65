#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_FAILURE_STRINGS
#include "stb_image.h"

#define LUA_IMPLEMENTATION
#include "lua.h"
#include "scripts.h"

extern int luaopen_lpeg(lua_State *L);
extern int luaopen_lfs(lua_State *L);

// l65 lib
static int r_s32be(uint8_t **b) { uint8_t *p = *b; int v = ((int)(p[0]))<<24 | ((int)(p[1]))<<16 | ((int)p[2])<<8 | p[3]; *b += 4; return v; }
typedef struct { int len, nam; } chunk_s;
static chunk_s r_chunk(uint8_t **b) { int len = r_s32be(b), nam = r_s32be(b); chunk_s c = { len, nam }; return c; }
static int open_image(lua_State *L)
{
    const char *filename = luaL_checkstring(L, 1);
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        lua_pushnil(L);
        lua_pushfstring(L, "failed to open file %s", filename);
        return 2;
    }
    fseek(file, 0, SEEK_END);
    size_t sz = ftell(file);
    fseek(file, 0, SEEK_SET);
    uint8_t *png = malloc(sz);
    fread(png, sz, 1, file);
    fclose(file);
    static uint8_t png_sig[8] = { 137,80,78,71,13,10,26,10 };
    if (memcmp(png, png_sig, 8) != 0)
    {
        free(png);
        lua_pushnil(L);
        lua_pushfstring(L, "file %s is not a PNG", filename);
        return 2;
    }
    uint8_t *b = png + 8;
    int w, h;
    uint8_t *d = 0; long d_sz = 0;
#define CHUNK_NAM(a,b,c,d) (((a) << 24) + ((b) << 16) + ((c) << 8) + (d))
    for (;;)
    {
        chunk_s chunk = r_chunk(&b);
        switch (chunk.nam)
        {
            case CHUNK_NAM('I','H','D','R'): {
                w = r_s32be(&b); h = r_s32be(&b);
                if (b[0] != 8 || b[1] != 3)
                {
                    free(png);
                    lua_pushnil(L);
                    lua_pushfstring(L, "PNG file %s must be 8b indexed", filename);
                    return 2;
                }
                b += 9;
            } break;
            case CHUNK_NAM('I','D','A','T'): {
                d = realloc(d, d_sz + chunk.len);
                memcpy(d + d_sz, b, chunk.len);
                d_sz += chunk.len;
                b += chunk.len+4;
            } break;
            case CHUNK_NAM('I','E','N','D'): {
                free(png);
                if (!d)
                {
                    lua_pushnil(L);
                    lua_pushfstring(L, "invalid PNG file %s", filename);
                    return 2;
                }
                int px_sz;
                uint8_t *px_raw = (uint8_t*)stbi_zlib_decode_malloc_guesssize_headerflag((void*)d, d_sz, (w+1) * h, &px_sz, 1);
                free(d);
                uint8_t *px = calloc(w,h);
                uint8_t *px0 = px, *px_raw0 = px_raw;
                for (int y = 0; y < h; ++y)
                {
                    int filter = *px_raw++;
                    #define prev (x==0 ? 0 : px[x-1])
                    #define up (px[x-w])
                    #define prevup (x==0 ? 0 : px[x-w-1])
                    switch (filter)
                    {
                        case 0: memcpy(px, px_raw, w); break;
                        case 1: for (int x = 0; x < w; ++x) { px[x] = px_raw[x] + prev; } break;
                        case 2: for (int x = 0; x < w; ++x) { px[x] = px_raw[x] + up; } break;
                        case 3: for (int x = 0; x < w; ++x) { px[x] = px_raw[x] + ((prev+up)>>1); } break;
                        case 4: for (int x = 0; x < w; ++x) { px[x] = px_raw[x] + stbi__paeth(prev,up,prevup); } break;
                    }
                    #undef prev
                    #undef up
                    #undef prevup
                    px += w;
                    px_raw += w;
                }
                STBI_FREE(px_raw0);

                lua_createtable(L, w*h, 3);
                lua_pushstring(L, filename);
                lua_setfield(L, -2, "filename");
                lua_pushinteger(L, w);
                lua_setfield(L, -2, "width");
                lua_pushinteger(L, h);
                lua_setfield(L, -2, "height");
                for (int i = 0; i < w*h; ++i)
                {
                    lua_pushinteger(L, px0[i]);
                    lua_rawseti(L, -2, i+1);
                }
                free(px0);
                return 1;
            }
            default:
                b += chunk.len+4;
        }
    }
#undef CHUNK_NAM
    if (d) free(d);
    free(png);
    lua_pushnil(L);
    lua_pushfstring(L, "invalid PNG file %s", filename);
    return 2;
}
static const struct luaL_Reg l65lib[] = {
    {"image", open_image},
    {NULL, NULL},
};
static int luaopen_l65(lua_State *L)
{
    luaL_newlib(L, l65lib);
    return 1;
}

#define SRC_LUA(name) { #name, 0, script_ ## name ## _lua, sizeof(script_ ## name ## _lua) }
#define SRC_L65(name) { #name, 1, script_ ## name ## _l65, sizeof(script_ ## name ## _l65) }
static struct script { const char *name; int t;  const char *data; size_t sz; } embedded[] = {
    SRC_LUA(dkjson),
    SRC_LUA(l65cfg),
    SRC_LUA(re),
    SRC_L65(nes),
    SRC_L65(vcs),
};
#undef SRC_LUA
#undef SRC_L65

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
    luaL_requiref(L, "l65", luaopen_l65, 1); lua_pop(L, 1);

    // preload embedded lua scripts
    luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
    luaL_loadbufferx(L, script_asm_lua, sizeof(script_asm_lua), "asm.lua", "b");
    lua_setfield(L, -2, "asm");
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

