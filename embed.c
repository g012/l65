#define LUA_IMPLEMENTATION
#include "lua.h"

#define PROGNAME	"embed"		/* default program name */
#define OUTPUT		"scripts.h"	/* default output file */

static int dumping = 1;			/* dump bytecodes? */
static int stripping = 0;			/* strip debug information? */
static char Output[] = { OUTPUT };	/* default output file name */
static const char* output = Output;	/* actual output file name */
static const char* progname = PROGNAME;	/* actual program name */

static void fatal(const char* message)
{
    fprintf(stderr, "%s: %s\n", progname, message);
    exit(EXIT_FAILURE);
}

static void cannot(const char* what)
{
    fprintf(stderr, "%s: cannot %s %s: %s\n", progname, what, output, strerror(errno));
    exit(EXIT_FAILURE);
}

static void usage(const char* message)
{
    if (*message == '-')
        fprintf(stderr, "%s: unrecognized option '%s'\n", progname, message);
    else
        fprintf(stderr, "%s: %s\n", progname, message);
    fprintf(stderr,
        "usage: %s [options] [filenames]\n"
        "Available options are:\n"
        "  -o name  output to file 'name' (default is \"%s\")\n"
        , progname, Output);
    exit(EXIT_FAILURE);
}

#define IS(s)	(strcmp(argv[i],s)==0)

static int doargs(int argc, char* argv[])
{
    int i;
    if (argv[0] != NULL && *argv[0] != 0) progname = argv[0];
    for (i = 1; i < argc; i++)
    {
        if (*argv[i] != '-')			/* end of options; keep it */
            break;
        else if (IS("--"))			/* end of options; skip it */
        {
            ++i;
            break;
        }
        else if (IS("-"))			/* end of options; use stdin */
            break;
        else if (IS("-o"))			/* output file */
        {
            output = argv[++i];
            if (output == NULL || *output == 0 || (*output == '-' && output[1] != 0))
                usage("'-o' needs argument");
            if (IS("-")) output = NULL;
        }
        else					/* unknown option */
            usage(argv[i]);
    }
    return i;
}

#define FUNCTION "(function()end)();"

static size_t w_o;
static int writer(lua_State* L, const void* p, size_t size, void* f)
{
    for (size_t i = 0; i < size; ++i)
    {
        if ((i+w_o) % 32 == 0) fprintf(f, "\n");
        fprintf(f, "0x%02X, ", (int)(((unsigned char*)p)[i]));
    }
    w_o += size;
    return 0;
}

// use custom version so that filename is not used for debug info
LUALIB_API int luaL_loadfilex2 (lua_State *L, const char *filename,
        const char *mode, const char *chunkname) {
    luai_LoadF lf;
    int status, readstatus;
    int c;
    int fnameindex = lua_gettop(L) + 1;  /* index of filename on the stack */
    if (filename == NULL) {
        lua_pushliteral(L, "=stdin");
        lf.f = stdin;
    }
    else {
        lua_pushfstring(L, "@%s", filename);
        lf.f = fopen(filename, "r");
        if (lf.f == NULL) return luai_errfile(L, "open", fnameindex);
    }
    if (luai_skipcomment(&lf, &c))  /* read initial portion */
        lf.buff[lf.n++] = '\n';  /* add line to correct line numbers */
    if (c == LUA_SIGNATURE[0] && filename) {  /* binary file? */
        lf.f = freopen(filename, "rb", lf.f);  /* reopen in binary mode */
        if (lf.f == NULL) return luai_errfile(L, "reopen", fnameindex);
        luai_skipcomment(&lf, &c);  /* re-read initial portion */
    }
    if (c != EOF)
        lf.buff[lf.n++] = c;  /* 'c' is the first character of the stream */
    lua_pop(L, 1);
    lua_pushstring(L, chunkname);
    status = lua_load(L, luai_getF, &lf, lua_tostring(L, -1), mode);
    readstatus = ferror(lf.f);
    if (filename) fclose(lf.f);  /* close file (even in case of errors) */
    if (readstatus) {
        lua_settop(L, fnameindex);  /* ignore results from 'lua_load' */
        return luai_errfile(L, "read", fnameindex);
    }
    lua_remove(L, fnameindex);
    return status;
}

static int pmain(lua_State* L)
{
    int argc = (int)lua_tointeger(L, 1);
    char** argv = (char**)lua_touserdata(L, 2);
    int i;
    if (!lua_checkstack(L, argc)) fatal("too many input files");
    FILE* f = (output == NULL) ? stdout : fopen(output, "wb");
    if (f == NULL) cannot("open");
    for (i = 0; i < argc; i++)
    {
        int compile = 0;
        const char* filename = IS("-") ? NULL : argv[i];
        if (!filename) fatal("invalid input file");
        char *ufname = strdup(filename);
        int fnix = 0; for (int i = 0; ufname[i]; ++i) { if (ufname[i] == '/' || ufname[i] == '\\') fnix = i + 1; if (ufname[i] == '.') ufname[i] = '_'; }
        char *name = malloc(1024);
        sprintf(name, "script_%s", ufname + fnix);
        free(ufname);
        int fnl = (int)strlen(filename);
        compile = fnl > 4 && !strcmp(".lua", filename + fnl - 4);
        if (compile)
        {
            // do no use luaL_loadfile so that the filename is not used for error printing
            char *p = strdup(filename), *n = p;
            for (p = p + strlen(p); p != n && *p != '/' && *p != '\\'; --p) ;
            if (p != n) ++p;
            char *chunkname = malloc(strlen(p) + 16);
            strcpy(chunkname, "=(");
            strcat(chunkname, p);
            strcat(chunkname, ")");
            free(n);
            if (luaL_loadfilex2(L, filename, 0, chunkname) != LUA_OK) fatal(lua_tostring(L, -1));
            free(chunkname);
            fprintf(f, "static const char %s[] = {", name);
            w_o = 0;
            lua_dump(L, writer, f, 0);
            lua_pop(L, 1);
            fprintf(f, "\n};\n");
        }
        else
        {
            FILE* l = fopen(filename, "rb"); if (!l) cannot("open");
            fseek(l, 0, SEEK_END);
            int size = (int)ftell(l);
            rewind(l);
            unsigned char *buffer = malloc(size);
            if (size != (int)fread(buffer, 1, size, l)) fatal("failed reading input file");
            fclose(l);
            fprintf(f, "static const char %s[] = {", name);
            for (int i = 0; i < size; ++i)
            {
                if (i % 32 == 0) fprintf(f, "\n");
                fprintf(f, "0x%02X, ", (int)buffer[i]);
            }
            fprintf(f, "\n};\n");
            free(buffer);
        }
        free(name);
    }
    if (ferror(f)) cannot("write");
    if (fclose(f)) cannot("close");
    return 0;
}

int main(int argc, char* argv[])
{
    lua_State* L;
    int i = doargs(argc, argv);
    argc -= i; argv += i;
    if (argc <= 0) usage("no input files given");
    L = luaL_newstate();
    if (L == NULL) fatal("cannot create state: not enough memory");
    lua_pushcfunction(L, &pmain);
    lua_pushinteger(L, argc);
    lua_pushlightuserdata(L, argv);
    if (lua_pcall(L, 2, 0, 0) != LUA_OK) fatal(lua_tostring(L, -1));
    lua_close(L);
    return EXIT_SUCCESS;
}

