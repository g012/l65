/*
 * Lua 5.3.4 as one file, STB style.
 * Options from Makefile: LUA_USE_LINUX, LUA_USE_MACOSX, LUA_USE_POSIX, LUA_USE_DLOPEN, LUA_USE_C89
 * Add extern "C" before including if needed.
 */

/******************************************************************************
* Copyright (C) 1994-2017 Lua.org, PUC-Rio.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/

#ifdef LUA_IMPLEMENTATION
#define LUA_CORE

/*__lprefix.h__*/

/*
** Allows POSIX/XSI stuff
*/
#if !defined(LUA_USE_C89)	/* { */

#if !defined(_XOPEN_SOURCE)
#define _XOPEN_SOURCE           600
#elif _XOPEN_SOURCE == 0
#undef _XOPEN_SOURCE  /* use -D_XOPEN_SOURCE=0 to undefine it */
#endif

/*
** Allows manipulation of large files in gcc and some other compilers
*/
#if !defined(LUA_32BITS) && !defined(_FILE_OFFSET_BITS)
#define _LARGEFILE_SOURCE       1
#define _FILE_OFFSET_BITS       64
#endif

#endif				/* } */


/*
** Windows stuff
*/
#if defined(_WIN32) 	/* { */

#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS  /* avoid warnings about ISO C functions */
#endif

#endif			/* } */

#endif

/***************************************************************************
 * HEADER
 **************************************************************************/

#ifndef lua_h
#define lua_h

#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

/*__luaconf.h__*/

/*
** ===================================================================
** Search for "@@" to find all configurable definitions.
** ===================================================================
*/


/*
** {====================================================================
** System Configuration: macros to adapt (if needed) Lua to some
** particular platform, for instance compiling it with 32-bit numbers or
** restricting it to C89.
** =====================================================================
*/

/*
@@ LUA_32BITS enables Lua with 32-bit integers and 32-bit floats. You
** can also define LUA_32BITS in the make file, but changing here you
** ensure that all software connected to Lua will be compiled with the
** same configuration.
*/
/* #define LUA_32BITS */


/*
@@ LUA_USE_C89 controls the use of non-ISO-C89 features.
** Define it if you want Lua to avoid the use of a few C99 features
** or Windows-specific features on Windows.
*/
/* #define LUA_USE_C89 */


/*
** By default, Lua on Windows use (some) specific Windows features
*/
#if !defined(LUA_USE_C89) && defined(_WIN32) && !defined(_WIN32_WCE)
#define LUA_USE_WINDOWS  /* enable goodies for regular Windows */
#endif


#if defined(LUA_USE_WINDOWS)
#define LUA_DL_DLL	/* enable support for DLL */
#define LUA_USE_C89	/* broadly, Windows is C89 */
#endif


#if defined(LUA_USE_LINUX)
#define LUA_USE_POSIX
#define LUA_USE_DLOPEN		/* needs an extra library: -ldl */
#define LUA_USE_READLINE	/* needs some extra libraries */
#endif


#if defined(LUA_USE_MACOSX)
#define LUA_USE_POSIX
#define LUA_USE_DLOPEN		/* MacOS does not need -ldl */
#define LUA_USE_READLINE	/* needs an extra library: -lreadline */
#endif


/*
@@ LUA_C89_NUMBERS ensures that Lua uses the largest types available for
** C89 ('long' and 'double'); Windows always has '__int64', so it does
** not need to use this case.
*/
#if defined(LUA_USE_C89) && !defined(LUA_USE_WINDOWS)
#define LUA_C89_NUMBERS
#endif



/*
@@ LUAI_BITSINT defines the (minimum) number of bits in an 'int'.
*/
/* avoid undefined shifts */
#if ((INT_MAX >> 15) >> 15) >= 1
#define LUAI_BITSINT	32
#else
/* 'int' always must have at least 16 bits */
#define LUAI_BITSINT	16
#endif


/*
@@ LUA_INT_TYPE defines the type for Lua integers.
@@ LUA_FLOAT_TYPE defines the type for Lua floats.
** Lua should work fine with any mix of these options (if supported
** by your C compiler). The usual configurations are 64-bit integers
** and 'double' (the default), 32-bit integers and 'float' (for
** restricted platforms), and 'long'/'double' (for C compilers not
** compliant with C99, which may not have support for 'long long').
*/

/* predefined options for LUA_INT_TYPE */
#define LUA_INT_INT		1
#define LUA_INT_LONG		2
#define LUA_INT_LONGLONG	3

/* predefined options for LUA_FLOAT_TYPE */
#define LUA_FLOAT_FLOAT		1
#define LUA_FLOAT_DOUBLE	2
#define LUA_FLOAT_LONGDOUBLE	3

#if defined(LUA_32BITS)		/* { */
/*
** 32-bit integers and 'float'
*/
#if LUAI_BITSINT >= 32  /* use 'int' if big enough */
#define LUA_INT_TYPE	LUA_INT_INT
#else  /* otherwise use 'long' */
#define LUA_INT_TYPE	LUA_INT_LONG
#endif
#define LUA_FLOAT_TYPE	LUA_FLOAT_FLOAT

#elif defined(LUA_C89_NUMBERS)	/* }{ */
/*
** largest types available for C89 ('long' and 'double')
*/
#define LUA_INT_TYPE	LUA_INT_LONG
#define LUA_FLOAT_TYPE	LUA_FLOAT_DOUBLE

#endif				/* } */


/*
** default configuration for 64-bit Lua ('long long' and 'double')
*/
#if !defined(LUA_INT_TYPE)
#define LUA_INT_TYPE	LUA_INT_LONGLONG
#endif

#if !defined(LUA_FLOAT_TYPE)
#define LUA_FLOAT_TYPE	LUA_FLOAT_DOUBLE
#endif

/* }================================================================== */




/*
** {==================================================================
** Configuration for Paths.
** ===================================================================
*/

/*
** LUA_PATH_SEP is the character that separates templates in a path.
** LUA_PATH_MARK is the string that marks the substitution points in a
** template.
** LUA_EXEC_DIR in a Windows path is replaced by the executable's
** directory.
*/
#define LUA_PATH_SEP            ";"
#define LUA_PATH_MARK           "?"
#define LUA_EXEC_DIR            "!"


/*
@@ LUA_PATH_DEFAULT is the default path that Lua uses to look for
** Lua libraries.
@@ LUA_CPATH_DEFAULT is the default path that Lua uses to look for
** C libraries.
** CHANGE them if your machine has a non-conventional directory
** hierarchy or if you want to install your libraries in
** non-conventional directories.
*/
#define LUA_VDIR	LUA_VERSION_MAJOR "." LUA_VERSION_MINOR
#if defined(_WIN32)	/* { */
/*
** In Windows, any exclamation mark ('!') in the path is replaced by the
** path of the directory of the executable file of the current process.
*/
#define LUA_LDIR	"!\\lua\\"
#define LUA_CDIR	"!\\"
#define LUA_SHRDIR	"!\\..\\share\\lua\\" LUA_VDIR "\\"
#define LUA_PATH_DEFAULT  \
		LUA_LDIR"?.lua;"  LUA_LDIR"?\\init.lua;" \
		LUA_CDIR"?.lua;"  LUA_CDIR"?\\init.lua;" \
		LUA_SHRDIR"?.lua;" LUA_SHRDIR"?\\init.lua;" \
		".\\?.lua;" ".\\?\\init.lua"
#define LUA_CPATH_DEFAULT \
		LUA_CDIR"?.dll;" \
		LUA_CDIR"..\\lib\\lua\\" LUA_VDIR "\\?.dll;" \
		LUA_CDIR"loadall.dll;" ".\\?.dll"

#else			/* }{ */

#define LUA_ROOT	"/usr/local/"
#define LUA_LDIR	LUA_ROOT "share/lua/" LUA_VDIR "/"
#define LUA_CDIR	LUA_ROOT "lib/lua/" LUA_VDIR "/"
#define LUA_PATH_DEFAULT  \
		LUA_LDIR"?.lua;"  LUA_LDIR"?/init.lua;" \
		LUA_CDIR"?.lua;"  LUA_CDIR"?/init.lua;" \
		"./?.lua;" "./?/init.lua"
#define LUA_CPATH_DEFAULT \
		LUA_CDIR"?.so;" LUA_CDIR"loadall.so;" "./?.so"
#endif			/* } */


/*
@@ LUA_DIRSEP is the directory separator (for submodules).
** CHANGE it if your machine does not use "/" as the directory separator
** and is not Windows. (On Windows Lua automatically uses "\".)
*/
#if defined(_WIN32)
#define LUA_DIRSEP	"\\"
#else
#define LUA_DIRSEP	"/"
#endif

/* }================================================================== */


/*
** {==================================================================
** Marks for exported symbols in the C code
** ===================================================================
*/

/*
@@ LUA_API is a mark for all core API functions.
@@ LUALIB_API is a mark for all auxiliary library functions.
@@ LUAMOD_API is a mark for all standard library opening functions.
** CHANGE them if you need to define those functions in some special way.
** For instance, if you want to create one Windows DLL with the core and
** the libraries, you may want to use the following definition (define
** LUA_BUILD_AS_DLL to get it).
*/
#if defined(LUA_BUILD_AS_DLL)	/* { */

#if defined(LUA_CORE) || defined(LUA_LIB)	/* { */
#define LUA_API __declspec(dllexport)
#else						/* }{ */
#define LUA_API __declspec(dllimport)
#endif						/* } */

#else				/* }{ */

#define LUA_API		extern

#endif				/* } */


/* more often than not the libs go together with the core */
#define LUALIB_API	LUA_API
#define LUAMOD_API	LUALIB_API


/*
@@ LUAI_FUNC is a mark for all extern functions that are not to be
** exported to outside modules.
@@ LUAI_DDEF and LUAI_DDEC are marks for all extern (const) variables
** that are not to be exported to outside modules (LUAI_DDEF for
** definitions and LUAI_DDEC for declarations).
** CHANGE them if you need to mark them in some special way. Elf/gcc
** (versions 3.2 and later) mark them as "hidden" to optimize access
** when Lua is compiled as a shared library. Not all elf targets support
** this attribute. Unfortunately, gcc does not offer a way to luai_check
** whether the target offers that support, and those without support
** give a warning about it. To avoid these warnings, change to the
** default definition.
*/
#if defined(__GNUC__) && ((__GNUC__*100 + __GNUC_MINOR__) >= 302) && \
    defined(__ELF__)		/* { */
#define LUAI_FUNC	__attribute__((visibility("hidden"))) extern
#else				/* }{ */
#define LUAI_FUNC	extern
#endif				/* } */

#define LUAI_DDEC	LUAI_FUNC
#define LUAI_DDEF	/* empty */

/* }================================================================== */


/*
** {==================================================================
** Compatibility with previous versions
** ===================================================================
*/

/*
@@ LUA_COMPAT_5_2 controls other macros for compatibility with Lua 5.2.
@@ LUA_COMPAT_5_1 controls other macros for compatibility with Lua 5.1.
** You can define it to get all options, or change specific options
** to fit your specific needs.
*/
#if defined(LUA_COMPAT_5_2)	/* { */

/*
@@ LUA_COMPAT_MATHLIB controls the presence of several deprecated
** functions in the mathematical library.
*/
#define LUA_COMPAT_MATHLIB

/*
@@ LUA_COMPAT_BITLIB controls the presence of library 'bit32'.
*/
#define LUA_COMPAT_BITLIB

/*
@@ LUA_COMPAT_IPAIRS controls the effectiveness of the __ipairs metamethod.
*/
#define LUA_COMPAT_IPAIRS

/*
@@ LUA_COMPAT_APIINTCASTS controls the presence of macros for
** manipulating other integer types (lua_pushunsigned, lua_tounsigned,
** luaL_checkint, luaL_checklong, etc.)
*/
#define LUA_COMPAT_APIINTCASTS

#endif				/* } */


#if defined(LUA_COMPAT_5_1)	/* { */

/* Incompatibilities from 5.2 -> 5.3 */
#define LUA_COMPAT_MATHLIB
#define LUA_COMPAT_APIINTCASTS

/*
@@ LUA_COMPAT_UNPACK controls the presence of global 'luai_unpack'.
** You can replace it with 'table.luai_unpack'.
*/
#define LUA_COMPAT_UNPACK

/*
@@ LUA_COMPAT_LOADERS controls the presence of table 'package.loaders'.
** You can replace it with 'package.searchers'.
*/
#define LUA_COMPAT_LOADERS

/*
@@ macro 'lua_cpcall' emulates deprecated function lua_cpcall.
** You can call your C function directly (with light C functions).
*/
#define lua_cpcall(L,f,u)  \
	(lua_pushcfunction(L, (f)), \
	 lua_pushlightuserdata(L,(u)), \
	 lua_pcall(L,1,0,0))


/*
@@ LUA_COMPAT_LOG10 defines the function 'log10' in the math library.
** You can rewrite 'log10(x)' as 'log(x, 10)'.
*/
#define LUA_COMPAT_LOG10

/*
@@ LUA_COMPAT_LOADSTRING defines the function 'loadstring' in the base
** library. You can rewrite 'loadstring(s)' as 'load(s)'.
*/
#define LUA_COMPAT_LOADSTRING

/*
@@ LUA_COMPAT_MAXN defines the function 'luai_maxn' in the table library.
*/
#define LUA_COMPAT_MAXN

/*
@@ The following macros supply trivial compatibility for some
** changes in the API. The macros themselves document how to
** change your code to avoid using them.
*/
#define lua_strlen(L,i)		lua_rawlen(L, (i))

#define lua_objlen(L,i)		lua_rawlen(L, (i))

#define lua_equal(L,idx1,idx2)		lua_compare(L,(idx1),(idx2),LUA_OPEQ)
#define lua_lessthan(L,idx1,idx2)	lua_compare(L,(idx1),(idx2),LUA_OPLT)

/*
@@ LUA_COMPAT_MODULE controls compatibility with previous
** module functions 'module' (Lua) and 'luaL_register' (C).
*/
#define LUA_COMPAT_MODULE

#endif				/* } */


/*
@@ LUA_COMPAT_FLOATSTRING makes Lua format integral floats without a
@@ a float mark ('.0').
** This macro is not on by default even in compatibility mode,
** because this is not really an incompatibility.
*/
/* #define LUA_COMPAT_FLOATSTRING */

/* }================================================================== */



/*
** {==================================================================
** Configuration for Numbers.
** Change these definitions if no predefined LUA_FLOAT_* / LUA_INT_*
** satisfy your needs.
** ===================================================================
*/

/*
@@ LUA_NUMBER is the floating-point type used by Lua.
@@ LUAI_UACNUMBER is the result of a 'default argument promotion'
@@ over a floating number.
@@ luai_l_mathlim(x) corrects limit name 'x' to the proper float type
** by prefixing it with one of FLT/DBL/LDBL.
@@ LUA_NUMBER_FRMLEN is the length modifier for writing floats.
@@ LUA_NUMBER_FMT is the format for writing floats.
@@ lua_number2str converts a float to a string.
@@ luai_l_mathop allows the addition of an 'l' or 'f' to all math operations.
@@ luai_l_floor takes the floor of a float.
@@ lua_str2number converts a decimal numeric string to a number.
*/


/* The following definitions are good for most cases here */

#define luai_l_floor(x)		(luai_l_mathop(floor)(x))

#define lua_number2str(s,sz,n)  \
	luai_l_sprintf((s), sz, LUA_NUMBER_FMT, (LUAI_UACNUMBER)(n))

/*
@@ lua_numbertointeger converts a float number to an integer, or
** returns 0 if float is not within the range of a lua_Integer.
** (The range comparisons are tricky because of rounding. The tests
** here assume a two-complement representation, where MININTEGER always
** has an exact representation as a float; MAXINTEGER may not have one,
** and therefore its conversion to float may have an ill-defined value.)
*/
#define lua_numbertointeger(n,p) \
  ((n) >= (LUA_NUMBER)(LUA_MININTEGER) && \
   (n) < -(LUA_NUMBER)(LUA_MININTEGER) && \
      (*(p) = (LUA_INTEGER)(n), 1))


/* now the variable definitions */

#if LUA_FLOAT_TYPE == LUA_FLOAT_FLOAT		/* { single float */

#define LUA_NUMBER	float

#define luai_l_mathlim(n)		(FLT_##n)

#define LUAI_UACNUMBER	double

#define LUA_NUMBER_FRMLEN	""
#define LUA_NUMBER_FMT		"%.7g"

#define luai_l_mathop(op)		op##f

#define lua_str2number(s,p)	strtof((s), (p))


#elif LUA_FLOAT_TYPE == LUA_FLOAT_LONGDOUBLE	/* }{ long double */

#define LUA_NUMBER	long double

#define luai_l_mathlim(n)		(LDBL_##n)

#define LUAI_UACNUMBER	long double

#define LUA_NUMBER_FRMLEN	"L"
#define LUA_NUMBER_FMT		"%.19Lg"

#define luai_l_mathop(op)		op##l

#define lua_str2number(s,p)	strtold((s), (p))

#elif LUA_FLOAT_TYPE == LUA_FLOAT_DOUBLE	/* }{ double */

#define LUA_NUMBER	double

#define luai_l_mathlim(n)		(DBL_##n)

#define LUAI_UACNUMBER	double

#define LUA_NUMBER_FRMLEN	""
#define LUA_NUMBER_FMT		"%.14g"

#define luai_l_mathop(op)		op

#define lua_str2number(s,p)	strtod((s), (p))

#else						/* }{ */

#error "numeric float type not defined"

#endif					/* } */



/*
@@ LUA_INTEGER is the integer type used by Lua.
**
@@ LUA_UNSIGNED is the unsigned version of LUA_INTEGER.
**
@@ LUAI_UACINT is the result of a 'default argument promotion'
@@ over a lUA_INTEGER.
@@ LUA_INTEGER_FRMLEN is the length modifier for reading/writing integers.
@@ LUA_INTEGER_FMT is the format for writing integers.
@@ LUA_MAXINTEGER is the maximum value for a LUA_INTEGER.
@@ LUA_MININTEGER is the minimum value for a LUA_INTEGER.
@@ lua_integer2str converts an integer to a string.
*/


/* The following definitions are good for most cases here */

#define LUA_INTEGER_FMT		"%" LUA_INTEGER_FRMLEN "d"

#define LUAI_UACINT		LUA_INTEGER

#define lua_integer2str(s,sz,n)  \
	luai_l_sprintf((s), sz, LUA_INTEGER_FMT, (LUAI_UACINT)(n))

/*
** use LUAI_UACINT here to avoid problems with promotions (which
** can turn a comparison between unsigneds into a signed comparison)
*/
#define LUA_UNSIGNED		unsigned LUAI_UACINT


/* now the variable definitions */

#if LUA_INT_TYPE == LUA_INT_INT		/* { int */

#define LUA_INTEGER		int
#define LUA_INTEGER_FRMLEN	""

#define LUA_MAXINTEGER		INT_MAX
#define LUA_MININTEGER		INT_MIN

#elif LUA_INT_TYPE == LUA_INT_LONG	/* }{ long */

#define LUA_INTEGER		long
#define LUA_INTEGER_FRMLEN	"l"

#define LUA_MAXINTEGER		LONG_MAX
#define LUA_MININTEGER		LONG_MIN

#elif LUA_INT_TYPE == LUA_INT_LONGLONG	/* }{ long long */

/* use presence of macro LLONG_MAX as proxy for C99 compliance */
#if defined(LLONG_MAX)		/* { */
/* use ISO C99 stuff */

#define LUA_INTEGER		long long
#define LUA_INTEGER_FRMLEN	"ll"

#define LUA_MAXINTEGER		LLONG_MAX
#define LUA_MININTEGER		LLONG_MIN

#elif defined(LUA_USE_WINDOWS) /* }{ */
/* in Windows, can use specific Windows types */

#define LUA_INTEGER		__int64
#define LUA_INTEGER_FRMLEN	"I64"

#define LUA_MAXINTEGER		_I64_MAX
#define LUA_MININTEGER		_I64_MIN

#else				/* }{ */

#error "Compiler does not support 'long long'. Use option '-DLUA_32BITS' \
  or '-DLUA_C89_NUMBERS' (see file 'luaconf.h' for details)"

#endif				/* } */

#else				/* }{ */

#error "numeric integer type not defined"

#endif				/* } */

/* }================================================================== */


/*
** {==================================================================
** Dependencies with C99 and other C details
** ===================================================================
*/

/*
@@ luai_l_sprintf is equivalent to 'snprintf' or 'sprintf' in C89.
** (All uses in Lua have only one format item.)
*/
#if !defined(LUA_USE_C89)
#define luai_l_sprintf(s,sz,f,i)	snprintf(s,sz,f,i)
#else
#define luai_l_sprintf(s,sz,f,i)	((void)(sz), sprintf(s,f,i))
#endif


/*
@@ lua_strx2number converts an hexadecimal numeric string to a number.
** In C99, 'strtod' does that conversion. Otherwise, you can
** leave 'lua_strx2number' undefined and Lua will provide its own
** implementation.
*/
#if !defined(LUA_USE_C89)
#define lua_strx2number(s,p)		lua_str2number(s,p)
#endif


/*
@@ lua_number2strx converts a float to an hexadecimal numeric string.
** In C99, 'sprintf' (with format specifiers '%a'/'%A') does that.
** Otherwise, you can leave 'lua_number2strx' undefined and Lua will
** provide its own implementation.
*/
#if !defined(LUA_USE_C89)
#define lua_number2strx(L,b,sz,f,n)  \
	((void)L, luai_l_sprintf(b,sz,f,(LUAI_UACNUMBER)(n)))
#endif


/*
** 'strtof' and 'opf' variants for math functions are not valid in
** C89. Otherwise, the macro 'HUGE_VALF' is a good proxy for testing the
** availability of these variants. ('math.h' is already included in
** all files that use these macros.)
*/
#if defined(LUA_USE_C89) || (defined(HUGE_VAL) && !defined(HUGE_VALF))
#undef luai_l_mathop  /* variants not available */
#undef lua_str2number
#define luai_l_mathop(op)		(lua_Number)op  /* no variant */
#define lua_str2number(s,p)	((lua_Number)strtod((s), (p)))
#endif


/*
@@ LUA_KCONTEXT is the type of the context ('ctx') for continuation
** functions.  It must be a numerical type; Lua will use 'intptr_t' if
** available, otherwise it will use 'ptrdiff_t' (the nearest thing to
** 'intptr_t' in C89)
*/
#define LUA_KCONTEXT	ptrdiff_t

#if !defined(LUA_USE_C89) && defined(__STDC_VERSION__) && \
    __STDC_VERSION__ >= 199901L
#include <stdint.h>
#if defined(INTPTR_MAX)  /* even in C99 this type is optional */
#undef LUA_KCONTEXT
#define LUA_KCONTEXT	intptr_t
#endif
#endif


/*
@@ lua_getlocaledecpoint gets the locale "radix character" (decimal point).
** Change that if you do not want to use C locales. (Code using this
** macro must include header 'locale.h'.)
*/
#if !defined(lua_getlocaledecpoint)
#define lua_getlocaledecpoint()		(localeconv()->decimal_point[0])
#endif

/* }================================================================== */


/*
** {==================================================================
** Language Variations
** =====================================================================
*/

/*
@@ LUA_NOCVTN2S/LUA_NOCVTS2N control how Lua performs some
** coercions. Define LUA_NOCVTN2S to turn off automatic coercion from
** numbers to strings. Define LUA_NOCVTS2N to turn off automatic
** coercion from strings to numbers.
*/
/* #define LUA_NOCVTN2S */
/* #define LUA_NOCVTS2N */


/*
@@ LUA_USE_APICHECK turns on several consistency checks on the C API.
** Define it as a help when debugging C code.
*/
#if defined(LUA_USE_APICHECK)
#include <assert.h>
#define luai_apicheck(l,e)	assert(e)
#endif

/* }================================================================== */


/*
** {==================================================================
** Macros that affect the API and must be stable (that is, must be the
** same when you compile Lua and when you compile code that links to
** Lua). You probably do not want/need to change them.
** =====================================================================
*/

/*
@@ LUAI_MAXSTACK limits the size of the Lua stack.
** CHANGE it if you need a different limit. This limit is arbitrary;
** its only purpose is to stop Lua from consuming unlimited stack
** space (and to reserve some numbers for pseudo-indices).
*/
#if LUAI_BITSINT >= 32
#define LUAI_MAXSTACK		1000000
#else
#define LUAI_MAXSTACK		15000
#endif


/*
@@ LUA_EXTRASPACE defines the size of a raw memory area associated with
** a Lua state with very fast access.
** CHANGE it if you need a different size.
*/
#define LUA_EXTRASPACE		(sizeof(void *))


/*
@@ LUA_IDSIZE gives the maximum size for the description of the source
@@ of a function in debug information.
** CHANGE it if you want a different size.
*/
#define LUA_IDSIZE	60


/*
@@ LUAL_BUFFERSIZE is the buffer size used by the lauxlib buffer system.
** CHANGE it if it uses too much C-stack space. (For long double,
** 'string.format("%.99f", -1e4932)' needs 5034 bytes, so a
** smaller buffer would force a memory allocation for each call to
** 'string.format'.)
*/
#if LUA_FLOAT_TYPE == LUA_FLOAT_LONGDOUBLE
#define LUAL_BUFFERSIZE		8192
#else
#define LUAL_BUFFERSIZE   ((int)(0x80 * sizeof(void*) * sizeof(lua_Integer)))
#endif

/* }================================================================== */


/*
@@ LUA_QL describes how error messages quote program elements.
** Lua does not use these macros anymore; they are here for
** compatibility only.
*/
#define LUA_QL(x)	"'" x "'"
#define LUA_QS		LUA_QL("%s")




/* =================================================================== */

/*
** Local configuration. You can use this space to add your redefinitions
** without modifying the main part of the file.
*/


/*__lua.h__*/

#define LUA_VERSION_MAJOR	"5"
#define LUA_VERSION_MINOR	"3"
#define LUA_VERSION_NUM		503
#define LUA_VERSION_RELEASE	"4"

#define LUA_VERSION	"Lua " LUA_VERSION_MAJOR "." LUA_VERSION_MINOR
#define LUA_RELEASE	LUA_VERSION "." LUA_VERSION_RELEASE
#define LUA_COPYRIGHT	LUA_RELEASE "  Copyright (C) 1994-2017 Lua.org, PUC-Rio"
#define LUA_AUTHORS	"R. Ierusalimschy, L. H. de Figueiredo, W. Celes"


/* mark for precompiled code ('<esc>Lua') */
#define LUA_SIGNATURE	"\x1bLua"

/* option for multiple returns in 'lua_pcall' and 'lua_call' */
#define LUA_MULTRET	(-1)


/*
** Pseudo-indices
** (-LUAI_MAXSTACK is the minimum valid index; we keep some free empty
** space after that to help overflow detection)
*/
#define LUA_REGISTRYINDEX	(-LUAI_MAXSTACK - 1000)
#define lua_upvalueindex(i)	(LUA_REGISTRYINDEX - (i))


/* thread status */
#define LUA_OK		0
#define LUA_YIELD	1
#define LUA_ERRRUN	2
#define LUA_ERRSYNTAX	3
#define LUA_ERRMEM	4
#define LUA_ERRGCMM	5
#define LUA_ERRERR	6


typedef struct lua_State lua_State;


/*
** basic types
*/
#define LUA_TNONE		(-1)

#define LUA_TNIL		0
#define LUA_TBOOLEAN		1
#define LUA_TLIGHTUSERDATA	2
#define LUA_TNUMBER		3
#define LUA_TSTRING		4
#define LUA_TTABLE		5
#define LUA_TFUNCTION		6
#define LUA_TUSERDATA		7
#define LUA_TTHREAD		8

#define LUA_NUMTAGS		9



/* minimum Lua stack available to a C function */
#define LUA_MINSTACK	20


/* predefined values in the registry */
#define LUA_RIDX_MAINTHREAD	1
#define LUA_RIDX_GLOBALS	2
#define LUA_RIDX_LAST		LUA_RIDX_GLOBALS


/* type of numbers in Lua */
typedef LUA_NUMBER lua_Number;


/* type for integer functions */
typedef LUA_INTEGER lua_Integer;

/* unsigned integer type */
typedef LUA_UNSIGNED lua_Unsigned;

/* type for continuation-function contexts */
typedef LUA_KCONTEXT lua_KContext;


/*
** Type for C functions registered with Lua
*/
typedef int (*lua_CFunction) (lua_State *L);

/*
** Type for continuation functions
*/
typedef int (*lua_KFunction) (lua_State *L, int status, lua_KContext ctx);


/*
** Type for functions that read/write blocks when loading/dumping Lua chunks
*/
typedef const char * (*lua_Reader) (lua_State *L, void *ud, size_t *sz);

typedef int (*lua_Writer) (lua_State *L, const void *p, size_t sz, void *ud);


/*
** Type for memory-allocation functions
*/
typedef void * (*lua_Alloc) (void *ud, void *ptr, size_t osize, size_t nsize);



/*
** generic extra include file
*/
#if defined(LUA_USER_H)
#include LUA_USER_H
#endif


/*
** RCS ident string
*/
extern const char lua_ident[];


/*
** state manipulation
*/
LUA_API lua_State *(lua_newstate) (lua_Alloc f, void *ud);
LUA_API void       (lua_close) (lua_State *L);
LUA_API lua_State *(lua_newthread) (lua_State *L);

LUA_API lua_CFunction (lua_atpanic) (lua_State *L, lua_CFunction panicf);


LUA_API const lua_Number *(lua_version) (lua_State *L);


/*
** basic stack manipulation
*/
LUA_API int   (lua_absindex) (lua_State *L, int idx);
LUA_API int   (lua_gettop) (lua_State *L);
LUA_API void  (lua_settop) (lua_State *L, int idx);
LUA_API void  (lua_pushvalue) (lua_State *L, int idx);
LUA_API void  (lua_rotate) (lua_State *L, int idx, int n);
LUA_API void  (lua_copy) (lua_State *L, int fromidx, int toidx);
LUA_API int   (lua_checkstack) (lua_State *L, int n);

LUA_API void  (lua_xmove) (lua_State *from, lua_State *to, int n);


/*
** access functions (stack -> C)
*/

LUA_API int             (lua_isnumber) (lua_State *L, int idx);
LUA_API int             (lua_isstring) (lua_State *L, int idx);
LUA_API int             (lua_iscfunction) (lua_State *L, int idx);
LUA_API int             (lua_isinteger) (lua_State *L, int idx);
LUA_API int             (lua_isuserdata) (lua_State *L, int idx);
LUA_API int             (lua_type) (lua_State *L, int idx);
LUA_API const char     *(lua_typename) (lua_State *L, int tp);

LUA_API lua_Number      (lua_tonumberx) (lua_State *L, int idx, int *isnum);
LUA_API lua_Integer     (lua_tointegerx) (lua_State *L, int idx, int *isnum);
LUA_API int             (lua_toboolean) (lua_State *L, int idx);
LUA_API const char     *(lua_tolstring) (lua_State *L, int idx, size_t *len);
LUA_API size_t          (lua_rawlen) (lua_State *L, int idx);
LUA_API lua_CFunction   (lua_tocfunction) (lua_State *L, int idx);
LUA_API void	       *(lua_touserdata) (lua_State *L, int idx);
LUA_API lua_State      *(lua_tothread) (lua_State *L, int idx);
LUA_API const void     *(lua_topointer) (lua_State *L, int idx);


/*
** Comparison and arithmetic functions
*/

#define LUA_OPADD	0	/* ORDER TM, ORDER OP */
#define LUA_OPSUB	1
#define LUA_OPMUL	2
#define LUA_OPMOD	3
#define LUA_OPPOW	4
#define LUA_OPDIV	5
#define LUA_OPIDIV	6
#define LUA_OPBAND	7
#define LUA_OPBOR	8
#define LUA_OPBXOR	9
#define LUA_OPSHL	10
#define LUA_OPSHR	11
#define LUA_OPUNM	12
#define LUA_OPBNOT	13

LUA_API void  (lua_arith) (lua_State *L, int op);

#define LUA_OPEQ	0
#define LUA_OPLT	1
#define LUA_OPLE	2

LUA_API int   (lua_rawequal) (lua_State *L, int idx1, int idx2);
LUA_API int   (lua_compare) (lua_State *L, int idx1, int idx2, int op);


/*
** push functions (C -> stack)
*/
LUA_API void        (lua_pushnil) (lua_State *L);
LUA_API void        (lua_pushnumber) (lua_State *L, lua_Number n);
LUA_API void        (lua_pushinteger) (lua_State *L, lua_Integer n);
LUA_API const char *(lua_pushlstring) (lua_State *L, const char *s, size_t len);
LUA_API const char *(lua_pushstring) (lua_State *L, const char *s);
LUA_API const char *(lua_pushvfstring) (lua_State *L, const char *fmt,
                                                      va_list argp);
LUA_API const char *(lua_pushfstring) (lua_State *L, const char *fmt, ...);
LUA_API void  (lua_pushcclosure) (lua_State *L, lua_CFunction fn, int n);
LUA_API void  (lua_pushboolean) (lua_State *L, int b);
LUA_API void  (lua_pushlightuserdata) (lua_State *L, void *p);
LUA_API int   (lua_pushthread) (lua_State *L);


/*
** get functions (Lua -> stack)
*/
LUA_API int (lua_getglobal) (lua_State *L, const char *name);
LUA_API int (lua_gettable) (lua_State *L, int idx);
LUA_API int (lua_getfield) (lua_State *L, int idx, const char *k);
LUA_API int (lua_geti) (lua_State *L, int idx, lua_Integer n);
LUA_API int (lua_rawget) (lua_State *L, int idx);
LUA_API int (lua_rawgeti) (lua_State *L, int idx, lua_Integer n);
LUA_API int (lua_rawgetp) (lua_State *L, int idx, const void *p);

LUA_API void  (lua_createtable) (lua_State *L, int narr, int nrec);
LUA_API void *(lua_newuserdata) (lua_State *L, size_t sz);
LUA_API int   (lua_getmetatable) (lua_State *L, int objindex);
LUA_API int  (lua_getuservalue) (lua_State *L, int idx);


/*
** set functions (stack -> Lua)
*/
LUA_API void  (lua_setglobal) (lua_State *L, const char *name);
LUA_API void  (lua_settable) (lua_State *L, int idx);
LUA_API void  (lua_setfield) (lua_State *L, int idx, const char *k);
LUA_API void  (lua_seti) (lua_State *L, int idx, lua_Integer n);
LUA_API void  (lua_rawset) (lua_State *L, int idx);
LUA_API void  (lua_rawseti) (lua_State *L, int idx, lua_Integer n);
LUA_API void  (lua_rawsetp) (lua_State *L, int idx, const void *p);
LUA_API int   (lua_setmetatable) (lua_State *L, int objindex);
LUA_API void  (lua_setuservalue) (lua_State *L, int idx);


/*
** 'load' and 'call' functions (load and run Lua code)
*/
LUA_API void  (lua_callk) (lua_State *L, int nargs, int nresults,
                           lua_KContext ctx, lua_KFunction k);
#define lua_call(L,n,r)		lua_callk(L, (n), (r), 0, NULL)

LUA_API int   (lua_pcallk) (lua_State *L, int nargs, int nresults, int errfunc,
                            lua_KContext ctx, lua_KFunction k);
#define lua_pcall(L,n,r,f)	lua_pcallk(L, (n), (r), (f), 0, NULL)

LUA_API int   (lua_load) (lua_State *L, lua_Reader reader, void *dt,
                          const char *chunkname, const char *mode);

LUA_API int (lua_dump) (lua_State *L, lua_Writer writer, void *data, int strip);


/*
** coroutine functions
*/
LUA_API int  (lua_yieldk)     (lua_State *L, int nresults, lua_KContext ctx,
                               lua_KFunction k);
LUA_API int  (lua_resume)     (lua_State *L, lua_State *from, int narg);
LUA_API int  (lua_status)     (lua_State *L);
LUA_API int (lua_isyieldable) (lua_State *L);

#define lua_yield(L,n)		lua_yieldk(L, (n), 0, NULL)


/*
** garbage-collection function and options
*/

#define LUA_GCSTOP		0
#define LUA_GCRESTART		1
#define LUA_GCCOLLECT		2
#define LUA_GCCOUNT		3
#define LUA_GCCOUNTB		4
#define LUA_GCSTEP		5
#define LUA_GCSETPAUSE		6
#define LUA_GCSETSTEPMUL	7
#define LUA_GCISRUNNING		9

LUA_API int (lua_gc) (lua_State *L, int what, int data);


/*
** miscellaneous functions
*/

LUA_API int   (lua_error) (lua_State *L);

LUA_API int   (lua_next) (lua_State *L, int idx);

LUA_API void  (lua_concat) (lua_State *L, int n);
LUA_API void  (lua_len)    (lua_State *L, int idx);

LUA_API size_t   (lua_stringtonumber) (lua_State *L, const char *s);

LUA_API lua_Alloc (lua_getallocf) (lua_State *L, void **ud);
LUA_API void      (lua_setallocf) (lua_State *L, lua_Alloc f, void *ud);



/*
** {==============================================================
** some useful macros
** ===============================================================
*/

#define lua_getextraspace(L)	((void *)((char *)(L) - LUA_EXTRASPACE))

#define lua_tonumber(L,i)	lua_tonumberx(L,(i),NULL)
#define lua_tointeger(L,i)	lua_tointegerx(L,(i),NULL)

#define lua_pop(L,n)		lua_settop(L, -(n)-1)

#define lua_newtable(L)		lua_createtable(L, 0, 0)

#define lua_register(L,n,f) (lua_pushcfunction(L, (f)), lua_setglobal(L, (n)))

#define lua_pushcfunction(L,f)	lua_pushcclosure(L, (f), 0)

#define lua_isfunction(L,n)	(lua_type(L, (n)) == LUA_TFUNCTION)
#define lua_istable(L,n)	(lua_type(L, (n)) == LUA_TTABLE)
#define lua_islightuserdata(L,n)	(lua_type(L, (n)) == LUA_TLIGHTUSERDATA)
#define lua_isnil(L,n)		(lua_type(L, (n)) == LUA_TNIL)
#define lua_isboolean(L,n)	(lua_type(L, (n)) == LUA_TBOOLEAN)
#define lua_isthread(L,n)	(lua_type(L, (n)) == LUA_TTHREAD)
#define lua_isnone(L,n)		(lua_type(L, (n)) == LUA_TNONE)
#define lua_isnoneornil(L, n)	(lua_type(L, (n)) <= 0)

#define lua_pushliteral(L, s)	lua_pushstring(L, "" s)

#define lua_pushglobaltable(L)  \
	((void)lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS))

#define lua_tostring(L,i)	lua_tolstring(L, (i), NULL)


#define lua_insert(L,idx)	lua_rotate(L, (idx), 1)

#define lua_remove(L,idx)	(lua_rotate(L, (idx), -1), lua_pop(L, 1))

#define lua_replace(L,idx)	(lua_copy(L, -1, (idx)), lua_pop(L, 1))

/* }============================================================== */


/*
** {==============================================================
** compatibility macros for unsigned conversions
** ===============================================================
*/
#if defined(LUA_COMPAT_APIINTCASTS)

#define lua_pushunsigned(L,n)	lua_pushinteger(L, (lua_Integer)(n))
#define lua_tounsignedx(L,i,is)	((lua_Unsigned)lua_tointegerx(L,i,is))
#define lua_tounsigned(L,i)	lua_tounsignedx(L,(i),NULL)

#endif
/* }============================================================== */

/*
** {======================================================================
** Debug API
** =======================================================================
*/


/*
** Event codes
*/
#define LUA_HOOKCALL	0
#define LUA_HOOKRET	1
#define LUA_HOOKLINE	2
#define LUA_HOOKCOUNT	3
#define LUA_HOOKTAILCALL 4


/*
** Event luai_masks
*/
#define LUA_MASKCALL	(1 << LUA_HOOKCALL)
#define LUA_MASKRET	(1 << LUA_HOOKRET)
#define LUA_MASKLINE	(1 << LUA_HOOKLINE)
#define LUA_MASKCOUNT	(1 << LUA_HOOKCOUNT)

typedef struct lua_Debug lua_Debug;  /* activation record */


/* Functions to be called by the debugger in specific events */
typedef void (*lua_Hook) (lua_State *L, lua_Debug *ar);


LUA_API int (lua_getstack) (lua_State *L, int level, lua_Debug *ar);
LUA_API int (lua_getinfo) (lua_State *L, const char *what, lua_Debug *ar);
LUA_API const char *(lua_getlocal) (lua_State *L, const lua_Debug *ar, int n);
LUA_API const char *(lua_setlocal) (lua_State *L, const lua_Debug *ar, int n);
LUA_API const char *(lua_getupvalue) (lua_State *L, int funcindex, int n);
LUA_API const char *(lua_setupvalue) (lua_State *L, int funcindex, int n);

LUA_API void *(lua_upvalueid) (lua_State *L, int fidx, int n);
LUA_API void  (lua_upvaluejoin) (lua_State *L, int fidx1, int n1,
                                               int fidx2, int n2);

LUA_API void (lua_sethook) (lua_State *L, lua_Hook func, int mask, int count);
LUA_API lua_Hook (lua_gethook) (lua_State *L);
LUA_API int (lua_gethookmask) (lua_State *L);
LUA_API int (lua_gethookcount) (lua_State *L);


struct lua_Debug {
  int event;
  const char *name;	/* (n) */
  const char *namewhat;	/* (n) 'global', 'local', 'luai_field', 'method' */
  const char *what;	/* (S) 'Lua', 'C', 'main', 'tail' */
  const char *source;	/* (S) */
  int luai_currentline;	/* (l) */
  int linedefined;	/* (S) */
  int lastlinedefined;	/* (S) */
  unsigned char nups;	/* (u) number of upvalues */
  unsigned char nparams;/* (u) number of parameters */
  char isvararg;        /* (u) */
  char istailcall;	/* (t) */
  char short_src[LUA_IDSIZE]; /* (S) */
  /* private part */
  struct luai_CallInfo *i_ci;  /* active function */
};

/*__lauxlib.h__*/

/* extra error code for 'luaL_loadfilex' */
#define LUA_ERRFILE     (LUA_ERRERR+1)


/* key, in the registry, for table of loaded modules */
#define LUA_LOADED_TABLE	"_LOADED"


/* key, in the registry, for table of preloaded loaders */
#define LUA_PRELOAD_TABLE	"_PRELOAD"


typedef struct luaL_Reg {
  const char *name;
  lua_CFunction func;
} luaL_Reg;


#define LUAL_NUMSIZES	(sizeof(lua_Integer)*16 + sizeof(lua_Number))

LUALIB_API void (luaL_checkversion_) (lua_State *L, lua_Number ver, size_t sz);
#define luaL_checkversion(L)  \
	  luaL_checkversion_(L, LUA_VERSION_NUM, LUAL_NUMSIZES)

LUALIB_API int (luaL_getmetafield) (lua_State *L, int obj, const char *e);
LUALIB_API int (luaL_callmeta) (lua_State *L, int obj, const char *e);
LUALIB_API const char *(luaL_tolstring) (lua_State *L, int idx, size_t *len);
LUALIB_API int (luaL_argerror) (lua_State *L, int arg, const char *extramsg);
LUALIB_API const char *(luaL_checklstring) (lua_State *L, int arg,
                                                          size_t *l);
LUALIB_API const char *(luaL_optlstring) (lua_State *L, int arg,
                                          const char *def, size_t *l);
LUALIB_API lua_Number (luaL_checknumber) (lua_State *L, int arg);
LUALIB_API lua_Number (luaL_optnumber) (lua_State *L, int arg, lua_Number def);

LUALIB_API lua_Integer (luaL_checkinteger) (lua_State *L, int arg);
LUALIB_API lua_Integer (luaL_optinteger) (lua_State *L, int arg,
                                          lua_Integer def);

LUALIB_API void (luaL_checkstack) (lua_State *L, int sz, const char *msg);
LUALIB_API void (luaL_checktype) (lua_State *L, int arg, int t);
LUALIB_API void (luaL_checkany) (lua_State *L, int arg);

LUALIB_API int   (luaL_newmetatable) (lua_State *L, const char *tname);
LUALIB_API void  (luaL_setmetatable) (lua_State *L, const char *tname);
LUALIB_API void *(luaL_testudata) (lua_State *L, int ud, const char *tname);
LUALIB_API void *(luaL_checkudata) (lua_State *L, int ud, const char *tname);

LUALIB_API void (luaL_where) (lua_State *L, int lvl);
LUALIB_API int (luaL_error) (lua_State *L, const char *fmt, ...);

LUALIB_API int (luaL_checkoption) (lua_State *L, int arg, const char *def,
                                   const char *const lst[]);

LUALIB_API int (luaL_fileresult) (lua_State *L, int stat, const char *fname);
LUALIB_API int (luaL_execresult) (lua_State *L, int stat);

/* predefined references */
#define LUA_NOREF       (-2)
#define LUA_REFNIL      (-1)

LUALIB_API int (luaL_ref) (lua_State *L, int t);
LUALIB_API void (luaL_unref) (lua_State *L, int t, int ref);

LUALIB_API int (luaL_loadfilex) (lua_State *L, const char *filename,
                                               const char *mode);

#define luaL_loadfile(L,f)	luaL_loadfilex(L,f,NULL)

LUALIB_API int (luaL_loadbufferx) (lua_State *L, const char *buff, size_t sz,
                                   const char *name, const char *mode);
LUALIB_API int (luaL_loadstring) (lua_State *L, const char *s);

LUALIB_API lua_State *(luaL_newstate) (void);

LUALIB_API lua_Integer (luaL_len) (lua_State *L, int idx);

LUALIB_API const char *(luaL_gsub) (lua_State *L, const char *s, const char *p,
                                                  const char *r);

LUALIB_API void (luaL_setfuncs) (lua_State *L, const luaL_Reg *l, int nup);

LUALIB_API int (luaL_getsubtable) (lua_State *L, int idx, const char *fname);

LUALIB_API void (luaL_traceback) (lua_State *L, lua_State *L1,
                                  const char *msg, int level);

LUALIB_API void (luaL_requiref) (lua_State *L, const char *modname,
                                 lua_CFunction openf, int glb);

/*
** ===============================================================
** some useful macros
** ===============================================================
*/


#define luaL_newlibtable(L,l)	\
  lua_createtable(L, 0, sizeof(l)/sizeof((l)[0]) - 1)

#define luaL_newlib(L,l)  \
  (luaL_checkversion(L), luaL_newlibtable(L,l), luaL_setfuncs(L,l,0))

#define luaL_argcheck(L, luai_cond,arg,extramsg)	\
		((void)((luai_cond) || luaL_argerror(L, (arg), (extramsg))))
#define luaL_checkstring(L,n)	(luaL_checklstring(L, (n), NULL))
#define luaL_optstring(L,n,d)	(luaL_optlstring(L, (n), (d), NULL))

#define luaL_typename(L,i)	lua_typename(L, lua_type(L,(i)))

#define luaL_dofile(L, fn) \
	(luaL_loadfile(L, fn) || lua_pcall(L, 0, LUA_MULTRET, 0))

#define luaL_dostring(L, s) \
	(luaL_loadstring(L, s) || lua_pcall(L, 0, LUA_MULTRET, 0))

#define luaL_getmetatable(L,n)	(lua_getfield(L, LUA_REGISTRYINDEX, (n)))

#define luaL_opt(L,f,n,d)	(lua_isnoneornil(L,(n)) ? (d) : f(L,(n)))

#define luaL_loadbuffer(L,s,sz,n)	luaL_loadbufferx(L,s,sz,n,NULL)


/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

typedef struct luaL_Buffer {
  char *b;  /* buffer address */
  size_t size;  /* buffer size */
  size_t n;  /* number of characters in buffer */
  lua_State *L;
  char initb[LUAL_BUFFERSIZE];  /* initial buffer */
} luaL_Buffer;


#define luaL_addchar(B,c) \
  ((void)((B)->n < (B)->size || luaL_prepbuffsize((B), 1)), \
   ((B)->b[(B)->n++] = (c)))

#define luaL_addsize(B,s)	((B)->n += (s))

LUALIB_API void (luaL_buffinit) (lua_State *L, luaL_Buffer *B);
LUALIB_API char *(luaL_prepbuffsize) (luaL_Buffer *B, size_t sz);
LUALIB_API void (luaL_addlstring) (luaL_Buffer *B, const char *s, size_t l);
LUALIB_API void (luaL_addstring) (luaL_Buffer *B, const char *s);
LUALIB_API void (luaL_addvalue) (luaL_Buffer *B);
LUALIB_API void (luaL_pushresult) (luaL_Buffer *B);
LUALIB_API void (luaL_pushresultsize) (luaL_Buffer *B, size_t sz);
LUALIB_API char *(luaL_buffinitsize) (lua_State *L, luaL_Buffer *B, size_t sz);

#define luaL_prepbuffer(B)	luaL_prepbuffsize(B, LUAL_BUFFERSIZE)

/* }====================================================== */



/*
** {======================================================
** File handles for IO library
** =======================================================
*/

/*
** A file handle is a userdata with metatable 'LUA_FILEHANDLE' and
** initial structure 'luaL_Stream' (it may contain other fields
** after that initial structure).
*/

#define LUA_FILEHANDLE          "FILE*"


typedef struct luaL_Stream {
  FILE *f;  /* stream (NULL for incompletely created streams) */
  lua_CFunction closef;  /* to close stream (NULL for closed streams) */
} luaL_Stream;

/* }====================================================== */



/* compatibility with old module system */
#if defined(LUA_COMPAT_MODULE)

LUALIB_API void (luaL_pushmodule) (lua_State *L, const char *modname,
                                   int sizehint);
LUALIB_API void (luaL_openlib) (lua_State *L, const char *libname,
                                const luaL_Reg *l, int nup);

#define luaL_register(L,n,l)	(luaL_openlib(L,(n),(l),0))

#endif


/*
** {==================================================================
** "Abstraction Layer" for basic report of messages and errors
** ===================================================================
*/

/* print a string */
#if !defined(lua_writestring)
#define lua_writestring(s,l)   fwrite((s), sizeof(char), (l), stdout)
#endif

/* print a newline and flush the output */
#if !defined(lua_writeline)
#define lua_writeline()        (lua_writestring("\n", 1), fflush(stdout))
#endif

/* print an error message */
#if !defined(lua_writestringerror)
#define lua_writestringerror(s,p) \
        (fprintf(stderr, (s), (p)), fflush(stderr))
#endif

/* }================================================================== */


/*
** {============================================================
** Compatibility with deprecated conversions
** =============================================================
*/
#if defined(LUA_COMPAT_APIINTCASTS)

#define luaL_checkunsigned(L,a)	((lua_Unsigned)luaL_checkinteger(L,a))
#define luaL_optunsigned(L,a,d)	\
	((lua_Unsigned)luaL_optinteger(L,a,(lua_Integer)(d)))

#define luaL_checkint(L,n)	((int)luaL_checkinteger(L, (n)))
#define luaL_optint(L,n,d)	((int)luaL_optinteger(L, (n), (d)))

#define luaL_checklong(L,n)	((long)luaL_checkinteger(L, (n)))
#define luaL_optlong(L,n,d)	((long)luaL_optinteger(L, (n), (d)))

#endif
/* }============================================================ */

/*__lualib.h__*/

/* version suffix for environment variable names */
#define LUA_VERSUFFIX          "_" LUA_VERSION_MAJOR "_" LUA_VERSION_MINOR


LUAMOD_API int (luaopen_base) (lua_State *L);

#define LUA_COLIBNAME	"coroutine"
LUAMOD_API int (luaopen_coroutine) (lua_State *L);

#define LUA_TABLIBNAME	"table"
LUAMOD_API int (luaopen_table) (lua_State *L);

#define LUA_IOLIBNAME	"io"
LUAMOD_API int (luaopen_io) (lua_State *L);

#define LUA_OSLIBNAME	"os"
LUAMOD_API int (luaopen_os) (lua_State *L);

#define LUA_STRLIBNAME	"string"
LUAMOD_API int (luaopen_string) (lua_State *L);

#define LUA_UTF8LIBNAME	"utf8"
LUAMOD_API int (luaopen_utf8) (lua_State *L);

#define LUA_BITLIBNAME	"bit32"
LUAMOD_API int (luaopen_bit32) (lua_State *L);

#define LUA_MATHLIBNAME	"math"
LUAMOD_API int (luaopen_math) (lua_State *L);

#define LUA_DBLIBNAME	"debug"
LUAMOD_API int (luaopen_debug) (lua_State *L);

#define LUA_LOADLIBNAME	"package"
LUAMOD_API int (luaopen_package) (lua_State *L);


/* open all previous libraries */
LUALIB_API void (luaL_openlibs) (lua_State *L);



#if !defined(lua_assert)
#define lua_assert(x)	((void)0)
#endif

#endif // lua_h

/***************************************************************************
 * IMPLEMENTATION
 **************************************************************************/

#ifdef LUA_IMPLEMENTATION

#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <locale.h>
#include <math.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*__llimits.h__*/

/*
** 'luai_lu_mem' and 'luai_l_mem' are unsigned/signed integers big enough to count
** the total memory used by Lua (in bytes). Usually, 'size_t' and
** 'ptrdiff_t' should work, but we use 'long' for 16-bit machines.
*/
#if defined(LUAI_MEM)		/* { external definitions? */
typedef LUAI_UMEM luai_lu_mem;
typedef LUAI_MEM luai_l_mem;
#elif LUAI_BITSINT >= 32	/* }{ */
typedef size_t luai_lu_mem;
typedef ptrdiff_t luai_l_mem;
#else  /* 16-bit ints */	/* }{ */
typedef unsigned long luai_lu_mem;
typedef long luai_l_mem;
#endif				/* } */


/* chars used as small naturals (so that 'char' is reserved for characters) */
typedef unsigned char luai_lu_byte;


/* maximum value for size_t */
#define LUAI_MAX_SIZET	((size_t)(~(size_t)0))

/* maximum size visible for Lua (must be representable in a lua_Integer */
#define LUAI_MAX_SIZE	(sizeof(size_t) < sizeof(lua_Integer) ? LUAI_MAX_SIZET \
                          : (size_t)(LUA_MAXINTEGER))


#define LUAI_MAX_LUMEM	((luai_lu_mem)(~(luai_lu_mem)0))

#define LUAI_MAX_LMEM	((luai_l_mem)(LUAI_MAX_LUMEM >> 1))


#define LUAI_MAX_INT		INT_MAX  /* maximum value of an int */


/*
** conversion of pointer to unsigned integer:
** this is for hashing only; there is no problem if the integer
** cannot hold the whole pointer value
*/
#define luai_point2uint(p)	((unsigned int)((size_t)(p) & UINT_MAX))



/* type to ensure maximum alignment */
#if defined(LUAI_USER_ALIGNMENT_T)
typedef LUAI_USER_ALIGNMENT_T LUAI_L_Umaxalign;
#else
typedef union {
  lua_Number n;
  double u;
  void *s;
  lua_Integer i;
  long l;
} LUAI_L_Umaxalign;
#endif



/* types of 'usual argument conversions' for lua_Number and lua_Integer */
typedef LUAI_UACNUMBER luai_l_uacNumber;
typedef LUAI_UACINT luai_l_uacInt;


/* internal assertions for in-house debugging */
#if defined(lua_assert)
#define luai_check_exp(c,e)		(lua_assert(c), (e))
/* to avoid problems with conditions too long */
#define lua_longassert(c)	((c) ? (void)0 : lua_assert(0))
#else
#define lua_assert(c)		((void)0)
#define luai_check_exp(c,e)		(e)
#define lua_longassert(c)	((void)0)
#endif

/*
** assertion for checking API calls
*/
#if !defined(luai_apicheck)
#define luai_apicheck(l,e)	lua_assert(e)
#endif

#define luai_api_check(l,e,msg)	luai_apicheck(l,(e) && msg)


/* macro to avoid warnings about unused variables */
#if !defined(LUAI_UNUSED)
#define LUAI_UNUSED(x)	((void)(x))
#endif


/* type casts (a macro highlights casts in the code) */
#define luai_cast(t, exp)	((t)(exp))

#define luai_cast_void(i)	luai_cast(void, (i))
#define luai_cast_byte(i)	luai_cast(luai_lu_byte, (i))
#define luai_cast_num(i)	luai_cast(lua_Number, (i))
#define luai_cast_int(i)	luai_cast(int, (i))
#define luai_cast_uchar(i)	luai_cast(unsigned char, (i))


/* luai_cast a signed lua_Integer to lua_Unsigned */
#if !defined(luai_l_castS2U)
#define luai_l_castS2U(i)	((lua_Unsigned)(i))
#endif

/*
** luai_cast a lua_Unsigned to a signed lua_Integer; this luai_cast is
** not strict ISO C, but two-complement architectures should
** work fine.
*/
#if !defined(luai_l_castU2S)
#define luai_l_castU2S(i)	((lua_Integer)(i))
#endif


/*
** non-return type
*/
#if defined(__GNUC__)
#define luai_l_noret		void __attribute__((noreturn))
#elif defined(_MSC_VER) && _MSC_VER >= 1200
#define luai_l_noret		void __declspec(noreturn)
#else
#define luai_l_noret		void
#endif



/*
** maximum depth for nested C calls and syntactical nested non-terminals
** in a program. (Value must fit in an unsigned short int.)
*/
#if !defined(LUAI_MAXCCALLS)
#define LUAI_MAXCCALLS		200
#endif



/*
** type for virtual-machine instructions;
** must be an unsigned with (at least) 4 bytes (see details in lopcodes.h)
*/
#if LUAI_BITSINT >= 32
typedef unsigned int Instruction;
#else
typedef unsigned long Instruction;
#endif



/*
** Maximum length for short strings, that is, strings that are
** internalized. (Cannot be smaller than reserved words or tags for
** metamethods, as these strings must be internalized;
** #("function") = 8, #("__newindex") = 10.)
*/
#if !defined(LUAI_MAXSHORTLEN)
#define LUAI_MAXSHORTLEN	40
#endif


/*
** Initial size for the string table (must be power of 2).
** The Lua core alone registers ~50 strings (reserved words +
** metaevent keys + a few others). Libraries would typically add
** a few dozens more.
*/
#if !defined(LUAI_MINSTRTABSIZE)
#define LUAI_MINSTRTABSIZE	128
#endif


/*
** Size of cache for strings in the API. 'N' is the number of
** sets (better be a prime) and "M" is the size of each set (M == 1
** makes a direct cache.)
*/
#if !defined(LUAI_STRCACHE_N)
#define LUAI_STRCACHE_N		53
#define LUAI_STRCACHE_M		2
#endif


/* minimum size for string buffer */
#if !defined(LUA_MINBUFFER)
#define LUA_MINBUFFER	32
#endif


/*
** macros that are executed whenever program enters the Lua core
** ('lua_lock') and leaves the core ('lua_unlock')
*/
#if !defined(lua_lock)
#define lua_lock(L)	((void) 0)
#define lua_unlock(L)	((void) 0)
#endif

/*
** macro executed during Lua functions at points where the
** function can yield.
*/
#if !defined(luai_threadyield)
#define luai_threadyield(L)	{lua_unlock(L); lua_lock(L);}
#endif


/*
** these macros allow user-specific actions on threads when you defined
** LUAI_EXTRASPACE and need to do something extra when a thread is
** created/deleted/resumed/yielded.
*/
#if !defined(luai_userstateopen)
#define luai_userstateopen(L)		((void)L)
#endif

#if !defined(luai_userstateclose)
#define luai_userstateclose(L)		((void)L)
#endif

#if !defined(luai_userstatethread)
#define luai_userstatethread(L,L1)	((void)L)
#endif

#if !defined(luai_userstatefree)
#define luai_userstatefree(L,L1)	((void)L)
#endif

#if !defined(luai_userstateresume)
#define luai_userstateresume(L,n)	((void)L)
#endif

#if !defined(luai_userstateyield)
#define luai_userstateyield(L,n)	((void)L)
#endif



/*
** The luai_num* macros define the primitive operations over numbers.
*/

/* floor division (defined as 'floor(a/b)') */
#if !defined(luai_numidiv)
#define luai_numidiv(L,a,b)     ((void)L, luai_l_floor(luai_numdiv(L,a,b)))
#endif

/* float division */
#if !defined(luai_numdiv)
#define luai_numdiv(L,a,b)      ((a)/(b))
#endif

/*
** modulo: defined as 'a - floor(a/b)*b'; this definition gives NaN when
** 'b' is huge, but the result should be 'a'. 'fmod' gives the result of
** 'a - trunc(a/b)*b', and therefore must be corrected when 'trunc(a/b)
** ~= floor(a/b)'. That happens when the division has a non-integer
** negative result, which is equivalent to the test below.
*/
#if !defined(luai_nummod)
#define luai_nummod(L,a,b,m)  \
  { (m) = luai_l_mathop(fmod)(a,b); if ((m)*(b) < 0) (m) += (b); }
#endif

/* exponentiation */
#if !defined(luai_numpow)
#define luai_numpow(L,a,b)      ((void)L, luai_l_mathop(pow)(a,b))
#endif

/* the others are quite standard operations */
#if !defined(luai_numadd)
#define luai_numadd(L,a,b)      ((a)+(b))
#define luai_numsub(L,a,b)      ((a)-(b))
#define luai_nummul(L,a,b)      ((a)*(b))
#define luai_numunm(L,a)        (-(a))
#define luai_numeq(a,b)         ((a)==(b))
#define luai_numlt(a,b)         ((a)<(b))
#define luai_numle(a,b)         ((a)<=(b))
#define luai_numisnan(a)        (!luai_numeq((a), (a)))
#endif





/*
** macro to control inclusion of some hard tests on stack reallocation
*/
#if !defined(LUAI_HARDSTACKTESTS)
#define luai_condmovestack(L,pre,pos)	((void)0)
#else
/* realloc stack keeping its size */
#define luai_condmovestack(L,pre,pos)  \
	{ int sz_ = (L)->stacksize; pre; luaD_reallocstack((L), sz_); pos; }
#endif

#if !defined(HARDMEMTESTS)
#define luai_condchangemem(L,pre,pos)	((void)0)
#else
#define luai_condchangemem(L,pre,pos)  \
	{ if (LUAI_G(L)->gcrunning) { pre; luaC_fullgc(L, 0); pos; } }
#endif

/*__lobject.h__*/

/*
** Extra tags for non-values
*/
#define LUA_TPROTO	LUA_NUMTAGS		/* function prototypes */
#define LUA_TDEADKEY	(LUA_NUMTAGS+1)		/* removed keys in tables */

/*
** number of all possible tags (including LUA_TNONE but excluding DEADKEY)
*/
#define LUA_TOTALTAGS	(LUA_TPROTO + 2)


/*
** tags for Tagged Values have the following use of bits:
** bits 0-3: actual tag (a LUA_T* value)
** bits 4-5: variant bits
** bit 6: whether value is collectable
*/


/*
** LUA_TFUNCTION variants:
** 0 - Lua function
** 1 - light C function
** 2 - regular C function (closure)
*/

/* Variant tags for functions */
#define LUA_TLCL	(LUA_TFUNCTION | (0 << 4))  /* Lua closure */
#define LUA_TLCF	(LUA_TFUNCTION | (1 << 4))  /* light C function */
#define LUA_TCCL	(LUA_TFUNCTION | (2 << 4))  /* C closure */


/* Variant tags for strings */
#define LUA_TSHRSTR	(LUA_TSTRING | (0 << 4))  /* short strings */
#define LUA_TLNGSTR	(LUA_TSTRING | (1 << 4))  /* long strings */


/* Variant tags for numbers */
#define LUA_TNUMFLT	(LUA_TNUMBER | (0 << 4))  /* float numbers */
#define LUA_TNUMINT	(LUA_TNUMBER | (1 << 4))  /* integer numbers */


/* Bit mark for collectable types */
#define LUAI_BIT_ISCOLLECTABLE	(1 << 6)

/* mark a tag as collectable */
#define luai_ctb(t)			((t) | LUAI_BIT_ISCOLLECTABLE)


/*
** Common type for all collectable objects
*/
typedef struct LUAI_GCObject LUAI_GCObject;


/*
** Common luai_Header for all collectable objects (in macro form, to be
** included in other objects)
*/
#define luai_CommonHeader	LUAI_GCObject *luai_next; luai_lu_byte tt; luai_lu_byte marked


/*
** Common type has only the common header
*/
struct LUAI_GCObject {
  luai_CommonHeader;
};




/*
** Tagged Values. This is the basic representation of values in Lua,
** an actual value plus a tag with its type.
*/

/*
** Union of all Lua values
*/
typedef union lua_Value {
  LUAI_GCObject *gc;    /* collectable objects */
  void *p;         /* light userdata */
  int b;           /* booleans */
  lua_CFunction f; /* light C functions */
  lua_Integer i;   /* integer numbers */
  lua_Number n;    /* float numbers */
} lua_Value;


#define luai_TValuefields	lua_Value value_; int tt_


typedef struct lua_TValue {
  luai_TValuefields;
} luai_TValue;



/* macro defining a nil value */
#define LUAI_NILCONSTANT	{NULL}, LUA_TNIL


#define luai_val_(o)		((o)->value_)


/* raw type tag of a luai_TValue */
#define luai_rttype(o)	((o)->tt_)

/* tag with no variants (bits 0-3) */
#define luai_novariant(x)	((x) & 0x0F)

/* type tag of a luai_TValue (bits 0-3 for tags + variant bits 4-5) */
#define luai_ttype(o)	(luai_rttype(o) & 0x3F)

/* type tag of a luai_TValue with no variants (bits 0-3) */
#define luai_ttnov(o)	(luai_novariant(luai_rttype(o)))


/* Macros to test type */
#define luai_checktag(o,t)		(luai_rttype(o) == (t))
#define luai_checktype(o,t)		(luai_ttnov(o) == (t))
#define luai_ttisnumber(o)		luai_checktype((o), LUA_TNUMBER)
#define luai_ttisfloat(o)		luai_checktag((o), LUA_TNUMFLT)
#define luai_ttisinteger(o)		luai_checktag((o), LUA_TNUMINT)
#define luai_ttisnil(o)		luai_checktag((o), LUA_TNIL)
#define luai_ttisboolean(o)		luai_checktag((o), LUA_TBOOLEAN)
#define luai_ttislightuserdata(o)	luai_checktag((o), LUA_TLIGHTUSERDATA)
#define luai_ttisstring(o)		luai_checktype((o), LUA_TSTRING)
#define luai_ttisshrstring(o)	luai_checktag((o), luai_ctb(LUA_TSHRSTR))
#define luai_ttislngstring(o)	luai_checktag((o), luai_ctb(LUA_TLNGSTR))
#define luai_ttistable(o)		luai_checktag((o), luai_ctb(LUA_TTABLE))
#define luai_ttisfunction(o)		luai_checktype(o, LUA_TFUNCTION)
#define luai_ttisclosure(o)		((luai_rttype(o) & 0x1F) == LUA_TFUNCTION)
#define luai_ttisCclosure(o)		luai_checktag((o), luai_ctb(LUA_TCCL))
#define luai_ttisLclosure(o)		luai_checktag((o), luai_ctb(LUA_TLCL))
#define luai_ttislcf(o)		luai_checktag((o), LUA_TLCF)
#define luai_ttisfulluserdata(o)	luai_checktag((o), luai_ctb(LUA_TUSERDATA))
#define luai_ttisthread(o)		luai_checktag((o), luai_ctb(LUA_TTHREAD))
#define luai_ttisdeadkey(o)		luai_checktag((o), LUA_TDEADKEY)


/* Macros to access values */
#define luai_ivalue(o)	luai_check_exp(luai_ttisinteger(o), luai_val_(o).i)
#define luai_fltvalue(o)	luai_check_exp(luai_ttisfloat(o), luai_val_(o).n)
#define luai_nvalue(o)	luai_check_exp(luai_ttisnumber(o), \
	(luai_ttisinteger(o) ? luai_cast_num(luai_ivalue(o)) : luai_fltvalue(o)))
#define luai_gcvalue(o)	luai_check_exp(luai_iscollectable(o), luai_val_(o).gc)
#define luai_pvalue(o)	luai_check_exp(luai_ttislightuserdata(o), luai_val_(o).p)
#define luai_tsvalue(o)	luai_check_exp(luai_ttisstring(o), luai_gco2ts(luai_val_(o).gc))
#define luai_uvalue(o)	luai_check_exp(luai_ttisfulluserdata(o), luai_gco2u(luai_val_(o).gc))
#define luai_clvalue(o)	luai_check_exp(luai_ttisclosure(o), luai_gco2cl(luai_val_(o).gc))
#define luai_clLvalue(o)	luai_check_exp(luai_ttisLclosure(o), luai_gco2lcl(luai_val_(o).gc))
#define luai_clCvalue(o)	luai_check_exp(luai_ttisCclosure(o), luai_gco2ccl(luai_val_(o).gc))
#define luai_fvalue(o)	luai_check_exp(luai_ttislcf(o), luai_val_(o).f)
#define luai_hvalue(o)	luai_check_exp(luai_ttistable(o), luai_gco2t(luai_val_(o).gc))
#define luai_bvalue(o)	luai_check_exp(luai_ttisboolean(o), luai_val_(o).b)
#define luai_thvalue(o)	luai_check_exp(luai_ttisthread(o), luai_gco2th(luai_val_(o).gc))
/* a dead value may get the 'gc' luai_field, but cannot access its contents */
#define luai_deadvalue(o)	luai_check_exp(luai_ttisdeadkey(o), luai_cast(void *, luai_val_(o).gc))

#define luai_l_isfalse(o)	(luai_ttisnil(o) || (luai_ttisboolean(o) && luai_bvalue(o) == 0))


#define luai_iscollectable(o)	(luai_rttype(o) & LUAI_BIT_ISCOLLECTABLE)


/* Macros for internal tests */
#define luai_righttt(obj)		(luai_ttype(obj) == luai_gcvalue(obj)->tt)

#define luai_checkliveness(L,obj) \
	lua_longassert(!luai_iscollectable(obj) || \
		(luai_righttt(obj) && (L == NULL || !luai_isdead(LUAI_G(L),luai_gcvalue(obj)))))


/* Macros to set values */
#define luai_settt_(o,t)	((o)->tt_=(t))

#define luai_setfltvalue(obj,x) \
  { luai_TValue *io=(obj); luai_val_(io).n=(x); luai_settt_(io, LUA_TNUMFLT); }

#define luai_chgfltvalue(obj,x) \
  { luai_TValue *io=(obj); lua_assert(luai_ttisfloat(io)); luai_val_(io).n=(x); }

#define luai_setivalue(obj,x) \
  { luai_TValue *io=(obj); luai_val_(io).i=(x); luai_settt_(io, LUA_TNUMINT); }

#define luai_chgivalue(obj,x) \
  { luai_TValue *io=(obj); lua_assert(luai_ttisinteger(io)); luai_val_(io).i=(x); }

#define luai_setnilvalue(obj) luai_settt_(obj, LUA_TNIL)

#define luai_setfvalue(obj,x) \
  { luai_TValue *io=(obj); luai_val_(io).f=(x); luai_settt_(io, LUA_TLCF); }

#define luai_setpvalue(obj,x) \
  { luai_TValue *io=(obj); luai_val_(io).p=(x); luai_settt_(io, LUA_TLIGHTUSERDATA); }

#define luai_setbvalue(obj,x) \
  { luai_TValue *io=(obj); luai_val_(io).b=(x); luai_settt_(io, LUA_TBOOLEAN); }

#define luai_setgcovalue(L,obj,x) \
  { luai_TValue *io = (obj); LUAI_GCObject *i_g=(x); \
    luai_val_(io).gc = i_g; luai_settt_(io, luai_ctb(i_g->tt)); }

#define luai_setsvalue(L,obj,x) \
  { luai_TValue *io = (obj); luai_TString *x_ = (x); \
    luai_val_(io).gc = luai_obj2gco(x_); luai_settt_(io, luai_ctb(x_->tt)); \
    luai_checkliveness(L,io); }

#define luai_setuvalue(L,obj,x) \
  { luai_TValue *io = (obj); luai_Udata *x_ = (x); \
    luai_val_(io).gc = luai_obj2gco(x_); luai_settt_(io, luai_ctb(LUA_TUSERDATA)); \
    luai_checkliveness(L,io); }

#define luai_setthvalue(L,obj,x) \
  { luai_TValue *io = (obj); lua_State *x_ = (x); \
    luai_val_(io).gc = luai_obj2gco(x_); luai_settt_(io, luai_ctb(LUA_TTHREAD)); \
    luai_checkliveness(L,io); }

#define luai_setclLvalue(L,obj,x) \
  { luai_TValue *io = (obj); luai_LClosure *x_ = (x); \
    luai_val_(io).gc = luai_obj2gco(x_); luai_settt_(io, luai_ctb(LUA_TLCL)); \
    luai_checkliveness(L,io); }

#define luai_setclCvalue(L,obj,x) \
  { luai_TValue *io = (obj); luai_CClosure *x_ = (x); \
    luai_val_(io).gc = luai_obj2gco(x_); luai_settt_(io, luai_ctb(LUA_TCCL)); \
    luai_checkliveness(L,io); }

#define luai_sethvalue(L,obj,x) \
  { luai_TValue *io = (obj); luai_Table *x_ = (x); \
    luai_val_(io).gc = luai_obj2gco(x_); luai_settt_(io, luai_ctb(LUA_TTABLE)); \
    luai_checkliveness(L,io); }

#define luai_setdeadvalue(obj)	luai_settt_(obj, LUA_TDEADKEY)



#define luai_setobj(L,obj1,obj2) \
	{ luai_TValue *io1=(obj1); *io1 = *(obj2); \
	  (void)L; luai_checkliveness(L,io1); }


/*
** different types of assignments, according to destination
*/

/* from stack to (same) stack */
#define luai_setobjs2s	luai_setobj
/* to stack (not from same stack) */
#define luai_setobj2s	luai_setobj
#define luai_setsvalue2s	luai_setsvalue
#define luai_sethvalue2s	luai_sethvalue
#define luai_setptvalue2s	setptvalue
/* from table to same table */
#define luai_setobjt2t	luai_setobj
/* to new object */
#define luai_setobj2n	luai_setobj
#define luai_setsvalue2n	luai_setsvalue

/* to table (define it as an expression to be used in macros) */
#define luai_setobj2t(L,o1,o2)  ((void)L, *(o1)=*(o2), luai_checkliveness(L,(o1)))




/*
** {======================================================
** types and prototypes
** =======================================================
*/


typedef luai_TValue *luai_StkId;  /* index to stack elements */




/*
** luai_Header for string value; string bytes follow the end of this structure
** (aligned according to 'luai_UTString'; see luai_next).
*/
typedef struct luai_TString {
  luai_CommonHeader;
  luai_lu_byte extra;  /* reserved words for short strings; "has hash" for longs */
  luai_lu_byte shrlen;  /* length for short strings */
  unsigned int hash;
  union {
    size_t lnglen;  /* length for long strings */
    struct luai_TString *hnext;  /* linked list for hash table */
  } u;
} luai_TString;


/*
** Ensures that address after this type is always fully aligned.
*/
typedef union luai_UTString {
  LUAI_L_Umaxalign dummy;  /* ensures maximum alignment for strings */
  luai_TString tsv;
} luai_UTString;


/*
** Get the actual string (array of bytes) from a 'luai_TString'.
** (Access to 'extra' ensures that value is really a 'luai_TString'.)
*/
#define luai_getstr(ts)  \
  luai_check_exp(sizeof((ts)->extra), luai_cast(char *, (ts)) + sizeof(luai_UTString))


/* get the actual string (array of bytes) from a Lua value */
#define luai_svalue(o)       luai_getstr(luai_tsvalue(o))

/* get string length from 'luai_TString *s' */
#define luai_tsslen(s)	((s)->tt == LUA_TSHRSTR ? (s)->shrlen : (s)->u.lnglen)

/* get string length from 'luai_TValue *o' */
#define luai_vslen(o)	luai_tsslen(luai_tsvalue(o))


/*
** luai_Header for userdata; memory area follows the end of this structure
** (aligned according to 'luai_UUdata'; see luai_next).
*/
typedef struct luai_Udata {
  luai_CommonHeader;
  luai_lu_byte ttuv_;  /* user value's tag */
  struct luai_Table *metatable;
  size_t len;  /* number of bytes */
  union lua_Value user_;  /* user value */
} luai_Udata;


/*
** Ensures that address after this type is always fully aligned.
*/
typedef union luai_UUdata {
  LUAI_L_Umaxalign dummy;  /* ensures maximum alignment for 'local' udata */
  luai_Udata uv;
} luai_UUdata;


/*
**  Get the address of memory luai_getblock inside 'luai_Udata'.
** (Access to 'ttuv_' ensures that value is really a 'luai_Udata'.)
*/
#define luai_getudatamem(u)  \
  luai_check_exp(sizeof((u)->ttuv_), (luai_cast(char*, (u)) + sizeof(luai_UUdata)))

#define luai_setuservalue(L,u,o) \
	{ const luai_TValue *io=(o); luai_Udata *iu = (u); \
	  iu->user_ = io->value_; iu->ttuv_ = luai_rttype(io); \
	  luai_checkliveness(L,io); }


#define luai_getuservalue(L,u,o) \
	{ luai_TValue *io=(o); const luai_Udata *iu = (u); \
	  io->value_ = iu->user_; luai_settt_(io, iu->ttuv_); \
	  luai_checkliveness(L,io); }


/*
** Description of an upvalue for function prototypes
*/
typedef struct luai_Upvaldesc {
  luai_TString *name;  /* upvalue name (for debug information) */
  luai_lu_byte instack;  /* whether it is in stack (register) */
  luai_lu_byte idx;  /* index of upvalue (in stack or in outer function's list) */
} luai_Upvaldesc;


/*
** Description of a local variable for function prototypes
** (used for debug information)
*/
typedef struct luai_LocVar {
  luai_TString *varname;
  int startpc;  /* first point where variable is active */
  int endpc;    /* first point where variable is dead */
} luai_LocVar;


/*
** Function Prototypes
*/
typedef struct luai_Proto {
  luai_CommonHeader;
  luai_lu_byte numparams;  /* number of fixed parameters */
  luai_lu_byte is_vararg;
  luai_lu_byte maxstacksize;  /* number of registers needed by this function */
  int sizeupvalues;  /* size of 'upvalues' */
  int sizek;  /* size of 'k' */
  int sizecode;
  int sizelineinfo;
  int sizep;  /* size of 'p' */
  int sizelocvars;
  int linedefined;  /* debug information  */
  int lastlinedefined;  /* debug information  */
  luai_TValue *k;  /* constants used by the function */
  Instruction *code;  /* opcodes */
  struct luai_Proto **p;  /* functions defined inside the function */
  int *lineinfo;  /* map from opcodes to source lines (debug information) */
  luai_LocVar *locvars;  /* information about local variables (debug information) */
  luai_Upvaldesc *upvalues;  /* upvalue information */
  struct luai_LClosure *cache;  /* last-created closure with this prototype */
  luai_TString  *source;  /* used for debug information */
  LUAI_GCObject *gclist;
} luai_Proto;



/*
** Lua Upvalues
*/
typedef struct luai_UpVal luai_UpVal;


/*
** Closures
*/

#define luai_ClosureHeader \
	luai_CommonHeader; luai_lu_byte nupvalues; LUAI_GCObject *gclist

typedef struct luai_CClosure {
  luai_ClosureHeader;
  lua_CFunction f;
  luai_TValue upvalue[1];  /* list of upvalues */
} luai_CClosure;


typedef struct luai_LClosure {
  luai_ClosureHeader;
  struct luai_Proto *p;
  luai_UpVal *upvals[1];  /* list of upvalues */
} luai_LClosure;


typedef union luai_Closure {
  luai_CClosure c;
  luai_LClosure l;
} luai_Closure;


#define luai_isLfunction(o)	luai_ttisLclosure(o)

#define luai_getproto(o)	(luai_clLvalue(o)->p)


/*
** Tables
*/

typedef union LUAI_TKey {
  struct {
    luai_TValuefields;
    int luai_next;  /* for chaining (offset for luai_next node) */
  } nk;
  luai_TValue tvk;
} LUAI_TKey;


/* copy a value into a key without messing up luai_field 'luai_next' */
#define luai_setnodekey(L,key,obj) \
	{ LUAI_TKey *k_=(key); const luai_TValue *luai_io_=(obj); \
	  k_->nk.value_ = luai_io_->value_; k_->nk.tt_ = luai_io_->tt_; \
	  (void)L; luai_checkliveness(L,luai_io_); }


typedef struct luai_Node {
  luai_TValue i_val;
  LUAI_TKey i_key;
} luai_Node;


typedef struct luai_Table {
  luai_CommonHeader;
  luai_lu_byte flags;  /* 1<<p means tagmethod(p) is not present */
  luai_lu_byte lsizenode;  /* log2 of size of 'node' array */
  unsigned int sizearray;  /* size of 'array' array */
  luai_TValue *array;  /* array part */
  luai_Node *node;
  luai_Node *lastfree;  /* any free position is before this position */
  struct luai_Table *metatable;
  LUAI_GCObject *gclist;
} luai_Table;



/*
** 'module' operation for hashing (size is always a power of 2)
*/
#define luai_lmod(s,size) \
	(luai_check_exp((size&(size-1))==0, (luai_cast(int, (s) & ((size)-1)))))


#define luai_twoto(x)	(1<<(x))
#define luai_sizenode(t)	(luai_twoto((t)->lsizenode))


/*
** (address of) a fixed nil value
*/
#define luaO_nilobject		(&luaO_nilobject_)


LUAI_DDEC const luai_TValue luaO_nilobject_;

/* size of buffer for 'luaO_utf8esc' function */
#define LUAI_UTF8BUFFSZ	8

LUAI_FUNC int luaO_int2fb (unsigned int x);
LUAI_FUNC int luaO_fb2int (int x);
LUAI_FUNC int luaO_utf8esc (char *buff, unsigned long x);
LUAI_FUNC int luaO_ceillog2 (unsigned int x);
LUAI_FUNC void luaO_arith (lua_State *L, int op, const luai_TValue *p1,
                           const luai_TValue *p2, luai_TValue *res);
LUAI_FUNC size_t luaO_str2num (const char *s, luai_TValue *o);
LUAI_FUNC int luaO_hexavalue (int c);
LUAI_FUNC void luaO_tostring (lua_State *L, luai_StkId obj);
LUAI_FUNC const char *luaO_pushvfstring (lua_State *L, const char *fmt,
                                                       va_list argp);
LUAI_FUNC const char *luaO_pushfstring (lua_State *L, const char *fmt, ...);
LUAI_FUNC void luaO_chunkid (char *out, const char *source, size_t len);

/*__ltm.h__*/

/*
* WARNING: if you change the order of this enumeration,
* grep "ORDER TM" and "ORDER OP"
*/
typedef enum {
  LUAI_TM_INDEX,
  LUAI_TM_NEWINDEX,
  LUAI_TM_GC,
  LUAI_TM_MODE,
  LUAI_TM_LEN,
  LUAI_TM_EQ,  /* last tag method with fast access */
  LUAI_TM_ADD,
  LUAI_TM_SUB,
  LUAI_TM_MUL,
  LUAI_TM_MOD,
  LUAI_TM_POW,
  LUAI_TM_DIV,
  LUAI_TM_IDIV,
  LUAI_TM_BAND,
  LUAI_TM_BOR,
  LUAI_TM_BXOR,
  LUAI_TM_SHL,
  LUAI_TM_SHR,
  LUAI_TM_UNM,
  LUAI_TM_BNOT,
  LUAI_TM_LT,
  LUAI_TM_LE,
  LUAI_TM_CONCAT,
  LUAI_TM_CALL,
  LUAI_TM_N		/* number of elements in the enum */
} luai_TMS;



#define luai_gfasttm(g,et,e) ((et) == NULL ? NULL : \
  ((et)->flags & (1u<<(e))) ? NULL : luaT_gettm(et, e, (g)->tmname[e]))

#define luai_fasttm(l,et,e)	luai_gfasttm(LUAI_G(l), et, e)

#define luai_ttypename(x)	luaT_typenames_[(x) + 1]

LUAI_DDEC const char *const luaT_typenames_[LUA_TOTALTAGS];


LUAI_FUNC const char *luaT_objtypename (lua_State *L, const luai_TValue *o);

LUAI_FUNC const luai_TValue *luaT_gettm (luai_Table *events, luai_TMS event, luai_TString *ename);
LUAI_FUNC const luai_TValue *luaT_gettmbyobj (lua_State *L, const luai_TValue *o,
                                                       luai_TMS event);
LUAI_FUNC void luaT_init (lua_State *L);

LUAI_FUNC void luaT_callTM (lua_State *L, const luai_TValue *f, const luai_TValue *p1,
                            const luai_TValue *p2, luai_TValue *p3, int hasres);
LUAI_FUNC int luaT_callbinTM (lua_State *L, const luai_TValue *p1, const luai_TValue *p2,
                              luai_StkId res, luai_TMS event);
LUAI_FUNC void luaT_trybinTM (lua_State *L, const luai_TValue *p1, const luai_TValue *p2,
                              luai_StkId res, luai_TMS event);
LUAI_FUNC int luaT_callorderTM (lua_State *L, const luai_TValue *p1,
                                const luai_TValue *p2, luai_TMS event);


/*__lmem.h__*/

/*
** This macro reallocs a vector 'b' from 'on' to 'n' elements, where
** each element has size 'e'. In case of arithmetic overflow of the
** product 'n'*'e', it raises an error (calling 'luaM_toobig'). Because
** 'e' is always constant, it avoids the runtime division LUAI_MAX_SIZET/(e).
**
** (The macro is somewhat complex to avoid warnings:  The 'sizeof'
** comparison avoids a runtime comparison when overflow cannot occur.
** The compiler should be able to optimize the real test by itself, but
** when it does it, it may give a warning about "comparison is always
** false due to limited range of data type"; the +1 tricks the compiler,
** avoiding this warning but also this optimization.)
*/
#define luaM_reallocv(L,b,on,n,e) \
  (((sizeof(n) >= sizeof(size_t) && luai_cast(size_t, (n)) + 1 > LUAI_MAX_SIZET/(e)) \
      ? luaM_toobig(L) : luai_cast_void(0)) , \
   luaM_realloc_(L, (b), (on)*(e), (n)*(e)))

/*
** Arrays of chars do not need any test
*/
#define luaM_reallocvchar(L,b,on,n)  \
    luai_cast(char *, luaM_realloc_(L, (b), (on)*sizeof(char), (n)*sizeof(char)))

#define luaM_freemem(L, b, s)	luaM_realloc_(L, (b), (s), 0)
#define luaM_free(L, b)		luaM_realloc_(L, (b), sizeof(*(b)), 0)
#define luaM_freearray(L, b, n)   luaM_realloc_(L, (b), (n)*sizeof(*(b)), 0)

#define luaM_malloc(L,s)	luaM_realloc_(L, NULL, 0, (s))
#define luaM_new(L,t)		luai_cast(t *, luaM_malloc(L, sizeof(t)))
#define luaM_newvector(L,n,t) \
		luai_cast(t *, luaM_reallocv(L, NULL, 0, n, sizeof(t)))

#define luaM_newobject(L,tag,s)	luaM_realloc_(L, NULL, tag, (s))

#define luaM_growvector(L,v,nelems,size,t,limit,e) \
          if ((nelems)+1 > (size)) \
            ((v)=luai_cast(t *, luaM_growaux_(L,v,&(size),sizeof(t),limit,e)))

#define luaM_reallocvector(L, v,oldn,n,t) \
   ((v)=luai_cast(t *, luaM_reallocv(L, v, oldn, n, sizeof(t))))

LUAI_FUNC luai_l_noret luaM_toobig (lua_State *L);

/* not to be called directly */
LUAI_FUNC void *luaM_realloc_ (lua_State *L, void *luai_getblock, size_t oldsize,
                                                          size_t size);
LUAI_FUNC void *luaM_growaux_ (lua_State *L, void *luai_getblock, int *size,
                               size_t size_elem, int limit,
                               const char *what);

/*__lzio.h__*/

#define LUAI_EOZ	(-1)			/* end of stream */

typedef struct luai_Zio LUAI_ZIO;

#define luai_zgetc(z)  (((z)->n--)>0 ?  luai_cast_uchar(*(z)->p++) : luaZ_fill(z))


typedef struct luai_Mbuffer {
  char *buffer;
  size_t n;
  size_t buffsize;
} luai_Mbuffer;

#define luaZ_initbuffer(L, buff) ((buff)->buffer = NULL, (buff)->buffsize = 0)

#define luaZ_buffer(buff)	((buff)->buffer)
#define luaZ_sizebuffer(buff)	((buff)->buffsize)
#define luaZ_bufflen(buff)	((buff)->n)

#define luaZ_buffremove(buff,i)	((buff)->n -= (i))
#define luaZ_resetbuffer(buff) ((buff)->n = 0)


#define luaZ_resizebuffer(L, buff, size) \
	((buff)->buffer = luaM_reallocvchar(L, (buff)->buffer, \
				(buff)->buffsize, size), \
	(buff)->buffsize = size)

#define luaZ_freebuffer(L, buff)	luaZ_resizebuffer(L, buff, 0)


LUAI_FUNC void luaZ_init (lua_State *L, LUAI_ZIO *z, lua_Reader reader,
                                        void *data);
LUAI_FUNC size_t luaZ_read (LUAI_ZIO* z, void *b, size_t n);	/* read luai_next n bytes */



/* --------- Private Part ------------------ */

struct luai_Zio {
  size_t n;			/* bytes still unread */
  const char *p;		/* current position in buffer */
  lua_Reader reader;		/* reader function */
  void *data;			/* additional data */
  lua_State *L;			/* Lua state (for reader) */
};


LUAI_FUNC int luaZ_fill (LUAI_ZIO *z);

/*__lstate.h__*/

/*

** Some notes about garbage-collected objects: All objects in Lua must
** be kept somehow accessible until being freed, so all objects always
** belong to one (and only one) of these lists, using luai_field 'luai_next' of
** the 'luai_CommonHeader' for the link:
**
** 'allgc': all objects not marked for finalization;
** 'finobj': all objects marked for finalization;
** 'tobefnz': all objects ready to be finalized;
** 'fixedgc': all objects that are not to be collected (currently
** only small strings, such as reserved words).

*/


struct lua_longjmp;  /* defined in ldo.c */


/*
** Atomic type (relative to signals) to better ensure that 'lua_sethook'
** is thread safe
*/
#if !defined(luai_l_signalT)
#include <signal.h>
#define luai_l_signalT	sig_atomic_t
#endif


/* extra stack space to handle TM calls and some other extras */
#define LUAI_EXTRA_STACK   5


#define LUAI_BASIC_STACK_SIZE        (2*LUA_MINSTACK)


/* kinds of Garbage Collection */
#define LUAI_KGC_NORMAL	0
#define LUAI_KGC_EMERGENCY	1	/* gc was forced by an allocation failure */


typedef struct luai_stringtable {
  luai_TString **hash;
  int nuse;  /* number of elements */
  int size;
} luai_stringtable;


/*
** Information about a call.
** When a thread yields, 'func' is adjusted to pretend that the
** top function has only the yielded values in its stack; in that
** case, the actual 'func' value is saved in luai_field 'extra'.
** When a function calls another with a continuation, 'extra' keeps
** the function index so that, in case of errors, the continuation
** function can be called with the correct top.
*/
typedef struct luai_CallInfo {
  luai_StkId func;  /* function index in the stack */
  luai_StkId	top;  /* top for this function */
  struct luai_CallInfo *previous, *luai_next;  /* dynamic call link */
  union {
    struct {  /* only for Lua functions */
      luai_StkId base;  /* base for this function */
      const Instruction *savedpc;
    } l;
    struct {  /* only for C functions */
      lua_KFunction k;  /* continuation in case of yields */
      ptrdiff_t old_errfunc;
      lua_KContext ctx;  /* context info. in case of yields */
    } c;
  } u;
  ptrdiff_t extra;
  short nresults;  /* expected number of results from this function */
  unsigned short callstatus;
} luai_CallInfo;


/*
** Bits in luai_CallInfo status
*/
#define LUAI_CIST_OAH	(1<<0)	/* original value of 'allowhook' */
#define LUAI_CIST_LUA	(1<<1)	/* call is running a Lua function */
#define LUAI_CIST_HOOKED	(1<<2)	/* call is running a debug hook */
#define LUAI_CIST_FRESH	(1<<3)	/* call is running on a fresh invocation
                                   of luaV_execute */
#define LUAI_CIST_YPCALL	(1<<4)	/* call is a yieldable protected call */
#define LUAI_CIST_TAIL	(1<<5)	/* call was tail called */
#define LUAI_CIST_HOOKYIELD	(1<<6)	/* last hook called yielded */
#define LUAI_CIST_LEQ	(1<<7)  /* using __lt for __le */
#define LUAI_CIST_FIN	(1<<8)  /* call is running a finalizer */

#define luai_isLua(ci)	((ci)->callstatus & LUAI_CIST_LUA)

/* assume that LUAI_CIST_OAH has offset 0 and that 'v' is strictly 0/1 */
#define luai_setoah(st,v)	((st) = ((st) & ~LUAI_CIST_OAH) | (v))
#define luai_getoah(st)	((st) & LUAI_CIST_OAH)


/*
** 'global state', shared by all threads of this state
*/
typedef struct luai_global_State {
  lua_Alloc frealloc;  /* function to reallocate memory */
  void *ud;         /* auxiliary data to 'frealloc' */
  luai_l_mem totalbytes;  /* number of bytes currently allocated - LUAI_GCdebt */
  luai_l_mem LUAI_GCdebt;  /* bytes allocated not yet compensated by the collector */
  luai_lu_mem LUAI_GCmemtrav;  /* memory traversed by the LUAI_GC */
  luai_lu_mem LUAI_GCestimate;  /* an estimate of the non-garbage memory in use */
  luai_stringtable strt;  /* hash table for strings */
  luai_TValue luai_l_registry;
  unsigned int seed;  /* randomized seed for hashes */
  luai_lu_byte currentwhite;
  luai_lu_byte gcstate;  /* state of garbage collector */
  luai_lu_byte gckind;  /* kind of LUAI_GC running */
  luai_lu_byte gcrunning;  /* true if LUAI_GC is running */
  LUAI_GCObject *allgc;  /* list of all collectable objects */
  LUAI_GCObject **sweepgc;  /* current position of sweep in list */
  LUAI_GCObject *finobj;  /* list of collectable objects with finalizers */
  LUAI_GCObject *gray;  /* list of gray objects */
  LUAI_GCObject *grayagain;  /* list of objects to be traversed atomically */
  LUAI_GCObject *weak;  /* list of tables with weak values */
  LUAI_GCObject *ephemeron;  /* list of ephemeron tables (weak keys) */
  LUAI_GCObject *allweak;  /* list of all-weak tables */
  LUAI_GCObject *tobefnz;  /* list of userdata to be LUAI_GC */
  LUAI_GCObject *fixedgc;  /* list of objects not to be collected */
  struct lua_State *twups;  /* list of threads with open upvalues */
  unsigned int gcfinnum;  /* number of finalizers to call in each LUAI_GC step */
  int gcpause;  /* size of pause between successive LUAI_GCs */
  int gcstepmul;  /* LUAI_GC 'granularity' */
  lua_CFunction panic;  /* to be called in unprotected errors */
  struct lua_State *mainthread;
  const lua_Number *version;  /* pointer to version number */
  luai_TString *memerrmsg;  /* memory-error message */
  luai_TString *tmname[LUAI_TM_N];  /* array with tag-method names */
  struct luai_Table *mt[LUA_NUMTAGS];  /* metatables for basic types */
  luai_TString *strcache[LUAI_STRCACHE_N][LUAI_STRCACHE_M];  /* cache for strings in API */
} luai_global_State;


/*
** 'per thread' state
*/
struct lua_State {
  luai_CommonHeader;
  unsigned short nci;  /* number of items in 'ci' list */
  luai_lu_byte status;
  luai_StkId top;  /* first free slot in the stack */
  luai_global_State *luai_l_G;
  luai_CallInfo *ci;  /* call info for current function */
  const Instruction *oldpc;  /* last pc traced */
  luai_StkId stack_last;  /* last free slot in the stack */
  luai_StkId stack;  /* stack base */
  luai_UpVal *openupval;  /* list of open upvalues in this stack */
  LUAI_GCObject *gclist;
  struct lua_State *twups;  /* list of threads with open upvalues */
  struct lua_longjmp *errorJmp;  /* current error luai_recover point */
  luai_CallInfo base_ci;  /* luai_CallInfo for first level (C calling Lua) */
  volatile lua_Hook hook;
  ptrdiff_t errfunc;  /* current error handling function (stack index) */
  int stacksize;
  int basehookcount;
  int hookcount;
  unsigned short nny;  /* number of non-yieldable calls in stack */
  unsigned short nCcalls;  /* number of nested C calls */
  luai_l_signalT hookmask;
  luai_lu_byte allowhook;
};


#define LUAI_G(L)	(L->luai_l_G)


/*
** Union of all collectable objects (only for conversions)
*/
union LUAI_GCUnion {
  LUAI_GCObject gc;  /* common header */
  struct luai_TString ts;
  struct luai_Udata u;
  union luai_Closure cl;
  struct luai_Table h;
  struct luai_Proto p;
  struct lua_State th;  /* thread */
};


#define luai_cast_u(o)	luai_cast(union LUAI_GCUnion *, (o))

/* macros to convert a LUAI_GCObject into a specific value */
#define luai_gco2ts(o)  \
	luai_check_exp(luai_novariant((o)->tt) == LUA_TSTRING, &((luai_cast_u(o))->ts))
#define luai_gco2u(o)  luai_check_exp((o)->tt == LUA_TUSERDATA, &((luai_cast_u(o))->u))
#define luai_gco2lcl(o)  luai_check_exp((o)->tt == LUA_TLCL, &((luai_cast_u(o))->cl.l))
#define luai_gco2ccl(o)  luai_check_exp((o)->tt == LUA_TCCL, &((luai_cast_u(o))->cl.c))
#define luai_gco2cl(o)  \
	luai_check_exp(luai_novariant((o)->tt) == LUA_TFUNCTION, &((luai_cast_u(o))->cl))
#define luai_gco2t(o)  luai_check_exp((o)->tt == LUA_TTABLE, &((luai_cast_u(o))->h))
#define luai_gco2p(o)  luai_check_exp((o)->tt == LUA_TPROTO, &((luai_cast_u(o))->p))
#define luai_gco2th(o)  luai_check_exp((o)->tt == LUA_TTHREAD, &((luai_cast_u(o))->th))


/* macro to convert a Lua object into a LUAI_GCObject */
#define luai_obj2gco(v) \
	luai_check_exp(luai_novariant((v)->tt) < LUA_TDEADKEY, (&(luai_cast_u(v)->gc)))


/* actual number of total bytes allocated */
#define luai_gettotalbytes(g)	luai_cast(luai_lu_mem, (g)->totalbytes + (g)->LUAI_GCdebt)

LUAI_FUNC void luaE_setdebt (luai_global_State *g, luai_l_mem debt);
LUAI_FUNC void luaE_freethread (lua_State *L, lua_State *L1);
LUAI_FUNC luai_CallInfo *luaE_extendCI (lua_State *L);
LUAI_FUNC void luaE_freeCI (lua_State *L);
LUAI_FUNC void luaE_shrinkCI (lua_State *L);

/*__lapi.h__*/

#define luai_api_incr_top(L)   {L->top++; luai_api_check(L, L->top <= L->ci->top, \
				"stack overflow");}

#define luai_adjustresults(L,nres) \
    { if ((nres) == LUA_MULTRET && L->ci->top < L->top) L->ci->top = L->top; }

#define luai_api_checknelems(L,n)	luai_api_check(L, (n) < (L->top - L->ci->func), \
				  "not enough elements in the stack")

/*__llex.h__*/

#define LUAI_FIRST_RESERVED	257


#if !defined(LUA_ENV)
#define LUA_ENV		"_ENV"
#endif


/*
* WARNING: if you change the order of this enumeration,
* grep "ORDER LUAI_RESERVED"
*/
enum LUAI_RESERVED {
  /* terminal symbols denoted by reserved words */
  LUAI_TK_AND = LUAI_FIRST_RESERVED, LUAI_TK_BREAK,
  LUAI_TK_DO, LUAI_TK_ELSE, LUAI_TK_ELSEIF, LUAI_TK_END, LUAI_TK_FALSE, LUAI_TK_FOR, LUAI_TK_FUNCTION,
  LUAI_TK_GOTO, LUAI_TK_IF, LUAI_TK_IN, LUAI_TK_LOCAL, LUAI_TK_NIL, LUAI_TK_NOT, LUAI_TK_OR, LUAI_TK_REPEAT,
  LUAI_TK_RETURN, LUAI_TK_THEN, LUAI_TK_TRUE, LUAI_TK_UNTIL, LUAI_TK_WHILE,
  /* other terminal symbols */
  LUAI_TK_IDIV, LUAI_TK_CONCAT, LUAI_TK_DOTS, LUAI_TK_EQ, LUAI_TK_GE, LUAI_TK_LE, LUAI_TK_NE,
  LUAI_TK_SHL, LUAI_TK_SHR,
  LUAI_TK_DBCOLON, LUAI_TK_EOS,
  LUAI_TK_FLT, LUAI_TK_INT, LUAI_TK_NAME, LUAI_TK_STRING
};

/* number of reserved words */
#define LUAI_NUM_RESERVED	(luai_cast(int, LUAI_TK_WHILE-LUAI_FIRST_RESERVED+1))


typedef union {
  lua_Number r;
  lua_Integer i;
  luai_TString *ts;
} luai_SemInfo;  /* semantics information */


typedef struct luai_Token {
  int token;
  luai_SemInfo seminfo;
} luai_Token;


/* state of the lexer plus state of the parser when shared by all
   functions */
typedef struct luai_LexState {
  int current;  /* current character (charint) */
  int linenumber;  /* input line counter */
  int lastline;  /* line of last token 'consumed' */
  luai_Token t;  /* current token */
  luai_Token lookahead;  /* look ahead token */
  struct luai_FuncState *fs;  /* current function (parser) */
  struct lua_State *L;
  LUAI_ZIO *z;  /* input stream */
  luai_Mbuffer *buff;  /* buffer for tokens */
  luai_Table *h;  /* to avoid collection/reuse strings */
  struct luai_Dyndata *dyd;  /* dynamic structures used by the parser */
  luai_TString *source;  /* current source name */
  luai_TString *envn;  /* environment variable name */
} luai_LexState;


LUAI_FUNC void luaX_init (lua_State *L);
LUAI_FUNC void luaX_setinput (lua_State *L, luai_LexState *ls, LUAI_ZIO *z,
                              luai_TString *source, int firstchar);
LUAI_FUNC luai_TString *luaX_newstring (luai_LexState *ls, const char *str, size_t l);
LUAI_FUNC void luaX_next (luai_LexState *ls);
LUAI_FUNC int luaX_lookahead (luai_LexState *ls);
LUAI_FUNC luai_l_noret luaX_syntaxerror (luai_LexState *ls, const char *s);
LUAI_FUNC const char *luaX_token2str (luai_LexState *ls, int token);

/*__lopcodes.h__*/

/*===========================================================================
  We assume that instructions are unsigned numbers.
  All instructions have an opcode in the first 6 bits.
  Instructions can have the following fields:
	'A' : 8 bits
	'B' : 9 bits
	'C' : 9 bits
	'Ax' : 26 bits ('A', 'B', and 'C' together)
	'Bx' : 18 bits ('B' and 'C' together)
	'sBx' : signed Bx

  A signed argument is represented in excess K; that is, the number
  value is the unsigned value minus K. K is exactly the maximum value
  for that argument (so that -max is represented by 0, and +max is
  represented by 2*max), which is half the maximum for the corresponding
  unsigned argument.
===========================================================================*/


enum luai_OpMode {luai_iABC, luai_iABx, luai_iAsBx, luai_iAx};  /* basic instruction format */


/*
** size and position of opcode arguments.
*/
#define luai_SIZE_C		9
#define luai_SIZE_B		9
#define luai_SIZE_Bx		(luai_SIZE_C + luai_SIZE_B)
#define luai_SIZE_A		8
#define luai_SIZE_Ax		(luai_SIZE_C + luai_SIZE_B + luai_SIZE_A)

#define luai_SIZE_OP		6

#define luai_POS_OP		0
#define luai_POS_A		(luai_POS_OP + luai_SIZE_OP)
#define luai_POS_C		(luai_POS_A + luai_SIZE_A)
#define luai_POS_B		(luai_POS_C + luai_SIZE_C)
#define luai_POS_Bx		luai_POS_C
#define luai_POS_Ax		luai_POS_A


/*
** limits for opcode arguments.
** we use (signed) int to manipulate most arguments,
** so they must fit in LUAI_BITSINT-1 bits (-1 for sign)
*/
#if luai_SIZE_Bx < LUAI_BITSINT-1
#define luai_MAXARG_Bx        ((1<<luai_SIZE_Bx)-1)
#define luai_MAXARG_sBx        (luai_MAXARG_Bx>>1)         /* 'sBx' is signed */
#else
#define luai_MAXARG_Bx        LUAI_MAX_INT
#define luai_MAXARG_sBx        LUAI_MAX_INT
#endif

#if luai_SIZE_Ax < LUAI_BITSINT-1
#define luai_MAXARG_Ax	((1<<luai_SIZE_Ax)-1)
#else
#define luai_MAXARG_Ax	LUAI_MAX_INT
#endif


#define luai_MAXARG_A        ((1<<luai_SIZE_A)-1)
#define luai_MAXARG_B        ((1<<luai_SIZE_B)-1)
#define luai_MAXARG_C        ((1<<luai_SIZE_C)-1)


/* creates a mask with 'n' 1 bits at position 'p' */
#define luai_MASK1(n,p)	((~((~(Instruction)0)<<(n)))<<(p))

/* creates a mask with 'n' 0 bits at position 'p' */
#define luai_MASK0(n,p)	(~luai_MASK1(n,p))

/*
** the following macros help to manipulate instructions
*/

#define luai_GETOPCODE(i)	(luai_cast(luai_OpCode, ((i)>>luai_POS_OP) & luai_MASK1(luai_SIZE_OP,0)))
#define luai_SETOPCODE(i,o)	((i) = (((i)&luai_MASK0(luai_SIZE_OP,luai_POS_OP)) | \
		((luai_cast(Instruction, o)<<luai_POS_OP)&luai_MASK1(luai_SIZE_OP,luai_POS_OP))))

#define luai_getarg(i,pos,size)	(luai_cast(int, ((i)>>pos) & luai_MASK1(size,0)))
#define luai_setarg(i,v,pos,size)	((i) = (((i)&luai_MASK0(size,pos)) | \
                ((luai_cast(Instruction, v)<<pos)&luai_MASK1(size,pos))))

#define luai_GETARG_A(i)	luai_getarg(i, luai_POS_A, luai_SIZE_A)
#define luai_SETARG_A(i,v)	luai_setarg(i, v, luai_POS_A, luai_SIZE_A)

#define luai_GETARG_B(i)	luai_getarg(i, luai_POS_B, luai_SIZE_B)
#define luai_SETARG_B(i,v)	luai_setarg(i, v, luai_POS_B, luai_SIZE_B)

#define luai_GETARG_C(i)	luai_getarg(i, luai_POS_C, luai_SIZE_C)
#define luai_SETARG_C(i,v)	luai_setarg(i, v, luai_POS_C, luai_SIZE_C)

#define luai_GETARG_Bx(i)	luai_getarg(i, luai_POS_Bx, luai_SIZE_Bx)
#define luai_SETARG_Bx(i,v)	luai_setarg(i, v, luai_POS_Bx, luai_SIZE_Bx)

#define luai_GETARG_Ax(i)	luai_getarg(i, luai_POS_Ax, luai_SIZE_Ax)
#define luai_SETARG_Ax(i,v)	luai_setarg(i, v, luai_POS_Ax, luai_SIZE_Ax)

#define luai_GETARG_sBx(i)	(luai_GETARG_Bx(i)-luai_MAXARG_sBx)
#define luai_SETARG_sBx(i,b)	luai_SETARG_Bx((i),luai_cast(unsigned int, (b)+luai_MAXARG_sBx))


#define luai_CREATE_ABC(o,a,b,c)	((luai_cast(Instruction, o)<<luai_POS_OP) \
			| (luai_cast(Instruction, a)<<luai_POS_A) \
			| (luai_cast(Instruction, b)<<luai_POS_B) \
			| (luai_cast(Instruction, c)<<luai_POS_C))

#define luai_CREATE_ABx(o,a,bc)	((luai_cast(Instruction, o)<<luai_POS_OP) \
			| (luai_cast(Instruction, a)<<luai_POS_A) \
			| (luai_cast(Instruction, bc)<<luai_POS_Bx))

#define luai_CREATE_Ax(o,a)		((luai_cast(Instruction, o)<<luai_POS_OP) \
			| (luai_cast(Instruction, a)<<luai_POS_Ax))


/*
** Macros to operate RK indices
*/

/* this bit 1 means constant (0 means register) */
#define luai_BITRK		(1 << (luai_SIZE_B - 1))

/* test whether value is a constant */
#define luai_ISK(x)		((x) & luai_BITRK)

/* gets the index of the constant */
#define luai_INDEXK(r)	((int)(r) & ~luai_BITRK)

#if !defined(luai_MAXINDEXRK)  /* (for debugging only) */
#define luai_MAXINDEXRK	(luai_BITRK - 1)
#endif

/* code a constant index as a RK value */
#define luai_RKASK(x)	((x) | luai_BITRK)


/*
** invalid register that fits in 8 bits
*/
#define luai_NO_REG		luai_MAXARG_A


/*
** R(x) - register
** Kst(x) - constant (in constant table)
** RK(x) == if luai_ISK(x) then Kst(luai_INDEXK(x)) else R(x)
*/


/*
** grep "ORDER OP" if you change these enums
*/

typedef enum {
/*----------------------------------------------------------------------
name		args	description
------------------------------------------------------------------------*/
luai_OP_MOVE,/*	A B	R(A) := R(B)					*/
luai_OP_LOADK,/*	A Bx	R(A) := Kst(Bx)					*/
luai_OP_LOADKX,/*	A 	R(A) := Kst(extra arg)				*/
luai_OP_LOADBOOL,/*	A B C	R(A) := (Bool)B; if (C) pc++			*/
luai_OP_LOADNIL,/*	A B	R(A), R(A+1), ..., R(A+B) := nil		*/
luai_OP_GETUPVAL,/*	A B	R(A) := UpValue[B]				*/

luai_OP_GETTABUP,/*	A B C	R(A) := UpValue[B][RK(C)]			*/
luai_OP_GETTABLE,/*	A B C	R(A) := R(B)[RK(C)]				*/

luai_OP_SETTABUP,/*	A B C	UpValue[A][RK(B)] := RK(C)			*/
luai_OP_SETUPVAL,/*	A B	UpValue[B] := R(A)				*/
luai_OP_SETTABLE,/*	A B C	R(A)[RK(B)] := RK(C)				*/

luai_OP_NEWTABLE,/*	A B C	R(A) := {} (size = B,C)				*/

luai_OP_SELF,/*	A B C	R(A+1) := R(B); R(A) := R(B)[RK(C)]		*/

luai_OP_ADD,/*	A B C	R(A) := RK(B) + RK(C)				*/
luai_OP_SUB,/*	A B C	R(A) := RK(B) - RK(C)				*/
luai_OP_MUL,/*	A B C	R(A) := RK(B) * RK(C)				*/
luai_OP_MOD,/*	A B C	R(A) := RK(B) % RK(C)				*/
luai_OP_POW,/*	A B C	R(A) := RK(B) ^ RK(C)				*/
luai_OP_DIV,/*	A B C	R(A) := RK(B) / RK(C)				*/
luai_OP_IDIV,/*	A B C	R(A) := RK(B) // RK(C)				*/
luai_OP_BAND,/*	A B C	R(A) := RK(B) & RK(C)				*/
luai_OP_BOR,/*	A B C	R(A) := RK(B) | RK(C)				*/
luai_OP_BXOR,/*	A B C	R(A) := RK(B) ~ RK(C)				*/
luai_OP_SHL,/*	A B C	R(A) := RK(B) << RK(C)				*/
luai_OP_SHR,/*	A B C	R(A) := RK(B) >> RK(C)				*/
luai_OP_UNM,/*	A B	R(A) := -R(B)					*/
luai_OP_BNOT,/*	A B	R(A) := ~R(B)					*/
luai_OP_NOT,/*	A B	R(A) := not R(B)				*/
luai_OP_LEN,/*	A B	R(A) := length of R(B)				*/

luai_OP_CONCAT,/*	A B C	R(A) := R(B).. ... ..R(C)			*/

luai_OP_JMP,/*	A sBx	pc+=sBx; if (A) close all upvalues >= R(A - 1)	*/
luai_OP_EQ,/*	A B C	if ((RK(B) == RK(C)) ~= A) then pc++		*/
luai_OP_LT,/*	A B C	if ((RK(B) <  RK(C)) ~= A) then pc++		*/
luai_OP_LE,/*	A B C	if ((RK(B) <= RK(C)) ~= A) then pc++		*/

luai_OP_TEST,/*	A C	if not (R(A) <=> C) then pc++			*/
luai_OP_TESTSET,/*	A B C	if (R(B) <=> C) then R(A) := R(B) else pc++	*/

luai_OP_CALL,/*	A B C	R(A), ... ,R(A+C-2) := R(A)(R(A+1), ... ,R(A+B-1)) */
luai_OP_TAILCALL,/*	A B C	return R(A)(R(A+1), ... ,R(A+B-1))		*/
luai_OP_RETURN,/*	A B	return R(A), ... ,R(A+B-2)	(see note)	*/

luai_OP_FORLOOP,/*	A sBx	R(A)+=R(A+2);
			if R(A) <?= R(A+1) then { pc+=sBx; R(A+3)=R(A) }*/
luai_OP_FORPREP,/*	A sBx	R(A)-=R(A+2); pc+=sBx				*/

luai_OP_TFORCALL,/*	A C	R(A+3), ... ,R(A+2+C) := R(A)(R(A+1), R(A+2));	*/
luai_OP_TFORLOOP,/*	A sBx	if R(A+1) ~= nil then { R(A)=R(A+1); pc += sBx }*/

luai_OP_SETLIST,/*	A B C	R(A)[(C-1)*FPF+i] := R(A+i), 1 <= i <= B	*/

luai_OP_CLOSURE,/*	A Bx	R(A) := closure(KPROTO[Bx])			*/

luai_OP_VARARG,/*	A B	R(A), R(A+1), ..., R(A+B-2) = vararg		*/

luai_OP_EXTRAARG/*	Ax	extra (larger) argument for previous opcode	*/
} luai_OpCode;


#define luai_NUM_OPCODES	(luai_cast(int, luai_OP_EXTRAARG) + 1)



/*===========================================================================
  Notes:
  (*) In luai_OP_CALL, if (B == 0) then B = top. If (C == 0), then 'top' is
  set to last_result+1, so luai_next open instruction (luai_OP_CALL, luai_OP_RETURN,
  luai_OP_SETLIST) may use 'top'.

  (*) In luai_OP_VARARG, if (B == 0) then use actual number of varargs and
  set top (like in luai_OP_CALL with C == 0).

  (*) In luai_OP_RETURN, if (B == 0) then return up to 'top'.

  (*) In luai_OP_SETLIST, if (B == 0) then B = 'top'; if (C == 0) then luai_next
  'instruction' is EXTRAARG(real C).

  (*) In luai_OP_LOADKX, the luai_next 'instruction' is always EXTRAARG.

  (*) For comparisons, A specifies what condition the test should accept
  (true or false).

  (*) All 'skips' (pc++) assume that luai_next instruction is a jump.

===========================================================================*/


/*
** luai_masks for instruction properties. The format is:
** bits 0-1: op mode
** bits 2-3: C arg mode
** bits 4-5: B arg mode
** bit 6: instruction set register A
** bit 7: operator is a test (luai_next instruction must be a jump)
*/

enum luai_OpArgMask {
  luai_OpArgN,  /* argument is not used */
  luai_OpArgU,  /* argument is used */
  luai_OpArgR,  /* argument is a register or a jump offset */
  luai_OpArgK   /* argument is a constant or register/constant */
};

LUAI_DDEC const luai_lu_byte luaP_opmodes[luai_NUM_OPCODES];

#define luai_getOpMode(m)	(luai_cast(enum luai_OpMode, luaP_opmodes[m] & 3))
#define luai_getBMode(m)	(luai_cast(enum luai_OpArgMask, (luaP_opmodes[m] >> 4) & 3))
#define luai_getCMode(m)	(luai_cast(enum luai_OpArgMask, (luaP_opmodes[m] >> 2) & 3))
#define luai_testAMode(m)	(luaP_opmodes[m] & (1 << 6))
#define luai_testTMode(m)	(luaP_opmodes[m] & (1 << 7))


LUAI_DDEC const char *const luaP_opnames[luai_NUM_OPCODES+1];  /* opcode names */


/* number of list items to accumulate before a SETLIST instruction */
#define LUAI_LFIELDS_PER_FLUSH	50

/*__lparser.h__*/

/*
** Expression and variable descriptor.
** Code generation for variables and expressions can be delayed to allow
** optimizations; An 'luai_expdesc' structure describes a potentially-delayed
** variable/expression. It has a description of its "main" value plus a
** list of conditional jumps that can also produce its value (generated
** by short-circuit operators 'and'/'or').
*/

/* kinds of variables/expressions */
typedef enum {
  LUAI_VVOID,  /* when 'luai_expdesc' describes the last expression a list,
             this kind means an empty list (so, no expression) */
  LUAI_VNIL,  /* constant nil */
  LUAI_VTRUE,  /* constant true */
  LUAI_VFALSE,  /* constant false */
  LUAI_VK,  /* constant in 'k'; info = index of constant in 'k' */
  LUAI_VKFLT,  /* floating constant; nval = numerical float value */
  LUAI_VKINT,  /* integer constant; nval = numerical integer value */
  LUAI_VNONRELOC,  /* expression has its value in a fixed register;
                 info = result register */
  LUAI_VLOCAL,  /* local variable; info = local register */
  LUAI_VUPVAL,  /* upvalue variable; info = index of upvalue in 'upvalues' */
  LUAI_VINDEXED,  /* indexed variable;
                ind.vt = whether 't' is register or upvalue;
                ind.t = table register or upvalue;
                ind.idx = key's R/K index */
  LUAI_VJMP,  /* expression is a test/comparison;
            info = pc of corresponding jump instruction */
  LUAI_VRELOCABLE,  /* expression can put result in any register;
                  info = instruction pc */
  LUAI_VCALL,  /* expression is a function call; info = instruction pc */
  LUAI_VVARARG  /* vararg expression; info = instruction pc */
} luai_expkind;


#define luai_vkisvar(k)	(LUAI_VLOCAL <= (k) && (k) <= LUAI_VINDEXED)
#define luai_vkisinreg(k)	((k) == LUAI_VNONRELOC || (k) == LUAI_VLOCAL)

typedef struct luai_expdesc {
  luai_expkind k;
  union {
    lua_Integer ival;    /* for LUAI_VKINT */
    lua_Number nval;  /* for LUAI_VKFLT */
    int info;  /* for generic use */
    struct {  /* for indexed variables (LUAI_VINDEXED) */
      short idx;  /* index (R/K) */
      luai_lu_byte t;  /* table (register or upvalue) */
      luai_lu_byte vt;  /* whether 't' is register (LUAI_VLOCAL) or upvalue (LUAI_VUPVAL) */
    } ind;
  } u;
  int t;  /* patch list of 'exit when true' */
  int f;  /* patch list of 'exit when false' */
} luai_expdesc;


/* description of active local variable */
typedef struct luai_Vardesc {
  short idx;  /* variable index in stack */
} luai_Vardesc;


/* description of pending goto statements and label statements */
typedef struct luai_Labeldesc {
  luai_TString *name;  /* label identifier */
  int pc;  /* position in code */
  int line;  /* line where it appeared */
  luai_lu_byte nactvar;  /* local level where it appears in current luai_getblock */
} luai_Labeldesc;


/* list of labels or gotos */
typedef struct luai_Labellist {
  luai_Labeldesc *arr;  /* array */
  int n;  /* number of entries in use */
  int size;  /* array size */
} luai_Labellist;


/* dynamic structures used by the parser */
typedef struct luai_Dyndata {
  struct {  /* list of active local variables */
    luai_Vardesc *arr;
    int n;
    int size;
  } actvar;
  luai_Labellist gt;  /* list of pending gotos */
  luai_Labellist label;   /* list of active labels */
} luai_Dyndata;


/* control of blocks */
struct luai_BlockCnt;  /* defined in lparser.c */


/* state needed to generate code for a given function */
typedef struct luai_FuncState {
  luai_Proto *f;  /* current function header */
  struct luai_FuncState *prev;  /* enclosing function */
  struct luai_LexState *ls;  /* lexical state */
  struct luai_BlockCnt *bl;  /* chain of current blocks */
  int pc;  /* luai_next position to code (equivalent to 'ncode') */
  int lasttarget;   /* 'label' of last 'jump label' */
  int jpc;  /* list of pending jumps to 'pc' */
  int nk;  /* number of elements in 'k' */
  int np;  /* number of elements in 'p' */
  int firstlocal;  /* index of first local var (in luai_Dyndata array) */
  short nlocvars;  /* number of elements in 'f->locvars' */
  luai_lu_byte nactvar;  /* number of active local variables */
  luai_lu_byte nups;  /* number of upvalues */
  luai_lu_byte luai_freereg;  /* first free register */
} luai_FuncState;


LUAI_FUNC luai_LClosure *luaY_parser (lua_State *L, LUAI_ZIO *z, luai_Mbuffer *buff,
                                 luai_Dyndata *dyd, const char *name, int firstchar);

/*__lcode.h__*/

/*
** Marks the end of a patch list. It is an invalid value both as an absolute
** address, and as a list link (would link an element to itself).
*/
#define LUAI_NO_JUMP (-1)


/*
** grep "ORDER OPR" if you change these enums  (ORDER OP)
*/
typedef enum luai_BinOpr {
  LUAI_OPRADD, LUAI_OPRSUB, LUAI_OPRMUL, LUAI_OPRMOD, LUAI_OPRPOW,
  LUAI_OPRDIV,
  LUAI_OPRIDIV,
  LUAI_OPRBAND, LUAI_OPRBOR, LUAI_OPRBXOR,
  LUAI_OPRSHL, LUAI_OPRSHR,
  LUAI_OPRCONCAT,
  LUAI_OPREQ, LUAI_OPRLT, LUAI_OPRLE,
  LUAI_OPRNE, LUAI_OPRGT, LUAI_OPRGE,
  LUAI_OPRAND, LUAI_OPROR,
  LUAI_OPRNOBINOPR
} luai_BinOpr;


typedef enum laui_UnOpr { LUAI_OPRMINUS, LUAI_OPRBNOT, LUAI_OPRNOT, LUAI_OPRLEN, LUAI_OPRNOUNOPR } laui_UnOpr;


/* get (pointer to) instruction of given 'luai_expdesc' */
#define luai_getinstruction(fs,e)	((fs)->f->code[(e)->u.info])

#define luaK_codeAsBx(fs,o,A,sBx)	luaK_codeABx(fs,o,A,(sBx)+luai_MAXARG_sBx)

#define luaK_setmultret(fs,e)	luaK_setreturns(fs, e, LUA_MULTRET)

#define luaK_jumpto(fs,t)	luaK_patchlist(fs, luaK_jump(fs), t)

LUAI_FUNC int luaK_codeABx (luai_FuncState *fs, luai_OpCode o, int A, unsigned int Bx);
LUAI_FUNC int luaK_codeABC (luai_FuncState *fs, luai_OpCode o, int A, int B, int C);
LUAI_FUNC int luaK_codek (luai_FuncState *fs, int reg, int k);
LUAI_FUNC void luaK_fixline (luai_FuncState *fs, int line);
LUAI_FUNC void luaK_nil (luai_FuncState *fs, int from, int n);
LUAI_FUNC void luaK_reserveregs (luai_FuncState *fs, int n);
LUAI_FUNC void luaK_checkstack (luai_FuncState *fs, int n);
LUAI_FUNC int luaK_stringK (luai_FuncState *fs, luai_TString *s);
LUAI_FUNC int luaK_intK (luai_FuncState *fs, lua_Integer n);
LUAI_FUNC void luaK_dischargevars (luai_FuncState *fs, luai_expdesc *e);
LUAI_FUNC int luaK_exp2anyreg (luai_FuncState *fs, luai_expdesc *e);
LUAI_FUNC void luaK_exp2anyregup (luai_FuncState *fs, luai_expdesc *e);
LUAI_FUNC void luaK_exp2nextreg (luai_FuncState *fs, luai_expdesc *e);
LUAI_FUNC void luaK_exp2val (luai_FuncState *fs, luai_expdesc *e);
LUAI_FUNC int luaK_exp2RK (luai_FuncState *fs, luai_expdesc *e);
LUAI_FUNC void luaK_self (luai_FuncState *fs, luai_expdesc *e, luai_expdesc *key);
LUAI_FUNC void luaK_indexed (luai_FuncState *fs, luai_expdesc *t, luai_expdesc *k);
LUAI_FUNC void luaK_goiftrue (luai_FuncState *fs, luai_expdesc *e);
LUAI_FUNC void luaK_goiffalse (luai_FuncState *fs, luai_expdesc *e);
LUAI_FUNC void luaK_storevar (luai_FuncState *fs, luai_expdesc *var, luai_expdesc *e);
LUAI_FUNC void luaK_setreturns (luai_FuncState *fs, luai_expdesc *e, int nresults);
LUAI_FUNC void luaK_setoneret (luai_FuncState *fs, luai_expdesc *e);
LUAI_FUNC int luaK_jump (luai_FuncState *fs);
LUAI_FUNC void luaK_ret (luai_FuncState *fs, int first, int nret);
LUAI_FUNC void luaK_patchlist (luai_FuncState *fs, int list, int target);
LUAI_FUNC void luaK_patchtohere (luai_FuncState *fs, int list);
LUAI_FUNC void luaK_patchclose (luai_FuncState *fs, int list, int level);
LUAI_FUNC void luaK_concat (luai_FuncState *fs, int *l1, int l2);
LUAI_FUNC int luaK_getlabel (luai_FuncState *fs);
LUAI_FUNC void luaK_prefix (luai_FuncState *fs, laui_UnOpr op, luai_expdesc *v, int line);
LUAI_FUNC void luaK_infix (luai_FuncState *fs, luai_BinOpr op, luai_expdesc *v);
LUAI_FUNC void luaK_posfix (luai_FuncState *fs, luai_BinOpr op, luai_expdesc *v1,
                            luai_expdesc *v2, int line);
LUAI_FUNC void luaK_setlist (luai_FuncState *fs, int base, int nelems, int tostore);

/*__lctype.h__*/

/*
** WARNING: the functions defined here do not necessarily correspond
** to the similar functions in the standard C ctype.h. They are
** optimized for the specific needs of Lua
*/

#if !defined(LUA_USE_CTYPE)

#if 'A' == 65 && '0' == 48
/* ASCII case: can use its own tables; faster and fixed */
#define LUA_USE_CTYPE	0
#else
/* must use standard C ctype */
#define LUA_USE_CTYPE	1
#endif

#endif


#if !LUA_USE_CTYPE	/* { */

#define LUAI_ALPHABIT	0
#define LUAI_DIGITBIT	1
#define LUAI_PRINTBIT	2
#define LUAI_SPACEBIT	3
#define LUAI_XDIGITBIT	4


#define LUAI_MASK(B)		(1 << (B))


/*
** add 1 to char to allow index -1 (LUAI_EOZ)
*/
#define luai_testprop(c,p)	(luai_ctype_[(c)+1] & (p))

/*
** 'lalpha' (Lua alphabetic) and 'lalnum' (Lua alphanumeric) both include '_'
*/
#define luai_lislalpha(c)	luai_testprop(c, LUAI_MASK(LUAI_ALPHABIT))
#define luai_lislalnum(c)	luai_testprop(c, (LUAI_MASK(LUAI_ALPHABIT) | LUAI_MASK(LUAI_DIGITBIT)))
#define luai_lisdigit(c)	luai_testprop(c, LUAI_MASK(LUAI_DIGITBIT))
#define luai_lisspace(c)	luai_testprop(c, LUAI_MASK(LUAI_SPACEBIT))
#define luai_lisprint(c)	luai_testprop(c, LUAI_MASK(LUAI_PRINTBIT))
#define luai_lisxdigit(c)	luai_testprop(c, LUAI_MASK(LUAI_XDIGITBIT))

/*
** this 'luai_tolower' only works for alphabetic characters
*/
#define luai_tolower(c)	((c) | ('A' ^ 'a'))


/* two more entries for 0 and -1 (LUAI_EOZ) */
LUAI_DDEC const luai_lu_byte luai_ctype_[UCHAR_MAX + 2];


#else			/* }{ */

/*
** use standard C ctypes
*/

#include <ctype.h>


#define luai_lislalpha(c)	(isalpha(c) || (c) == '_')
#define luai_lislalnum(c)	(isalnum(c) || (c) == '_')
#define luai_lisdigit(c)	(isdigit(c))
#define luai_lisspace(c)	(isspace(c))
#define luai_lisprint(c)	(isprint(c))
#define luai_lisxdigit(c)	(isxdigit(c))

#define luai_tolower(c)	(tolower(c))

#endif			/* } */

/*__ldebug.h__*/

#define luai_pcRel(pc, p)	(luai_cast(int, (pc) - (p)->code) - 1)

#define luai_getfuncline(f,pc)	(((f)->lineinfo) ? (f)->lineinfo[pc] : -1)

#define luai_resethookcount(L)	(L->hookcount = L->basehookcount)


LUAI_FUNC luai_l_noret luaG_typeerror (lua_State *L, const luai_TValue *o,
                                                const char *opname);
LUAI_FUNC luai_l_noret luaG_concaterror (lua_State *L, const luai_TValue *p1,
                                                  const luai_TValue *p2);
LUAI_FUNC luai_l_noret luaG_opinterror (lua_State *L, const luai_TValue *p1,
                                                 const luai_TValue *p2,
                                                 const char *msg);
LUAI_FUNC luai_l_noret luaG_tointerror (lua_State *L, const luai_TValue *p1,
                                                 const luai_TValue *p2);
LUAI_FUNC luai_l_noret luaG_ordererror (lua_State *L, const luai_TValue *p1,
                                                 const luai_TValue *p2);
LUAI_FUNC luai_l_noret luaG_runerror (lua_State *L, const char *fmt, ...);
LUAI_FUNC const char *luaG_addinfo (lua_State *L, const char *msg,
                                                  luai_TString *src, int line);
LUAI_FUNC luai_l_noret luaG_errormsg (lua_State *L);
LUAI_FUNC void luaG_traceexec (lua_State *L);

/*__ldo.h__*/

/*
** Macro to luai_check stack size and grow stack if needed.  Parameters
** 'pre'/'pos' allow the macro to preserve a pointer into the
** stack across reallocations, doing the work only when needed.
** 'luai_condmovestack' is used in heavy tests to force a stack reallocation
** at every luai_check.
*/
#define luaD_checkstackaux(L,n,pre,pos)  \
	if (L->stack_last - L->top <= (n)) \
	  { pre; luaD_growstack(L, n); pos; } else { luai_condmovestack(L,pre,pos); }

/* In general, 'pre'/'pos' are empty (nothing to luai_save) */
#define luaD_checkstack(L,n)	luaD_checkstackaux(L,n,(void)0,(void)0)



#define luai_savestack(L,p)		((char *)(p) - (char *)L->stack)
#define luai_restorestack(L,n)	((luai_TValue *)((char *)L->stack + (n)))


/* type of protected functions, to be ran by 'runprotected' */
typedef void (*Pfunc) (lua_State *L, void *ud);

LUAI_FUNC int luaD_protectedparser (lua_State *L, LUAI_ZIO *z, const char *name,
                                                  const char *mode);
LUAI_FUNC void luaD_hook (lua_State *L, int event, int line);
LUAI_FUNC int luaD_precall (lua_State *L, luai_StkId func, int nresults);
LUAI_FUNC void luaD_call (lua_State *L, luai_StkId func, int nResults);
LUAI_FUNC void luaD_callnoyield (lua_State *L, luai_StkId func, int nResults);
LUAI_FUNC int luaD_pcall (lua_State *L, Pfunc func, void *u,
                                        ptrdiff_t oldtop, ptrdiff_t ef);
LUAI_FUNC int luaD_poscall (lua_State *L, luai_CallInfo *ci, luai_StkId firstResult,
                                          int nres);
LUAI_FUNC void luaD_reallocstack (lua_State *L, int newsize);
LUAI_FUNC void luaD_growstack (lua_State *L, int n);
LUAI_FUNC void luaD_shrinkstack (lua_State *L);
LUAI_FUNC void luaD_inctop (lua_State *L);

LUAI_FUNC luai_l_noret luaD_throw (lua_State *L, int errcode);
LUAI_FUNC int luaD_rawrunprotected (lua_State *L, Pfunc f, void *ud);

/*__lfunc.h__*/

#define luai_sizeCclosure(n)	(luai_cast(int, sizeof(luai_CClosure)) + \
                         luai_cast(int, sizeof(luai_TValue)*((n)-1)))

#define luai_sizeLclosure(n)	(luai_cast(int, sizeof(luai_LClosure)) + \
                         luai_cast(int, sizeof(luai_TValue *)*((n)-1)))


/* test whether thread is in 'twups' list */
#define luai_isintwups(L)	(L->twups != L)


/*
** maximum number of upvalues in a closure (both C and Lua). (Value
** must fit in a VM register.)
*/
#define LUAI_MAXUPVAL	255


/*
** Upvalues for Lua closures
*/
struct luai_UpVal {
  luai_TValue *v;  /* points to stack or to its own value */
  luai_lu_mem refcount;  /* reference counter */
  union {
    struct {  /* (when open) */
      luai_UpVal *luai_next;  /* linked list */
      int touched;  /* mark to avoid cycles with dead threads */
    } open;
    luai_TValue value;  /* the value (when closed) */
  } u;
};

#define luai_upisopen(up)	((up)->v != &(up)->u.value)


LUAI_FUNC luai_Proto *luaF_newproto (lua_State *L);
LUAI_FUNC luai_CClosure *luaF_newCclosure (lua_State *L, int nelems);
LUAI_FUNC luai_LClosure *luaF_newLclosure (lua_State *L, int nelems);
LUAI_FUNC void luaF_initupvals (lua_State *L, luai_LClosure *cl);
LUAI_FUNC luai_UpVal *luaF_findupval (lua_State *L, luai_StkId level);
LUAI_FUNC void luaF_close (lua_State *L, luai_StkId level);
LUAI_FUNC void luaF_freeproto (lua_State *L, luai_Proto *f);
LUAI_FUNC const char *luaF_getlocalname (const luai_Proto *func, int local_number,
                                         int pc);

/*__lgc.h__*/

/*
** Collectable objects may have one of three colors: white, which
** means the object is not marked; gray, which means the
** object is marked, but its references may be not marked; and
** black, which means that the object and all its references are marked.
** The main invariant of the garbage collector, while marking objects,
** is that a black object can never point to a white one. Moreover,
** any gray object must be in a "gray list" (gray, grayagain, weak,
** allweak, ephemeron) so that it can be visited again before finishing
** the collection cycle. These lists have no meaning when the invariant
** is not being enforced (e.g., sweep phase).
*/



/* how much to allocate before luai_next LUAI_GC step */
#if !defined(LUA_GCSTEPSIZE)
/* ~100 small strings */
#define LUA_GCSTEPSIZE	(luai_cast_int(100 * sizeof(luai_TString)))
#endif


/*
** Possible states of the Garbage Collector
*/
#define LUAI_GCSpropagate	0
#define LUAI_GCSatomic	1
#define LUAI_GCSswpallgc	2
#define LUAI_GCSswpfinobj	3
#define LUAI_GCSswptobefnz	4
#define LUAI_GCSswpend	5
#define LUAI_GCScallfin	6
#define LUAI_GCSpause	7


#define luai_issweepphase(g)  \
	(LUAI_GCSswpallgc <= (g)->gcstate && (g)->gcstate <= LUAI_GCSswpend)


/*
** macro to tell when main invariant (white objects cannot point to black
** ones) must be kept. During a collection, the sweep
** phase may break the invariant, as objects turned white may point to
** still-black objects. The invariant is restored when sweep ends and
** all objects are white again.
*/

#define luai_keepinvariant(g)	((g)->gcstate <= LUAI_GCSatomic)


/*
** some useful bit tricks
*/
#define luai_resetbits(x,m)		((x) &= luai_cast(luai_lu_byte, ~(m)))
#define luai_setbits(x,m)		((x) |= (m))
#define luai_testbits(x,m)		((x) & (m))
#define luai_bitmask(b)		(1<<(b))
#define luai_bit2mask(b1,b2)		(luai_bitmask(b1) | luai_bitmask(b2))
#define luai_l_setbit(x,b)		luai_setbits(x, luai_bitmask(b))
#define luai_resetbit(x,b)		luai_resetbits(x, luai_bitmask(b))
#define luai_testbit(x,b)		luai_testbits(x, luai_bitmask(b))


/* Layout for bit use in 'marked' luai_field: */
#define LUAI_WHITE0BIT	0  /* object is white (type 0) */
#define LUAI_WHITE1BIT	1  /* object is white (type 1) */
#define LUAI_BLACKBIT	2  /* object is black */
#define LUAI_FINALIZEDBIT	3  /* object has been marked for finalization */
/* bit 7 is currently used by tests (luaL_checkmemory) */

#define LUAI_WHITEBITS	luai_bit2mask(LUAI_WHITE0BIT, LUAI_WHITE1BIT)


#define luai_iswhite(x)      luai_testbits((x)->marked, LUAI_WHITEBITS)
#define luai_isblack(x)      luai_testbit((x)->marked, LUAI_BLACKBIT)
#define luai_isgray(x)  /* neither white nor black */  \
	(!luai_testbits((x)->marked, LUAI_WHITEBITS | luai_bitmask(LUAI_BLACKBIT)))

#define luai_tofinalize(x)	luai_testbit((x)->marked, LUAI_FINALIZEDBIT)

#define luai_otherwhite(g)	((g)->currentwhite ^ LUAI_WHITEBITS)
#define luai_isdeadm(ow,m)	(!(((m) ^ LUAI_WHITEBITS) & (ow)))
#define luai_isdead(g,v)	luai_isdeadm(luai_otherwhite(g), (v)->marked)

#define luai_changewhite(x)	((x)->marked ^= LUAI_WHITEBITS)
#define luai_gray2black(x)	luai_l_setbit((x)->marked, LUAI_BLACKBIT)

#define luaC_white(g)	luai_cast(luai_lu_byte, (g)->currentwhite & LUAI_WHITEBITS)


/*
** Does one step of collection when debt becomes positive. 'pre'/'pos'
** allows some adjustments to be done only when needed. macro
** 'luai_condchangemem' is used only for heavy tests (forcing a full
** LUAI_GC cycle on every opportunity)
*/
#define luaC_condGC(L,pre,pos) \
	{ if (LUAI_G(L)->LUAI_GCdebt > 0) { pre; luaC_step(L); pos;}; \
	  luai_condchangemem(L,pre,pos); }

/* more often than not, 'pre'/'pos' are empty */
#define luaC_checkGC(L)		luaC_condGC(L,(void)0,(void)0)


#define luaC_barrier(L,p,v) (  \
	(luai_iscollectable(v) && luai_isblack(p) && luai_iswhite(luai_gcvalue(v))) ?  \
	luaC_barrier_(L,luai_obj2gco(p),luai_gcvalue(v)) : luai_cast_void(0))

#define luaC_barrierback(L,p,v) (  \
	(luai_iscollectable(v) && luai_isblack(p) && luai_iswhite(luai_gcvalue(v))) ? \
	luaC_barrierback_(L,p) : luai_cast_void(0))

#define luaC_objbarrier(L,p,o) (  \
	(luai_isblack(p) && luai_iswhite(o)) ? \
	luaC_barrier_(L,luai_obj2gco(p),luai_obj2gco(o)) : luai_cast_void(0))

#define luaC_upvalbarrier(L,uv) ( \
	(luai_iscollectable((uv)->v) && !luai_upisopen(uv)) ? \
         luaC_upvalbarrier_(L,uv) : luai_cast_void(0))

LUAI_FUNC void luaC_fix (lua_State *L, LUAI_GCObject *o);
LUAI_FUNC void luaC_freeallobjects (lua_State *L);
LUAI_FUNC void luaC_step (lua_State *L);
LUAI_FUNC void luaC_runtilstate (lua_State *L, int statesmask);
LUAI_FUNC void luaC_fullgc (lua_State *L, int isemergency);
LUAI_FUNC LUAI_GCObject *luaC_newobj (lua_State *L, int tt, size_t sz);
LUAI_FUNC void luaC_barrier_ (lua_State *L, LUAI_GCObject *o, LUAI_GCObject *v);
LUAI_FUNC void luaC_barrierback_ (lua_State *L, luai_Table *o);
LUAI_FUNC void luaC_upvalbarrier_ (lua_State *L, luai_UpVal *uv);
LUAI_FUNC void luaC_checkfinalizer (lua_State *L, LUAI_GCObject *o, luai_Table *mt);
LUAI_FUNC void luaC_upvdeccount (lua_State *L, luai_UpVal *uv);

/*__lstring.h__*/

#define luai_sizelstring(l)  (sizeof(union luai_UTString) + ((l) + 1) * sizeof(char))

#define luai_sizeludata(l)	(sizeof(union luai_UUdata) + (l))
#define luai_sizeudata(u)	luai_sizeludata((u)->len)

#define luaS_newliteral(L, s)	(luaS_newlstr(L, "" s, \
                                 (sizeof(s)/sizeof(char))-1))


/*
** test whether a string is a reserved word
*/
#define luai_isreserved(s)	((s)->tt == LUA_TSHRSTR && (s)->extra > 0)


/*
** equality for short strings, which are always internalized
*/
#define luai_eqshrstr(a,b)	luai_check_exp((a)->tt == LUA_TSHRSTR, (a) == (b))


LUAI_FUNC unsigned int luaS_hash (const char *str, size_t l, unsigned int seed);
LUAI_FUNC unsigned int luaS_hashlongstr (luai_TString *ts);
LUAI_FUNC int luaS_eqlngstr (luai_TString *a, luai_TString *b);
LUAI_FUNC void luaS_resize (lua_State *L, int newsize);
LUAI_FUNC void luaS_clearcache (luai_global_State *g);
LUAI_FUNC void luaS_init (lua_State *L);
LUAI_FUNC void luaS_remove (lua_State *L, luai_TString *ts);
LUAI_FUNC luai_Udata *luaS_newudata (lua_State *L, size_t s);
LUAI_FUNC luai_TString *luaS_newlstr (lua_State *L, const char *str, size_t l);
LUAI_FUNC luai_TString *luaS_new (lua_State *L, const char *str);
LUAI_FUNC luai_TString *luaS_createlngstrobj (lua_State *L, size_t l);

/*__ltable.h__*/


#define luai_gnode(t,i)	(&(t)->node[i])
#define luai_gval(n)		(&(n)->i_val)
#define luai_gnext(n)	((n)->i_key.nk.luai_next)


/* 'const' to avoid wrong writings that can mess up luai_field 'luai_next' */
#define luai_gkey(n)		luai_cast(const luai_TValue*, (&(n)->i_key.tvk))

/*
** writable version of 'luai_gkey'; allows updates to individual fields,
** but not to the whole (which has incompatible type)
*/
#define luai_wgkey(n)		(&(n)->i_key.nk)

#define luai_invalidateTMcache(t)	((t)->flags = 0)


/* true when 't' is using 'luai_dummynode' as its hash part */
#define luai_isdummy(t)		((t)->lastfree == NULL)


/* allocated size for hash nodes */
#define luai_allocsizenode(t)	(luai_isdummy(t) ? 0 : luai_sizenode(t))


/* returns the key, given the value of a table entry */
#define luai_keyfromval(v) \
  (luai_gkey(luai_cast(luai_Node *, luai_cast(char *, (v)) - offsetof(luai_Node, i_val))))


LUAI_FUNC const luai_TValue *luaH_getint (luai_Table *t, lua_Integer key);
LUAI_FUNC void luaH_setint (lua_State *L, luai_Table *t, lua_Integer key,
                                                    luai_TValue *value);
LUAI_FUNC const luai_TValue *luaH_getshortstr (luai_Table *t, luai_TString *key);
LUAI_FUNC const luai_TValue *luaH_getstr (luai_Table *t, luai_TString *key);
LUAI_FUNC const luai_TValue *luaH_get (luai_Table *t, const luai_TValue *key);
LUAI_FUNC luai_TValue *luaH_newkey (lua_State *L, luai_Table *t, const luai_TValue *key);
LUAI_FUNC luai_TValue *luaH_set (lua_State *L, luai_Table *t, const luai_TValue *key);
LUAI_FUNC luai_Table *luaH_new (lua_State *L);
LUAI_FUNC void luaH_resize (lua_State *L, luai_Table *t, unsigned int nasize,
                                                    unsigned int nhsize);
LUAI_FUNC void luaH_resizearray (lua_State *L, luai_Table *t, unsigned int nasize);
LUAI_FUNC void luaH_free (lua_State *L, luai_Table *t);
LUAI_FUNC int luaH_next (lua_State *L, luai_Table *t, luai_StkId key);
LUAI_FUNC int luaH_getn (luai_Table *t);


#if defined(LUA_DEBUG)
LUAI_FUNC luai_Node *luaH_mainposition (const luai_Table *t, const luai_TValue *key);
LUAI_FUNC int luaH_isdummy (const luai_Table *t);
#endif

/*__lundump.h__*/

/* data to catch conversion errors */
#define LUAC_DATA	"\x19\x93\r\n\x1a\n"

#define LUAC_INT	0x5678
#define LUAC_NUM	luai_cast_num(370.5)

#define MYINT(s)	(s[0]-'0')
#define LUAC_VERSION	(MYINT(LUA_VERSION_MAJOR)*16+MYINT(LUA_VERSION_MINOR))
#define LUAC_FORMAT	0	/* this is the official format */

/* load one chunk; from lundump.c */
LUAI_FUNC luai_LClosure* luaU_undump (lua_State* L, LUAI_ZIO* Z, const char* name);

/* dump one chunk; from ldump.c */
LUAI_FUNC int luaU_dump (lua_State* L, const luai_Proto* f, lua_Writer w,
                         void* data, int strip);

/*__lvm.h__*/

#if !defined(LUA_NOCVTN2S)
#define luai_cvt2str(o)	luai_ttisnumber(o)
#else
#define luai_cvt2str(o)	0	/* no conversion from numbers to strings */
#endif


#if !defined(LUA_NOCVTS2N)
#define luai_cvt2num(o)	luai_ttisstring(o)
#else
#define luai_cvt2num(o)	0	/* no conversion from strings to numbers */
#endif


/*
** You can define LUA_FLOORN2I if you want to convert floats to integers
** by flooring them (instead of raising an error if they are not
** integral values)
*/
#if !defined(LUA_FLOORN2I)
#define LUA_FLOORN2I		0
#endif


#define luai_tonumber(o,n) \
	(luai_ttisfloat(o) ? (*(n) = luai_fltvalue(o), 1) : luaV_tonumber_(o,n))

#define luai_tointeger(o,i) \
    (luai_ttisinteger(o) ? (*(i) = luai_ivalue(o), 1) : luaV_tointeger(o,i,LUA_FLOORN2I))

#define luai_intop(op,v1,v2) luai_l_castU2S(luai_l_castS2U(v1) op luai_l_castS2U(v2))

#define luaV_rawequalobj(t1,t2)		luaV_equalobj(NULL,t1,t2)


/*
** fast track for 'gettable': if 't' is a table and 't[k]' is not nil,
** return 1 with 'slot' pointing to 't[k]' (final result).  Otherwise,
** return 0 (meaning it will have to luai_check metamethod) with 'slot'
** pointing to a nil 't[k]' (if 't' is a table) or NULL (otherwise).
** 'f' is the raw get function to use.
*/
#define luaV_fastget(L,t,k,slot,f) \
  (!luai_ttistable(t)  \
   ? (slot = NULL, 0)  /* not a table; 'slot' is NULL and result is 0 */  \
   : (slot = f(luai_hvalue(t), k),  /* else, do raw access */  \
      !luai_ttisnil(slot)))  /* result not nil? */

/*
** standard implementation for 'gettable'
*/
#define luaV_gettable(L,t,k,v) { const luai_TValue *slot; \
  if (luaV_fastget(L,t,k,slot,luaH_get)) { luai_setobj2s(L, v, slot); } \
  else luaV_finishget(L,t,k,v,slot); }


/*
** Fast track for set table. If 't' is a table and 't[k]' is not nil,
** call LUAI_GC barrier, do a raw 't[k]=v', and return true; otherwise,
** return false with 'slot' equal to NULL (if 't' is not a table) or
** 'nil'. (This is needed by 'luaV_finishget'.) Note that, if the macro
** returns true, there is no need to 'luai_invalidateTMcache', because the
** call is not creating a new entry.
*/
#define luaV_fastset(L,t,k,slot,f,v) \
  (!luai_ttistable(t) \
   ? (slot = NULL, 0) \
   : (slot = f(luai_hvalue(t), k), \
     luai_ttisnil(slot) ? 0 \
     : (luaC_barrierback(L, luai_hvalue(t), v), \
        luai_setobj2t(L, luai_cast(luai_TValue *,slot), v), \
        1)))


#define luaV_settable(L,t,k,v) { const luai_TValue *slot; \
  if (!luaV_fastset(L,t,k,slot,luaH_get,v)) \
    luaV_finishset(L,t,k,v,slot); }



LUAI_FUNC int luaV_equalobj (lua_State *L, const luai_TValue *t1, const luai_TValue *t2);
LUAI_FUNC int luaV_lessthan (lua_State *L, const luai_TValue *l, const luai_TValue *r);
LUAI_FUNC int luaV_lessequal (lua_State *L, const luai_TValue *l, const luai_TValue *r);
LUAI_FUNC int luaV_tonumber_ (const luai_TValue *obj, lua_Number *n);
LUAI_FUNC int luaV_tointeger (const luai_TValue *obj, lua_Integer *p, int mode);
LUAI_FUNC void luaV_finishget (lua_State *L, const luai_TValue *t, luai_TValue *key,
                               luai_StkId val, const luai_TValue *slot);
LUAI_FUNC void luaV_finishset (lua_State *L, const luai_TValue *t, luai_TValue *key,
                               luai_StkId val, const luai_TValue *slot);
LUAI_FUNC void luaV_finishOp (lua_State *L);
LUAI_FUNC void luaV_execute (lua_State *L);
LUAI_FUNC void luaV_concat (lua_State *L, int total);
LUAI_FUNC lua_Integer luaV_div (lua_State *L, lua_Integer x, lua_Integer y);
LUAI_FUNC lua_Integer luaV_mod (lua_State *L, lua_Integer x, lua_Integer y);
LUAI_FUNC lua_Integer luaV_shiftl (lua_Integer x, lua_Integer y);
LUAI_FUNC void luaV_objlen (lua_State *L, luai_StkId ra, const luai_TValue *rb);

/*__lapi.c__*/

const char lua_ident[] =
  "$LuaVersion: " LUA_COPYRIGHT " $"
  "$LuaAuthors: " LUA_AUTHORS " $";


/* value at a non-valid index */
#define LUA_NONVALIDVALUE		luai_cast(luai_TValue *, luaO_nilobject)

/* corresponding test */
#define lua_isvalid(o)	((o) != luaO_nilobject)

/* test for pseudo index */
#define lua_ispseudo(i)		((i) <= LUA_REGISTRYINDEX)

/* test for upvalue */
#define lua_isupvalue(i)		((i) < LUA_REGISTRYINDEX)

/* test for valid but not pseudo index */
#define lua_isstackindex(i, o)	(lua_isvalid(o) && !lua_ispseudo(i))

#define lua_api_checkvalidindex(l,o)  luai_api_check(l, lua_isvalid(o), "invalid index")

#define lua_api_checkstackindex(l, i, o)  \
	luai_api_check(l, lua_isstackindex(i, o), "index not in the stack")


static luai_TValue *lua_index2addr (lua_State *L, int idx) {
  luai_CallInfo *ci = L->ci;
  if (idx > 0) {
    luai_TValue *o = ci->func + idx;
    luai_api_check(L, idx <= ci->top - (ci->func + 1), "unacceptable index");
    if (o >= L->top) return LUA_NONVALIDVALUE;
    else return o;
  }
  else if (!lua_ispseudo(idx)) {  /* negative index */
    luai_api_check(L, idx != 0 && -idx <= L->top - (ci->func + 1), "invalid index");
    return L->top + idx;
  }
  else if (idx == LUA_REGISTRYINDEX)
    return &LUAI_G(L)->luai_l_registry;
  else {  /* upvalues */
    idx = LUA_REGISTRYINDEX - idx;
    luai_api_check(L, idx <= LUAI_MAXUPVAL + 1, "upvalue index too large");
    if (luai_ttislcf(ci->func))  /* light C function? */
      return LUA_NONVALIDVALUE;  /* it has no upvalues */
    else {
      luai_CClosure *func = luai_clCvalue(ci->func);
      return (idx <= func->nupvalues) ? &func->upvalue[idx-1] : LUA_NONVALIDVALUE;
    }
  }
}


/*
** to be called by 'lua_checkstack' in protected mode, to grow stack
** capturing memory errors
*/
static void lua_growstack (lua_State *L, void *ud) {
  int size = *(int *)ud;
  luaD_growstack(L, size);
}


LUA_API int lua_checkstack (lua_State *L, int n) {
  int res;
  luai_CallInfo *ci = L->ci;
  lua_lock(L);
  luai_api_check(L, n >= 0, "negative 'n'");
  if (L->stack_last - L->top > n)  /* stack large enough? */
    res = 1;  /* yes; luai_check is OK */
  else {  /* no; need to grow stack */
    int inuse = luai_cast_int(L->top - L->stack) + LUAI_EXTRA_STACK;
    if (inuse > LUAI_MAXSTACK - n)  /* can grow without overflow? */
      res = 0;  /* no */
    else  /* try to grow stack */
      res = (luaD_rawrunprotected(L, &lua_growstack, &n) == LUA_OK);
  }
  if (res && ci->top < L->top + n)
    ci->top = L->top + n;  /* adjust frame top */
  lua_unlock(L);
  return res;
}


LUA_API void lua_xmove (lua_State *from, lua_State *to, int n) {
  int i;
  if (from == to) return;
  lua_lock(to);
  luai_api_checknelems(from, n);
  luai_api_check(from, LUAI_G(from) == LUAI_G(to), "moving among independent states");
  luai_api_check(from, to->ci->top - to->top >= n, "stack overflow");
  from->top -= n;
  for (i = 0; i < n; i++) {
    luai_setobj2s(to, to->top, from->top + i);
    to->top++;  /* stack already checked by previous 'luai_api_check' */
  }
  lua_unlock(to);
}


LUA_API lua_CFunction lua_atpanic (lua_State *L, lua_CFunction panicf) {
  lua_CFunction old;
  lua_lock(L);
  old = LUAI_G(L)->panic;
  LUAI_G(L)->panic = panicf;
  lua_unlock(L);
  return old;
}


LUA_API const lua_Number *lua_version (lua_State *L) {
  static const lua_Number version = LUA_VERSION_NUM;
  if (L == NULL) return &version;
  else return LUAI_G(L)->version;
}



/*
** basic stack manipulation
*/


/*
** convert an acceptable stack index into an absolute index
*/
LUA_API int lua_absindex (lua_State *L, int idx) {
  return (idx > 0 || lua_ispseudo(idx))
         ? idx
         : luai_cast_int(L->top - L->ci->func) + idx;
}


LUA_API int lua_gettop (lua_State *L) {
  return luai_cast_int(L->top - (L->ci->func + 1));
}


LUA_API void lua_settop (lua_State *L, int idx) {
  luai_StkId func = L->ci->func;
  lua_lock(L);
  if (idx >= 0) {
    luai_api_check(L, idx <= L->stack_last - (func + 1), "new top too large");
    while (L->top < (func + 1) + idx)
      luai_setnilvalue(L->top++);
    L->top = (func + 1) + idx;
  }
  else {
    luai_api_check(L, -(idx+1) <= (L->top - (func + 1)), "invalid new top");
    L->top += idx+1;  /* 'subtract' index (index is negative) */
  }
  lua_unlock(L);
}


/*
** Reverse the stack segment from 'from' to 'to'
** (auxiliary to 'lua_rotate')
*/
static void luai_reverse (lua_State *L, luai_StkId from, luai_StkId to) {
  for (; from < to; from++, to--) {
    luai_TValue temp;
    luai_setobj(L, &temp, from);
    luai_setobjs2s(L, from, to);
    luai_setobj2s(L, to, &temp);
  }
}


/*
** Let x = AB, where A is a prefix of length 'n'. Then,
** rotate x n == BA. But BA == (A^r . B^r)^r.
*/
LUA_API void lua_rotate (lua_State *L, int idx, int n) {
  luai_StkId p, t, m;
  lua_lock(L);
  t = L->top - 1;  /* end of stack segment being rotated */
  p = lua_index2addr(L, idx);  /* start of segment */
  lua_api_checkstackindex(L, idx, p);
  luai_api_check(L, (n >= 0 ? n : -n) <= (t - p + 1), "invalid 'n'");
  m = (n >= 0 ? t - n : p - n - 1);  /* end of prefix */
  luai_reverse(L, p, m);  /* reverse the prefix with length 'n' */
  luai_reverse(L, m + 1, t);  /* reverse the suffix */
  luai_reverse(L, p, t);  /* reverse the entire segment */
  lua_unlock(L);
}


LUA_API void lua_copy (lua_State *L, int fromidx, int toidx) {
  luai_TValue *fr, *to;
  lua_lock(L);
  fr = lua_index2addr(L, fromidx);
  to = lua_index2addr(L, toidx);
  lua_api_checkvalidindex(L, to);
  luai_setobj(L, to, fr);
  if (lua_isupvalue(toidx))  /* function upvalue? */
    luaC_barrier(L, luai_clCvalue(L->ci->func), fr);
  /* LUA_REGISTRYINDEX does not need gc barrier
     (collector revisits it before finishing collection) */
  lua_unlock(L);
}


LUA_API void lua_pushvalue (lua_State *L, int idx) {
  lua_lock(L);
  luai_setobj2s(L, L->top, lua_index2addr(L, idx));
  luai_api_incr_top(L);
  lua_unlock(L);
}



/*
** access functions (stack -> C)
*/


LUA_API int lua_type (lua_State *L, int idx) {
  luai_StkId o = lua_index2addr(L, idx);
  return (lua_isvalid(o) ? luai_ttnov(o) : LUA_TNONE);
}


LUA_API const char *lua_typename (lua_State *L, int t) {
  LUAI_UNUSED(L);
  luai_api_check(L, LUA_TNONE <= t && t < LUA_NUMTAGS, "invalid tag");
  return luai_ttypename(t);
}


LUA_API int lua_iscfunction (lua_State *L, int idx) {
  luai_StkId o = lua_index2addr(L, idx);
  return (luai_ttislcf(o) || (luai_ttisCclosure(o)));
}


LUA_API int lua_isinteger (lua_State *L, int idx) {
  luai_StkId o = lua_index2addr(L, idx);
  return luai_ttisinteger(o);
}


LUA_API int lua_isnumber (lua_State *L, int idx) {
  lua_Number n;
  const luai_TValue *o = lua_index2addr(L, idx);
  return luai_tonumber(o, &n);
}


LUA_API int lua_isstring (lua_State *L, int idx) {
  const luai_TValue *o = lua_index2addr(L, idx);
  return (luai_ttisstring(o) || luai_cvt2str(o));
}


LUA_API int lua_isuserdata (lua_State *L, int idx) {
  const luai_TValue *o = lua_index2addr(L, idx);
  return (luai_ttisfulluserdata(o) || luai_ttislightuserdata(o));
}


LUA_API int lua_rawequal (lua_State *L, int index1, int index2) {
  luai_StkId o1 = lua_index2addr(L, index1);
  luai_StkId o2 = lua_index2addr(L, index2);
  return (lua_isvalid(o1) && lua_isvalid(o2)) ? luaV_rawequalobj(o1, o2) : 0;
}


LUA_API void lua_arith (lua_State *L, int op) {
  lua_lock(L);
  if (op != LUA_OPUNM && op != LUA_OPBNOT)
    luai_api_checknelems(L, 2);  /* all other operations expect two operands */
  else {  /* for unary operations, add fake 2nd operand */
    luai_api_checknelems(L, 1);
    luai_setobjs2s(L, L->top, L->top - 1);
    luai_api_incr_top(L);
  }
  /* first operand at top - 2, second at top - 1; result go to top - 2 */
  luaO_arith(L, op, L->top - 2, L->top - 1, L->top - 2);
  L->top--;  /* remove second operand */
  lua_unlock(L);
}


LUA_API int lua_compare (lua_State *L, int index1, int index2, int op) {
  luai_StkId o1, o2;
  int i = 0;
  lua_lock(L);  /* may call tag method */
  o1 = lua_index2addr(L, index1);
  o2 = lua_index2addr(L, index2);
  if (lua_isvalid(o1) && lua_isvalid(o2)) {
    switch (op) {
      case LUA_OPEQ: i = luaV_equalobj(L, o1, o2); break;
      case LUA_OPLT: i = luaV_lessthan(L, o1, o2); break;
      case LUA_OPLE: i = luaV_lessequal(L, o1, o2); break;
      default: luai_api_check(L, 0, "invalid option");
    }
  }
  lua_unlock(L);
  return i;
}


LUA_API size_t lua_stringtonumber (lua_State *L, const char *s) {
  size_t sz = luaO_str2num(s, L->top);
  if (sz != 0)
    luai_api_incr_top(L);
  return sz;
}


LUA_API lua_Number lua_tonumberx (lua_State *L, int idx, int *pisnum) {
  lua_Number n;
  const luai_TValue *o = lua_index2addr(L, idx);
  int isnum = luai_tonumber(o, &n);
  if (!isnum)
    n = 0;  /* call to 'luai_tonumber' may change 'n' even if it fails */
  if (pisnum) *pisnum = isnum;
  return n;
}


LUA_API lua_Integer lua_tointegerx (lua_State *L, int idx, int *pisnum) {
  lua_Integer res;
  const luai_TValue *o = lua_index2addr(L, idx);
  int isnum = luai_tointeger(o, &res);
  if (!isnum)
    res = 0;  /* call to 'luai_tointeger' may change 'n' even if it fails */
  if (pisnum) *pisnum = isnum;
  return res;
}


LUA_API int lua_toboolean (lua_State *L, int idx) {
  const luai_TValue *o = lua_index2addr(L, idx);
  return !luai_l_isfalse(o);
}


LUA_API const char *lua_tolstring (lua_State *L, int idx, size_t *len) {
  luai_StkId o = lua_index2addr(L, idx);
  if (!luai_ttisstring(o)) {
    if (!luai_cvt2str(o)) {  /* not convertible? */
      if (len != NULL) *len = 0;
      return NULL;
    }
    lua_lock(L);  /* 'luaO_tostring' may create a new string */
    luaO_tostring(L, o);
    luaC_checkGC(L);
    o = lua_index2addr(L, idx);  /* previous call may reallocate the stack */
    lua_unlock(L);
  }
  if (len != NULL)
    *len = luai_vslen(o);
  return luai_svalue(o);
}


LUA_API size_t lua_rawlen (lua_State *L, int idx) {
  luai_StkId o = lua_index2addr(L, idx);
  switch (luai_ttype(o)) {
    case LUA_TSHRSTR: return luai_tsvalue(o)->shrlen;
    case LUA_TLNGSTR: return luai_tsvalue(o)->u.lnglen;
    case LUA_TUSERDATA: return luai_uvalue(o)->len;
    case LUA_TTABLE: return luaH_getn(luai_hvalue(o));
    default: return 0;
  }
}


LUA_API lua_CFunction lua_tocfunction (lua_State *L, int idx) {
  luai_StkId o = lua_index2addr(L, idx);
  if (luai_ttislcf(o)) return luai_fvalue(o);
  else if (luai_ttisCclosure(o))
    return luai_clCvalue(o)->f;
  else return NULL;  /* not a C function */
}


LUA_API void *lua_touserdata (lua_State *L, int idx) {
  luai_StkId o = lua_index2addr(L, idx);
  switch (luai_ttnov(o)) {
    case LUA_TUSERDATA: return luai_getudatamem(luai_uvalue(o));
    case LUA_TLIGHTUSERDATA: return luai_pvalue(o);
    default: return NULL;
  }
}


LUA_API lua_State *lua_tothread (lua_State *L, int idx) {
  luai_StkId o = lua_index2addr(L, idx);
  return (!luai_ttisthread(o)) ? NULL : luai_thvalue(o);
}


LUA_API const void *lua_topointer (lua_State *L, int idx) {
  luai_StkId o = lua_index2addr(L, idx);
  switch (luai_ttype(o)) {
    case LUA_TTABLE: return luai_hvalue(o);
    case LUA_TLCL: return luai_clLvalue(o);
    case LUA_TCCL: return luai_clCvalue(o);
    case LUA_TLCF: return luai_cast(void *, luai_cast(size_t, luai_fvalue(o)));
    case LUA_TTHREAD: return luai_thvalue(o);
    case LUA_TUSERDATA: return luai_getudatamem(luai_uvalue(o));
    case LUA_TLIGHTUSERDATA: return luai_pvalue(o);
    default: return NULL;
  }
}



/*
** push functions (C -> stack)
*/


LUA_API void lua_pushnil (lua_State *L) {
  lua_lock(L);
  luai_setnilvalue(L->top);
  luai_api_incr_top(L);
  lua_unlock(L);
}


LUA_API void lua_pushnumber (lua_State *L, lua_Number n) {
  lua_lock(L);
  luai_setfltvalue(L->top, n);
  luai_api_incr_top(L);
  lua_unlock(L);
}


LUA_API void lua_pushinteger (lua_State *L, lua_Integer n) {
  lua_lock(L);
  luai_setivalue(L->top, n);
  luai_api_incr_top(L);
  lua_unlock(L);
}


/*
** Pushes on the stack a string with given length. Avoid using 's' when
** 'len' == 0 (as 's' can be NULL in that case), due to later use of
** 'memcmp' and 'memcpy'.
*/
LUA_API const char *lua_pushlstring (lua_State *L, const char *s, size_t len) {
  luai_TString *ts;
  lua_lock(L);
  ts = (len == 0) ? luaS_new(L, "") : luaS_newlstr(L, s, len);
  luai_setsvalue2s(L, L->top, ts);
  luai_api_incr_top(L);
  luaC_checkGC(L);
  lua_unlock(L);
  return luai_getstr(ts);
}


LUA_API const char *lua_pushstring (lua_State *L, const char *s) {
  lua_lock(L);
  if (s == NULL)
    luai_setnilvalue(L->top);
  else {
    luai_TString *ts;
    ts = luaS_new(L, s);
    luai_setsvalue2s(L, L->top, ts);
    s = luai_getstr(ts);  /* internal copy's address */
  }
  luai_api_incr_top(L);
  luaC_checkGC(L);
  lua_unlock(L);
  return s;
}


LUA_API const char *lua_pushvfstring (lua_State *L, const char *fmt,
                                      va_list argp) {
  const char *ret;
  lua_lock(L);
  ret = luaO_pushvfstring(L, fmt, argp);
  luaC_checkGC(L);
  lua_unlock(L);
  return ret;
}


LUA_API const char *lua_pushfstring (lua_State *L, const char *fmt, ...) {
  const char *ret;
  va_list argp;
  lua_lock(L);
  va_start(argp, fmt);
  ret = luaO_pushvfstring(L, fmt, argp);
  va_end(argp);
  luaC_checkGC(L);
  lua_unlock(L);
  return ret;
}


LUA_API void lua_pushcclosure (lua_State *L, lua_CFunction fn, int n) {
  lua_lock(L);
  if (n == 0) {
    luai_setfvalue(L->top, fn);
  }
  else {
    luai_CClosure *cl;
    luai_api_checknelems(L, n);
    luai_api_check(L, n <= LUAI_MAXUPVAL, "upvalue index too large");
    cl = luaF_newCclosure(L, n);
    cl->f = fn;
    L->top -= n;
    while (n--) {
      luai_setobj2n(L, &cl->upvalue[n], L->top + n);
      /* does not need barrier because closure is white */
    }
    luai_setclCvalue(L, L->top, cl);
  }
  luai_api_incr_top(L);
  luaC_checkGC(L);
  lua_unlock(L);
}


LUA_API void lua_pushboolean (lua_State *L, int b) {
  lua_lock(L);
  luai_setbvalue(L->top, (b != 0));  /* ensure that true is 1 */
  luai_api_incr_top(L);
  lua_unlock(L);
}


LUA_API void lua_pushlightuserdata (lua_State *L, void *p) {
  lua_lock(L);
  luai_setpvalue(L->top, p);
  luai_api_incr_top(L);
  lua_unlock(L);
}


LUA_API int lua_pushthread (lua_State *L) {
  lua_lock(L);
  luai_setthvalue(L, L->top, L);
  luai_api_incr_top(L);
  lua_unlock(L);
  return (LUAI_G(L)->mainthread == L);
}



/*
** get functions (Lua -> stack)
*/


static int luai_auxgetstr (lua_State *L, const luai_TValue *t, const char *k) {
  const luai_TValue *slot;
  luai_TString *str = luaS_new(L, k);
  if (luaV_fastget(L, t, str, slot, luaH_getstr)) {
    luai_setobj2s(L, L->top, slot);
    luai_api_incr_top(L);
  }
  else {
    luai_setsvalue2s(L, L->top, str);
    luai_api_incr_top(L);
    luaV_finishget(L, t, L->top - 1, L->top - 1, slot);
  }
  lua_unlock(L);
  return luai_ttnov(L->top - 1);
}


LUA_API int lua_getglobal (lua_State *L, const char *name) {
  luai_Table *reg = luai_hvalue(&LUAI_G(L)->luai_l_registry);
  lua_lock(L);
  return luai_auxgetstr(L, luaH_getint(reg, LUA_RIDX_GLOBALS), name);
}


LUA_API int lua_gettable (lua_State *L, int idx) {
  luai_StkId t;
  lua_lock(L);
  t = lua_index2addr(L, idx);
  luaV_gettable(L, t, L->top - 1, L->top - 1);
  lua_unlock(L);
  return luai_ttnov(L->top - 1);
}


LUA_API int lua_getfield (lua_State *L, int idx, const char *k) {
  lua_lock(L);
  return luai_auxgetstr(L, lua_index2addr(L, idx), k);
}


LUA_API int lua_geti (lua_State *L, int idx, lua_Integer n) {
  luai_StkId t;
  const luai_TValue *slot;
  lua_lock(L);
  t = lua_index2addr(L, idx);
  if (luaV_fastget(L, t, n, slot, luaH_getint)) {
    luai_setobj2s(L, L->top, slot);
    luai_api_incr_top(L);
  }
  else {
    luai_setivalue(L->top, n);
    luai_api_incr_top(L);
    luaV_finishget(L, t, L->top - 1, L->top - 1, slot);
  }
  lua_unlock(L);
  return luai_ttnov(L->top - 1);
}


LUA_API int lua_rawget (lua_State *L, int idx) {
  luai_StkId t;
  lua_lock(L);
  t = lua_index2addr(L, idx);
  luai_api_check(L, luai_ttistable(t), "table expected");
  luai_setobj2s(L, L->top - 1, luaH_get(luai_hvalue(t), L->top - 1));
  lua_unlock(L);
  return luai_ttnov(L->top - 1);
}


LUA_API int lua_rawgeti (lua_State *L, int idx, lua_Integer n) {
  luai_StkId t;
  lua_lock(L);
  t = lua_index2addr(L, idx);
  luai_api_check(L, luai_ttistable(t), "table expected");
  luai_setobj2s(L, L->top, luaH_getint(luai_hvalue(t), n));
  luai_api_incr_top(L);
  lua_unlock(L);
  return luai_ttnov(L->top - 1);
}


LUA_API int lua_rawgetp (lua_State *L, int idx, const void *p) {
  luai_StkId t;
  luai_TValue k;
  lua_lock(L);
  t = lua_index2addr(L, idx);
  luai_api_check(L, luai_ttistable(t), "table expected");
  luai_setpvalue(&k, luai_cast(void *, p));
  luai_setobj2s(L, L->top, luaH_get(luai_hvalue(t), &k));
  luai_api_incr_top(L);
  lua_unlock(L);
  return luai_ttnov(L->top - 1);
}


LUA_API void lua_createtable (lua_State *L, int narray, int nrec) {
  luai_Table *t;
  lua_lock(L);
  t = luaH_new(L);
  luai_sethvalue(L, L->top, t);
  luai_api_incr_top(L);
  if (narray > 0 || nrec > 0)
    luaH_resize(L, t, narray, nrec);
  luaC_checkGC(L);
  lua_unlock(L);
}


LUA_API int lua_getmetatable (lua_State *L, int objindex) {
  const luai_TValue *obj;
  luai_Table *mt;
  int res = 0;
  lua_lock(L);
  obj = lua_index2addr(L, objindex);
  switch (luai_ttnov(obj)) {
    case LUA_TTABLE:
      mt = luai_hvalue(obj)->metatable;
      break;
    case LUA_TUSERDATA:
      mt = luai_uvalue(obj)->metatable;
      break;
    default:
      mt = LUAI_G(L)->mt[luai_ttnov(obj)];
      break;
  }
  if (mt != NULL) {
    luai_sethvalue(L, L->top, mt);
    luai_api_incr_top(L);
    res = 1;
  }
  lua_unlock(L);
  return res;
}


LUA_API int lua_getuservalue (lua_State *L, int idx) {
  luai_StkId o;
  lua_lock(L);
  o = lua_index2addr(L, idx);
  luai_api_check(L, luai_ttisfulluserdata(o), "full userdata expected");
  luai_getuservalue(L, luai_uvalue(o), L->top);
  luai_api_incr_top(L);
  lua_unlock(L);
  return luai_ttnov(L->top - 1);
}


/*
** set functions (stack -> Lua)
*/

/*
** t[k] = value at the top of the stack (where 'k' is a string)
*/
static void luai_auxsetstr (lua_State *L, const luai_TValue *t, const char *k) {
  const luai_TValue *slot;
  luai_TString *str = luaS_new(L, k);
  luai_api_checknelems(L, 1);
  if (luaV_fastset(L, t, str, slot, luaH_getstr, L->top - 1))
    L->top--;  /* pop value */
  else {
    luai_setsvalue2s(L, L->top, str);  /* push 'str' (to make it a luai_TValue) */
    luai_api_incr_top(L);
    luaV_finishset(L, t, L->top - 1, L->top - 2, slot);
    L->top -= 2;  /* pop value and key */
  }
  lua_unlock(L);  /* lock done by caller */
}


LUA_API void lua_setglobal (lua_State *L, const char *name) {
  luai_Table *reg = luai_hvalue(&LUAI_G(L)->luai_l_registry);
  lua_lock(L);  /* unlock done in 'luai_auxsetstr' */
  luai_auxsetstr(L, luaH_getint(reg, LUA_RIDX_GLOBALS), name);
}


LUA_API void lua_settable (lua_State *L, int idx) {
  luai_StkId t;
  lua_lock(L);
  luai_api_checknelems(L, 2);
  t = lua_index2addr(L, idx);
  luaV_settable(L, t, L->top - 2, L->top - 1);
  L->top -= 2;  /* pop index and value */
  lua_unlock(L);
}


LUA_API void lua_setfield (lua_State *L, int idx, const char *k) {
  lua_lock(L);  /* unlock done in 'luai_auxsetstr' */
  luai_auxsetstr(L, lua_index2addr(L, idx), k);
}


LUA_API void lua_seti (lua_State *L, int idx, lua_Integer n) {
  luai_StkId t;
  const luai_TValue *slot;
  lua_lock(L);
  luai_api_checknelems(L, 1);
  t = lua_index2addr(L, idx);
  if (luaV_fastset(L, t, n, slot, luaH_getint, L->top - 1))
    L->top--;  /* pop value */
  else {
    luai_setivalue(L->top, n);
    luai_api_incr_top(L);
    luaV_finishset(L, t, L->top - 1, L->top - 2, slot);
    L->top -= 2;  /* pop value and key */
  }
  lua_unlock(L);
}


LUA_API void lua_rawset (lua_State *L, int idx) {
  luai_StkId o;
  luai_TValue *slot;
  lua_lock(L);
  luai_api_checknelems(L, 2);
  o = lua_index2addr(L, idx);
  luai_api_check(L, luai_ttistable(o), "table expected");
  slot = luaH_set(L, luai_hvalue(o), L->top - 2);
  luai_setobj2t(L, slot, L->top - 1);
  luai_invalidateTMcache(luai_hvalue(o));
  luaC_barrierback(L, luai_hvalue(o), L->top-1);
  L->top -= 2;
  lua_unlock(L);
}


LUA_API void lua_rawseti (lua_State *L, int idx, lua_Integer n) {
  luai_StkId o;
  lua_lock(L);
  luai_api_checknelems(L, 1);
  o = lua_index2addr(L, idx);
  luai_api_check(L, luai_ttistable(o), "table expected");
  luaH_setint(L, luai_hvalue(o), n, L->top - 1);
  luaC_barrierback(L, luai_hvalue(o), L->top-1);
  L->top--;
  lua_unlock(L);
}


LUA_API void lua_rawsetp (lua_State *L, int idx, const void *p) {
  luai_StkId o;
  luai_TValue k, *slot;
  lua_lock(L);
  luai_api_checknelems(L, 1);
  o = lua_index2addr(L, idx);
  luai_api_check(L, luai_ttistable(o), "table expected");
  luai_setpvalue(&k, luai_cast(void *, p));
  slot = luaH_set(L, luai_hvalue(o), &k);
  luai_setobj2t(L, slot, L->top - 1);
  luaC_barrierback(L, luai_hvalue(o), L->top - 1);
  L->top--;
  lua_unlock(L);
}


LUA_API int lua_setmetatable (lua_State *L, int objindex) {
  luai_TValue *obj;
  luai_Table *mt;
  lua_lock(L);
  luai_api_checknelems(L, 1);
  obj = lua_index2addr(L, objindex);
  if (luai_ttisnil(L->top - 1))
    mt = NULL;
  else {
    luai_api_check(L, luai_ttistable(L->top - 1), "table expected");
    mt = luai_hvalue(L->top - 1);
  }
  switch (luai_ttnov(obj)) {
    case LUA_TTABLE: {
      luai_hvalue(obj)->metatable = mt;
      if (mt) {
        luaC_objbarrier(L, luai_gcvalue(obj), mt);
        luaC_checkfinalizer(L, luai_gcvalue(obj), mt);
      }
      break;
    }
    case LUA_TUSERDATA: {
      luai_uvalue(obj)->metatable = mt;
      if (mt) {
        luaC_objbarrier(L, luai_uvalue(obj), mt);
        luaC_checkfinalizer(L, luai_gcvalue(obj), mt);
      }
      break;
    }
    default: {
      LUAI_G(L)->mt[luai_ttnov(obj)] = mt;
      break;
    }
  }
  L->top--;
  lua_unlock(L);
  return 1;
}


LUA_API void lua_setuservalue (lua_State *L, int idx) {
  luai_StkId o;
  lua_lock(L);
  luai_api_checknelems(L, 1);
  o = lua_index2addr(L, idx);
  luai_api_check(L, luai_ttisfulluserdata(o), "full userdata expected");
  luai_setuservalue(L, luai_uvalue(o), L->top - 1);
  luaC_barrier(L, luai_gcvalue(o), L->top - 1);
  L->top--;
  lua_unlock(L);
}


/*
** 'load' and 'call' functions (run Lua code)
*/


#define lua_checkresults(L,na,nr) \
     luai_api_check(L, (nr) == LUA_MULTRET || (L->ci->top - L->top >= (nr) - (na)), \
	"results from function overflow current stack size")


LUA_API void lua_callk (lua_State *L, int nargs, int nresults,
                        lua_KContext ctx, lua_KFunction k) {
  luai_StkId func;
  lua_lock(L);
  luai_api_check(L, k == NULL || !luai_isLua(L->ci),
    "cannot use continuations inside hooks");
  luai_api_checknelems(L, nargs+1);
  luai_api_check(L, L->status == LUA_OK, "cannot do calls on non-normal thread");
  lua_checkresults(L, nargs, nresults);
  func = L->top - (nargs+1);
  if (k != NULL && L->nny == 0) {  /* need to prepare continuation? */
    L->ci->u.c.k = k;  /* luai_save continuation */
    L->ci->u.c.ctx = ctx;  /* luai_save context */
    luaD_call(L, func, nresults);  /* do the call */
  }
  else  /* no continuation or no yieldable */
    luaD_callnoyield(L, func, nresults);  /* just do the call */
  luai_adjustresults(L, nresults);
  lua_unlock(L);
}



/*
** Execute a protected call.
*/
struct CallS {  /* data to 'luai_f_call' */
  luai_StkId func;
  int nresults;
};


static void luai_f_call (lua_State *L, void *ud) {
  struct CallS *c = luai_cast(struct CallS *, ud);
  luaD_callnoyield(L, c->func, c->nresults);
}



LUA_API int lua_pcallk (lua_State *L, int nargs, int nresults, int errfunc,
                        lua_KContext ctx, lua_KFunction k) {
  struct CallS c;
  int status;
  ptrdiff_t func;
  lua_lock(L);
  luai_api_check(L, k == NULL || !luai_isLua(L->ci),
    "cannot use continuations inside hooks");
  luai_api_checknelems(L, nargs+1);
  luai_api_check(L, L->status == LUA_OK, "cannot do calls on non-normal thread");
  lua_checkresults(L, nargs, nresults);
  if (errfunc == 0)
    func = 0;
  else {
    luai_StkId o = lua_index2addr(L, errfunc);
    lua_api_checkstackindex(L, errfunc, o);
    func = luai_savestack(L, o);
  }
  c.func = L->top - (nargs+1);  /* function to be called */
  if (k == NULL || L->nny > 0) {  /* no continuation or no yieldable? */
    c.nresults = nresults;  /* do a 'conventional' protected call */
    status = luaD_pcall(L, luai_f_call, &c, luai_savestack(L, c.func), func);
  }
  else {  /* prepare continuation (call is already protected by 'luai_resume') */
    luai_CallInfo *ci = L->ci;
    ci->u.c.k = k;  /* luai_save continuation */
    ci->u.c.ctx = ctx;  /* luai_save context */
    /* luai_save information for error recovery */
    ci->extra = luai_savestack(L, c.func);
    ci->u.c.old_errfunc = L->errfunc;
    L->errfunc = func;
    luai_setoah(ci->callstatus, L->allowhook);  /* luai_save value of 'allowhook' */
    ci->callstatus |= LUAI_CIST_YPCALL;  /* function can do error recovery */
    luaD_call(L, c.func, nresults);  /* do the call */
    ci->callstatus &= ~LUAI_CIST_YPCALL;
    L->errfunc = ci->u.c.old_errfunc;
    status = LUA_OK;  /* if it is here, there were no errors */
  }
  luai_adjustresults(L, nresults);
  lua_unlock(L);
  return status;
}


LUA_API int lua_load (lua_State *L, lua_Reader reader, void *data,
                      const char *chunkname, const char *mode) {
  LUAI_ZIO z;
  int status;
  lua_lock(L);
  if (!chunkname) chunkname = "?";
  luaZ_init(L, &z, reader, data);
  status = luaD_protectedparser(L, &z, chunkname, mode);
  if (status == LUA_OK) {  /* no errors? */
    luai_LClosure *f = luai_clLvalue(L->top - 1);  /* get newly created function */
    if (f->nupvalues >= 1) {  /* does it have an upvalue? */
      /* get global table from registry */
      luai_Table *reg = luai_hvalue(&LUAI_G(L)->luai_l_registry);
      const luai_TValue *gt = luaH_getint(reg, LUA_RIDX_GLOBALS);
      /* set global table as 1st upvalue of 'f' (may be LUA_ENV) */
      luai_setobj(L, f->upvals[0]->v, gt);
      luaC_upvalbarrier(L, f->upvals[0]);
    }
  }
  lua_unlock(L);
  return status;
}


LUA_API int lua_dump (lua_State *L, lua_Writer luai_writer, void *data, int strip) {
  int status;
  luai_TValue *o;
  lua_lock(L);
  luai_api_checknelems(L, 1);
  o = L->top - 1;
  if (luai_isLfunction(o))
    status = luaU_dump(L, luai_getproto(o), luai_writer, data, strip);
  else
    status = 1;
  lua_unlock(L);
  return status;
}


LUA_API int lua_status (lua_State *L) {
  return L->status;
}


/*
** Garbage-collection function
*/

LUA_API int lua_gc (lua_State *L, int what, int data) {
  int res = 0;
  luai_global_State *g;
  lua_lock(L);
  g = LUAI_G(L);
  switch (what) {
    case LUA_GCSTOP: {
      g->gcrunning = 0;
      break;
    }
    case LUA_GCRESTART: {
      luaE_setdebt(g, 0);
      g->gcrunning = 1;
      break;
    }
    case LUA_GCCOLLECT: {
      luaC_fullgc(L, 0);
      break;
    }
    case LUA_GCCOUNT: {
      /* LUAI_GC values are expressed in Kbytes: #bytes/2^10 */
      res = luai_cast_int(luai_gettotalbytes(g) >> 10);
      break;
    }
    case LUA_GCCOUNTB: {
      res = luai_cast_int(luai_gettotalbytes(g) & 0x3ff);
      break;
    }
    case LUA_GCSTEP: {
      luai_l_mem debt = 1;  /* =1 to signal that it did an actual step */
      luai_lu_byte oldrunning = g->gcrunning;
      g->gcrunning = 1;  /* allow LUAI_GC to run */
      if (data == 0) {
        luaE_setdebt(g, -LUA_GCSTEPSIZE);  /* to do a "small" step */
        luaC_step(L);
      }
      else {  /* add 'data' to total debt */
        debt = luai_cast(luai_l_mem, data) * 1024 + g->LUAI_GCdebt;
        luaE_setdebt(g, debt);
        luaC_checkGC(L);
      }
      g->gcrunning = oldrunning;  /* restore previous state */
      if (debt > 0 && g->gcstate == LUAI_GCSpause)  /* end of cycle? */
        res = 1;  /* signal it */
      break;
    }
    case LUA_GCSETPAUSE: {
      res = g->gcpause;
      g->gcpause = data;
      break;
    }
    case LUA_GCSETSTEPMUL: {
      res = g->gcstepmul;
      if (data < 40) data = 40;  /* avoid ridiculous low values (and 0) */
      g->gcstepmul = data;
      break;
    }
    case LUA_GCISRUNNING: {
      res = g->gcrunning;
      break;
    }
    default: res = -1;  /* invalid option */
  }
  lua_unlock(L);
  return res;
}



/*
** miscellaneous functions
*/


LUA_API int lua_error (lua_State *L) {
  lua_lock(L);
  luai_api_checknelems(L, 1);
  luaG_errormsg(L);
  /* code unreachable; will unlock when control actually leaves the kernel */
  return 0;  /* to avoid warnings */
}


LUA_API int lua_next (lua_State *L, int idx) {
  luai_StkId t;
  int more;
  lua_lock(L);
  t = lua_index2addr(L, idx);
  luai_api_check(L, luai_ttistable(t), "table expected");
  more = luaH_next(L, luai_hvalue(t), L->top - 1);
  if (more) {
    luai_api_incr_top(L);
  }
  else  /* no more elements */
    L->top -= 1;  /* remove key */
  lua_unlock(L);
  return more;
}


LUA_API void lua_concat (lua_State *L, int n) {
  lua_lock(L);
  luai_api_checknelems(L, n);
  if (n >= 2) {
    luaV_concat(L, n);
  }
  else if (n == 0) {  /* push empty string */
    luai_setsvalue2s(L, L->top, luaS_newlstr(L, "", 0));
    luai_api_incr_top(L);
  }
  /* else n == 1; nothing to do */
  luaC_checkGC(L);
  lua_unlock(L);
}


LUA_API void lua_len (lua_State *L, int idx) {
  luai_StkId t;
  lua_lock(L);
  t = lua_index2addr(L, idx);
  luaV_objlen(L, L->top, t);
  luai_api_incr_top(L);
  lua_unlock(L);
}


LUA_API lua_Alloc lua_getallocf (lua_State *L, void **ud) {
  lua_Alloc f;
  lua_lock(L);
  if (ud) *ud = LUAI_G(L)->ud;
  f = LUAI_G(L)->frealloc;
  lua_unlock(L);
  return f;
}


LUA_API void lua_setallocf (lua_State *L, lua_Alloc f, void *ud) {
  lua_lock(L);
  LUAI_G(L)->ud = ud;
  LUAI_G(L)->frealloc = f;
  lua_unlock(L);
}


LUA_API void *lua_newuserdata (lua_State *L, size_t size) {
  luai_Udata *u;
  lua_lock(L);
  u = luaS_newudata(L, size);
  luai_setuvalue(L, L->top, u);
  luai_api_incr_top(L);
  luaC_checkGC(L);
  lua_unlock(L);
  return luai_getudatamem(u);
}



static const char *luai_aux_upvalue (luai_StkId fi, int n, luai_TValue **val,
                                luai_CClosure **owner, luai_UpVal **uv) {
  switch (luai_ttype(fi)) {
    case LUA_TCCL: {  /* C closure */
      luai_CClosure *f = luai_clCvalue(fi);
      if (!(1 <= n && n <= f->nupvalues)) return NULL;
      *val = &f->upvalue[n-1];
      if (owner) *owner = f;
      return "";
    }
    case LUA_TLCL: {  /* Lua closure */
      luai_LClosure *f = luai_clLvalue(fi);
      luai_TString *name;
      luai_Proto *p = f->p;
      if (!(1 <= n && n <= p->sizeupvalues)) return NULL;
      *val = f->upvals[n-1]->v;
      if (uv) *uv = f->upvals[n - 1];
      name = p->upvalues[n-1].name;
      return (name == NULL) ? "(*no name)" : luai_getstr(name);
    }
    default: return NULL;  /* not a closure */
  }
}


LUA_API const char *lua_getupvalue (lua_State *L, int funcindex, int n) {
  const char *name;
  luai_TValue *val = NULL;  /* to avoid warnings */
  lua_lock(L);
  name = luai_aux_upvalue(lua_index2addr(L, funcindex), n, &val, NULL, NULL);
  if (name) {
    luai_setobj2s(L, L->top, val);
    luai_api_incr_top(L);
  }
  lua_unlock(L);
  return name;
}


LUA_API const char *lua_setupvalue (lua_State *L, int funcindex, int n) {
  const char *name;
  luai_TValue *val = NULL;  /* to avoid warnings */
  luai_CClosure *owner = NULL;
  luai_UpVal *uv = NULL;
  luai_StkId fi;
  lua_lock(L);
  fi = lua_index2addr(L, funcindex);
  luai_api_checknelems(L, 1);
  name = luai_aux_upvalue(fi, n, &val, &owner, &uv);
  if (name) {
    L->top--;
    luai_setobj(L, val, L->top);
    if (owner) { luaC_barrier(L, owner, L->top); }
    else if (uv) { luaC_upvalbarrier(L, uv); }
  }
  lua_unlock(L);
  return name;
}


static luai_UpVal **luai_getupvalref (lua_State *L, int fidx, int n, luai_LClosure **pf) {
  luai_LClosure *f;
  luai_StkId fi = lua_index2addr(L, fidx);
  luai_api_check(L, luai_ttisLclosure(fi), "Lua function expected");
  f = luai_clLvalue(fi);
  luai_api_check(L, (1 <= n && n <= f->p->sizeupvalues), "invalid upvalue index");
  if (pf) *pf = f;
  return &f->upvals[n - 1];  /* get its upvalue pointer */
}


LUA_API void *lua_upvalueid (lua_State *L, int fidx, int n) {
  luai_StkId fi = lua_index2addr(L, fidx);
  switch (luai_ttype(fi)) {
    case LUA_TLCL: {  /* lua closure */
      return *luai_getupvalref(L, fidx, n, NULL);
    }
    case LUA_TCCL: {  /* C closure */
      luai_CClosure *f = luai_clCvalue(fi);
      luai_api_check(L, 1 <= n && n <= f->nupvalues, "invalid upvalue index");
      return &f->upvalue[n - 1];
    }
    default: {
      luai_api_check(L, 0, "closure expected");
      return NULL;
    }
  }
}


LUA_API void lua_upvaluejoin (lua_State *L, int fidx1, int n1,
                                            int fidx2, int n2) {
  luai_LClosure *f1;
  luai_UpVal **up1 = luai_getupvalref(L, fidx1, n1, &f1);
  luai_UpVal **up2 = luai_getupvalref(L, fidx2, n2, NULL);
  luaC_upvdeccount(L, *up1);
  *up1 = *up2;
  (*up1)->refcount++;
  if (luai_upisopen(*up1)) (*up1)->u.open.touched = 1;
  luaC_upvalbarrier(L, *up1);
}

/*__lauxlib.c__*/

/*
** {======================================================
** Traceback
** =======================================================
*/


#define LUA_LEVELS1	10	/* size of the first part of the stack */
#define LUA_LEVELS2	11	/* size of the second part of the stack */



/*
** search for 'objidx' in table at index -1.
** return 1 + string at top if find a good name.
*/
static int luai_findfield (lua_State *L, int objidx, int level) {
  if (level == 0 || !lua_istable(L, -1))
    return 0;  /* not found */
  lua_pushnil(L);  /* start 'luai_next' loop */
  while (lua_next(L, -2)) {  /* for each pair in table */
    if (lua_type(L, -2) == LUA_TSTRING) {  /* ignore non-string keys */
      if (lua_rawequal(L, objidx, -1)) {  /* found object? */
        lua_pop(L, 1);  /* remove value (but keep name) */
        return 1;
      }
      else if (luai_findfield(L, objidx, level - 1)) {  /* try recursively */
        lua_remove(L, -2);  /* remove table (but keep name) */
        lua_pushliteral(L, ".");
        lua_insert(L, -2);  /* place '.' between the two names */
        lua_concat(L, 3);
        return 1;
      }
    }
    lua_pop(L, 1);  /* remove value */
  }
  return 0;  /* not found */
}


/*
** Search for a name for a function in all loaded modules
*/
static int luai_pushglobalfuncname (lua_State *L, lua_Debug *ar) {
  int top = lua_gettop(L);
  lua_getinfo(L, "f", ar);  /* push function */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  if (luai_findfield(L, top + 1, 2)) {
    const char *name = lua_tostring(L, -1);
    if (strncmp(name, "_G.", 3) == 0) {  /* name start with '_G.'? */
      lua_pushstring(L, name + 3);  /* push name without prefix */
      lua_remove(L, -2);  /* remove original name */
    }
    lua_copy(L, -1, top + 1);  /* move name to proper place */
    lua_pop(L, 2);  /* remove pushed values */
    return 1;
  }
  else {
    lua_settop(L, top);  /* remove function and global table */
    return 0;
  }
}


static void luai_pushfuncname (lua_State *L, lua_Debug *ar) {
  if (luai_pushglobalfuncname(L, ar)) {  /* try first a global name */
    lua_pushfstring(L, "function '%s'", lua_tostring(L, -1));
    lua_remove(L, -2);  /* remove name */
  }
  else if (*ar->namewhat != '\0')  /* is there a name from code? */
    lua_pushfstring(L, "%s '%s'", ar->namewhat, ar->name);  /* use it */
  else if (*ar->what == 'm')  /* main? */
      lua_pushliteral(L, "main chunk");
  else if (*ar->what != 'C')  /* for Lua functions, use <file:line> */
    lua_pushfstring(L, "function <%s:%d>", ar->short_src, ar->linedefined);
  else  /* nothing left... */
    lua_pushliteral(L, "?");
}


static int luai_lastlevel (lua_State *L) {
  lua_Debug ar;
  int li = 1, le = 1;
  /* find an upper bound */
  while (lua_getstack(L, le, &ar)) { li = le; le *= 2; }
  /* do a binary search */
  while (li < le) {
    int m = (li + le)/2;
    if (lua_getstack(L, m, &ar)) li = m + 1;
    else le = m;
  }
  return le - 1;
}


LUALIB_API void luaL_traceback (lua_State *L, lua_State *L1,
                                const char *msg, int level) {
  lua_Debug ar;
  int top = lua_gettop(L);
  int last = luai_lastlevel(L1);
  int n1 = (last - level > LUA_LEVELS1 + LUA_LEVELS2) ? LUA_LEVELS1 : -1;
  if (msg)
    lua_pushfstring(L, "%s\n", msg);
  luaL_checkstack(L, 10, NULL);
  lua_pushliteral(L, "stack traceback:");
  while (lua_getstack(L1, level++, &ar)) {
    if (n1-- == 0) {  /* too many levels? */
      lua_pushliteral(L, "\n\t...");  /* add a '...' */
      level = last - LUA_LEVELS2 + 1;  /* and skip to last ones */
    }
    else {
      lua_getinfo(L1, "Slnt", &ar);
      lua_pushfstring(L, "\n\t%s:", ar.short_src);
      if (ar.luai_currentline > 0)
        lua_pushfstring(L, "%d:", ar.luai_currentline);
      lua_pushliteral(L, " in ");
      luai_pushfuncname(L, &ar);
      if (ar.istailcall)
        lua_pushliteral(L, "\n\t(...tail calls...)");
      lua_concat(L, lua_gettop(L) - top);
    }
  }
  lua_concat(L, lua_gettop(L) - top);
}

/* }====================================================== */


/*
** {======================================================
** Error-report functions
** =======================================================
*/

LUALIB_API int luaL_argerror (lua_State *L, int arg, const char *extramsg) {
  lua_Debug ar;
  if (!lua_getstack(L, 0, &ar))  /* no stack frame? */
    return luaL_error(L, "bad argument #%d (%s)", arg, extramsg);
  lua_getinfo(L, "n", &ar);
  if (strcmp(ar.namewhat, "method") == 0) {
    arg--;  /* do not count 'self' */
    if (arg == 0)  /* error is in the self argument itself? */
      return luaL_error(L, "calling '%s' on bad self (%s)",
                           ar.name, extramsg);
  }
  if (ar.name == NULL)
    ar.name = (luai_pushglobalfuncname(L, &ar)) ? lua_tostring(L, -1) : "?";
  return luaL_error(L, "bad argument #%d to '%s' (%s)",
                        arg, ar.name, extramsg);
}


static int luai_typeerror (lua_State *L, int arg, const char *tname) {
  const char *msg;
  const char *typearg;  /* name for the type of the actual argument */
  if (luaL_getmetafield(L, arg, "__name") == LUA_TSTRING)
    typearg = lua_tostring(L, -1);  /* use the given type name */
  else if (lua_type(L, arg) == LUA_TLIGHTUSERDATA)
    typearg = "light userdata";  /* special name for messages */
  else
    typearg = luaL_typename(L, arg);  /* standard name */
  msg = lua_pushfstring(L, "%s expected, got %s", tname, typearg);
  return luaL_argerror(L, arg, msg);
}


static void luai_tagerror (lua_State *L, int arg, int tag) {
  luai_typeerror(L, arg, lua_typename(L, tag));
}


/*
** The use of 'lua_pushfstring' ensures this function does not
** need reserved stack space when called.
*/
LUALIB_API void luaL_where (lua_State *L, int level) {
  lua_Debug ar;
  if (lua_getstack(L, level, &ar)) {  /* luai_check function at level */
    lua_getinfo(L, "Sl", &ar);  /* get info about it */
    if (ar.luai_currentline > 0) {  /* is there info? */
      lua_pushfstring(L, "%s:%d: ", ar.short_src, ar.luai_currentline);
      return;
    }
  }
  lua_pushfstring(L, "");  /* else, no information available... */
}


/*
** Again, the use of 'lua_pushvfstring' ensures this function does
** not need reserved stack space when called. (At worst, it generates
** an error with "stack overflow" instead of the given message.)
*/
LUALIB_API int luaL_error (lua_State *L, const char *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  luaL_where(L, 1);
  lua_pushvfstring(L, fmt, argp);
  va_end(argp);
  lua_concat(L, 2);
  return lua_error(L);
}


LUALIB_API int luaL_fileresult (lua_State *L, int stat, const char *fname) {
  int en = errno;  /* calls to Lua API may change this value */
  if (stat) {
    lua_pushboolean(L, 1);
    return 1;
  }
  else {
    lua_pushnil(L);
    if (fname)
      lua_pushfstring(L, "%s: %s", fname, strerror(en));
    else
      lua_pushstring(L, strerror(en));
    lua_pushinteger(L, en);
    return 3;
  }
}


#if !defined(luai_l_inspectstat)	/* { */

#if defined(LUA_USE_POSIX)

#include <sys/wait.h>

/*
** use appropriate macros to interpret 'pclose' return status
*/
#define luai_l_inspectstat(stat,what)  \
   if (WIFEXITED(stat)) { stat = WEXITSTATUS(stat); } \
   else if (WIFSIGNALED(stat)) { stat = WTERMSIG(stat); what = "signal"; }

#else

#define luai_l_inspectstat(stat,what)  /* no op */

#endif

#endif				/* } */


LUALIB_API int luaL_execresult (lua_State *L, int stat) {
  const char *what = "exit";  /* type of termination */
  if (stat == -1)  /* error? */
    return luaL_fileresult(L, 0, NULL);
  else {
    luai_l_inspectstat(stat, what);  /* interpret result */
    if (*what == 'e' && stat == 0)  /* successful termination? */
      lua_pushboolean(L, 1);
    else
      lua_pushnil(L);
    lua_pushstring(L, what);
    lua_pushinteger(L, stat);
    return 3;  /* return true/nil,what,code */
  }
}

/* }====================================================== */


/*
** {======================================================
** Userdata's metatable manipulation
** =======================================================
*/

LUALIB_API int luaL_newmetatable (lua_State *L, const char *tname) {
  if (luaL_getmetatable(L, tname) != LUA_TNIL)  /* name already in use? */
    return 0;  /* leave previous value on top, but return 0 */
  lua_pop(L, 1);
  lua_createtable(L, 0, 2);  /* create metatable */
  lua_pushstring(L, tname);
  lua_setfield(L, -2, "__name");  /* metatable.__name = tname */
  lua_pushvalue(L, -1);
  lua_setfield(L, LUA_REGISTRYINDEX, tname);  /* registry.name = metatable */
  return 1;
}


LUALIB_API void luaL_setmetatable (lua_State *L, const char *tname) {
  luaL_getmetatable(L, tname);
  lua_setmetatable(L, -2);
}


LUALIB_API void *luaL_testudata (lua_State *L, int ud, const char *tname) {
  void *p = lua_touserdata(L, ud);
  if (p != NULL) {  /* value is a userdata? */
    if (lua_getmetatable(L, ud)) {  /* does it have a metatable? */
      luaL_getmetatable(L, tname);  /* get correct metatable */
      if (!lua_rawequal(L, -1, -2))  /* not the same? */
        p = NULL;  /* value is a userdata with wrong metatable */
      lua_pop(L, 2);  /* remove both metatables */
      return p;
    }
  }
  return NULL;  /* value is not a userdata with a metatable */
}


LUALIB_API void *luaL_checkudata (lua_State *L, int ud, const char *tname) {
  void *p = luaL_testudata(L, ud, tname);
  if (p == NULL) luai_typeerror(L, ud, tname);
  return p;
}

/* }====================================================== */


/*
** {======================================================
** Argument luai_check functions
** =======================================================
*/

LUALIB_API int luaL_checkoption (lua_State *L, int arg, const char *def,
                                 const char *const lst[]) {
  const char *name = (def) ? luaL_optstring(L, arg, def) :
                             luaL_checkstring(L, arg);
  int i;
  for (i=0; lst[i]; i++)
    if (strcmp(lst[i], name) == 0)
      return i;
  return luaL_argerror(L, arg,
                       lua_pushfstring(L, "invalid option '%s'", name));
}


/*
** Ensures the stack has at least 'space' extra slots, raising an error
** if it cannot fulfill the request. (The error handling needs a few
** extra slots to format the error message. In case of an error without
** this extra space, Lua will generate the same 'stack overflow' error,
** but without 'msg'.)
*/
LUALIB_API void luaL_checkstack (lua_State *L, int space, const char *msg) {
  if (!lua_checkstack(L, space)) {
    if (msg)
      luaL_error(L, "stack overflow (%s)", msg);
    else
      luaL_error(L, "stack overflow");
  }
}


LUALIB_API void luaL_checktype (lua_State *L, int arg, int t) {
  if (lua_type(L, arg) != t)
    luai_tagerror(L, arg, t);
}


LUALIB_API void luaL_checkany (lua_State *L, int arg) {
  if (lua_type(L, arg) == LUA_TNONE)
    luaL_argerror(L, arg, "value expected");
}


LUALIB_API const char *luaL_checklstring (lua_State *L, int arg, size_t *len) {
  const char *s = lua_tolstring(L, arg, len);
  if (!s) luai_tagerror(L, arg, LUA_TSTRING);
  return s;
}


LUALIB_API const char *luaL_optlstring (lua_State *L, int arg,
                                        const char *def, size_t *len) {
  if (lua_isnoneornil(L, arg)) {
    if (len)
      *len = (def ? strlen(def) : 0);
    return def;
  }
  else return luaL_checklstring(L, arg, len);
}


LUALIB_API lua_Number luaL_checknumber (lua_State *L, int arg) {
  int isnum;
  lua_Number d = lua_tonumberx(L, arg, &isnum);
  if (!isnum)
    luai_tagerror(L, arg, LUA_TNUMBER);
  return d;
}


LUALIB_API lua_Number luaL_optnumber (lua_State *L, int arg, lua_Number def) {
  return luaL_opt(L, luaL_checknumber, arg, def);
}


static void luai_interror (lua_State *L, int arg) {
  if (lua_isnumber(L, arg))
    luaL_argerror(L, arg, "number has no integer representation");
  else
    luai_tagerror(L, arg, LUA_TNUMBER);
}


LUALIB_API lua_Integer luaL_checkinteger (lua_State *L, int arg) {
  int isnum;
  lua_Integer d = lua_tointegerx(L, arg, &isnum);
  if (!isnum) {
    luai_interror(L, arg);
  }
  return d;
}


LUALIB_API lua_Integer luaL_optinteger (lua_State *L, int arg,
                                                      lua_Integer def) {
  return luaL_opt(L, luaL_checkinteger, arg, def);
}

/* }====================================================== */


/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

/* userdata to box arbitrary data */
typedef struct luai_UBox {
  void *box;
  size_t bsize;
} luai_UBox;


static void *luai_resizebox (lua_State *L, int idx, size_t newsize) {
  void *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);
  luai_UBox *box = (luai_UBox *)lua_touserdata(L, idx);
  void *temp = allocf(ud, box->box, box->bsize, newsize);
  if (temp == NULL && newsize > 0) {  /* allocation error? */
    luai_resizebox(L, idx, 0);  /* free buffer */
    luaL_error(L, "not enough memory for buffer allocation");
  }
  box->box = temp;
  box->bsize = newsize;
  return temp;
}


static int luai_boxgc (lua_State *L) {
  luai_resizebox(L, 1, 0);
  return 0;
}


static void *luai_newbox (lua_State *L, size_t newsize) {
  luai_UBox *box = (luai_UBox *)lua_newuserdata(L, sizeof(luai_UBox));
  box->box = NULL;
  box->bsize = 0;
  if (luaL_newmetatable(L, "LUABOX")) {  /* creating metatable? */
    lua_pushcfunction(L, luai_boxgc);
    lua_setfield(L, -2, "__gc");  /* metatable.__gc = luai_boxgc */
  }
  lua_setmetatable(L, -2);
  return luai_resizebox(L, -1, newsize);
}


/*
** luai_check whether buffer is using a userdata on the stack as a temporary
** buffer
*/
#define lua_buffonstack(B)	((B)->b != (B)->initb)


/*
** returns a pointer to a free area with at least 'sz' bytes
*/
LUALIB_API char *luaL_prepbuffsize (luaL_Buffer *B, size_t sz) {
  lua_State *L = B->L;
  if (B->size - B->n < sz) {  /* not enough space? */
    char *newbuff;
    size_t newsize = B->size * 2;  /* double buffer size */
    if (newsize - B->n < sz)  /* not big enough? */
      newsize = B->n + sz;
    if (newsize < B->n || newsize - B->n < sz)
      luaL_error(L, "buffer too large");
    /* create larger buffer */
    if (lua_buffonstack(B))
      newbuff = (char *)luai_resizebox(L, -1, newsize);
    else {  /* no buffer yet */
      newbuff = (char *)luai_newbox(L, newsize);
      memcpy(newbuff, B->b, B->n * sizeof(char));  /* copy original content */
    }
    B->b = newbuff;
    B->size = newsize;
  }
  return &B->b[B->n];
}


LUALIB_API void luaL_addlstring (luaL_Buffer *B, const char *s, size_t l) {
  if (l > 0) {  /* avoid 'memcpy' when 's' can be NULL */
    char *b = luaL_prepbuffsize(B, l);
    memcpy(b, s, l * sizeof(char));
    luaL_addsize(B, l);
  }
}


LUALIB_API void luaL_addstring (luaL_Buffer *B, const char *s) {
  luaL_addlstring(B, s, strlen(s));
}


LUALIB_API void luaL_pushresult (luaL_Buffer *B) {
  lua_State *L = B->L;
  lua_pushlstring(L, B->b, B->n);
  if (lua_buffonstack(B)) {
    luai_resizebox(L, -2, 0);  /* delete old buffer */
    lua_remove(L, -2);  /* remove its header from the stack */
  }
}


LUALIB_API void luaL_pushresultsize (luaL_Buffer *B, size_t sz) {
  luaL_addsize(B, sz);
  luaL_pushresult(B);
}


LUALIB_API void luaL_addvalue (luaL_Buffer *B) {
  lua_State *L = B->L;
  size_t l;
  const char *s = lua_tolstring(L, -1, &l);
  if (lua_buffonstack(B))
    lua_insert(L, -2);  /* put value below buffer */
  luaL_addlstring(B, s, l);
  lua_remove(L, (lua_buffonstack(B)) ? -2 : -1);  /* remove value */
}


LUALIB_API void luaL_buffinit (lua_State *L, luaL_Buffer *B) {
  B->L = L;
  B->b = B->initb;
  B->n = 0;
  B->size = LUAL_BUFFERSIZE;
}


LUALIB_API char *luaL_buffinitsize (lua_State *L, luaL_Buffer *B, size_t sz) {
  luaL_buffinit(L, B);
  return luaL_prepbuffsize(B, sz);
}

/* }====================================================== */


/*
** {======================================================
** Reference system
** =======================================================
*/

/* index of free-list header */
#define lua_freelist	0


LUALIB_API int luaL_ref (lua_State *L, int t) {
  int ref;
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);  /* remove from stack */
    return LUA_REFNIL;  /* 'nil' has a unique fixed reference */
  }
  t = lua_absindex(L, t);
  lua_rawgeti(L, t, lua_freelist);  /* get first free element */
  ref = (int)lua_tointeger(L, -1);  /* ref = t[lua_freelist] */
  lua_pop(L, 1);  /* remove it from stack */
  if (ref != 0) {  /* any free element? */
    lua_rawgeti(L, t, ref);  /* remove it luai_list */
    lua_rawseti(L, t, lua_freelist);  /* (t[lua_freelist] = t[ref]) */
  }
  else  /* no free elements */
    ref = (int)lua_rawlen(L, t) + 1;  /* get a new reference */
  lua_rawseti(L, t, ref);
  return ref;
}


LUALIB_API void luaL_unref (lua_State *L, int t, int ref) {
  if (ref >= 0) {
    t = lua_absindex(L, t);
    lua_rawgeti(L, t, lua_freelist);
    lua_rawseti(L, t, ref);  /* t[ref] = t[lua_freelist] */
    lua_pushinteger(L, ref);
    lua_rawseti(L, t, lua_freelist);  /* t[lua_freelist] = ref */
  }
}

/* }====================================================== */


/*
** {======================================================
** Load functions
** =======================================================
*/

typedef struct luai_LoadF {
  int n;  /* number of pre-read characters */
  FILE *f;  /* file being read */
  char buff[BUFSIZ];  /* area for reading file */
} luai_LoadF;


static const char *luai_getF (lua_State *L, void *ud, size_t *size) {
  luai_LoadF *lf = (luai_LoadF *)ud;
  (void)L;  /* not used */
  if (lf->n > 0) {  /* are there pre-read characters to be read? */
    *size = lf->n;  /* return them (chars already in buffer) */
    lf->n = 0;  /* no more pre-read characters */
  }
  else {  /* read a luai_getblock from file */
    /* 'fread' can return > 0 *and* set the EOF flag. If luai_next call to
       'luai_getF' called 'fread', it might still wait for user input.
       The luai_next luai_check avoids this problem. */
    if (feof(lf->f)) return NULL;
    *size = fread(lf->buff, 1, sizeof(lf->buff), lf->f);  /* read luai_getblock */
  }
  return lf->buff;
}


static int luai_errfile (lua_State *L, const char *what, int fnameindex) {
  const char *serr = strerror(errno);
  const char *filename = lua_tostring(L, fnameindex) + 1;
  lua_pushfstring(L, "cannot %s %s: %s", what, filename, serr);
  lua_remove(L, fnameindex);
  return LUA_ERRFILE;
}


static int luai_skipBOM (luai_LoadF *lf) {
  const char *p = "\xEF\xBB\xBF";  /* UTF-8 BOM mark */
  int c;
  lf->n = 0;
  do {
    c = getc(lf->f);
    if (c == EOF || c != *(const unsigned char *)p++) return c;
    lf->buff[lf->n++] = c;  /* to be read by the parser */
  } while (*p != '\0');
  lf->n = 0;  /* prefix matched; discard it */
  return getc(lf->f);  /* return luai_next character */
}


/*
** reads the first character of file 'f' and skips an optional BOM mark
** in its beginning plus its first line if it starts with '#'. Returns
** true if it skipped the first line.  In any case, '*cp' has the
** first "valid" character of the file (after the optional BOM and
** a first-line comment).
*/
static int luai_skipcomment (luai_LoadF *lf, int *cp) {
  int c = *cp = luai_skipBOM(lf);
  if (c == '#') {  /* first line is a comment (Unix exec. file)? */
    do {  /* skip first line */
      c = getc(lf->f);
    } while (c != EOF && c != '\n');
    *cp = getc(lf->f);  /* skip end-of-line, if present */
    return 1;  /* there was a comment */
  }
  else return 0;  /* no comment */
}


LUALIB_API int luaL_loadfilex (lua_State *L, const char *filename,
                                             const char *mode) {
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


typedef struct luai_LoadS {
  const char *s;
  size_t size;
} luai_LoadS;


static const char *luai_getS (lua_State *L, void *ud, size_t *size) {
  luai_LoadS *ls = (luai_LoadS *)ud;
  (void)L;  /* not used */
  if (ls->size == 0) return NULL;
  *size = ls->size;
  ls->size = 0;
  return ls->s;
}


LUALIB_API int luaL_loadbufferx (lua_State *L, const char *buff, size_t size,
                                 const char *name, const char *mode) {
  luai_LoadS ls;
  ls.s = buff;
  ls.size = size;
  return lua_load(L, luai_getS, &ls, name, mode);
}


LUALIB_API int luaL_loadstring (lua_State *L, const char *s) {
  return luaL_loadbuffer(L, s, strlen(s), s);
}

/* }====================================================== */



LUALIB_API int luaL_getmetafield (lua_State *L, int obj, const char *event) {
  if (!lua_getmetatable(L, obj))  /* no metatable? */
    return LUA_TNIL;
  else {
    int tt;
    lua_pushstring(L, event);
    tt = lua_rawget(L, -2);
    if (tt == LUA_TNIL)  /* is metafield nil? */
      lua_pop(L, 2);  /* remove metatable and metafield */
    else
      lua_remove(L, -2);  /* remove only metatable */
    return tt;  /* return metafield type */
  }
}


LUALIB_API int luaL_callmeta (lua_State *L, int obj, const char *event) {
  obj = lua_absindex(L, obj);
  if (luaL_getmetafield(L, obj, event) == LUA_TNIL)  /* no metafield? */
    return 0;
  lua_pushvalue(L, obj);
  lua_call(L, 1, 1);
  return 1;
}


LUALIB_API lua_Integer luaL_len (lua_State *L, int idx) {
  lua_Integer l;
  int isnum;
  lua_len(L, idx);
  l = lua_tointegerx(L, -1, &isnum);
  if (!isnum)
    luaL_error(L, "object length is not an integer");
  lua_pop(L, 1);  /* remove object */
  return l;
}


LUALIB_API const char *luaL_tolstring (lua_State *L, int idx, size_t *len) {
  if (luaL_callmeta(L, idx, "__tostring")) {  /* metafield? */
    if (!lua_isstring(L, -1))
      luaL_error(L, "'__tostring' must return a string");
  }
  else {
    switch (lua_type(L, idx)) {
      case LUA_TNUMBER: {
        if (lua_isinteger(L, idx))
          lua_pushfstring(L, "%I", (LUAI_UACINT)lua_tointeger(L, idx));
        else
          lua_pushfstring(L, "%f", (LUAI_UACNUMBER)lua_tonumber(L, idx));
        break;
      }
      case LUA_TSTRING:
        lua_pushvalue(L, idx);
        break;
      case LUA_TBOOLEAN:
        lua_pushstring(L, (lua_toboolean(L, idx) ? "true" : "false"));
        break;
      case LUA_TNIL:
        lua_pushliteral(L, "nil");
        break;
      default: {
        int tt = luaL_getmetafield(L, idx, "__name");  /* try name */
        const char *kind = (tt == LUA_TSTRING) ? lua_tostring(L, -1) :
                                                 luaL_typename(L, idx);
        lua_pushfstring(L, "%s: %p", kind, lua_topointer(L, idx));
        if (tt != LUA_TNIL)
          lua_remove(L, -2);  /* remove '__name' */
        break;
      }
    }
  }
  return lua_tolstring(L, -1, len);
}


/*
** {======================================================
** Compatibility with 5.1 module functions
** =======================================================
*/
#if defined(LUA_COMPAT_MODULE)

static const char *luaL_findtable (lua_State *L, int idx,
                                   const char *fname, int szhint) {
  const char *e;
  if (idx) lua_pushvalue(L, idx);
  do {
    e = strchr(fname, '.');
    if (e == NULL) e = fname + strlen(fname);
    lua_pushlstring(L, fname, e - fname);
    if (lua_rawget(L, -2) == LUA_TNIL) {  /* no such luai_field? */
      lua_pop(L, 1);  /* remove this nil */
      lua_createtable(L, 0, (*e == '.' ? 1 : szhint)); /* new table for luai_field */
      lua_pushlstring(L, fname, e - fname);
      lua_pushvalue(L, -2);
      lua_settable(L, -4);  /* set new table into luai_field */
    }
    else if (!lua_istable(L, -1)) {  /* luai_field has a non-table value? */
      lua_pop(L, 2);  /* remove table and value */
      return fname;  /* return problematic part of the name */
    }
    lua_remove(L, -2);  /* remove previous table */
    fname = e + 1;
  } while (*e == '.');
  return NULL;
}


/*
** Count number of elements in a luaL_Reg list.
*/
static int luai_libsize (const luaL_Reg *l) {
  int size = 0;
  for (; l && l->name; l++) size++;
  return size;
}


/*
** Find or create a module table with a given name. The function
** first looks at the LOADED table and, if that fails, try a
** global variable with that name. In any case, leaves on the stack
** the module table.
*/
LUALIB_API void luaL_pushmodule (lua_State *L, const char *modname,
                                 int sizehint) {
  luaL_findtable(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE, 1);
  if (lua_getfield(L, -1, modname) != LUA_TTABLE) {  /* no LOADED[modname]? */
    lua_pop(L, 1);  /* remove previous result */
    /* try global variable (and create one if it does not exist) */
    lua_pushglobaltable(L);
    if (luaL_findtable(L, 0, modname, sizehint) != NULL)
      luaL_error(L, "name conflict for module '%s'", modname);
    lua_pushvalue(L, -1);
    lua_setfield(L, -3, modname);  /* LOADED[modname] = new table */
  }
  lua_remove(L, -2);  /* remove LOADED table */
}


LUALIB_API void luaL_openlib (lua_State *L, const char *libname,
                               const luaL_Reg *l, int nup) {
  luaL_checkversion(L);
  if (libname) {
    luaL_pushmodule(L, libname, luai_libsize(l));  /* get/create library table */
    lua_insert(L, -(nup + 1));  /* move library table to below upvalues */
  }
  if (l)
    luaL_setfuncs(L, l, nup);
  else
    lua_pop(L, nup);  /* remove upvalues */
}

#endif
/* }====================================================== */

/*
** set functions from list 'l' into table at top - 'nup'; each
** function gets the 'nup' elements at the top as upvalues.
** Returns with only the table at the stack.
*/
LUALIB_API void luaL_setfuncs (lua_State *L, const luaL_Reg *l, int nup) {
  luaL_checkstack(L, nup, "too many upvalues");
  for (; l->name != NULL; l++) {  /* fill the table with given functions */
    int i;
    for (i = 0; i < nup; i++)  /* copy upvalues to the top */
      lua_pushvalue(L, -nup);
    lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
    lua_setfield(L, -(nup + 2), l->name);
  }
  lua_pop(L, nup);  /* remove upvalues */
}


/*
** ensure that stack[idx][fname] has a table and push that table
** into the stack
*/
LUALIB_API int luaL_getsubtable (lua_State *L, int idx, const char *fname) {
  if (lua_getfield(L, idx, fname) == LUA_TTABLE)
    return 1;  /* table already there */
  else {
    lua_pop(L, 1);  /* remove previous result */
    idx = lua_absindex(L, idx);
    lua_newtable(L);
    lua_pushvalue(L, -1);  /* copy to be left at top */
    lua_setfield(L, idx, fname);  /* assign new table to luai_field */
    return 0;  /* false, because did not find table there */
  }
}


/*
** Stripped-down 'require': After checking "loaded" table, calls 'openf'
** to open a module, registers the result in 'package.loaded' table and,
** if 'glb' is true, also registers the result in the global table.
** Leaves resulting module on the top.
*/
LUALIB_API void luaL_requiref (lua_State *L, const char *modname,
                               lua_CFunction openf, int glb) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  lua_getfield(L, -1, modname);  /* LOADED[modname] */
  if (!lua_toboolean(L, -1)) {  /* package not already loaded? */
    lua_pop(L, 1);  /* remove luai_field */
    lua_pushcfunction(L, openf);
    lua_pushstring(L, modname);  /* argument to open function */
    lua_call(L, 1, 1);  /* call 'openf' to open module */
    lua_pushvalue(L, -1);  /* make copy of module (call result) */
    lua_setfield(L, -3, modname);  /* LOADED[modname] = module */
  }
  lua_remove(L, -2);  /* remove LOADED table */
  if (glb) {
    lua_pushvalue(L, -1);  /* copy of module */
    lua_setglobal(L, modname);  /* _G[modname] = module */
  }
}


LUALIB_API const char *luaL_gsub (lua_State *L, const char *s, const char *p,
                                                               const char *r) {
  const char *wild;
  size_t l = strlen(p);
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  while ((wild = strstr(s, p)) != NULL) {
    luaL_addlstring(&b, s, wild - s);  /* push prefix */
    luaL_addstring(&b, r);  /* push replacement in place of pattern */
    s = wild + l;  /* continue after 'p' */
  }
  luaL_addstring(&b, s);  /* push last suffix */
  luaL_pushresult(&b);
  return lua_tostring(L, -1);
}


static void *luai_l_alloc (void *ud, void *ptr, size_t osize, size_t nsize) {
  (void)ud; (void)osize;  /* not used */
  if (nsize == 0) {
    free(ptr);
    return NULL;
  }
  else
    return realloc(ptr, nsize);
}


static int luai_panic (lua_State *L) {
  lua_writestringerror("PANIC: unprotected error in call to Lua API (%s)\n",
                        lua_tostring(L, -1));
  return 0;  /* return to Lua to abort */
}


LUALIB_API lua_State *luaL_newstate (void) {
  lua_State *L = lua_newstate(luai_l_alloc, NULL);
  if (L) lua_atpanic(L, &luai_panic);
  return L;
}


LUALIB_API void luaL_checkversion_ (lua_State *L, lua_Number ver, size_t sz) {
  const lua_Number *v = lua_version(L);
  if (sz != LUAL_NUMSIZES)  /* luai_check numeric types */
    luaL_error(L, "core and library have incompatible numeric types");
  if (v != lua_version(NULL))
    luaL_error(L, "multiple Lua VMs detected");
  else if (*v != ver)
    luaL_error(L, "version mismatch: app. needs %f, Lua core provides %f",
                  (LUAI_UACNUMBER)ver, (LUAI_UACNUMBER)*v);
}

/*__lbaselib.c__*/

static int luaB_print (lua_State *L) {
  int n = lua_gettop(L);  /* number of arguments */
  int i;
  lua_getglobal(L, "tostring");
  for (i=1; i<=n; i++) {
    const char *s;
    size_t l;
    lua_pushvalue(L, -1);  /* function to be called */
    lua_pushvalue(L, i);   /* value to print */
    lua_call(L, 1, 1);
    s = lua_tolstring(L, -1, &l);  /* get result */
    if (s == NULL)
      return luaL_error(L, "'tostring' must return a string to 'print'");
    if (i>1) lua_writestring("\t", 1);
    lua_writestring(s, l);
    lua_pop(L, 1);  /* pop result */
  }
  lua_writeline();
  return 0;
}


#define LUA_SPACECHARS	" \f\n\r\t\v"

static const char *luai_b_str2int (const char *s, int base, lua_Integer *pn) {
  lua_Unsigned n = 0;
  int neg = 0;
  s += strspn(s, LUA_SPACECHARS);  /* skip initial spaces */
  if (*s == '-') { s++; neg = 1; }  /* handle signal */
  else if (*s == '+') s++;
  if (!isalnum((unsigned char)*s))  /* no luai_digit? */
    return NULL;
  do {
    int luai_digit = (isdigit((unsigned char)*s)) ? *s - '0'
                   : (toupper((unsigned char)*s) - 'A') + 10;
    if (luai_digit >= base) return NULL;  /* invalid numeral */
    n = n * base + luai_digit;
    s++;
  } while (isalnum((unsigned char)*s));
  s += strspn(s, LUA_SPACECHARS);  /* skip trailing spaces */
  *pn = (lua_Integer)((neg) ? (0u - n) : n);
  return s;
}


static int luaB_tonumber (lua_State *L) {
  if (lua_isnoneornil(L, 2)) {  /* standard conversion? */
    luaL_checkany(L, 1);
    if (lua_type(L, 1) == LUA_TNUMBER) {  /* already a number? */
      lua_settop(L, 1);  /* yes; return it */
      return 1;
    }
    else {
      size_t l;
      const char *s = lua_tolstring(L, 1, &l);
      if (s != NULL && lua_stringtonumber(L, s) == l + 1)
        return 1;  /* successful conversion to number */
      /* else not a number */
    }
  }
  else {
    size_t l;
    const char *s;
    lua_Integer n = 0;  /* to avoid warnings */
    lua_Integer base = luaL_checkinteger(L, 2);
    luaL_checktype(L, 1, LUA_TSTRING);  /* no numbers as strings */
    s = lua_tolstring(L, 1, &l);
    luaL_argcheck(L, 2 <= base && base <= 36, 2, "base out of range");
    if (luai_b_str2int(s, (int)base, &n) == s + l) {
      lua_pushinteger(L, n);
      return 1;
    }  /* else not a number */
  }  /* else not a number */
  lua_pushnil(L);  /* not a number */
  return 1;
}


static int luaB_error (lua_State *L) {
  int level = (int)luaL_optinteger(L, 2, 1);
  lua_settop(L, 1);
  if (lua_type(L, 1) == LUA_TSTRING && level > 0) {
    luaL_where(L, level);   /* add extra information */
    lua_pushvalue(L, 1);
    lua_concat(L, 2);
  }
  return lua_error(L);
}


static int luaB_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  if (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);
    return 1;  /* no metatable */
  }
  luaL_getmetafield(L, 1, "__metatable");
  return 1;  /* returns either __metatable luai_field (if present) or metatable */
}


static int luaB_setmetatable (lua_State *L) {
  int t = lua_type(L, 2);
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_argcheck(L, t == LUA_TNIL || t == LUA_TTABLE, 2,
                    "nil or table expected");
  if (luaL_getmetafield(L, 1, "__metatable") != LUA_TNIL)
    return luaL_error(L, "cannot change a protected metatable");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  return 1;
}


static int luaB_rawequal (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_checkany(L, 2);
  lua_pushboolean(L, lua_rawequal(L, 1, 2));
  return 1;
}


static int luaB_rawlen (lua_State *L) {
  int t = lua_type(L, 1);
  luaL_argcheck(L, t == LUA_TTABLE || t == LUA_TSTRING, 1,
                   "table or string expected");
  lua_pushinteger(L, lua_rawlen(L, 1));
  return 1;
}


static int luaB_rawget (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  lua_rawget(L, 1);
  return 1;
}

static int luaB_rawset (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  luaL_checkany(L, 3);
  lua_settop(L, 3);
  lua_rawset(L, 1);
  return 1;
}


static int luaB_collectgarbage (lua_State *L) {
  static const char *const opts[] = {"stop", "restart", "collect",
    "count", "step", "setpause", "setstepmul",
    "isrunning", NULL};
  static const int optsnum[] = {LUA_GCSTOP, LUA_GCRESTART, LUA_GCCOLLECT,
    LUA_GCCOUNT, LUA_GCSTEP, LUA_GCSETPAUSE, LUA_GCSETSTEPMUL,
    LUA_GCISRUNNING};
  int o = optsnum[luaL_checkoption(L, 1, "collect", opts)];
  int ex = (int)luaL_optinteger(L, 2, 0);
  int res = lua_gc(L, o, ex);
  switch (o) {
    case LUA_GCCOUNT: {
      int b = lua_gc(L, LUA_GCCOUNTB, 0);
      lua_pushnumber(L, (lua_Number)res + ((lua_Number)b/1024));
      return 1;
    }
    case LUA_GCSTEP: case LUA_GCISRUNNING: {
      lua_pushboolean(L, res);
      return 1;
    }
    default: {
      lua_pushinteger(L, res);
      return 1;
    }
  }
}


static int luaB_type (lua_State *L) {
  int t = lua_type(L, 1);
  luaL_argcheck(L, t != LUA_TNONE, 1, "value expected");
  lua_pushstring(L, lua_typename(L, t));
  return 1;
}


static int luai_pairsmeta (lua_State *L, const char *method, int iszero,
                      lua_CFunction iter) {
  luaL_checkany(L, 1);
  if (luaL_getmetafield(L, 1, method) == LUA_TNIL) {  /* no metamethod? */
    lua_pushcfunction(L, iter);  /* will return generator, */
    lua_pushvalue(L, 1);  /* state, */
    if (iszero) lua_pushinteger(L, 0);  /* and initial value */
    else lua_pushnil(L);
  }
  else {
    lua_pushvalue(L, 1);  /* argument 'self' to metamethod */
    lua_call(L, 1, 3);  /* get 3 values from metamethod */
  }
  return 3;
}


static int luaB_next (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_settop(L, 2);  /* create a 2nd argument if there isn't one */
  if (lua_next(L, 1))
    return 2;
  else {
    lua_pushnil(L);
    return 1;
  }
}


static int luaB_pairs (lua_State *L) {
  return luai_pairsmeta(L, "__pairs", 0, luaB_next);
}


/*
** Traversal function for 'ipairs'
*/
static int luai_ipairsaux (lua_State *L) {
  lua_Integer i = luaL_checkinteger(L, 2) + 1;
  lua_pushinteger(L, i);
  return (lua_geti(L, 1, i) == LUA_TNIL) ? 1 : 2;
}


/*
** 'ipairs' function. Returns 'luai_ipairsaux', given "table", 0.
** (The given "table" may not be a table.)
*/
static int luaB_ipairs (lua_State *L) {
#if defined(LUA_COMPAT_IPAIRS)
  return luai_pairsmeta(L, "__ipairs", 1, luai_ipairsaux);
#else
  luaL_checkany(L, 1);
  lua_pushcfunction(L, luai_ipairsaux);  /* iteration function */
  lua_pushvalue(L, 1);  /* state */
  lua_pushinteger(L, 0);  /* initial value */
  return 3;
#endif
}


static int luai_load_aux (lua_State *L, int status, int envidx) {
  if (status == LUA_OK) {
    if (envidx != 0) {  /* 'env' parameter? */
      lua_pushvalue(L, envidx);  /* environment for loaded function */
      if (!lua_setupvalue(L, -2, 1))  /* set it as 1st upvalue */
        lua_pop(L, 1);  /* remove 'env' if not used by previous call */
    }
    return 1;
  }
  else {  /* error (message is on top of the stack) */
    lua_pushnil(L);
    lua_insert(L, -2);  /* put before error message */
    return 2;  /* return nil plus error message */
  }
}


static int luaB_loadfile (lua_State *L) {
  const char *fname = luaL_optstring(L, 1, NULL);
  const char *mode = luaL_optstring(L, 2, NULL);
  int env = (!lua_isnone(L, 3) ? 3 : 0);  /* 'env' index or 0 if no 'env' */
  int status = luaL_loadfilex(L, fname, mode);
  return luai_load_aux(L, status, env);
}


/*
** {======================================================
** Generic Read function
** =======================================================
*/


/*
** reserved slot, above all arguments, to hold a copy of the returned
** string to avoid it being collected while parsed. 'load' has four
** optional arguments (chunk, source name, mode, and environment).
*/
#define LUA_RESERVEDSLOT	5


/*
** Reader for generic 'load' function: 'lua_load' uses the
** stack for internal stuff, so the reader cannot change the
** stack top. Instead, it keeps its resulting string in a
** reserved slot inside the stack.
*/
static const char *luai_generic_reader (lua_State *L, void *ud, size_t *size) {
  (void)(ud);  /* not used */
  luaL_checkstack(L, 2, "too many nested functions");
  lua_pushvalue(L, 1);  /* get function */
  lua_call(L, 0, 1);  /* call it */
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);  /* pop result */
    *size = 0;
    return NULL;
  }
  else if (!lua_isstring(L, -1))
    luaL_error(L, "reader function must return a string");
  lua_replace(L, LUA_RESERVEDSLOT);  /* luai_save string in reserved slot */
  return lua_tolstring(L, LUA_RESERVEDSLOT, size);
}


static int luaB_load (lua_State *L) {
  int status;
  size_t l;
  const char *s = lua_tolstring(L, 1, &l);
  const char *mode = luaL_optstring(L, 3, "bt");
  int env = (!lua_isnone(L, 4) ? 4 : 0);  /* 'env' index or 0 if no 'env' */
  if (s != NULL) {  /* loading a string? */
    const char *chunkname = luaL_optstring(L, 2, s);
    status = luaL_loadbufferx(L, s, l, chunkname, mode);
  }
  else {  /* loading from a reader function */
    const char *chunkname = luaL_optstring(L, 2, "=(load)");
    luaL_checktype(L, 1, LUA_TFUNCTION);
    lua_settop(L, LUA_RESERVEDSLOT);  /* create reserved slot */
    status = lua_load(L, luai_generic_reader, NULL, chunkname, mode);
  }
  return luai_load_aux(L, status, env);
}

/* }====================================================== */


static int luai_dofilecont (lua_State *L, int d1, lua_KContext d2) {
  (void)d1;  (void)d2;  /* only to luai_match 'lua_Kfunction' prototype */
  return lua_gettop(L) - 1;
}


static int luaB_dofile (lua_State *L) {
  const char *fname = luaL_optstring(L, 1, NULL);
  lua_settop(L, 1);
  if (luaL_loadfile(L, fname) != LUA_OK)
    return lua_error(L);
  lua_callk(L, 0, LUA_MULTRET, 0, luai_dofilecont);
  return luai_dofilecont(L, 0, 0);
}


static int luaB_assert (lua_State *L) {
  if (lua_toboolean(L, 1))  /* condition is true? */
    return lua_gettop(L);  /* return all arguments */
  else {  /* error */
    luaL_checkany(L, 1);  /* there must be a condition */
    lua_remove(L, 1);  /* remove it */
    lua_pushliteral(L, "assertion failed!");  /* default message */
    lua_settop(L, 1);  /* leave only message (default if no other one) */
    return luaB_error(L);  /* call 'error' */
  }
}


static int luaB_select (lua_State *L) {
  int n = lua_gettop(L);
  if (lua_type(L, 1) == LUA_TSTRING && *lua_tostring(L, 1) == '#') {
    lua_pushinteger(L, n-1);
    return 1;
  }
  else {
    lua_Integer i = luaL_checkinteger(L, 1);
    if (i < 0) i = n + i;
    else if (i > n) i = n;
    luaL_argcheck(L, 1 <= i, 1, "index out of range");
    return n - (int)i;
  }
}


/*
** Continuation function for 'pcall' and 'xpcall'. Both functions
** already pushed a 'true' before doing the call, so in case of success
** 'luai_finishpcall' only has to return everything in the stack minus
** 'extra' values (where 'extra' is exactly the number of items to be
** ignored).
*/
static int luai_finishpcall (lua_State *L, int status, lua_KContext extra) {
  if (status != LUA_OK && status != LUA_YIELD) {  /* error? */
    lua_pushboolean(L, 0);  /* first result (false) */
    lua_pushvalue(L, -2);  /* error message */
    return 2;  /* return false, msg */
  }
  else
    return lua_gettop(L) - (int)extra;  /* return all results */
}


static int luaB_pcall (lua_State *L) {
  int status;
  luaL_checkany(L, 1);
  lua_pushboolean(L, 1);  /* first result if no errors */
  lua_insert(L, 1);  /* put it in place */
  status = lua_pcallk(L, lua_gettop(L) - 2, LUA_MULTRET, 0, 0, luai_finishpcall);
  return luai_finishpcall(L, status, 0);
}


/*
** Do a protected call with error handling. After 'lua_rotate', the
** stack will have <f, err, true, f, [args...]>; so, the function passes
** 2 to 'luai_finishpcall' to skip the 2 first values when returning results.
*/
static int luaB_xpcall (lua_State *L) {
  int status;
  int n = lua_gettop(L);
  luaL_checktype(L, 2, LUA_TFUNCTION);  /* luai_check error function */
  lua_pushboolean(L, 1);  /* first result */
  lua_pushvalue(L, 1);  /* function */
  lua_rotate(L, 3, 2);  /* move them below function's arguments */
  status = lua_pcallk(L, n - 2, LUA_MULTRET, 2, 2, luai_finishpcall);
  return luai_finishpcall(L, status, 2);
}


static int luaB_tostring (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_tolstring(L, 1, NULL);
  return 1;
}


static const luaL_Reg luai_base_funcs[] = {
  {"assert", luaB_assert},
  {"collectgarbage", luaB_collectgarbage},
  {"dofile", luaB_dofile},
  {"error", luaB_error},
  {"getmetatable", luaB_getmetatable},
  {"ipairs", luaB_ipairs},
  {"loadfile", luaB_loadfile},
  {"load", luaB_load},
#if defined(LUA_COMPAT_LOADSTRING)
  {"loadstring", luaB_load},
#endif
  {"next", luaB_next},
  {"pairs", luaB_pairs},
  {"pcall", luaB_pcall},
  {"print", luaB_print},
  {"rawequal", luaB_rawequal},
  {"rawlen", luaB_rawlen},
  {"rawget", luaB_rawget},
  {"rawset", luaB_rawset},
  {"select", luaB_select},
  {"setmetatable", luaB_setmetatable},
  {"tonumber", luaB_tonumber},
  {"tostring", luaB_tostring},
  {"type", luaB_type},
  {"xpcall", luaB_xpcall},
  /* placeholders */
  {"_G", NULL},
  {"_VERSION", NULL},
  {NULL, NULL}
};


LUAMOD_API int luaopen_base (lua_State *L) {
  /* open lib into global table */
  lua_pushglobaltable(L);
  luaL_setfuncs(L, luai_base_funcs, 0);
  /* set global _G */
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "_G");
  /* set global _VERSION */
  lua_pushliteral(L, LUA_VERSION);
  lua_setfield(L, -2, "_VERSION");
  return 1;
}

/*__lbitlib.c__*/

#if defined(LUA_COMPAT_BITLIB)		/* { */


#define luai_pushunsigned(L,n)	lua_pushinteger(L, (lua_Integer)(n))
#define luai_checkunsigned(L,i)	((lua_Unsigned)luaL_checkinteger(L,i))


/* number of bits to consider in a number */
#if !defined(LUA_NBITS)
#define LUA_NBITS	32
#endif


/*
** a lua_Unsigned with its first LUA_NBITS bits equal to 1. (Shift must
** be made in two parts to avoid problems when LUA_NBITS is equal to the
** number of bits in a lua_Unsigned.)
*/
#define LUA_ALLONES		(~(((~(lua_Unsigned)0) << (LUA_NBITS - 1)) << 1))


/* macro to trim extra bits */
#define lua_trim(x)		((x) & LUA_ALLONES)


/* builds a number with 'n' ones (1 <= n <= LUA_NBITS) */
#define luai_mask(n)		(~((LUA_ALLONES << 1) << ((n) - 1)))



static lua_Unsigned lua_andaux (lua_State *L) {
  int i, n = lua_gettop(L);
  lua_Unsigned r = ~(lua_Unsigned)0;
  for (i = 1; i <= n; i++)
    r &= luai_checkunsigned(L, i);
  return lua_trim(r);
}


static int lua_b_and (lua_State *L) {
  lua_Unsigned r = lua_andaux(L);
  luai_pushunsigned(L, r);
  return 1;
}


static int lua_b_test (lua_State *L) {
  lua_Unsigned r = lua_andaux(L);
  lua_pushboolean(L, r != 0);
  return 1;
}


static int lua_b_or (lua_State *L) {
  int i, n = lua_gettop(L);
  lua_Unsigned r = 0;
  for (i = 1; i <= n; i++)
    r |= luai_checkunsigned(L, i);
  luai_pushunsigned(L, lua_trim(r));
  return 1;
}


static int lua_b_xor (lua_State *L) {
  int i, n = lua_gettop(L);
  lua_Unsigned r = 0;
  for (i = 1; i <= n; i++)
    r ^= luai_checkunsigned(L, i);
  luai_pushunsigned(L, lua_trim(r));
  return 1;
}


static int lua_b_not (lua_State *L) {
  lua_Unsigned r = ~luai_checkunsigned(L, 1);
  luai_pushunsigned(L, lua_trim(r));
  return 1;
}


static int lua_b_shift (lua_State *L, lua_Unsigned r, lua_Integer i) {
  if (i < 0) {  /* shift right? */
    i = -i;
    r = lua_trim(r);
    if (i >= LUA_NBITS) r = 0;
    else r >>= i;
  }
  else {  /* shift left */
    if (i >= LUA_NBITS) r = 0;
    else r <<= i;
    r = lua_trim(r);
  }
  luai_pushunsigned(L, r);
  return 1;
}


static int lua_b_lshift (lua_State *L) {
  return lua_b_shift(L, luai_checkunsigned(L, 1), luaL_checkinteger(L, 2));
}


static int lua_b_rshift (lua_State *L) {
  return lua_b_shift(L, luai_checkunsigned(L, 1), -luaL_checkinteger(L, 2));
}


static int lua_b_arshift (lua_State *L) {
  lua_Unsigned r = luai_checkunsigned(L, 1);
  lua_Integer i = luaL_checkinteger(L, 2);
  if (i < 0 || !(r & ((lua_Unsigned)1 << (LUA_NBITS - 1))))
    return lua_b_shift(L, r, -i);
  else {  /* arithmetic shift for 'negative' number */
    if (i >= LUA_NBITS) r = LUA_ALLONES;
    else
      r = lua_trim((r >> i) | ~(lua_trim(~(lua_Unsigned)0) >> i));  /* add signal bit */
    luai_pushunsigned(L, r);
    return 1;
  }
}


static int lua_b_rot (lua_State *L, lua_Integer d) {
  lua_Unsigned r = luai_checkunsigned(L, 1);
  int i = d & (LUA_NBITS - 1);  /* i = d % NBITS */
  r = lua_trim(r);
  if (i != 0)  /* avoid undefined shift of LUA_NBITS when i == 0 */
    r = (r << i) | (r >> (LUA_NBITS - i));
  luai_pushunsigned(L, lua_trim(r));
  return 1;
}


static int lua_b_lrot (lua_State *L) {
  return lua_b_rot(L, luaL_checkinteger(L, 2));
}


static int lua_b_rrot (lua_State *L) {
  return lua_b_rot(L, -luaL_checkinteger(L, 2));
}


/*
** get luai_field and width arguments for luai_field-manipulation functions,
** checking whether they are valid.
** ('luaL_error' called without 'return' to avoid later warnings about
** 'width' being used uninitialized.)
*/
static int lua_fieldargs (lua_State *L, int farg, int *width) {
  lua_Integer f = luaL_checkinteger(L, farg);
  lua_Integer w = luaL_optinteger(L, farg + 1, 1);
  luaL_argcheck(L, 0 <= f, farg, "field cannot be negative");
  luaL_argcheck(L, 0 < w, farg + 1, "width must be positive");
  if (f + w > LUA_NBITS)
    luaL_error(L, "trying to access non-existent bits");
  *width = (int)w;
  return (int)f;
}


static int lua_b_extract (lua_State *L) {
  int w;
  lua_Unsigned r = lua_trim(luai_checkunsigned(L, 1));
  int f = lua_fieldargs(L, 2, &w);
  r = (r >> f) & luai_mask(w);
  luai_pushunsigned(L, r);
  return 1;
}


static int lua_b_replace (lua_State *L) {
  int w;
  lua_Unsigned r = lua_trim(luai_checkunsigned(L, 1));
  lua_Unsigned v = lua_trim(luai_checkunsigned(L, 2));
  int f = lua_fieldargs(L, 3, &w);
  lua_Unsigned m = luai_mask(w);
  r = (r & ~(m << f)) | ((v & m) << f);
  luai_pushunsigned(L, r);
  return 1;
}


static const luaL_Reg luai_bitlib[] = {
  {"arshift", lua_b_arshift},
  {"band", lua_b_and},
  {"bnot", lua_b_not},
  {"bor", lua_b_or},
  {"bxor", lua_b_xor},
  {"btest", lua_b_test},
  {"extract", lua_b_extract},
  {"lrotate", lua_b_lrot},
  {"lshift", lua_b_lshift},
  {"replace", lua_b_replace},
  {"rrotate", lua_b_rrot},
  {"rshift", lua_b_rshift},
  {NULL, NULL}
};



LUAMOD_API int luaopen_bit32 (lua_State *L) {
  luaL_newlib(L, luai_bitlib);
  return 1;
}


#else					/* }{ */


LUAMOD_API int luaopen_bit32 (lua_State *L) {
  return luaL_error(L, "library 'bit32' has been deprecated");
}

#endif			        /* } */


/*__lcode.c__*/

/* Maximum number of registers in a Lua function (must fit in 8 bits) */
#define LUA_MAXREGS		255


#define lua_hasjumps(e)	((e)->t != (e)->f)


/*
** If expression is a numeric constant, fills 'v' with its value
** and returns 1. Otherwise, returns 0.
*/
static int lua_tonumeral(const luai_expdesc *e, luai_TValue *v) {
  if (lua_hasjumps(e))
    return 0;  /* not a numeral */
  switch (e->k) {
    case LUAI_VKINT:
      if (v) luai_setivalue(v, e->u.ival);
      return 1;
    case LUAI_VKFLT:
      if (v) luai_setfltvalue(v, e->u.nval);
      return 1;
    default: return 0;
  }
}


/*
** Create a luai_OP_LOADNIL instruction, but try to optimize: if the previous
** instruction is also luai_OP_LOADNIL and ranges are compatible, adjust
** range of previous instruction instead of emitting a new one. (For
** instance, 'local a; local b' will generate a single opcode.)
*/
void luaK_nil (luai_FuncState *fs, int from, int n) {
  Instruction *previous;
  int l = from + n - 1;  /* last register to set nil */
  if (fs->pc > fs->lasttarget) {  /* no jumps to current position? */
    previous = &fs->f->code[fs->pc-1];
    if (luai_GETOPCODE(*previous) == luai_OP_LOADNIL) {  /* previous is LOADNIL? */
      int pfrom = luai_GETARG_A(*previous);  /* get previous range */
      int pl = pfrom + luai_GETARG_B(*previous);
      if ((pfrom <= from && from <= pl + 1) ||
          (from <= pfrom && pfrom <= l + 1)) {  /* can connect both? */
        if (pfrom < from) from = pfrom;  /* from = min(from, pfrom) */
        if (pl > l) l = pl;  /* l = max(l, pl) */
        luai_SETARG_A(*previous, from);
        luai_SETARG_B(*previous, l - from);
        return;
      }
    }  /* else go through */
  }
  luaK_codeABC(fs, luai_OP_LOADNIL, from, n - 1, 0);  /* else no optimization */
}


/*
** Gets the destination address of a jump instruction. Used to traverse
** a list of jumps.
*/
static int luai_getjump (luai_FuncState *fs, int pc) {
  int offset = luai_GETARG_sBx(fs->f->code[pc]);
  if (offset == LUAI_NO_JUMP)  /* point to itself represents end of list */
    return LUAI_NO_JUMP;  /* end of list */
  else
    return (pc+1)+offset;  /* turn offset into absolute position */
}


/*
** Fix jump instruction at position 'pc' to jump to 'dest'.
** (Jump addresses are relative in Lua)
*/
static void luai_fixjump (luai_FuncState *fs, int pc, int dest) {
  Instruction *jmp = &fs->f->code[pc];
  int offset = dest - (pc + 1);
  lua_assert(dest != LUAI_NO_JUMP);
  if (abs(offset) > luai_MAXARG_sBx)
    luaX_syntaxerror(fs->ls, "control structure too long");
  luai_SETARG_sBx(*jmp, offset);
}


/*
** Concatenate jump-list 'l2' into jump-list 'l1'
*/
void luaK_concat (luai_FuncState *fs, int *l1, int l2) {
  if (l2 == LUAI_NO_JUMP) return;  /* nothing to concatenate? */
  else if (*l1 == LUAI_NO_JUMP)  /* no original list? */
    *l1 = l2;  /* 'l1' points to 'l2' */
  else {
    int list = *l1;
    int luai_next;
    while ((luai_next = luai_getjump(fs, list)) != LUAI_NO_JUMP)  /* find last element */
      list = luai_next;
    luai_fixjump(fs, list, l2);  /* last element links to 'l2' */
  }
}


/*
** Create a jump instruction and return its position, so its destination
** can be fixed later (with 'luai_fixjump'). If there are jumps to
** this position (kept in 'jpc'), link them all together so that
** 'luai_patchlistaux' will fix all them directly to the final destination.
*/
int luaK_jump (luai_FuncState *fs) {
  int jpc = fs->jpc;  /* luai_save list of jumps to here */
  int j;
  fs->jpc = LUAI_NO_JUMP;  /* no more jumps to here */
  j = luaK_codeAsBx(fs, luai_OP_JMP, 0, LUAI_NO_JUMP);
  luaK_concat(fs, &j, jpc);  /* keep them on hold */
  return j;
}


/*
** Code a 'return' instruction
*/
void luaK_ret (luai_FuncState *fs, int first, int nret) {
  luaK_codeABC(fs, luai_OP_RETURN, first, nret+1, 0);
}


/*
** Code a "conditional jump", that is, a test or comparison opcode
** followed by a jump. Return jump position.
*/
static int luai_condjump (luai_FuncState *fs, luai_OpCode op, int A, int B, int C) {
  luaK_codeABC(fs, op, A, B, C);
  return luaK_jump(fs);
}


/*
** returns current 'pc' and marks it as a jump target (to avoid wrong
** optimizations with consecutive instructions not in the same basic luai_getblock).
*/
int luaK_getlabel (luai_FuncState *fs) {
  fs->lasttarget = fs->pc;
  return fs->pc;
}


/*
** Returns the position of the instruction "controlling" a given
** jump (that is, its condition), or the jump itself if it is
** unconditional.
*/
static Instruction *luai_getjumpcontrol (luai_FuncState *fs, int pc) {
  Instruction *pi = &fs->f->code[pc];
  if (pc >= 1 && luai_testTMode(luai_GETOPCODE(*(pi-1))))
    return pi-1;
  else
    return pi;
}


/*
** Patch destination register for a TESTSET instruction.
** If instruction in position 'node' is not a TESTSET, return 0 ("fails").
** Otherwise, if 'reg' is not 'luai_NO_REG', set it as the destination
** register. Otherwise, change instruction to a simple 'TEST' (produces
** no register value)
*/
static int luai_patchtestreg (luai_FuncState *fs, int node, int reg) {
  Instruction *i = luai_getjumpcontrol(fs, node);
  if (luai_GETOPCODE(*i) != luai_OP_TESTSET)
    return 0;  /* cannot patch other instructions */
  if (reg != luai_NO_REG && reg != luai_GETARG_B(*i))
    luai_SETARG_A(*i, reg);
  else {
     /* no register to put value or register already has the value;
        change instruction to simple test */
    *i = luai_CREATE_ABC(luai_OP_TEST, luai_GETARG_B(*i), 0, luai_GETARG_C(*i));
  }
  return 1;
}


/*
** Traverse a list of tests ensuring no one produces a value
*/
static void luai_removevalues (luai_FuncState *fs, int list) {
  for (; list != LUAI_NO_JUMP; list = luai_getjump(fs, list))
      luai_patchtestreg(fs, list, luai_NO_REG);
}


/*
** Traverse a list of tests, patching their destination address and
** registers: tests producing values jump to 'vtarget' (and put their
** values in 'reg'), other tests jump to 'dtarget'.
*/
static void luai_patchlistaux (luai_FuncState *fs, int list, int vtarget, int reg,
                          int dtarget) {
  while (list != LUAI_NO_JUMP) {
    int luai_next = luai_getjump(fs, list);
    if (luai_patchtestreg(fs, list, reg))
      luai_fixjump(fs, list, vtarget);
    else
      luai_fixjump(fs, list, dtarget);  /* jump to default target */
    list = luai_next;
  }
}


/*
** Ensure all pending jumps to current position are fixed (jumping
** to current position with no values) and reset list of pending
** jumps
*/
static void luai_dischargejpc (luai_FuncState *fs) {
  luai_patchlistaux(fs, fs->jpc, fs->pc, luai_NO_REG, fs->pc);
  fs->jpc = LUAI_NO_JUMP;
}


/*
** Add elements in 'list' to list of pending jumps to "here"
** (current position)
*/
void luaK_patchtohere (luai_FuncState *fs, int list) {
  luaK_getlabel(fs);  /* mark "here" as a jump target */
  luaK_concat(fs, &fs->jpc, list);
}


/*
** Path all jumps in 'list' to jump to 'target'.
** (The assert means that we cannot fix a jump to a forward address
** because we only know addresses once code is generated.)
*/
void luaK_patchlist (luai_FuncState *fs, int list, int target) {
  if (target == fs->pc)  /* 'target' is current position? */
    luaK_patchtohere(fs, list);  /* add list to pending jumps */
  else {
    lua_assert(target < fs->pc);
    luai_patchlistaux(fs, list, target, luai_NO_REG, target);
  }
}


/*
** Path all jumps in 'list' to close upvalues up to given 'level'
** (The assertion checks that jumps either were closing nothing
** or were closing higher levels, from inner blocks.)
*/
void luaK_patchclose (luai_FuncState *fs, int list, int level) {
  level++;  /* argument is +1 to reserve 0 as non-op */
  for (; list != LUAI_NO_JUMP; list = luai_getjump(fs, list)) {
    lua_assert(luai_GETOPCODE(fs->f->code[list]) == luai_OP_JMP &&
                (luai_GETARG_A(fs->f->code[list]) == 0 ||
                 luai_GETARG_A(fs->f->code[list]) >= level));
    luai_SETARG_A(fs->f->code[list], level);
  }
}


/*
** Emit instruction 'i', checking for array sizes and saving also its
** line information. Return 'i' position.
*/
static int luaK_code (luai_FuncState *fs, Instruction i) {
  luai_Proto *f = fs->f;
  luai_dischargejpc(fs);  /* 'pc' will change */
  /* put new instruction in code array */
  luaM_growvector(fs->ls->L, f->code, fs->pc, f->sizecode, Instruction,
                  LUAI_MAX_INT, "opcodes");
  f->code[fs->pc] = i;
  /* luai_save corresponding line information */
  luaM_growvector(fs->ls->L, f->lineinfo, fs->pc, f->sizelineinfo, int,
                  LUAI_MAX_INT, "opcodes");
  f->lineinfo[fs->pc] = fs->ls->lastline;
  return fs->pc++;
}


/*
** Format and emit an 'luai_iABC' instruction. (Assertions luai_check consistency
** of parameters versus opcode.)
*/
int luaK_codeABC (luai_FuncState *fs, luai_OpCode o, int a, int b, int c) {
  lua_assert(luai_getOpMode(o) == luai_iABC);
  lua_assert(luai_getBMode(o) != luai_OpArgN || b == 0);
  lua_assert(luai_getCMode(o) != luai_OpArgN || c == 0);
  lua_assert(a <= luai_MAXARG_A && b <= luai_MAXARG_B && c <= luai_MAXARG_C);
  return luaK_code(fs, luai_CREATE_ABC(o, a, b, c));
}


/*
** Format and emit an 'luai_iABx' instruction.
*/
int luaK_codeABx (luai_FuncState *fs, luai_OpCode o, int a, unsigned int bc) {
  lua_assert(luai_getOpMode(o) == luai_iABx || luai_getOpMode(o) == luai_iAsBx);
  lua_assert(luai_getCMode(o) == luai_OpArgN);
  lua_assert(a <= luai_MAXARG_A && bc <= luai_MAXARG_Bx);
  return luaK_code(fs, luai_CREATE_ABx(o, a, bc));
}


/*
** Emit an "extra argument" instruction (format 'luai_iAx')
*/
static int luai_codeextraarg (luai_FuncState *fs, int a) {
  lua_assert(a <= luai_MAXARG_Ax);
  return luaK_code(fs, luai_CREATE_Ax(luai_OP_EXTRAARG, a));
}


/*
** Emit a "load constant" instruction, using either 'luai_OP_LOADK'
** (if constant index 'k' fits in 18 bits) or an 'luai_OP_LOADKX'
** instruction with "extra argument".
*/
int luaK_codek (luai_FuncState *fs, int reg, int k) {
  if (k <= luai_MAXARG_Bx)
    return luaK_codeABx(fs, luai_OP_LOADK, reg, k);
  else {
    int p = luaK_codeABx(fs, luai_OP_LOADKX, reg, 0);
    luai_codeextraarg(fs, k);
    return p;
  }
}


/*
** Check register-stack level, keeping track of its maximum size
** in luai_field 'maxstacksize'
*/
void luaK_checkstack (luai_FuncState *fs, int n) {
  int newstack = fs->luai_freereg + n;
  if (newstack > fs->f->maxstacksize) {
    if (newstack >= LUA_MAXREGS)
      luaX_syntaxerror(fs->ls,
        "function or expression needs too many registers");
    fs->f->maxstacksize = luai_cast_byte(newstack);
  }
}


/*
** Reserve 'n' registers in register stack
*/
void luaK_reserveregs (luai_FuncState *fs, int n) {
  luaK_checkstack(fs, n);
  fs->luai_freereg += n;
}


/*
** Free register 'reg', if it is neither a constant index nor
** a local variable.
)
*/
static void luai_freereg (luai_FuncState *fs, int reg) {
  if (!luai_ISK(reg) && reg >= fs->nactvar) {
    fs->luai_freereg--;
    lua_assert(reg == fs->luai_freereg);
  }
}


/*
** Free register used by expression 'e' (if any)
*/
static void luai_freeexp (luai_FuncState *fs, luai_expdesc *e) {
  if (e->k == LUAI_VNONRELOC)
    luai_freereg(fs, e->u.info);
}


/*
** Free registers used by expressions 'e1' and 'e2' (if any) in proper
** order.
*/
static void luai_freeexps (luai_FuncState *fs, luai_expdesc *e1, luai_expdesc *e2) {
  int r1 = (e1->k == LUAI_VNONRELOC) ? e1->u.info : -1;
  int r2 = (e2->k == LUAI_VNONRELOC) ? e2->u.info : -1;
  if (r1 > r2) {
    luai_freereg(fs, r1);
    luai_freereg(fs, r2);
  }
  else {
    luai_freereg(fs, r2);
    luai_freereg(fs, r1);
  }
}


/*
** Add constant 'v' to prototype's list of constants (luai_field 'k').
** Use scanner's table to cache position of constants in constant list
** and try to reuse constants. Because some values should not be used
** as keys (nil cannot be a key, integer keys can collapse with float
** keys), the caller must provide a useful 'key' for indexing the cache.
*/
static int luai_addk (luai_FuncState *fs, luai_TValue *key, luai_TValue *v) {
  lua_State *L = fs->ls->L;
  luai_Proto *f = fs->f;
  luai_TValue *idx = luaH_set(L, fs->ls->h, key);  /* index scanner table */
  int k, oldsize;
  if (luai_ttisinteger(idx)) {  /* is there an index there? */
    k = luai_cast_int(luai_ivalue(idx));
    /* correct value? (warning: must distinguish floats from integers!) */
    if (k < fs->nk && luai_ttype(&f->k[k]) == luai_ttype(v) &&
                      luaV_rawequalobj(&f->k[k], v))
      return k;  /* reuse index */
  }
  /* constant not found; create a new entry */
  oldsize = f->sizek;
  k = fs->nk;
  /* numerical value does not need LUAI_GC barrier;
     table has no metatable, so it does not need to invalidate cache */
  luai_setivalue(idx, k);
  luaM_growvector(L, f->k, k, f->sizek, luai_TValue, luai_MAXARG_Ax, "constants");
  while (oldsize < f->sizek) luai_setnilvalue(&f->k[oldsize++]);
  luai_setobj(L, &f->k[k], v);
  fs->nk++;
  luaC_barrier(L, f, v);
  return k;
}


/*
** Add a string to list of constants and return its index.
*/
int luaK_stringK (luai_FuncState *fs, luai_TString *s) {
  luai_TValue o;
  luai_setsvalue(fs->ls->L, &o, s);
  return luai_addk(fs, &o, &o);  /* use string itself as key */
}


/*
** Add an integer to list of constants and return its index.
** Integers use userdata as keys to avoid collision with floats with
** same value; conversion to 'void*' is used only for hashing, so there
** are no "precision" problems.
*/
int luaK_intK (luai_FuncState *fs, lua_Integer n) {
  luai_TValue k, o;
  luai_setpvalue(&k, luai_cast(void*, luai_cast(size_t, n)));
  luai_setivalue(&o, n);
  return luai_addk(fs, &k, &o);
}

/*
** Add a float to list of constants and return its index.
*/
static int luaK_numberK (luai_FuncState *fs, lua_Number r) {
  luai_TValue o;
  luai_setfltvalue(&o, r);
  return luai_addk(fs, &o, &o);  /* use number itself as key */
}


/*
** Add a boolean to list of constants and return its index.
*/
static int luai_boolK (luai_FuncState *fs, int b) {
  luai_TValue o;
  luai_setbvalue(&o, b);
  return luai_addk(fs, &o, &o);  /* use boolean itself as key */
}


/*
** Add nil to list of constants and return its index.
*/
static int luai_nilK (luai_FuncState *fs) {
  luai_TValue k, v;
  luai_setnilvalue(&v);
  /* cannot use nil as key; instead use table itself to represent nil */
  luai_sethvalue(fs->ls->L, &k, fs->ls->h);
  return luai_addk(fs, &k, &v);
}


/*
** Fix an expression to return the number of results 'nresults'.
** Either 'e' is a multi-ret expression (function call or vararg)
** or 'nresults' is LUA_MULTRET (as any expression can satisfy that).
*/
void luaK_setreturns (luai_FuncState *fs, luai_expdesc *e, int nresults) {
  if (e->k == LUAI_VCALL) {  /* expression is an open function call? */
    luai_SETARG_C(luai_getinstruction(fs, e), nresults + 1);
  }
  else if (e->k == LUAI_VVARARG) {
    Instruction *pc = &luai_getinstruction(fs, e);
    luai_SETARG_B(*pc, nresults + 1);
    luai_SETARG_A(*pc, fs->luai_freereg);
    luaK_reserveregs(fs, 1);
  }
  else lua_assert(nresults == LUA_MULTRET);
}


/*
** Fix an expression to return one result.
** If expression is not a multi-ret expression (function call or
** vararg), it already returns one result, so nothing needs to be done.
** Function calls become LUAI_VNONRELOC expressions (as its result comes
** fixed in the base register of the call), while vararg expressions
** become LUAI_VRELOCABLE (as luai_OP_VARARG puts its results where it wants).
** (Calls are created returning one result, so that does not need
** to be fixed.)
*/
void luaK_setoneret (luai_FuncState *fs, luai_expdesc *e) {
  if (e->k == LUAI_VCALL) {  /* expression is an open function call? */
    /* already returns 1 value */
    lua_assert(luai_GETARG_C(luai_getinstruction(fs, e)) == 2);
    e->k = LUAI_VNONRELOC;  /* result has fixed position */
    e->u.info = luai_GETARG_A(luai_getinstruction(fs, e));
  }
  else if (e->k == LUAI_VVARARG) {
    luai_SETARG_B(luai_getinstruction(fs, e), 2);
    e->k = LUAI_VRELOCABLE;  /* can relocate its simple result */
  }
}


/*
** Ensure that expression 'e' is not a variable.
*/
void luaK_dischargevars (luai_FuncState *fs, luai_expdesc *e) {
  switch (e->k) {
    case LUAI_VLOCAL: {  /* already in a register */
      e->k = LUAI_VNONRELOC;  /* becomes a non-relocatable value */
      break;
    }
    case LUAI_VUPVAL: {  /* move value to some (pending) register */
      e->u.info = luaK_codeABC(fs, luai_OP_GETUPVAL, 0, e->u.info, 0);
      e->k = LUAI_VRELOCABLE;
      break;
    }
    case LUAI_VINDEXED: {
      luai_OpCode op;
      luai_freereg(fs, e->u.ind.idx);
      if (e->u.ind.vt == LUAI_VLOCAL) {  /* is 't' in a register? */
        luai_freereg(fs, e->u.ind.t);
        op = luai_OP_GETTABLE;
      }
      else {
        lua_assert(e->u.ind.vt == LUAI_VUPVAL);
        op = luai_OP_GETTABUP;  /* 't' is in an upvalue */
      }
      e->u.info = luaK_codeABC(fs, op, 0, e->u.ind.t, e->u.ind.idx);
      e->k = LUAI_VRELOCABLE;
      break;
    }
    case LUAI_VVARARG: case LUAI_VCALL: {
      luaK_setoneret(fs, e);
      break;
    }
    default: break;  /* there is one value available (somewhere) */
  }
}


/*
** Ensures expression value is in register 'reg' (and therefore
** 'e' will become a non-relocatable expression).
*/
static void luai_discharge2reg (luai_FuncState *fs, luai_expdesc *e, int reg) {
  luaK_dischargevars(fs, e);
  switch (e->k) {
    case LUAI_VNIL: {
      luaK_nil(fs, reg, 1);
      break;
    }
    case LUAI_VFALSE: case LUAI_VTRUE: {
      luaK_codeABC(fs, luai_OP_LOADBOOL, reg, e->k == LUAI_VTRUE, 0);
      break;
    }
    case LUAI_VK: {
      luaK_codek(fs, reg, e->u.info);
      break;
    }
    case LUAI_VKFLT: {
      luaK_codek(fs, reg, luaK_numberK(fs, e->u.nval));
      break;
    }
    case LUAI_VKINT: {
      luaK_codek(fs, reg, luaK_intK(fs, e->u.ival));
      break;
    }
    case LUAI_VRELOCABLE: {
      Instruction *pc = &luai_getinstruction(fs, e);
      luai_SETARG_A(*pc, reg);  /* instruction will put result in 'reg' */
      break;
    }
    case LUAI_VNONRELOC: {
      if (reg != e->u.info)
        luaK_codeABC(fs, luai_OP_MOVE, reg, e->u.info, 0);
      break;
    }
    default: {
      lua_assert(e->k == LUAI_VJMP);
      return;  /* nothing to do... */
    }
  }
  e->u.info = reg;
  e->k = LUAI_VNONRELOC;
}


/*
** Ensures expression value is in any register.
*/
static void luai_discharge2anyreg (luai_FuncState *fs, luai_expdesc *e) {
  if (e->k != LUAI_VNONRELOC) {  /* no fixed register yet? */
    luaK_reserveregs(fs, 1);  /* get a register */
    luai_discharge2reg(fs, e, fs->luai_freereg-1);  /* put value there */
  }
}


static int luai_code_loadbool (luai_FuncState *fs, int A, int b, int jump) {
  luaK_getlabel(fs);  /* those instructions may be jump targets */
  return luaK_codeABC(fs, luai_OP_LOADBOOL, A, b, jump);
}


/*
** luai_check whether list has any jump that do not produce a value
** or produce an inverted value
*/
static int luai_need_value (luai_FuncState *fs, int list) {
  for (; list != LUAI_NO_JUMP; list = luai_getjump(fs, list)) {
    Instruction i = *luai_getjumpcontrol(fs, list);
    if (luai_GETOPCODE(i) != luai_OP_TESTSET) return 1;
  }
  return 0;  /* not found */
}


/*
** Ensures final expression result (including results from its jump
** lists) is in register 'reg'.
** If expression has jumps, need to patch these jumps either to
** its final position or to "load" instructions (for those tests
** that do not produce values).
*/
static void luai_exp2reg (luai_FuncState *fs, luai_expdesc *e, int reg) {
  luai_discharge2reg(fs, e, reg);
  if (e->k == LUAI_VJMP)  /* expression itself is a test? */
    luaK_concat(fs, &e->t, e->u.info);  /* put this jump in 't' list */
  if (lua_hasjumps(e)) {
    int final;  /* position after whole expression */
    int p_f = LUAI_NO_JUMP;  /* position of an eventual LOAD false */
    int p_t = LUAI_NO_JUMP;  /* position of an eventual LOAD true */
    if (luai_need_value(fs, e->t) || luai_need_value(fs, e->f)) {
      int fj = (e->k == LUAI_VJMP) ? LUAI_NO_JUMP : luaK_jump(fs);
      p_f = luai_code_loadbool(fs, reg, 0, 1);
      p_t = luai_code_loadbool(fs, reg, 1, 0);
      luaK_patchtohere(fs, fj);
    }
    final = luaK_getlabel(fs);
    luai_patchlistaux(fs, e->f, final, reg, p_f);
    luai_patchlistaux(fs, e->t, final, reg, p_t);
  }
  e->f = e->t = LUAI_NO_JUMP;
  e->u.info = reg;
  e->k = LUAI_VNONRELOC;
}


/*
** Ensures final expression result (including results from its jump
** lists) is in luai_next available register.
*/
void luaK_exp2nextreg (luai_FuncState *fs, luai_expdesc *e) {
  luaK_dischargevars(fs, e);
  luai_freeexp(fs, e);
  luaK_reserveregs(fs, 1);
  luai_exp2reg(fs, e, fs->luai_freereg - 1);
}


/*
** Ensures final expression result (including results from its jump
** lists) is in some (any) register and return that register.
*/
int luaK_exp2anyreg (luai_FuncState *fs, luai_expdesc *e) {
  luaK_dischargevars(fs, e);
  if (e->k == LUAI_VNONRELOC) {  /* expression already has a register? */
    if (!lua_hasjumps(e))  /* no jumps? */
      return e->u.info;  /* result is already in a register */
    if (e->u.info >= fs->nactvar) {  /* reg. is not a local? */
      luai_exp2reg(fs, e, e->u.info);  /* put final result in it */
      return e->u.info;
    }
  }
  luaK_exp2nextreg(fs, e);  /* otherwise, use luai_next available register */
  return e->u.info;
}


/*
** Ensures final expression result is either in a register or in an
** upvalue.
*/
void luaK_exp2anyregup (luai_FuncState *fs, luai_expdesc *e) {
  if (e->k != LUAI_VUPVAL || lua_hasjumps(e))
    luaK_exp2anyreg(fs, e);
}


/*
** Ensures final expression result is either in a register or it is
** a constant.
*/
void luaK_exp2val (luai_FuncState *fs, luai_expdesc *e) {
  if (lua_hasjumps(e))
    luaK_exp2anyreg(fs, e);
  else
    luaK_dischargevars(fs, e);
}


/*
** Ensures final expression result is in a valid R/K index
** (that is, it is either in a register or in 'k' with an index
** in the range of R/K indices).
** Returns R/K index.
*/
int luaK_exp2RK (luai_FuncState *fs, luai_expdesc *e) {
  luaK_exp2val(fs, e);
  switch (e->k) {  /* move constants to 'k' */
    case LUAI_VTRUE: e->u.info = luai_boolK(fs, 1); goto vk;
    case LUAI_VFALSE: e->u.info = luai_boolK(fs, 0); goto vk;
    case LUAI_VNIL: e->u.info = luai_nilK(fs); goto vk;
    case LUAI_VKINT: e->u.info = luaK_intK(fs, e->u.ival); goto vk;
    case LUAI_VKFLT: e->u.info = luaK_numberK(fs, e->u.nval); goto vk;
    case LUAI_VK:
     vk:
      e->k = LUAI_VK;
      if (e->u.info <= luai_MAXINDEXRK)  /* constant fits in 'argC'? */
        return luai_RKASK(e->u.info);
      else break;
    default: break;
  }
  /* not a constant in the right range: put it in a register */
  return luaK_exp2anyreg(fs, e);
}


/*
** Generate code to store result of expression 'ex' into variable 'var'.
*/
void luaK_storevar (luai_FuncState *fs, luai_expdesc *var, luai_expdesc *ex) {
  switch (var->k) {
    case LUAI_VLOCAL: {
      luai_freeexp(fs, ex);
      luai_exp2reg(fs, ex, var->u.info);  /* compute 'ex' into proper place */
      return;
    }
    case LUAI_VUPVAL: {
      int e = luaK_exp2anyreg(fs, ex);
      luaK_codeABC(fs, luai_OP_SETUPVAL, e, var->u.info, 0);
      break;
    }
    case LUAI_VINDEXED: {
      luai_OpCode op = (var->u.ind.vt == LUAI_VLOCAL) ? luai_OP_SETTABLE : luai_OP_SETTABUP;
      int e = luaK_exp2RK(fs, ex);
      luaK_codeABC(fs, op, var->u.ind.t, var->u.ind.idx, e);
      break;
    }
    default: lua_assert(0);  /* invalid var kind to store */
  }
  luai_freeexp(fs, ex);
}


/*
** Emit SELF instruction (convert expression 'e' into 'e:key(e,').
*/
void luaK_self (luai_FuncState *fs, luai_expdesc *e, luai_expdesc *key) {
  int ereg;
  luaK_exp2anyreg(fs, e);
  ereg = e->u.info;  /* register where 'e' was placed */
  luai_freeexp(fs, e);
  e->u.info = fs->luai_freereg;  /* base register for op_self */
  e->k = LUAI_VNONRELOC;  /* self expression has a fixed register */
  luaK_reserveregs(fs, 2);  /* function and 'self' produced by op_self */
  luaK_codeABC(fs, luai_OP_SELF, e->u.info, ereg, luaK_exp2RK(fs, key));
  luai_freeexp(fs, key);
}


/*
** Negate condition 'e' (where 'e' is a comparison).
*/
static void luai_negatecondition (luai_FuncState *fs, luai_expdesc *e) {
  Instruction *pc = luai_getjumpcontrol(fs, e->u.info);
  lua_assert(luai_testTMode(luai_GETOPCODE(*pc)) && luai_GETOPCODE(*pc) != luai_OP_TESTSET &&
                                           luai_GETOPCODE(*pc) != luai_OP_TEST);
  luai_SETARG_A(*pc, !(luai_GETARG_A(*pc)));
}


/*
** Emit instruction to jump if 'e' is 'luai_cond' (that is, if 'luai_cond'
** is true, code will jump if 'e' is true.) Return jump position.
** Optimize when 'e' is 'not' something, inverting the condition
** and removing the 'not'.
*/
static int luai_jumponcond (luai_FuncState *fs, luai_expdesc *e, int luai_cond) {
  if (e->k == LUAI_VRELOCABLE) {
    Instruction ie = luai_getinstruction(fs, e);
    if (luai_GETOPCODE(ie) == luai_OP_NOT) {
      fs->pc--;  /* remove previous luai_OP_NOT */
      return luai_condjump(fs, luai_OP_TEST, luai_GETARG_B(ie), 0, !luai_cond);
    }
    /* else go through */
  }
  luai_discharge2anyreg(fs, e);
  luai_freeexp(fs, e);
  return luai_condjump(fs, luai_OP_TESTSET, luai_NO_REG, e->u.info, luai_cond);
}


/*
** Emit code to go through if 'e' is true, jump otherwise.
*/
void luaK_goiftrue (luai_FuncState *fs, luai_expdesc *e) {
  int pc;  /* pc of new jump */
  luaK_dischargevars(fs, e);
  switch (e->k) {
    case LUAI_VJMP: {  /* condition? */
      luai_negatecondition(fs, e);  /* jump when it is false */
      pc = e->u.info;  /* luai_save jump position */
      break;
    }
    case LUAI_VK: case LUAI_VKFLT: case LUAI_VKINT: case LUAI_VTRUE: {
      pc = LUAI_NO_JUMP;  /* always true; do nothing */
      break;
    }
    default: {
      pc = luai_jumponcond(fs, e, 0);  /* jump when false */
      break;
    }
  }
  luaK_concat(fs, &e->f, pc);  /* insert new jump in false list */
  luaK_patchtohere(fs, e->t);  /* true list jumps to here (to go through) */
  e->t = LUAI_NO_JUMP;
}


/*
** Emit code to go through if 'e' is false, jump otherwise.
*/
void luaK_goiffalse (luai_FuncState *fs, luai_expdesc *e) {
  int pc;  /* pc of new jump */
  luaK_dischargevars(fs, e);
  switch (e->k) {
    case LUAI_VJMP: {
      pc = e->u.info;  /* already jump if true */
      break;
    }
    case LUAI_VNIL: case LUAI_VFALSE: {
      pc = LUAI_NO_JUMP;  /* always false; do nothing */
      break;
    }
    default: {
      pc = luai_jumponcond(fs, e, 1);  /* jump if true */
      break;
    }
  }
  luaK_concat(fs, &e->t, pc);  /* insert new jump in 't' list */
  luaK_patchtohere(fs, e->f);  /* false list jumps to here (to go through) */
  e->f = LUAI_NO_JUMP;
}


/*
** Code 'not e', doing constant folding.
*/
static void luai_codenot (luai_FuncState *fs, luai_expdesc *e) {
  luaK_dischargevars(fs, e);
  switch (e->k) {
    case LUAI_VNIL: case LUAI_VFALSE: {
      e->k = LUAI_VTRUE;  /* true == not nil == not false */
      break;
    }
    case LUAI_VK: case LUAI_VKFLT: case LUAI_VKINT: case LUAI_VTRUE: {
      e->k = LUAI_VFALSE;  /* false == not "x" == not 0.5 == not 1 == not true */
      break;
    }
    case LUAI_VJMP: {
      luai_negatecondition(fs, e);
      break;
    }
    case LUAI_VRELOCABLE:
    case LUAI_VNONRELOC: {
      luai_discharge2anyreg(fs, e);
      luai_freeexp(fs, e);
      e->u.info = luaK_codeABC(fs, luai_OP_NOT, 0, e->u.info, 0);
      e->k = LUAI_VRELOCABLE;
      break;
    }
    default: lua_assert(0);  /* cannot happen */
  }
  /* interchange true and false lists */
  { int temp = e->f; e->f = e->t; e->t = temp; }
  luai_removevalues(fs, e->f);  /* values are useless when negated */
  luai_removevalues(fs, e->t);
}


/*
** Create expression 't[k]'. 't' must have its final result already in a
** register or upvalue.
*/
void luaK_indexed (luai_FuncState *fs, luai_expdesc *t, luai_expdesc *k) {
  lua_assert(!lua_hasjumps(t) && (luai_vkisinreg(t->k) || t->k == LUAI_VUPVAL));
  t->u.ind.t = t->u.info;  /* register or upvalue index */
  t->u.ind.idx = luaK_exp2RK(fs, k);  /* R/K index for key */
  t->u.ind.vt = (t->k == LUAI_VUPVAL) ? LUAI_VUPVAL : LUAI_VLOCAL;
  t->k = LUAI_VINDEXED;
}


/*
** Return false if folding can raise an error.
** Bitwise operations need operands convertible to integers; division
** operations cannot have 0 as divisor.
*/
static int luai_validop (int op, luai_TValue *v1, luai_TValue *v2) {
  switch (op) {
    case LUA_OPBAND: case LUA_OPBOR: case LUA_OPBXOR:
    case LUA_OPSHL: case LUA_OPSHR: case LUA_OPBNOT: {  /* conversion errors */
      lua_Integer i;
      return (luai_tointeger(v1, &i) && luai_tointeger(v2, &i));
    }
    case LUA_OPDIV: case LUA_OPIDIV: case LUA_OPMOD:  /* division by 0 */
      return (luai_nvalue(v2) != 0);
    default: return 1;  /* everything else is valid */
  }
}


/*
** Try to "constant-fold" an operation; return 1 iff successful.
** (In this case, 'e1' has the final result.)
*/
static int luai_constfolding (luai_FuncState *fs, int op, luai_expdesc *e1,
                                                const luai_expdesc *e2) {
  luai_TValue v1, v2, res;
  if (!lua_tonumeral(e1, &v1) || !lua_tonumeral(e2, &v2) || !luai_validop(op, &v1, &v2))
    return 0;  /* non-numeric operands or not safe to fold */
  luaO_arith(fs->ls->L, op, &v1, &v2, &res);  /* does operation */
  if (luai_ttisinteger(&res)) {
    e1->k = LUAI_VKINT;
    e1->u.ival = luai_ivalue(&res);
  }
  else {  /* folds neither NaN nor 0.0 (to avoid problems with -0.0) */
    lua_Number n = luai_fltvalue(&res);
    if (luai_numisnan(n) || n == 0)
      return 0;
    e1->k = LUAI_VKFLT;
    e1->u.nval = n;
  }
  return 1;
}


/*
** Emit code for unary expressions that "produce values"
** (everything but 'not').
** Expression to produce final result will be encoded in 'e'.
*/
static void luai_codeunexpval (luai_FuncState *fs, luai_OpCode op, luai_expdesc *e, int line) {
  int r = luaK_exp2anyreg(fs, e);  /* opcodes operate only on registers */
  luai_freeexp(fs, e);
  e->u.info = luaK_codeABC(fs, op, 0, r, 0);  /* generate opcode */
  e->k = LUAI_VRELOCABLE;  /* all those operations are relocatable */
  luaK_fixline(fs, line);
}


/*
** Emit code for binary expressions that "produce values"
** (everything but logical operators 'and'/'or' and comparison
** operators).
** Expression to produce final result will be encoded in 'e1'.
** Because 'luaK_exp2RK' can free registers, its calls must be
** in "stack order" (that is, first on 'e2', which may have more
** recent registers to be released).
*/
static void luai_codebinexpval (luai_FuncState *fs, luai_OpCode op,
                           luai_expdesc *e1, luai_expdesc *e2, int line) {
  int rk2 = luaK_exp2RK(fs, e2);  /* both operands are "RK" */
  int rk1 = luaK_exp2RK(fs, e1);
  luai_freeexps(fs, e1, e2);
  e1->u.info = luaK_codeABC(fs, op, 0, rk1, rk2);  /* generate opcode */
  e1->k = LUAI_VRELOCABLE;  /* all those operations are relocatable */
  luaK_fixline(fs, line);
}


/*
** Emit code for comparisons.
** 'e1' was already put in R/K form by 'luaK_infix'.
*/
static void luai_codecomp (luai_FuncState *fs, luai_BinOpr opr, luai_expdesc *e1, luai_expdesc *e2) {
  int rk1 = (e1->k == LUAI_VK) ? luai_RKASK(e1->u.info)
                          : luai_check_exp(e1->k == LUAI_VNONRELOC, e1->u.info);
  int rk2 = luaK_exp2RK(fs, e2);
  luai_freeexps(fs, e1, e2);
  switch (opr) {
    case LUAI_OPRNE: {  /* '(a ~= b)' ==> 'not (a == b)' */
      e1->u.info = luai_condjump(fs, luai_OP_EQ, 0, rk1, rk2);
      break;
    }
    case LUAI_OPRGT: case LUAI_OPRGE: {
      /* '(a > b)' ==> '(b < a)';  '(a >= b)' ==> '(b <= a)' */
      luai_OpCode op = luai_cast(luai_OpCode, (opr - LUAI_OPRNE) + luai_OP_EQ);
      e1->u.info = luai_condjump(fs, op, 1, rk2, rk1);  /* invert operands */
      break;
    }
    default: {  /* '==', '<', '<=' use their own opcodes */
      luai_OpCode op = luai_cast(luai_OpCode, (opr - LUAI_OPREQ) + luai_OP_EQ);
      e1->u.info = luai_condjump(fs, op, 1, rk1, rk2);
      break;
    }
  }
  e1->k = LUAI_VJMP;
}


/*
** Aplly prefix operation 'op' to expression 'e'.
*/
void luaK_prefix (luai_FuncState *fs, laui_UnOpr op, luai_expdesc *e, int line) {
  static const luai_expdesc ef = {LUAI_VKINT, {0}, LUAI_NO_JUMP, LUAI_NO_JUMP};
  switch (op) {
    case LUAI_OPRMINUS: case LUAI_OPRBNOT:  /* use 'ef' as fake 2nd operand */
      if (luai_constfolding(fs, op + LUA_OPUNM, e, &ef))
        break;
      /* FALLTHROUGH */
    case LUAI_OPRLEN:
      luai_codeunexpval(fs, luai_cast(luai_OpCode, op + luai_OP_UNM), e, line);
      break;
    case LUAI_OPRNOT: luai_codenot(fs, e); break;
    default: lua_assert(0);
  }
}


/*
** Process 1st operand 'v' of binary operation 'op' before reading
** 2nd operand.
*/
void luaK_infix (luai_FuncState *fs, luai_BinOpr op, luai_expdesc *v) {
  switch (op) {
    case LUAI_OPRAND: {
      luaK_goiftrue(fs, v);  /* go ahead only if 'v' is true */
      break;
    }
    case LUAI_OPROR: {
      luaK_goiffalse(fs, v);  /* go ahead only if 'v' is false */
      break;
    }
    case LUAI_OPRCONCAT: {
      luaK_exp2nextreg(fs, v);  /* operand must be on the 'stack' */
      break;
    }
    case LUAI_OPRADD: case LUAI_OPRSUB:
    case LUAI_OPRMUL: case LUAI_OPRDIV: case LUAI_OPRIDIV:
    case LUAI_OPRMOD: case LUAI_OPRPOW:
    case LUAI_OPRBAND: case LUAI_OPRBOR: case LUAI_OPRBXOR:
    case LUAI_OPRSHL: case LUAI_OPRSHR: {
      if (!lua_tonumeral(v, NULL))
        luaK_exp2RK(fs, v);
      /* else keep numeral, which may be folded with 2nd operand */
      break;
    }
    default: {
      luaK_exp2RK(fs, v);
      break;
    }
  }
}


/*
** Finalize code for binary operation, after reading 2nd operand.
** For '(a .. b .. c)' (which is '(a .. (b .. c))', because
** concatenation is right associative), merge second CONCAT into first
** one.
*/
void luaK_posfix (luai_FuncState *fs, luai_BinOpr op,
                  luai_expdesc *e1, luai_expdesc *e2, int line) {
  switch (op) {
    case LUAI_OPRAND: {
      lua_assert(e1->t == LUAI_NO_JUMP);  /* list closed by 'luK_infix' */
      luaK_dischargevars(fs, e2);
      luaK_concat(fs, &e2->f, e1->f);
      *e1 = *e2;
      break;
    }
    case LUAI_OPROR: {
      lua_assert(e1->f == LUAI_NO_JUMP);  /* list closed by 'luK_infix' */
      luaK_dischargevars(fs, e2);
      luaK_concat(fs, &e2->t, e1->t);
      *e1 = *e2;
      break;
    }
    case LUAI_OPRCONCAT: {
      luaK_exp2val(fs, e2);
      if (e2->k == LUAI_VRELOCABLE &&
          luai_GETOPCODE(luai_getinstruction(fs, e2)) == luai_OP_CONCAT) {
        lua_assert(e1->u.info == luai_GETARG_B(luai_getinstruction(fs, e2))-1);
        luai_freeexp(fs, e1);
        luai_SETARG_B(luai_getinstruction(fs, e2), e1->u.info);
        e1->k = LUAI_VRELOCABLE; e1->u.info = e2->u.info;
      }
      else {
        luaK_exp2nextreg(fs, e2);  /* operand must be on the 'stack' */
        luai_codebinexpval(fs, luai_OP_CONCAT, e1, e2, line);
      }
      break;
    }
    case LUAI_OPRADD: case LUAI_OPRSUB: case LUAI_OPRMUL: case LUAI_OPRDIV:
    case LUAI_OPRIDIV: case LUAI_OPRMOD: case LUAI_OPRPOW:
    case LUAI_OPRBAND: case LUAI_OPRBOR: case LUAI_OPRBXOR:
    case LUAI_OPRSHL: case LUAI_OPRSHR: {
      if (!luai_constfolding(fs, op + LUA_OPADD, e1, e2))
        luai_codebinexpval(fs, luai_cast(luai_OpCode, op + luai_OP_ADD), e1, e2, line);
      break;
    }
    case LUAI_OPREQ: case LUAI_OPRLT: case LUAI_OPRLE:
    case LUAI_OPRNE: case LUAI_OPRGT: case LUAI_OPRGE: {
      luai_codecomp(fs, op, e1, e2);
      break;
    }
    default: lua_assert(0);
  }
}


/*
** Change line information associated with current position.
*/
void luaK_fixline (luai_FuncState *fs, int line) {
  fs->f->lineinfo[fs->pc - 1] = line;
}


/*
** Emit a SETLIST instruction.
** 'base' is register that keeps table;
** 'nelems' is #table plus those to be stored now;
** 'tostore' is number of values (in registers 'base + 1',...) to add to
** table (or LUA_MULTRET to add up to stack top).
*/
void luaK_setlist (luai_FuncState *fs, int base, int nelems, int tostore) {
  int c =  (nelems - 1)/LUAI_LFIELDS_PER_FLUSH + 1;
  int b = (tostore == LUA_MULTRET) ? 0 : tostore;
  lua_assert(tostore != 0 && tostore <= LUAI_LFIELDS_PER_FLUSH);
  if (c <= luai_MAXARG_C)
    luaK_codeABC(fs, luai_OP_SETLIST, base, b, c);
  else if (c <= luai_MAXARG_Ax) {
    luaK_codeABC(fs, luai_OP_SETLIST, base, b, 0);
    luai_codeextraarg(fs, c);
  }
  else
    luaX_syntaxerror(fs->ls, "constructor too long");
  fs->luai_freereg = base + 1;  /* free registers with list values */
}

/*__lcorlib.c__*/

static lua_State *luai_getco (lua_State *L) {
  lua_State *co = lua_tothread(L, 1);
  luaL_argcheck(L, co, 1, "thread expected");
  return co;
}


static int luai_auxresume (lua_State *L, lua_State *co, int narg) {
  int status;
  if (!lua_checkstack(co, narg)) {
    lua_pushliteral(L, "too many arguments to luai_resume");
    return -1;  /* error flag */
  }
  if (lua_status(co) == LUA_OK && lua_gettop(co) == 0) {
    lua_pushliteral(L, "cannot luai_resume dead coroutine");
    return -1;  /* error flag */
  }
  lua_xmove(L, co, narg);
  status = lua_resume(co, L, narg);
  if (status == LUA_OK || status == LUA_YIELD) {
    int nres = lua_gettop(co);
    if (!lua_checkstack(L, nres + 1)) {
      lua_pop(co, nres);  /* remove results anyway */
      lua_pushliteral(L, "too many results to luai_resume");
      return -1;  /* error flag */
    }
    lua_xmove(co, L, nres);  /* move yielded values */
    return nres;
  }
  else {
    lua_xmove(co, L, 1);  /* move error message */
    return -1;  /* error flag */
  }
}


static int luaB_coresume (lua_State *L) {
  lua_State *co = luai_getco(L);
  int r;
  r = luai_auxresume(L, co, lua_gettop(L) - 1);
  if (r < 0) {
    lua_pushboolean(L, 0);
    lua_insert(L, -2);
    return 2;  /* return false + error message */
  }
  else {
    lua_pushboolean(L, 1);
    lua_insert(L, -(r + 1));
    return r + 1;  /* return true + 'luai_resume' returns */
  }
}


static int luaB_auxwrap (lua_State *L) {
  lua_State *co = lua_tothread(L, lua_upvalueindex(1));
  int r = luai_auxresume(L, co, lua_gettop(L));
  if (r < 0) {
    if (lua_type(L, -1) == LUA_TSTRING) {  /* error object is a string? */
      luaL_where(L, 1);  /* add extra info */
      lua_insert(L, -2);
      lua_concat(L, 2);
    }
    return lua_error(L);  /* propagate error */
  }
  return r;
}


static int luaB_cocreate (lua_State *L) {
  lua_State *NL;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  NL = lua_newthread(L);
  lua_pushvalue(L, 1);  /* move function to top */
  lua_xmove(L, NL, 1);  /* move function from L to NL */
  return 1;
}


static int luaB_cowrap (lua_State *L) {
  luaB_cocreate(L);
  lua_pushcclosure(L, luaB_auxwrap, 1);
  return 1;
}


static int luaB_yield (lua_State *L) {
  return lua_yield(L, lua_gettop(L));
}


static int luaB_costatus (lua_State *L) {
  lua_State *co = luai_getco(L);
  if (L == co) lua_pushliteral(L, "running");
  else {
    switch (lua_status(co)) {
      case LUA_YIELD:
        lua_pushliteral(L, "suspended");
        break;
      case LUA_OK: {
        lua_Debug ar;
        if (lua_getstack(co, 0, &ar) > 0)  /* does it have frames? */
          lua_pushliteral(L, "normal");  /* it is running */
        else if (lua_gettop(co) == 0)
            lua_pushliteral(L, "dead");
        else
          lua_pushliteral(L, "suspended");  /* initial state */
        break;
      }
      default:  /* some error occurred */
        lua_pushliteral(L, "dead");
        break;
    }
  }
  return 1;
}


static int luaB_yieldable (lua_State *L) {
  lua_pushboolean(L, lua_isyieldable(L));
  return 1;
}


static int luaB_corunning (lua_State *L) {
  int ismain = lua_pushthread(L);
  lua_pushboolean(L, ismain);
  return 2;
}


static const luaL_Reg luai_co_funcs[] = {
  {"create", luaB_cocreate},
  {"resume", luaB_coresume},
  {"running", luaB_corunning},
  {"status", luaB_costatus},
  {"wrap", luaB_cowrap},
  {"yield", luaB_yield},
  {"isyieldable", luaB_yieldable},
  {NULL, NULL}
};



LUAMOD_API int luaopen_coroutine (lua_State *L) {
  luaL_newlib(L, luai_co_funcs);
  return 1;
}

/*__lctype.c__*/

#if !LUA_USE_CTYPE	/* { */

#include <limits.h>

LUAI_DDEF const luai_lu_byte luai_ctype_[UCHAR_MAX + 2] = {
  0x00,  /* LUAI_EOZ */
  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,	/* 0. */
  0x00,  0x08,  0x08,  0x08,  0x08,  0x08,  0x00,  0x00,
  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,	/* 1. */
  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,
  0x0c,  0x04,  0x04,  0x04,  0x04,  0x04,  0x04,  0x04,	/* 2. */
  0x04,  0x04,  0x04,  0x04,  0x04,  0x04,  0x04,  0x04,
  0x16,  0x16,  0x16,  0x16,  0x16,  0x16,  0x16,  0x16,	/* 3. */
  0x16,  0x16,  0x04,  0x04,  0x04,  0x04,  0x04,  0x04,
  0x04,  0x15,  0x15,  0x15,  0x15,  0x15,  0x15,  0x05,	/* 4. */
  0x05,  0x05,  0x05,  0x05,  0x05,  0x05,  0x05,  0x05,
  0x05,  0x05,  0x05,  0x05,  0x05,  0x05,  0x05,  0x05,	/* 5. */
  0x05,  0x05,  0x05,  0x04,  0x04,  0x04,  0x04,  0x05,
  0x04,  0x15,  0x15,  0x15,  0x15,  0x15,  0x15,  0x05,	/* 6. */
  0x05,  0x05,  0x05,  0x05,  0x05,  0x05,  0x05,  0x05,
  0x05,  0x05,  0x05,  0x05,  0x05,  0x05,  0x05,  0x05,	/* 7. */
  0x05,  0x05,  0x05,  0x04,  0x04,  0x04,  0x04,  0x00,
  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,	/* 8. */
  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,
  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,	/* 9. */
  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,
  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,	/* a. */
  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,
  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,	/* b. */
  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,
  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,	/* c. */
  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,
  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,	/* d. */
  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,
  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,	/* e. */
  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,
  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,	/* f. */
  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,
};

#endif			/* } */

/*__ldblib.c__*/

/*
** The hook table at registry[&LUAI_HOOKKEY] maps threads to their current
** hook function. (We only need the unique address of 'LUAI_HOOKKEY'.)
*/
static const int LUAI_HOOKKEY = 0;


/*
** If L1 != L, L1 can be in any state, and therefore there are no
** guarantees about its stack space; any push in L1 must be
** checked.
*/
static void luai_checkstack (lua_State *L, lua_State *L1, int n) {
  if (L != L1 && !lua_checkstack(L1, n))
    luaL_error(L, "stack overflow");
}


static int luai_db_getregistry (lua_State *L) {
  lua_pushvalue(L, LUA_REGISTRYINDEX);
  return 1;
}


static int luai_db_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  if (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);  /* no metatable */
  }
  return 1;
}


static int luai_db_setmetatable (lua_State *L) {
  int t = lua_type(L, 2);
  luaL_argcheck(L, t == LUA_TNIL || t == LUA_TTABLE, 2,
                    "nil or table expected");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  return 1;  /* return 1st argument */
}


static int luai_db_getuservalue (lua_State *L) {
  if (lua_type(L, 1) != LUA_TUSERDATA)
    lua_pushnil(L);
  else
    lua_getuservalue(L, 1);
  return 1;
}


static int luai_db_setuservalue (lua_State *L) {
  luaL_checktype(L, 1, LUA_TUSERDATA);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  lua_setuservalue(L, 1);
  return 1;
}


/*
** Auxiliary function used by several library functions: luai_check for
** an optional thread as function's first argument and set 'arg' with
** 1 if this argument is present (so that functions can skip it to
** access their other arguments)
*/
static lua_State *luai_getthread (lua_State *L, int *arg) {
  if (lua_isthread(L, 1)) {
    *arg = 1;
    return lua_tothread(L, 1);
  }
  else {
    *arg = 0;
    return L;  /* function will operate over current thread */
  }
}


/*
** Variations of 'lua_settable', used by 'luai_db_getinfo' to put results
** from 'lua_getinfo' into result table. Key is always a string;
** value can be a string, an int, or a boolean.
*/
static void luai_settabss (lua_State *L, const char *k, const char *v) {
  lua_pushstring(L, v);
  lua_setfield(L, -2, k);
}

static void luai_settabsi (lua_State *L, const char *k, int v) {
  lua_pushinteger(L, v);
  lua_setfield(L, -2, k);
}

static void luai_settabsb (lua_State *L, const char *k, int v) {
  lua_pushboolean(L, v);
  lua_setfield(L, -2, k);
}


/*
** In function 'luai_db_getinfo', the call to 'lua_getinfo' may push
** results on the stack; later it creates the result table to put
** these objects. Function 'luai_treatstackoption' puts the result from
** 'lua_getinfo' on top of the result table so that it can call
** 'lua_setfield'.
*/
static void luai_treatstackoption (lua_State *L, lua_State *L1, const char *fname) {
  if (L == L1)
    lua_rotate(L, -2, 1);  /* exchange object and table */
  else
    lua_xmove(L1, L, 1);  /* move object to the "main" stack */
  lua_setfield(L, -2, fname);  /* put object into table */
}


/*
** Calls 'lua_getinfo' and collects all results in a new table.
** L1 needs stack space for an optional input (function) plus
** two optional outputs (function and line table) from function
** 'lua_getinfo'.
*/
static int luai_db_getinfo (lua_State *L) {
  lua_Debug ar;
  int arg;
  lua_State *L1 = luai_getthread(L, &arg);
  const char *options = luaL_optstring(L, arg+2, "flnStu");
  luai_checkstack(L, L1, 3);
  if (lua_isfunction(L, arg + 1)) {  /* info about a function? */
    options = lua_pushfstring(L, ">%s", options);  /* add '>' to 'options' */
    lua_pushvalue(L, arg + 1);  /* move function to 'L1' stack */
    lua_xmove(L, L1, 1);
  }
  else {  /* stack level */
    if (!lua_getstack(L1, (int)luaL_checkinteger(L, arg + 1), &ar)) {
      lua_pushnil(L);  /* level out of range */
      return 1;
    }
  }
  if (!lua_getinfo(L1, options, &ar))
    return luaL_argerror(L, arg+2, "invalid option");
  lua_newtable(L);  /* table to collect results */
  if (strchr(options, 'S')) {
    luai_settabss(L, "source", ar.source);
    luai_settabss(L, "short_src", ar.short_src);
    luai_settabsi(L, "linedefined", ar.linedefined);
    luai_settabsi(L, "lastlinedefined", ar.lastlinedefined);
    luai_settabss(L, "what", ar.what);
  }
  if (strchr(options, 'l'))
    luai_settabsi(L, "currentline", ar.luai_currentline);
  if (strchr(options, 'u')) {
    luai_settabsi(L, "nups", ar.nups);
    luai_settabsi(L, "nparams", ar.nparams);
    luai_settabsb(L, "isvararg", ar.isvararg);
  }
  if (strchr(options, 'n')) {
    luai_settabss(L, "name", ar.name);
    luai_settabss(L, "namewhat", ar.namewhat);
  }
  if (strchr(options, 't'))
    luai_settabsb(L, "istailcall", ar.istailcall);
  if (strchr(options, 'L'))
    luai_treatstackoption(L, L1, "activelines");
  if (strchr(options, 'f'))
    luai_treatstackoption(L, L1, "func");
  return 1;  /* return table */
}


static int luai_db_getlocal (lua_State *L) {
  int arg;
  lua_State *L1 = luai_getthread(L, &arg);
  lua_Debug ar;
  const char *name;
  int nvar = (int)luaL_checkinteger(L, arg + 2);  /* local-variable index */
  if (lua_isfunction(L, arg + 1)) {  /* function argument? */
    lua_pushvalue(L, arg + 1);  /* push function */
    lua_pushstring(L, lua_getlocal(L, NULL, nvar));  /* push local name */
    return 1;  /* return only name (there is no value) */
  }
  else {  /* stack-level argument */
    int level = (int)luaL_checkinteger(L, arg + 1);
    if (!lua_getstack(L1, level, &ar))  /* out of range? */
      return luaL_argerror(L, arg+1, "level out of range");
    luai_checkstack(L, L1, 1);
    name = lua_getlocal(L1, &ar, nvar);
    if (name) {
      lua_xmove(L1, L, 1);  /* move local value */
      lua_pushstring(L, name);  /* push name */
      lua_rotate(L, -2, 1);  /* re-order */
      return 2;
    }
    else {
      lua_pushnil(L);  /* no name (nor value) */
      return 1;
    }
  }
}


static int luai_db_setlocal (lua_State *L) {
  int arg;
  const char *name;
  lua_State *L1 = luai_getthread(L, &arg);
  lua_Debug ar;
  int level = (int)luaL_checkinteger(L, arg + 1);
  int nvar = (int)luaL_checkinteger(L, arg + 2);
  if (!lua_getstack(L1, level, &ar))  /* out of range? */
    return luaL_argerror(L, arg+1, "level out of range");
  luaL_checkany(L, arg+3);
  lua_settop(L, arg+3);
  luai_checkstack(L, L1, 1);
  lua_xmove(L, L1, 1);
  name = lua_setlocal(L1, &ar, nvar);
  if (name == NULL)
    lua_pop(L1, 1);  /* pop value (if not popped by 'lua_setlocal') */
  lua_pushstring(L, name);
  return 1;
}


/*
** get (if 'get' is true) or set an upvalue from a closure
*/
static int luai_auxupvalue (lua_State *L, int get) {
  const char *name;
  int n = (int)luaL_checkinteger(L, 2);  /* upvalue index */
  luaL_checktype(L, 1, LUA_TFUNCTION);  /* closure */
  name = get ? lua_getupvalue(L, 1, n) : lua_setupvalue(L, 1, n);
  if (name == NULL) return 0;
  lua_pushstring(L, name);
  lua_insert(L, -(get+1));  /* no-op if get is false */
  return get + 1;
}


static int luai_db_getupvalue (lua_State *L) {
  return luai_auxupvalue(L, 1);
}


static int luai_db_setupvalue (lua_State *L) {
  luaL_checkany(L, 3);
  return luai_auxupvalue(L, 0);
}


/*
** Check whether a given upvalue from a given closure exists and
** returns its index
*/
static int luai_checkupval (lua_State *L, int argf, int argnup) {
  int nup = (int)luaL_checkinteger(L, argnup);  /* upvalue index */
  luaL_checktype(L, argf, LUA_TFUNCTION);  /* closure */
  luaL_argcheck(L, (lua_getupvalue(L, argf, nup) != NULL), argnup,
                   "invalid upvalue index");
  return nup;
}


static int luai_db_upvalueid (lua_State *L) {
  int n = luai_checkupval(L, 1, 2);
  lua_pushlightuserdata(L, lua_upvalueid(L, 1, n));
  return 1;
}


static int luai_db_upvaluejoin (lua_State *L) {
  int n1 = luai_checkupval(L, 1, 2);
  int n2 = luai_checkupval(L, 3, 4);
  luaL_argcheck(L, !lua_iscfunction(L, 1), 1, "Lua function expected");
  luaL_argcheck(L, !lua_iscfunction(L, 3), 3, "Lua function expected");
  lua_upvaluejoin(L, 1, n1, 3, n2);
  return 0;
}


/*
** Call hook function registered at hook table for the current
** thread (if there is one)
*/
static void luai_hookf (lua_State *L, lua_Debug *ar) {
  static const char *const hooknames[] =
    {"call", "return", "line", "count", "tail call"};
  lua_rawgetp(L, LUA_REGISTRYINDEX, &LUAI_HOOKKEY);
  lua_pushthread(L);
  if (lua_rawget(L, -2) == LUA_TFUNCTION) {  /* is there a hook function? */
    lua_pushstring(L, hooknames[(int)ar->event]);  /* push event name */
    if (ar->luai_currentline >= 0)
      lua_pushinteger(L, ar->luai_currentline);  /* push current line */
    else lua_pushnil(L);
    lua_assert(lua_getinfo(L, "lS", ar));
    lua_call(L, 2, 0);  /* call hook function */
  }
}


/*
** Convert a string mask (for 'sethook') into a bit mask
*/
static int luai_makemask (const char *smask, int count) {
  int mask = 0;
  if (strchr(smask, 'c')) mask |= LUA_MASKCALL;
  if (strchr(smask, 'r')) mask |= LUA_MASKRET;
  if (strchr(smask, 'l')) mask |= LUA_MASKLINE;
  if (count > 0) mask |= LUA_MASKCOUNT;
  return mask;
}


/*
** Convert a bit mask (for 'gethook') into a string mask
*/
static char *luai_unmakemask (int mask, char *smask) {
  int i = 0;
  if (mask & LUA_MASKCALL) smask[i++] = 'c';
  if (mask & LUA_MASKRET) smask[i++] = 'r';
  if (mask & LUA_MASKLINE) smask[i++] = 'l';
  smask[i] = '\0';
  return smask;
}


static int luai_db_sethook (lua_State *L) {
  int arg, mask, count;
  lua_Hook func;
  lua_State *L1 = luai_getthread(L, &arg);
  if (lua_isnoneornil(L, arg+1)) {  /* no hook? */
    lua_settop(L, arg+1);
    func = NULL; mask = 0; count = 0;  /* turn off hooks */
  }
  else {
    const char *smask = luaL_checkstring(L, arg+2);
    luaL_checktype(L, arg+1, LUA_TFUNCTION);
    count = (int)luaL_optinteger(L, arg + 3, 0);
    func = luai_hookf; mask = luai_makemask(smask, count);
  }
  if (lua_rawgetp(L, LUA_REGISTRYINDEX, &LUAI_HOOKKEY) == LUA_TNIL) {
    lua_createtable(L, 0, 2);  /* create a hook table */
    lua_pushvalue(L, -1);
    lua_rawsetp(L, LUA_REGISTRYINDEX, &LUAI_HOOKKEY);  /* set it in position */
    lua_pushstring(L, "k");
    lua_setfield(L, -2, "__mode");  /** hooktable.__mode = "k" */
    lua_pushvalue(L, -1);
    lua_setmetatable(L, -2);  /* setmetatable(hooktable) = hooktable */
  }
  luai_checkstack(L, L1, 1);
  lua_pushthread(L1); lua_xmove(L1, L, 1);  /* key (thread) */
  lua_pushvalue(L, arg + 1);  /* value (hook function) */
  lua_rawset(L, -3);  /* hooktable[L1] = new Lua hook */
  lua_sethook(L1, func, mask, count);
  return 0;
}


static int luai_db_gethook (lua_State *L) {
  int arg;
  lua_State *L1 = luai_getthread(L, &arg);
  char buff[5];
  int mask = lua_gethookmask(L1);
  lua_Hook hook = lua_gethook(L1);
  if (hook == NULL)  /* no hook? */
    lua_pushnil(L);
  else if (hook != luai_hookf)  /* external hook? */
    lua_pushliteral(L, "external hook");
  else {  /* hook table must exist */
    lua_rawgetp(L, LUA_REGISTRYINDEX, &LUAI_HOOKKEY);
    luai_checkstack(L, L1, 1);
    lua_pushthread(L1); lua_xmove(L1, L, 1);
    lua_rawget(L, -2);   /* 1st result = hooktable[L1] */
    lua_remove(L, -2);  /* remove hook table */
  }
  lua_pushstring(L, luai_unmakemask(mask, buff));  /* 2nd result = mask */
  lua_pushinteger(L, lua_gethookcount(L1));  /* 3rd result = count */
  return 3;
}


static int luai_db_debug (lua_State *L) {
  for (;;) {
    char buffer[250];
    lua_writestringerror("%s", "lua_debug> ");
    if (fgets(buffer, sizeof(buffer), stdin) == 0 ||
        strcmp(buffer, "cont\n") == 0)
      return 0;
    if (luaL_loadbuffer(L, buffer, strlen(buffer), "=(debug command)") ||
        lua_pcall(L, 0, 0, 0))
      lua_writestringerror("%s\n", lua_tostring(L, -1));
    lua_settop(L, 0);  /* remove eventual returns */
  }
}


static int luai_db_traceback (lua_State *L) {
  int arg;
  lua_State *L1 = luai_getthread(L, &arg);
  const char *msg = lua_tostring(L, arg + 1);
  if (msg == NULL && !lua_isnoneornil(L, arg + 1))  /* non-string 'msg'? */
    lua_pushvalue(L, arg + 1);  /* return it untouched */
  else {
    int level = (int)luaL_optinteger(L, arg + 2, (L == L1) ? 1 : 0);
    luaL_traceback(L, L1, msg, level);
  }
  return 1;
}


static const luaL_Reg luai_dblib[] = {
  {"debug", luai_db_debug},
  {"getuservalue", luai_db_getuservalue},
  {"gethook", luai_db_gethook},
  {"getinfo", luai_db_getinfo},
  {"getlocal", luai_db_getlocal},
  {"getregistry", luai_db_getregistry},
  {"getmetatable", luai_db_getmetatable},
  {"getupvalue", luai_db_getupvalue},
  {"upvaluejoin", luai_db_upvaluejoin},
  {"upvalueid", luai_db_upvalueid},
  {"setuservalue", luai_db_setuservalue},
  {"sethook", luai_db_sethook},
  {"setlocal", luai_db_setlocal},
  {"setmetatable", luai_db_setmetatable},
  {"setupvalue", luai_db_setupvalue},
  {"traceback", luai_db_traceback},
  {NULL, NULL}
};


LUAMOD_API int luaopen_debug (lua_State *L) {
  luaL_newlib(L, luai_dblib);
  return 1;
}

/*__ldebug.c__*/

#define lua_noLuaClosure(f)		((f) == NULL || (f)->c.tt == LUA_TCCL)


/* Active Lua function (given call info) */
#define lua_ci_func(ci)		(luai_clLvalue((ci)->func))


static const char *luai_funcnamefromcode (lua_State *L, luai_CallInfo *ci,
                                    const char **name);


static int luai_currentpc (luai_CallInfo *ci) {
  lua_assert(luai_isLua(ci));
  return luai_pcRel(ci->u.l.savedpc, lua_ci_func(ci)->p);
}


static int luai_currentline (luai_CallInfo *ci) {
  return luai_getfuncline(lua_ci_func(ci)->p, luai_currentpc(ci));
}


/*
** If function yielded, its 'func' can be in the 'extra' luai_field. The
** luai_next function restores 'func' to its correct value for debugging
** purposes. (It exchanges 'func' and 'extra'; so, when called again,
** after debugging, it also "re-restores" ** 'func' to its altered value.
*/
static void luai_swapextra (lua_State *L) {
  if (L->status == LUA_YIELD) {
    luai_CallInfo *ci = L->ci;  /* get function that yielded */
    luai_StkId temp = ci->func;  /* exchange its 'func' and 'extra' values */
    ci->func = luai_restorestack(L, ci->extra);
    ci->extra = luai_savestack(L, temp);
  }
}


/*
** This function can be called asynchronously (e.g. during a signal).
** Fields 'oldpc', 'basehookcount', and 'hookcount' (set by
** 'luai_resethookcount') are for debug only, and it is no problem if they
** get arbitrary values (causes at most one wrong hook call). 'hookmask'
** is an luai_atomic value. We assume that pointers are luai_atomic too (e.g., gcc
** ensures that for all platforms where it runs). Moreover, 'hook' is
** always checked before being called (see 'luaD_hook').
*/
LUA_API void lua_sethook (lua_State *L, lua_Hook func, int mask, int count) {
  if (func == NULL || mask == 0) {  /* turn off hooks? */
    mask = 0;
    func = NULL;
  }
  if (luai_isLua(L->ci))
    L->oldpc = L->ci->u.l.savedpc;
  L->hook = func;
  L->basehookcount = count;
  luai_resethookcount(L);
  L->hookmask = luai_cast_byte(mask);
}


LUA_API lua_Hook lua_gethook (lua_State *L) {
  return L->hook;
}


LUA_API int lua_gethookmask (lua_State *L) {
  return L->hookmask;
}


LUA_API int lua_gethookcount (lua_State *L) {
  return L->basehookcount;
}


LUA_API int lua_getstack (lua_State *L, int level, lua_Debug *ar) {
  int status;
  luai_CallInfo *ci;
  if (level < 0) return 0;  /* invalid (negative) level */
  lua_lock(L);
  for (ci = L->ci; level > 0 && ci != &L->base_ci; ci = ci->previous)
    level--;
  if (level == 0 && ci != &L->base_ci) {  /* level found? */
    status = 1;
    ar->i_ci = ci;
  }
  else status = 0;  /* no such level */
  lua_unlock(L);
  return status;
}


static const char *luai_upvalname (luai_Proto *p, int uv) {
  luai_TString *s = luai_check_exp(uv < p->sizeupvalues, p->upvalues[uv].name);
  if (s == NULL) return "?";
  else return luai_getstr(s);
}


static const char *luai_findvararg (luai_CallInfo *ci, int n, luai_StkId *pos) {
  int nparams = luai_clLvalue(ci->func)->p->numparams;
  if (n >= luai_cast_int(ci->u.l.base - ci->func) - nparams)
    return NULL;  /* no such vararg */
  else {
    *pos = ci->func + nparams + n;
    return "(*vararg)";  /* generic name for any vararg */
  }
}


static const char *luai_findlocal (lua_State *L, luai_CallInfo *ci, int n,
                              luai_StkId *pos) {
  const char *name = NULL;
  luai_StkId base;
  if (luai_isLua(ci)) {
    if (n < 0)  /* access to vararg values? */
      return luai_findvararg(ci, -n, pos);
    else {
      base = ci->u.l.base;
      name = luaF_getlocalname(lua_ci_func(ci)->p, n, luai_currentpc(ci));
    }
  }
  else
    base = ci->func + 1;
  if (name == NULL) {  /* no 'standard' name? */
    luai_StkId limit = (ci == L->ci) ? L->top : ci->luai_next->func;
    if (limit - base >= n && n > 0)  /* is 'n' inside 'ci' stack? */
      name = "(*temporary)";  /* generic name for any valid slot */
    else
      return NULL;  /* no name */
  }
  *pos = base + (n - 1);
  return name;
}


LUA_API const char *lua_getlocal (lua_State *L, const lua_Debug *ar, int n) {
  const char *name;
  lua_lock(L);
  luai_swapextra(L);
  if (ar == NULL) {  /* information about non-active function? */
    if (!luai_isLfunction(L->top - 1))  /* not a Lua function? */
      name = NULL;
    else  /* consider live variables at function start (parameters) */
      name = luaF_getlocalname(luai_clLvalue(L->top - 1)->p, n, 0);
  }
  else {  /* active function; get information through 'ar' */
    luai_StkId pos = NULL;  /* to avoid warnings */
    name = luai_findlocal(L, ar->i_ci, n, &pos);
    if (name) {
      luai_setobj2s(L, L->top, pos);
      luai_api_incr_top(L);
    }
  }
  luai_swapextra(L);
  lua_unlock(L);
  return name;
}


LUA_API const char *lua_setlocal (lua_State *L, const lua_Debug *ar, int n) {
  luai_StkId pos = NULL;  /* to avoid warnings */
  const char *name;
  lua_lock(L);
  luai_swapextra(L);
  name = luai_findlocal(L, ar->i_ci, n, &pos);
  if (name) {
    luai_setobjs2s(L, pos, L->top - 1);
    L->top--;  /* pop value */
  }
  luai_swapextra(L);
  lua_unlock(L);
  return name;
}


static void luai_funcinfo (lua_Debug *ar, luai_Closure *cl) {
  if (lua_noLuaClosure(cl)) {
    ar->source = "=[C]";
    ar->linedefined = -1;
    ar->lastlinedefined = -1;
    ar->what = "C";
  }
  else {
    luai_Proto *p = cl->l.p;
    ar->source = p->source ? luai_getstr(p->source) : "=?";
    ar->linedefined = p->linedefined;
    ar->lastlinedefined = p->lastlinedefined;
    ar->what = (ar->linedefined == 0) ? "main" : "Lua";
  }
  luaO_chunkid(ar->short_src, ar->source, LUA_IDSIZE);
}


static void luai_collectvalidlines (lua_State *L, luai_Closure *f) {
  if (lua_noLuaClosure(f)) {
    luai_setnilvalue(L->top);
    luai_api_incr_top(L);
  }
  else {
    int i;
    luai_TValue v;
    int *lineinfo = f->l.p->lineinfo;
    luai_Table *t = luaH_new(L);  /* new table to store active lines */
    luai_sethvalue(L, L->top, t);  /* push it on stack */
    luai_api_incr_top(L);
    luai_setbvalue(&v, 1);  /* boolean 'true' to be the value of all indices */
    for (i = 0; i < f->l.p->sizelineinfo; i++)  /* for all lines with code */
      luaH_setint(L, t, lineinfo[i], &v);  /* table[line] = true */
  }
}


static const char *luai_getfuncname (lua_State *L, luai_CallInfo *ci, const char **name) {
  if (ci == NULL)  /* no 'ci'? */
    return NULL;  /* no info */
  else if (ci->callstatus & LUAI_CIST_FIN) {  /* is this a finalizer? */
    *name = "__gc";
    return "metamethod";  /* report it as such */
  }
  /* calling function is a known Lua function? */
  else if (!(ci->callstatus & LUAI_CIST_TAIL) && luai_isLua(ci->previous))
    return luai_funcnamefromcode(L, ci->previous, name);
  else return NULL;  /* no way to find a name */
}


static int luai_auxgetinfo (lua_State *L, const char *what, lua_Debug *ar,
                       luai_Closure *f, luai_CallInfo *ci) {
  int status = 1;
  for (; *what; what++) {
    switch (*what) {
      case 'S': {
        luai_funcinfo(ar, f);
        break;
      }
      case 'l': {
        ar->luai_currentline = (ci && luai_isLua(ci)) ? luai_currentline(ci) : -1;
        break;
      }
      case 'u': {
        ar->nups = (f == NULL) ? 0 : f->c.nupvalues;
        if (lua_noLuaClosure(f)) {
          ar->isvararg = 1;
          ar->nparams = 0;
        }
        else {
          ar->isvararg = f->l.p->is_vararg;
          ar->nparams = f->l.p->numparams;
        }
        break;
      }
      case 't': {
        ar->istailcall = (ci) ? ci->callstatus & LUAI_CIST_TAIL : 0;
        break;
      }
      case 'n': {
        ar->namewhat = luai_getfuncname(L, ci, &ar->name);
        if (ar->namewhat == NULL) {
          ar->namewhat = "";  /* not found */
          ar->name = NULL;
        }
        break;
      }
      case 'L':
      case 'f':  /* handled by lua_getinfo */
        break;
      default: status = 0;  /* invalid option */
    }
  }
  return status;
}


LUA_API int lua_getinfo (lua_State *L, const char *what, lua_Debug *ar) {
  int status;
  luai_Closure *cl;
  luai_CallInfo *ci;
  luai_StkId func;
  lua_lock(L);
  luai_swapextra(L);
  if (*what == '>') {
    ci = NULL;
    func = L->top - 1;
    luai_api_check(L, luai_ttisfunction(func), "function expected");
    what++;  /* skip the '>' */
    L->top--;  /* pop function */
  }
  else {
    ci = ar->i_ci;
    func = ci->func;
    lua_assert(luai_ttisfunction(ci->func));
  }
  cl = luai_ttisclosure(func) ? luai_clvalue(func) : NULL;
  status = luai_auxgetinfo(L, what, ar, cl, ci);
  if (strchr(what, 'f')) {
    luai_setobjs2s(L, L->top, func);
    luai_api_incr_top(L);
  }
  luai_swapextra(L);  /* correct before option 'L', which can raise a mem. error */
  if (strchr(what, 'L'))
    luai_collectvalidlines(L, cl);
  lua_unlock(L);
  return status;
}


/*
** {======================================================
** Symbolic Execution
** =======================================================
*/

static const char *luai_getobjname (luai_Proto *p, int lastpc, int reg,
                               const char **name);


/*
** find a "name" for the RK value 'c'
*/
static void luai_kname (luai_Proto *p, int pc, int c, const char **name) {
  if (luai_ISK(c)) {  /* is 'c' a constant? */
    luai_TValue *kvalue = &p->k[luai_INDEXK(c)];
    if (luai_ttisstring(kvalue)) {  /* literal constant? */
      *name = luai_svalue(kvalue);  /* it is its own name */
      return;
    }
    /* else no reasonable name found */
  }
  else {  /* 'c' is a register */
    const char *what = luai_getobjname(p, pc, c, name); /* search for 'c' */
    if (what && *what == 'c') {  /* found a constant name? */
      return;  /* 'name' already filled */
    }
    /* else no reasonable name found */
  }
  *name = "?";  /* no reasonable name found */
}


static int luai_filterpc (int pc, int jmptarget) {
  if (pc < jmptarget)  /* is code conditional (inside a jump)? */
    return -1;  /* cannot know who sets that register */
  else return pc;  /* current position sets that register */
}


/*
** try to find last instruction before 'lastpc' that modified register 'reg'
*/
static int luai_findsetreg (luai_Proto *p, int lastpc, int reg) {
  int pc;
  int setreg = -1;  /* keep last instruction that changed 'reg' */
  int jmptarget = 0;  /* any code before this address is conditional */
  for (pc = 0; pc < lastpc; pc++) {
    Instruction i = p->code[pc];
    luai_OpCode op = luai_GETOPCODE(i);
    int a = luai_GETARG_A(i);
    switch (op) {
      case luai_OP_LOADNIL: {
        int b = luai_GETARG_B(i);
        if (a <= reg && reg <= a + b)  /* set registers from 'a' to 'a+b' */
          setreg = luai_filterpc(pc, jmptarget);
        break;
      }
      case luai_OP_TFORCALL: {
        if (reg >= a + 2)  /* affect all regs above its base */
          setreg = luai_filterpc(pc, jmptarget);
        break;
      }
      case luai_OP_CALL:
      case luai_OP_TAILCALL: {
        if (reg >= a)  /* affect all registers above base */
          setreg = luai_filterpc(pc, jmptarget);
        break;
      }
      case luai_OP_JMP: {
        int b = luai_GETARG_sBx(i);
        int dest = pc + 1 + b;
        /* jump is forward and do not skip 'lastpc'? */
        if (pc < dest && dest <= lastpc) {
          if (dest > jmptarget)
            jmptarget = dest;  /* update 'jmptarget' */
        }
        break;
      }
      default:
        if (luai_testAMode(op) && reg == a)  /* any instruction that set A */
          setreg = luai_filterpc(pc, jmptarget);
        break;
    }
  }
  return setreg;
}


static const char *luai_getobjname (luai_Proto *p, int lastpc, int reg,
                               const char **name) {
  int pc;
  *name = luaF_getlocalname(p, reg + 1, lastpc);
  if (*name)  /* is a local? */
    return "local";
  /* else try symbolic execution */
  pc = luai_findsetreg(p, lastpc, reg);
  if (pc != -1) {  /* could find instruction? */
    Instruction i = p->code[pc];
    luai_OpCode op = luai_GETOPCODE(i);
    switch (op) {
      case luai_OP_MOVE: {
        int b = luai_GETARG_B(i);  /* move from 'b' to 'a' */
        if (b < luai_GETARG_A(i))
          return luai_getobjname(p, pc, b, name);  /* get name for 'b' */
        break;
      }
      case luai_OP_GETTABUP:
      case luai_OP_GETTABLE: {
        int k = luai_GETARG_C(i);  /* key index */
        int t = luai_GETARG_B(i);  /* table index */
        const char *vn = (op == luai_OP_GETTABLE)  /* name of indexed variable */
                         ? luaF_getlocalname(p, t + 1, pc)
                         : luai_upvalname(p, t);
        luai_kname(p, pc, k, name);
        return (vn && strcmp(vn, LUA_ENV) == 0) ? "global" : "field";
      }
      case luai_OP_GETUPVAL: {
        *name = luai_upvalname(p, luai_GETARG_B(i));
        return "upvalue";
      }
      case luai_OP_LOADK:
      case luai_OP_LOADKX: {
        int b = (op == luai_OP_LOADK) ? luai_GETARG_Bx(i)
                                 : luai_GETARG_Ax(p->code[pc + 1]);
        if (luai_ttisstring(&p->k[b])) {
          *name = luai_svalue(&p->k[b]);
          return "constant";
        }
        break;
      }
      case luai_OP_SELF: {
        int k = luai_GETARG_C(i);  /* key index */
        luai_kname(p, pc, k, name);
        return "method";
      }
      default: break;  /* go through to return NULL */
    }
  }
  return NULL;  /* could not find reasonable name */
}


/*
** Try to find a name for a function based on the code that called it.
** (Only works when function was called by a Lua function.)
** Returns what the name is (e.g., "for iterator", "method",
** "metamethod") and sets '*name' to point to the name.
*/
static const char *luai_funcnamefromcode (lua_State *L, luai_CallInfo *ci,
                                     const char **name) {
  luai_TMS tm = (luai_TMS)0;  /* (initial value avoids warnings) */
  luai_Proto *p = lua_ci_func(ci)->p;  /* calling function */
  int pc = luai_currentpc(ci);  /* calling instruction index */
  Instruction i = p->code[pc];  /* calling instruction */
  if (ci->callstatus & LUAI_CIST_HOOKED) {  /* was it called inside a hook? */
    *name = "?";
    return "hook";
  }
  switch (luai_GETOPCODE(i)) {
    case luai_OP_CALL:
    case luai_OP_TAILCALL:
      return luai_getobjname(p, pc, luai_GETARG_A(i), name);  /* get function name */
    case luai_OP_TFORCALL: {  /* for iterator */
      *name = "for iterator";
       return "for iterator";
    }
    /* other instructions can do calls through metamethods */
    case luai_OP_SELF: case luai_OP_GETTABUP: case luai_OP_GETTABLE:
      tm = LUAI_TM_INDEX;
      break;
    case luai_OP_SETTABUP: case luai_OP_SETTABLE:
      tm = LUAI_TM_NEWINDEX;
      break;
    case luai_OP_ADD: case luai_OP_SUB: case luai_OP_MUL: case luai_OP_MOD:
    case luai_OP_POW: case luai_OP_DIV: case luai_OP_IDIV: case luai_OP_BAND:
    case luai_OP_BOR: case luai_OP_BXOR: case luai_OP_SHL: case luai_OP_SHR: {
      int offset = luai_cast_int(luai_GETOPCODE(i)) - luai_cast_int(luai_OP_ADD);  /* ORDER OP */
      tm = luai_cast(luai_TMS, offset + luai_cast_int(LUAI_TM_ADD));  /* ORDER TM */
      break;
    }
    case luai_OP_UNM: tm = LUAI_TM_UNM; break;
    case luai_OP_BNOT: tm = LUAI_TM_BNOT; break;
    case luai_OP_LEN: tm = LUAI_TM_LEN; break;
    case luai_OP_CONCAT: tm = LUAI_TM_CONCAT; break;
    case luai_OP_EQ: tm = LUAI_TM_EQ; break;
    case luai_OP_LT: tm = LUAI_TM_LT; break;
    case luai_OP_LE: tm = LUAI_TM_LE; break;
    default:
      return NULL;  /* cannot find a reasonable name */
  }
  *name = luai_getstr(LUAI_G(L)->tmname[tm]);
  return "metamethod";
}

/* }====================================================== */



/*
** The subtraction of two potentially unrelated pointers is
** not ISO C, but it should not crash a program; the subsequent
** checks are ISO C and ensure a correct result.
*/
static int luai_isinstack (luai_CallInfo *ci, const luai_TValue *o) {
  ptrdiff_t i = o - ci->u.l.base;
  return (0 <= i && i < (ci->top - ci->u.l.base) && ci->u.l.base + i == o);
}


/*
** Checks whether value 'o' came from an upvalue. (That can only happen
** with instructions luai_OP_GETTABUP/luai_OP_SETTABUP, which operate directly on
** upvalues.)
*/
static const char *luai_getupvalname (luai_CallInfo *ci, const luai_TValue *o,
                                 const char **name) {
  luai_LClosure *c = lua_ci_func(ci);
  int i;
  for (i = 0; i < c->nupvalues; i++) {
    if (c->upvals[i]->v == o) {
      *name = luai_upvalname(c->p, i);
      return "upvalue";
    }
  }
  return NULL;
}


static const char *luai_varinfo (lua_State *L, const luai_TValue *o) {
  const char *name = NULL;  /* to avoid warnings */
  luai_CallInfo *ci = L->ci;
  const char *kind = NULL;
  if (luai_isLua(ci)) {
    kind = luai_getupvalname(ci, o, &name);  /* luai_check whether 'o' is an upvalue */
    if (!kind && luai_isinstack(ci, o))  /* no? try a register */
      kind = luai_getobjname(lua_ci_func(ci)->p, luai_currentpc(ci),
                        luai_cast_int(o - ci->u.l.base), &name);
  }
  return (kind) ? luaO_pushfstring(L, " (%s '%s')", kind, name) : "";
}


luai_l_noret luaG_typeerror (lua_State *L, const luai_TValue *o, const char *op) {
  const char *t = luaT_objtypename(L, o);
  luaG_runerror(L, "attempt to %s a %s value%s", op, t, luai_varinfo(L, o));
}


luai_l_noret luaG_concaterror (lua_State *L, const luai_TValue *p1, const luai_TValue *p2) {
  if (luai_ttisstring(p1) || luai_cvt2str(p1)) p1 = p2;
  luaG_typeerror(L, p1, "concatenate");
}


luai_l_noret luaG_opinterror (lua_State *L, const luai_TValue *p1,
                         const luai_TValue *p2, const char *msg) {
  lua_Number temp;
  if (!luai_tonumber(p1, &temp))  /* first operand is wrong? */
    p2 = p1;  /* now second is wrong */
  luaG_typeerror(L, p2, msg);
}


/*
** Error when both values are convertible to numbers, but not to integers
*/
luai_l_noret luaG_tointerror (lua_State *L, const luai_TValue *p1, const luai_TValue *p2) {
  lua_Integer temp;
  if (!luai_tointeger(p1, &temp))
    p2 = p1;
  luaG_runerror(L, "number%s has no integer representation", luai_varinfo(L, p2));
}


luai_l_noret luaG_ordererror (lua_State *L, const luai_TValue *p1, const luai_TValue *p2) {
  const char *t1 = luaT_objtypename(L, p1);
  const char *t2 = luaT_objtypename(L, p2);
  if (strcmp(t1, t2) == 0)
    luaG_runerror(L, "attempt to compare two %s values", t1);
  else
    luaG_runerror(L, "attempt to compare %s with %s", t1, t2);
}


/* add src:line information to 'msg' */
const char *luaG_addinfo (lua_State *L, const char *msg, luai_TString *src,
                                        int line) {
  char buff[LUA_IDSIZE];
  if (src)
    luaO_chunkid(buff, luai_getstr(src), LUA_IDSIZE);
  else {  /* no source available; use "?" instead */
    buff[0] = '?'; buff[1] = '\0';
  }
  return luaO_pushfstring(L, "%s:%d: %s", buff, line, msg);
}


luai_l_noret luaG_errormsg (lua_State *L) {
  if (L->errfunc != 0) {  /* is there an error handling function? */
    luai_StkId errfunc = luai_restorestack(L, L->errfunc);
    luai_setobjs2s(L, L->top, L->top - 1);  /* move argument */
    luai_setobjs2s(L, L->top - 1, errfunc);  /* push function */
    L->top++;  /* assume LUAI_EXTRA_STACK */
    luaD_callnoyield(L, L->top - 2, 1);  /* call it */
  }
  luaD_throw(L, LUA_ERRRUN);
}


luai_l_noret luaG_runerror (lua_State *L, const char *fmt, ...) {
  luai_CallInfo *ci = L->ci;
  const char *msg;
  va_list argp;
  va_start(argp, fmt);
  msg = luaO_pushvfstring(L, fmt, argp);  /* format message */
  va_end(argp);
  if (luai_isLua(ci))  /* if Lua function, add source:line information */
    luaG_addinfo(L, msg, lua_ci_func(ci)->p->source, luai_currentline(ci));
  luaG_errormsg(L);
}


void luaG_traceexec (lua_State *L) {
  luai_CallInfo *ci = L->ci;
  luai_lu_byte mask = L->hookmask;
  int counthook = (--L->hookcount == 0 && (mask & LUA_MASKCOUNT));
  if (counthook)
    luai_resethookcount(L);  /* reset count */
  else if (!(mask & LUA_MASKLINE))
    return;  /* no line hook and count != 0; nothing to be done */
  if (ci->callstatus & LUAI_CIST_HOOKYIELD) {  /* called hook last time? */
    ci->callstatus &= ~LUAI_CIST_HOOKYIELD;  /* erase mark */
    return;  /* do not call hook again (VM yielded, so it did not move) */
  }
  if (counthook)
    luaD_hook(L, LUA_HOOKCOUNT, -1);  /* call count hook */
  if (mask & LUA_MASKLINE) {
    luai_Proto *p = lua_ci_func(ci)->p;
    int npc = luai_pcRel(ci->u.l.savedpc, p);
    int newline = luai_getfuncline(p, npc);
    if (npc == 0 ||  /* call linehook when enter a new function, */
        ci->u.l.savedpc <= L->oldpc ||  /* when jump back (loop), or when */
        newline != luai_getfuncline(p, luai_pcRel(L->oldpc, p)))  /* enter a new line */
      luaD_hook(L, LUA_HOOKLINE, newline);  /* call line hook */
  }
  L->oldpc = ci->u.l.savedpc;
  if (L->status == LUA_YIELD) {  /* did hook yield? */
    if (counthook)
      L->hookcount = 1;  /* undo decrement to zero */
    ci->u.l.savedpc--;  /* undo increment (luai_resume will increment it again) */
    ci->callstatus |= LUAI_CIST_HOOKYIELD;  /* mark that it yielded */
    ci->func = L->top - 1;  /* protect stack below results */
    luaD_throw(L, LUA_YIELD);
  }
}

/*__ldo.c__*/

#define errorstatus(s)	((s) > LUA_YIELD)


/*
** {======================================================
** Error-recovery functions
** =======================================================
*/

/*
** LUAI_THROW/LUAI_TRY define how Lua does exception handling. By
** default, Lua handles errors with exceptions when compiling as
** C++ code, with _longjmp/_setjmp when asked to use them, and with
** longjmp/setjmp otherwise.
*/
#if !defined(LUAI_THROW)				/* { */

#if defined(__cplusplus) && !defined(LUA_USE_LONGJMP)	/* { */

/* C++ exceptions */
#define LUAI_THROW(L,c)		throw(c)
#define LUAI_TRY(L,c,a) \
	try { a } catch(...) { if ((c)->status == 0) (c)->status = -1; }
#define luai_jmpbuf		int  /* dummy variable */

#elif defined(LUA_USE_POSIX)				/* }{ */

/* in POSIX, try _longjmp/_setjmp (more efficient) */
#define LUAI_THROW(L,c)		_longjmp((c)->b, 1)
#define LUAI_TRY(L,c,a)		if (_setjmp((c)->b) == 0) { a }
#define luai_jmpbuf		jmp_buf

#else							/* }{ */

/* ISO C handling with long jumps */
#define LUAI_THROW(L,c)		longjmp((c)->b, 1)
#define LUAI_TRY(L,c,a)		if (setjmp((c)->b) == 0) { a }
#define luai_jmpbuf		jmp_buf

#endif							/* } */

#endif							/* } */



/* chain list of long jump buffers */
struct lua_longjmp {
  struct lua_longjmp *previous;
  luai_jmpbuf b;
  volatile int status;  /* error code */
};


static void luai_seterrorobj (lua_State *L, int errcode, luai_StkId oldtop) {
  switch (errcode) {
    case LUA_ERRMEM: {  /* memory error? */
      luai_setsvalue2s(L, oldtop, LUAI_G(L)->memerrmsg); /* reuse preregistered msg. */
      break;
    }
    case LUA_ERRERR: {
      luai_setsvalue2s(L, oldtop, luaS_newliteral(L, "error in error handling"));
      break;
    }
    default: {
      luai_setobjs2s(L, oldtop, L->top - 1);  /* error message on current top */
      break;
    }
  }
  L->top = oldtop + 1;
}


luai_l_noret luaD_throw (lua_State *L, int errcode) {
  if (L->errorJmp) {  /* thread has an error handler? */
    L->errorJmp->status = errcode;  /* set status */
    LUAI_THROW(L, L->errorJmp);  /* jump to it */
  }
  else {  /* thread has no error handler */
    luai_global_State *g = LUAI_G(L);
    L->status = luai_cast_byte(errcode);  /* mark it as dead */
    if (g->mainthread->errorJmp) {  /* main thread has a handler? */
      luai_setobjs2s(L, g->mainthread->top++, L->top - 1);  /* copy error obj. */
      luaD_throw(g->mainthread, errcode);  /* re-throw in main thread */
    }
    else {  /* no handler at all; abort */
      if (g->panic) {  /* panic function? */
        luai_seterrorobj(L, errcode, L->top);  /* assume LUAI_EXTRA_STACK */
        if (L->ci->top < L->top)
          L->ci->top = L->top;  /* pushing msg. can break this invariant */
        lua_unlock(L);
        g->panic(L);  /* call panic function (last chance to jump out) */
      }
      abort();
    }
  }
}


int luaD_rawrunprotected (lua_State *L, Pfunc f, void *ud) {
  unsigned short oldnCcalls = L->nCcalls;
  struct lua_longjmp lj;
  lj.status = LUA_OK;
  lj.previous = L->errorJmp;  /* chain new error handler */
  L->errorJmp = &lj;
  LUAI_TRY(L, &lj,
    (*f)(L, ud);
  );
  L->errorJmp = lj.previous;  /* restore old error handler */
  L->nCcalls = oldnCcalls;
  return lj.status;
}

/* }====================================================== */


/*
** {==================================================================
** Stack reallocation
** ===================================================================
*/
static void luai_correctstack (lua_State *L, luai_TValue *oldstack) {
  luai_CallInfo *ci;
  luai_UpVal *up;
  L->top = (L->top - oldstack) + L->stack;
  for (up = L->openupval; up != NULL; up = up->u.open.luai_next)
    up->v = (up->v - oldstack) + L->stack;
  for (ci = L->ci; ci != NULL; ci = ci->previous) {
    ci->top = (ci->top - oldstack) + L->stack;
    ci->func = (ci->func - oldstack) + L->stack;
    if (luai_isLua(ci))
      ci->u.l.base = (ci->u.l.base - oldstack) + L->stack;
  }
}


/* some space for error handling */
#define ERRORSTACKSIZE	(LUAI_MAXSTACK + 200)


void luaD_reallocstack (lua_State *L, int newsize) {
  luai_TValue *oldstack = L->stack;
  int lim = L->stacksize;
  lua_assert(newsize <= LUAI_MAXSTACK || newsize == ERRORSTACKSIZE);
  lua_assert(L->stack_last - L->stack == L->stacksize - LUAI_EXTRA_STACK);
  luaM_reallocvector(L, L->stack, L->stacksize, newsize, luai_TValue);
  for (; lim < newsize; lim++)
    luai_setnilvalue(L->stack + lim); /* erase new segment */
  L->stacksize = newsize;
  L->stack_last = L->stack + newsize - LUAI_EXTRA_STACK;
  luai_correctstack(L, oldstack);
}


void luaD_growstack (lua_State *L, int n) {
  int size = L->stacksize;
  if (size > LUAI_MAXSTACK)  /* error after extra size? */
    luaD_throw(L, LUA_ERRERR);
  else {
    int needed = luai_cast_int(L->top - L->stack) + n + LUAI_EXTRA_STACK;
    int newsize = 2 * size;
    if (newsize > LUAI_MAXSTACK) newsize = LUAI_MAXSTACK;
    if (newsize < needed) newsize = needed;
    if (newsize > LUAI_MAXSTACK) {  /* stack overflow? */
      luaD_reallocstack(L, ERRORSTACKSIZE);
      luaG_runerror(L, "stack overflow");
    }
    else
      luaD_reallocstack(L, newsize);
  }
}


static int luai_stackinuse (lua_State *L) {
  luai_CallInfo *ci;
  luai_StkId lim = L->top;
  for (ci = L->ci; ci != NULL; ci = ci->previous) {
    if (lim < ci->top) lim = ci->top;
  }
  lua_assert(lim <= L->stack_last);
  return luai_cast_int(lim - L->stack) + 1;  /* part of stack in use */
}


void luaD_shrinkstack (lua_State *L) {
  int inuse = luai_stackinuse(L);
  int goodsize = inuse + (inuse / 8) + 2*LUAI_EXTRA_STACK;
  if (goodsize > LUAI_MAXSTACK)
    goodsize = LUAI_MAXSTACK;  /* respect stack limit */
  if (L->stacksize > LUAI_MAXSTACK)  /* had been handling stack overflow? */
    luaE_freeCI(L);  /* free all CIs (list grew because of an error) */
  else
    luaE_shrinkCI(L);  /* shrink list */
  /* if thread is currently not handling a stack overflow and its
     good size is smaller than current size, shrink its stack */
  if (inuse <= (LUAI_MAXSTACK - LUAI_EXTRA_STACK) &&
      goodsize < L->stacksize)
    luaD_reallocstack(L, goodsize);
  else  /* don't change stack */
    luai_condmovestack(L,{},{});  /* (change only for debugging) */
}


void luaD_inctop (lua_State *L) {
  luaD_checkstack(L, 1);
  L->top++;
}

/* }================================================================== */


/*
** Call a hook for the given event. Make sure there is a hook to be
** called. (Both 'L->hook' and 'L->hookmask', which triggers this
** function, can be changed asynchronously by signals.)
*/
void luaD_hook (lua_State *L, int event, int line) {
  lua_Hook hook = L->hook;
  if (hook && L->allowhook) {  /* make sure there is a hook */
    luai_CallInfo *ci = L->ci;
    ptrdiff_t top = luai_savestack(L, L->top);
    ptrdiff_t ci_top = luai_savestack(L, ci->top);
    lua_Debug ar;
    ar.event = event;
    ar.luai_currentline = line;
    ar.i_ci = ci;
    luaD_checkstack(L, LUA_MINSTACK);  /* ensure minimum stack size */
    ci->top = L->top + LUA_MINSTACK;
    lua_assert(ci->top <= L->stack_last);
    L->allowhook = 0;  /* cannot call hooks inside a hook */
    ci->callstatus |= LUAI_CIST_HOOKED;
    lua_unlock(L);
    (*hook)(L, &ar);
    lua_lock(L);
    lua_assert(!L->allowhook);
    L->allowhook = 1;
    ci->top = luai_restorestack(L, ci_top);
    L->top = luai_restorestack(L, top);
    ci->callstatus &= ~LUAI_CIST_HOOKED;
  }
}


static void luai_callhook (lua_State *L, luai_CallInfo *ci) {
  int hook = LUA_HOOKCALL;
  ci->u.l.savedpc++;  /* hooks assume 'pc' is already incremented */
  if (luai_isLua(ci->previous) &&
      luai_GETOPCODE(*(ci->previous->u.l.savedpc - 1)) == luai_OP_TAILCALL) {
    ci->callstatus |= LUAI_CIST_TAIL;
    hook = LUA_HOOKTAILCALL;
  }
  luaD_hook(L, hook, -1);
  ci->u.l.savedpc--;  /* correct 'pc' */
}


static luai_StkId luai_adjust_varargs (lua_State *L, luai_Proto *p, int actual) {
  int i;
  int nfixargs = p->numparams;
  luai_StkId base, fixed;
  /* move fixed parameters to final position */
  fixed = L->top - actual;  /* first fixed argument */
  base = L->top;  /* final position of first argument */
  for (i = 0; i < nfixargs && i < actual; i++) {
    luai_setobjs2s(L, L->top++, fixed + i);
    luai_setnilvalue(fixed + i);  /* erase original copy (for LUAI_GC) */
  }
  for (; i < nfixargs; i++)
    luai_setnilvalue(L->top++);  /* complete missing arguments */
  return base;
}


/*
** Check whether __call metafield of 'func' is a function. If so, put
** it in stack below original 'func' so that 'luaD_precall' can call
** it. Raise an error if __call metafield is not a function.
*/
static void luai_tryfuncTM (lua_State *L, luai_StkId func) {
  const luai_TValue *tm = luaT_gettmbyobj(L, func, LUAI_TM_CALL);
  luai_StkId p;
  if (!luai_ttisfunction(tm))
    luaG_typeerror(L, func, "call");
  /* Open a hole inside the stack at 'func' */
  for (p = L->top; p > func; p--)
    luai_setobjs2s(L, p, p-1);
  L->top++;  /* slot ensured by caller */
  luai_setobj2s(L, func, tm);  /* tag method is the new function to be called */
}


/*
** Given 'nres' results at 'firstResult', move 'wanted' of them to 'res'.
** Handle most typical cases (zero results for commands, one result for
** expressions, multiple results for tail calls/single parameters)
** separated.
*/
static int luai_moveresults (lua_State *L, const luai_TValue *firstResult, luai_StkId res,
                                      int nres, int wanted) {
  switch (wanted) {  /* handle typical cases separately */
    case 0: break;  /* nothing to move */
    case 1: {  /* one result needed */
      if (nres == 0)   /* no results? */
        firstResult = luaO_nilobject;  /* adjust with nil */
      luai_setobjs2s(L, res, firstResult);  /* move it to proper place */
      break;
    }
    case LUA_MULTRET: {
      int i;
      for (i = 0; i < nres; i++)  /* move all results to correct place */
        luai_setobjs2s(L, res + i, firstResult + i);
      L->top = res + nres;
      return 0;  /* wanted == LUA_MULTRET */
    }
    default: {
      int i;
      if (wanted <= nres) {  /* enough results? */
        for (i = 0; i < wanted; i++)  /* move wanted results to correct place */
          luai_setobjs2s(L, res + i, firstResult + i);
      }
      else {  /* not enough results; use all of them plus nils */
        for (i = 0; i < nres; i++)  /* move all results to correct place */
          luai_setobjs2s(L, res + i, firstResult + i);
        for (; i < wanted; i++)  /* complete wanted number of results */
          luai_setnilvalue(res + i);
      }
      break;
    }
  }
  L->top = res + wanted;  /* top points after the last result */
  return 1;
}


/*
** Finishes a function call: calls hook if necessary, removes luai_CallInfo,
** moves current number of results to proper place; returns 0 iff call
** wanted multiple (variable number of) results.
*/
int luaD_poscall (lua_State *L, luai_CallInfo *ci, luai_StkId firstResult, int nres) {
  luai_StkId res;
  int wanted = ci->nresults;
  if (L->hookmask & (LUA_MASKRET | LUA_MASKLINE)) {
    if (L->hookmask & LUA_MASKRET) {
      ptrdiff_t fr = luai_savestack(L, firstResult);  /* hook may change stack */
      luaD_hook(L, LUA_HOOKRET, -1);
      firstResult = luai_restorestack(L, fr);
    }
    L->oldpc = ci->previous->u.l.savedpc;  /* 'oldpc' for caller function */
  }
  res = ci->func;  /* res == final position of 1st result */
  L->ci = ci->previous;  /* back to caller */
  /* move results to proper place */
  return luai_moveresults(L, firstResult, res, nres, wanted);
}



#define next_ci(L) (L->ci = (L->ci->luai_next ? L->ci->luai_next : luaE_extendCI(L)))


/* macro to luai_check stack size, preserving 'p' */
#define checkstackp(L,n,p)  \
  luaD_checkstackaux(L, n, \
    ptrdiff_t t__ = luai_savestack(L, p);  /* luai_save 'p' */ \
    luaC_checkGC(L),  /* stack grow uses memory */ \
    p = luai_restorestack(L, t__))  /* 'pos' part: restore 'p' */


/*
** Prepares a function call: checks the stack, creates a new luai_CallInfo
** entry, fills in the relevant information, calls hook if needed.
** If function is a C function, does the call, too. (Otherwise, leave
** the execution ('luaV_execute') to the caller, to allow stackless
** calls.) Returns true iff function has been executed (C function).
*/
int luaD_precall (lua_State *L, luai_StkId func, int nresults) {
  lua_CFunction f;
  luai_CallInfo *ci;
  switch (luai_ttype(func)) {
    case LUA_TCCL:  /* C closure */
      f = luai_clCvalue(func)->f;
      goto Cfunc;
    case LUA_TLCF:  /* light C function */
      f = luai_fvalue(func);
     Cfunc: {
      int n;  /* number of returns */
      checkstackp(L, LUA_MINSTACK, func);  /* ensure minimum stack size */
      ci = next_ci(L);  /* now 'enter' new function */
      ci->nresults = nresults;
      ci->func = func;
      ci->top = L->top + LUA_MINSTACK;
      lua_assert(ci->top <= L->stack_last);
      ci->callstatus = 0;
      if (L->hookmask & LUA_MASKCALL)
        luaD_hook(L, LUA_HOOKCALL, -1);
      lua_unlock(L);
      n = (*f)(L);  /* do the actual call */
      lua_lock(L);
      luai_api_checknelems(L, n);
      luaD_poscall(L, ci, L->top - n, n);
      return 1;
    }
    case LUA_TLCL: {  /* Lua function: prepare its call */
      luai_StkId base;
      luai_Proto *p = luai_clLvalue(func)->p;
      int n = luai_cast_int(L->top - func) - 1;  /* number of real arguments */
      int fsize = p->maxstacksize;  /* frame size */
      checkstackp(L, fsize, func);
      if (p->is_vararg)
        base = luai_adjust_varargs(L, p, n);
      else {  /* non vararg function */
        for (; n < p->numparams; n++)
          luai_setnilvalue(L->top++);  /* complete missing arguments */
        base = func + 1;
      }
      ci = next_ci(L);  /* now 'enter' new function */
      ci->nresults = nresults;
      ci->func = func;
      ci->u.l.base = base;
      L->top = ci->top = base + fsize;
      lua_assert(ci->top <= L->stack_last);
      ci->u.l.savedpc = p->code;  /* starting point */
      ci->callstatus = LUAI_CIST_LUA;
      if (L->hookmask & LUA_MASKCALL)
        luai_callhook(L, ci);
      return 0;
    }
    default: {  /* not a function */
      checkstackp(L, 1, func);  /* ensure space for metamethod */
      luai_tryfuncTM(L, func);  /* try to get '__call' metamethod */
      return luaD_precall(L, func, nresults);  /* now it must be a function */
    }
  }
}


/*
** Check appropriate error for stack overflow ("regular" overflow or
** overflow while handling stack overflow). If 'nCalls' is larger than
** LUAI_MAXCCALLS (which means it is handling a "regular" overflow) but
** smaller than 9/8 of LUAI_MAXCCALLS, does not report an error (to
** allow overflow handling to work)
*/
static void luai_stackerror (lua_State *L) {
  if (L->nCcalls == LUAI_MAXCCALLS)
    luaG_runerror(L, "C stack overflow");
  else if (L->nCcalls >= (LUAI_MAXCCALLS + (LUAI_MAXCCALLS>>3)))
    luaD_throw(L, LUA_ERRERR);  /* error while handing stack error */
}


/*
** Call a function (C or Lua). The function to be called is at *func.
** The arguments are on the stack, right after the function.
** When returns, all the results are on the stack, starting at the original
** function position.
*/
void luaD_call (lua_State *L, luai_StkId func, int nResults) {
  if (++L->nCcalls >= LUAI_MAXCCALLS)
    luai_stackerror(L);
  if (!luaD_precall(L, func, nResults))  /* is a Lua function? */
    luaV_execute(L);  /* call it */
  L->nCcalls--;
}


/*
** Similar to 'luaD_call', but does not allow yields during the call
*/
void luaD_callnoyield (lua_State *L, luai_StkId func, int nResults) {
  L->nny++;
  luaD_call(L, func, nResults);
  L->nny--;
}


/*
** Completes the execution of an interrupted C function, calling its
** continuation function.
*/
static void luai_finishCcall (lua_State *L, int status) {
  luai_CallInfo *ci = L->ci;
  int n;
  /* must have a continuation and must be able to call it */
  lua_assert(ci->u.c.k != NULL && L->nny == 0);
  /* error status can only happen in a protected call */
  lua_assert((ci->callstatus & LUAI_CIST_YPCALL) || status == LUA_YIELD);
  if (ci->callstatus & LUAI_CIST_YPCALL) {  /* was inside a pcall? */
    ci->callstatus &= ~LUAI_CIST_YPCALL;  /* continuation is also inside it */
    L->errfunc = ci->u.c.old_errfunc;  /* with the same error function */
  }
  /* finish 'lua_callk'/'lua_pcall'; LUAI_CIST_YPCALL and 'errfunc' already
     handled */
  luai_adjustresults(L, ci->nresults);
  lua_unlock(L);
  n = (*ci->u.c.k)(L, status, ci->u.c.ctx);  /* call continuation function */
  lua_lock(L);
  luai_api_checknelems(L, n);
  luaD_poscall(L, ci, L->top - n, n);  /* finish 'luaD_precall' */
}


/*
** Executes "full continuation" (everything in the stack) of a
** previously interrupted coroutine until the stack is empty (or another
** interruption long-jumps out of the loop). If the coroutine is
** recovering from an error, 'ud' points to the error status, which must
** be passed to the first continuation function (otherwise the default
** status is LUA_YIELD).
*/
static void luai_unroll (lua_State *L, void *ud) {
  if (ud != NULL)  /* error status? */
    luai_finishCcall(L, *(int *)ud);  /* finish 'lua_pcallk' callee */
  while (L->ci != &L->base_ci) {  /* something in the stack */
    if (!luai_isLua(L->ci))  /* C function? */
      luai_finishCcall(L, LUA_YIELD);  /* complete its execution */
    else {  /* Lua function */
      luaV_finishOp(L);  /* finish interrupted instruction */
      luaV_execute(L);  /* execute down to higher C 'boundary' */
    }
  }
}


/*
** Try to find a suspended protected call (a "recover point") for the
** given thread.
*/
static luai_CallInfo *luai_findpcall (lua_State *L) {
  luai_CallInfo *ci;
  for (ci = L->ci; ci != NULL; ci = ci->previous) {  /* search for a pcall */
    if (ci->callstatus & LUAI_CIST_YPCALL)
      return ci;
  }
  return NULL;  /* no pending pcall */
}


/*
** Recovers from an error in a coroutine. Finds a luai_recover point (if
** there is one) and completes the execution of the interrupted
** 'luaD_pcall'. If there is no luai_recover point, returns zero.
*/
static int luai_recover (lua_State *L, int status) {
  luai_StkId oldtop;
  luai_CallInfo *ci = luai_findpcall(L);
  if (ci == NULL) return 0;  /* no recovery point */
  /* "finish" luaD_pcall */
  oldtop = luai_restorestack(L, ci->extra);
  luaF_close(L, oldtop);
  luai_seterrorobj(L, status, oldtop);
  L->ci = ci;
  L->allowhook = luai_getoah(ci->callstatus);  /* restore original 'allowhook' */
  L->nny = 0;  /* should be zero to be yieldable */
  luaD_shrinkstack(L);
  L->errfunc = ci->u.c.old_errfunc;
  return 1;  /* continue running the coroutine */
}


/*
** Signal an error in the call to 'lua_resume', not in the execution
** of the coroutine itself. (Such errors should not be handled by any
** coroutine error handler and should not kill the coroutine.)
*/
static int luai_resume_error (lua_State *L, const char *msg, int narg) {
  L->top -= narg;  /* remove args from the stack */
  luai_setsvalue2s(L, L->top, luaS_new(L, msg));  /* push error message */
  luai_api_incr_top(L);
  lua_unlock(L);
  return LUA_ERRRUN;
}


/*
** Do the work for 'lua_resume' in protected mode. Most of the work
** depends on the status of the coroutine: initial state, suspended
** inside a hook, or regularly suspended (optionally with a continuation
** function), plus erroneous cases: non-suspended coroutine or dead
** coroutine.
*/
static void luai_resume (lua_State *L, void *ud) {
  int n = *(luai_cast(int*, ud));  /* number of arguments */
  luai_StkId firstArg = L->top - n;  /* first argument */
  luai_CallInfo *ci = L->ci;
  if (L->status == LUA_OK) {  /* starting a coroutine? */
    if (!luaD_precall(L, firstArg - 1, LUA_MULTRET))  /* Lua function? */
      luaV_execute(L);  /* call it */
  }
  else {  /* resuming from previous yield */
    lua_assert(L->status == LUA_YIELD);
    L->status = LUA_OK;  /* mark that it is running (again) */
    ci->func = luai_restorestack(L, ci->extra);
    if (luai_isLua(ci))  /* yielded inside a hook? */
      luaV_execute(L);  /* just continue running Lua code */
    else {  /* 'common' yield */
      if (ci->u.c.k != NULL) {  /* does it have a continuation function? */
        lua_unlock(L);
        n = (*ci->u.c.k)(L, LUA_YIELD, ci->u.c.ctx); /* call continuation */
        lua_lock(L);
        luai_api_checknelems(L, n);
        firstArg = L->top - n;  /* yield results come from continuation */
      }
      luaD_poscall(L, ci, firstArg, n);  /* finish 'luaD_precall' */
    }
    luai_unroll(L, NULL);  /* run continuation */
  }
}


LUA_API int lua_resume (lua_State *L, lua_State *from, int nargs) {
  int status;
  unsigned short oldnny = L->nny;  /* luai_save "number of non-yieldable" calls */
  lua_lock(L);
  if (L->status == LUA_OK) {  /* may be starting a coroutine */
    if (L->ci != &L->base_ci)  /* not in base level? */
      return luai_resume_error(L, "cannot luai_resume non-suspended coroutine", nargs);
  }
  else if (L->status != LUA_YIELD)
    return luai_resume_error(L, "cannot luai_resume dead coroutine", nargs);
  L->nCcalls = (from) ? from->nCcalls + 1 : 1;
  if (L->nCcalls >= LUAI_MAXCCALLS)
    return luai_resume_error(L, "C stack overflow", nargs);
  luai_userstateresume(L, nargs);
  L->nny = 0;  /* allow yields */
  luai_api_checknelems(L, (L->status == LUA_OK) ? nargs + 1 : nargs);
  status = luaD_rawrunprotected(L, luai_resume, &nargs);
  if (status == -1)  /* error calling 'lua_resume'? */
    status = LUA_ERRRUN;
  else {  /* continue running after recoverable errors */
    while (errorstatus(status) && luai_recover(L, status)) {
      /* luai_unroll continuation */
      status = luaD_rawrunprotected(L, luai_unroll, &status);
    }
    if (errorstatus(status)) {  /* unrecoverable error? */
      L->status = luai_cast_byte(status);  /* mark thread as 'dead' */
      luai_seterrorobj(L, status, L->top);  /* push error message */
      L->ci->top = L->top;
    }
    else lua_assert(status == L->status);  /* normal end or yield */
  }
  L->nny = oldnny;  /* restore 'nny' */
  L->nCcalls--;
  lua_assert(L->nCcalls == ((from) ? from->nCcalls : 0));
  lua_unlock(L);
  return status;
}


LUA_API int lua_isyieldable (lua_State *L) {
  return (L->nny == 0);
}


LUA_API int lua_yieldk (lua_State *L, int nresults, lua_KContext ctx,
                        lua_KFunction k) {
  luai_CallInfo *ci = L->ci;
  luai_userstateyield(L, nresults);
  lua_lock(L);
  luai_api_checknelems(L, nresults);
  if (L->nny > 0) {
    if (L != LUAI_G(L)->mainthread)
      luaG_runerror(L, "attempt to yield across a C-call boundary");
    else
      luaG_runerror(L, "attempt to yield from outside a coroutine");
  }
  L->status = LUA_YIELD;
  ci->extra = luai_savestack(L, ci->func);  /* luai_save current 'func' */
  if (luai_isLua(ci)) {  /* inside a hook? */
    luai_api_check(L, k == NULL, "hooks cannot continue after yielding");
  }
  else {
    if ((ci->u.c.k = k) != NULL)  /* is there a continuation? */
      ci->u.c.ctx = ctx;  /* luai_save context */
    ci->func = L->top - nresults - 1;  /* protect stack below results */
    luaD_throw(L, LUA_YIELD);
  }
  lua_assert(ci->callstatus & LUAI_CIST_HOOKED);  /* must be inside a hook */
  lua_unlock(L);
  return 0;  /* return to 'luaD_hook' */
}


int luaD_pcall (lua_State *L, Pfunc func, void *u,
                ptrdiff_t old_top, ptrdiff_t ef) {
  int status;
  luai_CallInfo *old_ci = L->ci;
  luai_lu_byte old_allowhooks = L->allowhook;
  unsigned short old_nny = L->nny;
  ptrdiff_t old_errfunc = L->errfunc;
  L->errfunc = ef;
  status = luaD_rawrunprotected(L, func, u);
  if (status != LUA_OK) {  /* an error occurred? */
    luai_StkId oldtop = luai_restorestack(L, old_top);
    luaF_close(L, oldtop);  /* close possible pending closures */
    luai_seterrorobj(L, status, oldtop);
    L->ci = old_ci;
    L->allowhook = old_allowhooks;
    L->nny = old_nny;
    luaD_shrinkstack(L);
  }
  L->errfunc = old_errfunc;
  return status;
}



/*
** Execute a protected parser.
*/
struct luai_SParser {  /* data to 'luai_f_parser' */
  LUAI_ZIO *z;
  luai_Mbuffer buff;  /* dynamic structure used by the scanner */
  luai_Dyndata dyd;  /* dynamic structures used by the parser */
  const char *mode;
  const char *name;
};


static void luai_checkmode (lua_State *L, const char *mode, const char *x) {
  if (mode && strchr(mode, x[0]) == NULL) {
    luaO_pushfstring(L,
       "attempt to load a %s chunk (mode is '%s')", x, mode);
    luaD_throw(L, LUA_ERRSYNTAX);
  }
}


static void luai_f_parser (lua_State *L, void *ud) {
  luai_LClosure *cl;
  struct luai_SParser *p = luai_cast(struct luai_SParser *, ud);
  int c = luai_zgetc(p->z);  /* read first character */
  if (c == LUA_SIGNATURE[0]) {
    luai_checkmode(L, p->mode, "binary");
    cl = luaU_undump(L, p->z, p->name);
  }
  else {
    luai_checkmode(L, p->mode, "text");
    cl = luaY_parser(L, p->z, &p->buff, &p->dyd, p->name, c);
  }
  lua_assert(cl->nupvalues == cl->p->sizeupvalues);
  luaF_initupvals(L, cl);
}


int luaD_protectedparser (lua_State *L, LUAI_ZIO *z, const char *name,
                                        const char *mode) {
  struct luai_SParser p;
  int status;
  L->nny++;  /* cannot yield during parsing */
  p.z = z; p.name = name; p.mode = mode;
  p.dyd.actvar.arr = NULL; p.dyd.actvar.size = 0;
  p.dyd.gt.arr = NULL; p.dyd.gt.size = 0;
  p.dyd.label.arr = NULL; p.dyd.label.size = 0;
  luaZ_initbuffer(L, &p.buff);
  status = luaD_pcall(L, luai_f_parser, &p, luai_savestack(L, L->top), L->errfunc);
  luaZ_freebuffer(L, &p.buff);
  luaM_freearray(L, p.dyd.actvar.arr, p.dyd.actvar.size);
  luaM_freearray(L, p.dyd.gt.arr, p.dyd.gt.size);
  luaM_freearray(L, p.dyd.label.arr, p.dyd.label.size);
  L->nny--;
  return status;
}

/*__ldump.c__*/

typedef struct {
  lua_State *L;
  lua_Writer luai_writer;
  void *data;
  int strip;
  int status;
} luai_DumpState;


/*
** All high-level dumps go through luai_DumpVector; you can change it to
** change the endianness of the result
*/
#define luai_DumpVector(v,n,D)	luai_DumpBlock(v,(n)*sizeof((v)[0]),D)

#define luai_DumpLiteral(s,D)	luai_DumpBlock(s, sizeof(s) - sizeof(char), D)


static void luai_DumpBlock (const void *b, size_t size, luai_DumpState *D) {
  if (D->status == 0 && size > 0) {
    lua_unlock(D->L);
    D->status = (*D->luai_writer)(D->L, b, size, D->data);
    lua_lock(D->L);
  }
}


#define luai_DumpVar(x,D)		luai_DumpVector(&x,1,D)


static void luai_DumpByte (int y, luai_DumpState *D) {
  luai_lu_byte x = (luai_lu_byte)y;
  luai_DumpVar(x, D);
}


static void luai_DumpInt (int x, luai_DumpState *D) {
  luai_DumpVar(x, D);
}


static void luai_DumpNumber (lua_Number x, luai_DumpState *D) {
  luai_DumpVar(x, D);
}


static void luai_DumpInteger (lua_Integer x, luai_DumpState *D) {
  luai_DumpVar(x, D);
}


static void luai_DumpString (const luai_TString *s, luai_DumpState *D) {
  if (s == NULL)
    luai_DumpByte(0, D);
  else {
    size_t size = luai_tsslen(s) + 1;  /* include trailing '\0' */
    const char *str = luai_getstr(s);
    if (size < 0xFF)
      luai_DumpByte(luai_cast_int(size), D);
    else {
      luai_DumpByte(0xFF, D);
      luai_DumpVar(size, D);
    }
    luai_DumpVector(str, size - 1, D);  /* no need to luai_save '\0' */
  }
}


static void luai_DumpCode (const luai_Proto *f, luai_DumpState *D) {
  luai_DumpInt(f->sizecode, D);
  luai_DumpVector(f->code, f->sizecode, D);
}


static void luai_DumpFunction(const luai_Proto *f, luai_TString *psource, luai_DumpState *D);

static void luai_DumpConstants (const luai_Proto *f, luai_DumpState *D) {
  int i;
  int n = f->sizek;
  luai_DumpInt(n, D);
  for (i = 0; i < n; i++) {
    const luai_TValue *o = &f->k[i];
    luai_DumpByte(luai_ttype(o), D);
    switch (luai_ttype(o)) {
    case LUA_TNIL:
      break;
    case LUA_TBOOLEAN:
      luai_DumpByte(luai_bvalue(o), D);
      break;
    case LUA_TNUMFLT:
      luai_DumpNumber(luai_fltvalue(o), D);
      break;
    case LUA_TNUMINT:
      luai_DumpInteger(luai_ivalue(o), D);
      break;
    case LUA_TSHRSTR:
    case LUA_TLNGSTR:
      luai_DumpString(luai_tsvalue(o), D);
      break;
    default:
      lua_assert(0);
    }
  }
}


static void luai_DumpProtos (const luai_Proto *f, luai_DumpState *D) {
  int i;
  int n = f->sizep;
  luai_DumpInt(n, D);
  for (i = 0; i < n; i++)
    luai_DumpFunction(f->p[i], f->source, D);
}


static void luai_DumpUpvalues (const luai_Proto *f, luai_DumpState *D) {
  int i, n = f->sizeupvalues;
  luai_DumpInt(n, D);
  for (i = 0; i < n; i++) {
    luai_DumpByte(f->upvalues[i].instack, D);
    luai_DumpByte(f->upvalues[i].idx, D);
  }
}


static void luai_DumpDebug (const luai_Proto *f, luai_DumpState *D) {
  int i, n;
  n = (D->strip) ? 0 : f->sizelineinfo;
  luai_DumpInt(n, D);
  luai_DumpVector(f->lineinfo, n, D);
  n = (D->strip) ? 0 : f->sizelocvars;
  luai_DumpInt(n, D);
  for (i = 0; i < n; i++) {
    luai_DumpString(f->locvars[i].varname, D);
    luai_DumpInt(f->locvars[i].startpc, D);
    luai_DumpInt(f->locvars[i].endpc, D);
  }
  n = (D->strip) ? 0 : f->sizeupvalues;
  luai_DumpInt(n, D);
  for (i = 0; i < n; i++)
    luai_DumpString(f->upvalues[i].name, D);
}


static void luai_DumpFunction (const luai_Proto *f, luai_TString *psource, luai_DumpState *D) {
  if (D->strip || f->source == psource)
    luai_DumpString(NULL, D);  /* no debug info or same source as its parent */
  else
    luai_DumpString(f->source, D);
  luai_DumpInt(f->linedefined, D);
  luai_DumpInt(f->lastlinedefined, D);
  luai_DumpByte(f->numparams, D);
  luai_DumpByte(f->is_vararg, D);
  luai_DumpByte(f->maxstacksize, D);
  luai_DumpCode(f, D);
  luai_DumpConstants(f, D);
  luai_DumpUpvalues(f, D);
  luai_DumpProtos(f, D);
  luai_DumpDebug(f, D);
}


static void luai_DumpHeader (luai_DumpState *D) {
  luai_DumpLiteral(LUA_SIGNATURE, D);
  luai_DumpByte(LUAC_VERSION, D);
  luai_DumpByte(LUAC_FORMAT, D);
  luai_DumpLiteral(LUAC_DATA, D);
  luai_DumpByte(sizeof(int), D);
  luai_DumpByte(sizeof(size_t), D);
  luai_DumpByte(sizeof(Instruction), D);
  luai_DumpByte(sizeof(lua_Integer), D);
  luai_DumpByte(sizeof(lua_Number), D);
  luai_DumpInteger(LUAC_INT, D);
  luai_DumpNumber(LUAC_NUM, D);
}


/*
** dump Lua function as precompiled chunk
*/
int luaU_dump(lua_State *L, const luai_Proto *f, lua_Writer w, void *data,
              int strip) {
  luai_DumpState D;
  D.L = L;
  D.luai_writer = w;
  D.data = data;
  D.strip = strip;
  D.status = 0;
  luai_DumpHeader(&D);
  luai_DumpByte(f->sizeupvalues, &D);
  luai_DumpFunction(f, NULL, &D);
  return D.status;
}

/*__lfunc.c__*/

luai_CClosure *luaF_newCclosure (lua_State *L, int n) {
  LUAI_GCObject *o = luaC_newobj(L, LUA_TCCL, luai_sizeCclosure(n));
  luai_CClosure *c = luai_gco2ccl(o);
  c->nupvalues = luai_cast_byte(n);
  return c;
}


luai_LClosure *luaF_newLclosure (lua_State *L, int n) {
  LUAI_GCObject *o = luaC_newobj(L, LUA_TLCL, luai_sizeLclosure(n));
  luai_LClosure *c = luai_gco2lcl(o);
  c->p = NULL;
  c->nupvalues = luai_cast_byte(n);
  while (n--) c->upvals[n] = NULL;
  return c;
}

/*
** fill a closure with new closed upvalues
*/
void luaF_initupvals (lua_State *L, luai_LClosure *cl) {
  int i;
  for (i = 0; i < cl->nupvalues; i++) {
    luai_UpVal *uv = luaM_new(L, luai_UpVal);
    uv->refcount = 1;
    uv->v = &uv->u.value;  /* make it closed */
    luai_setnilvalue(uv->v);
    cl->upvals[i] = uv;
  }
}


luai_UpVal *luaF_findupval (lua_State *L, luai_StkId level) {
  luai_UpVal **pp = &L->openupval;
  luai_UpVal *p;
  luai_UpVal *uv;
  lua_assert(luai_isintwups(L) || L->openupval == NULL);
  while (*pp != NULL && (p = *pp)->v >= level) {
    lua_assert(luai_upisopen(p));
    if (p->v == level)  /* found a corresponding upvalue? */
      return p;  /* return it */
    pp = &p->u.open.luai_next;
  }
  /* not found: create a new upvalue */
  uv = luaM_new(L, luai_UpVal);
  uv->refcount = 0;
  uv->u.open.luai_next = *pp;  /* link it to list of open upvalues */
  uv->u.open.touched = 1;
  *pp = uv;
  uv->v = level;  /* current value lives in the stack */
  if (!luai_isintwups(L)) {  /* thread not in list of threads with upvalues? */
    L->twups = LUAI_G(L)->twups;  /* link it to the list */
    LUAI_G(L)->twups = L;
  }
  return uv;
}


void luaF_close (lua_State *L, luai_StkId level) {
  luai_UpVal *uv;
  while (L->openupval != NULL && (uv = L->openupval)->v >= level) {
    lua_assert(luai_upisopen(uv));
    L->openupval = uv->u.open.luai_next;  /* remove from 'open' list */
    if (uv->refcount == 0)  /* no references? */
      luaM_free(L, uv);  /* free upvalue */
    else {
      luai_setobj(L, &uv->u.value, uv->v);  /* move value to upvalue slot */
      uv->v = &uv->u.value;  /* now current value lives here */
      luaC_upvalbarrier(L, uv);
    }
  }
}


luai_Proto *luaF_newproto (lua_State *L) {
  LUAI_GCObject *o = luaC_newobj(L, LUA_TPROTO, sizeof(luai_Proto));
  luai_Proto *f = luai_gco2p(o);
  f->k = NULL;
  f->sizek = 0;
  f->p = NULL;
  f->sizep = 0;
  f->code = NULL;
  f->cache = NULL;
  f->sizecode = 0;
  f->lineinfo = NULL;
  f->sizelineinfo = 0;
  f->upvalues = NULL;
  f->sizeupvalues = 0;
  f->numparams = 0;
  f->is_vararg = 0;
  f->maxstacksize = 0;
  f->locvars = NULL;
  f->sizelocvars = 0;
  f->linedefined = 0;
  f->lastlinedefined = 0;
  f->source = NULL;
  return f;
}


void luaF_freeproto (lua_State *L, luai_Proto *f) {
  luaM_freearray(L, f->code, f->sizecode);
  luaM_freearray(L, f->p, f->sizep);
  luaM_freearray(L, f->k, f->sizek);
  luaM_freearray(L, f->lineinfo, f->sizelineinfo);
  luaM_freearray(L, f->locvars, f->sizelocvars);
  luaM_freearray(L, f->upvalues, f->sizeupvalues);
  luaM_free(L, f);
}


/*
** Look for n-th local variable at line 'line' in function 'func'.
** Returns NULL if not found.
*/
const char *luaF_getlocalname (const luai_Proto *f, int local_number, int pc) {
  int i;
  for (i = 0; i<f->sizelocvars && f->locvars[i].startpc <= pc; i++) {
    if (pc < f->locvars[i].endpc) {  /* is variable active? */
      local_number--;
      if (local_number == 0)
        return luai_getstr(f->locvars[i].varname);
    }
  }
  return NULL;  /* not found */
}

/*__lgc.c__*/

/*
** internal state for collector while inside the luai_atomic phase. The
** collector should never be in this state while running regular code.
*/
#define LUAI_GCSinsideatomic		(LUAI_GCSpause + 1)

/*
** cost of sweeping one element (the size of a small object divided
** by some adjust for the sweep speed)
*/
#define LUAI_GCSWEEPCOST	((sizeof(luai_TString) + 4) / 4)

/* maximum number of elements to sweep in each single step */
#define LUAI_GCSWEEPMAX	(luai_cast_int((LUA_GCSTEPSIZE / LUAI_GCSWEEPCOST) / 4))

/* cost of calling one finalizer */
#define LUAI_GCFINALIZECOST	LUAI_GCSWEEPCOST


/*
** macro to adjust 'stepmul': 'stepmul' is actually used like
** 'stepmul / LUAI_STEPMULADJ' (value chosen by tests)
*/
#define LUAI_STEPMULADJ		200


/*
** macro to adjust 'pause': 'pause' is actually used like
** 'pause / LUAI_PAUSEADJ' (value chosen by tests)
*/
#define LUAI_PAUSEADJ		100


/*
** 'luai_makewhite' erases all color bits then sets only the current white
** bit
*/
#define luai_maskcolors	(~(luai_bitmask(LUAI_BLACKBIT) | LUAI_WHITEBITS))
#define luai_makewhite(g,x)	\
 (x->marked = luai_cast_byte((x->marked & luai_maskcolors) | luaC_white(g)))

#define luai_white2gray(x)	luai_resetbits(x->marked, LUAI_WHITEBITS)
#define luai_black2gray(x)	luai_resetbit(x->marked, LUAI_BLACKBIT)


#define luai_valiswhite(x)   (luai_iscollectable(x) && luai_iswhite(luai_gcvalue(x)))

#define luai_checkdeadkey(n)	lua_assert(!luai_ttisdeadkey(luai_gkey(n)) || luai_ttisnil(luai_gval(n)))


#define luai_checkconsistency(obj)  \
  lua_longassert(!luai_iscollectable(obj) || luai_righttt(obj))


#define luai_markvalue(g,o) { luai_checkconsistency(o); \
  if (luai_valiswhite(o)) luai_reallymarkobject(g,luai_gcvalue(o)); }

#define luai_markobject(g,t)	{ if (luai_iswhite(t)) luai_reallymarkobject(g, luai_obj2gco(t)); }

/*
** mark an object that can be NULL (either because it is really optional,
** or it was stripped as debug info, or inside an uncompleted structure)
*/
#define luai_markobjectN(g,t)	{ if (t) luai_markobject(g,t); }

static void luai_reallymarkobject (luai_global_State *g, LUAI_GCObject *o);


/*
** {======================================================
** Generic functions
** =======================================================
*/


/*
** one after last element in a hash array
*/
#define luai_gnodelast(h)	luai_gnode(h, luai_cast(size_t, luai_sizenode(h)))


/*
** link collectable object 'o' into list pointed by 'p'
*/
#define luai_linkgclist(o,p)	((o)->gclist = (p), (p) = luai_obj2gco(o))


/*
** If key is not marked, mark its entry as dead. This allows key to be
** collected, but keeps its entry in the table.  A dead node is needed
** when Lua looks up for a key (it may be part of a chain) and when
** traversing a weak table (key might be removed from the table during
** traversal). Other places never manipulate dead keys, because its
** associated nil value is enough to signal that the entry is logically
** empty.
*/
static void luai_removeentry (luai_Node *n) {
  lua_assert(luai_ttisnil(luai_gval(n)));
  if (luai_valiswhite(luai_gkey(n)))
    luai_setdeadvalue(luai_wgkey(n));  /* unused and unmarked key; remove it */
}


/*
** tells whether a key or value can be cleared from a weak
** table. Non-collectable objects are never removed from weak
** tables. Strings behave as 'values', so are never removed too. for
** other objects: if really collected, cannot keep them; for objects
** being finalized, keep them in keys, but not in values
*/
static int luai_iscleared (luai_global_State *g, const luai_TValue *o) {
  if (!luai_iscollectable(o)) return 0;
  else if (luai_ttisstring(o)) {
    luai_markobject(g, luai_tsvalue(o));  /* strings are 'values', so are never weak */
    return 0;
  }
  else return luai_iswhite(luai_gcvalue(o));
}


/*
** barrier that moves collector forward, that is, mark the white object
** being pointed by a black object. (If in sweep phase, clear the black
** object to white [sweep it] to avoid other barrier calls for this
** same object.)
*/
void luaC_barrier_ (lua_State *L, LUAI_GCObject *o, LUAI_GCObject *v) {
  luai_global_State *g = LUAI_G(L);
  lua_assert(luai_isblack(o) && luai_iswhite(v) && !luai_isdead(g, v) && !luai_isdead(g, o));
  if (luai_keepinvariant(g))  /* must keep invariant? */
    luai_reallymarkobject(g, v);  /* restore invariant */
  else {  /* sweep phase */
    lua_assert(luai_issweepphase(g));
    luai_makewhite(g, o);  /* mark main obj. as white to avoid other barriers */
  }
}


/*
** barrier that moves collector backward, that is, mark the black object
** pointing to a white object as gray again.
*/
void luaC_barrierback_ (lua_State *L, luai_Table *t) {
  luai_global_State *g = LUAI_G(L);
  lua_assert(luai_isblack(t) && !luai_isdead(g, t));
  luai_black2gray(t);  /* make table gray (again) */
  luai_linkgclist(t, g->grayagain);
}


/*
** barrier for assignments to closed upvalues. Because upvalues are
** shared among closures, it is impossible to know the color of all
** closures pointing to it. So, we assume that the object being assigned
** must be marked.
*/
void luaC_upvalbarrier_ (lua_State *L, luai_UpVal *uv) {
  luai_global_State *g = LUAI_G(L);
  LUAI_GCObject *o = luai_gcvalue(uv->v);
  lua_assert(!luai_upisopen(uv));  /* ensured by macro luaC_upvalbarrier */
  if (luai_keepinvariant(g))
    luai_markobject(g, o);
}


void luaC_fix (lua_State *L, LUAI_GCObject *o) {
  luai_global_State *g = LUAI_G(L);
  lua_assert(g->allgc == o);  /* object must be 1st in 'allgc' list! */
  luai_white2gray(o);  /* they will be gray forever */
  g->allgc = o->luai_next;  /* remove object from 'allgc' list */
  o->luai_next = g->fixedgc;  /* link it to 'fixedgc' list */
  g->fixedgc = o;
}


/*
** create a new collectable object (with given type and size) and link
** it to 'allgc' list.
*/
LUAI_GCObject *luaC_newobj (lua_State *L, int tt, size_t sz) {
  luai_global_State *g = LUAI_G(L);
  LUAI_GCObject *o = luai_cast(LUAI_GCObject *, luaM_newobject(L, luai_novariant(tt), sz));
  o->marked = luaC_white(g);
  o->tt = tt;
  o->luai_next = g->allgc;
  g->allgc = o;
  return o;
}

/* }====================================================== */



/*
** {======================================================
** Mark functions
** =======================================================
*/


/*
** mark an object. Userdata, strings, and closed upvalues are visited
** and turned black here. Other objects are marked gray and added
** to appropriate list to be visited (and turned black) later. (Open
** upvalues are already linked in 'headuv' list.)
*/
static void luai_reallymarkobject (luai_global_State *g, LUAI_GCObject *o) {
 reentry:
  luai_white2gray(o);
  switch (o->tt) {
    case LUA_TSHRSTR: {
      luai_gray2black(o);
      g->LUAI_GCmemtrav += luai_sizelstring(luai_gco2ts(o)->shrlen);
      break;
    }
    case LUA_TLNGSTR: {
      luai_gray2black(o);
      g->LUAI_GCmemtrav += luai_sizelstring(luai_gco2ts(o)->u.lnglen);
      break;
    }
    case LUA_TUSERDATA: {
      luai_TValue luai_uvalue;
      luai_markobjectN(g, luai_gco2u(o)->metatable);  /* mark its metatable */
      luai_gray2black(o);
      g->LUAI_GCmemtrav += luai_sizeudata(luai_gco2u(o));
      luai_getuservalue(g->mainthread, luai_gco2u(o), &luai_uvalue);
      if (luai_valiswhite(&luai_uvalue)) {  /* luai_markvalue(g, &luai_uvalue); */
        o = luai_gcvalue(&luai_uvalue);
        goto reentry;
      }
      break;
    }
    case LUA_TLCL: {
      luai_linkgclist(luai_gco2lcl(o), g->gray);
      break;
    }
    case LUA_TCCL: {
      luai_linkgclist(luai_gco2ccl(o), g->gray);
      break;
    }
    case LUA_TTABLE: {
      luai_linkgclist(luai_gco2t(o), g->gray);
      break;
    }
    case LUA_TTHREAD: {
      luai_linkgclist(luai_gco2th(o), g->gray);
      break;
    }
    case LUA_TPROTO: {
      luai_linkgclist(luai_gco2p(o), g->gray);
      break;
    }
    default: lua_assert(0); break;
  }
}


/*
** mark metamethods for basic types
*/
static void luai_markmt (luai_global_State *g) {
  int i;
  for (i=0; i < LUA_NUMTAGS; i++)
    luai_markobjectN(g, g->mt[i]);
}


/*
** mark all objects in list of being-finalized
*/
static void luai_markbeingfnz (luai_global_State *g) {
  LUAI_GCObject *o;
  for (o = g->tobefnz; o != NULL; o = o->luai_next)
    luai_markobject(g, o);
}


/*
** Mark all values stored in marked open upvalues from non-marked threads.
** (Values from marked threads were already marked when traversing the
** thread.) Remove from the list threads that no longer have upvalues and
** not-marked threads.
*/
static void luai_remarkupvals (luai_global_State *g) {
  lua_State *thread;
  lua_State **p = &g->twups;
  while ((thread = *p) != NULL) {
    lua_assert(!luai_isblack(thread));  /* threads are never black */
    if (luai_isgray(thread) && thread->openupval != NULL)
      p = &thread->twups;  /* keep marked thread with upvalues in the list */
    else {  /* thread is not marked or without upvalues */
      luai_UpVal *uv;
      *p = thread->twups;  /* remove thread from the list */
      thread->twups = thread;  /* mark that it is out of list */
      for (uv = thread->openupval; uv != NULL; uv = uv->u.open.luai_next) {
        if (uv->u.open.touched) {
          luai_markvalue(g, uv->v);  /* remark upvalue's value */
          uv->u.open.touched = 0;
        }
      }
    }
  }
}


/*
** mark root set and reset all gray lists, to start a new collection
*/
static void luai_restartcollection (luai_global_State *g) {
  g->gray = g->grayagain = NULL;
  g->weak = g->allweak = g->ephemeron = NULL;
  luai_markobject(g, g->mainthread);
  luai_markvalue(g, &g->luai_l_registry);
  luai_markmt(g);
  luai_markbeingfnz(g);  /* mark any finalizing object left from previous cycle */
}

/* }====================================================== */


/*
** {======================================================
** Traverse functions
** =======================================================
*/

/*
** Traverse a table with weak values and link it to proper list. During
** propagate phase, keep it in 'grayagain' list, to be revisited in the
** luai_atomic phase. In the luai_atomic phase, if table has any white value,
** put it in 'weak' list, to be cleared.
*/
static void luai_traverseweakvalue (luai_global_State *g, luai_Table *h) {
  luai_Node *n, *limit = luai_gnodelast(h);
  /* if there is array part, assume it may have white values (it is not
     worth traversing it now just to luai_check) */
  int hasclears = (h->sizearray > 0);
  for (n = luai_gnode(h, 0); n < limit; n++) {  /* traverse hash part */
    luai_checkdeadkey(n);
    if (luai_ttisnil(luai_gval(n)))  /* entry is empty? */
      luai_removeentry(n);  /* remove it */
    else {
      lua_assert(!luai_ttisnil(luai_gkey(n)));
      luai_markvalue(g, luai_gkey(n));  /* mark key */
      if (!hasclears && luai_iscleared(g, luai_gval(n)))  /* is there a white value? */
        hasclears = 1;  /* table will have to be cleared */
    }
  }
  if (g->gcstate == LUAI_GCSpropagate)
    luai_linkgclist(h, g->grayagain);  /* must retraverse it in luai_atomic phase */
  else if (hasclears)
    luai_linkgclist(h, g->weak);  /* has to be cleared later */
}


/*
** Traverse an ephemeron table and link it to proper list. Returns true
** iff any object was marked during this traversal (which implies that
** convergence has to continue). During propagation phase, keep table
** in 'grayagain' list, to be visited again in the luai_atomic phase. In
** the luai_atomic phase, if table has any white->white entry, it has to
** be revisited during ephemeron convergence (as that key may turn
** black). Otherwise, if it has any white key, table has to be cleared
** (in the luai_atomic phase).
*/
static int luai_traverseephemeron (luai_global_State *g, luai_Table *h) {
  int marked = 0;  /* true if an object is marked in this traversal */
  int hasclears = 0;  /* true if table has white keys */
  int hasww = 0;  /* true if table has entry "white-key -> white-value" */
  luai_Node *n, *limit = luai_gnodelast(h);
  unsigned int i;
  /* traverse array part */
  for (i = 0; i < h->sizearray; i++) {
    if (luai_valiswhite(&h->array[i])) {
      marked = 1;
      luai_reallymarkobject(g, luai_gcvalue(&h->array[i]));
    }
  }
  /* traverse hash part */
  for (n = luai_gnode(h, 0); n < limit; n++) {
    luai_checkdeadkey(n);
    if (luai_ttisnil(luai_gval(n)))  /* entry is empty? */
      luai_removeentry(n);  /* remove it */
    else if (luai_iscleared(g, luai_gkey(n))) {  /* key is not marked (yet)? */
      hasclears = 1;  /* table must be cleared */
      if (luai_valiswhite(luai_gval(n)))  /* value not marked yet? */
        hasww = 1;  /* white-white entry */
    }
    else if (luai_valiswhite(luai_gval(n))) {  /* value not marked yet? */
      marked = 1;
      luai_reallymarkobject(g, luai_gcvalue(luai_gval(n)));  /* mark it now */
    }
  }
  /* link table into proper list */
  if (g->gcstate == LUAI_GCSpropagate)
    luai_linkgclist(h, g->grayagain);  /* must retraverse it in luai_atomic phase */
  else if (hasww)  /* table has white->white entries? */
    luai_linkgclist(h, g->ephemeron);  /* have to propagate again */
  else if (hasclears)  /* table has white keys? */
    luai_linkgclist(h, g->allweak);  /* may have to clean white keys */
  return marked;
}


static void luai_traversestrongtable (luai_global_State *g, luai_Table *h) {
  luai_Node *n, *limit = luai_gnodelast(h);
  unsigned int i;
  for (i = 0; i < h->sizearray; i++)  /* traverse array part */
    luai_markvalue(g, &h->array[i]);
  for (n = luai_gnode(h, 0); n < limit; n++) {  /* traverse hash part */
    luai_checkdeadkey(n);
    if (luai_ttisnil(luai_gval(n)))  /* entry is empty? */
      luai_removeentry(n);  /* remove it */
    else {
      lua_assert(!luai_ttisnil(luai_gkey(n)));
      luai_markvalue(g, luai_gkey(n));  /* mark key */
      luai_markvalue(g, luai_gval(n));  /* mark value */
    }
  }
}


static luai_lu_mem luai_traversetable (luai_global_State *g, luai_Table *h) {
  const char *weakkey, *weakvalue;
  const luai_TValue *mode = luai_gfasttm(g, h->metatable, LUAI_TM_MODE);
  luai_markobjectN(g, h->metatable);
  if (mode && luai_ttisstring(mode) &&  /* is there a weak mode? */
      ((weakkey = strchr(luai_svalue(mode), 'k')),
       (weakvalue = strchr(luai_svalue(mode), 'v')),
       (weakkey || weakvalue))) {  /* is really weak? */
    luai_black2gray(h);  /* keep table gray */
    if (!weakkey)  /* strong keys? */
      luai_traverseweakvalue(g, h);
    else if (!weakvalue)  /* strong values? */
      luai_traverseephemeron(g, h);
    else  /* all weak */
      luai_linkgclist(h, g->allweak);  /* nothing to traverse now */
  }
  else  /* not weak */
    luai_traversestrongtable(g, h);
  return sizeof(luai_Table) + sizeof(luai_TValue) * h->sizearray +
                         sizeof(luai_Node) * luai_cast(size_t, luai_allocsizenode(h));
}


/*
** Traverse a prototype. (While a prototype is being build, its
** arrays can be larger than needed; the extra slots are filled with
** NULL, so the use of 'luai_markobjectN')
*/
static int luai_traverseproto (luai_global_State *g, luai_Proto *f) {
  int i;
  if (f->cache && luai_iswhite(f->cache))
    f->cache = NULL;  /* allow cache to be collected */
  luai_markobjectN(g, f->source);
  for (i = 0; i < f->sizek; i++)  /* mark literals */
    luai_markvalue(g, &f->k[i]);
  for (i = 0; i < f->sizeupvalues; i++)  /* mark upvalue names */
    luai_markobjectN(g, f->upvalues[i].name);
  for (i = 0; i < f->sizep; i++)  /* mark nested protos */
    luai_markobjectN(g, f->p[i]);
  for (i = 0; i < f->sizelocvars; i++)  /* mark local-variable names */
    luai_markobjectN(g, f->locvars[i].varname);
  return sizeof(luai_Proto) + sizeof(Instruction) * f->sizecode +
                         sizeof(luai_Proto *) * f->sizep +
                         sizeof(luai_TValue) * f->sizek +
                         sizeof(int) * f->sizelineinfo +
                         sizeof(luai_LocVar) * f->sizelocvars +
                         sizeof(luai_Upvaldesc) * f->sizeupvalues;
}


static luai_lu_mem luai_traverseCclosure (luai_global_State *g, luai_CClosure *cl) {
  int i;
  for (i = 0; i < cl->nupvalues; i++)  /* mark its upvalues */
    luai_markvalue(g, &cl->upvalue[i]);
  return luai_sizeCclosure(cl->nupvalues);
}

/*
** open upvalues point to values in a thread, so those values should
** be marked when the thread is traversed except in the luai_atomic phase
** (because then the value cannot be changed by the thread and the
** thread may not be traversed again)
*/
static luai_lu_mem luai_traverseLclosure (luai_global_State *g, luai_LClosure *cl) {
  int i;
  luai_markobjectN(g, cl->p);  /* mark its prototype */
  for (i = 0; i < cl->nupvalues; i++) {  /* mark its upvalues */
    luai_UpVal *uv = cl->upvals[i];
    if (uv != NULL) {
      if (luai_upisopen(uv) && g->gcstate != LUAI_GCSinsideatomic)
        uv->u.open.touched = 1;  /* can be marked in 'luai_remarkupvals' */
      else
        luai_markvalue(g, uv->v);
    }
  }
  return luai_sizeLclosure(cl->nupvalues);
}


static luai_lu_mem luai_traversethread (luai_global_State *g, lua_State *th) {
  luai_StkId o = th->stack;
  if (o == NULL)
    return 1;  /* stack not completely built yet */
  lua_assert(g->gcstate == LUAI_GCSinsideatomic ||
             th->openupval == NULL || luai_isintwups(th));
  for (; o < th->top; o++)  /* mark live elements in the stack */
    luai_markvalue(g, o);
  if (g->gcstate == LUAI_GCSinsideatomic) {  /* final traversal? */
    luai_StkId lim = th->stack + th->stacksize;  /* real end of stack */
    for (; o < lim; o++)  /* clear not-marked stack slice */
      luai_setnilvalue(o);
    /* 'luai_remarkupvals' may have removed thread from 'twups' list */
    if (!luai_isintwups(th) && th->openupval != NULL) {
      th->twups = g->twups;  /* link it back to the list */
      g->twups = th;
    }
  }
  else if (g->gckind != LUAI_KGC_EMERGENCY)
    luaD_shrinkstack(th); /* do not change stack in emergency cycle */
  return (sizeof(lua_State) + sizeof(luai_TValue) * th->stacksize +
          sizeof(luai_CallInfo) * th->nci);
}


/*
** traverse one gray object, turning it to black (except for threads,
** which are always gray).
*/
static void luai_propagatemark (luai_global_State *g) {
  luai_lu_mem size;
  LUAI_GCObject *o = g->gray;
  lua_assert(luai_isgray(o));
  luai_gray2black(o);
  switch (o->tt) {
    case LUA_TTABLE: {
      luai_Table *h = luai_gco2t(o);
      g->gray = h->gclist;  /* remove from 'gray' list */
      size = luai_traversetable(g, h);
      break;
    }
    case LUA_TLCL: {
      luai_LClosure *cl = luai_gco2lcl(o);
      g->gray = cl->gclist;  /* remove from 'gray' list */
      size = luai_traverseLclosure(g, cl);
      break;
    }
    case LUA_TCCL: {
      luai_CClosure *cl = luai_gco2ccl(o);
      g->gray = cl->gclist;  /* remove from 'gray' list */
      size = luai_traverseCclosure(g, cl);
      break;
    }
    case LUA_TTHREAD: {
      lua_State *th = luai_gco2th(o);
      g->gray = th->gclist;  /* remove from 'gray' list */
      luai_linkgclist(th, g->grayagain);  /* insert into 'grayagain' list */
      luai_black2gray(o);
      size = luai_traversethread(g, th);
      break;
    }
    case LUA_TPROTO: {
      luai_Proto *p = luai_gco2p(o);
      g->gray = p->gclist;  /* remove from 'gray' list */
      size = luai_traverseproto(g, p);
      break;
    }
    default: lua_assert(0); return;
  }
  g->LUAI_GCmemtrav += size;
}


static void luai_propagateall (luai_global_State *g) {
  while (g->gray) luai_propagatemark(g);
}


static void luai_convergeephemerons (luai_global_State *g) {
  int changed;
  do {
    LUAI_GCObject *w;
    LUAI_GCObject *luai_next = g->ephemeron;  /* get ephemeron list */
    g->ephemeron = NULL;  /* tables may return to this list when traversed */
    changed = 0;
    while ((w = luai_next) != NULL) {
      luai_next = luai_gco2t(w)->gclist;
      if (luai_traverseephemeron(g, luai_gco2t(w))) {  /* traverse marked some value? */
        luai_propagateall(g);  /* propagate changes */
        changed = 1;  /* will have to revisit all ephemeron tables */
      }
    }
  } while (changed);
}

/* }====================================================== */


/*
** {======================================================
** Sweep Functions
** =======================================================
*/


/*
** clear entries with unmarked keys from all weaktables in list 'l' up
** to element 'f'
*/
static void luai_clearkeys (luai_global_State *g, LUAI_GCObject *l, LUAI_GCObject *f) {
  for (; l != f; l = luai_gco2t(l)->gclist) {
    luai_Table *h = luai_gco2t(l);
    luai_Node *n, *limit = luai_gnodelast(h);
    for (n = luai_gnode(h, 0); n < limit; n++) {
      if (!luai_ttisnil(luai_gval(n)) && (luai_iscleared(g, luai_gkey(n)))) {
        luai_setnilvalue(luai_gval(n));  /* remove value ... */
        luai_removeentry(n);  /* and remove entry from table */
      }
    }
  }
}


/*
** clear entries with unmarked values from all weaktables in list 'l' up
** to element 'f'
*/
static void luai_clearvalues (luai_global_State *g, LUAI_GCObject *l, LUAI_GCObject *f) {
  for (; l != f; l = luai_gco2t(l)->gclist) {
    luai_Table *h = luai_gco2t(l);
    luai_Node *n, *limit = luai_gnodelast(h);
    unsigned int i;
    for (i = 0; i < h->sizearray; i++) {
      luai_TValue *o = &h->array[i];
      if (luai_iscleared(g, o))  /* value was collected? */
        luai_setnilvalue(o);  /* remove value */
    }
    for (n = luai_gnode(h, 0); n < limit; n++) {
      if (!luai_ttisnil(luai_gval(n)) && luai_iscleared(g, luai_gval(n))) {
        luai_setnilvalue(luai_gval(n));  /* remove value ... */
        luai_removeentry(n);  /* and remove entry from table */
      }
    }
  }
}


void luaC_upvdeccount (lua_State *L, luai_UpVal *uv) {
  lua_assert(uv->refcount > 0);
  uv->refcount--;
  if (uv->refcount == 0 && !luai_upisopen(uv))
    luaM_free(L, uv);
}


static void luai_freeLclosure (lua_State *L, luai_LClosure *cl) {
  int i;
  for (i = 0; i < cl->nupvalues; i++) {
    luai_UpVal *uv = cl->upvals[i];
    if (uv)
      luaC_upvdeccount(L, uv);
  }
  luaM_freemem(L, cl, luai_sizeLclosure(cl->nupvalues));
}


static void luai_freeobj (lua_State *L, LUAI_GCObject *o) {
  switch (o->tt) {
    case LUA_TPROTO: luaF_freeproto(L, luai_gco2p(o)); break;
    case LUA_TLCL: {
      luai_freeLclosure(L, luai_gco2lcl(o));
      break;
    }
    case LUA_TCCL: {
      luaM_freemem(L, o, luai_sizeCclosure(luai_gco2ccl(o)->nupvalues));
      break;
    }
    case LUA_TTABLE: luaH_free(L, luai_gco2t(o)); break;
    case LUA_TTHREAD: luaE_freethread(L, luai_gco2th(o)); break;
    case LUA_TUSERDATA: luaM_freemem(L, o, luai_sizeudata(luai_gco2u(o))); break;
    case LUA_TSHRSTR:
      luaS_remove(L, luai_gco2ts(o));  /* remove it from hash table */
      luaM_freemem(L, o, luai_sizelstring(luai_gco2ts(o)->shrlen));
      break;
    case LUA_TLNGSTR: {
      luaM_freemem(L, o, luai_sizelstring(luai_gco2ts(o)->u.lnglen));
      break;
    }
    default: lua_assert(0);
  }
}


#define luai_sweepwholelist(L,p)	luai_sweeplist(L,p,LUAI_MAX_LUMEM)
static LUAI_GCObject **luai_sweeplist (lua_State *L, LUAI_GCObject **p, luai_lu_mem count);


/*
** sweep at most 'count' elements from a list of LUAI_GCObjects erasing dead
** objects, where a dead object is one marked with the old (non current)
** white; change all non-dead objects back to white, preparing for luai_next
** collection cycle. Return where to continue the traversal or NULL if
** list is finished.
*/
static LUAI_GCObject **luai_sweeplist (lua_State *L, LUAI_GCObject **p, luai_lu_mem count) {
  luai_global_State *g = LUAI_G(L);
  int ow = luai_otherwhite(g);
  int white = luaC_white(g);  /* current white */
  while (*p != NULL && count-- > 0) {
    LUAI_GCObject *curr = *p;
    int marked = curr->marked;
    if (luai_isdeadm(ow, marked)) {  /* is 'curr' dead? */
      *p = curr->luai_next;  /* remove 'curr' from list */
      luai_freeobj(L, curr);  /* erase 'curr' */
    }
    else {  /* change mark to 'white' */
      curr->marked = luai_cast_byte((marked & luai_maskcolors) | white);
      p = &curr->luai_next;  /* go to luai_next element */
    }
  }
  return (*p == NULL) ? NULL : p;
}


/*
** sweep a list until a live object (or end of list)
*/
static LUAI_GCObject **luai_sweeptolive (lua_State *L, LUAI_GCObject **p) {
  LUAI_GCObject **old = p;
  do {
    p = luai_sweeplist(L, p, 1);
  } while (p == old);
  return p;
}

/* }====================================================== */


/*
** {======================================================
** Finalization
** =======================================================
*/

/*
** If possible, shrink string table
*/
static void luai_checkSizes (lua_State *L, luai_global_State *g) {
  if (g->gckind != LUAI_KGC_EMERGENCY) {
    luai_l_mem olddebt = g->LUAI_GCdebt;
    if (g->strt.nuse < g->strt.size / 4)  /* string table too big? */
      luaS_resize(L, g->strt.size / 2);  /* shrink it a little */
    g->LUAI_GCestimate += g->LUAI_GCdebt - olddebt;  /* update estimate */
  }
}


static LUAI_GCObject *luai_udata2finalize (luai_global_State *g) {
  LUAI_GCObject *o = g->tobefnz;  /* get first element */
  lua_assert(luai_tofinalize(o));
  g->tobefnz = o->luai_next;  /* remove it from 'tobefnz' list */
  o->luai_next = g->allgc;  /* return it to 'allgc' list */
  g->allgc = o;
  luai_resetbit(o->marked, LUAI_FINALIZEDBIT);  /* object is "normal" again */
  if (luai_issweepphase(g))
    luai_makewhite(g, o);  /* "sweep" object */
  return o;
}


static void luai_dothecall (lua_State *L, void *ud) {
  LUAI_UNUSED(ud);
  luaD_callnoyield(L, L->top - 2, 0);
}


static void LUAI_GCTM (lua_State *L, int propagateerrors) {
  luai_global_State *g = LUAI_G(L);
  const luai_TValue *tm;
  luai_TValue v;
  luai_setgcovalue(L, &v, luai_udata2finalize(g));
  tm = luaT_gettmbyobj(L, &v, LUAI_TM_GC);
  if (tm != NULL && luai_ttisfunction(tm)) {  /* is there a finalizer? */
    int status;
    luai_lu_byte oldah = L->allowhook;
    int running  = g->gcrunning;
    L->allowhook = 0;  /* stop debug hooks during LUAI_GC metamethod */
    g->gcrunning = 0;  /* avoid LUAI_GC steps */
    luai_setobj2s(L, L->top, tm);  /* push finalizer... */
    luai_setobj2s(L, L->top + 1, &v);  /* ... and its argument */
    L->top += 2;  /* and (luai_next line) call the finalizer */
    L->ci->callstatus |= LUAI_CIST_FIN;  /* will run a finalizer */
    status = luaD_pcall(L, luai_dothecall, NULL, luai_savestack(L, L->top - 2), 0);
    L->ci->callstatus &= ~LUAI_CIST_FIN;  /* not running a finalizer anymore */
    L->allowhook = oldah;  /* restore hooks */
    g->gcrunning = running;  /* restore state */
    if (status != LUA_OK && propagateerrors) {  /* error while running __gc? */
      if (status == LUA_ERRRUN) {  /* is there an error object? */
        const char *msg = (luai_ttisstring(L->top - 1))
                            ? luai_svalue(L->top - 1)
                            : "no message";
        luaO_pushfstring(L, "error in __gc metamethod (%s)", msg);
        status = LUA_ERRGCMM;  /* error in __gc metamethod */
      }
      luaD_throw(L, status);  /* re-throw error */
    }
  }
}


/*
** call a few (up to 'g->gcfinnum') finalizers
*/
static int luai_runafewfinalizers (lua_State *L) {
  luai_global_State *g = LUAI_G(L);
  unsigned int i;
  lua_assert(!g->tobefnz || g->gcfinnum > 0);
  for (i = 0; g->tobefnz && i < g->gcfinnum; i++)
    LUAI_GCTM(L, 1);  /* call one finalizer */
  g->gcfinnum = (!g->tobefnz) ? 0  /* nothing more to finalize? */
                    : g->gcfinnum * 2;  /* else call a few more luai_next time */
  return i;
}


/*
** call all pending finalizers
*/
static void luai_callallpendingfinalizers (lua_State *L) {
  luai_global_State *g = LUAI_G(L);
  while (g->tobefnz)
    LUAI_GCTM(L, 0);
}


/*
** find last 'luai_next' luai_field in list 'p' list (to add elements in its end)
*/
static LUAI_GCObject **luai_findlast (LUAI_GCObject **p) {
  while (*p != NULL)
    p = &(*p)->luai_next;
  return p;
}


/*
** move all unreachable objects (or 'all' objects) that need
** finalization from list 'finobj' to list 'tobefnz' (to be finalized)
*/
static void luai_separatetobefnz (luai_global_State *g, int all) {
  LUAI_GCObject *curr;
  LUAI_GCObject **p = &g->finobj;
  LUAI_GCObject **lastnext = luai_findlast(&g->tobefnz);
  while ((curr = *p) != NULL) {  /* traverse all finalizable objects */
    lua_assert(luai_tofinalize(curr));
    if (!(luai_iswhite(curr) || all))  /* not being collected? */
      p = &curr->luai_next;  /* don't bother with it */
    else {
      *p = curr->luai_next;  /* remove 'curr' from 'finobj' list */
      curr->luai_next = *lastnext;  /* link at the end of 'tobefnz' list */
      *lastnext = curr;
      lastnext = &curr->luai_next;
    }
  }
}


/*
** if object 'o' has a finalizer, remove it from 'allgc' list (must
** search the list to find it) and link it in 'finobj' list.
*/
void luaC_checkfinalizer (lua_State *L, LUAI_GCObject *o, luai_Table *mt) {
  luai_global_State *g = LUAI_G(L);
  if (luai_tofinalize(o) ||                 /* obj. is already marked... */
      luai_gfasttm(g, mt, LUAI_TM_GC) == NULL)   /* or has no finalizer? */
    return;  /* nothing to be done */
  else {  /* move 'o' to 'finobj' list */
    LUAI_GCObject **p;
    if (luai_issweepphase(g)) {
      luai_makewhite(g, o);  /* "sweep" object 'o' */
      if (g->sweepgc == &o->luai_next)  /* should not remove 'sweepgc' object */
        g->sweepgc = luai_sweeptolive(L, g->sweepgc);  /* change 'sweepgc' */
    }
    /* search for pointer pointing to 'o' */
    for (p = &g->allgc; *p != o; p = &(*p)->luai_next) { /* empty */ }
    *p = o->luai_next;  /* remove 'o' from 'allgc' list */
    o->luai_next = g->finobj;  /* link it in 'finobj' list */
    g->finobj = o;
    luai_l_setbit(o->marked, LUAI_FINALIZEDBIT);  /* mark it as such */
  }
}

/* }====================================================== */



/*
** {======================================================
** LUAI_GC control
** =======================================================
*/


/*
** Set a reasonable "time" to wait before starting a new LUAI_GC cycle; cycle
** will start when memory use hits threshold. (Division by 'estimate'
** should be OK: it cannot be zero (because Lua cannot even start with
** less than LUAI_PAUSEADJ bytes).
*/
static void luai_setpause (luai_global_State *g) {
  luai_l_mem threshold, debt;
  luai_l_mem estimate = g->LUAI_GCestimate / LUAI_PAUSEADJ;  /* adjust 'estimate' */
  lua_assert(estimate > 0);
  threshold = (g->gcpause < LUAI_MAX_LMEM / estimate)  /* overflow? */
            ? estimate * g->gcpause  /* no overflow */
            : LUAI_MAX_LMEM;  /* overflow; truncate to maximum */
  debt = luai_gettotalbytes(g) - threshold;
  luaE_setdebt(g, debt);
}


/*
** Enter first sweep phase.
** The call to 'luai_sweeplist' tries to make pointer point to an object
** inside the list (instead of to the header), so that the real sweep do
** not need to skip objects created between "now" and the start of the
** real sweep.
*/
static void luai_entersweep (lua_State *L) {
  luai_global_State *g = LUAI_G(L);
  g->gcstate = LUAI_GCSswpallgc;
  lua_assert(g->sweepgc == NULL);
  g->sweepgc = luai_sweeplist(L, &g->allgc, 1);
}


void luaC_freeallobjects (lua_State *L) {
  luai_global_State *g = LUAI_G(L);
  luai_separatetobefnz(g, 1);  /* separate all objects with finalizers */
  lua_assert(g->finobj == NULL);
  luai_callallpendingfinalizers(L);
  lua_assert(g->tobefnz == NULL);
  g->currentwhite = LUAI_WHITEBITS; /* this "white" makes all objects look dead */
  g->gckind = LUAI_KGC_NORMAL;
  luai_sweepwholelist(L, &g->finobj);
  luai_sweepwholelist(L, &g->allgc);
  luai_sweepwholelist(L, &g->fixedgc);  /* collect fixed objects */
  lua_assert(g->strt.nuse == 0);
}


static luai_l_mem luai_atomic (lua_State *L) {
  luai_global_State *g = LUAI_G(L);
  luai_l_mem work;
  LUAI_GCObject *origweak, *origall;
  LUAI_GCObject *grayagain = g->grayagain;  /* luai_save original list */
  lua_assert(g->ephemeron == NULL && g->weak == NULL);
  lua_assert(!luai_iswhite(g->mainthread));
  g->gcstate = LUAI_GCSinsideatomic;
  g->LUAI_GCmemtrav = 0;  /* start counting work */
  luai_markobject(g, L);  /* mark running thread */
  /* registry and global metatables may be changed by API */
  luai_markvalue(g, &g->luai_l_registry);
  luai_markmt(g);  /* mark global metatables */
  /* remark occasional upvalues of (maybe) dead threads */
  luai_remarkupvals(g);
  luai_propagateall(g);  /* propagate changes */
  work = g->LUAI_GCmemtrav;  /* stop counting (do not recount 'grayagain') */
  g->gray = grayagain;
  luai_propagateall(g);  /* traverse 'grayagain' list */
  g->LUAI_GCmemtrav = 0;  /* restart counting */
  luai_convergeephemerons(g);
  /* at this point, all strongly accessible objects are marked. */
  /* Clear values from weak tables, before checking finalizers */
  luai_clearvalues(g, g->weak, NULL);
  luai_clearvalues(g, g->allweak, NULL);
  origweak = g->weak; origall = g->allweak;
  work += g->LUAI_GCmemtrav;  /* stop counting (objects being finalized) */
  luai_separatetobefnz(g, 0);  /* separate objects to be finalized */
  g->gcfinnum = 1;  /* there may be objects to be finalized */
  luai_markbeingfnz(g);  /* mark objects that will be finalized */
  luai_propagateall(g);  /* remark, to propagate 'resurrection' */
  g->LUAI_GCmemtrav = 0;  /* restart counting */
  luai_convergeephemerons(g);
  /* at this point, all resurrected objects are marked. */
  /* remove dead objects from weak tables */
  luai_clearkeys(g, g->ephemeron, NULL);  /* clear keys from all ephemeron tables */
  luai_clearkeys(g, g->allweak, NULL);  /* clear keys from all 'allweak' tables */
  /* clear values from resurrected weak tables */
  luai_clearvalues(g, g->weak, origweak);
  luai_clearvalues(g, g->allweak, origall);
  luaS_clearcache(g);
  g->currentwhite = luai_cast_byte(luai_otherwhite(g));  /* flip current white */
  work += g->LUAI_GCmemtrav;  /* complete counting */
  return work;  /* estimate of memory marked by 'luai_atomic' */
}


static luai_lu_mem luai_sweepstep (lua_State *L, luai_global_State *g,
                         int nextstate, LUAI_GCObject **nextlist) {
  if (g->sweepgc) {
    luai_l_mem olddebt = g->LUAI_GCdebt;
    g->sweepgc = luai_sweeplist(L, g->sweepgc, LUAI_GCSWEEPMAX);
    g->LUAI_GCestimate += g->LUAI_GCdebt - olddebt;  /* update estimate */
    if (g->sweepgc)  /* is there still something to sweep? */
      return (LUAI_GCSWEEPMAX * LUAI_GCSWEEPCOST);
  }
  /* else enter luai_next state */
  g->gcstate = nextstate;
  g->sweepgc = nextlist;
  return 0;
}


static luai_lu_mem luai_singlestep (lua_State *L) {
  luai_global_State *g = LUAI_G(L);
  switch (g->gcstate) {
    case LUAI_GCSpause: {
      g->LUAI_GCmemtrav = g->strt.size * sizeof(LUAI_GCObject*);
      luai_restartcollection(g);
      g->gcstate = LUAI_GCSpropagate;
      return g->LUAI_GCmemtrav;
    }
    case LUAI_GCSpropagate: {
      g->LUAI_GCmemtrav = 0;
      lua_assert(g->gray);
      luai_propagatemark(g);
       if (g->gray == NULL)  /* no more gray objects? */
        g->gcstate = LUAI_GCSatomic;  /* finish propagate phase */
      return g->LUAI_GCmemtrav;  /* memory traversed in this step */
    }
    case LUAI_GCSatomic: {
      luai_lu_mem work;
      luai_propagateall(g);  /* make sure gray list is empty */
      work = luai_atomic(L);  /* work is what was traversed by 'luai_atomic' */
      luai_entersweep(L);
      g->LUAI_GCestimate = luai_gettotalbytes(g);  /* first estimate */;
      return work;
    }
    case LUAI_GCSswpallgc: {  /* sweep "regular" objects */
      return luai_sweepstep(L, g, LUAI_GCSswpfinobj, &g->finobj);
    }
    case LUAI_GCSswpfinobj: {  /* sweep objects with finalizers */
      return luai_sweepstep(L, g, LUAI_GCSswptobefnz, &g->tobefnz);
    }
    case LUAI_GCSswptobefnz: {  /* sweep objects to be finalized */
      return luai_sweepstep(L, g, LUAI_GCSswpend, NULL);
    }
    case LUAI_GCSswpend: {  /* finish sweeps */
      luai_makewhite(g, g->mainthread);  /* sweep main thread */
      luai_checkSizes(L, g);
      g->gcstate = LUAI_GCScallfin;
      return 0;
    }
    case LUAI_GCScallfin: {  /* call remaining finalizers */
      if (g->tobefnz && g->gckind != LUAI_KGC_EMERGENCY) {
        int n = luai_runafewfinalizers(L);
        return (n * LUAI_GCFINALIZECOST);
      }
      else {  /* emergency mode or no more finalizers */
        g->gcstate = LUAI_GCSpause;  /* finish collection */
        return 0;
      }
    }
    default: lua_assert(0); return 0;
  }
}


/*
** advances the garbage collector until it reaches a state allowed
** by 'statemask'
*/
void luaC_runtilstate (lua_State *L, int statesmask) {
  luai_global_State *g = LUAI_G(L);
  while (!luai_testbit(statesmask, g->gcstate))
    luai_singlestep(L);
}


/*
** get LUAI_GC debt and convert it from Kb to 'work units' (avoid zero debt
** and overflows)
*/
static luai_l_mem luai_getdebt (luai_global_State *g) {
  luai_l_mem debt = g->LUAI_GCdebt;
  int stepmul = g->gcstepmul;
  if (debt <= 0) return 0;  /* minimal debt */
  else {
    debt = (debt / LUAI_STEPMULADJ) + 1;
    debt = (debt < LUAI_MAX_LMEM / stepmul) ? debt * stepmul : LUAI_MAX_LMEM;
    return debt;
  }
}

/*
** performs a basic LUAI_GC step when collector is running
*/
void luaC_step (lua_State *L) {
  luai_global_State *g = LUAI_G(L);
  luai_l_mem debt = luai_getdebt(g);  /* LUAI_GC deficit (be paid now) */
  if (!g->gcrunning) {  /* not running? */
    luaE_setdebt(g, -LUA_GCSTEPSIZE * 10);  /* avoid being called too often */
    return;
  }
  do {  /* repeat until pause or enough "credit" (negative debt) */
    luai_lu_mem work = luai_singlestep(L);  /* perform one single step */
    debt -= work;
  } while (debt > -LUA_GCSTEPSIZE && g->gcstate != LUAI_GCSpause);
  if (g->gcstate == LUAI_GCSpause)
    luai_setpause(g);  /* pause until luai_next cycle */
  else {
    debt = (debt / g->gcstepmul) * LUAI_STEPMULADJ;  /* convert 'work units' to Kb */
    luaE_setdebt(g, debt);
    luai_runafewfinalizers(L);
  }
}


/*
** Performs a full LUAI_GC cycle; if 'isemergency', set a flag to avoid
** some operations which could change the interpreter state in some
** unexpected ways (running finalizers and shrinking some structures).
** Before running the collection, luai_check 'luai_keepinvariant'; if it is true,
** there may be some objects marked as black, so the collector has
** to sweep all objects to turn them back to white (as white has not
** changed, nothing will be collected).
*/
void luaC_fullgc (lua_State *L, int isemergency) {
  luai_global_State *g = LUAI_G(L);
  lua_assert(g->gckind == LUAI_KGC_NORMAL);
  if (isemergency) g->gckind = LUAI_KGC_EMERGENCY;  /* set flag */
  if (luai_keepinvariant(g)) {  /* black objects? */
    luai_entersweep(L); /* sweep everything to turn them back to white */
  }
  /* finish any pending sweep phase to start a new cycle */
  luaC_runtilstate(L, luai_bitmask(LUAI_GCSpause));
  luaC_runtilstate(L, ~luai_bitmask(LUAI_GCSpause));  /* start new collection */
  luaC_runtilstate(L, luai_bitmask(LUAI_GCScallfin));  /* run up to finalizers */
  /* estimate must be correct after a full LUAI_GC cycle */
  lua_assert(g->LUAI_GCestimate == luai_gettotalbytes(g));
  luaC_runtilstate(L, luai_bitmask(LUAI_GCSpause));  /* finish collection */
  g->gckind = LUAI_KGC_NORMAL;
  luai_setpause(g);
}

/* }====================================================== */

/*__linit.c__*/

/*
** these libs are loaded by lua.c and are readily available to any Lua
** program
*/
static const luaL_Reg luai_loadedlibs[] = {
  {"_G", luaopen_base},
  {LUA_LOADLIBNAME, luaopen_package},
  {LUA_COLIBNAME, luaopen_coroutine},
  {LUA_TABLIBNAME, luaopen_table},
  {LUA_IOLIBNAME, luaopen_io},
  {LUA_OSLIBNAME, luaopen_os},
  {LUA_STRLIBNAME, luaopen_string},
  {LUA_MATHLIBNAME, luaopen_math},
  {LUA_UTF8LIBNAME, luaopen_utf8},
  {LUA_DBLIBNAME, luaopen_debug},
#if defined(LUA_COMPAT_BITLIB)
  {LUA_BITLIBNAME, luaopen_bit32},
#endif
  {NULL, NULL}
};


LUALIB_API void luaL_openlibs (lua_State *L) {
  const luaL_Reg *lib;
  /* "require" functions from 'luai_loadedlibs' and set results to global table */
  for (lib = luai_loadedlibs; lib->func; lib++) {
    luaL_requiref(L, lib->name, lib->func, 1);
    lua_pop(L, 1);  /* remove lib */
  }
}

/*__liolib.c__*/

/*
** Change this macro to accept other modes for 'fopen' besides
** the standard ones.
*/
#if !defined(luai_l_checkmode)

/* accepted extensions to 'mode' in 'fopen' */
#if !defined(LUAI_L_MODEEXT)
#define LUAI_L_MODEEXT	"b"
#endif

/* Check whether 'mode' matches '[rwa]%+?[LUAI_L_MODEEXT]*' */
static int luai_l_checkmode (const char *mode) {
  return (*mode != '\0' && strchr("rwa", *(mode++)) != NULL &&
         (*mode != '+' || (++mode, 1)) &&  /* skip if char is '+' */
         (strspn(mode, LUAI_L_MODEEXT) == strlen(mode)));  /* luai_check extensions */
}

#endif

/*
** {======================================================
** luai_l_popen spawns a new process connected to the current
** one through the file streams.
** =======================================================
*/

#if !defined(luai_l_popen)		/* { */

#if defined(LUA_USE_POSIX)	/* { */

#define luai_l_popen(L,c,m)		(fflush(NULL), popen(c,m))
#define luai_l_pclose(L,file)	(pclose(file))

#elif defined(LUA_USE_WINDOWS)	/* }{ */

#define luai_l_popen(L,c,m)		(_popen(c,m))
#define luai_l_pclose(L,file)	(_pclose(file))

#else				/* }{ */

/* ISO C definitions */
#define luai_l_popen(L,c,m)  \
	  ((void)((void)c, m), \
	  luaL_error(L, "'popen' not supported"), \
	  (FILE*)0)
#define luai_l_pclose(L,file)		((void)L, (void)file, -1)

#endif				/* } */

#endif				/* } */

/* }====================================================== */


#if !defined(luai_l_getc)		/* { */

#if defined(LUA_USE_POSIX)
#define luai_l_getc(f)		getc_unlocked(f)
#define luai_l_lockfile(f)		flockfile(f)
#define luai_l_unlockfile(f)		funlockfile(f)
#else
#define luai_l_getc(f)		getc(f)
#define luai_l_lockfile(f)		((void)0)
#define luai_l_unlockfile(f)		((void)0)
#endif

#endif				/* } */


/*
** {======================================================
** luai_l_fseek: configuration for longer offsets
** =======================================================
*/

#if !defined(luai_l_fseek)		/* { */

#if defined(LUA_USE_POSIX)	/* { */

#include <sys/types.h>

#define luai_l_fseek(f,o,w)		fseeko(f,o,w)
#define luai_l_ftell(f)		ftello(f)
#define luai_l_seeknum		off_t

#elif defined(LUA_USE_WINDOWS) && !defined(_CRTIMP_TYPEINFO) \
   && defined(_MSC_VER) && (_MSC_VER >= 1400)	/* }{ */

/* Windows (but not DDK) and Visual C++ 2005 or higher */
#define luai_l_fseek(f,o,w)		_fseeki64(f,o,w)
#define luai_l_ftell(f)		_ftelli64(f)
#define luai_l_seeknum		__int64

#else				/* }{ */

/* ISO C definitions */
#define luai_l_fseek(f,o,w)		fseek(f,o,w)
#define luai_l_ftell(f)		ftell(f)
#define luai_l_seeknum		long

#endif				/* } */

#endif				/* } */

/* }====================================================== */


#define LUAI_IO_PREFIX	"_IO_"
#define LUAI_IOPREF_LEN	(sizeof(LUAI_IO_PREFIX)/sizeof(char) - 1)
#define LUAI_IO_INPUT	(LUAI_IO_PREFIX "input")
#define LUAI_IO_OUTPUT	(LUAI_IO_PREFIX "output")


typedef luaL_Stream luai_LStream;


#define luai_tolstream(L)	((luai_LStream *)luaL_checkudata(L, 1, LUA_FILEHANDLE))

#define luai_isclosed(p)	((p)->closef == NULL)


static int luai_io_type (lua_State *L) {
  luai_LStream *p;
  luaL_checkany(L, 1);
  p = (luai_LStream *)luaL_testudata(L, 1, LUA_FILEHANDLE);
  if (p == NULL)
    lua_pushnil(L);  /* not a file */
  else if (luai_isclosed(p))
    lua_pushliteral(L, "closed file");
  else
    lua_pushliteral(L, "file");
  return 1;
}


static int luai_f_tostring (lua_State *L) {
  luai_LStream *p = luai_tolstream(L);
  if (luai_isclosed(p))
    lua_pushliteral(L, "file (closed)");
  else
    lua_pushfstring(L, "file (%p)", p->f);
  return 1;
}


static FILE *luai_tofile (lua_State *L) {
  luai_LStream *p = luai_tolstream(L);
  if (luai_isclosed(p))
    luaL_error(L, "attempt to use a closed file");
  lua_assert(p->f);
  return p->f;
}


/*
** When creating file handles, always creates a 'closed' file handle
** before opening the actual file; so, if there is a memory error, the
** handle is in a consistent state.
*/
static luai_LStream *luai_toprefile (lua_State *L) {
  luai_LStream *p = (luai_LStream *)lua_newuserdata(L, sizeof(luai_LStream));
  p->closef = NULL;  /* mark file handle as 'closed' */
  luaL_setmetatable(L, LUA_FILEHANDLE);
  return p;
}


/*
** Calls the 'close' function from a file handle. The 'volatile' avoids
** a bug in some versions of the Clang compiler (e.g., clang 3.0 for
** 32 bits).
*/
static int luai_aux_close (lua_State *L) {
  luai_LStream *p = luai_tolstream(L);
  volatile lua_CFunction cf = p->closef;
  p->closef = NULL;  /* mark stream as closed */
  return (*cf)(L);  /* close it */
}


static int luai_io_close (lua_State *L) {
  if (lua_isnone(L, 1))  /* no argument? */
    lua_getfield(L, LUA_REGISTRYINDEX, LUAI_IO_OUTPUT);  /* use standard output */
  luai_tofile(L);  /* make sure argument is an open stream */
  return luai_aux_close(L);
}


static int luai_f_gc (lua_State *L) {
  luai_LStream *p = luai_tolstream(L);
  if (!luai_isclosed(p) && p->f != NULL)
    luai_aux_close(L);  /* ignore closed and incompletely open files */
  return 0;
}


/*
** function to close regular files
*/
static int luai_io_fclose (lua_State *L) {
  luai_LStream *p = luai_tolstream(L);
  int res = fclose(p->f);
  return luaL_fileresult(L, (res == 0), NULL);
}


static luai_LStream *luai_newfile (lua_State *L) {
  luai_LStream *p = luai_toprefile(L);
  p->f = NULL;
  p->closef = &luai_io_fclose;
  return p;
}


static void luai_opencheck (lua_State *L, const char *fname, const char *mode) {
  luai_LStream *p = luai_newfile(L);
  p->f = fopen(fname, mode);
  if (p->f == NULL)
    luaL_error(L, "cannot open file '%s' (%s)", fname, strerror(errno));
}


static int luai_io_open (lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  const char *mode = luaL_optstring(L, 2, "r");
  luai_LStream *p = luai_newfile(L);
  const char *md = mode;  /* to traverse/luai_check mode */
  luaL_argcheck(L, luai_l_checkmode(md), 2, "invalid mode");
  p->f = fopen(filename, mode);
  return (p->f == NULL) ? luaL_fileresult(L, 0, filename) : 1;
}


/*
** function to close 'popen' files
*/
static int luai_io_pclose (lua_State *L) {
  luai_LStream *p = luai_tolstream(L);
  return luaL_execresult(L, luai_l_pclose(L, p->f));
}


static int luai_io_popen (lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  const char *mode = luaL_optstring(L, 2, "r");
  luai_LStream *p = luai_toprefile(L);
  p->f = luai_l_popen(L, filename, mode);
  p->closef = &luai_io_pclose;
  return (p->f == NULL) ? luaL_fileresult(L, 0, filename) : 1;
}


static int luai_io_tmpfile (lua_State *L) {
  luai_LStream *p = luai_newfile(L);
  p->f = tmpfile();
  return (p->f == NULL) ? luaL_fileresult(L, 0, NULL) : 1;
}


static FILE *luai_getiofile (lua_State *L, const char *findex) {
  luai_LStream *p;
  lua_getfield(L, LUA_REGISTRYINDEX, findex);
  p = (luai_LStream *)lua_touserdata(L, -1);
  if (luai_isclosed(p))
    luaL_error(L, "standard %s file is closed", findex + LUAI_IOPREF_LEN);
  return p->f;
}


static int luai_g_iofile (lua_State *L, const char *f, const char *mode) {
  if (!lua_isnoneornil(L, 1)) {
    const char *filename = lua_tostring(L, 1);
    if (filename)
      luai_opencheck(L, filename, mode);
    else {
      luai_tofile(L);  /* luai_check that it's a valid file handle */
      lua_pushvalue(L, 1);
    }
    lua_setfield(L, LUA_REGISTRYINDEX, f);
  }
  /* return current value */
  lua_getfield(L, LUA_REGISTRYINDEX, f);
  return 1;
}


static int luai_io_input (lua_State *L) {
  return luai_g_iofile(L, LUAI_IO_INPUT, "r");
}


static int luai_io_output (lua_State *L) {
  return luai_g_iofile(L, LUAI_IO_OUTPUT, "w");
}


static int luai_io_readline (lua_State *L);


/*
** maximum number of arguments to 'f:lines'/'io.lines' (it + 3 must fit
** in the limit for upvalues of a closure)
*/
#define LUAI_MAXARGLINE	250

static void luai_aux_lines (lua_State *L, int toclose) {
  int n = lua_gettop(L) - 1;  /* number of arguments to read */
  luaL_argcheck(L, n <= LUAI_MAXARGLINE, LUAI_MAXARGLINE + 2, "too many arguments");
  lua_pushinteger(L, n);  /* number of arguments to read */
  lua_pushboolean(L, toclose);  /* close/not close file when finished */
  lua_rotate(L, 2, 2);  /* move 'n' and 'toclose' to their positions */
  lua_pushcclosure(L, luai_io_readline, 3 + n);
}


static int luai_f_lines (lua_State *L) {
  luai_tofile(L);  /* luai_check that it's a valid file handle */
  luai_aux_lines(L, 0);
  return 1;
}


static int luai_io_lines (lua_State *L) {
  int toclose;
  if (lua_isnone(L, 1)) lua_pushnil(L);  /* at least one argument */
  if (lua_isnil(L, 1)) {  /* no file name? */
    lua_getfield(L, LUA_REGISTRYINDEX, LUAI_IO_INPUT);  /* get default input */
    lua_replace(L, 1);  /* put it at index 1 */
    luai_tofile(L);  /* luai_check that it's a valid file handle */
    toclose = 0;  /* do not close it after iteration */
  }
  else {  /* open a new file */
    const char *filename = luaL_checkstring(L, 1);
    luai_opencheck(L, filename, "r");
    lua_replace(L, 1);  /* put file at index 1 */
    toclose = 1;  /* close it after iteration */
  }
  luai_aux_lines(L, toclose);
  return 1;
}


/*
** {======================================================
** READ
** =======================================================
*/


/* maximum length of a numeral */
#if !defined (LUAI_L_MAXLENNUM)
#define LUAI_L_MAXLENNUM     200
#endif


/* auxiliary structure used by 'luai_read_number' */
typedef struct {
  FILE *f;  /* file being read */
  int c;  /* current character (look ahead) */
  int n;  /* number of elements in buffer 'buff' */
  char buff[LUAI_L_MAXLENNUM + 1];  /* +1 for ending '\0' */
} luai_RN;


/*
** Add current char to buffer (if not out of space) and read luai_next one
*/
static int luai_nextc (luai_RN *rn) {
  if (rn->n >= LUAI_L_MAXLENNUM) {  /* buffer overflow? */
    rn->buff[0] = '\0';  /* invalidate result */
    return 0;  /* fail */
  }
  else {
    rn->buff[rn->n++] = rn->c;  /* luai_save current char */
    rn->c = luai_l_getc(rn->f);  /* read luai_next one */
    return 1;
  }
}


/*
** Accept current char if it is in 'set' (of size 2)
*/
static int luai_test2 (luai_RN *rn, const char *set) {
  if (rn->c == set[0] || rn->c == set[1])
    return luai_nextc(rn);
  else return 0;
}


/*
** Read a sequence of (hex)digits
*/
static int luai_readdigits (luai_RN *rn, int hex) {
  int count = 0;
  while ((hex ? isxdigit(rn->c) : isdigit(rn->c)) && luai_nextc(rn))
    count++;
  return count;
}


/*
** Read a number: first reads a valid prefix of a numeral into a buffer.
** Then it calls 'lua_stringtonumber' to luai_check whether the format is
** correct and to convert it to a Lua number
*/
static int luai_read_number (lua_State *L, FILE *f) {
  luai_RN rn;
  int count = 0;
  int hex = 0;
  char decp[2];
  rn.f = f; rn.n = 0;
  decp[0] = lua_getlocaledecpoint();  /* get decimal point from locale */
  decp[1] = '.';  /* always accept a dot */
  luai_l_lockfile(rn.f);
  do { rn.c = luai_l_getc(rn.f); } while (isspace(rn.c));  /* skip spaces */
  luai_test2(&rn, "-+");  /* optional signal */
  if (luai_test2(&rn, "00")) {
    if (luai_test2(&rn, "xX")) hex = 1;  /* numeral is hexadecimal */
    else count = 1;  /* count initial '0' as a valid luai_digit */
  }
  count += luai_readdigits(&rn, hex);  /* integral part */
  if (luai_test2(&rn, decp))  /* decimal point? */
    count += luai_readdigits(&rn, hex);  /* fractional part */
  if (count > 0 && luai_test2(&rn, (hex ? "pP" : "eE"))) {  /* exponent mark? */
    luai_test2(&rn, "-+");  /* exponent signal */
    luai_readdigits(&rn, 0);  /* exponent digits */
  }
  ungetc(rn.c, rn.f);  /* unread look-ahead char */
  luai_l_unlockfile(rn.f);
  rn.buff[rn.n] = '\0';  /* finish string */
  if (lua_stringtonumber(L, rn.buff))  /* is this a valid number? */
    return 1;  /* ok */
  else {  /* invalid format */
   lua_pushnil(L);  /* "result" to be removed */
   return 0;  /* read fails */
  }
}


static int luai_test_eof (lua_State *L, FILE *f) {
  int c = getc(f);
  ungetc(c, f);  /* no-op when c == EOF */
  lua_pushliteral(L, "");
  return (c != EOF);
}


static int luai_read_line (lua_State *L, FILE *f, int chop) {
  luaL_Buffer b;
  int c = '\0';
  luaL_buffinit(L, &b);
  while (c != EOF && c != '\n') {  /* repeat until end of line */
    char *buff = luaL_prepbuffer(&b);  /* preallocate buffer */
    int i = 0;
    luai_l_lockfile(f);  /* no memory errors can happen inside the lock */
    while (i < LUAL_BUFFERSIZE && (c = luai_l_getc(f)) != EOF && c != '\n')
      buff[i++] = c;
    luai_l_unlockfile(f);
    luaL_addsize(&b, i);
  }
  if (!chop && c == '\n')  /* want a newline and have one? */
    luaL_addchar(&b, c);  /* add ending newline to result */
  luaL_pushresult(&b);  /* close buffer */
  /* return ok if read something (either a newline or something else) */
  return (c == '\n' || lua_rawlen(L, -1) > 0);
}


static void luai_read_all (lua_State *L, FILE *f) {
  size_t nr;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  do {  /* read file in chunks of LUAL_BUFFERSIZE bytes */
    char *p = luaL_prepbuffer(&b);
    nr = fread(p, sizeof(char), LUAL_BUFFERSIZE, f);
    luaL_addsize(&b, nr);
  } while (nr == LUAL_BUFFERSIZE);
  luaL_pushresult(&b);  /* close buffer */
}


static int luai_read_chars (lua_State *L, FILE *f, size_t n) {
  size_t nr;  /* number of chars actually read */
  char *p;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  p = luaL_prepbuffsize(&b, n);  /* prepare buffer to read whole luai_getblock */
  nr = fread(p, sizeof(char), n, f);  /* try to read 'n' chars */
  luaL_addsize(&b, nr);
  luaL_pushresult(&b);  /* close buffer */
  return (nr > 0);  /* true iff read something */
}


static int luai_g_read (lua_State *L, FILE *f, int first) {
  int nargs = lua_gettop(L) - 1;
  int success;
  int n;
  clearerr(f);
  if (nargs == 0) {  /* no arguments? */
    success = luai_read_line(L, f, 1);
    n = first+1;  /* to return 1 result */
  }
  else {  /* ensure stack space for all results and for auxlib's buffer */
    luaL_checkstack(L, nargs+LUA_MINSTACK, "too many arguments");
    success = 1;
    for (n = first; nargs-- && success; n++) {
      if (lua_type(L, n) == LUA_TNUMBER) {
        size_t l = (size_t)luaL_checkinteger(L, n);
        success = (l == 0) ? luai_test_eof(L, f) : luai_read_chars(L, f, l);
      }
      else {
        const char *p = luaL_checkstring(L, n);
        if (*p == '*') p++;  /* skip optional '*' (for compatibility) */
        switch (*p) {
          case 'n':  /* number */
            success = luai_read_number(L, f);
            break;
          case 'l':  /* line */
            success = luai_read_line(L, f, 1);
            break;
          case 'L':  /* line with end-of-line */
            success = luai_read_line(L, f, 0);
            break;
          case 'a':  /* file */
            luai_read_all(L, f);  /* read entire file */
            success = 1; /* always success */
            break;
          default:
            return luaL_argerror(L, n, "invalid format");
        }
      }
    }
  }
  if (ferror(f))
    return luaL_fileresult(L, 0, NULL);
  if (!success) {
    lua_pop(L, 1);  /* remove last result */
    lua_pushnil(L);  /* push nil instead */
  }
  return n - first;
}


static int luai_io_read (lua_State *L) {
  return luai_g_read(L, luai_getiofile(L, LUAI_IO_INPUT), 1);
}


static int luai_f_read (lua_State *L) {
  return luai_g_read(L, luai_tofile(L), 2);
}


static int luai_io_readline (lua_State *L) {
  luai_LStream *p = (luai_LStream *)lua_touserdata(L, lua_upvalueindex(1));
  int i;
  int n = (int)lua_tointeger(L, lua_upvalueindex(2));
  if (luai_isclosed(p))  /* file is already closed? */
    return luaL_error(L, "file is already closed");
  lua_settop(L , 1);
  luaL_checkstack(L, n, "too many arguments");
  for (i = 1; i <= n; i++)  /* push arguments to 'luai_g_read' */
    lua_pushvalue(L, lua_upvalueindex(3 + i));
  n = luai_g_read(L, p->f, 2);  /* 'n' is number of results */
  lua_assert(n > 0);  /* should return at least a nil */
  if (lua_toboolean(L, -n))  /* read at least one value? */
    return n;  /* return them */
  else {  /* first result is nil: EOF or error */
    if (n > 1) {  /* is there error information? */
      /* 2nd result is error message */
      return luaL_error(L, "%s", lua_tostring(L, -n + 1));
    }
    if (lua_toboolean(L, lua_upvalueindex(3))) {  /* generator created file? */
      lua_settop(L, 0);
      lua_pushvalue(L, lua_upvalueindex(1));
      luai_aux_close(L);  /* close it */
    }
    return 0;
  }
}

/* }====================================================== */


static int luai_g_write (lua_State *L, FILE *f, int arg) {
  int nargs = lua_gettop(L) - arg;
  int status = 1;
  for (; nargs--; arg++) {
    if (lua_type(L, arg) == LUA_TNUMBER) {
      /* optimization: could be done exactly as for strings */
      int len = lua_isinteger(L, arg)
                ? fprintf(f, LUA_INTEGER_FMT,
                             (LUAI_UACINT)lua_tointeger(L, arg))
                : fprintf(f, LUA_NUMBER_FMT,
                             (LUAI_UACNUMBER)lua_tonumber(L, arg));
      status = status && (len > 0);
    }
    else {
      size_t l;
      const char *s = luaL_checklstring(L, arg, &l);
      status = status && (fwrite(s, sizeof(char), l, f) == l);
    }
  }
  if (status) return 1;  /* file handle already on stack top */
  else return luaL_fileresult(L, status, NULL);
}


static int luai_io_write (lua_State *L) {
  return luai_g_write(L, luai_getiofile(L, LUAI_IO_OUTPUT), 1);
}


static int luai_f_write (lua_State *L) {
  FILE *f = luai_tofile(L);
  lua_pushvalue(L, 1);  /* push file at the stack top (to be returned) */
  return luai_g_write(L, f, 2);
}


static int luai_f_seek (lua_State *L) {
  static const int mode[] = {SEEK_SET, SEEK_CUR, SEEK_END};
  static const char *const modenames[] = {"set", "cur", "end", NULL};
  FILE *f = luai_tofile(L);
  int op = luaL_checkoption(L, 2, "cur", modenames);
  lua_Integer p3 = luaL_optinteger(L, 3, 0);
  luai_l_seeknum offset = (luai_l_seeknum)p3;
  luaL_argcheck(L, (lua_Integer)offset == p3, 3,
                  "not an integer in proper range");
  op = luai_l_fseek(f, offset, mode[op]);
  if (op)
    return luaL_fileresult(L, 0, NULL);  /* error */
  else {
    lua_pushinteger(L, (lua_Integer)luai_l_ftell(f));
    return 1;
  }
}


static int luai_f_setvbuf (lua_State *L) {
  static const int mode[] = {_IONBF, _IOFBF, _IOLBF};
  static const char *const modenames[] = {"no", "full", "line", NULL};
  FILE *f = luai_tofile(L);
  int op = luaL_checkoption(L, 2, NULL, modenames);
  lua_Integer sz = luaL_optinteger(L, 3, LUAL_BUFFERSIZE);
  int res = setvbuf(f, NULL, mode[op], (size_t)sz);
  return luaL_fileresult(L, res == 0, NULL);
}



static int luai_io_flush (lua_State *L) {
  return luaL_fileresult(L, fflush(luai_getiofile(L, LUAI_IO_OUTPUT)) == 0, NULL);
}


static int luai_f_flush (lua_State *L) {
  return luaL_fileresult(L, fflush(luai_tofile(L)) == 0, NULL);
}


/*
** functions for 'io' library
*/
static const luaL_Reg luai_iolib[] = {
  {"close", luai_io_close},
  {"flush", luai_io_flush},
  {"input", luai_io_input},
  {"lines", luai_io_lines},
  {"open", luai_io_open},
  {"output", luai_io_output},
  {"popen", luai_io_popen},
  {"read", luai_io_read},
  {"tmpfile", luai_io_tmpfile},
  {"type", luai_io_type},
  {"write", luai_io_write},
  {NULL, NULL}
};


/*
** methods for file handles
*/
static const luaL_Reg luai_flib[] = {
  {"close", luai_io_close},
  {"flush", luai_f_flush},
  {"lines", luai_f_lines},
  {"read", luai_f_read},
  {"seek", luai_f_seek},
  {"setvbuf", luai_f_setvbuf},
  {"write", luai_f_write},
  {"__gc", luai_f_gc},
  {"__tostring", luai_f_tostring},
  {NULL, NULL}
};


static void luai_createmeta (lua_State *L) {
  luaL_newmetatable(L, LUA_FILEHANDLE);  /* create metatable for file handles */
  lua_pushvalue(L, -1);  /* push metatable */
  lua_setfield(L, -2, "__index");  /* metatable.__index = metatable */
  luaL_setfuncs(L, luai_flib, 0);  /* add file methods to new metatable */
  lua_pop(L, 1);  /* pop new metatable */
}


/*
** function to (not) close the standard files stdin, stdout, and stderr
*/
static int luai_io_noclose (lua_State *L) {
  luai_LStream *p = luai_tolstream(L);
  p->closef = &luai_io_noclose;  /* keep file opened */
  lua_pushnil(L);
  lua_pushliteral(L, "cannot close standard file");
  return 2;
}


static void luai_createstdfile (lua_State *L, FILE *f, const char *k,
                           const char *fname) {
  luai_LStream *p = luai_toprefile(L);
  p->f = f;
  p->closef = &luai_io_noclose;
  if (k != NULL) {
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, k);  /* add file to registry */
  }
  lua_setfield(L, -2, fname);  /* add file to module */
}


LUAMOD_API int luaopen_io (lua_State *L) {
  luaL_newlib(L, luai_iolib);  /* new module */
  luai_createmeta(L);
  /* create (and set) default files */
  luai_createstdfile(L, stdin, LUAI_IO_INPUT, "stdin");
  luai_createstdfile(L, stdout, LUAI_IO_OUTPUT, "stdout");
  luai_createstdfile(L, stderr, NULL, "stderr");
  return 1;
}

/*__llex.c__*/

#define luai_next(ls) (ls->current = luai_zgetc(ls->z))



#define luai_currIsNewline(ls)	(ls->current == '\n' || ls->current == '\r')


/* ORDER LUAI_RESERVED */
static const char *const luaX_tokens [] = {
    "and", "break", "do", "else", "elseif",
    "end", "false", "for", "function", "goto", "if",
    "in", "local", "nil", "not", "or", "repeat",
    "return", "then", "true", "until", "while",
    "//", "..", "...", "==", ">=", "<=", "~=",
    "<<", ">>", "::", "<eof>",
    "<number>", "<integer>", "<name>", "<string>"
};


#define luai_save_and_next(ls) (luai_save(ls, ls->current), luai_next(ls))


static luai_l_noret luai_lexerror (luai_LexState *ls, const char *msg, int token);


static void luai_save (luai_LexState *ls, int c) {
  luai_Mbuffer *b = ls->buff;
  if (luaZ_bufflen(b) + 1 > luaZ_sizebuffer(b)) {
    size_t newsize;
    if (luaZ_sizebuffer(b) >= LUAI_MAX_SIZE/2)
      luai_lexerror(ls, "lexical element too long", 0);
    newsize = luaZ_sizebuffer(b) * 2;
    luaZ_resizebuffer(ls->L, b, newsize);
  }
  b->buffer[luaZ_bufflen(b)++] = luai_cast(char, c);
}


void luaX_init (lua_State *L) {
  int i;
  luai_TString *e = luaS_newliteral(L, LUA_ENV);  /* create env name */
  luaC_fix(L, luai_obj2gco(e));  /* never collect this name */
  for (i=0; i<LUAI_NUM_RESERVED; i++) {
    luai_TString *ts = luaS_new(L, luaX_tokens[i]);
    luaC_fix(L, luai_obj2gco(ts));  /* reserved words are never collected */
    ts->extra = luai_cast_byte(i+1);  /* reserved word */
  }
}


const char *luaX_token2str (luai_LexState *ls, int token) {
  if (token < LUAI_FIRST_RESERVED) {  /* single-byte symbols? */
    lua_assert(token == luai_cast_uchar(token));
    return luaO_pushfstring(ls->L, "'%c'", token);
  }
  else {
    const char *s = luaX_tokens[token - LUAI_FIRST_RESERVED];
    if (token < LUAI_TK_EOS)  /* fixed format (symbols and reserved words)? */
      return luaO_pushfstring(ls->L, "'%s'", s);
    else  /* names, strings, and numerals */
      return s;
  }
}


static const char *luai_txtToken (luai_LexState *ls, int token) {
  switch (token) {
    case LUAI_TK_NAME: case LUAI_TK_STRING:
    case LUAI_TK_FLT: case LUAI_TK_INT:
      luai_save(ls, '\0');
      return luaO_pushfstring(ls->L, "'%s'", luaZ_buffer(ls->buff));
    default:
      return luaX_token2str(ls, token);
  }
}


static luai_l_noret luai_lexerror (luai_LexState *ls, const char *msg, int token) {
  msg = luaG_addinfo(ls->L, msg, ls->source, ls->linenumber);
  if (token)
    luaO_pushfstring(ls->L, "%s near %s", msg, luai_txtToken(ls, token));
  luaD_throw(ls->L, LUA_ERRSYNTAX);
}


luai_l_noret luaX_syntaxerror (luai_LexState *ls, const char *msg) {
  luai_lexerror(ls, msg, ls->t.token);
}


/*
** creates a new string and anchors it in scanner's table so that
** it will not be collected until the end of the compilation
** (by that time it should be anchored somewhere)
*/
luai_TString *luaX_newstring (luai_LexState *ls, const char *str, size_t l) {
  lua_State *L = ls->L;
  luai_TValue *o;  /* entry for 'str' */
  luai_TString *ts = luaS_newlstr(L, str, l);  /* create new string */
  luai_setsvalue2s(L, L->top++, ts);  /* temporarily anchor it in stack */
  o = luaH_set(L, ls->h, L->top - 1);
  if (luai_ttisnil(o)) {  /* not in use yet? */
    /* boolean value does not need LUAI_GC barrier;
       table has no metatable, so it does not need to invalidate cache */
    luai_setbvalue(o, 1);  /* t[string] = true */
    luaC_checkGC(L);
  }
  else {  /* string already present */
    ts = luai_tsvalue(luai_keyfromval(o));  /* re-use value previously stored */
  }
  L->top--;  /* remove string from stack */
  return ts;
}


/*
** increment line number and skips newline sequence (any of
** \n, \r, \n\r, or \r\n)
*/
static void luai_inclinenumber (luai_LexState *ls) {
  int old = ls->current;
  lua_assert(luai_currIsNewline(ls));
  luai_next(ls);  /* skip '\n' or '\r' */
  if (luai_currIsNewline(ls) && ls->current != old)
    luai_next(ls);  /* skip '\n\r' or '\r\n' */
  if (++ls->linenumber >= LUAI_MAX_INT)
    luai_lexerror(ls, "chunk has too many lines", 0);
}


void luaX_setinput (lua_State *L, luai_LexState *ls, LUAI_ZIO *z, luai_TString *source,
                    int firstchar) {
  ls->t.token = 0;
  ls->L = L;
  ls->current = firstchar;
  ls->lookahead.token = LUAI_TK_EOS;  /* no look-ahead token */
  ls->z = z;
  ls->fs = NULL;
  ls->linenumber = 1;
  ls->lastline = 1;
  ls->source = source;
  ls->envn = luaS_newliteral(L, LUA_ENV);  /* get env name */
  luaZ_resizebuffer(ls->L, ls->buff, LUA_MINBUFFER);  /* initialize buffer */
}



/*
** =======================================================
** LEXICAL ANALYZER
** =======================================================
*/


static int luai_check_next1 (luai_LexState *ls, int c) {
  if (ls->current == c) {
    luai_next(ls);
    return 1;
  }
  else return 0;
}


/*
** Check whether current char is in set 'set' (with two chars) and
** saves it
*/
static int luai_check_next2 (luai_LexState *ls, const char *set) {
  lua_assert(set[2] == '\0');
  if (ls->current == set[0] || ls->current == set[1]) {
    luai_save_and_next(ls);
    return 1;
  }
  else return 0;
}


/* LUA_NUMBER */
/*
** this function is quite liberal in what it accepts, as 'luaO_str2num'
** will reject ill-formed numerals.
*/
static int luai_read_numeral (luai_LexState *ls, luai_SemInfo *seminfo) {
  luai_TValue obj;
  const char *expo = "Ee";
  int first = ls->current;
  lua_assert(luai_lisdigit(ls->current));
  luai_save_and_next(ls);
  if (first == '0' && luai_check_next2(ls, "xX"))  /* hexadecimal? */
    expo = "Pp";
  for (;;) {
    if (luai_check_next2(ls, expo))  /* exponent part? */
      luai_check_next2(ls, "-+");  /* optional exponent sign */
    if (luai_lisxdigit(ls->current))
      luai_save_and_next(ls);
    else if (ls->current == '.')
      luai_save_and_next(ls);
    else break;
  }
  luai_save(ls, '\0');
  if (luaO_str2num(luaZ_buffer(ls->buff), &obj) == 0)  /* format error? */
    luai_lexerror(ls, "malformed number", LUAI_TK_FLT);
  if (luai_ttisinteger(&obj)) {
    seminfo->i = luai_ivalue(&obj);
    return LUAI_TK_INT;
  }
  else {
    lua_assert(luai_ttisfloat(&obj));
    seminfo->r = luai_fltvalue(&obj);
    return LUAI_TK_FLT;
  }
}


/*
** skip a sequence '[=*[' or ']=*]'; if sequence is well formed, return
** its number of '='s; otherwise, return a negative number (-1 iff there
** are no '='s after initial bracket)
*/
static int luai_skip_sep (luai_LexState *ls) {
  int count = 0;
  int s = ls->current;
  lua_assert(s == '[' || s == ']');
  luai_save_and_next(ls);
  while (ls->current == '=') {
    luai_save_and_next(ls);
    count++;
  }
  return (ls->current == s) ? count : (-count) - 1;
}


static void luai_read_long_string (luai_LexState *ls, luai_SemInfo *seminfo, int sep) {
  int line = ls->linenumber;  /* initial line (for error message) */
  luai_save_and_next(ls);  /* skip 2nd '[' */
  if (luai_currIsNewline(ls))  /* string starts with a newline? */
    luai_inclinenumber(ls);  /* skip it */
  for (;;) {
    switch (ls->current) {
      case LUAI_EOZ: {  /* error */
        const char *what = (seminfo ? "string" : "comment");
        const char *msg = luaO_pushfstring(ls->L,
                     "unfinished long %s (starting at line %d)", what, line);
        luai_lexerror(ls, msg, LUAI_TK_EOS);
        break;  /* to avoid warnings */
      }
      case ']': {
        if (luai_skip_sep(ls) == sep) {
          luai_save_and_next(ls);  /* skip 2nd ']' */
          goto endloop;
        }
        break;
      }
      case '\n': case '\r': {
        luai_save(ls, '\n');
        luai_inclinenumber(ls);
        if (!seminfo) luaZ_resetbuffer(ls->buff);  /* avoid wasting space */
        break;
      }
      default: {
        if (seminfo) luai_save_and_next(ls);
        else luai_next(ls);
      }
    }
  } endloop:
  if (seminfo)
    seminfo->ts = luaX_newstring(ls, luaZ_buffer(ls->buff) + (2 + sep),
                                     luaZ_bufflen(ls->buff) - 2*(2 + sep));
}


static void luai_esccheck (luai_LexState *ls, int c, const char *msg) {
  if (!c) {
    if (ls->current != LUAI_EOZ)
      luai_save_and_next(ls);  /* add current to buffer for error message */
    luai_lexerror(ls, msg, LUAI_TK_STRING);
  }
}


static int luai_gethexa (luai_LexState *ls) {
  luai_save_and_next(ls);
  luai_esccheck (ls, luai_lisxdigit(ls->current), "hexadecimal luai_digit expected");
  return luaO_hexavalue(ls->current);
}


static int luai_readhexaesc (luai_LexState *ls) {
  int r = luai_gethexa(ls);
  r = (r << 4) + luai_gethexa(ls);
  luaZ_buffremove(ls->buff, 2);  /* remove saved chars from buffer */
  return r;
}


static unsigned long luai_readutf8esc (luai_LexState *ls) {
  unsigned long r;
  int i = 4;  /* chars to be removed: '\', 'u', '{', and first luai_digit */
  luai_save_and_next(ls);  /* skip 'u' */
  luai_esccheck(ls, ls->current == '{', "missing '{'");
  r = luai_gethexa(ls);  /* must have at least one luai_digit */
  while ((luai_save_and_next(ls), luai_lisxdigit(ls->current))) {
    i++;
    r = (r << 4) + luaO_hexavalue(ls->current);
    luai_esccheck(ls, r <= 0x10FFFF, "UTF-8 value too large");
  }
  luai_esccheck(ls, ls->current == '}', "missing '}'");
  luai_next(ls);  /* skip '}' */
  luaZ_buffremove(ls->buff, i);  /* remove saved chars from buffer */
  return r;
}


static void luai_utf8esc (luai_LexState *ls) {
  char buff[LUAI_UTF8BUFFSZ];
  int n = luaO_utf8esc(buff, luai_readutf8esc(ls));
  for (; n > 0; n--)  /* add 'buff' to string */
    luai_save(ls, buff[LUAI_UTF8BUFFSZ - n]);
}


static int luai_readdecesc (luai_LexState *ls) {
  int i;
  int r = 0;  /* result accumulator */
  for (i = 0; i < 3 && luai_lisdigit(ls->current); i++) {  /* read up to 3 digits */
    r = 10*r + ls->current - '0';
    luai_save_and_next(ls);
  }
  luai_esccheck(ls, r <= UCHAR_MAX, "decimal escape too large");
  luaZ_buffremove(ls->buff, i);  /* remove read digits from buffer */
  return r;
}


static void luai_read_string (luai_LexState *ls, int del, luai_SemInfo *seminfo) {
  luai_save_and_next(ls);  /* keep delimiter (for error messages) */
  while (ls->current != del) {
    switch (ls->current) {
      case LUAI_EOZ:
        luai_lexerror(ls, "unfinished string", LUAI_TK_EOS);
        break;  /* to avoid warnings */
      case '\n':
      case '\r':
        luai_lexerror(ls, "unfinished string", LUAI_TK_STRING);
        break;  /* to avoid warnings */
      case '\\': {  /* escape sequences */
        int c;  /* final character to be saved */
        luai_save_and_next(ls);  /* keep '\\' for error messages */
        switch (ls->current) {
          case 'a': c = '\a'; goto luai_read_save;
          case 'b': c = '\b'; goto luai_read_save;
          case 'f': c = '\f'; goto luai_read_save;
          case 'n': c = '\n'; goto luai_read_save;
          case 'r': c = '\r'; goto luai_read_save;
          case 't': c = '\t'; goto luai_read_save;
          case 'v': c = '\v'; goto luai_read_save;
          case 'x': c = luai_readhexaesc(ls); goto luai_read_save;
          case 'u': luai_utf8esc(ls);  goto no_save;
          case '\n': case '\r':
            luai_inclinenumber(ls); c = '\n'; goto only_save;
          case '\\': case '\"': case '\'':
            c = ls->current; goto luai_read_save;
          case LUAI_EOZ: goto no_save;  /* will raise an error luai_next loop */
          case 'z': {  /* zap following span of spaces */
            luaZ_buffremove(ls->buff, 1);  /* remove '\\' */
            luai_next(ls);  /* skip the 'z' */
            while (luai_lisspace(ls->current)) {
              if (luai_currIsNewline(ls)) luai_inclinenumber(ls);
              else luai_next(ls);
            }
            goto no_save;
          }
          default: {
            luai_esccheck(ls, luai_lisdigit(ls->current), "invalid escape sequence");
            c = luai_readdecesc(ls);  /* digital escape '\ddd' */
            goto only_save;
          }
        }
       luai_read_save:
         luai_next(ls);
         /* go through */
       only_save:
         luaZ_buffremove(ls->buff, 1);  /* remove '\\' */
         luai_save(ls, c);
         /* go through */
       no_save: break;
      }
      default:
        luai_save_and_next(ls);
    }
  }
  luai_save_and_next(ls);  /* skip delimiter */
  seminfo->ts = luaX_newstring(ls, luaZ_buffer(ls->buff) + 1,
                                   luaZ_bufflen(ls->buff) - 2);
}


static int luai_llex (luai_LexState *ls, luai_SemInfo *seminfo) {
  luaZ_resetbuffer(ls->buff);
  for (;;) {
    switch (ls->current) {
      case '\n': case '\r': {  /* line breaks */
        luai_inclinenumber(ls);
        break;
      }
      case ' ': case '\f': case '\t': case '\v': {  /* spaces */
        luai_next(ls);
        break;
      }
      case '-': {  /* '-' or '--' (comment) */
        luai_next(ls);
        if (ls->current != '-') return '-';
        /* else is a comment */
        luai_next(ls);
        if (ls->current == '[') {  /* long comment? */
          int sep = luai_skip_sep(ls);
          luaZ_resetbuffer(ls->buff);  /* 'luai_skip_sep' may dirty the buffer */
          if (sep >= 0) {
            luai_read_long_string(ls, NULL, sep);  /* skip long comment */
            luaZ_resetbuffer(ls->buff);  /* previous call may dirty the buff. */
            break;
          }
        }
        /* else short comment */
        while (!luai_currIsNewline(ls) && ls->current != LUAI_EOZ)
          luai_next(ls);  /* skip until end of line (or end of file) */
        break;
      }
      case '[': {  /* long string or simply '[' */
        int sep = luai_skip_sep(ls);
        if (sep >= 0) {
          luai_read_long_string(ls, seminfo, sep);
          return LUAI_TK_STRING;
        }
        else if (sep != -1)  /* '[=...' missing second bracket */
          luai_lexerror(ls, "invalid long string delimiter", LUAI_TK_STRING);
        return '[';
      }
      case '=': {
        luai_next(ls);
        if (luai_check_next1(ls, '=')) return LUAI_TK_EQ;
        else return '=';
      }
      case '<': {
        luai_next(ls);
        if (luai_check_next1(ls, '=')) return LUAI_TK_LE;
        else if (luai_check_next1(ls, '<')) return LUAI_TK_SHL;
        else return '<';
      }
      case '>': {
        luai_next(ls);
        if (luai_check_next1(ls, '=')) return LUAI_TK_GE;
        else if (luai_check_next1(ls, '>')) return LUAI_TK_SHR;
        else return '>';
      }
      case '/': {
        luai_next(ls);
        if (luai_check_next1(ls, '/')) return LUAI_TK_IDIV;
        else return '/';
      }
      case '~': {
        luai_next(ls);
        if (luai_check_next1(ls, '=')) return LUAI_TK_NE;
        else return '~';
      }
      case ':': {
        luai_next(ls);
        if (luai_check_next1(ls, ':')) return LUAI_TK_DBCOLON;
        else return ':';
      }
      case '"': case '\'': {  /* short literal strings */
        luai_read_string(ls, ls->current, seminfo);
        return LUAI_TK_STRING;
      }
      case '.': {  /* '.', '..', '...', or number */
        luai_save_and_next(ls);
        if (luai_check_next1(ls, '.')) {
          if (luai_check_next1(ls, '.'))
            return LUAI_TK_DOTS;   /* '...' */
          else return LUAI_TK_CONCAT;   /* '..' */
        }
        else if (!luai_lisdigit(ls->current)) return '.';
        else return luai_read_numeral(ls, seminfo);
      }
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9': {
        return luai_read_numeral(ls, seminfo);
      }
      case LUAI_EOZ: {
        return LUAI_TK_EOS;
      }
      default: {
        if (luai_lislalpha(ls->current)) {  /* identifier or reserved word? */
          luai_TString *ts;
          do {
            luai_save_and_next(ls);
          } while (luai_lislalnum(ls->current));
          ts = luaX_newstring(ls, luaZ_buffer(ls->buff),
                                  luaZ_bufflen(ls->buff));
          seminfo->ts = ts;
          if (luai_isreserved(ts))  /* reserved word? */
            return ts->extra - 1 + LUAI_FIRST_RESERVED;
          else {
            return LUAI_TK_NAME;
          }
        }
        else {  /* single-char tokens (+ - / ...) */
          int c = ls->current;
          luai_next(ls);
          return c;
        }
      }
    }
  }
}


void luaX_next (luai_LexState *ls) {
  ls->lastline = ls->linenumber;
  if (ls->lookahead.token != LUAI_TK_EOS) {  /* is there a look-ahead token? */
    ls->t = ls->lookahead;  /* use this one */
    ls->lookahead.token = LUAI_TK_EOS;  /* and discharge it */
  }
  else
    ls->t.token = luai_llex(ls, &ls->t.seminfo);  /* read luai_next token */
}


int luaX_lookahead (luai_LexState *ls) {
  lua_assert(ls->lookahead.token == LUAI_TK_EOS);
  ls->lookahead.token = luai_llex(ls, &ls->lookahead.seminfo);
  return ls->lookahead.token;
}

/*__lmathlib.c__*/

#define LUA_PI	(luai_l_mathop(3.141592653589793238462643383279502884))


#if !defined(luai_l_rand)		/* { */
#if defined(LUA_USE_POSIX)
#define luai_l_rand()	random()
#define luai_l_srand(x)	srandom(x)
#define LUAI_L_RANDMAX	2147483647	/* (2^31 - 1), following POSIX */
#else
#define luai_l_rand()	rand()
#define luai_l_srand(x)	srand(x)
#define LUAI_L_RANDMAX	RAND_MAX
#endif
#endif				/* } */


static int luai_math_abs (lua_State *L) {
  if (lua_isinteger(L, 1)) {
    lua_Integer n = lua_tointeger(L, 1);
    if (n < 0) n = (lua_Integer)(0u - (lua_Unsigned)n);
    lua_pushinteger(L, n);
  }
  else
    lua_pushnumber(L, luai_l_mathop(fabs)(luaL_checknumber(L, 1)));
  return 1;
}

static int luai_math_sin (lua_State *L) {
  lua_pushnumber(L, luai_l_mathop(sin)(luaL_checknumber(L, 1)));
  return 1;
}

static int luai_math_cos (lua_State *L) {
  lua_pushnumber(L, luai_l_mathop(cos)(luaL_checknumber(L, 1)));
  return 1;
}

static int luai_math_tan (lua_State *L) {
  lua_pushnumber(L, luai_l_mathop(tan)(luaL_checknumber(L, 1)));
  return 1;
}

static int luai_math_asin (lua_State *L) {
  lua_pushnumber(L, luai_l_mathop(asin)(luaL_checknumber(L, 1)));
  return 1;
}

static int luai_math_acos (lua_State *L) {
  lua_pushnumber(L, luai_l_mathop(acos)(luaL_checknumber(L, 1)));
  return 1;
}

static int luai_math_atan (lua_State *L) {
  lua_Number y = luaL_checknumber(L, 1);
  lua_Number x = luaL_optnumber(L, 2, 1);
  lua_pushnumber(L, luai_l_mathop(atan2)(y, x));
  return 1;
}


static int luai_math_toint (lua_State *L) {
  int valid;
  lua_Integer n = lua_tointegerx(L, 1, &valid);
  if (valid)
    lua_pushinteger(L, n);
  else {
    luaL_checkany(L, 1);
    lua_pushnil(L);  /* value is not convertible to integer */
  }
  return 1;
}


static void luai_pushnumint (lua_State *L, lua_Number d) {
  lua_Integer n;
  if (lua_numbertointeger(d, &n))  /* does 'd' fit in an integer? */
    lua_pushinteger(L, n);  /* result is integer */
  else
    lua_pushnumber(L, d);  /* result is float */
}


static int luai_math_floor (lua_State *L) {
  if (lua_isinteger(L, 1))
    lua_settop(L, 1);  /* integer is its own floor */
  else {
    lua_Number d = luai_l_mathop(floor)(luaL_checknumber(L, 1));
    luai_pushnumint(L, d);
  }
  return 1;
}


static int luai_math_ceil (lua_State *L) {
  if (lua_isinteger(L, 1))
    lua_settop(L, 1);  /* integer is its own ceil */
  else {
    lua_Number d = luai_l_mathop(ceil)(luaL_checknumber(L, 1));
    luai_pushnumint(L, d);
  }
  return 1;
}


static int luai_math_fmod (lua_State *L) {
  if (lua_isinteger(L, 1) && lua_isinteger(L, 2)) {
    lua_Integer d = lua_tointeger(L, 2);
    if ((lua_Unsigned)d + 1u <= 1u) {  /* special cases: -1 or 0 */
      luaL_argcheck(L, d != 0, 2, "zero");
      lua_pushinteger(L, 0);  /* avoid overflow with 0x80000... / -1 */
    }
    else
      lua_pushinteger(L, lua_tointeger(L, 1) % d);
  }
  else
    lua_pushnumber(L, luai_l_mathop(fmod)(luaL_checknumber(L, 1),
                                     luaL_checknumber(L, 2)));
  return 1;
}


/*
** luai_next function does not use 'modf', avoiding problems with 'double*'
** (which is not compatible with 'float*') when lua_Number is not
** 'double'.
*/
static int luai_math_modf (lua_State *L) {
  if (lua_isinteger(L ,1)) {
    lua_settop(L, 1);  /* number is its own integer part */
    lua_pushnumber(L, 0);  /* no fractional part */
  }
  else {
    lua_Number n = luaL_checknumber(L, 1);
    /* integer part (rounds toward zero) */
    lua_Number ip = (n < 0) ? luai_l_mathop(ceil)(n) : luai_l_mathop(floor)(n);
    luai_pushnumint(L, ip);
    /* fractional part (test needed for inf/-inf) */
    lua_pushnumber(L, (n == ip) ? luai_l_mathop(0.0) : (n - ip));
  }
  return 2;
}


static int luai_math_sqrt (lua_State *L) {
  lua_pushnumber(L, luai_l_mathop(sqrt)(luaL_checknumber(L, 1)));
  return 1;
}


static int luai_math_ult (lua_State *L) {
  lua_Integer a = luaL_checkinteger(L, 1);
  lua_Integer b = luaL_checkinteger(L, 2);
  lua_pushboolean(L, (lua_Unsigned)a < (lua_Unsigned)b);
  return 1;
}

static int luai_math_log (lua_State *L) {
  lua_Number x = luaL_checknumber(L, 1);
  lua_Number res;
  if (lua_isnoneornil(L, 2))
    res = luai_l_mathop(log)(x);
  else {
    lua_Number base = luaL_checknumber(L, 2);
#if !defined(LUA_USE_C89)
    if (base == luai_l_mathop(2.0))
      res = luai_l_mathop(log2)(x); else
#endif
    if (base == luai_l_mathop(10.0))
      res = luai_l_mathop(log10)(x);
    else
      res = luai_l_mathop(log)(x)/luai_l_mathop(log)(base);
  }
  lua_pushnumber(L, res);
  return 1;
}

static int luai_math_exp (lua_State *L) {
  lua_pushnumber(L, luai_l_mathop(exp)(luaL_checknumber(L, 1)));
  return 1;
}

static int luai_math_deg (lua_State *L) {
  lua_pushnumber(L, luaL_checknumber(L, 1) * (luai_l_mathop(180.0) / LUA_PI));
  return 1;
}

static int luai_math_rad (lua_State *L) {
  lua_pushnumber(L, luaL_checknumber(L, 1) * (LUA_PI / luai_l_mathop(180.0)));
  return 1;
}


static int luai_math_min (lua_State *L) {
  int n = lua_gettop(L);  /* number of arguments */
  int imin = 1;  /* index of current minimum value */
  int i;
  luaL_argcheck(L, n >= 1, 1, "value expected");
  for (i = 2; i <= n; i++) {
    if (lua_compare(L, i, imin, LUA_OPLT))
      imin = i;
  }
  lua_pushvalue(L, imin);
  return 1;
}


static int luai_math_max (lua_State *L) {
  int n = lua_gettop(L);  /* number of arguments */
  int imax = 1;  /* index of current maximum value */
  int i;
  luaL_argcheck(L, n >= 1, 1, "value expected");
  for (i = 2; i <= n; i++) {
    if (lua_compare(L, imax, i, LUA_OPLT))
      imax = i;
  }
  lua_pushvalue(L, imax);
  return 1;
}

/*
** This function uses 'double' (instead of 'lua_Number') to ensure that
** all bits from 'luai_l_rand' can be represented, and that 'RANDMAX + 1.0'
** will keep full precision (ensuring that 'r' is always less than 1.0.)
*/
static int luai_math_random (lua_State *L) {
  lua_Integer low, up;
  double r = (double)luai_l_rand() * (1.0 / ((double)LUAI_L_RANDMAX + 1.0));
  switch (lua_gettop(L)) {  /* luai_check number of arguments */
    case 0: {  /* no arguments */
      lua_pushnumber(L, (lua_Number)r);  /* Number between 0 and 1 */
      return 1;
    }
    case 1: {  /* only upper limit */
      low = 1;
      up = luaL_checkinteger(L, 1);
      break;
    }
    case 2: {  /* lower and upper limits */
      low = luaL_checkinteger(L, 1);
      up = luaL_checkinteger(L, 2);
      break;
    }
    default: return luaL_error(L, "wrong number of arguments");
  }
  /* random integer in the interval [low, up] */
  luaL_argcheck(L, low <= up, 1, "interval is empty");
  luaL_argcheck(L, low >= 0 || up <= LUA_MAXINTEGER + low, 1,
                   "interval too large");
  r *= (double)(up - low) + 1.0;
  lua_pushinteger(L, (lua_Integer)r + low);
  return 1;
}


static int luai_math_randomseed (lua_State *L) {
  luai_l_srand((unsigned int)(lua_Integer)luaL_checknumber(L, 1));
  (void)luai_l_rand(); /* discard first value to avoid undesirable correlations */
  return 0;
}


static int luai_math_type (lua_State *L) {
  if (lua_type(L, 1) == LUA_TNUMBER) {
      if (lua_isinteger(L, 1))
        lua_pushliteral(L, "integer");
      else
        lua_pushliteral(L, "float");
  }
  else {
    luaL_checkany(L, 1);
    lua_pushnil(L);
  }
  return 1;
}


/*
** {==================================================================
** Deprecated functions (for compatibility only)
** ===================================================================
*/
#if defined(LUA_COMPAT_MATHLIB)

static int luai_math_cosh (lua_State *L) {
  lua_pushnumber(L, luai_l_mathop(cosh)(luaL_checknumber(L, 1)));
  return 1;
}

static int luai_math_sinh (lua_State *L) {
  lua_pushnumber(L, luai_l_mathop(sinh)(luaL_checknumber(L, 1)));
  return 1;
}

static int luai_math_tanh (lua_State *L) {
  lua_pushnumber(L, luai_l_mathop(tanh)(luaL_checknumber(L, 1)));
  return 1;
}

static int luai_math_pow (lua_State *L) {
  lua_Number x = luaL_checknumber(L, 1);
  lua_Number y = luaL_checknumber(L, 2);
  lua_pushnumber(L, luai_l_mathop(pow)(x, y));
  return 1;
}

static int luai_math_frexp (lua_State *L) {
  int e;
  lua_pushnumber(L, luai_l_mathop(frexp)(luaL_checknumber(L, 1), &e));
  lua_pushinteger(L, e);
  return 2;
}

static int luai_math_ldexp (lua_State *L) {
  lua_Number x = luaL_checknumber(L, 1);
  int ep = (int)luaL_checkinteger(L, 2);
  lua_pushnumber(L, luai_l_mathop(ldexp)(x, ep));
  return 1;
}

static int luai_math_log10 (lua_State *L) {
  lua_pushnumber(L, luai_l_mathop(log10)(luaL_checknumber(L, 1)));
  return 1;
}

#endif
/* }================================================================== */



static const luaL_Reg luai_mathlib[] = {
  {"abs",   luai_math_abs},
  {"acos",  luai_math_acos},
  {"asin",  luai_math_asin},
  {"atan",  luai_math_atan},
  {"ceil",  luai_math_ceil},
  {"cos",   luai_math_cos},
  {"deg",   luai_math_deg},
  {"exp",   luai_math_exp},
  {"tointeger", luai_math_toint},
  {"floor", luai_math_floor},
  {"fmod",   luai_math_fmod},
  {"ult",   luai_math_ult},
  {"log",   luai_math_log},
  {"max",   luai_math_max},
  {"min",   luai_math_min},
  {"modf",   luai_math_modf},
  {"rad",   luai_math_rad},
  {"random",     luai_math_random},
  {"randomseed", luai_math_randomseed},
  {"sin",   luai_math_sin},
  {"sqrt",  luai_math_sqrt},
  {"tan",   luai_math_tan},
  {"type", luai_math_type},
#if defined(LUA_COMPAT_MATHLIB)
  {"atan2", luai_math_atan},
  {"cosh",   luai_math_cosh},
  {"sinh",   luai_math_sinh},
  {"tanh",   luai_math_tanh},
  {"pow",   luai_math_pow},
  {"frexp", luai_math_frexp},
  {"ldexp", luai_math_ldexp},
  {"log10", luai_math_log10},
#endif
  /* placeholders */
  {"pi", NULL},
  {"huge", NULL},
  {"maxinteger", NULL},
  {"mininteger", NULL},
  {NULL, NULL}
};


/*
** Open math library
*/
LUAMOD_API int luaopen_math (lua_State *L) {
  luaL_newlib(L, luai_mathlib);
  lua_pushnumber(L, LUA_PI);
  lua_setfield(L, -2, "pi");
  lua_pushnumber(L, (lua_Number)HUGE_VAL);
  lua_setfield(L, -2, "huge");
  lua_pushinteger(L, LUA_MAXINTEGER);
  lua_setfield(L, -2, "maxinteger");
  lua_pushinteger(L, LUA_MININTEGER);
  lua_setfield(L, -2, "mininteger");
  return 1;
}

/*__lmem.c__*/

/*
** About the realloc function:
** void * frealloc (void *ud, void *ptr, size_t osize, size_t nsize);
** ('osize' is the old size, 'nsize' is the new size)
**
** * frealloc(ud, NULL, x, s) creates a new luai_getblock of size 's' (no
** matter 'x').
**
** * frealloc(ud, p, x, 0) frees the luai_getblock 'p'
** (in this specific case, frealloc must return NULL);
** particularly, frealloc(ud, NULL, 0, 0) does nothing
** (which is equivalent to free(NULL) in ISO C)
**
** frealloc returns NULL if it cannot create or reallocate the area
** (any reallocation to an equal or smaller size cannot fail!)
*/



#define MINSIZEARRAY	4


void *luaM_growaux_ (lua_State *L, void *luai_getblock, int *size, size_t size_elems,
                     int limit, const char *what) {
  void *newblock;
  int newsize;
  if (*size >= limit/2) {  /* cannot double it? */
    if (*size >= limit)  /* cannot grow even a little? */
      luaG_runerror(L, "too many %s (limit is %d)", what, limit);
    newsize = limit;  /* still have at least one free place */
  }
  else {
    newsize = (*size)*2;
    if (newsize < MINSIZEARRAY)
      newsize = MINSIZEARRAY;  /* minimum size */
  }
  newblock = luaM_reallocv(L, luai_getblock, *size, newsize, size_elems);
  *size = newsize;  /* update only when everything else is OK */
  return newblock;
}


luai_l_noret luaM_toobig (lua_State *L) {
  luaG_runerror(L, "memory allocation error: luai_getblock too big");
}



/*
** generic allocation routine.
*/
void *luaM_realloc_ (lua_State *L, void *luai_getblock, size_t osize, size_t nsize) {
  void *newblock;
  luai_global_State *g = LUAI_G(L);
  size_t realosize = (luai_getblock) ? osize : 0;
  lua_assert((realosize == 0) == (luai_getblock == NULL));
#if defined(HARDMEMTESTS)
  if (nsize > realosize && g->gcrunning)
    luaC_fullgc(L, 1);  /* force a LUAI_GC whenever possible */
#endif
  newblock = (*g->frealloc)(g->ud, luai_getblock, osize, nsize);
  if (newblock == NULL && nsize > 0) {
    lua_assert(nsize > realosize);  /* cannot fail when shrinking a luai_getblock */
    if (g->version) {  /* is state fully built? */
      luaC_fullgc(L, 1);  /* try to free some memory... */
      newblock = (*g->frealloc)(g->ud, luai_getblock, osize, nsize);  /* try again */
    }
    if (newblock == NULL)
      luaD_throw(L, LUA_ERRMEM);
  }
  lua_assert((nsize == 0) == (newblock == NULL));
  g->LUAI_GCdebt = (g->LUAI_GCdebt + nsize) - realosize;
  return newblock;
}

/*__loadlib.c__*/

/*
** LUA_IGMARK is a mark to ignore all before it when building the
** luaopen_ function name.
*/
#if !defined (LUA_IGMARK)
#define LUA_IGMARK		"-"
#endif


/*
** LUA_CSUBSEP is the character that replaces dots in submodule names
** when searching for a C loader.
** LUA_LSUBSEP is the character that replaces dots in submodule names
** when searching for a Lua loader.
*/
#if !defined(LUA_CSUBSEP)
#define LUA_CSUBSEP		LUA_DIRSEP
#endif

#if !defined(LUA_LSUBSEP)
#define LUA_LSUBSEP		LUA_DIRSEP
#endif


/* prefix for open functions in C libraries */
#define LUA_POF		"luaopen_"

/* separator for open functions in C libraries */
#define LUA_OFSEP	"_"


/*
** unique key for table in the registry that keeps handles
** for all loaded C libraries
*/
static const int LUAI_CLIBS = 0;

#define LUAI_LIB_FAIL	"open"


#define luai_setprogdir(L)           ((void)0)


/*
** system-dependent functions
*/

/*
** unload library 'lib'
*/
static void luai_lsys_unloadlib (void *lib);

/*
** load C library in file 'path'. If 'seeglb', load with all names in
** the library global.
** Returns the library; in case of error, returns NULL plus an
** error string in the stack.
*/
static void *luai_lsys_load (lua_State *L, const char *path, int seeglb);

/*
** Try to find a function named 'sym' in library 'lib'.
** Returns the function; in case of error, returns NULL plus an
** error string in the stack.
*/
static lua_CFunction luai_lsys_sym (lua_State *L, void *lib, const char *sym);




#if defined(LUA_USE_DLOPEN)	/* { */
/*
** {========================================================================
** This is an implementation of loadlib based on the dlfcn interface.
** The dlfcn interface is available in Linux, SunOS, Solaris, IRIX, FreeBSD,
** NetBSD, AIX 4.2, HPUX 11, and  probably most other Unix flavors, at least
** as an emulation layer on top of native functions.
** =========================================================================
*/

#include <dlfcn.h>

/*
** Macro to convert pointer-to-void* to pointer-to-function. This luai_cast
** is undefined according to ISO C, but POSIX assumes that it works.
** (The '__extension__' in gnu compilers is only to avoid warnings.)
*/
#if defined(__GNUC__)
#define luai_cast_func(p) (__extension__ (lua_CFunction)(p))
#else
#define luai_cast_func(p) ((lua_CFunction)(p))
#endif


static void luai_lsys_unloadlib (void *lib) {
  dlclose(lib);
}


static void *luai_lsys_load (lua_State *L, const char *path, int seeglb) {
  void *lib = dlopen(path, RTLD_NOW | (seeglb ? RTLD_GLOBAL : RTLD_LOCAL));
  if (lib == NULL) lua_pushstring(L, dlerror());
  return lib;
}


static lua_CFunction luai_lsys_sym (lua_State *L, void *lib, const char *sym) {
  lua_CFunction f = luai_cast_func(dlsym(lib, sym));
  if (f == NULL) lua_pushstring(L, dlerror());
  return f;
}

/* }====================================================== */



#elif defined(LUA_DL_DLL)	/* }{ */
/*
** {======================================================================
** This is an implementation of loadlib for Windows using native functions.
** =======================================================================
*/

#include <windows.h>


/*
** optional flags for LoadLibraryEx
*/
#if !defined(LUA_LLE_FLAGS)
#define LUA_LLE_FLAGS	0
#endif


#undef luai_setprogdir


/*
** Replace in the path (on the top of the stack) any occurrence
** of LUA_EXEC_DIR with the executable's path.
*/
static void luai_setprogdir (lua_State *L) {
  char buff[MAX_PATH + 1];
  char *lb;
  DWORD nsize = sizeof(buff)/sizeof(char);
  DWORD n = GetModuleFileNameA(NULL, buff, nsize);  /* get exec. name */
  if (n == 0 || n == nsize || (lb = strrchr(buff, '\\')) == NULL)
    luaL_error(L, "unable to get ModuleFileName");
  else {
    *lb = '\0';  /* cut name on the last '\\' to get the path */
    luaL_gsub(L, lua_tostring(L, -1), LUA_EXEC_DIR, buff);
    lua_remove(L, -2);  /* remove original string */
  }
}




static void luai_pusherror (lua_State *L) {
  int error = GetLastError();
  char buffer[128];
  if (FormatMessageA(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
      NULL, error, 0, buffer, sizeof(buffer)/sizeof(char), NULL))
    lua_pushstring(L, buffer);
  else
    lua_pushfstring(L, "system error %d\n", error);
}

static void luai_lsys_unloadlib (void *lib) {
  FreeLibrary((HMODULE)lib);
}


static void *luai_lsys_load (lua_State *L, const char *path, int seeglb) {
  HMODULE lib = LoadLibraryExA(path, NULL, LUA_LLE_FLAGS);
  (void)(seeglb);  /* not used: symbols are 'global' by default */
  if (lib == NULL) luai_pusherror(L);
  return lib;
}


static lua_CFunction luai_lsys_sym (lua_State *L, void *lib, const char *sym) {
  lua_CFunction f = (lua_CFunction)GetProcAddress((HMODULE)lib, sym);
  if (f == NULL) luai_pusherror(L);
  return f;
}

/* }====================================================== */


#else				/* }{ */
/*
** {======================================================
** Fallback for other systems
** =======================================================
*/

#undef LUAI_LIB_FAIL
#define LUAI_LIB_FAIL	"absent"


#define LUAI_DLMSG	"dynamic libraries not enabled; luai_check your Lua installation"


static void luai_lsys_unloadlib (void *lib) {
  (void)(lib);  /* not used */
}


static void *luai_lsys_load (lua_State *L, const char *path, int seeglb) {
  (void)(path); (void)(seeglb);  /* not used */
  lua_pushliteral(L, LUAI_DLMSG);
  return NULL;
}


static lua_CFunction luai_lsys_sym (lua_State *L, void *lib, const char *sym) {
  (void)(lib); (void)(sym);  /* not used */
  lua_pushliteral(L, LUAI_DLMSG);
  return NULL;
}

/* }====================================================== */
#endif				/* } */


/*
** {==================================================================
** Set Paths
** ===================================================================
*/

/*
** LUA_PATH_VAR and LUA_CPATH_VAR are the names of the environment
** variables that Lua luai_check to set its paths.
*/
#if !defined(LUA_PATH_VAR)
#define LUA_PATH_VAR    "LUA_PATH"
#endif

#if !defined(LUA_CPATH_VAR)
#define LUA_CPATH_VAR   "LUA_CPATH"
#endif


#define LUAI_AUXMARK         "\1"	/* auxiliary mark */


/*
** return registry.LUA_NOENV as a boolean
*/
static int luai_noenv (lua_State *L) {
  int b;
  lua_getfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  b = lua_toboolean(L, -1);
  lua_pop(L, 1);  /* remove value */
  return b;
}


/*
** Set a path
*/
static void luai_setpath (lua_State *L, const char *fieldname,
                                   const char *envname,
                                   const char *dft) {
  const char *nver = lua_pushfstring(L, "%s%s", envname, LUA_VERSUFFIX);
  const char *path = getenv(nver);  /* use versioned name */
  if (path == NULL)  /* no environment variable? */
    path = getenv(envname);  /* try unversioned name */
  if (path == NULL || luai_noenv(L))  /* no environment variable? */
    lua_pushstring(L, dft);  /* use default */
  else {
    /* replace ";;" by ";LUAI_AUXMARK;" and then LUAI_AUXMARK by default path */
    path = luaL_gsub(L, path, LUA_PATH_SEP LUA_PATH_SEP,
                              LUA_PATH_SEP LUAI_AUXMARK LUA_PATH_SEP);
    luaL_gsub(L, path, LUAI_AUXMARK, dft);
    lua_remove(L, -2); /* remove result from 1st 'gsub' */
  }
  luai_setprogdir(L);
  lua_setfield(L, -3, fieldname);  /* package[fieldname] = path value */
  lua_pop(L, 1);  /* pop versioned variable name */
}

/* }================================================================== */


/*
** return registry.LUAI_CLIBS[path]
*/
static void *luai_checkclib (lua_State *L, const char *path) {
  void *plib;
  lua_rawgetp(L, LUA_REGISTRYINDEX, &LUAI_CLIBS);
  lua_getfield(L, -1, path);
  plib = lua_touserdata(L, -1);  /* plib = LUAI_CLIBS[path] */
  lua_pop(L, 2);  /* pop LUAI_CLIBS table and 'plib' */
  return plib;
}


/*
** registry.LUAI_CLIBS[path] = plib        -- for queries
** registry.LUAI_CLIBS[#LUAI_CLIBS + 1] = plib  -- also keep a list of all libraries
*/
static void luai_addtoclib (lua_State *L, const char *path, void *plib) {
  lua_rawgetp(L, LUA_REGISTRYINDEX, &LUAI_CLIBS);
  lua_pushlightuserdata(L, plib);
  lua_pushvalue(L, -1);
  lua_setfield(L, -3, path);  /* LUAI_CLIBS[path] = plib */
  lua_rawseti(L, -2, luaL_len(L, -2) + 1);  /* LUAI_CLIBS[#LUAI_CLIBS + 1] = plib */
  lua_pop(L, 1);  /* pop LUAI_CLIBS table */
}


/*
** __gc tag method for LUAI_CLIBS table: calls 'luai_lsys_unloadlib' for all lib
** handles in list LUAI_CLIBS
*/
static int luai_gctm (lua_State *L) {
  lua_Integer n = luaL_len(L, 1);
  for (; n >= 1; n--) {  /* for each handle, in reverse order */
    lua_rawgeti(L, 1, n);  /* get handle LUAI_CLIBS[n] */
    luai_lsys_unloadlib(lua_touserdata(L, -1));
    lua_pop(L, 1);  /* pop handle */
  }
  return 0;
}



/* error codes for 'luai_lookforfunc' */
#define LUAI_ERRLIB		1
#define LUAI_ERRFUNC		2

/*
** Look for a C function named 'sym' in a dynamically loaded library
** 'path'.
** First, luai_check whether the library is already loaded; if not, try
** to load it.
** Then, if 'sym' is '*', return true (as library has been loaded).
** Otherwise, look for symbol 'sym' in the library and push a
** C function with that symbol.
** Return 0 and 'true' or a function in the stack; in case of
** errors, return an error code and an error message in the stack.
*/
static int luai_lookforfunc (lua_State *L, const char *path, const char *sym) {
  void *reg = luai_checkclib(L, path);  /* luai_check loaded C libraries */
  if (reg == NULL) {  /* must load library? */
    reg = luai_lsys_load(L, path, *sym == '*');  /* global symbols if 'sym'=='*' */
    if (reg == NULL) return LUAI_ERRLIB;  /* unable to load library */
    luai_addtoclib(L, path, reg);
  }
  if (*sym == '*') {  /* loading only library (no function)? */
    lua_pushboolean(L, 1);  /* return 'true' */
    return 0;  /* no errors */
  }
  else {
    lua_CFunction f = luai_lsys_sym(L, reg, sym);
    if (f == NULL)
      return LUAI_ERRFUNC;  /* unable to find function */
    lua_pushcfunction(L, f);  /* else create new function */
    return 0;  /* no errors */
  }
}


static int luai_ll_loadlib (lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *init = luaL_checkstring(L, 2);
  int stat = luai_lookforfunc(L, path, init);
  if (stat == 0)  /* no errors? */
    return 1;  /* return the loaded function */
  else {  /* error; error message is on stack top */
    lua_pushnil(L);
    lua_insert(L, -2);
    lua_pushstring(L, (stat == LUAI_ERRLIB) ?  LUAI_LIB_FAIL : "init");
    return 3;  /* return nil, error message, and where */
  }
}



/*
** {======================================================
** 'require' function
** =======================================================
*/


static int luai_readable (const char *filename) {
  FILE *f = fopen(filename, "r");  /* try to open file */
  if (f == NULL) return 0;  /* open failed */
  fclose(f);
  return 1;
}


static const char *luai_pushnexttemplate (lua_State *L, const char *path) {
  const char *l;
  while (*path == *LUA_PATH_SEP) path++;  /* skip separators */
  if (*path == '\0') return NULL;  /* no more templates */
  l = strchr(path, *LUA_PATH_SEP);  /* find luai_next separator */
  if (l == NULL) l = path + strlen(path);
  lua_pushlstring(L, path, l - path);  /* template */
  return l;
}


static const char *luai_searchpath (lua_State *L, const char *name,
                                             const char *path,
                                             const char *sep,
                                             const char *dirsep) {
  luaL_Buffer msg;  /* to build error message */
  luaL_buffinit(L, &msg);
  if (*sep != '\0')  /* non-empty separator? */
    name = luaL_gsub(L, name, sep, dirsep);  /* replace it by 'dirsep' */
  while ((path = luai_pushnexttemplate(L, path)) != NULL) {
    const char *filename = luaL_gsub(L, lua_tostring(L, -1),
                                     LUA_PATH_MARK, name);
    lua_remove(L, -2);  /* remove path template */
    if (luai_readable(filename))  /* does file exist and is luai_readable? */
      return filename;  /* return that file name */
    lua_pushfstring(L, "\n\tno file '%s'", filename);
    lua_remove(L, -2);  /* remove file name */
    luaL_addvalue(&msg);  /* concatenate error msg. entry */
  }
  luaL_pushresult(&msg);  /* create error message */
  return NULL;  /* not found */
}


static int luai_ll_searchpath (lua_State *L) {
  const char *f = luai_searchpath(L, luaL_checkstring(L, 1),
                                luaL_checkstring(L, 2),
                                luaL_optstring(L, 3, "."),
                                luaL_optstring(L, 4, LUA_DIRSEP));
  if (f != NULL) return 1;
  else {  /* error message is on top of the stack */
    lua_pushnil(L);
    lua_insert(L, -2);
    return 2;  /* return nil + error message */
  }
}


static const char *luai_findfile (lua_State *L, const char *name,
                                           const char *pname,
                                           const char *dirsep) {
  const char *path;
  lua_getfield(L, lua_upvalueindex(1), pname);
  path = lua_tostring(L, -1);
  if (path == NULL)
    luaL_error(L, "'package.%s' must be a string", pname);
  return luai_searchpath(L, name, path, ".", dirsep);
}


static int luai_checkload (lua_State *L, int stat, const char *filename) {
  if (stat) {  /* module loaded successfully? */
    lua_pushstring(L, filename);  /* will be 2nd argument to module */
    return 2;  /* return open function and file name */
  }
  else
    return luaL_error(L, "error loading module '%s' from file '%s':\n\t%s",
                          lua_tostring(L, 1), filename, lua_tostring(L, -1));
}


static int luai_searcher_Lua (lua_State *L) {
  const char *filename;
  const char *name = luaL_checkstring(L, 1);
  filename = luai_findfile(L, name, "path", LUA_LSUBSEP);
  if (filename == NULL) return 1;  /* module not found in this path */
  return luai_checkload(L, (luaL_loadfile(L, filename) == LUA_OK), filename);
}


/*
** Try to find a load function for module 'modname' at file 'filename'.
** First, change '.' to '_' in 'modname'; then, if 'modname' has
** the form X-Y (that is, it has an "ignore mark"), build a function
** name "luaopen_X" and look for it. (For compatibility, if that
** fails, it also tries "luaopen_Y".) If there is no ignore mark,
** look for a function named "luaopen_modname".
*/
static int luai_loadfunc (lua_State *L, const char *filename, const char *modname) {
  const char *openfunc;
  const char *mark;
  modname = luaL_gsub(L, modname, ".", LUA_OFSEP);
  mark = strchr(modname, *LUA_IGMARK);
  if (mark) {
    int stat;
    openfunc = lua_pushlstring(L, modname, mark - modname);
    openfunc = lua_pushfstring(L, LUA_POF"%s", openfunc);
    stat = luai_lookforfunc(L, filename, openfunc);
    if (stat != LUAI_ERRFUNC) return stat;
    modname = mark + 1;  /* else go ahead and try old-style name */
  }
  openfunc = lua_pushfstring(L, LUA_POF"%s", modname);
  return luai_lookforfunc(L, filename, openfunc);
}


static int luai_searcher_C (lua_State *L) {
  const char *name = luaL_checkstring(L, 1);
  const char *filename = luai_findfile(L, name, "cpath", LUA_CSUBSEP);
  if (filename == NULL) return 1;  /* module not found in this path */
  return luai_checkload(L, (luai_loadfunc(L, filename, name) == 0), filename);
}


static int luai_searcher_Croot (lua_State *L) {
  const char *filename;
  const char *name = luaL_checkstring(L, 1);
  const char *p = strchr(name, '.');
  int stat;
  if (p == NULL) return 0;  /* is root */
  lua_pushlstring(L, name, p - name);
  filename = luai_findfile(L, lua_tostring(L, -1), "cpath", LUA_CSUBSEP);
  if (filename == NULL) return 1;  /* root not found */
  if ((stat = luai_loadfunc(L, filename, name)) != 0) {
    if (stat != LUAI_ERRFUNC)
      return luai_checkload(L, 0, filename);  /* real error */
    else {  /* open function not found */
      lua_pushfstring(L, "\n\tno module '%s' in file '%s'", name, filename);
      return 1;
    }
  }
  lua_pushstring(L, filename);  /* will be 2nd argument to module */
  return 2;
}


static int luai_searcherpreload (lua_State *L) {
  const char *name = luaL_checkstring(L, 1);
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
  if (lua_getfield(L, -1, name) == LUA_TNIL)  /* not found? */
    lua_pushfstring(L, "\n\tno field package.preload['%s']", name);
  return 1;
}


static void luai_findloader (lua_State *L, const char *name) {
  int i;
  luaL_Buffer msg;  /* to build error message */
  luaL_buffinit(L, &msg);
  /* push 'package.searchers' to index 3 in the stack */
  if (lua_getfield(L, lua_upvalueindex(1), "searchers") != LUA_TTABLE)
    luaL_error(L, "'package.searchers' must be a table");
  /*  iterate over available searchers to find a loader */
  for (i = 1; ; i++) {
    if (lua_rawgeti(L, 3, i) == LUA_TNIL) {  /* no more searchers? */
      lua_pop(L, 1);  /* remove nil */
      luaL_pushresult(&msg);  /* create error message */
      luaL_error(L, "module '%s' not found:%s", name, lua_tostring(L, -1));
    }
    lua_pushstring(L, name);
    lua_call(L, 1, 2);  /* call it */
    if (lua_isfunction(L, -2))  /* did it find a loader? */
      return;  /* module loader found */
    else if (lua_isstring(L, -2)) {  /* searcher returned error message? */
      lua_pop(L, 1);  /* remove extra return */
      luaL_addvalue(&msg);  /* concatenate error message */
    }
    else
      lua_pop(L, 2);  /* remove both returns */
  }
}


static int luai_ll_require (lua_State *L) {
  const char *name = luaL_checkstring(L, 1);
  lua_settop(L, 1);  /* LOADED table will be at index 2 */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  lua_getfield(L, 2, name);  /* LOADED[name] */
  if (lua_toboolean(L, -1))  /* is it there? */
    return 1;  /* package is already loaded */
  /* else must load package */
  lua_pop(L, 1);  /* remove 'luai_getfield' result */
  luai_findloader(L, name);
  lua_pushstring(L, name);  /* pass name as argument to module loader */
  lua_insert(L, -2);  /* name is 1st argument (before search data) */
  lua_call(L, 2, 1);  /* run loader to load module */
  if (!lua_isnil(L, -1))  /* non-nil return? */
    lua_setfield(L, 2, name);  /* LOADED[name] = returned value */
  if (lua_getfield(L, 2, name) == LUA_TNIL) {   /* module set no value? */
    lua_pushboolean(L, 1);  /* use true as result */
    lua_pushvalue(L, -1);  /* extra copy to be returned */
    lua_setfield(L, 2, name);  /* LOADED[name] = true */
  }
  return 1;
}

/* }====================================================== */



/*
** {======================================================
** 'module' function
** =======================================================
*/
#if defined(LUA_COMPAT_MODULE)

/*
** changes the environment variable of calling function
*/
static void luai_set_env (lua_State *L) {
  lua_Debug ar;
  if (lua_getstack(L, 1, &ar) == 0 ||
      lua_getinfo(L, "f", &ar) == 0 ||  /* get calling function */
      lua_iscfunction(L, -1))
    luaL_error(L, "'module' not called from a Lua function");
  lua_pushvalue(L, -2);  /* copy new environment table to top */
  lua_setupvalue(L, -2, 1);
  lua_pop(L, 1);  /* remove function */
}


static void luai_dooptions (lua_State *L, int n) {
  int i;
  for (i = 2; i <= n; i++) {
    if (lua_isfunction(L, i)) {  /* avoid 'calling' extra info. */
      lua_pushvalue(L, i);  /* get option (a function) */
      lua_pushvalue(L, -2);  /* module */
      lua_call(L, 1, 0);
    }
  }
}


static void luai_modinit (lua_State *L, const char *modname) {
  const char *dot;
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "_M");  /* module._M = module */
  lua_pushstring(L, modname);
  lua_setfield(L, -2, "_NAME");
  dot = strrchr(modname, '.');  /* look for last dot in module name */
  if (dot == NULL) dot = modname;
  else dot++;
  /* set _PACKAGE as package name (full module name minus last part) */
  lua_pushlstring(L, modname, dot - modname);
  lua_setfield(L, -2, "_PACKAGE");
}


static int luai_ll_module (lua_State *L) {
  const char *modname = luaL_checkstring(L, 1);
  int lastarg = lua_gettop(L);  /* last parameter */
  luaL_pushmodule(L, modname, 1);  /* get/create module table */
  /* luai_check whether table already has a _NAME luai_field */
  if (lua_getfield(L, -1, "_NAME") != LUA_TNIL)
    lua_pop(L, 1);  /* table is an initialized module */
  else {  /* no; initialize it */
    lua_pop(L, 1);
    luai_modinit(L, modname);
  }
  lua_pushvalue(L, -1);
  luai_set_env(L);
  luai_dooptions(L, lastarg);
  return 1;
}


static int luai_ll_seeall (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  if (!lua_getmetatable(L, 1)) {
    lua_createtable(L, 0, 1); /* create new metatable */
    lua_pushvalue(L, -1);
    lua_setmetatable(L, 1);
  }
  lua_pushglobaltable(L);
  lua_setfield(L, -2, "__index");  /* mt.__index = _G */
  return 0;
}

#endif
/* }====================================================== */



static const luaL_Reg luai_pk_funcs[] = {
  {"loadlib", luai_ll_loadlib},
  {"searchpath", luai_ll_searchpath},
#if defined(LUA_COMPAT_MODULE)
  {"seeall", luai_ll_seeall},
#endif
  /* placeholders */
  {"preload", NULL},
  {"cpath", NULL},
  {"path", NULL},
  {"searchers", NULL},
  {"loaded", NULL},
  {NULL, NULL}
};


static const luaL_Reg luai_ll_funcs[] = {
#if defined(LUA_COMPAT_MODULE)
  {"module", luai_ll_module},
#endif
  {"require", luai_ll_require},
  {NULL, NULL}
};


static void luai_createsearcherstable (lua_State *L) {
  static const lua_CFunction searchers[] =
    {luai_searcherpreload, luai_searcher_Lua, luai_searcher_C, luai_searcher_Croot, NULL};
  int i;
  /* create 'searchers' table */
  lua_createtable(L, sizeof(searchers)/sizeof(searchers[0]) - 1, 0);
  /* fill it with predefined searchers */
  for (i=0; searchers[i] != NULL; i++) {
    lua_pushvalue(L, -2);  /* set 'package' as upvalue for all searchers */
    lua_pushcclosure(L, searchers[i], 1);
    lua_rawseti(L, -2, i+1);
  }
#if defined(LUA_COMPAT_LOADERS)
  lua_pushvalue(L, -1);  /* make a copy of 'searchers' table */
  lua_setfield(L, -3, "loaders");  /* put it in luai_field 'loaders' */
#endif
  lua_setfield(L, -2, "searchers");  /* put it in luai_field 'searchers' */
}


/*
** create table LUAI_CLIBS to keep track of loaded C libraries,
** setting a finalizer to close all libraries when closing state.
*/
static void luai_createclibstable (lua_State *L) {
  lua_newtable(L);  /* create LUAI_CLIBS table */
  lua_createtable(L, 0, 1);  /* create metatable for LUAI_CLIBS */
  lua_pushcfunction(L, luai_gctm);
  lua_setfield(L, -2, "__gc");  /* set finalizer for LUAI_CLIBS table */
  lua_setmetatable(L, -2);
  lua_rawsetp(L, LUA_REGISTRYINDEX, &LUAI_CLIBS);  /* set LUAI_CLIBS table in registry */
}


LUAMOD_API int luaopen_package (lua_State *L) {
  luai_createclibstable(L);
  luaL_newlib(L, luai_pk_funcs);  /* create 'package' table */
  luai_createsearcherstable(L);
  /* set paths */
  luai_setpath(L, "path", LUA_PATH_VAR, LUA_PATH_DEFAULT);
  luai_setpath(L, "cpath", LUA_CPATH_VAR, LUA_CPATH_DEFAULT);
  /* store config information */
  lua_pushliteral(L, LUA_DIRSEP "\n" LUA_PATH_SEP "\n" LUA_PATH_MARK "\n"
                     LUA_EXEC_DIR "\n" LUA_IGMARK "\n");
  lua_setfield(L, -2, "config");
  /* set luai_field 'loaded' */
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  lua_setfield(L, -2, "loaded");
  /* set luai_field 'preload' */
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
  lua_setfield(L, -2, "preload");
  lua_pushglobaltable(L);
  lua_pushvalue(L, -2);  /* set 'package' as upvalue for luai_next lib */
  luaL_setfuncs(L, luai_ll_funcs, 1);  /* open lib into global table */
  lua_pop(L, 1);  /* pop global table */
  return 1;  /* return 'package' table */
}

/*__lobject.c__*/

LUAI_DDEF const luai_TValue luaO_nilobject_ = {LUAI_NILCONSTANT};


/*
** converts an integer to a "floating point byte", represented as
** (eeeeexxx), where the real value is (1xxx) * 2^(eeeee - 1) if
** eeeee != 0 and (xxx) otherwise.
*/
int luaO_int2fb (unsigned int x) {
  int e = 0;  /* exponent */
  if (x < 8) return x;
  while (x >= (8 << 4)) {  /* coarse steps */
    x = (x + 0xf) >> 4;  /* x = ceil(x / 16) */
    e += 4;
  }
  while (x >= (8 << 1)) {  /* fine steps */
    x = (x + 1) >> 1;  /* x = ceil(x / 2) */
    e++;
  }
  return ((e+1) << 3) | (luai_cast_int(x) - 8);
}


/* converts back */
int luaO_fb2int (int x) {
  return (x < 8) ? x : ((x & 7) + 8) << ((x >> 3) - 1);
}


/*
** Computes ceil(log2(x))
*/
int luaO_ceillog2 (unsigned int x) {
  static const luai_lu_byte log_2[256] = {  /* log_2[i] = ceil(log2(i - 1)) */
    0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
  };
  int l = 0;
  x--;
  while (x >= 256) { l += 8; x >>= 8; }
  return l + log_2[x];
}


static lua_Integer luai_intarith (lua_State *L, int op, lua_Integer v1,
                                                   lua_Integer v2) {
  switch (op) {
    case LUA_OPADD: return luai_intop(+, v1, v2);
    case LUA_OPSUB:return luai_intop(-, v1, v2);
    case LUA_OPMUL:return luai_intop(*, v1, v2);
    case LUA_OPMOD: return luaV_mod(L, v1, v2);
    case LUA_OPIDIV: return luaV_div(L, v1, v2);
    case LUA_OPBAND: return luai_intop(&, v1, v2);
    case LUA_OPBOR: return luai_intop(|, v1, v2);
    case LUA_OPBXOR: return luai_intop(^, v1, v2);
    case LUA_OPSHL: return luaV_shiftl(v1, v2);
    case LUA_OPSHR: return luaV_shiftl(v1, -v2);
    case LUA_OPUNM: return luai_intop(-, 0, v1);
    case LUA_OPBNOT: return luai_intop(^, ~luai_l_castS2U(0), v1);
    default: lua_assert(0); return 0;
  }
}


static lua_Number luai_numarith (lua_State *L, int op, lua_Number v1,
                                                  lua_Number v2) {
  switch (op) {
    case LUA_OPADD: return luai_numadd(L, v1, v2);
    case LUA_OPSUB: return luai_numsub(L, v1, v2);
    case LUA_OPMUL: return luai_nummul(L, v1, v2);
    case LUA_OPDIV: return luai_numdiv(L, v1, v2);
    case LUA_OPPOW: return luai_numpow(L, v1, v2);
    case LUA_OPIDIV: return luai_numidiv(L, v1, v2);
    case LUA_OPUNM: return luai_numunm(L, v1);
    case LUA_OPMOD: {
      lua_Number m;
      luai_nummod(L, v1, v2, m);
      return m;
    }
    default: lua_assert(0); return 0;
  }
}


void luaO_arith (lua_State *L, int op, const luai_TValue *p1, const luai_TValue *p2,
                 luai_TValue *res) {
  switch (op) {
    case LUA_OPBAND: case LUA_OPBOR: case LUA_OPBXOR:
    case LUA_OPSHL: case LUA_OPSHR:
    case LUA_OPBNOT: {  /* operate only on integers */
      lua_Integer i1; lua_Integer i2;
      if (luai_tointeger(p1, &i1) && luai_tointeger(p2, &i2)) {
        luai_setivalue(res, luai_intarith(L, op, i1, i2));
        return;
      }
      else break;  /* go to the end */
    }
    case LUA_OPDIV: case LUA_OPPOW: {  /* operate only on floats */
      lua_Number n1; lua_Number n2;
      if (luai_tonumber(p1, &n1) && luai_tonumber(p2, &n2)) {
        luai_setfltvalue(res, luai_numarith(L, op, n1, n2));
        return;
      }
      else break;  /* go to the end */
    }
    default: {  /* other operations */
      lua_Number n1; lua_Number n2;
      if (luai_ttisinteger(p1) && luai_ttisinteger(p2)) {
        luai_setivalue(res, luai_intarith(L, op, luai_ivalue(p1), luai_ivalue(p2)));
        return;
      }
      else if (luai_tonumber(p1, &n1) && luai_tonumber(p2, &n2)) {
        luai_setfltvalue(res, luai_numarith(L, op, n1, n2));
        return;
      }
      else break;  /* go to the end */
    }
  }
  /* could not perform raw operation; try metamethod */
  lua_assert(L != NULL);  /* should not fail when folding (compile time) */
  luaT_trybinTM(L, p1, p2, res, luai_cast(luai_TMS, (op - LUA_OPADD) + LUAI_TM_ADD));
}


int luaO_hexavalue (int c) {
  if (luai_lisdigit(c)) return c - '0';
  else return (luai_tolower(c) - 'a') + 10;
}


static int luai_isneg (const char **s) {
  if (**s == '-') { (*s)++; return 1; }
  else if (**s == '+') (*s)++;
  return 0;
}



/*
** {==================================================================
** Lua's implementation for 'lua_strx2number'
** ===================================================================
*/

#if !defined(lua_strx2number)

/* maximum number of significant digits to read (to avoid overflows
   even with single floats) */
#define LUAI_MAXSIGDIG	30

/*
** convert an hexadecimal numeric string to a number, following
** C99 specification for 'strtod'
*/
static lua_Number lua_strx2number (const char *s, char **endptr) {
  int dot = lua_getlocaledecpoint();
  lua_Number r = 0.0;  /* result (accumulator) */
  int sigdig = 0;  /* number of significant digits */
  int nosigdig = 0;  /* number of non-significant digits */
  int e = 0;  /* exponent correction */
  int neg;  /* 1 if number is negative */
  int hasdot = 0;  /* true after seen a dot */
  *endptr = luai_cast(char *, s);  /* nothing is valid yet */
  while (luai_lisspace(luai_cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = luai_isneg(&s);  /* luai_check signal */
  if (!(*s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X')))  /* luai_check '0x' */
    return 0.0;  /* invalid format (no '0x') */
  for (s += 2; ; s++) {  /* skip '0x' and read numeral */
    if (*s == dot) {
      if (hasdot) break;  /* second dot? stop loop */
      else hasdot = 1;
    }
    else if (luai_lisxdigit(luai_cast_uchar(*s))) {
      if (sigdig == 0 && *s == '0')  /* non-significant luai_digit (zero)? */
        nosigdig++;
      else if (++sigdig <= LUAI_MAXSIGDIG)  /* can read it without overflow? */
          r = (r * luai_cast_num(16.0)) + luaO_hexavalue(*s);
      else e++; /* too many digits; ignore, but still count for exponent */
      if (hasdot) e--;  /* decimal luai_digit? correct exponent */
    }
    else break;  /* neither a dot nor a luai_digit */
  }
  if (nosigdig + sigdig == 0)  /* no digits? */
    return 0.0;  /* invalid format */
  *endptr = luai_cast(char *, s);  /* valid up to here */
  e *= 4;  /* each luai_digit multiplies/divides value by 2^4 */
  if (*s == 'p' || *s == 'P') {  /* exponent part? */
    int luai_exp1 = 0;  /* exponent value */
    int neg1;  /* exponent signal */
    s++;  /* skip 'p' */
    neg1 = luai_isneg(&s);  /* signal */
    if (!luai_lisdigit(luai_cast_uchar(*s)))
      return 0.0;  /* invalid; must have at least one luai_digit */
    while (luai_lisdigit(luai_cast_uchar(*s)))  /* read exponent */
      luai_exp1 = luai_exp1 * 10 + *(s++) - '0';
    if (neg1) luai_exp1 = -luai_exp1;
    e += luai_exp1;
    *endptr = luai_cast(char *, s);  /* valid up to here */
  }
  if (neg) r = -r;
  return luai_l_mathop(ldexp)(r, e);
}

#endif
/* }====================================================== */


/* maximum length of a numeral */
#if !defined (LUAI_L_MAXLENNUM)
#define LUAI_L_MAXLENNUM	200
#endif

static const char *luai_l_str2dloc (const char *s, lua_Number *result, int mode) {
  char *endptr;
  *result = (mode == 'x') ? lua_strx2number(s, &endptr)  /* try to convert */
                          : lua_str2number(s, &endptr);
  if (endptr == s) return NULL;  /* nothing recognized? */
  while (luai_lisspace(luai_cast_uchar(*endptr))) endptr++;  /* skip trailing spaces */
  return (*endptr == '\0') ? endptr : NULL;  /* OK if no trailing characters */
}


/*
** Convert string 's' to a Lua number (put in 'result'). Return NULL
** on fail or the address of the ending '\0' on success.
** 'pmode' points to (and 'mode' contains) special things in the string:
** - 'x'/'X' means an hexadecimal numeral
** - 'n'/'N' means 'inf' or 'nan' (which should be rejected)
** - '.' just optimizes the search for the common case (nothing special)
** This function accepts both the current locale or a dot as the radix
** mark. If the convertion fails, it may mean number has a dot but
** locale accepts something else. In that case, the code copies 's'
** to a buffer (because 's' is read-only), changes the dot to the
** current locale radix mark, and tries to convert again.
*/
static const char *luai_l_str2d (const char *s, lua_Number *result) {
  const char *endptr;
  const char *pmode = strpbrk(s, ".xXnN");
  int mode = pmode ? luai_tolower(luai_cast_uchar(*pmode)) : 0;
  if (mode == 'n')  /* reject 'inf' and 'nan' */
    return NULL;
  endptr = luai_l_str2dloc(s, result, mode);  /* try to convert */
  if (endptr == NULL) {  /* failed? may be a different locale */
    char buff[LUAI_L_MAXLENNUM + 1];
    const char *pdot = strchr(s, '.');
    if (strlen(s) > LUAI_L_MAXLENNUM || pdot == NULL)
      return NULL;  /* string too long or no dot; fail */
    strcpy(buff, s);  /* copy string to buffer */
    buff[pdot - s] = lua_getlocaledecpoint();  /* correct decimal point */
    endptr = luai_l_str2dloc(buff, result, mode);  /* try again */
    if (endptr != NULL)
      endptr = s + (endptr - buff);  /* make relative to 's' */
  }
  return endptr;
}


#define LUAI_MAXBY10		luai_cast(lua_Unsigned, LUA_MAXINTEGER / 10)
#define LUAI_MAXLASTD	luai_cast_int(LUA_MAXINTEGER % 10)

static const char *luai_l_str2int (const char *s, lua_Integer *result) {
  lua_Unsigned a = 0;
  int empty = 1;
  int neg;
  while (luai_lisspace(luai_cast_uchar(*s))) s++;  /* skip initial spaces */
  neg = luai_isneg(&s);
  if (s[0] == '0' &&
      (s[1] == 'x' || s[1] == 'X')) {  /* hex? */
    s += 2;  /* skip '0x' */
    for (; luai_lisxdigit(luai_cast_uchar(*s)); s++) {
      a = a * 16 + luaO_hexavalue(*s);
      empty = 0;
    }
  }
  else {  /* decimal */
    for (; luai_lisdigit(luai_cast_uchar(*s)); s++) {
      int d = *s - '0';
      if (a >= LUAI_MAXBY10 && (a > LUAI_MAXBY10 || d > LUAI_MAXLASTD + neg))  /* overflow? */
        return NULL;  /* do not accept it (as integer) */
      a = a * 10 + d;
      empty = 0;
    }
  }
  while (luai_lisspace(luai_cast_uchar(*s))) s++;  /* skip trailing spaces */
  if (empty || *s != '\0') return NULL;  /* something wrong in the numeral */
  else {
    *result = luai_l_castU2S((neg) ? 0u - a : a);
    return s;
  }
}


size_t luaO_str2num (const char *s, luai_TValue *o) {
  lua_Integer i; lua_Number n;
  const char *e;
  if ((e = luai_l_str2int(s, &i)) != NULL) {  /* try as an integer */
    luai_setivalue(o, i);
  }
  else if ((e = luai_l_str2d(s, &n)) != NULL) {  /* else try as a float */
    luai_setfltvalue(o, n);
  }
  else
    return 0;  /* conversion failed */
  return (e - s) + 1;  /* success; return string size */
}


int luaO_utf8esc (char *buff, unsigned long x) {
  int n = 1;  /* number of bytes put in buffer (backwards) */
  lua_assert(x <= 0x10FFFF);
  if (x < 0x80)  /* ascii? */
    buff[LUAI_UTF8BUFFSZ - 1] = luai_cast(char, x);
  else {  /* need continuation bytes */
    unsigned int mfb = 0x3f;  /* maximum that fits in first byte */
    do {  /* add continuation bytes */
      buff[LUAI_UTF8BUFFSZ - (n++)] = luai_cast(char, 0x80 | (x & 0x3f));
      x >>= 6;  /* remove added bits */
      mfb >>= 1;  /* now there is one less bit available in first byte */
    } while (x > mfb);  /* still needs continuation byte? */
    buff[LUAI_UTF8BUFFSZ - n] = luai_cast(char, (~mfb << 1) | x);  /* add first byte */
  }
  return n;
}


/* maximum length of the conversion of a number to a string */
#define LUAI_MAXNUMBER2STR	50


/*
** Convert a number object to a string
*/
void luaO_tostring (lua_State *L, luai_StkId obj) {
  char buff[LUAI_MAXNUMBER2STR];
  size_t len;
  lua_assert(luai_ttisnumber(obj));
  if (luai_ttisinteger(obj))
    len = lua_integer2str(buff, sizeof(buff), luai_ivalue(obj));
  else {
    len = lua_number2str(buff, sizeof(buff), luai_fltvalue(obj));
#if !defined(LUA_COMPAT_FLOATSTRING)
    if (buff[strspn(buff, "-0123456789")] == '\0') {  /* looks like an int? */
      buff[len++] = lua_getlocaledecpoint();
      buff[len++] = '0';  /* adds '.0' to result */
    }
#endif
  }
  luai_setsvalue2s(L, obj, luaS_newlstr(L, buff, len));
}


static void luai_pushstr (lua_State *L, const char *str, size_t l) {
  luai_setsvalue2s(L, L->top, luaS_newlstr(L, str, l));
  luaD_inctop(L);
}


/*
** this function handles only '%d', '%c', '%f', '%p', and '%s'
   conventional formats, plus Lua-specific '%I' and '%U'
*/
const char *luaO_pushvfstring (lua_State *L, const char *fmt, va_list argp) {
  int n = 0;
  for (;;) {
    const char *e = strchr(fmt, '%');
    if (e == NULL) break;
    luai_pushstr(L, fmt, e - fmt);
    switch (*(e+1)) {
      case 's': {  /* zero-terminated string */
        const char *s = va_arg(argp, char *);
        if (s == NULL) s = "(null)";
        luai_pushstr(L, s, strlen(s));
        break;
      }
      case 'c': {  /* an 'int' as a character */
        char buff = luai_cast(char, va_arg(argp, int));
        if (luai_lisprint(luai_cast_uchar(buff)))
          luai_pushstr(L, &buff, 1);
        else  /* non-printable character; print its code */
          luaO_pushfstring(L, "<\\%d>", luai_cast_uchar(buff));
        break;
      }
      case 'd': {  /* an 'int' */
        luai_setivalue(L->top, va_arg(argp, int));
        goto top2str;
      }
      case 'I': {  /* a 'lua_Integer' */
        luai_setivalue(L->top, luai_cast(lua_Integer, va_arg(argp, luai_l_uacInt)));
        goto top2str;
      }
      case 'f': {  /* a 'lua_Number' */
        luai_setfltvalue(L->top, luai_cast_num(va_arg(argp, luai_l_uacNumber)));
      top2str:  /* convert the top element to a string */
        luaD_inctop(L);
        luaO_tostring(L, L->top - 1);
        break;
      }
      case 'p': {  /* a pointer */
        char buff[4*sizeof(void *) + 8]; /* should be enough space for a '%p' */
        int l = luai_l_sprintf(buff, sizeof(buff), "%p", va_arg(argp, void *));
        luai_pushstr(L, buff, l);
        break;
      }
      case 'U': {  /* an 'int' as a UTF-8 sequence */
        char buff[LUAI_UTF8BUFFSZ];
        int l = luaO_utf8esc(buff, luai_cast(long, va_arg(argp, long)));
        luai_pushstr(L, buff + LUAI_UTF8BUFFSZ - l, l);
        break;
      }
      case '%': {
        luai_pushstr(L, "%", 1);
        break;
      }
      default: {
        luaG_runerror(L, "invalid option '%%%c' to 'lua_pushfstring'",
                         *(e + 1));
      }
    }
    n += 2;
    fmt = e+2;
  }
  luaD_checkstack(L, 1);
  luai_pushstr(L, fmt, strlen(fmt));
  if (n > 0) luaV_concat(L, n + 1);
  return luai_svalue(L->top - 1);
}


const char *luaO_pushfstring (lua_State *L, const char *fmt, ...) {
  const char *msg;
  va_list argp;
  va_start(argp, fmt);
  msg = luaO_pushvfstring(L, fmt, argp);
  va_end(argp);
  return msg;
}


/* number of chars of a literal string without the ending \0 */
#define LL(x)	(sizeof(x)/sizeof(char) - 1)

#define RETS	"..."
#define PRE	"[string \""
#define POS	"\"]"

#define addstr(a,b,l)	( memcpy(a,b,(l) * sizeof(char)), a += (l) )

void luaO_chunkid (char *out, const char *source, size_t bufflen) {
  size_t l = strlen(source);
  if (*source == '=') {  /* 'literal' source */
    if (l <= bufflen)  /* small enough? */
      memcpy(out, source + 1, l * sizeof(char));
    else {  /* truncate it */
      addstr(out, source + 1, bufflen - 1);
      *out = '\0';
    }
  }
  else if (*source == '@') {  /* file name */
    if (l <= bufflen)  /* small enough? */
      memcpy(out, source + 1, l * sizeof(char));
    else {  /* add '...' before rest of name */
      addstr(out, RETS, LL(RETS));
      bufflen -= LL(RETS);
      memcpy(out, source + 1 + l - bufflen, bufflen * sizeof(char));
    }
  }
  else {  /* string; format as [string "source"] */
    const char *nl = strchr(source, '\n');  /* find first new line (if any) */
    addstr(out, PRE, LL(PRE));  /* add prefix */
    bufflen -= LL(PRE RETS POS) + 1;  /* luai_save space for prefix+suffix+'\0' */
    if (l < bufflen && nl == NULL) {  /* small one-line source? */
      addstr(out, source, l);  /* keep it */
    }
    else {
      if (nl != NULL) l = nl - source;  /* stop at first newline */
      if (l > bufflen) l = bufflen;
      addstr(out, source, l);
      addstr(out, RETS, LL(RETS));
    }
    memcpy(out, POS, (LL(POS) + 1) * sizeof(char));
  }
}

/*__lopcodes.c__*/

/* ORDER OP */

LUAI_DDEF const char *const luaP_opnames[luai_NUM_OPCODES+1] = {
  "MOVE",
  "LOADK",
  "LOADKX",
  "LOADBOOL",
  "LOADNIL",
  "GETUPVAL",
  "GETTABUP",
  "GETTABLE",
  "SETTABUP",
  "SETUPVAL",
  "SETTABLE",
  "NEWTABLE",
  "SELF",
  "ADD",
  "SUB",
  "MUL",
  "MOD",
  "POW",
  "DIV",
  "IDIV",
  "BAND",
  "BOR",
  "BXOR",
  "SHL",
  "SHR",
  "UNM",
  "BNOT",
  "NOT",
  "LEN",
  "CONCAT",
  "JMP",
  "EQ",
  "LT",
  "LE",
  "TEST",
  "TESTSET",
  "CALL",
  "TAILCALL",
  "RETURN",
  "FORLOOP",
  "FORPREP",
  "TFORCALL",
  "TFORLOOP",
  "SETLIST",
  "CLOSURE",
  "VARARG",
  "EXTRAARG",
  NULL
};


#define opmode(t,a,b,c,m) (((t)<<7) | ((a)<<6) | ((b)<<4) | ((c)<<2) | (m))

LUAI_DDEF const luai_lu_byte luaP_opmodes[luai_NUM_OPCODES] = {
/*       T  A    B       C     mode		   opcode	*/
  opmode(0, 1, luai_OpArgR, luai_OpArgN, luai_iABC)		/* luai_OP_MOVE */
 ,opmode(0, 1, luai_OpArgK, luai_OpArgN, luai_iABx)		/* luai_OP_LOADK */
 ,opmode(0, 1, luai_OpArgN, luai_OpArgN, luai_iABx)		/* luai_OP_LOADKX */
 ,opmode(0, 1, luai_OpArgU, luai_OpArgU, luai_iABC)		/* luai_OP_LOADBOOL */
 ,opmode(0, 1, luai_OpArgU, luai_OpArgN, luai_iABC)		/* luai_OP_LOADNIL */
 ,opmode(0, 1, luai_OpArgU, luai_OpArgN, luai_iABC)		/* luai_OP_GETUPVAL */
 ,opmode(0, 1, luai_OpArgU, luai_OpArgK, luai_iABC)		/* luai_OP_GETTABUP */
 ,opmode(0, 1, luai_OpArgR, luai_OpArgK, luai_iABC)		/* luai_OP_GETTABLE */
 ,opmode(0, 0, luai_OpArgK, luai_OpArgK, luai_iABC)		/* luai_OP_SETTABUP */
 ,opmode(0, 0, luai_OpArgU, luai_OpArgN, luai_iABC)		/* luai_OP_SETUPVAL */
 ,opmode(0, 0, luai_OpArgK, luai_OpArgK, luai_iABC)		/* luai_OP_SETTABLE */
 ,opmode(0, 1, luai_OpArgU, luai_OpArgU, luai_iABC)		/* luai_OP_NEWTABLE */
 ,opmode(0, 1, luai_OpArgR, luai_OpArgK, luai_iABC)		/* luai_OP_SELF */
 ,opmode(0, 1, luai_OpArgK, luai_OpArgK, luai_iABC)		/* luai_OP_ADD */
 ,opmode(0, 1, luai_OpArgK, luai_OpArgK, luai_iABC)		/* luai_OP_SUB */
 ,opmode(0, 1, luai_OpArgK, luai_OpArgK, luai_iABC)		/* luai_OP_MUL */
 ,opmode(0, 1, luai_OpArgK, luai_OpArgK, luai_iABC)		/* luai_OP_MOD */
 ,opmode(0, 1, luai_OpArgK, luai_OpArgK, luai_iABC)		/* luai_OP_POW */
 ,opmode(0, 1, luai_OpArgK, luai_OpArgK, luai_iABC)		/* luai_OP_DIV */
 ,opmode(0, 1, luai_OpArgK, luai_OpArgK, luai_iABC)		/* luai_OP_IDIV */
 ,opmode(0, 1, luai_OpArgK, luai_OpArgK, luai_iABC)		/* luai_OP_BAND */
 ,opmode(0, 1, luai_OpArgK, luai_OpArgK, luai_iABC)		/* luai_OP_BOR */
 ,opmode(0, 1, luai_OpArgK, luai_OpArgK, luai_iABC)		/* luai_OP_BXOR */
 ,opmode(0, 1, luai_OpArgK, luai_OpArgK, luai_iABC)		/* luai_OP_SHL */
 ,opmode(0, 1, luai_OpArgK, luai_OpArgK, luai_iABC)		/* luai_OP_SHR */
 ,opmode(0, 1, luai_OpArgR, luai_OpArgN, luai_iABC)		/* luai_OP_UNM */
 ,opmode(0, 1, luai_OpArgR, luai_OpArgN, luai_iABC)		/* luai_OP_BNOT */
 ,opmode(0, 1, luai_OpArgR, luai_OpArgN, luai_iABC)		/* luai_OP_NOT */
 ,opmode(0, 1, luai_OpArgR, luai_OpArgN, luai_iABC)		/* luai_OP_LEN */
 ,opmode(0, 1, luai_OpArgR, luai_OpArgR, luai_iABC)		/* luai_OP_CONCAT */
 ,opmode(0, 0, luai_OpArgR, luai_OpArgN, luai_iAsBx)		/* luai_OP_JMP */
 ,opmode(1, 0, luai_OpArgK, luai_OpArgK, luai_iABC)		/* luai_OP_EQ */
 ,opmode(1, 0, luai_OpArgK, luai_OpArgK, luai_iABC)		/* luai_OP_LT */
 ,opmode(1, 0, luai_OpArgK, luai_OpArgK, luai_iABC)		/* luai_OP_LE */
 ,opmode(1, 0, luai_OpArgN, luai_OpArgU, luai_iABC)		/* luai_OP_TEST */
 ,opmode(1, 1, luai_OpArgR, luai_OpArgU, luai_iABC)		/* luai_OP_TESTSET */
 ,opmode(0, 1, luai_OpArgU, luai_OpArgU, luai_iABC)		/* luai_OP_CALL */
 ,opmode(0, 1, luai_OpArgU, luai_OpArgU, luai_iABC)		/* luai_OP_TAILCALL */
 ,opmode(0, 0, luai_OpArgU, luai_OpArgN, luai_iABC)		/* luai_OP_RETURN */
 ,opmode(0, 1, luai_OpArgR, luai_OpArgN, luai_iAsBx)		/* luai_OP_FORLOOP */
 ,opmode(0, 1, luai_OpArgR, luai_OpArgN, luai_iAsBx)		/* luai_OP_FORPREP */
 ,opmode(0, 0, luai_OpArgN, luai_OpArgU, luai_iABC)		/* luai_OP_TFORCALL */
 ,opmode(0, 1, luai_OpArgR, luai_OpArgN, luai_iAsBx)		/* luai_OP_TFORLOOP */
 ,opmode(0, 0, luai_OpArgU, luai_OpArgU, luai_iABC)		/* luai_OP_SETLIST */
 ,opmode(0, 1, luai_OpArgU, luai_OpArgN, luai_iABx)		/* luai_OP_CLOSURE */
 ,opmode(0, 1, luai_OpArgU, luai_OpArgN, luai_iABC)		/* luai_OP_VARARG */
 ,opmode(0, 0, luai_OpArgU, luai_OpArgU, luai_iAx)		/* luai_OP_EXTRAARG */
};

/*__loslib.c__*/

/*
** {==================================================================
** List of valid conversion specifiers for the 'strftime' function;
** options are grouped by length; group of length 2 start with '||'.
** ===================================================================
*/
#if !defined(LUA_STRFTIMEOPTIONS)	/* { */

/* options for ANSI C 89 (only 1-char options) */
#define LUAI_L_STRFTIMEC89		"aAbBcdHIjmMpSUwWxXyYZ%"

/* options for ISO C 99 and POSIX */
#define LUAI_L_STRFTIMEC99 "aAbBcCdDeFgGhHIjmMnprRStTuUVwWxXyYzZ%" \
    "||" "EcECExEXEyEY" "OdOeOHOIOmOMOSOuOUOVOwOWOy"  /* two-char options */

/* options for Windows */
#define LUAI_L_STRFTIMEWIN "aAbBcdHIjmMpSUwWxXyYzZ%" \
    "||" "#c#x#d#H#I#j#m#M#S#U#w#W#y#Y"  /* two-char options */

#if defined(LUA_USE_WINDOWS)
#define LUA_STRFTIMEOPTIONS	LUAI_L_STRFTIMEWIN
#elif defined(LUA_USE_C89)
#define LUA_STRFTIMEOPTIONS	LUAI_L_STRFTIMEC89
#else  /* C99 specification */
#define LUA_STRFTIMEOPTIONS	LUAI_L_STRFTIMEC99
#endif

#endif					/* } */
/* }================================================================== */


/*
** {==================================================================
** Configuration for time-related stuff
** ===================================================================
*/

#if !defined(luai_l_time_t)		/* { */
/*
** type to represent time_t in Lua
*/
#define luai_l_timet			lua_Integer
#define luai_l_pushtime(L,t)		lua_pushinteger(L,(lua_Integer)(t))

static time_t luai_l_checktime (lua_State *L, int arg) {
  lua_Integer t = luaL_checkinteger(L, arg);
  luaL_argcheck(L, (time_t)t == t, arg, "time out-of-bounds");
  return (time_t)t;
}

#endif				/* } */


#if !defined(luai_l_gmtime)		/* { */
/*
** By default, Lua uses gmtime/localtime, except when POSIX is available,
** where it uses gmtime_r/localtime_r
*/

#if defined(LUA_USE_POSIX)	/* { */

#define luai_l_gmtime(t,r)		gmtime_r(t,r)
#define luai_l_localtime(t,r)	localtime_r(t,r)

#else				/* }{ */

/* ISO C definitions */
#define luai_l_gmtime(t,r)		((void)(r)->tm_sec, gmtime(t))
#define luai_l_localtime(t,r)  	((void)(r)->tm_sec, localtime(t))

#endif				/* } */

#endif				/* } */

/* }================================================================== */


/*
** {==================================================================
** Configuration for 'tmpnam':
** By default, Lua uses tmpnam except when POSIX is available, where
** it uses mkstemp.
** ===================================================================
*/
#if !defined(lua_tmpnam)	/* { */

#if defined(LUA_USE_POSIX)	/* { */

#include <unistd.h>

#define LUA_TMPNAMBUFSIZE	32

#if !defined(LUA_TMPNAMTEMPLATE)
#define LUA_TMPNAMTEMPLATE	"/tmp/lua_XXXXXX"
#endif

#define lua_tmpnam(b,e) { \
        strcpy(b, LUA_TMPNAMTEMPLATE); \
        e = mkstemp(b); \
        if (e != -1) close(e); \
        e = (e == -1); }

#else				/* }{ */

/* ISO C definitions */
#define LUA_TMPNAMBUFSIZE	L_tmpnam
#define lua_tmpnam(b,e)		{ e = (tmpnam(b) == NULL); }

#endif				/* } */

#endif				/* } */
/* }================================================================== */




static int luai_os_execute (lua_State *L) {
  const char *cmd = luaL_optstring(L, 1, NULL);
  int stat = system(cmd);
  if (cmd != NULL)
    return luaL_execresult(L, stat);
  else {
    lua_pushboolean(L, stat);  /* true if there is a shell */
    return 1;
  }
}


static int luai_os_remove (lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  return luaL_fileresult(L, remove(filename) == 0, filename);
}


static int luai_os_rename (lua_State *L) {
  const char *fromname = luaL_checkstring(L, 1);
  const char *toname = luaL_checkstring(L, 2);
  return luaL_fileresult(L, rename(fromname, toname) == 0, NULL);
}


static int luai_os_tmpname (lua_State *L) {
  char buff[LUA_TMPNAMBUFSIZE];
  int err;
  lua_tmpnam(buff, err);
  if (err)
    return luaL_error(L, "unable to generate a unique filename");
  lua_pushstring(L, buff);
  return 1;
}


static int luai_os_getenv (lua_State *L) {
  lua_pushstring(L, getenv(luaL_checkstring(L, 1)));  /* if NULL push nil */
  return 1;
}


static int luai_os_clock (lua_State *L) {
  lua_pushnumber(L, ((lua_Number)clock())/(lua_Number)CLOCKS_PER_SEC);
  return 1;
}


/*
** {======================================================
** Time/Date operations
** { year=%Y, month=%m, day=%d, hour=%H, min=%M, sec=%S,
**   wday=%w+1, yday=%j, isdst=? }
** =======================================================
*/

static void luai_setfield (lua_State *L, const char *key, int value) {
  lua_pushinteger(L, value);
  lua_setfield(L, -2, key);
}

static void luai_setboolfield (lua_State *L, const char *key, int value) {
  if (value < 0)  /* undefined? */
    return;  /* does not set luai_field */
  lua_pushboolean(L, value);
  lua_setfield(L, -2, key);
}


/*
** Set all fields from structure 'tm' in the table on top of the stack
*/
static void luai_setallfields (lua_State *L, struct tm *stm) {
  luai_setfield(L, "sec", stm->tm_sec);
  luai_setfield(L, "min", stm->tm_min);
  luai_setfield(L, "hour", stm->tm_hour);
  luai_setfield(L, "day", stm->tm_mday);
  luai_setfield(L, "month", stm->tm_mon + 1);
  luai_setfield(L, "year", stm->tm_year + 1900);
  luai_setfield(L, "wday", stm->tm_wday + 1);
  luai_setfield(L, "yday", stm->tm_yday + 1);
  luai_setboolfield(L, "isdst", stm->tm_isdst);
}


static int luai_getboolfield (lua_State *L, const char *key) {
  int res;
  res = (lua_getfield(L, -1, key) == LUA_TNIL) ? -1 : lua_toboolean(L, -1);
  lua_pop(L, 1);
  return res;
}


/* maximum value for date fields (to avoid arithmetic overflows with 'int') */
#if !defined(LUAI_L_MAXDATEFIELD)
#define LUAI_L_MAXDATEFIELD	(INT_MAX / 2)
#endif

static int luai_getfield (lua_State *L, const char *key, int d, int delta) {
  int isnum;
  int t = lua_getfield(L, -1, key);  /* get luai_field and its type */
  lua_Integer res = lua_tointegerx(L, -1, &isnum);
  if (!isnum) {  /* luai_field is not an integer? */
    if (t != LUA_TNIL)  /* some other value? */
      return luaL_error(L, "field '%s' is not an integer", key);
    else if (d < 0)  /* absent luai_field; no default? */
      return luaL_error(L, "field '%s' missing in date table", key);
    res = d;
  }
  else {
    if (!(-LUAI_L_MAXDATEFIELD <= res && res <= LUAI_L_MAXDATEFIELD))
      return luaL_error(L, "field '%s' is out-of-bound", key);
    res -= delta;
  }
  lua_pop(L, 1);
  return (int)res;
}


static const char *luai_checkoption (lua_State *L, const char *conv,
                                ptrdiff_t convlen, char *buff) {
  const char *option = LUA_STRFTIMEOPTIONS;
  int oplen = 1;  /* length of options being checked */
  for (; *option != '\0' && oplen <= convlen; option += oplen) {
    if (*option == '|')  /* luai_next luai_getblock? */
      oplen++;  /* will luai_check options with luai_next length (+1) */
    else if (memcmp(conv, option, oplen) == 0) {  /* luai_match? */
      memcpy(buff, conv, oplen);  /* copy valid option to buffer */
      buff[oplen] = '\0';
      return conv + oplen;  /* return luai_next item */
    }
  }
  luaL_argerror(L, 1,
    lua_pushfstring(L, "invalid conversion specifier '%%%s'", conv));
  return conv;  /* to avoid warnings */
}


/* maximum size for an individual 'strftime' item */
#define LUAI_SIZETIMEFMT	250


static int luai_os_date (lua_State *L) {
  size_t slen;
  const char *s = luaL_optlstring(L, 1, "%c", &slen);
  time_t t = luaL_opt(L, luai_l_checktime, 2, time(NULL));
  const char *se = s + slen;  /* 's' end */
  struct tm tmr, *stm;
  if (*s == '!') {  /* UTC? */
    stm = luai_l_gmtime(&t, &tmr);
    s++;  /* skip '!' */
  }
  else
    stm = luai_l_localtime(&t, &tmr);
  if (stm == NULL)  /* invalid date? */
    luaL_error(L, "time result cannot be represented in this installation");
  if (strcmp(s, "*t") == 0) {
    lua_createtable(L, 0, 9);  /* 9 = number of fields */
    luai_setallfields(L, stm);
  }
  else {
    char cc[4];  /* buffer for individual conversion specifiers */
    luaL_Buffer b;
    cc[0] = '%';
    luaL_buffinit(L, &b);
    while (s < se) {
      if (*s != '%')  /* not a conversion specifier? */
        luaL_addchar(&b, *s++);
      else {
        size_t reslen;
        char *buff = luaL_prepbuffsize(&b, LUAI_SIZETIMEFMT);
        s++;  /* skip '%' */
        s = luai_checkoption(L, s, se - s, cc + 1);  /* copy specifier to 'cc' */
        reslen = strftime(buff, LUAI_SIZETIMEFMT, cc, stm);
        luaL_addsize(&b, reslen);
      }
    }
    luaL_pushresult(&b);
  }
  return 1;
}


static int luai_os_time (lua_State *L) {
  time_t t;
  if (lua_isnoneornil(L, 1))  /* called without args? */
    t = time(NULL);  /* get current time */
  else {
    struct tm ts;
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_settop(L, 1);  /* make sure table is at the top */
    ts.tm_sec = luai_getfield(L, "sec", 0, 0);
    ts.tm_min = luai_getfield(L, "min", 0, 0);
    ts.tm_hour = luai_getfield(L, "hour", 12, 0);
    ts.tm_mday = luai_getfield(L, "day", -1, 0);
    ts.tm_mon = luai_getfield(L, "month", -1, 1);
    ts.tm_year = luai_getfield(L, "year", -1, 1900);
    ts.tm_isdst = luai_getboolfield(L, "isdst");
    t = mktime(&ts);
    luai_setallfields(L, &ts);  /* update fields with normalized values */
  }
  if (t != (time_t)(luai_l_timet)t || t == (time_t)(-1))
    luaL_error(L, "time result cannot be represented in this installation");
  luai_l_pushtime(L, t);
  return 1;
}


static int luai_os_difftime (lua_State *L) {
  time_t t1 = luai_l_checktime(L, 1);
  time_t t2 = luai_l_checktime(L, 2);
  lua_pushnumber(L, (lua_Number)difftime(t1, t2));
  return 1;
}

/* }====================================================== */


static int luai_os_setlocale (lua_State *L) {
  static const int cat[] = {LC_ALL, LC_COLLATE, LC_CTYPE, LC_MONETARY,
                      LC_NUMERIC, LC_TIME};
  static const char *const catnames[] = {"all", "collate", "ctype", "monetary",
     "numeric", "time", NULL};
  const char *l = luaL_optstring(L, 1, NULL);
  int op = luaL_checkoption(L, 2, "all", catnames);
  lua_pushstring(L, setlocale(cat[op], l));
  return 1;
}


static int luai_os_exit (lua_State *L) {
  int status;
  if (lua_isboolean(L, 1))
    status = (lua_toboolean(L, 1) ? EXIT_SUCCESS : EXIT_FAILURE);
  else
    status = (int)luaL_optinteger(L, 1, EXIT_SUCCESS);
  if (lua_toboolean(L, 2))
    lua_close(L);
  if (L) exit(status);  /* 'if' to avoid warnings for unreachable 'return' */
  return 0;
}


static const luaL_Reg luai_syslib[] = {
  {"clock",     luai_os_clock},
  {"date",      luai_os_date},
  {"difftime",  luai_os_difftime},
  {"execute",   luai_os_execute},
  {"exit",      luai_os_exit},
  {"getenv",    luai_os_getenv},
  {"remove",    luai_os_remove},
  {"rename",    luai_os_rename},
  {"setlocale", luai_os_setlocale},
  {"time",      luai_os_time},
  {"tmpname",   luai_os_tmpname},
  {NULL, NULL}
};

/* }====================================================== */



LUAMOD_API int luaopen_os (lua_State *L) {
  luaL_newlib(L, luai_syslib);
  return 1;
}

/*__lparser.c__*/

/* maximum number of local variables per function (must be smaller
   than 250, due to the bytecode format) */
#define MAXVARS		200


#define hasmultret(k)		((k) == LUAI_VCALL || (k) == LUAI_VVARARG)


/* because all strings are unified by the scanner, the parser
   can use pointer equality for string equality */
#define eqstr(a,b)	((a) == (b))


/*
** nodes for luai_getblock list (list of active blocks)
*/
typedef struct luai_BlockCnt {
  struct luai_BlockCnt *previous;  /* chain */
  int firstlabel;  /* index of first label in this luai_getblock */
  int firstgoto;  /* index of first pending goto in this luai_getblock */
  luai_lu_byte nactvar;  /* # active locals outside the luai_getblock */
  luai_lu_byte upval;  /* true if some variable in the luai_getblock is an upvalue */
  luai_lu_byte isloop;  /* true if 'luai_getblock' is a loop */
} luai_BlockCnt;



/*
** prototypes for recursive non-terminal functions
*/
static void luai_statement (luai_LexState *ls);
static void luai_expr (luai_LexState *ls, luai_expdesc *v);


/* semantic error */
static luai_l_noret luai_semerror (luai_LexState *ls, const char *msg) {
  ls->t.token = 0;  /* remove "near <token>" from final message */
  luaX_syntaxerror(ls, msg);
}


static luai_l_noret luai_error_expected (luai_LexState *ls, int token) {
  luaX_syntaxerror(ls,
      luaO_pushfstring(ls->L, "%s expected", luaX_token2str(ls, token)));
}


static luai_l_noret luai_errorlimit (luai_FuncState *fs, int limit, const char *what) {
  lua_State *L = fs->ls->L;
  const char *msg;
  int line = fs->f->linedefined;
  const char *where = (line == 0)
                      ? "main function"
                      : luaO_pushfstring(L, "function at line %d", line);
  msg = luaO_pushfstring(L, "too many %s (limit is %d) in %s",
                             what, limit, where);
  luaX_syntaxerror(fs->ls, msg);
}


static void luai_checklimit (luai_FuncState *fs, int v, int l, const char *what) {
  if (v > l) luai_errorlimit(fs, l, what);
}


static int luai_testnext (luai_LexState *ls, int c) {
  if (ls->t.token == c) {
    luaX_next(ls);
    return 1;
  }
  else return 0;
}


static void luai_check (luai_LexState *ls, int c) {
  if (ls->t.token != c)
    luai_error_expected(ls, c);
}


static void luai_checknext (luai_LexState *ls, int c) {
  luai_check(ls, c);
  luaX_next(ls);
}


#define luai_check_condition(ls,c,msg)	{ if (!(c)) luaX_syntaxerror(ls, msg); }



static void luai_check_match (luai_LexState *ls, int what, int who, int where) {
  if (!luai_testnext(ls, what)) {
    if (where == ls->linenumber)
      luai_error_expected(ls, what);
    else {
      luaX_syntaxerror(ls, luaO_pushfstring(ls->L,
             "%s expected (to close %s at line %d)",
              luaX_token2str(ls, what), luaX_token2str(ls, who), where));
    }
  }
}


static luai_TString *luai_str_checkname (luai_LexState *ls) {
  luai_TString *ts;
  luai_check(ls, LUAI_TK_NAME);
  ts = ls->t.seminfo.ts;
  luaX_next(ls);
  return ts;
}


static void luai_init_exp (luai_expdesc *e, luai_expkind k, int i) {
  e->f = e->t = LUAI_NO_JUMP;
  e->k = k;
  e->u.info = i;
}


static void luai_codestring (luai_LexState *ls, luai_expdesc *e, luai_TString *s) {
  luai_init_exp(e, LUAI_VK, luaK_stringK(ls->fs, s));
}


static void luai_checkname (luai_LexState *ls, luai_expdesc *e) {
  luai_codestring(ls, e, luai_str_checkname(ls));
}


static int luai_registerlocalvar (luai_LexState *ls, luai_TString *varname) {
  luai_FuncState *fs = ls->fs;
  luai_Proto *f = fs->f;
  int oldsize = f->sizelocvars;
  luaM_growvector(ls->L, f->locvars, fs->nlocvars, f->sizelocvars,
                  luai_LocVar, SHRT_MAX, "local variables");
  while (oldsize < f->sizelocvars)
    f->locvars[oldsize++].varname = NULL;
  f->locvars[fs->nlocvars].varname = varname;
  luaC_objbarrier(ls->L, f, varname);
  return fs->nlocvars++;
}


static void luai_new_localvar (luai_LexState *ls, luai_TString *name) {
  luai_FuncState *fs = ls->fs;
  luai_Dyndata *dyd = ls->dyd;
  int reg = luai_registerlocalvar(ls, name);
  luai_checklimit(fs, dyd->actvar.n + 1 - fs->firstlocal,
                  MAXVARS, "local variables");
  luaM_growvector(ls->L, dyd->actvar.arr, dyd->actvar.n + 1,
                  dyd->actvar.size, luai_Vardesc, LUAI_MAX_INT, "local variables");
  dyd->actvar.arr[dyd->actvar.n++].idx = luai_cast(short, reg);
}


static void luai_new_localvarliteral_ (luai_LexState *ls, const char *name, size_t sz) {
  luai_new_localvar(ls, luaX_newstring(ls, name, sz));
}

#define luai_new_localvarliteral(ls,v) \
	luai_new_localvarliteral_(ls, "" v, (sizeof(v)/sizeof(char))-1)


static luai_LocVar *luai_getlocvar (luai_FuncState *fs, int i) {
  int idx = fs->ls->dyd->actvar.arr[fs->firstlocal + i].idx;
  lua_assert(idx < fs->nlocvars);
  return &fs->f->locvars[idx];
}


static void luai_ajustlocalvars (luai_LexState *ls, int nvars) {
  luai_FuncState *fs = ls->fs;
  fs->nactvar = luai_cast_byte(fs->nactvar + nvars);
  for (; nvars; nvars--) {
    luai_getlocvar(fs, fs->nactvar - nvars)->startpc = fs->pc;
  }
}


static void luai_removevars (luai_FuncState *fs, int tolevel) {
  fs->ls->dyd->actvar.n -= (fs->nactvar - tolevel);
  while (fs->nactvar > tolevel)
    luai_getlocvar(fs, --fs->nactvar)->endpc = fs->pc;
}


static int luai_searchupvalue (luai_FuncState *fs, luai_TString *name) {
  int i;
  luai_Upvaldesc *up = fs->f->upvalues;
  for (i = 0; i < fs->nups; i++) {
    if (eqstr(up[i].name, name)) return i;
  }
  return -1;  /* not found */
}


static int luai_newupvalue (luai_FuncState *fs, luai_TString *name, luai_expdesc *v) {
  luai_Proto *f = fs->f;
  int oldsize = f->sizeupvalues;
  luai_checklimit(fs, fs->nups + 1, LUAI_MAXUPVAL, "upvalues");
  luaM_growvector(fs->ls->L, f->upvalues, fs->nups, f->sizeupvalues,
                  luai_Upvaldesc, LUAI_MAXUPVAL, "upvalues");
  while (oldsize < f->sizeupvalues)
    f->upvalues[oldsize++].name = NULL;
  f->upvalues[fs->nups].instack = (v->k == LUAI_VLOCAL);
  f->upvalues[fs->nups].idx = luai_cast_byte(v->u.info);
  f->upvalues[fs->nups].name = name;
  luaC_objbarrier(fs->ls->L, f, name);
  return fs->nups++;
}


static int luai_searchvar (luai_FuncState *fs, luai_TString *n) {
  int i;
  for (i = luai_cast_int(fs->nactvar) - 1; i >= 0; i--) {
    if (eqstr(n, luai_getlocvar(fs, i)->varname))
      return i;
  }
  return -1;  /* not found */
}


/*
  Mark luai_getblock where variable at given level was defined
  (to emit close instructions later).
*/
static void luai_markupval (luai_FuncState *fs, int level) {
  luai_BlockCnt *bl = fs->bl;
  while (bl->nactvar > level)
    bl = bl->previous;
  bl->upval = 1;
}


/*
  Find variable with given name 'n'. If it is an upvalue, add this
  upvalue into all intermediate functions.
*/
static void luai_singlevaraux (luai_FuncState *fs, luai_TString *n, luai_expdesc *var, int base) {
  if (fs == NULL)  /* no more levels? */
    luai_init_exp(var, LUAI_VVOID, 0);  /* default is global */
  else {
    int v = luai_searchvar(fs, n);  /* look up locals at current level */
    if (v >= 0) {  /* found? */
      luai_init_exp(var, LUAI_VLOCAL, v);  /* variable is local */
      if (!base)
        luai_markupval(fs, v);  /* local will be used as an upval */
    }
    else {  /* not found as local at current level; try upvalues */
      int idx = luai_searchupvalue(fs, n);  /* try existing upvalues */
      if (idx < 0) {  /* not found? */
        luai_singlevaraux(fs->prev, n, var, 0);  /* try upper levels */
        if (var->k == LUAI_VVOID)  /* not found? */
          return;  /* it is a global */
        /* else was LOCAL or UPVAL */
        idx  = luai_newupvalue(fs, n, var);  /* will be a new upvalue */
      }
      luai_init_exp(var, LUAI_VUPVAL, idx);  /* new or old upvalue */
    }
  }
}


static void luai_singlevar (luai_LexState *ls, luai_expdesc *var) {
  luai_TString *varname = luai_str_checkname(ls);
  luai_FuncState *fs = ls->fs;
  luai_singlevaraux(fs, varname, var, 1);
  if (var->k == LUAI_VVOID) {  /* global name? */
    luai_expdesc key;
    luai_singlevaraux(fs, ls->envn, var, 1);  /* get environment variable */
    lua_assert(var->k != LUAI_VVOID);  /* this one must exist */
    luai_codestring(ls, &key, varname);  /* key is variable name */
    luaK_indexed(fs, var, &key);  /* env[varname] */
  }
}


static void luai_adjust_assign (luai_LexState *ls, int nvars, int nexps, luai_expdesc *e) {
  luai_FuncState *fs = ls->fs;
  int extra = nvars - nexps;
  if (hasmultret(e->k)) {
    extra++;  /* includes call itself */
    if (extra < 0) extra = 0;
    luaK_setreturns(fs, e, extra);  /* last exp. provides the difference */
    if (extra > 1) luaK_reserveregs(fs, extra-1);
  }
  else {
    if (e->k != LUAI_VVOID) luaK_exp2nextreg(fs, e);  /* close last expression */
    if (extra > 0) {
      int reg = fs->luai_freereg;
      luaK_reserveregs(fs, extra);
      luaK_nil(fs, reg, extra);
    }
  }
  if (nexps > nvars)
    ls->fs->luai_freereg -= nexps - nvars;  /* remove extra values */
}


static void luai_enterlevel (luai_LexState *ls) {
  lua_State *L = ls->L;
  ++L->nCcalls;
  luai_checklimit(ls->fs, L->nCcalls, LUAI_MAXCCALLS, "C levels");
}


#define luai_leavelevel(ls)	((ls)->L->nCcalls--)


static void luai_closegoto (luai_LexState *ls, int g, luai_Labeldesc *label) {
  int i;
  luai_FuncState *fs = ls->fs;
  luai_Labellist *gl = &ls->dyd->gt;
  luai_Labeldesc *gt = &gl->arr[g];
  lua_assert(eqstr(gt->name, label->name));
  if (gt->nactvar < label->nactvar) {
    luai_TString *vname = luai_getlocvar(fs, gt->nactvar)->varname;
    const char *msg = luaO_pushfstring(ls->L,
      "<goto %s> at line %d jumps into the scope of local '%s'",
      luai_getstr(gt->name), gt->line, luai_getstr(vname));
    luai_semerror(ls, msg);
  }
  luaK_patchlist(fs, gt->pc, label->pc);
  /* remove goto from pending list */
  for (i = g; i < gl->n - 1; i++)
    gl->arr[i] = gl->arr[i + 1];
  gl->n--;
}


/*
** try to close a goto with existing labels; this solves backward jumps
*/
static int luai_findlabel (luai_LexState *ls, int g) {
  int i;
  luai_BlockCnt *bl = ls->fs->bl;
  luai_Dyndata *dyd = ls->dyd;
  luai_Labeldesc *gt = &dyd->gt.arr[g];
  /* luai_check labels in current luai_getblock for a luai_match */
  for (i = bl->firstlabel; i < dyd->label.n; i++) {
    luai_Labeldesc *lb = &dyd->label.arr[i];
    if (eqstr(lb->name, gt->name)) {  /* correct label? */
      if (gt->nactvar > lb->nactvar &&
          (bl->upval || dyd->label.n > bl->firstlabel))
        luaK_patchclose(ls->fs, gt->pc, lb->nactvar);
      luai_closegoto(ls, g, lb);  /* close it */
      return 1;
    }
  }
  return 0;  /* label not found; cannot close goto */
}


static int luai_newlabelentry (luai_LexState *ls, luai_Labellist *l, luai_TString *name,
                          int line, int pc) {
  int n = l->n;
  luaM_growvector(ls->L, l->arr, n, l->size,
                  luai_Labeldesc, SHRT_MAX, "labels/gotos");
  l->arr[n].name = name;
  l->arr[n].line = line;
  l->arr[n].nactvar = ls->fs->nactvar;
  l->arr[n].pc = pc;
  l->n = n + 1;
  return n;
}


/*
** luai_check whether new label 'lb' matches any pending gotos in current
** luai_getblock; solves forward jumps
*/
static void luai_findgotos (luai_LexState *ls, luai_Labeldesc *lb) {
  luai_Labellist *gl = &ls->dyd->gt;
  int i = ls->fs->bl->firstgoto;
  while (i < gl->n) {
    if (eqstr(gl->arr[i].name, lb->name))
      luai_closegoto(ls, i, lb);
    else
      i++;
  }
}


/*
** export pending gotos to outer level, to luai_check them against
** outer labels; if the luai_getblock being exited has upvalues, and
** the goto exits the scope of any variable (which can be the
** upvalue), close those variables being exited.
*/
static void luai_movegotosout (luai_FuncState *fs, luai_BlockCnt *bl) {
  int i = bl->firstgoto;
  luai_Labellist *gl = &fs->ls->dyd->gt;
  /* correct pending gotos to current luai_getblock and try to close it
     with visible labels */
  while (i < gl->n) {
    luai_Labeldesc *gt = &gl->arr[i];
    if (gt->nactvar > bl->nactvar) {
      if (bl->upval)
        luaK_patchclose(fs, gt->pc, bl->nactvar);
      gt->nactvar = bl->nactvar;
    }
    if (!luai_findlabel(fs->ls, i))
      i++;  /* move to luai_next one */
  }
}


static void luai_enterblock (luai_FuncState *fs, luai_BlockCnt *bl, luai_lu_byte isloop) {
  bl->isloop = isloop;
  bl->nactvar = fs->nactvar;
  bl->firstlabel = fs->ls->dyd->label.n;
  bl->firstgoto = fs->ls->dyd->gt.n;
  bl->upval = 0;
  bl->previous = fs->bl;
  fs->bl = bl;
  lua_assert(fs->luai_freereg == fs->nactvar);
}


/*
** create a label named 'break' to resolve break statements
*/
static void luai_breaklabel (luai_LexState *ls) {
  luai_TString *n = luaS_new(ls->L, "break");
  int l = luai_newlabelentry(ls, &ls->dyd->label, n, 0, ls->fs->pc);
  luai_findgotos(ls, &ls->dyd->label.arr[l]);
}

/*
** generates an error for an undefined 'goto'; choose appropriate
** message when label name is a reserved word (which can only be 'break')
*/
static luai_l_noret luai_undefgoto (luai_LexState *ls, luai_Labeldesc *gt) {
  const char *msg = luai_isreserved(gt->name)
                    ? "<%s> at line %d not inside a loop"
                    : "no visible label '%s' for <goto> at line %d";
  msg = luaO_pushfstring(ls->L, msg, luai_getstr(gt->name), gt->line);
  luai_semerror(ls, msg);
}


static void luai_leaveblock (luai_FuncState *fs) {
  luai_BlockCnt *bl = fs->bl;
  luai_LexState *ls = fs->ls;
  if (bl->previous && bl->upval) {
    /* create a 'jump to here' to close upvalues */
    int j = luaK_jump(fs);
    luaK_patchclose(fs, j, bl->nactvar);
    luaK_patchtohere(fs, j);
  }
  if (bl->isloop)
    luai_breaklabel(ls);  /* close pending breaks */
  fs->bl = bl->previous;
  luai_removevars(fs, bl->nactvar);
  lua_assert(bl->nactvar == fs->nactvar);
  fs->luai_freereg = fs->nactvar;  /* free registers */
  ls->dyd->label.n = bl->firstlabel;  /* remove local labels */
  if (bl->previous)  /* inner luai_getblock? */
    luai_movegotosout(fs, bl);  /* update pending gotos to outer luai_getblock */
  else if (bl->firstgoto < ls->dyd->gt.n)  /* pending gotos in outer luai_getblock? */
    luai_undefgoto(ls, &ls->dyd->gt.arr[bl->firstgoto]);  /* error */
}


/*
** adds a new prototype into list of prototypes
*/
static luai_Proto *luai_addprototype (luai_LexState *ls) {
  luai_Proto *clp;
  lua_State *L = ls->L;
  luai_FuncState *fs = ls->fs;
  luai_Proto *f = fs->f;  /* prototype of current function */
  if (fs->np >= f->sizep) {
    int oldsize = f->sizep;
    luaM_growvector(L, f->p, fs->np, f->sizep, luai_Proto *, luai_MAXARG_Bx, "functions");
    while (oldsize < f->sizep)
      f->p[oldsize++] = NULL;
  }
  f->p[fs->np++] = clp = luaF_newproto(L);
  luaC_objbarrier(L, f, clp);
  return clp;
}


/*
** codes instruction to create new closure in parent function.
** The luai_OP_CLOSURE instruction must use the last available register,
** so that, if it invokes the LUAI_GC, the LUAI_GC knows which registers
** are in use at that time.
*/
static void luai_codeclosure (luai_LexState *ls, luai_expdesc *v) {
  luai_FuncState *fs = ls->fs->prev;
  luai_init_exp(v, LUAI_VRELOCABLE, luaK_codeABx(fs, luai_OP_CLOSURE, 0, fs->np - 1));
  luaK_exp2nextreg(fs, v);  /* fix it at the last register */
}


static void luai_open_func (luai_LexState *ls, luai_FuncState *fs, luai_BlockCnt *bl) {
  luai_Proto *f;
  fs->prev = ls->fs;  /* linked list of luai_funcstates */
  fs->ls = ls;
  ls->fs = fs;
  fs->pc = 0;
  fs->lasttarget = 0;
  fs->jpc = LUAI_NO_JUMP;
  fs->luai_freereg = 0;
  fs->nk = 0;
  fs->np = 0;
  fs->nups = 0;
  fs->nlocvars = 0;
  fs->nactvar = 0;
  fs->firstlocal = ls->dyd->actvar.n;
  fs->bl = NULL;
  f = fs->f;
  f->source = ls->source;
  f->maxstacksize = 2;  /* registers 0/1 are always valid */
  luai_enterblock(fs, bl, 0);
}


static void luai_close_func (luai_LexState *ls) {
  lua_State *L = ls->L;
  luai_FuncState *fs = ls->fs;
  luai_Proto *f = fs->f;
  luaK_ret(fs, 0, 0);  /* final return */
  luai_leaveblock(fs);
  luaM_reallocvector(L, f->code, f->sizecode, fs->pc, Instruction);
  f->sizecode = fs->pc;
  luaM_reallocvector(L, f->lineinfo, f->sizelineinfo, fs->pc, int);
  f->sizelineinfo = fs->pc;
  luaM_reallocvector(L, f->k, f->sizek, fs->nk, luai_TValue);
  f->sizek = fs->nk;
  luaM_reallocvector(L, f->p, f->sizep, fs->np, luai_Proto *);
  f->sizep = fs->np;
  luaM_reallocvector(L, f->locvars, f->sizelocvars, fs->nlocvars, luai_LocVar);
  f->sizelocvars = fs->nlocvars;
  luaM_reallocvector(L, f->upvalues, f->sizeupvalues, fs->nups, luai_Upvaldesc);
  f->sizeupvalues = fs->nups;
  lua_assert(fs->bl == NULL);
  ls->fs = fs->prev;
  luaC_checkGC(L);
}



/*============================================================*/
/* GRAMMAR RULES */
/*============================================================*/


/*
** luai_check whether current token is in the follow set of a luai_getblock.
** 'until' closes syntactical blocks, but do not close scope,
** so it is handled in separate.
*/
static int luai_block_follow (luai_LexState *ls, int withuntil) {
  switch (ls->t.token) {
    case LUAI_TK_ELSE: case LUAI_TK_ELSEIF:
    case LUAI_TK_END: case LUAI_TK_EOS:
      return 1;
    case LUAI_TK_UNTIL: return withuntil;
    default: return 0;
  }
}


static void luai_statlist (luai_LexState *ls) {
  /* luai_statlist -> { stat [';'] } */
  while (!luai_block_follow(ls, 1)) {
    if (ls->t.token == LUAI_TK_RETURN) {
      luai_statement(ls);
      return;  /* 'return' must be last luai_statement */
    }
    luai_statement(ls);
  }
}


static void luai_fieldsel (luai_LexState *ls, luai_expdesc *v) {
  /* luai_fieldsel -> ['.' | ':'] NAME */
  luai_FuncState *fs = ls->fs;
  luai_expdesc key;
  luaK_exp2anyregup(fs, v);
  luaX_next(ls);  /* skip the dot or colon */
  luai_checkname(ls, &key);
  luaK_indexed(fs, v, &key);
}


static void luai_yindex (luai_LexState *ls, luai_expdesc *v) {
  /* index -> '[' luai_expr ']' */
  luaX_next(ls);  /* skip the '[' */
  luai_expr(ls, v);
  luaK_exp2val(ls->fs, v);
  luai_checknext(ls, ']');
}


/*
** {======================================================================
** Rules for Constructors
** =======================================================================
*/


struct luai_ConsControl {
  luai_expdesc v;  /* last list item read */
  luai_expdesc *t;  /* table descriptor */
  int nh;  /* total number of 'record' elements */
  int na;  /* total number of array elements */
  int tostore;  /* number of array elements pending to be stored */
};


static void luai_recfield (luai_LexState *ls, struct luai_ConsControl *cc) {
  /* luai_recfield -> (NAME | '['luai_exp1']') = luai_exp1 */
  luai_FuncState *fs = ls->fs;
  int reg = ls->fs->luai_freereg;
  luai_expdesc key, val;
  int rkkey;
  if (ls->t.token == LUAI_TK_NAME) {
    luai_checklimit(fs, cc->nh, LUAI_MAX_INT, "items in a luai_constructor");
    luai_checkname(ls, &key);
  }
  else  /* ls->t.token == '[' */
    luai_yindex(ls, &key);
  cc->nh++;
  luai_checknext(ls, '=');
  rkkey = luaK_exp2RK(fs, &key);
  luai_expr(ls, &val);
  luaK_codeABC(fs, luai_OP_SETTABLE, cc->t->u.info, rkkey, luaK_exp2RK(fs, &val));
  fs->luai_freereg = reg;  /* free registers */
}


static void luai_closelistfield (luai_FuncState *fs, struct luai_ConsControl *cc) {
  if (cc->v.k == LUAI_VVOID) return;  /* there is no list item */
  luaK_exp2nextreg(fs, &cc->v);
  cc->v.k = LUAI_VVOID;
  if (cc->tostore == LUAI_LFIELDS_PER_FLUSH) {
    luaK_setlist(fs, cc->t->u.info, cc->na, cc->tostore);  /* flush */
    cc->tostore = 0;  /* no more items pending */
  }
}


static void luai_lastlistfield (luai_FuncState *fs, struct luai_ConsControl *cc) {
  if (cc->tostore == 0) return;
  if (hasmultret(cc->v.k)) {
    luaK_setmultret(fs, &cc->v);
    luaK_setlist(fs, cc->t->u.info, cc->na, LUA_MULTRET);
    cc->na--;  /* do not count last expression (unknown number of elements) */
  }
  else {
    if (cc->v.k != LUAI_VVOID)
      luaK_exp2nextreg(fs, &cc->v);
    luaK_setlist(fs, cc->t->u.info, cc->na, cc->tostore);
  }
}


static void luai_listfield (luai_LexState *ls, struct luai_ConsControl *cc) {
  /* listfield -> exp */
  luai_expr(ls, &cc->v);
  luai_checklimit(ls->fs, cc->na, LUAI_MAX_INT, "items in a luai_constructor");
  cc->na++;
  cc->tostore++;
}


static void luai_field (luai_LexState *ls, struct luai_ConsControl *cc) {
  /* luai_field -> listfield | luai_recfield */
  switch(ls->t.token) {
    case LUAI_TK_NAME: {  /* may be 'listfield' or 'luai_recfield' */
      if (luaX_lookahead(ls) != '=')  /* expression? */
        luai_listfield(ls, cc);
      else
        luai_recfield(ls, cc);
      break;
    }
    case '[': {
      luai_recfield(ls, cc);
      break;
    }
    default: {
      luai_listfield(ls, cc);
      break;
    }
  }
}


static void luai_constructor (luai_LexState *ls, luai_expdesc *t) {
  /* luai_constructor -> '{' [ luai_field { sep luai_field } [sep] ] '}'
     sep -> ',' | ';' */
  luai_FuncState *fs = ls->fs;
  int line = ls->linenumber;
  int pc = luaK_codeABC(fs, luai_OP_NEWTABLE, 0, 0, 0);
  struct luai_ConsControl cc;
  cc.na = cc.nh = cc.tostore = 0;
  cc.t = t;
  luai_init_exp(t, LUAI_VRELOCABLE, pc);
  luai_init_exp(&cc.v, LUAI_VVOID, 0);  /* no value (yet) */
  luaK_exp2nextreg(ls->fs, t);  /* fix it at stack top */
  luai_checknext(ls, '{');
  do {
    lua_assert(cc.v.k == LUAI_VVOID || cc.tostore > 0);
    if (ls->t.token == '}') break;
    luai_closelistfield(fs, &cc);
    luai_field(ls, &cc);
  } while (luai_testnext(ls, ',') || luai_testnext(ls, ';'));
  luai_check_match(ls, '}', '{', line);
  luai_lastlistfield(fs, &cc);
  luai_SETARG_B(fs->f->code[pc], luaO_int2fb(cc.na)); /* set initial array size */
  luai_SETARG_C(fs->f->code[pc], luaO_int2fb(cc.nh));  /* set initial table size */
}

/* }====================================================================== */



static void luai_parlist (luai_LexState *ls) {
  /* luai_parlist -> [ param { ',' param } ] */
  luai_FuncState *fs = ls->fs;
  luai_Proto *f = fs->f;
  int nparams = 0;
  f->is_vararg = 0;
  if (ls->t.token != ')') {  /* is 'luai_parlist' not empty? */
    do {
      switch (ls->t.token) {
        case LUAI_TK_NAME: {  /* param -> NAME */
          luai_new_localvar(ls, luai_str_checkname(ls));
          nparams++;
          break;
        }
        case LUAI_TK_DOTS: {  /* param -> '...' */
          luaX_next(ls);
          f->is_vararg = 1;  /* declared vararg */
          break;
        }
        default: luaX_syntaxerror(ls, "<name> or '...' expected");
      }
    } while (!f->is_vararg && luai_testnext(ls, ','));
  }
  luai_ajustlocalvars(ls, nparams);
  f->numparams = luai_cast_byte(fs->nactvar);
  luaK_reserveregs(fs, fs->nactvar);  /* reserve register for parameters */
}


static void luai_body (luai_LexState *ls, luai_expdesc *e, int ismethod, int line) {
  /* luai_body ->  '(' luai_parlist ')' luai_getblock END */
  luai_FuncState new_fs;
  luai_BlockCnt bl;
  new_fs.f = luai_addprototype(ls);
  new_fs.f->linedefined = line;
  luai_open_func(ls, &new_fs, &bl);
  luai_checknext(ls, '(');
  if (ismethod) {
    luai_new_localvarliteral(ls, "self");  /* create 'self' parameter */
    luai_ajustlocalvars(ls, 1);
  }
  luai_parlist(ls);
  luai_checknext(ls, ')');
  luai_statlist(ls);
  new_fs.f->lastlinedefined = ls->linenumber;
  luai_check_match(ls, LUAI_TK_END, LUAI_TK_FUNCTION, line);
  luai_codeclosure(ls, e);
  luai_close_func(ls);
}


static int luai_explist (luai_LexState *ls, luai_expdesc *v) {
  /* luai_explist -> luai_expr { ',' luai_expr } */
  int n = 1;  /* at least one expression */
  luai_expr(ls, v);
  while (luai_testnext(ls, ',')) {
    luaK_exp2nextreg(ls->fs, v);
    luai_expr(ls, v);
    n++;
  }
  return n;
}


static void luai_funcargs (luai_LexState *ls, luai_expdesc *f, int line) {
  luai_FuncState *fs = ls->fs;
  luai_expdesc args;
  int base, nparams;
  switch (ls->t.token) {
    case '(': {  /* luai_funcargs -> '(' [ luai_explist ] ')' */
      luaX_next(ls);
      if (ls->t.token == ')')  /* arg list is empty? */
        args.k = LUAI_VVOID;
      else {
        luai_explist(ls, &args);
        luaK_setmultret(fs, &args);
      }
      luai_check_match(ls, ')', '(', line);
      break;
    }
    case '{': {  /* luai_funcargs -> luai_constructor */
      luai_constructor(ls, &args);
      break;
    }
    case LUAI_TK_STRING: {  /* luai_funcargs -> STRING */
      luai_codestring(ls, &args, ls->t.seminfo.ts);
      luaX_next(ls);  /* must use 'seminfo' before 'luai_next' */
      break;
    }
    default: {
      luaX_syntaxerror(ls, "function arguments expected");
    }
  }
  lua_assert(f->k == LUAI_VNONRELOC);
  base = f->u.info;  /* base register for call */
  if (hasmultret(args.k))
    nparams = LUA_MULTRET;  /* open call */
  else {
    if (args.k != LUAI_VVOID)
      luaK_exp2nextreg(fs, &args);  /* close last argument */
    nparams = fs->luai_freereg - (base+1);
  }
  luai_init_exp(f, LUAI_VCALL, luaK_codeABC(fs, luai_OP_CALL, base, nparams+1, 2));
  luaK_fixline(fs, line);
  fs->luai_freereg = base+1;  /* call remove function and arguments and leaves
                            (unless changed) one result */
}




/*
** {======================================================================
** Expression parsing
** =======================================================================
*/


static void luai_primaryexp (luai_LexState *ls, luai_expdesc *v) {
  /* luai_primaryexp -> NAME | '(' luai_expr ')' */
  switch (ls->t.token) {
    case '(': {
      int line = ls->linenumber;
      luaX_next(ls);
      luai_expr(ls, v);
      luai_check_match(ls, ')', '(', line);
      luaK_dischargevars(ls->fs, v);
      return;
    }
    case LUAI_TK_NAME: {
      luai_singlevar(ls, v);
      return;
    }
    default: {
      luaX_syntaxerror(ls, "unexpected symbol");
    }
  }
}


static void luai_suffixedexp (luai_LexState *ls, luai_expdesc *v) {
  /* luai_suffixedexp ->
       luai_primaryexp { '.' NAME | '[' exp ']' | ':' NAME luai_funcargs | luai_funcargs } */
  luai_FuncState *fs = ls->fs;
  int line = ls->linenumber;
  luai_primaryexp(ls, v);
  for (;;) {
    switch (ls->t.token) {
      case '.': {  /* luai_fieldsel */
        luai_fieldsel(ls, v);
        break;
      }
      case '[': {  /* '[' luai_exp1 ']' */
        luai_expdesc key;
        luaK_exp2anyregup(fs, v);
        luai_yindex(ls, &key);
        luaK_indexed(fs, v, &key);
        break;
      }
      case ':': {  /* ':' NAME luai_funcargs */
        luai_expdesc key;
        luaX_next(ls);
        luai_checkname(ls, &key);
        luaK_self(fs, v, &key);
        luai_funcargs(ls, v, line);
        break;
      }
      case '(': case LUAI_TK_STRING: case '{': {  /* luai_funcargs */
        luaK_exp2nextreg(fs, v);
        luai_funcargs(ls, v, line);
        break;
      }
      default: return;
    }
  }
}


static void luai_simpleexp (luai_LexState *ls, luai_expdesc *v) {
  /* luai_simpleexp -> FLT | INT | STRING | NIL | TRUE | FALSE | ... |
                  luai_constructor | FUNCTION luai_body | luai_suffixedexp */
  switch (ls->t.token) {
    case LUAI_TK_FLT: {
      luai_init_exp(v, LUAI_VKFLT, 0);
      v->u.nval = ls->t.seminfo.r;
      break;
    }
    case LUAI_TK_INT: {
      luai_init_exp(v, LUAI_VKINT, 0);
      v->u.ival = ls->t.seminfo.i;
      break;
    }
    case LUAI_TK_STRING: {
      luai_codestring(ls, v, ls->t.seminfo.ts);
      break;
    }
    case LUAI_TK_NIL: {
      luai_init_exp(v, LUAI_VNIL, 0);
      break;
    }
    case LUAI_TK_TRUE: {
      luai_init_exp(v, LUAI_VTRUE, 0);
      break;
    }
    case LUAI_TK_FALSE: {
      luai_init_exp(v, LUAI_VFALSE, 0);
      break;
    }
    case LUAI_TK_DOTS: {  /* vararg */
      luai_FuncState *fs = ls->fs;
      luai_check_condition(ls, fs->f->is_vararg,
                      "cannot use '...' outside a vararg function");
      luai_init_exp(v, LUAI_VVARARG, luaK_codeABC(fs, luai_OP_VARARG, 0, 1, 0));
      break;
    }
    case '{': {  /* luai_constructor */
      luai_constructor(ls, v);
      return;
    }
    case LUAI_TK_FUNCTION: {
      luaX_next(ls);
      luai_body(ls, v, 0, ls->linenumber);
      return;
    }
    default: {
      luai_suffixedexp(ls, v);
      return;
    }
  }
  luaX_next(ls);
}


static laui_UnOpr luai_getunopr (int op) {
  switch (op) {
    case LUAI_TK_NOT: return LUAI_OPRNOT;
    case '-': return LUAI_OPRMINUS;
    case '~': return LUAI_OPRBNOT;
    case '#': return LUAI_OPRLEN;
    default: return LUAI_OPRNOUNOPR;
  }
}


static luai_BinOpr getbinopr (int op) {
  switch (op) {
    case '+': return LUAI_OPRADD;
    case '-': return LUAI_OPRSUB;
    case '*': return LUAI_OPRMUL;
    case '%': return LUAI_OPRMOD;
    case '^': return LUAI_OPRPOW;
    case '/': return LUAI_OPRDIV;
    case LUAI_TK_IDIV: return LUAI_OPRIDIV;
    case '&': return LUAI_OPRBAND;
    case '|': return LUAI_OPRBOR;
    case '~': return LUAI_OPRBXOR;
    case LUAI_TK_SHL: return LUAI_OPRSHL;
    case LUAI_TK_SHR: return LUAI_OPRSHR;
    case LUAI_TK_CONCAT: return LUAI_OPRCONCAT;
    case LUAI_TK_NE: return LUAI_OPRNE;
    case LUAI_TK_EQ: return LUAI_OPREQ;
    case '<': return LUAI_OPRLT;
    case LUAI_TK_LE: return LUAI_OPRLE;
    case '>': return LUAI_OPRGT;
    case LUAI_TK_GE: return LUAI_OPRGE;
    case LUAI_TK_AND: return LUAI_OPRAND;
    case LUAI_TK_OR: return LUAI_OPROR;
    default: return LUAI_OPRNOBINOPR;
  }
}


static const struct {
  luai_lu_byte left;  /* left priority for each binary operator */
  luai_lu_byte right; /* right priority */
} priority[] = {  /* ORDER OPR */
   {10, 10}, {10, 10},           /* '+' '-' */
   {11, 11}, {11, 11},           /* '*' '%' */
   {14, 13},                  /* '^' (right associative) */
   {11, 11}, {11, 11},           /* '/' '//' */
   {6, 6}, {4, 4}, {5, 5},   /* '&' '|' '~' */
   {7, 7}, {7, 7},           /* '<<' '>>' */
   {9, 8},                   /* '..' (right associative) */
   {3, 3}, {3, 3}, {3, 3},   /* ==, <, <= */
   {3, 3}, {3, 3}, {3, 3},   /* ~=, >, >= */
   {2, 2}, {1, 1}            /* and, or */
};

#define UNARY_PRIORITY	12  /* priority for unary operators */


/*
** subexpr -> (luai_simpleexp | unop subexpr) { binop subexpr }
** where 'binop' is any binary operator with a priority higher than 'limit'
*/
static luai_BinOpr subexpr (luai_LexState *ls, luai_expdesc *v, int limit) {
  luai_BinOpr op;
  laui_UnOpr uop;
  luai_enterlevel(ls);
  uop = luai_getunopr(ls->t.token);
  if (uop != LUAI_OPRNOUNOPR) {
    int line = ls->linenumber;
    luaX_next(ls);
    subexpr(ls, v, UNARY_PRIORITY);
    luaK_prefix(ls->fs, uop, v, line);
  }
  else luai_simpleexp(ls, v);
  /* expand while operators have priorities higher than 'limit' */
  op = getbinopr(ls->t.token);
  while (op != LUAI_OPRNOBINOPR && priority[op].left > limit) {
    luai_expdesc v2;
    luai_BinOpr nextop;
    int line = ls->linenumber;
    luaX_next(ls);
    luaK_infix(ls->fs, op, v);
    /* read sub-expression with higher priority */
    nextop = subexpr(ls, &v2, priority[op].right);
    luaK_posfix(ls->fs, op, v, &v2, line);
    op = nextop;
  }
  luai_leavelevel(ls);
  return op;  /* return first untreated operator */
}


static void luai_expr (luai_LexState *ls, luai_expdesc *v) {
  subexpr(ls, v, 0);
}

/* }==================================================================== */



/*
** {======================================================================
** Rules for Statements
** =======================================================================
*/


static void luai_getblock (luai_LexState *ls) {
  /* luai_getblock -> luai_statlist */
  luai_FuncState *fs = ls->fs;
  luai_BlockCnt bl;
  luai_enterblock(fs, &bl, 0);
  luai_statlist(ls);
  luai_leaveblock(fs);
}


/*
** structure to chain all variables in the left-hand side of an
** luai_assignment
*/
struct luai_LHS_assign {
  struct luai_LHS_assign *prev;
  luai_expdesc v;  /* variable (global, local, upvalue, or indexed) */
};


/*
** luai_check whether, in an luai_assignment to an upvalue/local variable, the
** upvalue/local variable is begin used in a previous luai_assignment to a
** table. If so, luai_save original upvalue/local value in a safe place and
** use this safe copy in the previous luai_assignment.
*/
static void luai_check_conflict (luai_LexState *ls, struct luai_LHS_assign *lh, luai_expdesc *v) {
  luai_FuncState *fs = ls->fs;
  int extra = fs->luai_freereg;  /* eventual position to luai_save local variable */
  int conflict = 0;
  for (; lh; lh = lh->prev) {  /* luai_check all previous assignments */
    if (lh->v.k == LUAI_VINDEXED) {  /* assigning to a table? */
      /* table is the upvalue/local being assigned now? */
      if (lh->v.u.ind.vt == v->k && lh->v.u.ind.t == v->u.info) {
        conflict = 1;
        lh->v.u.ind.vt = LUAI_VLOCAL;
        lh->v.u.ind.t = extra;  /* previous luai_assignment will use safe copy */
      }
      /* index is the local being assigned? (index cannot be upvalue) */
      if (v->k == LUAI_VLOCAL && lh->v.u.ind.idx == v->u.info) {
        conflict = 1;
        lh->v.u.ind.idx = extra;  /* previous luai_assignment will use safe copy */
      }
    }
  }
  if (conflict) {
    /* copy upvalue/local value to a temporary (in position 'extra') */
    luai_OpCode op = (v->k == LUAI_VLOCAL) ? luai_OP_MOVE : luai_OP_GETUPVAL;
    luaK_codeABC(fs, op, extra, v->u.info, 0);
    luaK_reserveregs(fs, 1);
  }
}


static void luai_assignment (luai_LexState *ls, struct luai_LHS_assign *lh, int nvars) {
  luai_expdesc e;
  luai_check_condition(ls, luai_vkisvar(lh->v.k), "syntax error");
  if (luai_testnext(ls, ',')) {  /* luai_assignment -> ',' luai_suffixedexp luai_assignment */
    struct luai_LHS_assign nv;
    nv.prev = lh;
    luai_suffixedexp(ls, &nv.v);
    if (nv.v.k != LUAI_VINDEXED)
      luai_check_conflict(ls, lh, &nv.v);
    luai_checklimit(ls->fs, nvars + ls->L->nCcalls, LUAI_MAXCCALLS,
                    "C levels");
    luai_assignment(ls, &nv, nvars+1);
  }
  else {  /* luai_assignment -> '=' luai_explist */
    int nexps;
    luai_checknext(ls, '=');
    nexps = luai_explist(ls, &e);
    if (nexps != nvars)
      luai_adjust_assign(ls, nvars, nexps, &e);
    else {
      luaK_setoneret(ls->fs, &e);  /* close last expression */
      luaK_storevar(ls->fs, &lh->v, &e);
      return;  /* avoid default */
    }
  }
  luai_init_exp(&e, LUAI_VNONRELOC, ls->fs->luai_freereg-1);  /* default luai_assignment */
  luaK_storevar(ls->fs, &lh->v, &e);
}


static int luai_cond (luai_LexState *ls) {
  /* luai_cond -> exp */
  luai_expdesc v;
  luai_expr(ls, &v);  /* read condition */
  if (v.k == LUAI_VNIL) v.k = LUAI_VFALSE;  /* 'falses' are all equal here */
  luaK_goiftrue(ls->fs, &v);
  return v.f;
}


static void luai_gotostat (luai_LexState *ls, int pc) {
  int line = ls->linenumber;
  luai_TString *label;
  int g;
  if (luai_testnext(ls, LUAI_TK_GOTO))
    label = luai_str_checkname(ls);
  else {
    luaX_next(ls);  /* skip break */
    label = luaS_new(ls->L, "break");
  }
  g = luai_newlabelentry(ls, &ls->dyd->gt, label, line, pc);
  luai_findlabel(ls, g);  /* close it if label already defined */
}


/* luai_check for repeated labels on the same luai_getblock */
static void luai_checkrepeated (luai_FuncState *fs, luai_Labellist *ll, luai_TString *label) {
  int i;
  for (i = fs->bl->firstlabel; i < ll->n; i++) {
    if (eqstr(label, ll->arr[i].name)) {
      const char *msg = luaO_pushfstring(fs->ls->L,
                          "label '%s' already defined on line %d",
                          luai_getstr(label), ll->arr[i].line);
      luai_semerror(fs->ls, msg);
    }
  }
}


/* skip no-op statements */
static void luai_skipnoopstat (luai_LexState *ls) {
  while (ls->t.token == ';' || ls->t.token == LUAI_TK_DBCOLON)
    luai_statement(ls);
}


static void luai_labelstat (luai_LexState *ls, luai_TString *label, int line) {
  /* label -> '::' NAME '::' */
  luai_FuncState *fs = ls->fs;
  luai_Labellist *ll = &ls->dyd->label;
  int l;  /* index of new label being created */
  luai_checkrepeated(fs, ll, label);  /* luai_check for repeated labels */
  luai_checknext(ls, LUAI_TK_DBCOLON);  /* skip double colon */
  /* create new entry for this label */
  l = luai_newlabelentry(ls, ll, label, line, luaK_getlabel(fs));
  luai_skipnoopstat(ls);  /* skip other no-op statements */
  if (luai_block_follow(ls, 0)) {  /* label is last no-op luai_statement in the luai_getblock? */
    /* assume that locals are already out of scope */
    ll->arr[l].nactvar = fs->bl->nactvar;
  }
  luai_findgotos(ls, &ll->arr[l]);
}


static void luai_whilestat (luai_LexState *ls, int line) {
  /* luai_whilestat -> WHILE luai_cond DO luai_getblock END */
  luai_FuncState *fs = ls->fs;
  int whileinit;
  int condexit;
  luai_BlockCnt bl;
  luaX_next(ls);  /* skip WHILE */
  whileinit = luaK_getlabel(fs);
  condexit = luai_cond(ls);
  luai_enterblock(fs, &bl, 1);
  luai_checknext(ls, LUAI_TK_DO);
  luai_getblock(ls);
  luaK_jumpto(fs, whileinit);
  luai_check_match(ls, LUAI_TK_END, LUAI_TK_WHILE, line);
  luai_leaveblock(fs);
  luaK_patchtohere(fs, condexit);  /* false conditions finish the loop */
}


static void luai_repeatstat (luai_LexState *ls, int line) {
  /* luai_repeatstat -> REPEAT luai_getblock UNTIL luai_cond */
  int condexit;
  luai_FuncState *fs = ls->fs;
  int repeat_init = luaK_getlabel(fs);
  luai_BlockCnt bl1, bl2;
  luai_enterblock(fs, &bl1, 1);  /* loop luai_getblock */
  luai_enterblock(fs, &bl2, 0);  /* scope luai_getblock */
  luaX_next(ls);  /* skip REPEAT */
  luai_statlist(ls);
  luai_check_match(ls, LUAI_TK_UNTIL, LUAI_TK_REPEAT, line);
  condexit = luai_cond(ls);  /* read condition (inside scope luai_getblock) */
  if (bl2.upval)  /* upvalues? */
    luaK_patchclose(fs, condexit, bl2.nactvar);
  luai_leaveblock(fs);  /* finish scope */
  luaK_patchlist(fs, condexit, repeat_init);  /* close the loop */
  luai_leaveblock(fs);  /* finish loop */
}


static int luai_exp1 (luai_LexState *ls) {
  luai_expdesc e;
  int reg;
  luai_expr(ls, &e);
  luaK_exp2nextreg(ls->fs, &e);
  lua_assert(e.k == LUAI_VNONRELOC);
  reg = e.u.info;
  return reg;
}


static void luai_forbody (luai_LexState *ls, int base, int line, int nvars, int isnum) {
  /* luai_forbody -> DO luai_getblock */
  luai_BlockCnt bl;
  luai_FuncState *fs = ls->fs;
  int prep, endfor;
  luai_ajustlocalvars(ls, 3);  /* control variables */
  luai_checknext(ls, LUAI_TK_DO);
  prep = isnum ? luaK_codeAsBx(fs, luai_OP_FORPREP, base, LUAI_NO_JUMP) : luaK_jump(fs);
  luai_enterblock(fs, &bl, 0);  /* scope for declared variables */
  luai_ajustlocalvars(ls, nvars);
  luaK_reserveregs(fs, nvars);
  luai_getblock(ls);
  luai_leaveblock(fs);  /* end of scope for declared variables */
  luaK_patchtohere(fs, prep);
  if (isnum)  /* numeric for? */
    endfor = luaK_codeAsBx(fs, luai_OP_FORLOOP, base, LUAI_NO_JUMP);
  else {  /* generic for */
    luaK_codeABC(fs, luai_OP_TFORCALL, base, 0, nvars);
    luaK_fixline(fs, line);
    endfor = luaK_codeAsBx(fs, luai_OP_TFORLOOP, base + 2, LUAI_NO_JUMP);
  }
  luaK_patchlist(fs, endfor, prep + 1);
  luaK_fixline(fs, line);
}


static void luai_fornum (luai_LexState *ls, luai_TString *varname, int line) {
  /* luai_fornum -> NAME = luai_exp1,luai_exp1[,luai_exp1] luai_forbody */
  luai_FuncState *fs = ls->fs;
  int base = fs->luai_freereg;
  luai_new_localvarliteral(ls, "(for index)");
  luai_new_localvarliteral(ls, "(for limit)");
  luai_new_localvarliteral(ls, "(for step)");
  luai_new_localvar(ls, varname);
  luai_checknext(ls, '=');
  luai_exp1(ls);  /* initial value */
  luai_checknext(ls, ',');
  luai_exp1(ls);  /* limit */
  if (luai_testnext(ls, ','))
    luai_exp1(ls);  /* optional step */
  else {  /* default step = 1 */
    luaK_codek(fs, fs->luai_freereg, luaK_intK(fs, 1));
    luaK_reserveregs(fs, 1);
  }
  luai_forbody(ls, base, line, 1, 1);
}


static void luai_forlist (luai_LexState *ls, luai_TString *indexname) {
  /* luai_forlist -> NAME {,NAME} IN luai_explist luai_forbody */
  luai_FuncState *fs = ls->fs;
  luai_expdesc e;
  int nvars = 4;  /* gen, state, control, plus at least one declared var */
  int line;
  int base = fs->luai_freereg;
  /* create control variables */
  luai_new_localvarliteral(ls, "(for generator)");
  luai_new_localvarliteral(ls, "(for state)");
  luai_new_localvarliteral(ls, "(for control)");
  /* create declared variables */
  luai_new_localvar(ls, indexname);
  while (luai_testnext(ls, ',')) {
    luai_new_localvar(ls, luai_str_checkname(ls));
    nvars++;
  }
  luai_checknext(ls, LUAI_TK_IN);
  line = ls->linenumber;
  luai_adjust_assign(ls, 3, luai_explist(ls, &e), &e);
  luaK_checkstack(fs, 3);  /* extra space to call generator */
  luai_forbody(ls, base, line, nvars - 3, 0);
}


static void luai_forstat (luai_LexState *ls, int line) {
  /* luai_forstat -> FOR (luai_fornum | luai_forlist) END */
  luai_FuncState *fs = ls->fs;
  luai_TString *varname;
  luai_BlockCnt bl;
  luai_enterblock(fs, &bl, 1);  /* scope for loop and control variables */
  luaX_next(ls);  /* skip 'for' */
  varname = luai_str_checkname(ls);  /* first variable name */
  switch (ls->t.token) {
    case '=': luai_fornum(ls, varname, line); break;
    case ',': case LUAI_TK_IN: luai_forlist(ls, varname); break;
    default: luaX_syntaxerror(ls, "'=' or 'in' expected");
  }
  luai_check_match(ls, LUAI_TK_END, LUAI_TK_FOR, line);
  luai_leaveblock(fs);  /* loop scope ('break' jumps to this point) */
}


static void luai_test_then_block (luai_LexState *ls, int *escapelist) {
  /* luai_test_then_block -> [IF | ELSEIF] luai_cond THEN luai_getblock */
  luai_BlockCnt bl;
  luai_FuncState *fs = ls->fs;
  luai_expdesc v;
  int jf;  /* instruction to skip 'then' code (if condition is false) */
  luaX_next(ls);  /* skip IF or ELSEIF */
  luai_expr(ls, &v);  /* read condition */
  luai_checknext(ls, LUAI_TK_THEN);
  if (ls->t.token == LUAI_TK_GOTO || ls->t.token == LUAI_TK_BREAK) {
    luaK_goiffalse(ls->fs, &v);  /* will jump to label if condition is true */
    luai_enterblock(fs, &bl, 0);  /* must enter luai_getblock before 'goto' */
    luai_gotostat(ls, v.t);  /* handle goto/break */
    luai_skipnoopstat(ls);  /* skip other no-op statements */
    if (luai_block_follow(ls, 0)) {  /* 'goto' is the entire luai_getblock? */
      luai_leaveblock(fs);
      return;  /* and that is it */
    }
    else  /* must skip over 'then' part if condition is false */
      jf = luaK_jump(fs);
  }
  else {  /* regular case (not goto/break) */
    luaK_goiftrue(ls->fs, &v);  /* skip over luai_getblock if condition is false */
    luai_enterblock(fs, &bl, 0);
    jf = v.f;
  }
  luai_statlist(ls);  /* 'then' part */
  luai_leaveblock(fs);
  if (ls->t.token == LUAI_TK_ELSE ||
      ls->t.token == LUAI_TK_ELSEIF)  /* followed by 'else'/'elseif'? */
    luaK_concat(fs, escapelist, luaK_jump(fs));  /* must jump over it */
  luaK_patchtohere(fs, jf);
}


static void luai_ifstat (luai_LexState *ls, int line) {
  /* luai_ifstat -> IF luai_cond THEN luai_getblock {ELSEIF luai_cond THEN luai_getblock} [ELSE luai_getblock] END */
  luai_FuncState *fs = ls->fs;
  int escapelist = LUAI_NO_JUMP;  /* exit list for finished parts */
  luai_test_then_block(ls, &escapelist);  /* IF luai_cond THEN luai_getblock */
  while (ls->t.token == LUAI_TK_ELSEIF)
    luai_test_then_block(ls, &escapelist);  /* ELSEIF luai_cond THEN luai_getblock */
  if (luai_testnext(ls, LUAI_TK_ELSE))
    luai_getblock(ls);  /* 'else' part */
  luai_check_match(ls, LUAI_TK_END, LUAI_TK_IF, line);
  luaK_patchtohere(fs, escapelist);  /* patch escape list to 'if' end */
}


static void luai_localfunc (luai_LexState *ls) {
  luai_expdesc b;
  luai_FuncState *fs = ls->fs;
  luai_new_localvar(ls, luai_str_checkname(ls));  /* new local variable */
  luai_ajustlocalvars(ls, 1);  /* enter its scope */
  luai_body(ls, &b, 0, ls->linenumber);  /* function created in luai_next register */
  /* debug information will only see the variable after this point! */
  luai_getlocvar(fs, b.u.info)->startpc = fs->pc;
}


static void luai_localstat (luai_LexState *ls) {
  /* stat -> LOCAL NAME {',' NAME} ['=' luai_explist] */
  int nvars = 0;
  int nexps;
  luai_expdesc e;
  do {
    luai_new_localvar(ls, luai_str_checkname(ls));
    nvars++;
  } while (luai_testnext(ls, ','));
  if (luai_testnext(ls, '='))
    nexps = luai_explist(ls, &e);
  else {
    e.k = LUAI_VVOID;
    nexps = 0;
  }
  luai_adjust_assign(ls, nvars, nexps, &e);
  luai_ajustlocalvars(ls, nvars);
}


static int luai_funcname (luai_LexState *ls, luai_expdesc *v) {
  /* luai_funcname -> NAME {luai_fieldsel} [':' NAME] */
  int ismethod = 0;
  luai_singlevar(ls, v);
  while (ls->t.token == '.')
    luai_fieldsel(ls, v);
  if (ls->t.token == ':') {
    ismethod = 1;
    luai_fieldsel(ls, v);
  }
  return ismethod;
}


static void luai_funcstat (luai_LexState *ls, int line) {
  /* luai_funcstat -> FUNCTION luai_funcname luai_body */
  int ismethod;
  luai_expdesc v, b;
  luaX_next(ls);  /* skip FUNCTION */
  ismethod = luai_funcname(ls, &v);
  luai_body(ls, &b, ismethod, line);
  luaK_storevar(ls->fs, &v, &b);
  luaK_fixline(ls->fs, line);  /* definition "happens" in the first line */
}


static void luai_exprstat (luai_LexState *ls) {
  /* stat -> func | luai_assignment */
  luai_FuncState *fs = ls->fs;
  struct luai_LHS_assign v;
  luai_suffixedexp(ls, &v.v);
  if (ls->t.token == '=' || ls->t.token == ',') { /* stat -> luai_assignment ? */
    v.prev = NULL;
    luai_assignment(ls, &v, 1);
  }
  else {  /* stat -> func */
    luai_check_condition(ls, v.v.k == LUAI_VCALL, "syntax error");
    luai_SETARG_C(luai_getinstruction(fs, &v.v), 1);  /* call luai_statement uses no results */
  }
}


static void luai_retstat (luai_LexState *ls) {
  /* stat -> RETURN [luai_explist] [';'] */
  luai_FuncState *fs = ls->fs;
  luai_expdesc e;
  int first, nret;  /* registers with returned values */
  if (luai_block_follow(ls, 1) || ls->t.token == ';')
    first = nret = 0;  /* return no values */
  else {
    nret = luai_explist(ls, &e);  /* optional return values */
    if (hasmultret(e.k)) {
      luaK_setmultret(fs, &e);
      if (e.k == LUAI_VCALL && nret == 1) {  /* tail call? */
        luai_SETOPCODE(luai_getinstruction(fs,&e), luai_OP_TAILCALL);
        lua_assert(luai_GETARG_A(luai_getinstruction(fs,&e)) == fs->nactvar);
      }
      first = fs->nactvar;
      nret = LUA_MULTRET;  /* return all values */
    }
    else {
      if (nret == 1)  /* only one single value? */
        first = luaK_exp2anyreg(fs, &e);
      else {
        luaK_exp2nextreg(fs, &e);  /* values must go to the stack */
        first = fs->nactvar;  /* return all active values */
        lua_assert(nret == fs->luai_freereg - first);
      }
    }
  }
  luaK_ret(fs, first, nret);
  luai_testnext(ls, ';');  /* skip optional semicolon */
}


static void luai_statement (luai_LexState *ls) {
  int line = ls->linenumber;  /* may be needed for error messages */
  luai_enterlevel(ls);
  switch (ls->t.token) {
    case ';': {  /* stat -> ';' (empty luai_statement) */
      luaX_next(ls);  /* skip ';' */
      break;
    }
    case LUAI_TK_IF: {  /* stat -> luai_ifstat */
      luai_ifstat(ls, line);
      break;
    }
    case LUAI_TK_WHILE: {  /* stat -> luai_whilestat */
      luai_whilestat(ls, line);
      break;
    }
    case LUAI_TK_DO: {  /* stat -> DO luai_getblock END */
      luaX_next(ls);  /* skip DO */
      luai_getblock(ls);
      luai_check_match(ls, LUAI_TK_END, LUAI_TK_DO, line);
      break;
    }
    case LUAI_TK_FOR: {  /* stat -> luai_forstat */
      luai_forstat(ls, line);
      break;
    }
    case LUAI_TK_REPEAT: {  /* stat -> luai_repeatstat */
      luai_repeatstat(ls, line);
      break;
    }
    case LUAI_TK_FUNCTION: {  /* stat -> luai_funcstat */
      luai_funcstat(ls, line);
      break;
    }
    case LUAI_TK_LOCAL: {  /* stat -> luai_localstat */
      luaX_next(ls);  /* skip LOCAL */
      if (luai_testnext(ls, LUAI_TK_FUNCTION))  /* local function? */
        luai_localfunc(ls);
      else
        luai_localstat(ls);
      break;
    }
    case LUAI_TK_DBCOLON: {  /* stat -> label */
      luaX_next(ls);  /* skip double colon */
      luai_labelstat(ls, luai_str_checkname(ls), line);
      break;
    }
    case LUAI_TK_RETURN: {  /* stat -> luai_retstat */
      luaX_next(ls);  /* skip RETURN */
      luai_retstat(ls);
      break;
    }
    case LUAI_TK_BREAK:   /* stat -> breakstat */
    case LUAI_TK_GOTO: {  /* stat -> 'goto' NAME */
      luai_gotostat(ls, luaK_jump(ls->fs));
      break;
    }
    default: {  /* stat -> func | luai_assignment */
      luai_exprstat(ls);
      break;
    }
  }
  lua_assert(ls->fs->f->maxstacksize >= ls->fs->luai_freereg &&
             ls->fs->luai_freereg >= ls->fs->nactvar);
  ls->fs->luai_freereg = ls->fs->nactvar;  /* free registers */
  luai_leavelevel(ls);
}

/* }====================================================================== */


/*
** compiles the main function, which is a regular vararg function with an
** upvalue named LUA_ENV
*/
static void luai_mainfunc (luai_LexState *ls, luai_FuncState *fs) {
  luai_BlockCnt bl;
  luai_expdesc v;
  luai_open_func(ls, fs, &bl);
  fs->f->is_vararg = 1;  /* main function is always declared vararg */
  luai_init_exp(&v, LUAI_VLOCAL, 0);  /* create and... */
  luai_newupvalue(fs, ls->envn, &v);  /* ...set environment upvalue */
  luaX_next(ls);  /* read first token */
  luai_statlist(ls);  /* parse main luai_body */
  luai_check(ls, LUAI_TK_EOS);
  luai_close_func(ls);
}


luai_LClosure *luaY_parser (lua_State *L, LUAI_ZIO *z, luai_Mbuffer *buff,
                       luai_Dyndata *dyd, const char *name, int firstchar) {
  luai_LexState lexstate;
  luai_FuncState luai_funcstate;
  luai_LClosure *cl = luaF_newLclosure(L, 1);  /* create main closure */
  luai_setclLvalue(L, L->top, cl);  /* anchor it (to avoid being collected) */
  luaD_inctop(L);
  lexstate.h = luaH_new(L);  /* create table for scanner */
  luai_sethvalue(L, L->top, lexstate.h);  /* anchor it */
  luaD_inctop(L);
  luai_funcstate.f = cl->p = luaF_newproto(L);
  luai_funcstate.f->source = luaS_new(L, name);  /* create and anchor luai_TString */
  lua_assert(luai_iswhite(luai_funcstate.f));  /* do not need barrier here */
  lexstate.buff = buff;
  lexstate.dyd = dyd;
  dyd->actvar.n = dyd->gt.n = dyd->label.n = 0;
  luaX_setinput(L, &lexstate, z, luai_funcstate.f->source, firstchar);
  luai_mainfunc(&lexstate, &luai_funcstate);
  lua_assert(!luai_funcstate.prev && luai_funcstate.nups == 1 && !lexstate.fs);
  /* all scopes should be correctly finished */
  lua_assert(dyd->actvar.n == 0 && dyd->gt.n == 0 && dyd->label.n == 0);
  L->top--;  /* remove scanner's table */
  return cl;  /* closure is on the stack, too */
}

/*__lstate.c__*/

#if !defined(LUAI_GCPAUSE)
#define LUAI_GCPAUSE	200  /* 200% */
#endif

#if !defined(LUAI_GCMUL)
#define LUAI_GCMUL	200 /* LUAI_GC runs 'twice the speed' of memory allocation */
#endif


/*
** a macro to help the creation of a unique random seed when a state is
** created; the seed is used to randomize hashes.
*/
#if !defined(makeseed)
#include <time.h>
#define makeseed()		luai_cast(unsigned int, time(NULL))
#endif



/*
** thread state + extra space
*/
typedef struct luai_LX {
  luai_lu_byte extra_[LUA_EXTRASPACE];
  lua_State l;
} luai_LX;


/*
** Main thread combines a thread state and the global state
*/
typedef struct luai_LG {
  luai_LX l;
  luai_global_State g;
} luai_LG;



#define luai_fromstate(L)	(luai_cast(luai_LX *, luai_cast(luai_lu_byte *, (L)) - offsetof(luai_LX, l)))


/*
** Compute an initial seed as random as possible. Rely on Address Space
** Layout Randomization (if present) to increase randomness..
*/
#define luai_addbuff(b,p,e) \
  { size_t t = luai_cast(size_t, e); \
    memcpy(b + p, &t, sizeof(t)); p += sizeof(t); }

static unsigned int luai_makeseed (lua_State *L) {
  char buff[4 * sizeof(size_t)];
  unsigned int h = makeseed();
  int p = 0;
  luai_addbuff(buff, p, L);  /* heap variable */
  luai_addbuff(buff, p, &h);  /* local variable */
  luai_addbuff(buff, p, luaO_nilobject);  /* global variable */
  luai_addbuff(buff, p, &lua_newstate);  /* public function */
  lua_assert(p == sizeof(buff));
  return luaS_hash(buff, p, h);
}


/*
** set LUAI_GCdebt to a new value keeping the value (totalbytes + LUAI_GCdebt)
** invariant (and avoiding underflows in 'totalbytes')
*/
void luaE_setdebt (luai_global_State *g, luai_l_mem debt) {
  luai_l_mem tb = luai_gettotalbytes(g);
  lua_assert(tb > 0);
  if (debt < tb - LUAI_MAX_LMEM)
    debt = tb - LUAI_MAX_LMEM;  /* will make 'totalbytes == LUAI_MAX_LMEM' */
  g->totalbytes = tb - debt;
  g->LUAI_GCdebt = debt;
}


luai_CallInfo *luaE_extendCI (lua_State *L) {
  luai_CallInfo *ci = luaM_new(L, luai_CallInfo);
  lua_assert(L->ci->luai_next == NULL);
  L->ci->luai_next = ci;
  ci->previous = L->ci;
  ci->luai_next = NULL;
  L->nci++;
  return ci;
}


/*
** free all luai_CallInfo structures not in use by a thread
*/
void luaE_freeCI (lua_State *L) {
  luai_CallInfo *ci = L->ci;
  luai_CallInfo *luai_next = ci->luai_next;
  ci->luai_next = NULL;
  while ((ci = luai_next) != NULL) {
    luai_next = ci->luai_next;
    luaM_free(L, ci);
    L->nci--;
  }
}


/*
** free half of the luai_CallInfo structures not in use by a thread
*/
void luaE_shrinkCI (lua_State *L) {
  luai_CallInfo *ci = L->ci;
  luai_CallInfo *next2;  /* luai_next's luai_next */
  /* while there are two nexts */
  while (ci->luai_next != NULL && (next2 = ci->luai_next->luai_next) != NULL) {
    luaM_free(L, ci->luai_next);  /* free luai_next */
    L->nci--;
    ci->luai_next = next2;  /* remove 'luai_next' from the list */
    next2->previous = ci;
    ci = next2;  /* keep luai_next's luai_next */
  }
}


static void luai_stack_init (lua_State *L1, lua_State *L) {
  int i; luai_CallInfo *ci;
  /* initialize stack array */
  L1->stack = luaM_newvector(L, LUAI_BASIC_STACK_SIZE, luai_TValue);
  L1->stacksize = LUAI_BASIC_STACK_SIZE;
  for (i = 0; i < LUAI_BASIC_STACK_SIZE; i++)
    luai_setnilvalue(L1->stack + i);  /* erase new stack */
  L1->top = L1->stack;
  L1->stack_last = L1->stack + L1->stacksize - LUAI_EXTRA_STACK;
  /* initialize first ci */
  ci = &L1->base_ci;
  ci->luai_next = ci->previous = NULL;
  ci->callstatus = 0;
  ci->func = L1->top;
  luai_setnilvalue(L1->top++);  /* 'function' entry for this 'ci' */
  ci->top = L1->top + LUA_MINSTACK;
  L1->ci = ci;
}


static void luai_freestack (lua_State *L) {
  if (L->stack == NULL)
    return;  /* stack not completely built yet */
  L->ci = &L->base_ci;  /* free the entire 'ci' list */
  luaE_freeCI(L);
  lua_assert(L->nci == 0);
  luaM_freearray(L, L->stack, L->stacksize);  /* free stack array */
}


/*
** Create registry table and its predefined values
*/
static void init_registry (lua_State *L, luai_global_State *g) {
  luai_TValue temp;
  /* create registry */
  luai_Table *registry = luaH_new(L);
  luai_sethvalue(L, &g->luai_l_registry, registry);
  luaH_resize(L, registry, LUA_RIDX_LAST, 0);
  /* registry[LUA_RIDX_MAINTHREAD] = L */
  luai_setthvalue(L, &temp, L);  /* temp = L */
  luaH_setint(L, registry, LUA_RIDX_MAINTHREAD, &temp);
  /* registry[LUA_RIDX_GLOBALS] = table of globals */
  luai_sethvalue(L, &temp, luaH_new(L));  /* temp = new table (global table) */
  luaH_setint(L, registry, LUA_RIDX_GLOBALS, &temp);
}


/*
** open parts of the state that may cause memory-allocation errors.
** ('g->version' != NULL flags that the state was completely build)
*/
static void luai_f_luaopen (lua_State *L, void *ud) {
  luai_global_State *g = LUAI_G(L);
  LUAI_UNUSED(ud);
  luai_stack_init(L, L);  /* init stack */
  init_registry(L, g);
  luaS_init(L);
  luaT_init(L);
  luaX_init(L);
  g->gcrunning = 1;  /* allow gc */
  g->version = lua_version(NULL);
  luai_userstateopen(L);
}


/*
** preinitialize a thread with consistent values without allocating
** any memory (to avoid errors)
*/
static void luai_preinit_thread (lua_State *L, luai_global_State *g) {
  LUAI_G(L) = g;
  L->stack = NULL;
  L->ci = NULL;
  L->nci = 0;
  L->stacksize = 0;
  L->twups = L;  /* thread has no upvalues */
  L->errorJmp = NULL;
  L->nCcalls = 0;
  L->hook = NULL;
  L->hookmask = 0;
  L->basehookcount = 0;
  L->allowhook = 1;
  luai_resethookcount(L);
  L->openupval = NULL;
  L->nny = 1;
  L->status = LUA_OK;
  L->errfunc = 0;
}


static void luai_close_state (lua_State *L) {
  luai_global_State *g = LUAI_G(L);
  luaF_close(L, L->stack);  /* close all upvalues for this thread */
  luaC_freeallobjects(L);  /* collect all objects */
  if (g->version)  /* closing a fully built state? */
    luai_userstateclose(L);
  luaM_freearray(L, LUAI_G(L)->strt.hash, LUAI_G(L)->strt.size);
  luai_freestack(L);
  lua_assert(luai_gettotalbytes(g) == sizeof(luai_LG));
  (*g->frealloc)(g->ud, luai_fromstate(L), sizeof(luai_LG), 0);  /* free main luai_getblock */
}


LUA_API lua_State *lua_newthread (lua_State *L) {
  luai_global_State *g = LUAI_G(L);
  lua_State *L1;
  lua_lock(L);
  luaC_checkGC(L);
  /* create new thread */
  L1 = &luai_cast(luai_LX *, luaM_newobject(L, LUA_TTHREAD, sizeof(luai_LX)))->l;
  L1->marked = luaC_white(g);
  L1->tt = LUA_TTHREAD;
  /* link it on list 'allgc' */
  L1->luai_next = g->allgc;
  g->allgc = luai_obj2gco(L1);
  /* anchor it on L stack */
  luai_setthvalue(L, L->top, L1);
  luai_api_incr_top(L);
  luai_preinit_thread(L1, g);
  L1->hookmask = L->hookmask;
  L1->basehookcount = L->basehookcount;
  L1->hook = L->hook;
  luai_resethookcount(L1);
  /* initialize L1 extra space */
  memcpy(lua_getextraspace(L1), lua_getextraspace(g->mainthread),
         LUA_EXTRASPACE);
  luai_userstatethread(L, L1);
  luai_stack_init(L1, L);  /* init stack */
  lua_unlock(L);
  return L1;
}


void luaE_freethread (lua_State *L, lua_State *L1) {
  luai_LX *l = luai_fromstate(L1);
  luaF_close(L1, L1->stack);  /* close all upvalues for this thread */
  lua_assert(L1->openupval == NULL);
  luai_userstatefree(L, L1);
  luai_freestack(L1);
  luaM_free(L, l);
}


LUA_API lua_State *lua_newstate (lua_Alloc f, void *ud) {
  int i;
  lua_State *L;
  luai_global_State *g;
  luai_LG *l = luai_cast(luai_LG *, (*f)(ud, NULL, LUA_TTHREAD, sizeof(luai_LG)));
  if (l == NULL) return NULL;
  L = &l->l.l;
  g = &l->g;
  L->luai_next = NULL;
  L->tt = LUA_TTHREAD;
  g->currentwhite = luai_bitmask(LUAI_WHITE0BIT);
  L->marked = luaC_white(g);
  luai_preinit_thread(L, g);
  g->frealloc = f;
  g->ud = ud;
  g->mainthread = L;
  g->seed = luai_makeseed(L);
  g->gcrunning = 0;  /* no LUAI_GC while building state */
  g->LUAI_GCestimate = 0;
  g->strt.size = g->strt.nuse = 0;
  g->strt.hash = NULL;
  luai_setnilvalue(&g->luai_l_registry);
  g->panic = NULL;
  g->version = NULL;
  g->gcstate = LUAI_GCSpause;
  g->gckind = LUAI_KGC_NORMAL;
  g->allgc = g->finobj = g->tobefnz = g->fixedgc = NULL;
  g->sweepgc = NULL;
  g->gray = g->grayagain = NULL;
  g->weak = g->ephemeron = g->allweak = NULL;
  g->twups = NULL;
  g->totalbytes = sizeof(luai_LG);
  g->LUAI_GCdebt = 0;
  g->gcfinnum = 0;
  g->gcpause = LUAI_GCPAUSE;
  g->gcstepmul = LUAI_GCMUL;
  for (i=0; i < LUA_NUMTAGS; i++) g->mt[i] = NULL;
  if (luaD_rawrunprotected(L, luai_f_luaopen, NULL) != LUA_OK) {
    /* memory allocation error: free partial state */
    luai_close_state(L);
    L = NULL;
  }
  return L;
}


LUA_API void lua_close (lua_State *L) {
  L = LUAI_G(L)->mainthread;  /* only the main thread can be closed */
  lua_lock(L);
  luai_close_state(L);
}

/*__lstring.c__*/

#define MEMERRMSG       "not enough memory"


/*
** Lua will use at most ~(2^LUAI_HASHLIMIT) bytes from a string to
** compute its hash
*/
#if !defined(LUAI_HASHLIMIT)
#define LUAI_HASHLIMIT		5
#endif


/*
** equality for long strings
*/
int luaS_eqlngstr (luai_TString *a, luai_TString *b) {
  size_t len = a->u.lnglen;
  lua_assert(a->tt == LUA_TLNGSTR && b->tt == LUA_TLNGSTR);
  return (a == b) ||  /* same instance or... */
    ((len == b->u.lnglen) &&  /* equal length and ... */
     (memcmp(luai_getstr(a), luai_getstr(b), len) == 0));  /* equal contents */
}


unsigned int luaS_hash (const char *str, size_t l, unsigned int seed) {
  unsigned int h = seed ^ luai_cast(unsigned int, l);
  size_t step = (l >> LUAI_HASHLIMIT) + 1;
  for (; l >= step; l -= step)
    h ^= ((h<<5) + (h>>2) + luai_cast_byte(str[l - 1]));
  return h;
}


unsigned int luaS_hashlongstr (luai_TString *ts) {
  lua_assert(ts->tt == LUA_TLNGSTR);
  if (ts->extra == 0) {  /* no hash? */
    ts->hash = luaS_hash(luai_getstr(ts), ts->u.lnglen, ts->hash);
    ts->extra = 1;  /* now it has its hash */
  }
  return ts->hash;
}


/*
** resizes the string table
*/
void luaS_resize (lua_State *L, int newsize) {
  int i;
  luai_stringtable *tb = &LUAI_G(L)->strt;
  if (newsize > tb->size) {  /* grow table if needed */
    luaM_reallocvector(L, tb->hash, tb->size, newsize, luai_TString *);
    for (i = tb->size; i < newsize; i++)
      tb->hash[i] = NULL;
  }
  for (i = 0; i < tb->size; i++) {  /* luai_rehash */
    luai_TString *p = tb->hash[i];
    tb->hash[i] = NULL;
    while (p) {  /* for each node in the list */
      luai_TString *hnext = p->u.hnext;  /* luai_save luai_next */
      unsigned int h = luai_lmod(p->hash, newsize);  /* new position */
      p->u.hnext = tb->hash[h];  /* chain it */
      tb->hash[h] = p;
      p = hnext;
    }
  }
  if (newsize < tb->size) {  /* shrink table if needed */
    /* vanishing slice should be empty */
    lua_assert(tb->hash[newsize] == NULL && tb->hash[tb->size - 1] == NULL);
    luaM_reallocvector(L, tb->hash, tb->size, newsize, luai_TString *);
  }
  tb->size = newsize;
}


/*
** Clear API string cache. (Entries cannot be empty, so fill them with
** a non-collectable string.)
*/
void luaS_clearcache (luai_global_State *g) {
  int i, j;
  for (i = 0; i < LUAI_STRCACHE_N; i++)
    for (j = 0; j < LUAI_STRCACHE_M; j++) {
    if (luai_iswhite(g->strcache[i][j]))  /* will entry be collected? */
      g->strcache[i][j] = g->memerrmsg;  /* replace it with something fixed */
    }
}


/*
** Initialize the string table and the string cache
*/
void luaS_init (lua_State *L) {
  luai_global_State *g = LUAI_G(L);
  int i, j;
  luaS_resize(L, LUAI_MINSTRTABSIZE);  /* initial size of string table */
  /* pre-create memory-error message */
  g->memerrmsg = luaS_newliteral(L, MEMERRMSG);
  luaC_fix(L, luai_obj2gco(g->memerrmsg));  /* it should never be collected */
  for (i = 0; i < LUAI_STRCACHE_N; i++)  /* fill cache with valid strings */
    for (j = 0; j < LUAI_STRCACHE_M; j++)
      g->strcache[i][j] = g->memerrmsg;
}



/*
** creates a new string object
*/
static luai_TString *luai_createstrobj (lua_State *L, size_t l, int tag, unsigned int h) {
  luai_TString *ts;
  LUAI_GCObject *o;
  size_t totalsize;  /* total size of luai_TString object */
  totalsize = luai_sizelstring(l);
  o = luaC_newobj(L, tag, totalsize);
  ts = luai_gco2ts(o);
  ts->hash = h;
  ts->extra = 0;
  luai_getstr(ts)[l] = '\0';  /* ending 0 */
  return ts;
}


luai_TString *luaS_createlngstrobj (lua_State *L, size_t l) {
  luai_TString *ts = luai_createstrobj(L, l, LUA_TLNGSTR, LUAI_G(L)->seed);
  ts->u.lnglen = l;
  return ts;
}


void luaS_remove (lua_State *L, luai_TString *ts) {
  luai_stringtable *tb = &LUAI_G(L)->strt;
  luai_TString **p = &tb->hash[luai_lmod(ts->hash, tb->size)];
  while (*p != ts)  /* find previous element */
    p = &(*p)->u.hnext;
  *p = (*p)->u.hnext;  /* remove element from its list */
  tb->nuse--;
}


/*
** checks whether short string exists and reuses it or creates a new one
*/
static luai_TString *luai_internshrstr (lua_State *L, const char *str, size_t l) {
  luai_TString *ts;
  luai_global_State *g = LUAI_G(L);
  unsigned int h = luaS_hash(str, l, g->seed);
  luai_TString **list = &g->strt.hash[luai_lmod(h, g->strt.size)];
  lua_assert(str != NULL);  /* otherwise 'memcmp'/'memcpy' are undefined */
  for (ts = *list; ts != NULL; ts = ts->u.hnext) {
    if (l == ts->shrlen &&
        (memcmp(str, luai_getstr(ts), l * sizeof(char)) == 0)) {
      /* found! */
      if (luai_isdead(g, ts))  /* dead (but not collected yet)? */
        luai_changewhite(ts);  /* resurrect it */
      return ts;
    }
  }
  if (g->strt.nuse >= g->strt.size && g->strt.size <= LUAI_MAX_INT/2) {
    luaS_resize(L, g->strt.size * 2);
    list = &g->strt.hash[luai_lmod(h, g->strt.size)];  /* recompute with new size */
  }
  ts = luai_createstrobj(L, l, LUA_TSHRSTR, h);
  memcpy(luai_getstr(ts), str, l * sizeof(char));
  ts->shrlen = luai_cast_byte(l);
  ts->u.hnext = *list;
  *list = ts;
  g->strt.nuse++;
  return ts;
}


/*
** new string (with explicit length)
*/
luai_TString *luaS_newlstr (lua_State *L, const char *str, size_t l) {
  if (l <= LUAI_MAXSHORTLEN)  /* short string? */
    return luai_internshrstr(L, str, l);
  else {
    luai_TString *ts;
    if (l >= (LUAI_MAX_SIZE - sizeof(luai_TString))/sizeof(char))
      luaM_toobig(L);
    ts = luaS_createlngstrobj(L, l);
    memcpy(luai_getstr(ts), str, l * sizeof(char));
    return ts;
  }
}


/*
** Create or reuse a zero-terminated string, first checking in the
** cache (using the string address as a key). The cache can contain
** only zero-terminated strings, so it is safe to use 'strcmp' to
** luai_check hits.
*/
luai_TString *luaS_new (lua_State *L, const char *str) {
  unsigned int i = luai_point2uint(str) % LUAI_STRCACHE_N;  /* hash */
  int j;
  luai_TString **p = LUAI_G(L)->strcache[i];
  for (j = 0; j < LUAI_STRCACHE_M; j++) {
    if (strcmp(str, luai_getstr(p[j])) == 0)  /* hit? */
      return p[j];  /* that is it */
  }
  /* normal route */
  for (j = LUAI_STRCACHE_M - 1; j > 0; j--)
    p[j] = p[j - 1];  /* move out last element */
  /* new element is first in the list */
  p[0] = luaS_newlstr(L, str, strlen(str));
  return p[0];
}


luai_Udata *luaS_newudata (lua_State *L, size_t s) {
  luai_Udata *u;
  LUAI_GCObject *o;
  if (s > LUAI_MAX_SIZE - sizeof(luai_Udata))
    luaM_toobig(L);
  o = luaC_newobj(L, LUA_TUSERDATA, luai_sizeludata(s));
  u = luai_gco2u(o);
  u->len = s;
  u->metatable = NULL;
  luai_setuservalue(L, u, luaO_nilobject);
  return u;
}

/*__lstrlib.c__*/

/*
** maximum number of captures that a pattern can do during
** pattern-matching. This limit is arbitrary, but must fit in
** an unsigned char.
*/
#if !defined(LUA_MAXCAPTURES)
#define LUA_MAXCAPTURES		32
#endif


/* macro to 'unsign' a character */
#define luai_uchar(c)	((unsigned char)(c))


/*
** Some sizes are better limited to fit in 'int', but must also fit in
** 'size_t'. (We assume that 'lua_Integer' cannot be smaller than 'int'.)
*/
#define LUAI_MAX_SIZET	((size_t)(~(size_t)0))

#define LUAI_MAXSIZE  \
	(sizeof(size_t) < sizeof(int) ? LUAI_MAX_SIZET : (size_t)(INT_MAX))




static int luai_str_len (lua_State *L) {
  size_t l;
  luaL_checklstring(L, 1, &l);
  lua_pushinteger(L, (lua_Integer)l);
  return 1;
}


/* translate a relative string position: negative means back from end */
static lua_Integer luai_posrelat (lua_Integer pos, size_t len) {
  if (pos >= 0) return pos;
  else if (0u - (size_t)pos > len) return 0;
  else return (lua_Integer)len + pos + 1;
}


static int luai_str_sub (lua_State *L) {
  size_t l;
  const char *s = luaL_checklstring(L, 1, &l);
  lua_Integer start = luai_posrelat(luaL_checkinteger(L, 2), l);
  lua_Integer end = luai_posrelat(luaL_optinteger(L, 3, -1), l);
  if (start < 1) start = 1;
  if (end > (lua_Integer)l) end = l;
  if (start <= end)
    lua_pushlstring(L, s + start - 1, (size_t)(end - start) + 1);
  else lua_pushliteral(L, "");
  return 1;
}


static int luai_str_reverse (lua_State *L) {
  size_t l, i;
  luaL_Buffer b;
  const char *s = luaL_checklstring(L, 1, &l);
  char *p = luaL_buffinitsize(L, &b, l);
  for (i = 0; i < l; i++)
    p[i] = s[l - i - 1];
  luaL_pushresultsize(&b, l);
  return 1;
}


static int luai_str_lower (lua_State *L) {
  size_t l;
  size_t i;
  luaL_Buffer b;
  const char *s = luaL_checklstring(L, 1, &l);
  char *p = luaL_buffinitsize(L, &b, l);
  for (i=0; i<l; i++)
    p[i] = tolower(luai_uchar(s[i]));
  luaL_pushresultsize(&b, l);
  return 1;
}


static int luai_str_upper (lua_State *L) {
  size_t l;
  size_t i;
  luaL_Buffer b;
  const char *s = luaL_checklstring(L, 1, &l);
  char *p = luaL_buffinitsize(L, &b, l);
  for (i=0; i<l; i++)
    p[i] = toupper(luai_uchar(s[i]));
  luaL_pushresultsize(&b, l);
  return 1;
}


static int luai_str_rep (lua_State *L) {
  size_t l, lsep;
  const char *s = luaL_checklstring(L, 1, &l);
  lua_Integer n = luaL_checkinteger(L, 2);
  const char *sep = luaL_optlstring(L, 3, "", &lsep);
  if (n <= 0) lua_pushliteral(L, "");
  else if (l + lsep < l || l + lsep > LUAI_MAXSIZE / n)  /* may overflow? */
    return luaL_error(L, "resulting string too large");
  else {
    size_t totallen = (size_t)n * l + (size_t)(n - 1) * lsep;
    luaL_Buffer b;
    char *p = luaL_buffinitsize(L, &b, totallen);
    while (n-- > 1) {  /* first n-1 copies (followed by separator) */
      memcpy(p, s, l * sizeof(char)); p += l;
      if (lsep > 0) {  /* empty 'memcpy' is not that cheap */
        memcpy(p, sep, lsep * sizeof(char));
        p += lsep;
      }
    }
    memcpy(p, s, l * sizeof(char));  /* last copy (not followed by separator) */
    luaL_pushresultsize(&b, totallen);
  }
  return 1;
}


static int luai_str_byte (lua_State *L) {
  size_t l;
  const char *s = luaL_checklstring(L, 1, &l);
  lua_Integer posi = luai_posrelat(luaL_optinteger(L, 2, 1), l);
  lua_Integer pose = luai_posrelat(luaL_optinteger(L, 3, posi), l);
  int n, i;
  if (posi < 1) posi = 1;
  if (pose > (lua_Integer)l) pose = l;
  if (posi > pose) return 0;  /* empty interval; return no values */
  if (pose - posi >= INT_MAX)  /* arithmetic overflow? */
    return luaL_error(L, "string slice too long");
  n = (int)(pose -  posi) + 1;
  luaL_checkstack(L, n, "string slice too long");
  for (i=0; i<n; i++)
    lua_pushinteger(L, luai_uchar(s[posi+i-1]));
  return n;
}


static int luai_str_char (lua_State *L) {
  int n = lua_gettop(L);  /* number of arguments */
  int i;
  luaL_Buffer b;
  char *p = luaL_buffinitsize(L, &b, n);
  for (i=1; i<=n; i++) {
    lua_Integer c = luaL_checkinteger(L, i);
    luaL_argcheck(L, luai_uchar(c) == c, i, "value out of range");
    p[i - 1] = luai_uchar(c);
  }
  luaL_pushresultsize(&b, n);
  return 1;
}


static int luai_writer (lua_State *L, const void *b, size_t size, void *B) {
  (void)L;
  luaL_addlstring((luaL_Buffer *) B, (const char *)b, size);
  return 0;
}


static int luai_str_dump (lua_State *L) {
  luaL_Buffer b;
  int strip = lua_toboolean(L, 2);
  luaL_checktype(L, 1, LUA_TFUNCTION);
  lua_settop(L, 1);
  luaL_buffinit(L,&b);
  if (lua_dump(L, luai_writer, &b, strip) != 0)
    return luaL_error(L, "unable to dump given function");
  luaL_pushresult(&b);
  return 1;
}



/*
** {======================================================
** PATTERN MATCHING
** =======================================================
*/


#define LUAI_CAP_UNFINISHED	(-1)
#define LUAI_CAP_POSITION	(-2)


typedef struct luai_MatchState {
  const char *src_init;  /* init of source string */
  const char *src_end;  /* end ('\0') of source string */
  const char *p_end;  /* end ('\0') of pattern */
  lua_State *L;
  int matchdepth;  /* control for recursive depth (to avoid C stack overflow) */
  unsigned char level;  /* total number of captures (finished or unfinished) */
  struct {
    const char *init;
    ptrdiff_t len;
  } capture[LUA_MAXCAPTURES];
} luai_MatchState;


/* recursive function */
static const char *luai_match (luai_MatchState *ms, const char *s, const char *p);


/* maximum recursion depth for 'luai_match' */
#if !defined(LUAI_MAXCCALLS)
#define LUAI_MAXCCALLS	200
#endif


#define LUAI_L_ESC		'%'
#define LUAI_SPECIALS	"^$*+?.([%-"


static int luai_check_capture (luai_MatchState *ms, int l) {
  l -= '1';
  if (l < 0 || l >= ms->level || ms->capture[l].len == LUAI_CAP_UNFINISHED)
    return luaL_error(ms->L, "invalid capture index %%%d", l + 1);
  return l;
}


static int luai_capture_to_close (luai_MatchState *ms) {
  int level = ms->level;
  for (level--; level>=0; level--)
    if (ms->capture[level].len == LUAI_CAP_UNFINISHED) return level;
  return luaL_error(ms->L, "invalid pattern capture");
}


static const char *luai_classend (luai_MatchState *ms, const char *p) {
  switch (*p++) {
    case LUAI_L_ESC: {
      if (p == ms->p_end)
        luaL_error(ms->L, "malformed pattern (ends with '%%')");
      return p+1;
    }
    case '[': {
      if (*p == '^') p++;
      do {  /* look for a ']' */
        if (p == ms->p_end)
          luaL_error(ms->L, "malformed pattern (missing ']')");
        if (*(p++) == LUAI_L_ESC && p < ms->p_end)
          p++;  /* skip escapes (e.g. '%]') */
      } while (*p != ']');
      return p+1;
    }
    default: {
      return p;
    }
  }
}


static int luai_match_class (int c, int cl) {
  int res;
  switch (tolower(cl)) {
    case 'a' : res = isalpha(c); break;
    case 'c' : res = iscntrl(c); break;
    case 'd' : res = isdigit(c); break;
    case 'g' : res = isgraph(c); break;
    case 'l' : res = islower(c); break;
    case 'p' : res = ispunct(c); break;
    case 's' : res = isspace(c); break;
    case 'u' : res = isupper(c); break;
    case 'w' : res = isalnum(c); break;
    case 'x' : res = isxdigit(c); break;
    case 'z' : res = (c == 0); break;  /* deprecated option */
    default: return (cl == c);
  }
  return (islower(cl) ? res : !res);
}


static int luai_matchbracketclass (int c, const char *p, const char *ec) {
  int sig = 1;
  if (*(p+1) == '^') {
    sig = 0;
    p++;  /* skip the '^' */
  }
  while (++p < ec) {
    if (*p == LUAI_L_ESC) {
      p++;
      if (luai_match_class(c, luai_uchar(*p)))
        return sig;
    }
    else if ((*(p+1) == '-') && (p+2 < ec)) {
      p+=2;
      if (luai_uchar(*(p-2)) <= c && c <= luai_uchar(*p))
        return sig;
    }
    else if (luai_uchar(*p) == c) return sig;
  }
  return !sig;
}


static int luai_singlematch (luai_MatchState *ms, const char *s, const char *p,
                        const char *ep) {
  if (s >= ms->src_end)
    return 0;
  else {
    int c = luai_uchar(*s);
    switch (*p) {
      case '.': return 1;  /* matches any char */
      case LUAI_L_ESC: return luai_match_class(c, luai_uchar(*(p+1)));
      case '[': return luai_matchbracketclass(c, p, ep-1);
      default:  return (luai_uchar(*p) == c);
    }
  }
}


static const char *luai_matchbalance (luai_MatchState *ms, const char *s,
                                   const char *p) {
  if (p >= ms->p_end - 1)
    luaL_error(ms->L, "malformed pattern (missing arguments to '%%b')");
  if (*s != *p) return NULL;
  else {
    int b = *p;
    int e = *(p+1);
    int cont = 1;
    while (++s < ms->src_end) {
      if (*s == e) {
        if (--cont == 0) return s+1;
      }
      else if (*s == b) cont++;
    }
  }
  return NULL;  /* string ends out of balance */
}


static const char *luai_max_expand (luai_MatchState *ms, const char *s,
                                 const char *p, const char *ep) {
  ptrdiff_t i = 0;  /* counts maximum expand for item */
  while (luai_singlematch(ms, s + i, p, ep))
    i++;
  /* keeps trying to luai_match with the maximum repetitions */
  while (i>=0) {
    const char *res = luai_match(ms, (s+i), ep+1);
    if (res) return res;
    i--;  /* else didn't luai_match; reduce 1 repetition to try again */
  }
  return NULL;
}


static const char *luai_min_expand (luai_MatchState *ms, const char *s,
                                 const char *p, const char *ep) {
  for (;;) {
    const char *res = luai_match(ms, s, ep+1);
    if (res != NULL)
      return res;
    else if (luai_singlematch(ms, s, p, ep))
      s++;  /* try with one more repetition */
    else return NULL;
  }
}


static const char *luai_start_capture (luai_MatchState *ms, const char *s,
                                    const char *p, int what) {
  const char *res;
  int level = ms->level;
  if (level >= LUA_MAXCAPTURES) luaL_error(ms->L, "too many captures");
  ms->capture[level].init = s;
  ms->capture[level].len = what;
  ms->level = level+1;
  if ((res=luai_match(ms, s, p)) == NULL)  /* luai_match failed? */
    ms->level--;  /* undo capture */
  return res;
}


static const char *luai_end_capture (luai_MatchState *ms, const char *s,
                                  const char *p) {
  int l = luai_capture_to_close(ms);
  const char *res;
  ms->capture[l].len = s - ms->capture[l].init;  /* close capture */
  if ((res = luai_match(ms, s, p)) == NULL)  /* luai_match failed? */
    ms->capture[l].len = LUAI_CAP_UNFINISHED;  /* undo capture */
  return res;
}


static const char *luai_match_capture (luai_MatchState *ms, const char *s, int l) {
  size_t len;
  l = luai_check_capture(ms, l);
  len = ms->capture[l].len;
  if ((size_t)(ms->src_end-s) >= len &&
      memcmp(ms->capture[l].init, s, len) == 0)
    return s+len;
  else return NULL;
}


static const char *luai_match (luai_MatchState *ms, const char *s, const char *p) {
  if (ms->matchdepth-- == 0)
    luaL_error(ms->L, "pattern too complex");
  init: /* using goto's to optimize tail recursion */
  if (p != ms->p_end) {  /* end of pattern? */
    switch (*p) {
      case '(': {  /* start capture */
        if (*(p + 1) == ')')  /* position capture? */
          s = luai_start_capture(ms, s, p + 2, LUAI_CAP_POSITION);
        else
          s = luai_start_capture(ms, s, p + 1, LUAI_CAP_UNFINISHED);
        break;
      }
      case ')': {  /* end capture */
        s = luai_end_capture(ms, s, p + 1);
        break;
      }
      case '$': {
        if ((p + 1) != ms->p_end)  /* is the '$' the last char in pattern? */
          goto dflt;  /* no; go to default */
        s = (s == ms->src_end) ? s : NULL;  /* luai_check end of string */
        break;
      }
      case LUAI_L_ESC: {  /* escaped sequences not in the format class[*+?-]? */
        switch (*(p + 1)) {
          case 'b': {  /* balanced string? */
            s = luai_matchbalance(ms, s, p + 2);
            if (s != NULL) {
              p += 4; goto init;  /* return luai_match(ms, s, p + 4); */
            }  /* else fail (s == NULL) */
            break;
          }
          case 'f': {  /* frontier? */
            const char *ep; char previous;
            p += 2;
            if (*p != '[')
              luaL_error(ms->L, "missing '[' after '%%f' in pattern");
            ep = luai_classend(ms, p);  /* points to what is luai_next */
            previous = (s == ms->src_init) ? '\0' : *(s - 1);
            if (!luai_matchbracketclass(luai_uchar(previous), p, ep - 1) &&
               luai_matchbracketclass(luai_uchar(*s), p, ep - 1)) {
              p = ep; goto init;  /* return luai_match(ms, s, ep); */
            }
            s = NULL;  /* luai_match failed */
            break;
          }
          case '0': case '1': case '2': case '3':
          case '4': case '5': case '6': case '7':
          case '8': case '9': {  /* capture results (%0-%9)? */
            s = luai_match_capture(ms, s, luai_uchar(*(p + 1)));
            if (s != NULL) {
              p += 2; goto init;  /* return luai_match(ms, s, p + 2) */
            }
            break;
          }
          default: goto dflt;
        }
        break;
      }
      default: dflt: {  /* pattern class plus optional suffix */
        const char *ep = luai_classend(ms, p);  /* points to optional suffix */
        /* does not luai_match at least once? */
        if (!luai_singlematch(ms, s, p, ep)) {
          if (*ep == '*' || *ep == '?' || *ep == '-') {  /* accept empty? */
            p = ep + 1; goto init;  /* return luai_match(ms, s, ep + 1); */
          }
          else  /* '+' or no suffix */
            s = NULL;  /* fail */
        }
        else {  /* matched once */
          switch (*ep) {  /* handle optional suffix */
            case '?': {  /* optional */
              const char *res;
              if ((res = luai_match(ms, s + 1, ep + 1)) != NULL)
                s = res;
              else {
                p = ep + 1; goto init;  /* else return luai_match(ms, s, ep + 1); */
              }
              break;
            }
            case '+':  /* 1 or more repetitions */
              s++;  /* 1 luai_match already done */
              /* FALLTHROUGH */
            case '*':  /* 0 or more repetitions */
              s = luai_max_expand(ms, s, p, ep);
              break;
            case '-':  /* 0 or more repetitions (minimum) */
              s = luai_min_expand(ms, s, p, ep);
              break;
            default:  /* no suffix */
              s++; p = ep; goto init;  /* return luai_match(ms, s + 1, ep); */
          }
        }
        break;
      }
    }
  }
  ms->matchdepth++;
  return s;
}



static const char *luai_lmemfind (const char *s1, size_t l1,
                               const char *s2, size_t l2) {
  if (l2 == 0) return s1;  /* empty strings are everywhere */
  else if (l2 > l1) return NULL;  /* avoids a negative 'l1' */
  else {
    const char *init;  /* to search for a '*s2' inside 's1' */
    l2--;  /* 1st char will be checked by 'memchr' */
    l1 = l1-l2;  /* 's2' cannot be found after that */
    while (l1 > 0 && (init = (const char *)memchr(s1, *s2, l1)) != NULL) {
      init++;   /* 1st char is already checked */
      if (memcmp(init, s2+1, l2) == 0)
        return init-1;
      else {  /* correct 'l1' and 's1' to try again */
        l1 -= init-s1;
        s1 = init;
      }
    }
    return NULL;  /* not found */
  }
}


static void luai_push_onecapture (luai_MatchState *ms, int i, const char *s,
                                                    const char *e) {
  if (i >= ms->level) {
    if (i == 0)  /* ms->level == 0, too */
      lua_pushlstring(ms->L, s, e - s);  /* add whole luai_match */
    else
      luaL_error(ms->L, "invalid capture index %%%d", i + 1);
  }
  else {
    ptrdiff_t l = ms->capture[i].len;
    if (l == LUAI_CAP_UNFINISHED) luaL_error(ms->L, "unfinished capture");
    if (l == LUAI_CAP_POSITION)
      lua_pushinteger(ms->L, (ms->capture[i].init - ms->src_init) + 1);
    else
      lua_pushlstring(ms->L, ms->capture[i].init, l);
  }
}


static int luai_push_captures (luai_MatchState *ms, const char *s, const char *e) {
  int i;
  int nlevels = (ms->level == 0 && s) ? 1 : ms->level;
  luaL_checkstack(ms->L, nlevels, "too many captures");
  for (i = 0; i < nlevels; i++)
    luai_push_onecapture(ms, i, s, e);
  return nlevels;  /* number of strings pushed */
}


/* luai_check whether pattern has no special characters */
static int luai_nospecials (const char *p, size_t l) {
  size_t upto = 0;
  do {
    if (strpbrk(p + upto, LUAI_SPECIALS))
      return 0;  /* pattern has a special character */
    upto += strlen(p + upto) + 1;  /* may have more after \0 */
  } while (upto <= l);
  return 1;  /* no special chars found */
}


static void luai_prepstate (luai_MatchState *ms, lua_State *L,
                       const char *s, size_t ls, const char *p, size_t lp) {
  ms->L = L;
  ms->matchdepth = LUAI_MAXCCALLS;
  ms->src_init = s;
  ms->src_end = s + ls;
  ms->p_end = p + lp;
}


static void luai_reprepstate (luai_MatchState *ms) {
  ms->level = 0;
  lua_assert(ms->matchdepth == LUAI_MAXCCALLS);
}


static int luai_str_find_aux (lua_State *L, int find) {
  size_t ls, lp;
  const char *s = luaL_checklstring(L, 1, &ls);
  const char *p = luaL_checklstring(L, 2, &lp);
  lua_Integer init = luai_posrelat(luaL_optinteger(L, 3, 1), ls);
  if (init < 1) init = 1;
  else if (init > (lua_Integer)ls + 1) {  /* start after string's end? */
    lua_pushnil(L);  /* cannot find anything */
    return 1;
  }
  /* explicit request or no special characters? */
  if (find && (lua_toboolean(L, 4) || luai_nospecials(p, lp))) {
    /* do a plain search */
    const char *s2 = luai_lmemfind(s + init - 1, ls - (size_t)init + 1, p, lp);
    if (s2) {
      lua_pushinteger(L, (s2 - s) + 1);
      lua_pushinteger(L, (s2 - s) + lp);
      return 2;
    }
  }
  else {
    luai_MatchState ms;
    const char *s1 = s + init - 1;
    int anchor = (*p == '^');
    if (anchor) {
      p++; lp--;  /* skip anchor character */
    }
    luai_prepstate(&ms, L, s, ls, p, lp);
    do {
      const char *res;
      luai_reprepstate(&ms);
      if ((res=luai_match(&ms, s1, p)) != NULL) {
        if (find) {
          lua_pushinteger(L, (s1 - s) + 1);  /* start */
          lua_pushinteger(L, res - s);   /* end */
          return luai_push_captures(&ms, NULL, 0) + 2;
        }
        else
          return luai_push_captures(&ms, s1, res);
      }
    } while (s1++ < ms.src_end && !anchor);
  }
  lua_pushnil(L);  /* not found */
  return 1;
}


static int luai_str_find (lua_State *L) {
  return luai_str_find_aux(L, 1);
}


static int luai_str_match (lua_State *L) {
  return luai_str_find_aux(L, 0);
}


/* state for 'luai_gmatch' */
typedef struct luai_GMatchState {
  const char *src;  /* current position */
  const char *p;  /* pattern */
  const char *lastmatch;  /* end of last luai_match */
  luai_MatchState ms;  /* luai_match state */
} luai_GMatchState;


static int luai_gmatch_aux (lua_State *L) {
  luai_GMatchState *gm = (luai_GMatchState *)lua_touserdata(L, lua_upvalueindex(3));
  const char *src;
  gm->ms.L = L;
  for (src = gm->src; src <= gm->ms.src_end; src++) {
    const char *e;
    luai_reprepstate(&gm->ms);
    if ((e = luai_match(&gm->ms, src, gm->p)) != NULL && e != gm->lastmatch) {
      gm->src = gm->lastmatch = e;
      return luai_push_captures(&gm->ms, src, e);
    }
  }
  return 0;  /* not found */
}


static int luai_gmatch (lua_State *L) {
  size_t ls, lp;
  const char *s = luaL_checklstring(L, 1, &ls);
  const char *p = luaL_checklstring(L, 2, &lp);
  luai_GMatchState *gm;
  lua_settop(L, 2);  /* keep them on closure to avoid being collected */
  gm = (luai_GMatchState *)lua_newuserdata(L, sizeof(luai_GMatchState));
  luai_prepstate(&gm->ms, L, s, ls, p, lp);
  gm->src = s; gm->p = p; gm->lastmatch = NULL;
  lua_pushcclosure(L, luai_gmatch_aux, 3);
  return 1;
}


static void luai_add_s (luai_MatchState *ms, luaL_Buffer *b, const char *s,
                                                   const char *e) {
  size_t l, i;
  lua_State *L = ms->L;
  const char *news = lua_tolstring(L, 3, &l);
  for (i = 0; i < l; i++) {
    if (news[i] != LUAI_L_ESC)
      luaL_addchar(b, news[i]);
    else {
      i++;  /* skip ESC */
      if (!isdigit(luai_uchar(news[i]))) {
        if (news[i] != LUAI_L_ESC)
          luaL_error(L, "invalid use of '%c' in replacement string", LUAI_L_ESC);
        luaL_addchar(b, news[i]);
      }
      else if (news[i] == '0')
          luaL_addlstring(b, s, e - s);
      else {
        luai_push_onecapture(ms, news[i] - '1', s, e);
        luaL_tolstring(L, -1, NULL);  /* if number, convert it to string */
        lua_remove(L, -2);  /* remove original value */
        luaL_addvalue(b);  /* add capture to accumulated result */
      }
    }
  }
}


static void luai_add_value (luai_MatchState *ms, luaL_Buffer *b, const char *s,
                                       const char *e, int tr) {
  lua_State *L = ms->L;
  switch (tr) {
    case LUA_TFUNCTION: {
      int n;
      lua_pushvalue(L, 3);
      n = luai_push_captures(ms, s, e);
      lua_call(L, n, 1);
      break;
    }
    case LUA_TTABLE: {
      luai_push_onecapture(ms, 0, s, e);
      lua_gettable(L, 3);
      break;
    }
    default: {  /* LUA_TNUMBER or LUA_TSTRING */
      luai_add_s(ms, b, s, e);
      return;
    }
  }
  if (!lua_toboolean(L, -1)) {  /* nil or false? */
    lua_pop(L, 1);
    lua_pushlstring(L, s, e - s);  /* keep original text */
  }
  else if (!lua_isstring(L, -1))
    luaL_error(L, "invalid replacement value (a %s)", luaL_typename(L, -1));
  luaL_addvalue(b);  /* add result to accumulator */
}


static int luai_str_gsub (lua_State *L) {
  size_t srcl, lp;
  const char *src = luaL_checklstring(L, 1, &srcl);  /* subject */
  const char *p = luaL_checklstring(L, 2, &lp);  /* pattern */
  const char *lastmatch = NULL;  /* end of last luai_match */
  int tr = lua_type(L, 3);  /* replacement type */
  lua_Integer max_s = luaL_optinteger(L, 4, srcl + 1);  /* max replacements */
  int anchor = (*p == '^');
  lua_Integer n = 0;  /* replacement count */
  luai_MatchState ms;
  luaL_Buffer b;
  luaL_argcheck(L, tr == LUA_TNUMBER || tr == LUA_TSTRING ||
                   tr == LUA_TFUNCTION || tr == LUA_TTABLE, 3,
                      "string/function/table expected");
  luaL_buffinit(L, &b);
  if (anchor) {
    p++; lp--;  /* skip anchor character */
  }
  luai_prepstate(&ms, L, src, srcl, p, lp);
  while (n < max_s) {
    const char *e;
    luai_reprepstate(&ms);  /* (re)prepare state for new luai_match */
    if ((e = luai_match(&ms, src, p)) != NULL && e != lastmatch) {  /* luai_match? */
      n++;
      luai_add_value(&ms, &b, src, e, tr);  /* add replacement to buffer */
      src = lastmatch = e;
    }
    else if (src < ms.src_end)  /* otherwise, skip one character */
      luaL_addchar(&b, *src++);
    else break;  /* end of subject */
    if (anchor) break;
  }
  luaL_addlstring(&b, src, ms.src_end-src);
  luaL_pushresult(&b);
  lua_pushinteger(L, n);  /* number of substitutions */
  return 2;
}

/* }====================================================== */



/*
** {======================================================
** STRING FORMAT
** =======================================================
*/

#if !defined(lua_number2strx)	/* { */

/*
** Hexadecimal floating-point formatter
*/

#include <math.h>

#define LUAI_SIZELENMOD	(sizeof(LUA_NUMBER_FRMLEN)/sizeof(char))


/*
** Number of bits that goes into the first luai_digit. It can be any value
** between 1 and 4; the following definition tries to align the number
** to nibble boundaries by making what is left after that first luai_digit a
** multiple of 4.
*/
#define LUAI_L_NBFD		((luai_l_mathlim(MANT_DIG) - 1)%4 + 1)


/*
** Add integer part of 'x' to buffer and return new 'x'
*/
static lua_Number luai_adddigit (char *buff, int n, lua_Number x) {
  lua_Number dd = luai_l_mathop(floor)(x);  /* get integer part from 'x' */
  int d = (int)dd;
  buff[n] = (d < 10 ? d + '0' : d - 10 + 'a');  /* add to buffer */
  return x - dd;  /* return what is left */
}


static int luai_num2straux (char *buff, int sz, lua_Number x) {
  /* if 'inf' or 'NaN', format it like '%g' */
  if (x != x || x == (lua_Number)HUGE_VAL || x == -(lua_Number)HUGE_VAL)
    return luai_l_sprintf(buff, sz, LUA_NUMBER_FMT, (LUAI_UACNUMBER)x);
  else if (x == 0) {  /* can be -0... */
    /* create "0" or "-0" followed by exponent */
    return luai_l_sprintf(buff, sz, LUA_NUMBER_FMT "x0p+0", (LUAI_UACNUMBER)x);
  }
  else {
    int e;
    lua_Number m = luai_l_mathop(frexp)(x, &e);  /* 'x' fraction and exponent */
    int n = 0;  /* character count */
    if (m < 0) {  /* is number negative? */
      buff[n++] = '-';  /* add signal */
      m = -m;  /* make it positive */
    }
    buff[n++] = '0'; buff[n++] = 'x';  /* add "0x" */
    m = luai_adddigit(buff, n++, m * (1 << LUAI_L_NBFD));  /* add first luai_digit */
    e -= LUAI_L_NBFD;  /* this luai_digit goes before the radix point */
    if (m > 0) {  /* more digits? */
      buff[n++] = lua_getlocaledecpoint();  /* add radix point */
      do {  /* add as many digits as needed */
        m = luai_adddigit(buff, n++, m * 16);
      } while (m > 0);
    }
    n += luai_l_sprintf(buff + n, sz - n, "p%+d", e);  /* add exponent */
    lua_assert(n < sz);
    return n;
  }
}


static int lua_number2strx (lua_State *L, char *buff, int sz,
                            const char *fmt, lua_Number x) {
  int n = luai_num2straux(buff, sz, x);
  if (fmt[LUAI_SIZELENMOD] == 'A') {
    int i;
    for (i = 0; i < n; i++)
      buff[i] = toupper(luai_uchar(buff[i]));
  }
  else if (fmt[LUAI_SIZELENMOD] != 'a')
    luaL_error(L, "modifiers for format '%%a'/'%%A' not implemented");
  return n;
}

#endif				/* } */


/*
** Maximum size of each formatted item. This maximum size is produced
** by format('%.99f', -maxfloat), and is equal to 99 + 3 ('-', '.',
** and '\0') + number of decimal digits to represent maxfloat (which
** is maximum exponent + 1). (99+3+1 then rounded to 120 for "extra
** expenses", such as locale-dependent stuff)
*/
#define LUA_MAX_ITEM        (120 + luai_l_mathlim(MAX_10_EXP))


/* valid flags in a format specification */
#define LUA_FLAGS	"-+ #0"

/*
** maximum size of each format specification (such as "%-099.99d")
*/
#define LUA_MAX_FORMAT	32


static void luai_addquoted (luaL_Buffer *b, const char *s, size_t len) {
  luaL_addchar(b, '"');
  while (len--) {
    if (*s == '"' || *s == '\\' || *s == '\n') {
      luaL_addchar(b, '\\');
      luaL_addchar(b, *s);
    }
    else if (iscntrl(luai_uchar(*s))) {
      char buff[10];
      if (!isdigit(luai_uchar(*(s+1))))
        luai_l_sprintf(buff, sizeof(buff), "\\%d", (int)luai_uchar(*s));
      else
        luai_l_sprintf(buff, sizeof(buff), "\\%03d", (int)luai_uchar(*s));
      luaL_addstring(b, buff);
    }
    else
      luaL_addchar(b, *s);
    s++;
  }
  luaL_addchar(b, '"');
}


/*
** Ensures the 'buff' string uses a dot as the radix character.
*/
static void luai_checkdp (char *buff, int nb) {
  if (memchr(buff, '.', nb) == NULL) {  /* no dot? */
    char point = lua_getlocaledecpoint();  /* try locale point */
    char *ppoint = (char *)memchr(buff, point, nb);
    if (ppoint) *ppoint = '.';  /* change it to a dot */
  }
}


static void luai_addliteral (lua_State *L, luaL_Buffer *b, int arg) {
  switch (lua_type(L, arg)) {
    case LUA_TSTRING: {
      size_t len;
      const char *s = lua_tolstring(L, arg, &len);
      luai_addquoted(b, s, len);
      break;
    }
    case LUA_TNUMBER: {
      char *buff = luaL_prepbuffsize(b, LUA_MAX_ITEM);
      int nb;
      if (!lua_isinteger(L, arg)) {  /* float? */
        lua_Number n = lua_tonumber(L, arg);  /* write as hexa ('%a') */
        nb = lua_number2strx(L, buff, LUA_MAX_ITEM, "%" LUA_NUMBER_FRMLEN "a", n);
        luai_checkdp(buff, nb);  /* ensure it uses a dot */
      }
      else {  /* integers */
        lua_Integer n = lua_tointeger(L, arg);
        const char *format = (n == LUA_MININTEGER)  /* corner case? */
                           ? "0x%" LUA_INTEGER_FRMLEN "x"  /* use hexa */
                           : LUA_INTEGER_FMT;  /* else use default format */
        nb = luai_l_sprintf(buff, LUA_MAX_ITEM, format, (LUAI_UACINT)n);
      }
      luaL_addsize(b, nb);
      break;
    }
    case LUA_TNIL: case LUA_TBOOLEAN: {
      luaL_tolstring(L, arg, NULL);
      luaL_addvalue(b);
      break;
    }
    default: {
      luaL_argerror(L, arg, "value has no literal form");
    }
  }
}


static const char *luai_scanformat (lua_State *L, const char *strfrmt, char *form) {
  const char *p = strfrmt;
  while (*p != '\0' && strchr(LUA_FLAGS, *p) != NULL) p++;  /* skip flags */
  if ((size_t)(p - strfrmt) >= sizeof(LUA_FLAGS)/sizeof(char))
    luaL_error(L, "invalid format (repeated flags)");
  if (isdigit(luai_uchar(*p))) p++;  /* skip width */
  if (isdigit(luai_uchar(*p))) p++;  /* (2 digits at most) */
  if (*p == '.') {
    p++;
    if (isdigit(luai_uchar(*p))) p++;  /* skip precision */
    if (isdigit(luai_uchar(*p))) p++;  /* (2 digits at most) */
  }
  if (isdigit(luai_uchar(*p)))
    luaL_error(L, "invalid format (width or precision too long)");
  *(form++) = '%';
  memcpy(form, strfrmt, ((p - strfrmt) + 1) * sizeof(char));
  form += (p - strfrmt) + 1;
  *form = '\0';
  return p;
}


/*
** add length modifier into formats
*/
static void luai_addlenmod (char *form, const char *lenmod) {
  size_t l = strlen(form);
  size_t lm = strlen(lenmod);
  char spec = form[l - 1];
  strcpy(form + l - 1, lenmod);
  form[l + lm - 1] = spec;
  form[l + lm] = '\0';
}


static int luai_str_format (lua_State *L) {
  int top = lua_gettop(L);
  int arg = 1;
  size_t sfl;
  const char *strfrmt = luaL_checklstring(L, arg, &sfl);
  const char *strfrmt_end = strfrmt+sfl;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  while (strfrmt < strfrmt_end) {
    if (*strfrmt != LUAI_L_ESC)
      luaL_addchar(&b, *strfrmt++);
    else if (*++strfrmt == LUAI_L_ESC)
      luaL_addchar(&b, *strfrmt++);  /* %% */
    else { /* format item */
      char form[LUA_MAX_FORMAT];  /* to store the format ('%...') */
      char *buff = luaL_prepbuffsize(&b, LUA_MAX_ITEM);  /* to put formatted item */
      int nb = 0;  /* number of bytes in added item */
      if (++arg > top)
        luaL_argerror(L, arg, "no value");
      strfrmt = luai_scanformat(L, strfrmt, form);
      switch (*strfrmt++) {
        case 'c': {
          nb = luai_l_sprintf(buff, LUA_MAX_ITEM, form, (int)luaL_checkinteger(L, arg));
          break;
        }
        case 'd': case 'i':
        case 'o': case 'u': case 'x': case 'X': {
          lua_Integer n = luaL_checkinteger(L, arg);
          luai_addlenmod(form, LUA_INTEGER_FRMLEN);
          nb = luai_l_sprintf(buff, LUA_MAX_ITEM, form, (LUAI_UACINT)n);
          break;
        }
        case 'a': case 'A':
          luai_addlenmod(form, LUA_NUMBER_FRMLEN);
          nb = lua_number2strx(L, buff, LUA_MAX_ITEM, form,
                                  luaL_checknumber(L, arg));
          break;
        case 'e': case 'E': case 'f':
        case 'g': case 'G': {
          lua_Number n = luaL_checknumber(L, arg);
          luai_addlenmod(form, LUA_NUMBER_FRMLEN);
          nb = luai_l_sprintf(buff, LUA_MAX_ITEM, form, (LUAI_UACNUMBER)n);
          break;
        }
        case 'q': {
          luai_addliteral(L, &b, arg);
          break;
        }
        case 's': {
          size_t l;
          const char *s = luaL_tolstring(L, arg, &l);
          if (form[2] == '\0')  /* no modifiers? */
            luaL_addvalue(&b);  /* keep entire string */
          else {
            luaL_argcheck(L, l == strlen(s), arg, "string contains zeros");
            if (!strchr(form, '.') && l >= 100) {
              /* no precision and string is too long to be formatted */
              luaL_addvalue(&b);  /* keep entire string */
            }
            else {  /* format the string into 'buff' */
              nb = luai_l_sprintf(buff, LUA_MAX_ITEM, form, s);
              lua_pop(L, 1);  /* remove result from 'luaL_tolstring' */
            }
          }
          break;
        }
        default: {  /* also treat cases 'pnLlh' */
          return luaL_error(L, "invalid option '%%%c' to 'format'",
                               *(strfrmt - 1));
        }
      }
      lua_assert(nb < LUA_MAX_ITEM);
      luaL_addsize(&b, nb);
    }
  }
  luaL_pushresult(&b);
  return 1;
}

/* }====================================================== */


/*
** {======================================================
** PACK/UNPACK
** =======================================================
*/


/* value used for padding */
#if !defined(LUAL_PACKPADBYTE)
#define LUAL_PACKPADBYTE		0x00
#endif

/* maximum size for the binary representation of an integer */
#define LUA_MAXINTSIZE	16

/* number of bits in a character */
#define LUA_NB	CHAR_BIT

/* mask for one character (LUA_NB 1's) */
#define LUA_MC	((1 << LUA_NB) - 1)

/* size of a lua_Integer */
#define LUA_SZINT	((int)sizeof(lua_Integer))


/* dummy union to get native endianness */
static const union {
  int dummy;
  char little;  /* true iff machine is little endian */
} luai_nativeendian = {1};


/* dummy structure to get native alignment requirements */
struct luai_cD {
  char c;
  union { double d; void *p; lua_Integer i; lua_Number n; } u;
};

#define LUAI_MAXALIGN	(offsetof(struct luai_cD, u))


/*
** Union for serializing floats
*/
typedef union luai_Ftypes {
  float f;
  double d;
  lua_Number n;
  char buff[5 * sizeof(lua_Number)];  /* enough for any float type */
} luai_Ftypes;


/*
** information to luai_pack/luai_unpack stuff
*/
typedef struct luai_Header {
  lua_State *L;
  int islittle;
  int maxalign;
} luai_Header;


/*
** options for luai_pack/luai_unpack
*/
typedef enum luai_KOption {
  luai_Kint,		/* signed integers */
  luai_Kuint,	/* unsigned integers */
  luai_Kfloat,	/* floating-point numbers */
  luai_Kchar,	/* fixed-length strings */
  luai_Kstring,	/* strings with prefixed length */
  luai_Kzstr,	/* zero-terminated strings */
  luai_Kpadding,	/* padding */
  luai_Kpaddalign,	/* padding for alignment */
  luai_Knop		/* no-op (configuration or spaces) */
} luai_KOption;


/*
** Read an integer numeral from string 'fmt' or return 'df' if
** there is no numeral
*/
static int luai_digit (int c) { return '0' <= c && c <= '9'; }

static int luai_getnum (const char **fmt, int df) {
  if (!luai_digit(**fmt))  /* no number? */
    return df;  /* return default value */
  else {
    int a = 0;
    do {
      a = a*10 + (*((*fmt)++) - '0');
    } while (luai_digit(**fmt) && a <= ((int)LUAI_MAXSIZE - 9)/10);
    return a;
  }
}


/*
** Read an integer numeral and raises an error if it is larger
** than the maximum size for integers.
*/
static int luai_getnumlimit (luai_Header *h, const char **fmt, int df) {
  int sz = luai_getnum(fmt, df);
  if (sz > LUA_MAXINTSIZE || sz <= 0)
    luaL_error(h->L, "integral size (%d) out of limits [1,%d]",
                     sz, LUA_MAXINTSIZE);
  return sz;
}


/*
** Initialize luai_Header
*/
static void luai_initheader (lua_State *L, luai_Header *h) {
  h->L = L;
  h->islittle = luai_nativeendian.little;
  h->maxalign = 1;
}


/*
** Read and classify luai_next option. 'size' is filled with option's size.
*/
static luai_KOption luai_getoption (luai_Header *h, const char **fmt, int *size) {
  int opt = *((*fmt)++);
  *size = 0;  /* default */
  switch (opt) {
    case 'b': *size = sizeof(char); return luai_Kint;
    case 'B': *size = sizeof(char); return luai_Kuint;
    case 'h': *size = sizeof(short); return luai_Kint;
    case 'H': *size = sizeof(short); return luai_Kuint;
    case 'l': *size = sizeof(long); return luai_Kint;
    case 'L': *size = sizeof(long); return luai_Kuint;
    case 'j': *size = sizeof(lua_Integer); return luai_Kint;
    case 'J': *size = sizeof(lua_Integer); return luai_Kuint;
    case 'T': *size = sizeof(size_t); return luai_Kuint;
    case 'f': *size = sizeof(float); return luai_Kfloat;
    case 'd': *size = sizeof(double); return luai_Kfloat;
    case 'n': *size = sizeof(lua_Number); return luai_Kfloat;
    case 'i': *size = luai_getnumlimit(h, fmt, sizeof(int)); return luai_Kint;
    case 'I': *size = luai_getnumlimit(h, fmt, sizeof(int)); return luai_Kuint;
    case 's': *size = luai_getnumlimit(h, fmt, sizeof(size_t)); return luai_Kstring;
    case 'c':
      *size = luai_getnum(fmt, -1);
      if (*size == -1)
        luaL_error(h->L, "missing size for format option 'c'");
      return luai_Kchar;
    case 'z': return luai_Kzstr;
    case 'x': *size = 1; return luai_Kpadding;
    case 'X': return luai_Kpaddalign;
    case ' ': break;
    case '<': h->islittle = 1; break;
    case '>': h->islittle = 0; break;
    case '=': h->islittle = luai_nativeendian.little; break;
    case '!': h->maxalign = luai_getnumlimit(h, fmt, LUAI_MAXALIGN); break;
    default: luaL_error(h->L, "invalid format option '%c'", opt);
  }
  return luai_Knop;
}


/*
** Read, classify, and fill other details about the luai_next option.
** 'psize' is filled with option's size, 'notoalign' with its
** alignment requirements.
** Local variable 'size' gets the size to be aligned. (Kpadal option
** always gets its full alignment, other options are limited by
** the maximum alignment ('maxalign'). luai_Kchar option needs no alignment
** despite its size.
*/
static luai_KOption luai_getdetails (luai_Header *h, size_t totalsize,
                           const char **fmt, int *psize, int *ntoalign) {
  luai_KOption opt = luai_getoption(h, fmt, psize);
  int align = *psize;  /* usually, alignment follows size */
  if (opt == luai_Kpaddalign) {  /* 'X' gets alignment from following option */
    if (**fmt == '\0' || luai_getoption(h, fmt, &align) == luai_Kchar || align == 0)
      luaL_argerror(h->L, 1, "invalid luai_next option for option 'X'");
  }
  if (align <= 1 || opt == luai_Kchar)  /* need no alignment? */
    *ntoalign = 0;
  else {
    if (align > h->maxalign)  /* enforce maximum alignment */
      align = h->maxalign;
    if ((align & (align - 1)) != 0)  /* is 'align' not a power of 2? */
      luaL_argerror(h->L, 1, "format asks for alignment not power of 2");
    *ntoalign = (align - (int)(totalsize & (align - 1))) & (align - 1);
  }
  return opt;
}


/*
** Pack integer 'n' with 'size' bytes and 'islittle' endianness.
** The final 'if' handles the case when 'size' is larger than
** the size of a Lua integer, correcting the extra sign-extension
** bytes if necessary (by default they would be zeros).
*/
static void luai_packint (luaL_Buffer *b, lua_Unsigned n,
                     int islittle, int size, int neg) {
  char *buff = luaL_prepbuffsize(b, size);
  int i;
  buff[islittle ? 0 : size - 1] = (char)(n & LUA_MC);  /* first byte */
  for (i = 1; i < size; i++) {
    n >>= LUA_NB;
    buff[islittle ? i : size - 1 - i] = (char)(n & LUA_MC);
  }
  if (neg && size > LUA_SZINT) {  /* negative number need sign extension? */
    for (i = LUA_SZINT; i < size; i++)  /* correct extra bytes */
      buff[islittle ? i : size - 1 - i] = (char)LUA_MC;
  }
  luaL_addsize(b, size);  /* add result to buffer */
}


/*
** Copy 'size' bytes from 'src' to 'dest', correcting endianness if
** given 'islittle' is different from native endianness.
*/
static void luai_copywithendian (volatile char *dest, volatile const char *src,
                            int size, int islittle) {
  if (islittle == luai_nativeendian.little) {
    while (size-- != 0)
      *(dest++) = *(src++);
  }
  else {
    dest += size - 1;
    while (size-- != 0)
      *(dest--) = *(src++);
  }
}


static int luai_str_pack (lua_State *L) {
  luaL_Buffer b;
  luai_Header h;
  const char *fmt = luaL_checkstring(L, 1);  /* format string */
  int arg = 1;  /* current argument to luai_pack */
  size_t totalsize = 0;  /* accumulate total size of result */
  luai_initheader(L, &h);
  lua_pushnil(L);  /* mark to separate arguments from string buffer */
  luaL_buffinit(L, &b);
  while (*fmt != '\0') {
    int size, ntoalign;
    luai_KOption opt = luai_getdetails(&h, totalsize, &fmt, &size, &ntoalign);
    totalsize += ntoalign + size;
    while (ntoalign-- > 0)
     luaL_addchar(&b, LUAL_PACKPADBYTE);  /* fill alignment */
    arg++;
    switch (opt) {
      case luai_Kint: {  /* signed integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        if (size < LUA_SZINT) {  /* need overflow luai_check? */
          lua_Integer lim = (lua_Integer)1 << ((size * LUA_NB) - 1);
          luaL_argcheck(L, -lim <= n && n < lim, arg, "integer overflow");
        }
        luai_packint(&b, (lua_Unsigned)n, h.islittle, size, (n < 0));
        break;
      }
      case luai_Kuint: {  /* unsigned integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        if (size < LUA_SZINT)  /* need overflow luai_check? */
          luaL_argcheck(L, (lua_Unsigned)n < ((lua_Unsigned)1 << (size * LUA_NB)),
                           arg, "unsigned overflow");
        luai_packint(&b, (lua_Unsigned)n, h.islittle, size, 0);
        break;
      }
      case luai_Kfloat: {  /* floating-point options */
        volatile luai_Ftypes u;
        char *buff = luaL_prepbuffsize(&b, size);
        lua_Number n = luaL_checknumber(L, arg);  /* get argument */
        if (size == sizeof(u.f)) u.f = (float)n;  /* copy it into 'u' */
        else if (size == sizeof(u.d)) u.d = (double)n;
        else u.n = n;
        /* move 'u' to final result, correcting endianness if needed */
        luai_copywithendian(buff, u.buff, size, h.islittle);
        luaL_addsize(&b, size);
        break;
      }
      case luai_Kchar: {  /* fixed-size string */
        size_t len;
        const char *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, len <= (size_t)size, arg,
                         "string longer than given size");
        luaL_addlstring(&b, s, len);  /* add string */
        while (len++ < (size_t)size)  /* pad extra space */
          luaL_addchar(&b, LUAL_PACKPADBYTE);
        break;
      }
      case luai_Kstring: {  /* strings with length count */
        size_t len;
        const char *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, size >= (int)sizeof(size_t) ||
                         len < ((size_t)1 << (size * LUA_NB)),
                         arg, "string length does not fit in given size");
        luai_packint(&b, (lua_Unsigned)len, h.islittle, size, 0);  /* luai_pack length */
        luaL_addlstring(&b, s, len);
        totalsize += len;
        break;
      }
      case luai_Kzstr: {  /* zero-terminated string */
        size_t len;
        const char *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, strlen(s) == len, arg, "string contains zeros");
        luaL_addlstring(&b, s, len);
        luaL_addchar(&b, '\0');  /* add zero at the end */
        totalsize += len + 1;
        break;
      }
      case luai_Kpadding: luaL_addchar(&b, LUAL_PACKPADBYTE);  /* FALLTHROUGH */
      case luai_Kpaddalign: case luai_Knop:
        arg--;  /* undo increment */
        break;
    }
  }
  luaL_pushresult(&b);
  return 1;
}


static int luai_str_packsize (lua_State *L) {
  luai_Header h;
  const char *fmt = luaL_checkstring(L, 1);  /* format string */
  size_t totalsize = 0;  /* accumulate total size of result */
  luai_initheader(L, &h);
  while (*fmt != '\0') {
    int size, ntoalign;
    luai_KOption opt = luai_getdetails(&h, totalsize, &fmt, &size, &ntoalign);
    size += ntoalign;  /* total space used by option */
    luaL_argcheck(L, totalsize <= LUAI_MAXSIZE - size, 1,
                     "format result too large");
    totalsize += size;
    switch (opt) {
      case luai_Kstring:  /* strings with length count */
      case luai_Kzstr:    /* zero-terminated string */
        luaL_argerror(L, 1, "variable-length format");
        /* call never return, but to avoid warnings: *//* FALLTHROUGH */
      default:  break;
    }
  }
  lua_pushinteger(L, (lua_Integer)totalsize);
  return 1;
}


/*
** Unpack an integer with 'size' bytes and 'islittle' endianness.
** If size is smaller than the size of a Lua integer and integer
** is signed, must do sign extension (propagating the sign to the
** higher bits); if size is larger than the size of a Lua integer,
** it must luai_check the unread bytes to see whether they do not cause an
** overflow.
*/
static lua_Integer luai_unpackint (lua_State *L, const char *str,
                              int islittle, int size, int issigned) {
  lua_Unsigned res = 0;
  int i;
  int limit = (size  <= LUA_SZINT) ? size : LUA_SZINT;
  for (i = limit - 1; i >= 0; i--) {
    res <<= LUA_NB;
    res |= (lua_Unsigned)(unsigned char)str[islittle ? i : size - 1 - i];
  }
  if (size < LUA_SZINT) {  /* real size smaller than lua_Integer? */
    if (issigned) {  /* needs sign extension? */
      lua_Unsigned mask = (lua_Unsigned)1 << (size*LUA_NB - 1);
      res = ((res ^ mask) - mask);  /* do sign extension */
    }
  }
  else if (size > LUA_SZINT) {  /* must luai_check unread bytes */
    int mask = (!issigned || (lua_Integer)res >= 0) ? 0 : LUA_MC;
    for (i = limit; i < size; i++) {
      if ((unsigned char)str[islittle ? i : size - 1 - i] != mask)
        luaL_error(L, "%d-byte integer does not fit into Lua Integer", size);
    }
  }
  return (lua_Integer)res;
}


static int luai_str_unpack (lua_State *L) {
  luai_Header h;
  const char *fmt = luaL_checkstring(L, 1);
  size_t ld;
  const char *data = luaL_checklstring(L, 2, &ld);
  size_t pos = (size_t)luai_posrelat(luaL_optinteger(L, 3, 1), ld) - 1;
  int n = 0;  /* number of results */
  luaL_argcheck(L, pos <= ld, 3, "initial position out of string");
  luai_initheader(L, &h);
  while (*fmt != '\0') {
    int size, ntoalign;
    luai_KOption opt = luai_getdetails(&h, pos, &fmt, &size, &ntoalign);
    if ((size_t)ntoalign + size > ~pos || pos + ntoalign + size > ld)
      luaL_argerror(L, 2, "data string too short");
    pos += ntoalign;  /* skip alignment */
    /* stack space for item + luai_next position */
    luaL_checkstack(L, 2, "too many results");
    n++;
    switch (opt) {
      case luai_Kint:
      case luai_Kuint: {
        lua_Integer res = luai_unpackint(L, data + pos, h.islittle, size,
                                       (opt == luai_Kint));
        lua_pushinteger(L, res);
        break;
      }
      case luai_Kfloat: {
        volatile luai_Ftypes u;
        lua_Number num;
        luai_copywithendian(u.buff, data + pos, size, h.islittle);
        if (size == sizeof(u.f)) num = (lua_Number)u.f;
        else if (size == sizeof(u.d)) num = (lua_Number)u.d;
        else num = u.n;
        lua_pushnumber(L, num);
        break;
      }
      case luai_Kchar: {
        lua_pushlstring(L, data + pos, size);
        break;
      }
      case luai_Kstring: {
        size_t len = (size_t)luai_unpackint(L, data + pos, h.islittle, size, 0);
        luaL_argcheck(L, pos + len + size <= ld, 2, "data string too short");
        lua_pushlstring(L, data + pos + size, len);
        pos += len;  /* skip string */
        break;
      }
      case luai_Kzstr: {
        size_t len = (int)strlen(data + pos);
        lua_pushlstring(L, data + pos, len);
        pos += len + 1;  /* skip string plus final '\0' */
        break;
      }
      case luai_Kpaddalign: case luai_Kpadding: case luai_Knop:
        n--;  /* undo increment */
        break;
    }
    pos += size;
  }
  lua_pushinteger(L, pos + 1);  /* luai_next position */
  return n + 1;
}

/* }====================================================== */


static const luaL_Reg luai_strlib[] = {
  {"byte", luai_str_byte},
  {"char", luai_str_char},
  {"dump", luai_str_dump},
  {"find", luai_str_find},
  {"format", luai_str_format},
  {"gmatch", luai_gmatch},
  {"gsub", luai_str_gsub},
  {"len", luai_str_len},
  {"lower", luai_str_lower},
  {"match", luai_str_match},
  {"rep", luai_str_rep},
  {"reverse", luai_str_reverse},
  {"sub", luai_str_sub},
  {"upper", luai_str_upper},
  {"pack", luai_str_pack},
  {"packsize", luai_str_packsize},
  {"unpack", luai_str_unpack},
  {NULL, NULL}
};


static void luai_createmetatable (lua_State *L) {
  lua_createtable(L, 0, 1);  /* table to be metatable for strings */
  lua_pushliteral(L, "");  /* dummy string */
  lua_pushvalue(L, -2);  /* copy table */
  lua_setmetatable(L, -2);  /* set table as metatable for strings */
  lua_pop(L, 1);  /* pop dummy string */
  lua_pushvalue(L, -2);  /* get string library */
  lua_setfield(L, -2, "__index");  /* metatable.__index = string */
  lua_pop(L, 1);  /* pop metatable */
}


/*
** Open string library
*/
LUAMOD_API int luaopen_string (lua_State *L) {
  luaL_newlib(L, luai_strlib);
  luai_createmetatable(L);
  return 1;
}

/*__ltable.c__*/

/*
** Maximum size of array part (LUAI_MAXASIZE) is 2^LUAI_MAXABITS. LUAI_MAXABITS is
** the largest integer such that LUAI_MAXASIZE fits in an unsigned int.
*/
#define LUAI_MAXABITS	luai_cast_int(sizeof(int) * CHAR_BIT - 1)
#define LUAI_MAXASIZE	(1u << LUAI_MAXABITS)

/*
** Maximum size of hash part is 2^LUAI_MAXHBITS. LUAI_MAXHBITS is the largest
** integer such that 2^LUAI_MAXHBITS fits in a signed int. (Note that the
** maximum number of elements in a table, 2^LUAI_MAXABITS + 2^LUAI_MAXHBITS, still
** fits comfortably in an unsigned int.)
*/
#define LUAI_MAXHBITS	(LUAI_MAXABITS - 1)


#define luai_hashpow2(t,n)		(luai_gnode(t, luai_lmod((n), luai_sizenode(t))))

#define luai_hashstr(t,str)		luai_hashpow2(t, (str)->hash)
#define luai_hashboolean(t,p)	luai_hashpow2(t, p)
#define luai_hashint(t,i)		luai_hashpow2(t, i)


/*
** for some types, it is better to avoid modulus by power of 2, as
** they tend to have many 2 factors.
*/
#define luai_hashmod(t,n)	(luai_gnode(t, ((n) % ((luai_sizenode(t)-1)|1))))


#define luai_hashpointer(t,p)	luai_hashmod(t, luai_point2uint(p))


#define luai_dummynode		(&luai_dummynode_)

static const luai_Node luai_dummynode_ = {
  {LUAI_NILCONSTANT},  /* value */
  {{LUAI_NILCONSTANT, 0}}  /* key */
};


/*
** Hash for floating-point numbers.
** The main computation should be just
**     n = frexp(n, &i); return (n * INT_MAX) + i
** but there are some numerical subtleties.
** In a two-complement representation, INT_MAX does not has an exact
** representation as a float, but INT_MIN does; because the absolute
** value of 'frexp' is smaller than 1 (unless 'n' is inf/NaN), the
** absolute value of the product 'frexp * -INT_MIN' is smaller or equal
** to INT_MAX. Next, the use of 'unsigned int' avoids overflows when
** adding 'i'; the use of '~u' (instead of '-u') avoids problems with
** INT_MIN.
*/
#if !defined(luai_l_hashfloat)
static int luai_l_hashfloat (lua_Number n) {
  int i;
  lua_Integer ni;
  n = luai_l_mathop(frexp)(n, &i) * -luai_cast_num(INT_MIN);
  if (!lua_numbertointeger(n, &ni)) {  /* is 'n' inf/-inf/NaN? */
    lua_assert(luai_numisnan(n) || luai_l_mathop(fabs)(n) == luai_cast_num(HUGE_VAL));
    return 0;
  }
  else {  /* normal case */
    unsigned int u = luai_cast(unsigned int, i) + luai_cast(unsigned int, ni);
    return luai_cast_int(u <= luai_cast(unsigned int, INT_MAX) ? u : ~u);
  }
}
#endif


/*
** returns the 'main' position of an element in a table (that is, the index
** of its hash value)
*/
static luai_Node *luai_mainposition (const luai_Table *t, const luai_TValue *key) {
  switch (luai_ttype(key)) {
    case LUA_TNUMINT:
      return luai_hashint(t, luai_ivalue(key));
    case LUA_TNUMFLT:
      return luai_hashmod(t, luai_l_hashfloat(luai_fltvalue(key)));
    case LUA_TSHRSTR:
      return luai_hashstr(t, luai_tsvalue(key));
    case LUA_TLNGSTR:
      return luai_hashpow2(t, luaS_hashlongstr(luai_tsvalue(key)));
    case LUA_TBOOLEAN:
      return luai_hashboolean(t, luai_bvalue(key));
    case LUA_TLIGHTUSERDATA:
      return luai_hashpointer(t, luai_pvalue(key));
    case LUA_TLCF:
      return luai_hashpointer(t, luai_fvalue(key));
    default:
      lua_assert(!luai_ttisdeadkey(key));
      return luai_hashpointer(t, luai_gcvalue(key));
  }
}


/*
** returns the index for 'key' if 'key' is an appropriate key to live in
** the array part of the table, 0 otherwise.
*/
static unsigned int luai_arrayindex (const luai_TValue *key) {
  if (luai_ttisinteger(key)) {
    lua_Integer k = luai_ivalue(key);
    if (0 < k && (lua_Unsigned)k <= LUAI_MAXASIZE)
      return luai_cast(unsigned int, k);  /* 'key' is an appropriate array index */
  }
  return 0;  /* 'key' did not luai_match some condition */
}


/*
** returns the index of a 'key' for table traversals. First goes all
** elements in the array part, then elements in the hash part. The
** beginning of a traversal is signaled by 0.
*/
static unsigned int luai_findindex (lua_State *L, luai_Table *t, luai_StkId key) {
  unsigned int i;
  if (luai_ttisnil(key)) return 0;  /* first iteration */
  i = luai_arrayindex(key);
  if (i != 0 && i <= t->sizearray)  /* is 'key' inside array part? */
    return i;  /* yes; that's the index */
  else {
    int nx;
    luai_Node *n = luai_mainposition(t, key);
    for (;;) {  /* luai_check whether 'key' is somewhere in the chain */
      /* key may be dead already, but it is ok to use it in 'luai_next' */
      if (luaV_rawequalobj(luai_gkey(n), key) ||
            (luai_ttisdeadkey(luai_gkey(n)) && luai_iscollectable(key) &&
             luai_deadvalue(luai_gkey(n)) == luai_gcvalue(key))) {
        i = luai_cast_int(n - luai_gnode(t, 0));  /* key index in hash table */
        /* hash elements are numbered after array ones */
        return (i + 1) + t->sizearray;
      }
      nx = luai_gnext(n);
      if (nx == 0)
        luaG_runerror(L, "invalid key to 'luai_next'");  /* key not found */
      else n += nx;
    }
  }
}


int luaH_next (lua_State *L, luai_Table *t, luai_StkId key) {
  unsigned int i = luai_findindex(L, t, key);  /* find original element */
  for (; i < t->sizearray; i++) {  /* try first array part */
    if (!luai_ttisnil(&t->array[i])) {  /* a non-nil value? */
      luai_setivalue(key, i + 1);
      luai_setobj2s(L, key+1, &t->array[i]);
      return 1;
    }
  }
  for (i -= t->sizearray; luai_cast_int(i) < luai_sizenode(t); i++) {  /* hash part */
    if (!luai_ttisnil(luai_gval(luai_gnode(t, i)))) {  /* a non-nil value? */
      luai_setobj2s(L, key, luai_gkey(luai_gnode(t, i)));
      luai_setobj2s(L, key+1, luai_gval(luai_gnode(t, i)));
      return 1;
    }
  }
  return 0;  /* no more elements */
}


/*
** {=============================================================
** Rehash
** ==============================================================
*/

/*
** Compute the optimal size for the array part of table 't'. 'nums' is a
** "count array" where 'nums[i]' is the number of integers in the table
** between 2^(i - 1) + 1 and 2^i. 'pna' enters with the total number of
** integer keys in the table and leaves with the number of keys that
** will go to the array part; return the optimal size.
*/
static unsigned int luai_computesizes (unsigned int nums[], unsigned int *pna) {
  int i;
  unsigned int twotoi;  /* 2^i (candidate for optimal size) */
  unsigned int a = 0;  /* number of elements smaller than 2^i */
  unsigned int na = 0;  /* number of elements to go to array part */
  unsigned int optimal = 0;  /* optimal size for array part */
  /* loop while keys can fill more than half of total size */
  for (i = 0, twotoi = 1; *pna > twotoi / 2; i++, twotoi *= 2) {
    if (nums[i] > 0) {
      a += nums[i];
      if (a > twotoi/2) {  /* more than half elements present? */
        optimal = twotoi;  /* optimal size (till now) */
        na = a;  /* all elements up to 'optimal' will go to array part */
      }
    }
  }
  lua_assert((optimal == 0 || optimal / 2 < na) && na <= optimal);
  *pna = na;
  return optimal;
}


static int luai_countint (const luai_TValue *key, unsigned int *nums) {
  unsigned int k = luai_arrayindex(key);
  if (k != 0) {  /* is 'key' an appropriate array index? */
    nums[luaO_ceillog2(k)]++;  /* count as such */
    return 1;
  }
  else
    return 0;
}


/*
** Count keys in array part of table 't': Fill 'nums[i]' with
** number of keys that will go into corresponding slice and return
** total number of non-nil keys.
*/
static unsigned int luai_numusearray (const luai_Table *t, unsigned int *nums) {
  int lg;
  unsigned int ttlg;  /* 2^lg */
  unsigned int ause = 0;  /* summation of 'nums' */
  unsigned int i = 1;  /* count to traverse all array keys */
  /* traverse each slice */
  for (lg = 0, ttlg = 1; lg <= LUAI_MAXABITS; lg++, ttlg *= 2) {
    unsigned int lc = 0;  /* counter */
    unsigned int lim = ttlg;
    if (lim > t->sizearray) {
      lim = t->sizearray;  /* adjust upper limit */
      if (i > lim)
        break;  /* no more elements to count */
    }
    /* count elements in range (2^(lg - 1), 2^lg] */
    for (; i <= lim; i++) {
      if (!luai_ttisnil(&t->array[i-1]))
        lc++;
    }
    nums[lg] += lc;
    ause += lc;
  }
  return ause;
}


static int luai_numusehash (const luai_Table *t, unsigned int *nums, unsigned int *pna) {
  int totaluse = 0;  /* total number of elements */
  int ause = 0;  /* elements added to 'nums' (can go to array part) */
  int i = luai_sizenode(t);
  while (i--) {
    luai_Node *n = &t->node[i];
    if (!luai_ttisnil(luai_gval(n))) {
      ause += luai_countint(luai_gkey(n), nums);
      totaluse++;
    }
  }
  *pna += ause;
  return totaluse;
}


static void luai_setarrayvector (lua_State *L, luai_Table *t, unsigned int size) {
  unsigned int i;
  luaM_reallocvector(L, t->array, t->sizearray, size, luai_TValue);
  for (i=t->sizearray; i<size; i++)
     luai_setnilvalue(&t->array[i]);
  t->sizearray = size;
}


static void luai_setnodevector (lua_State *L, luai_Table *t, unsigned int size) {
  if (size == 0) {  /* no elements to hash part? */
    t->node = luai_cast(luai_Node *, luai_dummynode);  /* use common 'luai_dummynode' */
    t->lsizenode = 0;
    t->lastfree = NULL;  /* signal that it is using dummy node */
  }
  else {
    int i;
    int lsize = luaO_ceillog2(size);
    if (lsize > LUAI_MAXHBITS)
      luaG_runerror(L, "table overflow");
    size = luai_twoto(lsize);
    t->node = luaM_newvector(L, size, luai_Node);
    for (i = 0; i < (int)size; i++) {
      luai_Node *n = luai_gnode(t, i);
      luai_gnext(n) = 0;
      luai_setnilvalue(luai_wgkey(n));
      luai_setnilvalue(luai_gval(n));
    }
    t->lsizenode = luai_cast_byte(lsize);
    t->lastfree = luai_gnode(t, size);  /* all positions are free */
  }
}


void luaH_resize (lua_State *L, luai_Table *t, unsigned int nasize,
                                          unsigned int nhsize) {
  unsigned int i;
  int j;
  unsigned int oldasize = t->sizearray;
  int oldhsize = luai_allocsizenode(t);
  luai_Node *nold = t->node;  /* luai_save old hash ... */
  if (nasize > oldasize)  /* array part must grow? */
    luai_setarrayvector(L, t, nasize);
  /* create new hash part with appropriate size */
  luai_setnodevector(L, t, nhsize);
  if (nasize < oldasize) {  /* array part must shrink? */
    t->sizearray = nasize;
    /* re-insert elements from vanishing slice */
    for (i=nasize; i<oldasize; i++) {
      if (!luai_ttisnil(&t->array[i]))
        luaH_setint(L, t, i + 1, &t->array[i]);
    }
    /* shrink array */
    luaM_reallocvector(L, t->array, oldasize, nasize, luai_TValue);
  }
  /* re-insert elements from hash part */
  for (j = oldhsize - 1; j >= 0; j--) {
    luai_Node *old = nold + j;
    if (!luai_ttisnil(luai_gval(old))) {
      /* doesn't need barrier/invalidate cache, as entry was
         already present in the table */
      luai_setobjt2t(L, luaH_set(L, t, luai_gkey(old)), luai_gval(old));
    }
  }
  if (oldhsize > 0)  /* not the dummy node? */
    luaM_freearray(L, nold, luai_cast(size_t, oldhsize)); /* free old hash */
}


void luaH_resizearray (lua_State *L, luai_Table *t, unsigned int nasize) {
  int nsize = luai_allocsizenode(t);
  luaH_resize(L, t, nasize, nsize);
}

/*
** nums[i] = number of keys 'k' where 2^(i - 1) < k <= 2^i
*/
static void luai_rehash (lua_State *L, luai_Table *t, const luai_TValue *ek) {
  unsigned int asize;  /* optimal size for array part */
  unsigned int na;  /* number of keys in the array part */
  unsigned int nums[LUAI_MAXABITS + 1];
  int i;
  int totaluse;
  for (i = 0; i <= LUAI_MAXABITS; i++) nums[i] = 0;  /* reset counts */
  na = luai_numusearray(t, nums);  /* count keys in array part */
  totaluse = na;  /* all those keys are integer keys */
  totaluse += luai_numusehash(t, nums, &na);  /* count keys in hash part */
  /* count extra key */
  na += luai_countint(ek, nums);
  totaluse++;
  /* compute new size for array part */
  asize = luai_computesizes(nums, &na);
  /* resize the table to new computed sizes */
  luaH_resize(L, t, asize, totaluse - na);
}



/*
** }=============================================================
*/


luai_Table *luaH_new (lua_State *L) {
  LUAI_GCObject *o = luaC_newobj(L, LUA_TTABLE, sizeof(luai_Table));
  luai_Table *t = luai_gco2t(o);
  t->metatable = NULL;
  t->flags = luai_cast_byte(~0);
  t->array = NULL;
  t->sizearray = 0;
  luai_setnodevector(L, t, 0);
  return t;
}


void luaH_free (lua_State *L, luai_Table *t) {
  if (!luai_isdummy(t))
    luaM_freearray(L, t->node, luai_cast(size_t, luai_sizenode(t)));
  luaM_freearray(L, t->array, t->sizearray);
  luaM_free(L, t);
}


static luai_Node *luai_getfreepos (luai_Table *t) {
  if (!luai_isdummy(t)) {
    while (t->lastfree > t->node) {
      t->lastfree--;
      if (luai_ttisnil(luai_gkey(t->lastfree)))
        return t->lastfree;
    }
  }
  return NULL;  /* could not find a free place */
}



/*
** inserts a new key into a hash table; first, luai_check whether key's main
** position is free. If not, luai_check whether colliding node is in its main
** position or not: if it is not, move colliding node to an empty place and
** put new key in its main position; otherwise (colliding node is in its main
** position), new key goes to an empty position.
*/
luai_TValue *luaH_newkey (lua_State *L, luai_Table *t, const luai_TValue *key) {
  luai_Node *mp;
  luai_TValue aux;
  if (luai_ttisnil(key)) luaG_runerror(L, "table index is nil");
  else if (luai_ttisfloat(key)) {
    lua_Integer k;
    if (luaV_tointeger(key, &k, 0)) {  /* does index fit in an integer? */
      luai_setivalue(&aux, k);
      key = &aux;  /* insert it as an integer */
    }
    else if (luai_numisnan(luai_fltvalue(key)))
      luaG_runerror(L, "table index is NaN");
  }
  mp = luai_mainposition(t, key);
  if (!luai_ttisnil(luai_gval(mp)) || luai_isdummy(t)) {  /* main position is taken? */
    luai_Node *othern;
    luai_Node *f = luai_getfreepos(t);  /* get a free place */
    if (f == NULL) {  /* cannot find a free place? */
      luai_rehash(L, t, key);  /* grow table */
      /* whatever called 'newkey' takes care of TM cache */
      return luaH_set(L, t, key);  /* insert key into grown table */
    }
    lua_assert(!luai_isdummy(t));
    othern = luai_mainposition(t, luai_gkey(mp));
    if (othern != mp) {  /* is colliding node out of its main position? */
      /* yes; move colliding node into free position */
      while (othern + luai_gnext(othern) != mp)  /* find previous */
        othern += luai_gnext(othern);
      luai_gnext(othern) = luai_cast_int(f - othern);  /* rechain to point to 'f' */
      *f = *mp;  /* copy colliding node into free pos. (mp->luai_next also goes) */
      if (luai_gnext(mp) != 0) {
        luai_gnext(f) += luai_cast_int(mp - f);  /* correct 'luai_next' */
        luai_gnext(mp) = 0;  /* now 'mp' is free */
      }
      luai_setnilvalue(luai_gval(mp));
    }
    else {  /* colliding node is in its own main position */
      /* new node will go into free position */
      if (luai_gnext(mp) != 0)
        luai_gnext(f) = luai_cast_int((mp + luai_gnext(mp)) - f);  /* chain new position */
      else lua_assert(luai_gnext(f) == 0);
      luai_gnext(mp) = luai_cast_int(f - mp);
      mp = f;
    }
  }
  luai_setnodekey(L, &mp->i_key, key);
  luaC_barrierback(L, t, key);
  lua_assert(luai_ttisnil(luai_gval(mp)));
  return luai_gval(mp);
}


/*
** search function for integers
*/
const luai_TValue *luaH_getint (luai_Table *t, lua_Integer key) {
  /* (1 <= key && key <= t->sizearray) */
  if (luai_l_castS2U(key) - 1 < t->sizearray)
    return &t->array[key - 1];
  else {
    luai_Node *n = luai_hashint(t, key);
    for (;;) {  /* luai_check whether 'key' is somewhere in the chain */
      if (luai_ttisinteger(luai_gkey(n)) && luai_ivalue(luai_gkey(n)) == key)
        return luai_gval(n);  /* that's it */
      else {
        int nx = luai_gnext(n);
        if (nx == 0) break;
        n += nx;
      }
    }
    return luaO_nilobject;
  }
}


/*
** search function for short strings
*/
const luai_TValue *luaH_getshortstr (luai_Table *t, luai_TString *key) {
  luai_Node *n = luai_hashstr(t, key);
  lua_assert(key->tt == LUA_TSHRSTR);
  for (;;) {  /* luai_check whether 'key' is somewhere in the chain */
    const luai_TValue *k = luai_gkey(n);
    if (luai_ttisshrstring(k) && luai_eqshrstr(luai_tsvalue(k), key))
      return luai_gval(n);  /* that's it */
    else {
      int nx = luai_gnext(n);
      if (nx == 0)
        return luaO_nilobject;  /* not found */
      n += nx;
    }
  }
}


/*
** "Generic" get version. (Not that generic: not valid for integers,
** which may be in array part, nor for floats with integral values.)
*/
static const luai_TValue *luai_getgeneric (luai_Table *t, const luai_TValue *key) {
  luai_Node *n = luai_mainposition(t, key);
  for (;;) {  /* luai_check whether 'key' is somewhere in the chain */
    if (luaV_rawequalobj(luai_gkey(n), key))
      return luai_gval(n);  /* that's it */
    else {
      int nx = luai_gnext(n);
      if (nx == 0)
        return luaO_nilobject;  /* not found */
      n += nx;
    }
  }
}


const luai_TValue *luaH_getstr (luai_Table *t, luai_TString *key) {
  if (key->tt == LUA_TSHRSTR)
    return luaH_getshortstr(t, key);
  else {  /* for long strings, use generic case */
    luai_TValue ko;
    luai_setsvalue(luai_cast(lua_State *, NULL), &ko, key);
    return luai_getgeneric(t, &ko);
  }
}


/*
** main search function
*/
const luai_TValue *luaH_get (luai_Table *t, const luai_TValue *key) {
  switch (luai_ttype(key)) {
    case LUA_TSHRSTR: return luaH_getshortstr(t, luai_tsvalue(key));
    case LUA_TNUMINT: return luaH_getint(t, luai_ivalue(key));
    case LUA_TNIL: return luaO_nilobject;
    case LUA_TNUMFLT: {
      lua_Integer k;
      if (luaV_tointeger(key, &k, 0)) /* index is int? */
        return luaH_getint(t, k);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    default:
      return luai_getgeneric(t, key);
  }
}


/*
** beware: when using this function you probably need to luai_check a LUAI_GC
** barrier and invalidate the TM cache.
*/
luai_TValue *luaH_set (lua_State *L, luai_Table *t, const luai_TValue *key) {
  const luai_TValue *p = luaH_get(t, key);
  if (p != luaO_nilobject)
    return luai_cast(luai_TValue *, p);
  else return luaH_newkey(L, t, key);
}


void luaH_setint (lua_State *L, luai_Table *t, lua_Integer key, luai_TValue *value) {
  const luai_TValue *p = luaH_getint(t, key);
  luai_TValue *cell;
  if (p != luaO_nilobject)
    cell = luai_cast(luai_TValue *, p);
  else {
    luai_TValue k;
    luai_setivalue(&k, key);
    cell = luaH_newkey(L, t, &k);
  }
  luai_setobj2t(L, cell, value);
}


static int luai_unbound_search (luai_Table *t, unsigned int j) {
  unsigned int i = j;  /* i is zero or a present index */
  j++;
  /* find 'i' and 'j' such that i is present and j is not */
  while (!luai_ttisnil(luaH_getint(t, j))) {
    i = j;
    if (j > luai_cast(unsigned int, LUAI_MAX_INT)/2) {  /* overflow? */
      /* table was built with bad purposes: resort to linear search */
      i = 1;
      while (!luai_ttisnil(luaH_getint(t, i))) i++;
      return i - 1;
    }
    j *= 2;
  }
  /* now do a binary search between them */
  while (j - i > 1) {
    unsigned int m = (i+j)/2;
    if (luai_ttisnil(luaH_getint(t, m))) j = m;
    else i = m;
  }
  return i;
}


/*
** Try to find a boundary in table 't'. A 'boundary' is an integer index
** such that t[i] is non-nil and t[i+1] is nil (and 0 if t[1] is nil).
*/
int luaH_getn (luai_Table *t) {
  unsigned int j = t->sizearray;
  if (j > 0 && luai_ttisnil(&t->array[j - 1])) {
    /* there is a boundary in the array part: (binary) search for it */
    unsigned int i = 0;
    while (j - i > 1) {
      unsigned int m = (i+j)/2;
      if (luai_ttisnil(&t->array[m - 1])) j = m;
      else i = m;
    }
    return i;
  }
  /* else must find a boundary in hash part */
  else if (luai_isdummy(t))  /* hash part is empty? */
    return j;  /* that is easy... */
  else return luai_unbound_search(t, j);
}



#if defined(LUA_DEBUG)

luai_Node *luaH_mainposition (const luai_Table *t, const luai_TValue *key) {
  return luai_mainposition(t, key);
}

int luaH_isdummy (const luai_Table *t) { return luai_isdummy(t); }

#endif

/*__ltablib.c__*/

/*
** Operations that an object must define to mimic a table
** (some functions only need some of them)
*/
#define TAB_R	1			/* read */
#define TAB_W	2			/* write */
#define TAB_L	4			/* length */
#define TAB_RW	(TAB_R | TAB_W)		/* read/write */


#define aux_getn(L,n,w)	(luai_checktab(L, n, (w) | TAB_L), luaL_len(L, n))


static int luai_checkfield (lua_State *L, const char *key, int n) {
  lua_pushstring(L, key);
  return (lua_rawget(L, -n) != LUA_TNIL);
}


/*
** Check that 'arg' either is a table or can behave like one (that is,
** has a metatable with the required metamethods)
*/
static void luai_checktab (lua_State *L, int arg, int what) {
  if (lua_type(L, arg) != LUA_TTABLE) {  /* is it not a table? */
    int n = 1;  /* number of elements to pop */
    if (lua_getmetatable(L, arg) &&  /* must have metatable */
        (!(what & TAB_R) || luai_checkfield(L, "__index", ++n)) &&
        (!(what & TAB_W) || luai_checkfield(L, "__newindex", ++n)) &&
        (!(what & TAB_L) || luai_checkfield(L, "__len", ++n))) {
      lua_pop(L, n);  /* pop metatable and tested metamethods */
    }
    else
      luaL_checktype(L, arg, LUA_TTABLE);  /* force an error */
  }
}


#if defined(LUA_COMPAT_MAXN)
static int luai_maxn (lua_State *L) {
  lua_Number max = 0;
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_pushnil(L);  /* first key */
  while (lua_next(L, 1)) {
    lua_pop(L, 1);  /* remove value */
    if (lua_type(L, -1) == LUA_TNUMBER) {
      lua_Number v = lua_tonumber(L, -1);
      if (v > max) max = v;
    }
  }
  lua_pushnumber(L, max);
  return 1;
}
#endif


static int luai_tinsert (lua_State *L) {
  lua_Integer e = aux_getn(L, 1, TAB_RW) + 1;  /* first empty element */
  lua_Integer pos;  /* where to insert new element */
  switch (lua_gettop(L)) {
    case 2: {  /* called with only 2 arguments */
      pos = e;  /* insert new element at the end */
      break;
    }
    case 3: {
      lua_Integer i;
      pos = luaL_checkinteger(L, 2);  /* 2nd argument is the position */
      luaL_argcheck(L, 1 <= pos && pos <= e, 2, "position out of bounds");
      for (i = e; i > pos; i--) {  /* move up elements */
        lua_geti(L, 1, i - 1);
        lua_seti(L, 1, i);  /* t[i] = t[i - 1] */
      }
      break;
    }
    default: {
      return luaL_error(L, "wrong number of arguments to 'insert'");
    }
  }
  lua_seti(L, 1, pos);  /* t[pos] = v */
  return 0;
}


static int luai_tremove (lua_State *L) {
  lua_Integer size = aux_getn(L, 1, TAB_RW);
  lua_Integer pos = luaL_optinteger(L, 2, size);
  if (pos != size)  /* validate 'pos' if given */
    luaL_argcheck(L, 1 <= pos && pos <= size + 1, 1, "position out of bounds");
  lua_geti(L, 1, pos);  /* result = t[pos] */
  for ( ; pos < size; pos++) {
    lua_geti(L, 1, pos + 1);
    lua_seti(L, 1, pos);  /* t[pos] = t[pos + 1] */
  }
  lua_pushnil(L);
  lua_seti(L, 1, pos);  /* t[pos] = nil */
  return 1;
}


/*
** Copy elements (1[f], ..., 1[e]) into (tt[t], tt[t+1], ...). Whenever
** possible, copy in increasing order, which is better for rehashing.
** "possible" means destination after original range, or smaller
** than origin, or copying to another table.
*/
static int luai_tmove (lua_State *L) {
  lua_Integer f = luaL_checkinteger(L, 2);
  lua_Integer e = luaL_checkinteger(L, 3);
  lua_Integer t = luaL_checkinteger(L, 4);
  int tt = !lua_isnoneornil(L, 5) ? 5 : 1;  /* destination table */
  luai_checktab(L, 1, TAB_R);
  luai_checktab(L, tt, TAB_W);
  if (e >= f) {  /* otherwise, nothing to move */
    lua_Integer n, i;
    luaL_argcheck(L, f > 0 || e < LUA_MAXINTEGER + f, 3,
                  "too many elements to move");
    n = e - f + 1;  /* number of elements to move */
    luaL_argcheck(L, t <= LUA_MAXINTEGER - n + 1, 4,
                  "destination wrap around");
    if (t > e || t <= f || (tt != 1 && !lua_compare(L, 1, tt, LUA_OPEQ))) {
      for (i = 0; i < n; i++) {
        lua_geti(L, 1, f + i);
        lua_seti(L, tt, t + i);
      }
    }
    else {
      for (i = n - 1; i >= 0; i--) {
        lua_geti(L, 1, f + i);
        lua_seti(L, tt, t + i);
      }
    }
  }
  lua_pushvalue(L, tt);  /* return destination table */
  return 1;
}


static void luai_addfield (lua_State *L, luaL_Buffer *b, lua_Integer i) {
  lua_geti(L, 1, i);
  if (!lua_isstring(L, -1))
    luaL_error(L, "invalid value (%s) at index %d in table for 'concat'",
                  luaL_typename(L, -1), i);
  luaL_addvalue(b);
}


static int luai_tconcat (lua_State *L) {
  luaL_Buffer b;
  lua_Integer last = aux_getn(L, 1, TAB_R);
  size_t lsep;
  const char *sep = luaL_optlstring(L, 2, "", &lsep);
  lua_Integer i = luaL_optinteger(L, 3, 1);
  last = luaL_optinteger(L, 4, last);
  luaL_buffinit(L, &b);
  for (; i < last; i++) {
    luai_addfield(L, &b, i);
    luaL_addlstring(&b, sep, lsep);
  }
  if (i == last)  /* add last value (if interval was not empty) */
    luai_addfield(L, &b, i);
  luaL_pushresult(&b);
  return 1;
}


/*
** {======================================================
** Pack/luai_unpack
** =======================================================
*/

static int luai_pack (lua_State *L) {
  int i;
  int n = lua_gettop(L);  /* number of elements to luai_pack */
  lua_createtable(L, n, 1);  /* create result table */
  lua_insert(L, 1);  /* put it at index 1 */
  for (i = n; i >= 1; i--)  /* assign elements */
    lua_seti(L, 1, i);
  lua_pushinteger(L, n);
  lua_setfield(L, 1, "n");  /* t.n = number of elements */
  return 1;  /* return table */
}


static int luai_unpack (lua_State *L) {
  lua_Unsigned n;
  lua_Integer i = luaL_optinteger(L, 2, 1);
  lua_Integer e = luaL_opt(L, luaL_checkinteger, 3, luaL_len(L, 1));
  if (i > e) return 0;  /* empty range */
  n = (lua_Unsigned)e - i;  /* number of elements minus 1 (avoid overflows) */
  if (n >= (unsigned int)INT_MAX  || !lua_checkstack(L, (int)(++n)))
    return luaL_error(L, "too many results to luai_unpack");
  for (; i < e; i++) {  /* push arg[i..e - 1] (to avoid overflows) */
    lua_geti(L, 1, i);
  }
  lua_geti(L, 1, e);  /* push last element */
  return (int)n;
}

/* }====================================================== */



/*
** {======================================================
** Quicksort
** (based on 'Algorithms in MODULA-3', Robert Sedgewick;
**  Addison-Wesley, 1993.)
** =======================================================
*/


/* type for array indices */
typedef unsigned int luai_IdxT;


/*
** Produce a "random" 'unsigned int' to randomize pivot choice. This
** macro is used only when 'luai_sort' detects a big imbalance in the result
** of a luai_partition. (If you don't want/need this "randomness", ~0 is a
** good choice.)
*/
#if !defined(luai_l_randomizePivot)		/* { */

#include <time.h>

/* size of 'e' measured in number of 'unsigned int's */
#define sof(e)		(sizeof(e) / sizeof(unsigned int))

/*
** Use 'time' and 'clock' as sources of "randomness". Because we don't
** know the types 'clock_t' and 'time_t', we cannot luai_cast them to
** anything without risking overflows. A safe way to use their values
** is to copy them to an array of a known type and use the array values.
*/
static unsigned int luai_l_randomizePivot (void) {
  clock_t c = clock();
  time_t t = time(NULL);
  unsigned int buff[sof(c) + sof(t)];
  unsigned int i, rnd = 0;
  memcpy(buff, &c, sof(c) * sizeof(unsigned int));
  memcpy(buff + sof(c), &t, sof(t) * sizeof(unsigned int));
  for (i = 0; i < sof(buff); i++)
    rnd += buff[i];
  return rnd;
}

#endif					/* } */


/* arrays larger than 'LUAI_RANLIMIT' may use randomized pivots */
#define LUAI_RANLIMIT	100u


static void luai_set2 (lua_State *L, luai_IdxT i, luai_IdxT j) {
  lua_seti(L, 1, i);
  lua_seti(L, 1, j);
}


/*
** Return true iff value at stack index 'a' is less than the value at
** index 'b' (according to the order of the luai_sort).
*/
static int luai_sort_comp (lua_State *L, int a, int b) {
  if (lua_isnil(L, 2))  /* no function? */
    return lua_compare(L, a, b, LUA_OPLT);  /* a < b */
  else {  /* function */
    int res;
    lua_pushvalue(L, 2);    /* push function */
    lua_pushvalue(L, a-1);  /* -1 to compensate function */
    lua_pushvalue(L, b-2);  /* -2 to compensate function and 'a' */
    lua_call(L, 2, 1);      /* call function */
    res = lua_toboolean(L, -1);  /* get result */
    lua_pop(L, 1);          /* pop result */
    return res;
  }
}


/*
** Does the luai_partition: Pivot P is at the top of the stack.
** precondition: a[lo] <= P == a[up-1] <= a[up],
** so it only needs to do the luai_partition from lo + 1 to up - 2.
** Pos-condition: a[lo .. i - 1] <= a[i] == P <= a[i + 1 .. up]
** returns 'i'.
*/
static luai_IdxT luai_partition (lua_State *L, luai_IdxT lo, luai_IdxT up) {
  luai_IdxT i = lo;  /* will be incremented before first use */
  luai_IdxT j = up - 1;  /* will be decremented before first use */
  /* loop invariant: a[lo .. i] <= P <= a[j .. up] */
  for (;;) {
    /* luai_next loop: repeat ++i while a[i] < P */
    while (lua_geti(L, 1, ++i), luai_sort_comp(L, -1, -2)) {
      if (i == up - 1)  /* a[i] < P  but a[up - 1] == P  ?? */
        luaL_error(L, "invalid order function for sorting");
      lua_pop(L, 1);  /* remove a[i] */
    }
    /* after the loop, a[i] >= P and a[lo .. i - 1] < P */
    /* luai_next loop: repeat --j while P < a[j] */
    while (lua_geti(L, 1, --j), luai_sort_comp(L, -3, -1)) {
      if (j < i)  /* j < i  but  a[j] > P ?? */
        luaL_error(L, "invalid order function for sorting");
      lua_pop(L, 1);  /* remove a[j] */
    }
    /* after the loop, a[j] <= P and a[j + 1 .. up] >= P */
    if (j < i) {  /* no elements out of place? */
      /* a[lo .. i - 1] <= P <= a[j + 1 .. i .. up] */
      lua_pop(L, 1);  /* pop a[j] */
      /* swap pivot (a[up - 1]) with a[i] to satisfy pos-condition */
      luai_set2(L, up - 1, i);
      return i;
    }
    /* otherwise, swap a[i] - a[j] to restore invariant and repeat */
    luai_set2(L, i, j);
  }
}


/*
** Choose an element in the middle (2nd-3th quarters) of [lo,up]
** "randomized" by 'rnd'
*/
static luai_IdxT luai_choosePivot (luai_IdxT lo, luai_IdxT up, unsigned int rnd) {
  luai_IdxT r4 = (up - lo) / 4;  /* range/4 */
  luai_IdxT p = rnd % (r4 * 2) + (lo + r4);
  lua_assert(lo + r4 <= p && p <= up - r4);
  return p;
}


/*
** QuickSort algorithm (recursive function)
*/
static void luai_auxsort (lua_State *L, luai_IdxT lo, luai_IdxT up,
                                   unsigned int rnd) {
  while (lo < up) {  /* loop for tail recursion */
    luai_IdxT p;  /* Pivot index */
    luai_IdxT n;  /* to be used later */
    /* luai_sort elements 'lo', 'p', and 'up' */
    lua_geti(L, 1, lo);
    lua_geti(L, 1, up);
    if (luai_sort_comp(L, -1, -2))  /* a[up] < a[lo]? */
      luai_set2(L, lo, up);  /* swap a[lo] - a[up] */
    else
      lua_pop(L, 2);  /* remove both values */
    if (up - lo == 1)  /* only 2 elements? */
      return;  /* already sorted */
    if (up - lo < LUAI_RANLIMIT || rnd == 0)  /* small interval or no randomize? */
      p = (lo + up)/2;  /* middle element is a good pivot */
    else  /* for larger intervals, it is worth a random pivot */
      p = luai_choosePivot(lo, up, rnd);
    lua_geti(L, 1, p);
    lua_geti(L, 1, lo);
    if (luai_sort_comp(L, -2, -1))  /* a[p] < a[lo]? */
      luai_set2(L, p, lo);  /* swap a[p] - a[lo] */
    else {
      lua_pop(L, 1);  /* remove a[lo] */
      lua_geti(L, 1, up);
      if (luai_sort_comp(L, -1, -2))  /* a[up] < a[p]? */
        luai_set2(L, p, up);  /* swap a[up] - a[p] */
      else
        lua_pop(L, 2);
    }
    if (up - lo == 2)  /* only 3 elements? */
      return;  /* already sorted */
    lua_geti(L, 1, p);  /* get middle element (Pivot) */
    lua_pushvalue(L, -1);  /* push Pivot */
    lua_geti(L, 1, up - 1);  /* push a[up - 1] */
    luai_set2(L, p, up - 1);  /* swap Pivot (a[p]) with a[up - 1] */
    p = luai_partition(L, lo, up);
    /* a[lo .. p - 1] <= a[p] == P <= a[p + 1 .. up] */
    if (p - lo < up - p) {  /* lower interval is smaller? */
      luai_auxsort(L, lo, p - 1, rnd);  /* call recursively for lower interval */
      n = p - lo;  /* size of smaller interval */
      lo = p + 1;  /* tail call for [p + 1 .. up] (upper interval) */
    }
    else {
      luai_auxsort(L, p + 1, up, rnd);  /* call recursively for upper interval */
      n = up - p;  /* size of smaller interval */
      up = p - 1;  /* tail call for [lo .. p - 1]  (lower interval) */
    }
    if ((up - lo) / 128 > n) /* luai_partition too imbalanced? */
      rnd = luai_l_randomizePivot();  /* try a new randomization */
  }  /* tail call luai_auxsort(L, lo, up, rnd) */
}


static int luai_sort (lua_State *L) {
  lua_Integer n = aux_getn(L, 1, TAB_RW);
  if (n > 1) {  /* non-trivial interval? */
    luaL_argcheck(L, n < INT_MAX, 1, "array too big");
    if (!lua_isnoneornil(L, 2))  /* is there a 2nd argument? */
      luaL_checktype(L, 2, LUA_TFUNCTION);  /* must be a function */
    lua_settop(L, 2);  /* make sure there are two arguments */
    luai_auxsort(L, 1, (luai_IdxT)n, 0);
  }
  return 0;
}

/* }====================================================== */


static const luaL_Reg tab_funcs[] = {
  {"concat", luai_tconcat},
#if defined(LUA_COMPAT_MAXN)
  {"maxn", luai_maxn},
#endif
  {"insert", luai_tinsert},
  {"pack", luai_pack},
  {"unpack", luai_unpack},
  {"remove", luai_tremove},
  {"move", luai_tmove},
  {"sort", luai_sort},
  {NULL, NULL}
};


LUAMOD_API int luaopen_table (lua_State *L) {
  luaL_newlib(L, tab_funcs);
#if defined(LUA_COMPAT_UNPACK)
  /* _G.luai_unpack = table.luai_unpack */
  lua_getfield(L, -1, "unpack");
  lua_setglobal(L, "unpack");
#endif
  return 1;
}

/*__ltm.c__*/

static const char luai_udatatypename[] = "userdata";

LUAI_DDEF const char *const luaT_typenames_[LUA_TOTALTAGS] = {
  "no value",
  "nil", "boolean", luai_udatatypename, "number",
  "string", "table", "function", luai_udatatypename, "thread",
  "proto" /* this last case is used for tests only */
};


void luaT_init (lua_State *L) {
  static const char *const luaT_eventname[] = {  /* ORDER TM */
    "__index", "__newindex",
    "__gc", "__mode", "__len", "__eq",
    "__add", "__sub", "__mul", "__mod", "__pow",
    "__div", "__idiv",
    "__band", "__bor", "__bxor", "__shl", "__shr",
    "__unm", "__bnot", "__lt", "__le",
    "__concat", "__call"
  };
  int i;
  for (i=0; i<LUAI_TM_N; i++) {
    LUAI_G(L)->tmname[i] = luaS_new(L, luaT_eventname[i]);
    luaC_fix(L, luai_obj2gco(LUAI_G(L)->tmname[i]));  /* never collect these names */
  }
}


/*
** function to be used with macro "fasttm": optimized for absence of
** tag methods
*/
const luai_TValue *luaT_gettm (luai_Table *events, luai_TMS event, luai_TString *ename) {
  const luai_TValue *tm = luaH_getshortstr(events, ename);
  lua_assert(event <= LUAI_TM_EQ);
  if (luai_ttisnil(tm)) {  /* no tag method? */
    events->flags |= luai_cast_byte(1u<<event);  /* cache this fact */
    return NULL;
  }
  else return tm;
}


const luai_TValue *luaT_gettmbyobj (lua_State *L, const luai_TValue *o, luai_TMS event) {
  luai_Table *mt;
  switch (luai_ttnov(o)) {
    case LUA_TTABLE:
      mt = luai_hvalue(o)->metatable;
      break;
    case LUA_TUSERDATA:
      mt = luai_uvalue(o)->metatable;
      break;
    default:
      mt = LUAI_G(L)->mt[luai_ttnov(o)];
  }
  return (mt ? luaH_getshortstr(mt, LUAI_G(L)->tmname[event]) : luaO_nilobject);
}


/*
** Return the name of the type of an object. For tables and userdata
** with metatable, use their '__name' metafield, if present.
*/
const char *luaT_objtypename (lua_State *L, const luai_TValue *o) {
  luai_Table *mt;
  if ((luai_ttistable(o) && (mt = luai_hvalue(o)->metatable) != NULL) ||
      (luai_ttisfulluserdata(o) && (mt = luai_uvalue(o)->metatable) != NULL)) {
    const luai_TValue *name = luaH_getshortstr(mt, luaS_new(L, "__name"));
    if (luai_ttisstring(name))  /* is '__name' a string? */
      return luai_getstr(luai_tsvalue(name));  /* use it as type name */
  }
  return luai_ttypename(luai_ttnov(o));  /* else use standard type name */
}


void luaT_callTM (lua_State *L, const luai_TValue *f, const luai_TValue *p1,
                  const luai_TValue *p2, luai_TValue *p3, int hasres) {
  ptrdiff_t result = luai_savestack(L, p3);
  luai_StkId func = L->top;
  luai_setobj2s(L, func, f);  /* push function (assume LUAI_EXTRA_STACK) */
  luai_setobj2s(L, func + 1, p1);  /* 1st argument */
  luai_setobj2s(L, func + 2, p2);  /* 2nd argument */
  L->top += 3;
  if (!hasres)  /* no result? 'p3' is third argument */
    luai_setobj2s(L, L->top++, p3);  /* 3rd argument */
  /* metamethod may yield only when called from Lua code */
  if (luai_isLua(L->ci))
    luaD_call(L, func, hasres);
  else
    luaD_callnoyield(L, func, hasres);
  if (hasres) {  /* if has result, move it to its place */
    p3 = luai_restorestack(L, result);
    luai_setobjs2s(L, p3, --L->top);
  }
}


int luaT_callbinTM (lua_State *L, const luai_TValue *p1, const luai_TValue *p2,
                    luai_StkId res, luai_TMS event) {
  const luai_TValue *tm = luaT_gettmbyobj(L, p1, event);  /* try first operand */
  if (luai_ttisnil(tm))
    tm = luaT_gettmbyobj(L, p2, event);  /* try second operand */
  if (luai_ttisnil(tm)) return 0;
  luaT_callTM(L, tm, p1, p2, res, 1);
  return 1;
}


void luaT_trybinTM (lua_State *L, const luai_TValue *p1, const luai_TValue *p2,
                    luai_StkId res, luai_TMS event) {
  if (!luaT_callbinTM(L, p1, p2, res, event)) {
    switch (event) {
      case LUAI_TM_CONCAT:
        luaG_concaterror(L, p1, p2);
      /* call never returns, but to avoid warnings: *//* FALLTHROUGH */
      case LUAI_TM_BAND: case LUAI_TM_BOR: case LUAI_TM_BXOR:
      case LUAI_TM_SHL: case LUAI_TM_SHR: case LUAI_TM_BNOT: {
        lua_Number dummy;
        if (luai_tonumber(p1, &dummy) && luai_tonumber(p2, &dummy))
          luaG_tointerror(L, p1, p2);
        else
          luaG_opinterror(L, p1, p2, "perform bitwise operation on");
      }
      /* calls never return, but to avoid warnings: *//* FALLTHROUGH */
      default:
        luaG_opinterror(L, p1, p2, "perform arithmetic on");
    }
  }
}


int luaT_callorderTM (lua_State *L, const luai_TValue *p1, const luai_TValue *p2,
                      luai_TMS event) {
  if (!luaT_callbinTM(L, p1, p2, L->top, event))
    return -1;  /* no metamethod */
  else
    return !luai_l_isfalse(L->top);
}

/*__lundump.c__*/

#if !defined(luai_verifycode)
#define luai_verifycode(L,b,f)  /* empty */
#endif


typedef struct {
  lua_State *L;
  LUAI_ZIO *Z;
  const char *name;
} luai_LoadState;


static luai_l_noret luai_error(luai_LoadState *S, const char *why) {
  luaO_pushfstring(S->L, "%s: %s precompiled chunk", S->name, why);
  luaD_throw(S->L, LUA_ERRSYNTAX);
}


/*
** All high-level loads go through luai_LoadVector; you can change it to
** adapt to the endianness of the input
*/
#define luai_LoadVector(S,b,n)	luai_LoadBlock(S,b,(n)*sizeof((b)[0]))

static void luai_LoadBlock (luai_LoadState *S, void *b, size_t size) {
  if (luaZ_read(S->Z, b, size) != 0)
    luai_error(S, "truncated");
}


#define luai_LoadVar(S,x)		luai_LoadVector(S,&x,1)


static luai_lu_byte luai_LoadByte (luai_LoadState *S) {
  luai_lu_byte x;
  luai_LoadVar(S, x);
  return x;
}


static int luai_LoadInt (luai_LoadState *S) {
  int x;
  luai_LoadVar(S, x);
  return x;
}


static lua_Number luai_LoadNumber (luai_LoadState *S) {
  lua_Number x;
  luai_LoadVar(S, x);
  return x;
}


static lua_Integer luai_LoadInteger (luai_LoadState *S) {
  lua_Integer x;
  luai_LoadVar(S, x);
  return x;
}


static luai_TString *luai_LoadString (luai_LoadState *S) {
  size_t size = luai_LoadByte(S);
  if (size == 0xFF)
    luai_LoadVar(S, size);
  if (size == 0)
    return NULL;
  else if (--size <= LUAI_MAXSHORTLEN) {  /* short string? */
    char buff[LUAI_MAXSHORTLEN];
    luai_LoadVector(S, buff, size);
    return luaS_newlstr(S->L, buff, size);
  }
  else {  /* long string */
    luai_TString *ts = luaS_createlngstrobj(S->L, size);
    luai_LoadVector(S, luai_getstr(ts), size);  /* load directly in final place */
    return ts;
  }
}


static void luai_LoadCode (luai_LoadState *S, luai_Proto *f) {
  int n = luai_LoadInt(S);
  f->code = luaM_newvector(S->L, n, Instruction);
  f->sizecode = n;
  luai_LoadVector(S, f->code, n);
}


static void luai_LoadFunction(luai_LoadState *S, luai_Proto *f, luai_TString *psource);


static void luai_LoadConstants (luai_LoadState *S, luai_Proto *f) {
  int i;
  int n = luai_LoadInt(S);
  f->k = luaM_newvector(S->L, n, luai_TValue);
  f->sizek = n;
  for (i = 0; i < n; i++)
    luai_setnilvalue(&f->k[i]);
  for (i = 0; i < n; i++) {
    luai_TValue *o = &f->k[i];
    int t = luai_LoadByte(S);
    switch (t) {
    case LUA_TNIL:
      luai_setnilvalue(o);
      break;
    case LUA_TBOOLEAN:
      luai_setbvalue(o, luai_LoadByte(S));
      break;
    case LUA_TNUMFLT:
      luai_setfltvalue(o, luai_LoadNumber(S));
      break;
    case LUA_TNUMINT:
      luai_setivalue(o, luai_LoadInteger(S));
      break;
    case LUA_TSHRSTR:
    case LUA_TLNGSTR:
      luai_setsvalue2n(S->L, o, luai_LoadString(S));
      break;
    default:
      lua_assert(0);
    }
  }
}


static void luai_LoadProtos (luai_LoadState *S, luai_Proto *f) {
  int i;
  int n = luai_LoadInt(S);
  f->p = luaM_newvector(S->L, n, luai_Proto *);
  f->sizep = n;
  for (i = 0; i < n; i++)
    f->p[i] = NULL;
  for (i = 0; i < n; i++) {
    f->p[i] = luaF_newproto(S->L);
    luai_LoadFunction(S, f->p[i], f->source);
  }
}


static void luai_LoadUpvalues (luai_LoadState *S, luai_Proto *f) {
  int i, n;
  n = luai_LoadInt(S);
  f->upvalues = luaM_newvector(S->L, n, luai_Upvaldesc);
  f->sizeupvalues = n;
  for (i = 0; i < n; i++)
    f->upvalues[i].name = NULL;
  for (i = 0; i < n; i++) {
    f->upvalues[i].instack = luai_LoadByte(S);
    f->upvalues[i].idx = luai_LoadByte(S);
  }
}


static void luai_LoadDebug (luai_LoadState *S, luai_Proto *f) {
  int i, n;
  n = luai_LoadInt(S);
  f->lineinfo = luaM_newvector(S->L, n, int);
  f->sizelineinfo = n;
  luai_LoadVector(S, f->lineinfo, n);
  n = luai_LoadInt(S);
  f->locvars = luaM_newvector(S->L, n, luai_LocVar);
  f->sizelocvars = n;
  for (i = 0; i < n; i++)
    f->locvars[i].varname = NULL;
  for (i = 0; i < n; i++) {
    f->locvars[i].varname = luai_LoadString(S);
    f->locvars[i].startpc = luai_LoadInt(S);
    f->locvars[i].endpc = luai_LoadInt(S);
  }
  n = luai_LoadInt(S);
  for (i = 0; i < n; i++)
    f->upvalues[i].name = luai_LoadString(S);
}


static void luai_LoadFunction (luai_LoadState *S, luai_Proto *f, luai_TString *psource) {
  f->source = luai_LoadString(S);
  if (f->source == NULL)  /* no source in dump? */
    f->source = psource;  /* reuse parent's source */
  f->linedefined = luai_LoadInt(S);
  f->lastlinedefined = luai_LoadInt(S);
  f->numparams = luai_LoadByte(S);
  f->is_vararg = luai_LoadByte(S);
  f->maxstacksize = luai_LoadByte(S);
  luai_LoadCode(S, f);
  luai_LoadConstants(S, f);
  luai_LoadUpvalues(S, f);
  luai_LoadProtos(S, f);
  luai_LoadDebug(S, f);
}


static void luai_checkliteral (luai_LoadState *S, const char *s, const char *msg) {
  char buff[sizeof(LUA_SIGNATURE) + sizeof(LUAC_DATA)]; /* larger than both */
  size_t len = strlen(s);
  luai_LoadVector(S, buff, len);
  if (memcmp(s, buff, len) != 0)
    luai_error(S, msg);
}


static void luai_fchecksize (luai_LoadState *S, size_t size, const char *tname) {
  if (luai_LoadByte(S) != size)
    luai_error(S, luaO_pushfstring(S->L, "%s size mismatch in", tname));
}


#define luai_checksize(S,t)	luai_fchecksize(S,sizeof(t),#t)

static void luai_checkHeader (luai_LoadState *S) {
  luai_checkliteral(S, LUA_SIGNATURE + 1, "not a");  /* 1st char already checked */
  if (luai_LoadByte(S) != LUAC_VERSION)
    luai_error(S, "version mismatch in");
  if (luai_LoadByte(S) != LUAC_FORMAT)
    luai_error(S, "format mismatch in");
  luai_checkliteral(S, LUAC_DATA, "corrupted");
  luai_checksize(S, int);
  luai_checksize(S, size_t);
  luai_checksize(S, Instruction);
  luai_checksize(S, lua_Integer);
  luai_checksize(S, lua_Number);
  if (luai_LoadInteger(S) != LUAC_INT)
    luai_error(S, "endianness mismatch in");
  if (luai_LoadNumber(S) != LUAC_NUM)
    luai_error(S, "float format mismatch in");
}


/*
** load precompiled chunk
*/
luai_LClosure *luaU_undump(lua_State *L, LUAI_ZIO *Z, const char *name) {
  luai_LoadState S;
  luai_LClosure *cl;
  if (*name == '@' || *name == '=')
    S.name = name + 1;
  else if (*name == LUA_SIGNATURE[0])
    S.name = "binary string";
  else
    S.name = name;
  S.L = L;
  S.Z = Z;
  luai_checkHeader(&S);
  cl = luaF_newLclosure(L, luai_LoadByte(&S));
  luai_setclLvalue(L, L->top, cl);
  luaD_inctop(L);
  cl->p = luaF_newproto(L);
  luai_LoadFunction(&S, cl->p, NULL);
  lua_assert(cl->nupvalues == cl->p->sizeupvalues);
  luai_verifycode(L, buff, cl->p);
  return cl;
}

/*__lutf8lib.c__*/

#define LUA_MAXUNICODE	0x10FFFF

#define luai_iscont(p)	((*(p) & 0xC0) == 0x80)


/* from luai_strlib */
/* translate a relative string position: negative means back from end */
static lua_Integer luai_u_posrelat (lua_Integer pos, size_t len) {
  if (pos >= 0) return pos;
  else if (0u - (size_t)pos > len) return 0;
  else return (lua_Integer)len + pos + 1;
}


/*
** Decode one UTF-8 sequence, returning NULL if byte sequence is invalid.
*/
static const char *utf8_decode (const char *o, int *val) {
  static const unsigned int limits[] = {0xFF, 0x7F, 0x7FF, 0xFFFF};
  const unsigned char *s = (const unsigned char *)o;
  unsigned int c = s[0];
  unsigned int res = 0;  /* final result */
  if (c < 0x80)  /* ascii? */
    res = c;
  else {
    int count = 0;  /* to count number of continuation bytes */
    while (c & 0x40) {  /* still have continuation bytes? */
      int cc = s[++count];  /* read luai_next byte */
      if ((cc & 0xC0) != 0x80)  /* not a continuation byte? */
        return NULL;  /* invalid byte sequence */
      res = (res << 6) | (cc & 0x3F);  /* add lower 6 bits from cont. byte */
      c <<= 1;  /* to test luai_next bit */
    }
    res |= ((c & 0x7F) << (count * 5));  /* add first byte */
    if (count > 3 || res > LUA_MAXUNICODE || res <= limits[count])
      return NULL;  /* invalid byte sequence */
    s += count;  /* skip continuation bytes read */
  }
  if (val) *val = res;
  return (const char *)s + 1;  /* +1 to include first byte */
}


/*
** utf8len(s [, i [, j]]) --> number of characters that start in the
** range [i,j], or nil + current position if 's' is not well formed in
** that interval
*/
static int luai_utflen (lua_State *L) {
  int n = 0;
  size_t len;
  const char *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = luai_u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer posj = luai_u_posrelat(luaL_optinteger(L, 3, -1), len);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 2,
                   "initial position out of string");
  luaL_argcheck(L, --posj < (lua_Integer)len, 3,
                   "final position out of string");
  while (posi <= posj) {
    const char *s1 = utf8_decode(s + posi, NULL);
    if (s1 == NULL) {  /* conversion error? */
      lua_pushnil(L);  /* return nil ... */
      lua_pushinteger(L, posi + 1);  /* ... and current position */
      return 2;
    }
    posi = s1 - s;
    n++;
  }
  lua_pushinteger(L, n);
  return 1;
}


/*
** luai_codepoint(s, [i, [j]])  -> returns codepoints for all characters
** that start in the range [i,j]
*/
static int luai_codepoint (lua_State *L) {
  size_t len;
  const char *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = luai_u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer pose = luai_u_posrelat(luaL_optinteger(L, 3, posi), len);
  int n;
  const char *se;
  luaL_argcheck(L, posi >= 1, 2, "out of range");
  luaL_argcheck(L, pose <= (lua_Integer)len, 3, "out of range");
  if (posi > pose) return 0;  /* empty interval; return no values */
  if (pose - posi >= INT_MAX)  /* (lua_Integer -> int) overflow? */
    return luaL_error(L, "string slice too long");
  n = (int)(pose -  posi) + 1;
  luaL_checkstack(L, n, "string slice too long");
  n = 0;
  se = s + pose;
  for (s += posi - 1; s < se;) {
    int code;
    s = utf8_decode(s, &code);
    if (s == NULL)
      return luaL_error(L, "invalid UTF-8 code");
    lua_pushinteger(L, code);
    n++;
  }
  return n;
}


static void luai_pushutfchar (lua_State *L, int arg) {
  lua_Integer code = luaL_checkinteger(L, arg);
  luaL_argcheck(L, 0 <= code && code <= LUA_MAXUNICODE, arg, "value out of range");
  lua_pushfstring(L, "%U", (long)code);
}


/*
** luai_utfchar(n1, n2, ...)  -> char(n1)..char(n2)...
*/
static int luai_utfchar (lua_State *L) {
  int n = lua_gettop(L);  /* number of arguments */
  if (n == 1)  /* optimize common case of single char */
    luai_pushutfchar(L, 1);
  else {
    int i;
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    for (i = 1; i <= n; i++) {
      luai_pushutfchar(L, i);
      luaL_addvalue(&b);
    }
    luaL_pushresult(&b);
  }
  return 1;
}


/*
** offset(s, n, [i])  -> index where n-th character counting from
**   position 'i' starts; 0 means character at 'i'.
*/
static int luai_byteoffset (lua_State *L) {
  size_t len;
  const char *s = luaL_checklstring(L, 1, &len);
  lua_Integer n  = luaL_checkinteger(L, 2);
  lua_Integer posi = (n >= 0) ? 1 : len + 1;
  posi = luai_u_posrelat(luaL_optinteger(L, 3, posi), len);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 3,
                   "position out of range");
  if (n == 0) {
    /* find beginning of current byte sequence */
    while (posi > 0 && luai_iscont(s + posi)) posi--;
  }
  else {
    if (luai_iscont(s + posi))
      luaL_error(L, "initial position is a continuation byte");
    if (n < 0) {
       while (n < 0 && posi > 0) {  /* move back */
         do {  /* find beginning of previous character */
           posi--;
         } while (posi > 0 && luai_iscont(s + posi));
         n++;
       }
     }
     else {
       n--;  /* do not move for 1st character */
       while (n > 0 && posi < (lua_Integer)len) {
         do {  /* find beginning of luai_next character */
           posi++;
         } while (luai_iscont(s + posi));  /* (cannot pass final '\0') */
         n--;
       }
     }
  }
  if (n == 0)  /* did it find given character? */
    lua_pushinteger(L, posi + 1);
  else  /* no such character */
    lua_pushnil(L);
  return 1;
}


static int luai_iter_aux (lua_State *L) {
  size_t len;
  const char *s = luaL_checklstring(L, 1, &len);
  lua_Integer n = lua_tointeger(L, 2) - 1;
  if (n < 0)  /* first iteration? */
    n = 0;  /* start from here */
  else if (n < (lua_Integer)len) {
    n++;  /* skip current byte */
    while (luai_iscont(s + n)) n++;  /* and its continuations */
  }
  if (n >= (lua_Integer)len)
    return 0;  /* no more codepoints */
  else {
    int code;
    const char *luai_next = utf8_decode(s + n, &code);
    if (luai_next == NULL || luai_iscont(luai_next))
      return luaL_error(L, "invalid UTF-8 code");
    lua_pushinteger(L, n + 1);
    lua_pushinteger(L, code);
    return 2;
  }
}


static int luai_iter_codes (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_pushcfunction(L, luai_iter_aux);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, 0);
  return 3;
}


/* pattern to luai_match a single UTF-8 character */
#define LUAI_UTF8PATT	"[\0-\x7F\xC2-\xF4][\x80-\xBF]*"


static const luaL_Reg luai_funcs[] = {
  {"offset", luai_byteoffset},
  {"codepoint", luai_codepoint},
  {"char", luai_utfchar},
  {"len", luai_utflen},
  {"codes", luai_iter_codes},
  /* placeholders */
  {"charpattern", NULL},
  {NULL, NULL}
};


LUAMOD_API int luaopen_utf8 (lua_State *L) {
  luaL_newlib(L, luai_funcs);
  lua_pushlstring(L, LUAI_UTF8PATT, sizeof(LUAI_UTF8PATT)/sizeof(char) - 1);
  lua_setfield(L, -2, "charpattern");
  return 1;
}

/*__lvm.c__*/

/* limit for table tag-method chains (to avoid loops) */
#define LUAI_MAXTAGLOOP	2000



/*
** 'luai_l_intfitsf' checks whether a given integer can be converted to a
** float without rounding. Used in comparisons. Left undefined if
** all integers fit in a float precisely.
*/
#if !defined(luai_l_intfitsf)

/* number of bits in the mantissa of a float */
#define LUAI_NBM		(luai_l_mathlim(MANT_DIG))

/*
** Check whether some integers may not fit in a float, that is, whether
** (maxinteger >> LUAI_NBM) > 0 (that implies (1 << LUAI_NBM) <= maxinteger).
** (The shifts are done in parts to avoid shifting by more than the size
** of an integer. In a worst case, LUAI_NBM == 113 for long double and
** sizeof(integer) == 32.)
*/
#if ((((LUA_MAXINTEGER >> (LUAI_NBM / 4)) >> (LUAI_NBM / 4)) >> (LUAI_NBM / 4)) \
	>> (LUAI_NBM - (3 * (LUAI_NBM / 4))))  >  0

#define luai_l_intfitsf(i)  \
  (-((lua_Integer)1 << LUAI_NBM) <= (i) && (i) <= ((lua_Integer)1 << LUAI_NBM))

#endif

#endif



/*
** Try to convert a value to a float. The float case is already handled
** by the macro 'luai_tonumber'.
*/
int luaV_tonumber_ (const luai_TValue *obj, lua_Number *n) {
  luai_TValue v;
  if (luai_ttisinteger(obj)) {
    *n = luai_cast_num(luai_ivalue(obj));
    return 1;
  }
  else if (luai_cvt2num(obj) &&  /* string convertible to number? */
            luaO_str2num(luai_svalue(obj), &v) == luai_vslen(obj) + 1) {
    *n = luai_nvalue(&v);  /* convert result of 'luaO_str2num' to a float */
    return 1;
  }
  else
    return 0;  /* conversion failed */
}


/*
** try to convert a value to an integer, rounding according to 'mode':
** mode == 0: accepts only integral values
** mode == 1: takes the floor of the number
** mode == 2: takes the ceil of the number
*/
int luaV_tointeger (const luai_TValue *obj, lua_Integer *p, int mode) {
  luai_TValue v;
 again:
  if (luai_ttisfloat(obj)) {
    lua_Number n = luai_fltvalue(obj);
    lua_Number f = luai_l_floor(n);
    if (n != f) {  /* not an integral value? */
      if (mode == 0) return 0;  /* fails if mode demands integral value */
      else if (mode > 1)  /* needs ceil? */
        f += 1;  /* convert floor to ceil (remember: n != f) */
    }
    return lua_numbertointeger(f, p);
  }
  else if (luai_ttisinteger(obj)) {
    *p = luai_ivalue(obj);
    return 1;
  }
  else if (luai_cvt2num(obj) &&
            luaO_str2num(luai_svalue(obj), &v) == luai_vslen(obj) + 1) {
    obj = &v;
    goto again;  /* convert result from 'luaO_str2num' to an integer */
  }
  return 0;  /* conversion failed */
}


/*
** Try to convert a 'for' limit to an integer, preserving the
** semantics of the loop.
** (The following explanation assumes a non-negative step; it is valid
** for negative steps mutatis mutandis.)
** If the limit can be converted to an integer, rounding down, that is
** it.
** Otherwise, luai_check whether the limit can be converted to a number.  If
** the number is too large, it is OK to set the limit as LUA_MAXINTEGER,
** which means no limit.  If the number is too negative, the loop
** should not run, because any initial integer value is larger than the
** limit. So, it sets the limit to LUA_MININTEGER. 'stopnow' corrects
** the extreme case when the initial value is LUA_MININTEGER, in which
** case the LUA_MININTEGER limit would still run the loop once.
*/
static int luai_forlimit (const luai_TValue *obj, lua_Integer *p, lua_Integer step,
                     int *stopnow) {
  *stopnow = 0;  /* usually, let loops run */
  if (!luaV_tointeger(obj, p, (step < 0 ? 2 : 1))) {  /* not fit in integer? */
    lua_Number n;  /* try to convert to float */
    if (!luai_tonumber(obj, &n)) /* cannot convert to float? */
      return 0;  /* not a number */
    if (luai_numlt(0, n)) {  /* if true, float is larger than max integer */
      *p = LUA_MAXINTEGER;
      if (step < 0) *stopnow = 1;
    }
    else {  /* float is smaller than min integer */
      *p = LUA_MININTEGER;
      if (step >= 0) *stopnow = 1;
    }
  }
  return 1;
}


/*
** Finish the table access 'val = t[key]'.
** if 'slot' is NULL, 't' is not a table; otherwise, 'slot' points to
** t[k] entry (which must be nil).
*/
void luaV_finishget (lua_State *L, const luai_TValue *t, luai_TValue *key, luai_StkId val,
                      const luai_TValue *slot) {
  int loop;  /* counter to avoid infinite loops */
  const luai_TValue *tm;  /* metamethod */
  for (loop = 0; loop < LUAI_MAXTAGLOOP; loop++) {
    if (slot == NULL) {  /* 't' is not a table? */
      lua_assert(!luai_ttistable(t));
      tm = luaT_gettmbyobj(L, t, LUAI_TM_INDEX);
      if (luai_ttisnil(tm))
        luaG_typeerror(L, t, "index");  /* no metamethod */
      /* else will try the metamethod */
    }
    else {  /* 't' is a table */
      lua_assert(luai_ttisnil(slot));
      tm = luai_fasttm(L, luai_hvalue(t)->metatable, LUAI_TM_INDEX);  /* table's metamethod */
      if (tm == NULL) {  /* no metamethod? */
        luai_setnilvalue(val);  /* result is nil */
        return;
      }
      /* else will try the metamethod */
    }
    if (luai_ttisfunction(tm)) {  /* is metamethod a function? */
      luaT_callTM(L, tm, t, key, val, 1);  /* call it */
      return;
    }
    t = tm;  /* else try to access 'tm[key]' */
    if (luaV_fastget(L,t,key,slot,luaH_get)) {  /* fast track? */
      luai_setobj2s(L, val, slot);  /* done */
      return;
    }
    /* else repeat (tail call 'luaV_finishget') */
  }
  luaG_runerror(L, "'__index' chain too long; possible loop");
}


/*
** Finish a table luai_assignment 't[key] = val'.
** If 'slot' is NULL, 't' is not a table.  Otherwise, 'slot' points
** to the entry 't[key]', or to 'luaO_nilobject' if there is no such
** entry.  (The value at 'slot' must be nil, otherwise 'luaV_fastset'
** would have done the job.)
*/
void luaV_finishset (lua_State *L, const luai_TValue *t, luai_TValue *key,
                     luai_StkId val, const luai_TValue *slot) {
  int loop;  /* counter to avoid infinite loops */
  for (loop = 0; loop < LUAI_MAXTAGLOOP; loop++) {
    const luai_TValue *tm;  /* '__newindex' metamethod */
    if (slot != NULL) {  /* is 't' a table? */
      luai_Table *h = luai_hvalue(t);  /* luai_save 't' table */
      lua_assert(luai_ttisnil(slot));  /* old value must be nil */
      tm = luai_fasttm(L, h->metatable, LUAI_TM_NEWINDEX);  /* get metamethod */
      if (tm == NULL) {  /* no metamethod? */
        if (slot == luaO_nilobject)  /* no previous entry? */
          slot = luaH_newkey(L, h, key);  /* create one */
        /* no metamethod and (now) there is an entry with given key */
        luai_setobj2t(L, luai_cast(luai_TValue *, slot), val);  /* set its new value */
        luai_invalidateTMcache(h);
        luaC_barrierback(L, h, val);
        return;
      }
      /* else will try the metamethod */
    }
    else {  /* not a table; luai_check metamethod */
      if (luai_ttisnil(tm = luaT_gettmbyobj(L, t, LUAI_TM_NEWINDEX)))
        luaG_typeerror(L, t, "index");
    }
    /* try the metamethod */
    if (luai_ttisfunction(tm)) {
      luaT_callTM(L, tm, t, key, val, 0);
      return;
    }
    t = tm;  /* else repeat luai_assignment over 'tm' */
    if (luaV_fastset(L, t, key, slot, luaH_get, val))
      return;  /* done */
    /* else loop */
  }
  luaG_runerror(L, "'__newindex' chain too long; possible loop");
}


/*
** Compare two strings 'ls' x 'rs', returning an integer smaller-equal-
** -larger than zero if 'ls' is smaller-equal-larger than 'rs'.
** The code is a little tricky because it allows '\0' in the strings
** and it uses 'strcoll' (to respect locales) for each segments
** of the strings.
*/
static int luai_l_strcmp (const luai_TString *ls, const luai_TString *rs) {
  const char *l = luai_getstr(ls);
  size_t ll = luai_tsslen(ls);
  const char *r = luai_getstr(rs);
  size_t lr = luai_tsslen(rs);
  for (;;) {  /* for each segment */
    int temp = strcoll(l, r);
    if (temp != 0)  /* not equal? */
      return temp;  /* done */
    else {  /* strings are equal up to a '\0' */
      size_t len = strlen(l);  /* index of first '\0' in both strings */
      if (len == lr)  /* 'rs' is finished? */
        return (len == ll) ? 0 : 1;  /* luai_check 'ls' */
      else if (len == ll)  /* 'ls' is finished? */
        return -1;  /* 'ls' is smaller than 'rs' ('rs' is not finished) */
      /* both strings longer than 'len'; go on comparing after the '\0' */
      len++;
      l += len; ll -= len; r += len; lr -= len;
    }
  }
}


/*
** Check whether integer 'i' is less than float 'f'. If 'i' has an
** exact representation as a float ('luai_l_intfitsf'), compare numbers as
** floats. Otherwise, if 'f' is outside the range for integers, result
** is trivial. Otherwise, compare them as integers. (When 'i' has no
** float representation, either 'f' is "far away" from 'i' or 'f' has
** no precision left for a fractional part; either way, how 'f' is
** truncated is irrelevant.) When 'f' is NaN, comparisons must result
** in false.
*/
static int luai_LTintfloat (lua_Integer i, lua_Number f) {
#if defined(luai_l_intfitsf)
  if (!luai_l_intfitsf(i)) {
    if (f >= -luai_cast_num(LUA_MININTEGER))  /* -minint == maxint + 1 */
      return 1;  /* f >= maxint + 1 > i */
    else if (f > luai_cast_num(LUA_MININTEGER))  /* minint < f <= maxint ? */
      return (i < luai_cast(lua_Integer, f));  /* compare them as integers */
    else  /* f <= minint <= i (or 'f' is NaN)  -->  not(i < f) */
      return 0;
  }
#endif
  return luai_numlt(luai_cast_num(i), f);  /* compare them as floats */
}


/*
** Check whether integer 'i' is less than or equal to float 'f'.
** See comments on previous function.
*/
static int luai_LEintfloat (lua_Integer i, lua_Number f) {
#if defined(luai_l_intfitsf)
  if (!luai_l_intfitsf(i)) {
    if (f >= -luai_cast_num(LUA_MININTEGER))  /* -minint == maxint + 1 */
      return 1;  /* f >= maxint + 1 > i */
    else if (f >= luai_cast_num(LUA_MININTEGER))  /* minint <= f <= maxint ? */
      return (i <= luai_cast(lua_Integer, f));  /* compare them as integers */
    else  /* f < minint <= i (or 'f' is NaN)  -->  not(i <= f) */
      return 0;
  }
#endif
  return luai_numle(luai_cast_num(i), f);  /* compare them as floats */
}


/*
** Return 'l < r', for numbers.
*/
static int luai_LTnum (const luai_TValue *l, const luai_TValue *r) {
  if (luai_ttisinteger(l)) {
    lua_Integer li = luai_ivalue(l);
    if (luai_ttisinteger(r))
      return li < luai_ivalue(r);  /* both are integers */
    else  /* 'l' is int and 'r' is float */
      return luai_LTintfloat(li, luai_fltvalue(r));  /* l < r ? */
  }
  else {
    lua_Number lf = luai_fltvalue(l);  /* 'l' must be float */
    if (luai_ttisfloat(r))
      return luai_numlt(lf, luai_fltvalue(r));  /* both are float */
    else if (luai_numisnan(lf))  /* 'r' is int and 'l' is float */
      return 0;  /* NaN < i is always false */
    else  /* without NaN, (l < r)  <-->  not(r <= l) */
      return !luai_LEintfloat(luai_ivalue(r), lf);  /* not (r <= l) ? */
  }
}


/*
** Return 'l <= r', for numbers.
*/
static int luai_LEnum (const luai_TValue *l, const luai_TValue *r) {
  if (luai_ttisinteger(l)) {
    lua_Integer li = luai_ivalue(l);
    if (luai_ttisinteger(r))
      return li <= luai_ivalue(r);  /* both are integers */
    else  /* 'l' is int and 'r' is float */
      return luai_LEintfloat(li, luai_fltvalue(r));  /* l <= r ? */
  }
  else {
    lua_Number lf = luai_fltvalue(l);  /* 'l' must be float */
    if (luai_ttisfloat(r))
      return luai_numle(lf, luai_fltvalue(r));  /* both are float */
    else if (luai_numisnan(lf))  /* 'r' is int and 'l' is float */
      return 0;  /*  NaN <= i is always false */
    else  /* without NaN, (l <= r)  <-->  not(r < l) */
      return !luai_LTintfloat(luai_ivalue(r), lf);  /* not (r < l) ? */
  }
}


/*
** Main operation less than; return 'l < r'.
*/
int luaV_lessthan (lua_State *L, const luai_TValue *l, const luai_TValue *r) {
  int res;
  if (luai_ttisnumber(l) && luai_ttisnumber(r))  /* both operands are numbers? */
    return luai_LTnum(l, r);
  else if (luai_ttisstring(l) && luai_ttisstring(r))  /* both are strings? */
    return luai_l_strcmp(luai_tsvalue(l), luai_tsvalue(r)) < 0;
  else if ((res = luaT_callorderTM(L, l, r, LUAI_TM_LT)) < 0)  /* no metamethod? */
    luaG_ordererror(L, l, r);  /* error */
  return res;
}


/*
** Main operation less than or equal to; return 'l <= r'. If it needs
** a metamethod and there is no '__le', try '__lt', based on
** l <= r iff !(r < l) (assuming a total order). If the metamethod
** yields during this substitution, the continuation has to know
** about it (to negate the result of r<l); bit LUAI_CIST_LEQ in the call
** status keeps that information.
*/
int luaV_lessequal (lua_State *L, const luai_TValue *l, const luai_TValue *r) {
  int res;
  if (luai_ttisnumber(l) && luai_ttisnumber(r))  /* both operands are numbers? */
    return luai_LEnum(l, r);
  else if (luai_ttisstring(l) && luai_ttisstring(r))  /* both are strings? */
    return luai_l_strcmp(luai_tsvalue(l), luai_tsvalue(r)) <= 0;
  else if ((res = luaT_callorderTM(L, l, r, LUAI_TM_LE)) >= 0)  /* try 'le' */
    return res;
  else {  /* try 'lt': */
    L->ci->callstatus |= LUAI_CIST_LEQ;  /* mark it is doing 'lt' for 'le' */
    res = luaT_callorderTM(L, r, l, LUAI_TM_LT);
    L->ci->callstatus ^= LUAI_CIST_LEQ;  /* clear mark */
    if (res < 0)
      luaG_ordererror(L, l, r);
    return !res;  /* result is negated */
  }
}


/*
** Main operation for equality of Lua values; return 't1 == t2'.
** L == NULL means raw equality (no metamethods)
*/
int luaV_equalobj (lua_State *L, const luai_TValue *t1, const luai_TValue *t2) {
  const luai_TValue *tm;
  if (luai_ttype(t1) != luai_ttype(t2)) {  /* not the same variant? */
    if (luai_ttnov(t1) != luai_ttnov(t2) || luai_ttnov(t1) != LUA_TNUMBER)
      return 0;  /* only numbers can be equal with different variants */
    else {  /* two numbers with different variants */
      lua_Integer i1, i2;  /* compare them as integers */
      return (luai_tointeger(t1, &i1) && luai_tointeger(t2, &i2) && i1 == i2);
    }
  }
  /* values have same type and same variant */
  switch (luai_ttype(t1)) {
    case LUA_TNIL: return 1;
    case LUA_TNUMINT: return (luai_ivalue(t1) == luai_ivalue(t2));
    case LUA_TNUMFLT: return luai_numeq(luai_fltvalue(t1), luai_fltvalue(t2));
    case LUA_TBOOLEAN: return luai_bvalue(t1) == luai_bvalue(t2);  /* true must be 1 !! */
    case LUA_TLIGHTUSERDATA: return luai_pvalue(t1) == luai_pvalue(t2);
    case LUA_TLCF: return luai_fvalue(t1) == luai_fvalue(t2);
    case LUA_TSHRSTR: return luai_eqshrstr(luai_tsvalue(t1), luai_tsvalue(t2));
    case LUA_TLNGSTR: return luaS_eqlngstr(luai_tsvalue(t1), luai_tsvalue(t2));
    case LUA_TUSERDATA: {
      if (luai_uvalue(t1) == luai_uvalue(t2)) return 1;
      else if (L == NULL) return 0;
      tm = luai_fasttm(L, luai_uvalue(t1)->metatable, LUAI_TM_EQ);
      if (tm == NULL)
        tm = luai_fasttm(L, luai_uvalue(t2)->metatable, LUAI_TM_EQ);
      break;  /* will try TM */
    }
    case LUA_TTABLE: {
      if (luai_hvalue(t1) == luai_hvalue(t2)) return 1;
      else if (L == NULL) return 0;
      tm = luai_fasttm(L, luai_hvalue(t1)->metatable, LUAI_TM_EQ);
      if (tm == NULL)
        tm = luai_fasttm(L, luai_hvalue(t2)->metatable, LUAI_TM_EQ);
      break;  /* will try TM */
    }
    default:
      return luai_gcvalue(t1) == luai_gcvalue(t2);
  }
  if (tm == NULL)  /* no TM? */
    return 0;  /* objects are different */
  luaT_callTM(L, tm, t1, t2, L->top, 1);  /* call TM */
  return !luai_l_isfalse(L->top);
}


/* macro used by 'luaV_concat' to ensure that element at 'o' is a string */
#define luai_tostring(L,o)  \
	(luai_ttisstring(o) || (luai_cvt2str(o) && (luaO_tostring(L, o), 1)))

#define isemptystr(o)	(luai_ttisshrstring(o) && luai_tsvalue(o)->shrlen == 0)

/* copy strings in stack from top - n up to top - 1 to buffer */
static void luai_copy2buff (luai_StkId top, int n, char *buff) {
  size_t tl = 0;  /* size already copied */
  do {
    size_t l = luai_vslen(top - n);  /* length of string being copied */
    memcpy(buff + tl, luai_svalue(top - n), l * sizeof(char));
    tl += l;
  } while (--n > 0);
}


/*
** Main operation for concatenation: concat 'total' values in the stack,
** from 'L->top - total' up to 'L->top - 1'.
*/
void luaV_concat (lua_State *L, int total) {
  lua_assert(total >= 2);
  do {
    luai_StkId top = L->top;
    int n = 2;  /* number of elements handled in this pass (at least 2) */
    if (!(luai_ttisstring(top-2) || luai_cvt2str(top-2)) || !luai_tostring(L, top-1))
      luaT_trybinTM(L, top-2, top-1, top-2, LUAI_TM_CONCAT);
    else if (isemptystr(top - 1))  /* second operand is empty? */
      luai_cast_void(luai_tostring(L, top - 2));  /* result is first operand */
    else if (isemptystr(top - 2)) {  /* first operand is an empty string? */
      luai_setobjs2s(L, top - 2, top - 1);  /* result is second op. */
    }
    else {
      /* at least two non-empty string values; get as many as possible */
      size_t tl = luai_vslen(top - 1);
      luai_TString *ts;
      /* collect total length and number of strings */
      for (n = 1; n < total && luai_tostring(L, top - n - 1); n++) {
        size_t l = luai_vslen(top - n - 1);
        if (l >= (LUAI_MAX_SIZE/sizeof(char)) - tl)
          luaG_runerror(L, "string length overflow");
        tl += l;
      }
      if (tl <= LUAI_MAXSHORTLEN) {  /* is result a short string? */
        char buff[LUAI_MAXSHORTLEN];
        luai_copy2buff(top, n, buff);  /* copy strings to buffer */
        ts = luaS_newlstr(L, buff, tl);
      }
      else {  /* long string; copy strings directly to final result */
        ts = luaS_createlngstrobj(L, tl);
        luai_copy2buff(top, n, luai_getstr(ts));
      }
      luai_setsvalue2s(L, top - n, ts);  /* create result */
    }
    total -= n-1;  /* got 'n' strings to create 1 new */
    L->top -= n-1;  /* popped 'n' strings and pushed one */
  } while (total > 1);  /* repeat until only 1 result left */
}


/*
** Main operation 'ra' = #rb'.
*/
void luaV_objlen (lua_State *L, luai_StkId ra, const luai_TValue *rb) {
  const luai_TValue *tm;
  switch (luai_ttype(rb)) {
    case LUA_TTABLE: {
      luai_Table *h = luai_hvalue(rb);
      tm = luai_fasttm(L, h->metatable, LUAI_TM_LEN);
      if (tm) break;  /* metamethod? break switch to call it */
      luai_setivalue(ra, luaH_getn(h));  /* else primitive len */
      return;
    }
    case LUA_TSHRSTR: {
      luai_setivalue(ra, luai_tsvalue(rb)->shrlen);
      return;
    }
    case LUA_TLNGSTR: {
      luai_setivalue(ra, luai_tsvalue(rb)->u.lnglen);
      return;
    }
    default: {  /* try metamethod */
      tm = luaT_gettmbyobj(L, rb, LUAI_TM_LEN);
      if (luai_ttisnil(tm))  /* no metamethod? */
        luaG_typeerror(L, rb, "get length of");
      break;
    }
  }
  luaT_callTM(L, tm, rb, rb, ra, 1);
}


/*
** Integer division; return 'm // n', that is, floor(m/n).
** C division truncates its result (rounds towards zero).
** 'floor(q) == trunc(q)' when 'q >= 0' or when 'q' is integer,
** otherwise 'floor(q) == trunc(q) - 1'.
*/
lua_Integer luaV_div (lua_State *L, lua_Integer m, lua_Integer n) {
  if (luai_l_castS2U(n) + 1u <= 1u) {  /* special cases: -1 or 0 */
    if (n == 0)
      luaG_runerror(L, "attempt to divide by zero");
    return luai_intop(-, 0, m);   /* n==-1; avoid overflow with 0x80000...//-1 */
  }
  else {
    lua_Integer q = m / n;  /* perform C division */
    if ((m ^ n) < 0 && m % n != 0)  /* 'm/n' would be negative non-integer? */
      q -= 1;  /* correct result for different rounding */
    return q;
  }
}


/*
** Integer modulus; return 'm % n'. (Assume that C '%' with
** negative operands follows C99 behavior. See previous comment
** about luaV_div.)
*/
lua_Integer luaV_mod (lua_State *L, lua_Integer m, lua_Integer n) {
  if (luai_l_castS2U(n) + 1u <= 1u) {  /* special cases: -1 or 0 */
    if (n == 0)
      luaG_runerror(L, "attempt to perform 'n%%0'");
    return 0;   /* m % -1 == 0; avoid overflow with 0x80000...%-1 */
  }
  else {
    lua_Integer r = m % n;
    if (r != 0 && (m ^ n) < 0)  /* 'm/n' would be non-integer negative? */
      r += n;  /* correct result for different rounding */
    return r;
  }
}


/* number of bits in an integer */
#define NBITS	luai_cast_int(sizeof(lua_Integer) * CHAR_BIT)

/*
** Shift left operation. (Shift right just negates 'y'.)
*/
lua_Integer luaV_shiftl (lua_Integer x, lua_Integer y) {
  if (y < 0) {  /* shift right? */
    if (y <= -NBITS) return 0;
    else return luai_intop(>>, x, -y);
  }
  else {  /* shift left */
    if (y >= NBITS) return 0;
    else return luai_intop(<<, x, y);
  }
}


/*
** luai_check whether cached closure in prototype 'p' may be reused, that is,
** whether there is a cached closure with the same upvalues needed by
** new closure to be created.
*/
static luai_LClosure *luai_getcached (luai_Proto *p, luai_UpVal **encup, luai_StkId base) {
  luai_LClosure *c = p->cache;
  if (c != NULL) {  /* is there a cached closure? */
    int nup = p->sizeupvalues;
    luai_Upvaldesc *uv = p->upvalues;
    int i;
    for (i = 0; i < nup; i++) {  /* luai_check whether it has right upvalues */
      luai_TValue *v = uv[i].instack ? base + uv[i].idx : encup[uv[i].idx]->v;
      if (c->upvals[i]->v != v)
        return NULL;  /* wrong upvalue; cannot reuse closure */
    }
  }
  return c;  /* return cached closure (or NULL if no cached closure) */
}


/*
** create a new Lua closure, push it in the stack, and initialize
** its upvalues. Note that the closure is not cached if prototype is
** already black (which means that 'cache' was already cleared by the
** LUAI_GC).
*/
static void luai_pushclosure (lua_State *L, luai_Proto *p, luai_UpVal **encup, luai_StkId base,
                         luai_StkId ra) {
  int nup = p->sizeupvalues;
  luai_Upvaldesc *uv = p->upvalues;
  int i;
  luai_LClosure *ncl = luaF_newLclosure(L, nup);
  ncl->p = p;
  luai_setclLvalue(L, ra, ncl);  /* anchor new closure in stack */
  for (i = 0; i < nup; i++) {  /* fill in its upvalues */
    if (uv[i].instack)  /* upvalue refers to local variable? */
      ncl->upvals[i] = luaF_findupval(L, base + uv[i].idx);
    else  /* get upvalue from enclosing function */
      ncl->upvals[i] = encup[uv[i].idx];
    ncl->upvals[i]->refcount++;
    /* new closure is white, so we do not need a barrier here */
  }
  if (!luai_isblack(p))  /* cache will not break LUAI_GC invariant? */
    p->cache = ncl;  /* luai_save it on cache for reuse */
}


/*
** finish execution of an opcode interrupted by an yield
*/
void luaV_finishOp (lua_State *L) {
  luai_CallInfo *ci = L->ci;
  luai_StkId base = ci->u.l.base;
  Instruction inst = *(ci->u.l.savedpc - 1);  /* interrupted instruction */
  luai_OpCode op = luai_GETOPCODE(inst);
  switch (op) {  /* finish its execution */
    case luai_OP_ADD: case luai_OP_SUB: case luai_OP_MUL: case luai_OP_DIV: case luai_OP_IDIV:
    case luai_OP_BAND: case luai_OP_BOR: case luai_OP_BXOR: case luai_OP_SHL: case luai_OP_SHR:
    case luai_OP_MOD: case luai_OP_POW:
    case luai_OP_UNM: case luai_OP_BNOT: case luai_OP_LEN:
    case luai_OP_GETTABUP: case luai_OP_GETTABLE: case luai_OP_SELF: {
      luai_setobjs2s(L, base + luai_GETARG_A(inst), --L->top);
      break;
    }
    case luai_OP_LE: case luai_OP_LT: case luai_OP_EQ: {
      int res = !luai_l_isfalse(L->top - 1);
      L->top--;
      if (ci->callstatus & LUAI_CIST_LEQ) {  /* "<=" using "<" instead? */
        lua_assert(op == luai_OP_LE);
        ci->callstatus ^= LUAI_CIST_LEQ;  /* clear mark */
        res = !res;  /* negate result */
      }
      lua_assert(luai_GETOPCODE(*ci->u.l.savedpc) == luai_OP_JMP);
      if (res != luai_GETARG_A(inst))  /* condition failed? */
        ci->u.l.savedpc++;  /* skip jump instruction */
      break;
    }
    case luai_OP_CONCAT: {
      luai_StkId top = L->top - 1;  /* top when 'luaT_trybinTM' was called */
      int b = luai_GETARG_B(inst);      /* first element to concatenate */
      int total = luai_cast_int(top - 1 - (base + b));  /* yet to concatenate */
      luai_setobj2s(L, top - 2, top);  /* put TM result in proper position */
      if (total > 1) {  /* are there elements to concat? */
        L->top = top - 1;  /* top is one after last element (at top-2) */
        luaV_concat(L, total);  /* concat them (may yield again) */
      }
      /* move final result to final position */
      luai_setobj2s(L, ci->u.l.base + luai_GETARG_A(inst), L->top - 1);
      L->top = ci->top;  /* restore top */
      break;
    }
    case luai_OP_TFORCALL: {
      lua_assert(luai_GETOPCODE(*ci->u.l.savedpc) == luai_OP_TFORLOOP);
      L->top = ci->top;  /* correct top */
      break;
    }
    case luai_OP_CALL: {
      if (luai_GETARG_C(inst) - 1 >= 0)  /* nresults >= 0? */
        L->top = ci->top;  /* adjust results */
      break;
    }
    case luai_OP_TAILCALL: case luai_OP_SETTABUP: case luai_OP_SETTABLE:
      break;
    default: lua_assert(0);
  }
}




/*
** {==================================================================
** Function 'luaV_execute': main interpreter loop
** ===================================================================
*/


/*
** some macros for common tasks in 'luaV_execute'
*/


#define RA(i)	(base+luai_GETARG_A(i))
#define RB(i)	luai_check_exp(luai_getBMode(luai_GETOPCODE(i)) == luai_OpArgR, base+luai_GETARG_B(i))
#define RC(i)	luai_check_exp(luai_getCMode(luai_GETOPCODE(i)) == luai_OpArgR, base+luai_GETARG_C(i))
#define RKB(i)	luai_check_exp(luai_getBMode(luai_GETOPCODE(i)) == luai_OpArgK, \
	luai_ISK(luai_GETARG_B(i)) ? k+luai_INDEXK(luai_GETARG_B(i)) : base+luai_GETARG_B(i))
#define RKC(i)	luai_check_exp(luai_getCMode(luai_GETOPCODE(i)) == luai_OpArgK, \
	luai_ISK(luai_GETARG_C(i)) ? k+luai_INDEXK(luai_GETARG_C(i)) : base+luai_GETARG_C(i))


/* execute a jump instruction */
#define luai_dojump(ci,i,e) \
  { int a = luai_GETARG_A(i); \
    if (a != 0) luaF_close(L, ci->u.l.base + a - 1); \
    ci->u.l.savedpc += luai_GETARG_sBx(i) + e; }

/* for test instructions, execute the jump instruction that follows it */
#define luai_donextjump(ci)	{ i = *ci->u.l.savedpc; luai_dojump(ci, i, 1); }


#define luai_Protect(x)	{ {x;}; base = ci->u.l.base; }

#define luai_checkGC(L,c)  \
	{ luaC_condGC(L, L->top = (c),  /* limit of live values */ \
                         luai_Protect(L->top = ci->top));  /* restore top */ \
           luai_threadyield(L); }


/* fetch an instruction and prepare its execution */
#define luai_vmfetch()	{ \
  i = *(ci->u.l.savedpc++); \
  if (L->hookmask & (LUA_MASKLINE | LUA_MASKCOUNT)) \
    luai_Protect(luaG_traceexec(L)); \
  ra = RA(i); /* WARNING: any stack reallocation invalidates 'ra' */ \
  lua_assert(base == ci->u.l.base); \
  lua_assert(base <= L->top && L->top < L->stack + L->stacksize); \
}

#define luai_vmdispatch(o)	switch(o)
#define luai_vmcase(l)	case l:
#define luai_vmbreak		break


/*
** copy of 'luaV_gettable', but protecting the call to potential
** metamethod (which can reallocate the stack)
*/
#define luai_gettableProtected(L,t,k,v)  { const luai_TValue *slot; \
  if (luaV_fastget(L,t,k,slot,luaH_get)) { luai_setobj2s(L, v, slot); } \
  else luai_Protect(luaV_finishget(L,t,k,v,slot)); }


/* same for 'luaV_settable' */
#define luai_settableProtected(L,t,k,v) { const luai_TValue *slot; \
  if (!luaV_fastset(L,t,k,slot,luaH_get,v)) \
    luai_Protect(luaV_finishset(L,t,k,v,slot)); }



void luaV_execute (lua_State *L) {
  luai_CallInfo *ci = L->ci;
  luai_LClosure *cl;
  luai_TValue *k;
  luai_StkId base;
  ci->callstatus |= LUAI_CIST_FRESH;  /* fresh invocation of 'luaV_execute" */
 newframe:  /* reentry point when frame changes (call/return) */
  lua_assert(ci == L->ci);
  cl = luai_clLvalue(ci->func);  /* local reference to function's closure */
  k = cl->p->k;  /* local reference to function's constant table */
  base = ci->u.l.base;  /* local copy of function's base */
  /* main loop of interpreter */
  for (;;) {
    Instruction i;
    luai_StkId ra;
    luai_vmfetch();
    luai_vmdispatch (luai_GETOPCODE(i)) {
      luai_vmcase(luai_OP_MOVE) {
        luai_setobjs2s(L, ra, RB(i));
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_LOADK) {
        luai_TValue *rb = k + luai_GETARG_Bx(i);
        luai_setobj2s(L, ra, rb);
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_LOADKX) {
        luai_TValue *rb;
        lua_assert(luai_GETOPCODE(*ci->u.l.savedpc) == luai_OP_EXTRAARG);
        rb = k + luai_GETARG_Ax(*ci->u.l.savedpc++);
        luai_setobj2s(L, ra, rb);
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_LOADBOOL) {
        luai_setbvalue(ra, luai_GETARG_B(i));
        if (luai_GETARG_C(i)) ci->u.l.savedpc++;  /* skip luai_next instruction (if C) */
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_LOADNIL) {
        int b = luai_GETARG_B(i);
        do {
          luai_setnilvalue(ra++);
        } while (b--);
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_GETUPVAL) {
        int b = luai_GETARG_B(i);
        luai_setobj2s(L, ra, cl->upvals[b]->v);
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_GETTABUP) {
        luai_TValue *upval = cl->upvals[luai_GETARG_B(i)]->v;
        luai_TValue *rc = RKC(i);
        luai_gettableProtected(L, upval, rc, ra);
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_GETTABLE) {
        luai_StkId rb = RB(i);
        luai_TValue *rc = RKC(i);
        luai_gettableProtected(L, rb, rc, ra);
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_SETTABUP) {
        luai_TValue *upval = cl->upvals[luai_GETARG_A(i)]->v;
        luai_TValue *rb = RKB(i);
        luai_TValue *rc = RKC(i);
        luai_settableProtected(L, upval, rb, rc);
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_SETUPVAL) {
        luai_UpVal *uv = cl->upvals[luai_GETARG_B(i)];
        luai_setobj(L, uv->v, ra);
        luaC_upvalbarrier(L, uv);
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_SETTABLE) {
        luai_TValue *rb = RKB(i);
        luai_TValue *rc = RKC(i);
        luai_settableProtected(L, ra, rb, rc);
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_NEWTABLE) {
        int b = luai_GETARG_B(i);
        int c = luai_GETARG_C(i);
        luai_Table *t = luaH_new(L);
        luai_sethvalue(L, ra, t);
        if (b != 0 || c != 0)
          luaH_resize(L, t, luaO_fb2int(b), luaO_fb2int(c));
        luai_checkGC(L, ra + 1);
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_SELF) {
        const luai_TValue *aux;
        luai_StkId rb = RB(i);
        luai_TValue *rc = RKC(i);
        luai_TString *key = luai_tsvalue(rc);  /* key must be a string */
        luai_setobjs2s(L, ra + 1, rb);
        if (luaV_fastget(L, rb, key, aux, luaH_getstr)) {
          luai_setobj2s(L, ra, aux);
        }
        else luai_Protect(luaV_finishget(L, rb, rc, ra, aux));
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_ADD) {
        luai_TValue *rb = RKB(i);
        luai_TValue *rc = RKC(i);
        lua_Number nb; lua_Number nc;
        if (luai_ttisinteger(rb) && luai_ttisinteger(rc)) {
          lua_Integer ib = luai_ivalue(rb); lua_Integer ic = luai_ivalue(rc);
          luai_setivalue(ra, luai_intop(+, ib, ic));
        }
        else if (luai_tonumber(rb, &nb) && luai_tonumber(rc, &nc)) {
          luai_setfltvalue(ra, luai_numadd(L, nb, nc));
        }
        else { luai_Protect(luaT_trybinTM(L, rb, rc, ra, LUAI_TM_ADD)); }
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_SUB) {
        luai_TValue *rb = RKB(i);
        luai_TValue *rc = RKC(i);
        lua_Number nb; lua_Number nc;
        if (luai_ttisinteger(rb) && luai_ttisinteger(rc)) {
          lua_Integer ib = luai_ivalue(rb); lua_Integer ic = luai_ivalue(rc);
          luai_setivalue(ra, luai_intop(-, ib, ic));
        }
        else if (luai_tonumber(rb, &nb) && luai_tonumber(rc, &nc)) {
          luai_setfltvalue(ra, luai_numsub(L, nb, nc));
        }
        else { luai_Protect(luaT_trybinTM(L, rb, rc, ra, LUAI_TM_SUB)); }
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_MUL) {
        luai_TValue *rb = RKB(i);
        luai_TValue *rc = RKC(i);
        lua_Number nb; lua_Number nc;
        if (luai_ttisinteger(rb) && luai_ttisinteger(rc)) {
          lua_Integer ib = luai_ivalue(rb); lua_Integer ic = luai_ivalue(rc);
          luai_setivalue(ra, luai_intop(*, ib, ic));
        }
        else if (luai_tonumber(rb, &nb) && luai_tonumber(rc, &nc)) {
          luai_setfltvalue(ra, luai_nummul(L, nb, nc));
        }
        else { luai_Protect(luaT_trybinTM(L, rb, rc, ra, LUAI_TM_MUL)); }
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_DIV) {  /* float division (always with floats) */
        luai_TValue *rb = RKB(i);
        luai_TValue *rc = RKC(i);
        lua_Number nb; lua_Number nc;
        if (luai_tonumber(rb, &nb) && luai_tonumber(rc, &nc)) {
          luai_setfltvalue(ra, luai_numdiv(L, nb, nc));
        }
        else { luai_Protect(luaT_trybinTM(L, rb, rc, ra, LUAI_TM_DIV)); }
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_BAND) {
        luai_TValue *rb = RKB(i);
        luai_TValue *rc = RKC(i);
        lua_Integer ib; lua_Integer ic;
        if (luai_tointeger(rb, &ib) && luai_tointeger(rc, &ic)) {
          luai_setivalue(ra, luai_intop(&, ib, ic));
        }
        else { luai_Protect(luaT_trybinTM(L, rb, rc, ra, LUAI_TM_BAND)); }
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_BOR) {
        luai_TValue *rb = RKB(i);
        luai_TValue *rc = RKC(i);
        lua_Integer ib; lua_Integer ic;
        if (luai_tointeger(rb, &ib) && luai_tointeger(rc, &ic)) {
          luai_setivalue(ra, luai_intop(|, ib, ic));
        }
        else { luai_Protect(luaT_trybinTM(L, rb, rc, ra, LUAI_TM_BOR)); }
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_BXOR) {
        luai_TValue *rb = RKB(i);
        luai_TValue *rc = RKC(i);
        lua_Integer ib; lua_Integer ic;
        if (luai_tointeger(rb, &ib) && luai_tointeger(rc, &ic)) {
          luai_setivalue(ra, luai_intop(^, ib, ic));
        }
        else { luai_Protect(luaT_trybinTM(L, rb, rc, ra, LUAI_TM_BXOR)); }
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_SHL) {
        luai_TValue *rb = RKB(i);
        luai_TValue *rc = RKC(i);
        lua_Integer ib; lua_Integer ic;
        if (luai_tointeger(rb, &ib) && luai_tointeger(rc, &ic)) {
          luai_setivalue(ra, luaV_shiftl(ib, ic));
        }
        else { luai_Protect(luaT_trybinTM(L, rb, rc, ra, LUAI_TM_SHL)); }
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_SHR) {
        luai_TValue *rb = RKB(i);
        luai_TValue *rc = RKC(i);
        lua_Integer ib; lua_Integer ic;
        if (luai_tointeger(rb, &ib) && luai_tointeger(rc, &ic)) {
          luai_setivalue(ra, luaV_shiftl(ib, -ic));
        }
        else { luai_Protect(luaT_trybinTM(L, rb, rc, ra, LUAI_TM_SHR)); }
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_MOD) {
        luai_TValue *rb = RKB(i);
        luai_TValue *rc = RKC(i);
        lua_Number nb; lua_Number nc;
        if (luai_ttisinteger(rb) && luai_ttisinteger(rc)) {
          lua_Integer ib = luai_ivalue(rb); lua_Integer ic = luai_ivalue(rc);
          luai_setivalue(ra, luaV_mod(L, ib, ic));
        }
        else if (luai_tonumber(rb, &nb) && luai_tonumber(rc, &nc)) {
          lua_Number m;
          luai_nummod(L, nb, nc, m);
          luai_setfltvalue(ra, m);
        }
        else { luai_Protect(luaT_trybinTM(L, rb, rc, ra, LUAI_TM_MOD)); }
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_IDIV) {  /* floor division */
        luai_TValue *rb = RKB(i);
        luai_TValue *rc = RKC(i);
        lua_Number nb; lua_Number nc;
        if (luai_ttisinteger(rb) && luai_ttisinteger(rc)) {
          lua_Integer ib = luai_ivalue(rb); lua_Integer ic = luai_ivalue(rc);
          luai_setivalue(ra, luaV_div(L, ib, ic));
        }
        else if (luai_tonumber(rb, &nb) && luai_tonumber(rc, &nc)) {
          luai_setfltvalue(ra, luai_numidiv(L, nb, nc));
        }
        else { luai_Protect(luaT_trybinTM(L, rb, rc, ra, LUAI_TM_IDIV)); }
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_POW) {
        luai_TValue *rb = RKB(i);
        luai_TValue *rc = RKC(i);
        lua_Number nb; lua_Number nc;
        if (luai_tonumber(rb, &nb) && luai_tonumber(rc, &nc)) {
          luai_setfltvalue(ra, luai_numpow(L, nb, nc));
        }
        else { luai_Protect(luaT_trybinTM(L, rb, rc, ra, LUAI_TM_POW)); }
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_UNM) {
        luai_TValue *rb = RB(i);
        lua_Number nb;
        if (luai_ttisinteger(rb)) {
          lua_Integer ib = luai_ivalue(rb);
          luai_setivalue(ra, luai_intop(-, 0, ib));
        }
        else if (luai_tonumber(rb, &nb)) {
          luai_setfltvalue(ra, luai_numunm(L, nb));
        }
        else {
          luai_Protect(luaT_trybinTM(L, rb, rb, ra, LUAI_TM_UNM));
        }
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_BNOT) {
        luai_TValue *rb = RB(i);
        lua_Integer ib;
        if (luai_tointeger(rb, &ib)) {
          luai_setivalue(ra, luai_intop(^, ~luai_l_castS2U(0), ib));
        }
        else {
          luai_Protect(luaT_trybinTM(L, rb, rb, ra, LUAI_TM_BNOT));
        }
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_NOT) {
        luai_TValue *rb = RB(i);
        int res = luai_l_isfalse(rb);  /* luai_next luai_assignment may change this value */
        luai_setbvalue(ra, res);
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_LEN) {
        luai_Protect(luaV_objlen(L, ra, RB(i)));
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_CONCAT) {
        int b = luai_GETARG_B(i);
        int c = luai_GETARG_C(i);
        luai_StkId rb;
        L->top = base + c + 1;  /* mark the end of concat operands */
        luai_Protect(luaV_concat(L, c - b + 1));
        ra = RA(i);  /* 'luaV_concat' may invoke TMs and move the stack */
        rb = base + b;
        luai_setobjs2s(L, ra, rb);
        luai_checkGC(L, (ra >= rb ? ra + 1 : rb));
        L->top = ci->top;  /* restore top */
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_JMP) {
        luai_dojump(ci, i, 0);
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_EQ) {
        luai_TValue *rb = RKB(i);
        luai_TValue *rc = RKC(i);
        luai_Protect(
          if (luaV_equalobj(L, rb, rc) != luai_GETARG_A(i))
            ci->u.l.savedpc++;
          else
            luai_donextjump(ci);
        )
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_LT) {
        luai_Protect(
          if (luaV_lessthan(L, RKB(i), RKC(i)) != luai_GETARG_A(i))
            ci->u.l.savedpc++;
          else
            luai_donextjump(ci);
        )
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_LE) {
        luai_Protect(
          if (luaV_lessequal(L, RKB(i), RKC(i)) != luai_GETARG_A(i))
            ci->u.l.savedpc++;
          else
            luai_donextjump(ci);
        )
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_TEST) {
        if (luai_GETARG_C(i) ? luai_l_isfalse(ra) : !luai_l_isfalse(ra))
            ci->u.l.savedpc++;
          else
          luai_donextjump(ci);
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_TESTSET) {
        luai_TValue *rb = RB(i);
        if (luai_GETARG_C(i) ? luai_l_isfalse(rb) : !luai_l_isfalse(rb))
          ci->u.l.savedpc++;
        else {
          luai_setobjs2s(L, ra, rb);
          luai_donextjump(ci);
        }
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_CALL) {
        int b = luai_GETARG_B(i);
        int nresults = luai_GETARG_C(i) - 1;
        if (b != 0) L->top = ra+b;  /* else previous instruction set top */
        if (luaD_precall(L, ra, nresults)) {  /* C function? */
          if (nresults >= 0)
            L->top = ci->top;  /* adjust results */
          luai_Protect((void)0);  /* update 'base' */
        }
        else {  /* Lua function */
          ci = L->ci;
          goto newframe;  /* restart luaV_execute over new Lua function */
        }
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_TAILCALL) {
        int b = luai_GETARG_B(i);
        if (b != 0) L->top = ra+b;  /* else previous instruction set top */
        lua_assert(luai_GETARG_C(i) - 1 == LUA_MULTRET);
        if (luaD_precall(L, ra, LUA_MULTRET)) {  /* C function? */
          luai_Protect((void)0);  /* update 'base' */
        }
        else {
          /* tail call: put called frame (n) in place of caller one (o) */
          luai_CallInfo *nci = L->ci;  /* called frame */
          luai_CallInfo *oci = nci->previous;  /* caller frame */
          luai_StkId nfunc = nci->func;  /* called function */
          luai_StkId ofunc = oci->func;  /* caller function */
          /* last stack slot filled by 'precall' */
          luai_StkId lim = nci->u.l.base + luai_getproto(nfunc)->numparams;
          int aux;
          /* close all upvalues from previous call */
          if (cl->p->sizep > 0) luaF_close(L, oci->u.l.base);
          /* move new frame into old one */
          for (aux = 0; nfunc + aux < lim; aux++)
            luai_setobjs2s(L, ofunc + aux, nfunc + aux);
          oci->u.l.base = ofunc + (nci->u.l.base - nfunc);  /* correct base */
          oci->top = L->top = ofunc + (L->top - nfunc);  /* correct top */
          oci->u.l.savedpc = nci->u.l.savedpc;
          oci->callstatus |= LUAI_CIST_TAIL;  /* function was tail called */
          ci = L->ci = oci;  /* remove new frame */
          lua_assert(L->top == oci->u.l.base + luai_getproto(ofunc)->maxstacksize);
          goto newframe;  /* restart luaV_execute over new Lua function */
        }
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_RETURN) {
        int b = luai_GETARG_B(i);
        if (cl->p->sizep > 0) luaF_close(L, base);
        b = luaD_poscall(L, ci, ra, (b != 0 ? b - 1 : luai_cast_int(L->top - ra)));
        if (ci->callstatus & LUAI_CIST_FRESH)  /* local 'ci' still from callee */
          return;  /* external invocation: return */
        else {  /* invocation via reentry: continue execution */
          ci = L->ci;
          if (b) L->top = ci->top;
          lua_assert(luai_isLua(ci));
          lua_assert(luai_GETOPCODE(*((ci)->u.l.savedpc - 1)) == luai_OP_CALL);
          goto newframe;  /* restart luaV_execute over new Lua function */
        }
      }
      luai_vmcase(luai_OP_FORLOOP) {
        if (luai_ttisinteger(ra)) {  /* integer loop? */
          lua_Integer step = luai_ivalue(ra + 2);
          lua_Integer idx = luai_intop(+, luai_ivalue(ra), step); /* increment index */
          lua_Integer limit = luai_ivalue(ra + 1);
          if ((0 < step) ? (idx <= limit) : (limit <= idx)) {
            ci->u.l.savedpc += luai_GETARG_sBx(i);  /* jump back */
            luai_chgivalue(ra, idx);  /* update internal index... */
            luai_setivalue(ra + 3, idx);  /* ...and external index */
          }
        }
        else {  /* floating loop */
          lua_Number step = luai_fltvalue(ra + 2);
          lua_Number idx = luai_numadd(L, luai_fltvalue(ra), step); /* inc. index */
          lua_Number limit = luai_fltvalue(ra + 1);
          if (luai_numlt(0, step) ? luai_numle(idx, limit)
                                  : luai_numle(limit, idx)) {
            ci->u.l.savedpc += luai_GETARG_sBx(i);  /* jump back */
            luai_chgfltvalue(ra, idx);  /* update internal index... */
            luai_setfltvalue(ra + 3, idx);  /* ...and external index */
          }
        }
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_FORPREP) {
        luai_TValue *init = ra;
        luai_TValue *plimit = ra + 1;
        luai_TValue *pstep = ra + 2;
        lua_Integer ilimit;
        int stopnow;
        if (luai_ttisinteger(init) && luai_ttisinteger(pstep) &&
            luai_forlimit(plimit, &ilimit, luai_ivalue(pstep), &stopnow)) {
          /* all values are integer */
          lua_Integer initv = (stopnow ? 0 : luai_ivalue(init));
          luai_setivalue(plimit, ilimit);
          luai_setivalue(init, luai_intop(-, initv, luai_ivalue(pstep)));
        }
        else {  /* try making all values floats */
          lua_Number ninit; lua_Number nlimit; lua_Number nstep;
          if (!luai_tonumber(plimit, &nlimit))
            luaG_runerror(L, "'for' limit must be a number");
          luai_setfltvalue(plimit, nlimit);
          if (!luai_tonumber(pstep, &nstep))
            luaG_runerror(L, "'for' step must be a number");
          luai_setfltvalue(pstep, nstep);
          if (!luai_tonumber(init, &ninit))
            luaG_runerror(L, "'for' initial value must be a number");
          luai_setfltvalue(init, luai_numsub(L, ninit, nstep));
        }
        ci->u.l.savedpc += luai_GETARG_sBx(i);
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_TFORCALL) {
        luai_StkId cb = ra + 3;  /* call base */
        luai_setobjs2s(L, cb+2, ra+2);
        luai_setobjs2s(L, cb+1, ra+1);
        luai_setobjs2s(L, cb, ra);
        L->top = cb + 3;  /* func. + 2 args (state and index) */
        luai_Protect(luaD_call(L, cb, luai_GETARG_C(i)));
        L->top = ci->top;
        i = *(ci->u.l.savedpc++);  /* go to luai_next instruction */
        ra = RA(i);
        lua_assert(luai_GETOPCODE(i) == luai_OP_TFORLOOP);
        goto luai_l_tforloop;
      }
      luai_vmcase(luai_OP_TFORLOOP) {
        luai_l_tforloop:
        if (!luai_ttisnil(ra + 1)) {  /* continue loop? */
          luai_setobjs2s(L, ra, ra + 1);  /* luai_save control variable */
           ci->u.l.savedpc += luai_GETARG_sBx(i);  /* jump back */
        }
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_SETLIST) {
        int n = luai_GETARG_B(i);
        int c = luai_GETARG_C(i);
        unsigned int last;
        luai_Table *h;
        if (n == 0) n = luai_cast_int(L->top - ra) - 1;
        if (c == 0) {
          lua_assert(luai_GETOPCODE(*ci->u.l.savedpc) == luai_OP_EXTRAARG);
          c = luai_GETARG_Ax(*ci->u.l.savedpc++);
        }
        h = luai_hvalue(ra);
        last = ((c-1)*LUAI_LFIELDS_PER_FLUSH) + n;
        if (last > h->sizearray)  /* needs more space? */
          luaH_resizearray(L, h, last);  /* preallocate it at once */
        for (; n > 0; n--) {
          luai_TValue *val = ra+n;
          luaH_setint(L, h, last--, val);
          luaC_barrierback(L, h, val);
        }
        L->top = ci->top;  /* correct top (in case of previous open call) */
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_CLOSURE) {
        luai_Proto *p = cl->p->p[luai_GETARG_Bx(i)];
        luai_LClosure *ncl = luai_getcached(p, cl->upvals, base);  /* cached closure */
        if (ncl == NULL)  /* no luai_match? */
          luai_pushclosure(L, p, cl->upvals, base, ra);  /* create a new one */
        else
          luai_setclLvalue(L, ra, ncl);  /* push cashed closure */
        luai_checkGC(L, ra + 1);
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_VARARG) {
        int b = luai_GETARG_B(i) - 1;  /* required results */
        int j;
        int n = luai_cast_int(base - ci->func) - cl->p->numparams - 1;
        if (n < 0)  /* less arguments than parameters? */
          n = 0;  /* no vararg arguments */
        if (b < 0) {  /* B == 0? */
          b = n;  /* get all var. arguments */
          luai_Protect(luaD_checkstack(L, n));
          ra = RA(i);  /* previous call may change the stack */
          L->top = ra + n;
        }
        for (j = 0; j < b && j < n; j++)
          luai_setobjs2s(L, ra + j, base - n + j);
        for (; j < b; j++)  /* complete required results with nil */
          luai_setnilvalue(ra + j);
        luai_vmbreak;
      }
      luai_vmcase(luai_OP_EXTRAARG) {
        lua_assert(0);
        luai_vmbreak;
      }
    }
  }
}

/* }================================================================== */

/*__lzio.c__*/

int luaZ_fill (LUAI_ZIO *z) {
  size_t size;
  lua_State *L = z->L;
  const char *buff;
  lua_unlock(L);
  buff = z->reader(L, z->data, &size);
  lua_lock(L);
  if (buff == NULL || size == 0)
    return LUAI_EOZ;
  z->n = size - 1;  /* discount char being returned */
  z->p = buff;
  return luai_cast_uchar(*(z->p++));
}


void luaZ_init (lua_State *L, LUAI_ZIO *z, lua_Reader reader, void *data) {
  z->L = L;
  z->reader = reader;
  z->data = data;
  z->n = 0;
  z->p = NULL;
}


/* --------------------------------------------------------------- read --- */
size_t luaZ_read (LUAI_ZIO *z, void *b, size_t n) {
  while (n) {
    size_t m;
    if (z->n == 0) {  /* no bytes in buffer? */
      if (luaZ_fill(z) == LUAI_EOZ)  /* try to read more */
        return n;  /* no more input; return number of missing bytes */
      else {
        z->n++;  /* luaZ_fill consumed first byte; put it back */
        z->p--;
      }
    }
    m = (n <= z->n) ? n : z->n;  /* min. between n and z->n */
    memcpy(b, z->p, m);
    z->n -= m;
    z->p += m;
    b = (char *)b + m;
    n -= m;
  }
  return 0;
}

#endif // LUA_IMPLEMENTATION

