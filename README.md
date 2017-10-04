# l65
[![Build Status](https://travis-ci.org/g012/l65.svg?branch=master)](https://travis-ci.org/g012/l65)

l65 is a 6502 assembler, operating from within Lua and written in Lua. This means assembler mnemonics become regular Lua statements anywhere in the middle of Lua code.

## Building

Use CMake to build a standalone executable. Following are basic instructions if you've never used CMake.

### Windows

32b version:
```
mkdir build\win32
cd build\win32
cmake -G "Visual Studio 15 2017" ..\..
cd ..
cmake --build win32 --config Release
```

64b version:
```
mkdir build\win64
cd build\win64
cmake -G "Visual Studio 15 2017 Win64" ..\..
cd ..
cmake --build win64 --config Release
```

### Linux

```
mkdir -p build/linux
cd build/linux
cmake ../.. -DCMAKE_BUILD_TYPE=Release
make
```

Force 32b build on 64b system:
```
mkdir -p build/linux32
cd build/linux32
cmake ../.. -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS=-m32
make
```

## Vim files installation

 * copy vim/* into ~/vimfiles/
 * add `au BufRead,BufNewFile *.l65 set syntax=l65 filetype=l65` to ~/.vimrc (or ~/_vimrc on Windows)

## Credits

Developed by g012, using:
 * [Lua 5.3.4](https://www.lua.org)
 * [LuaMinify](https://github.com/stravant/LuaMinify)
 * Lua syntax file shipping with [vim](http://www.vim.org)
 * Lua indent file from [vim-lua](https://github.com/tbastos/vim-lua)
 * [stb_image](https://github.com/nothings/stb)

Not using, but integrated for end-user convenience:
 * [LuaFileSystem](https://keplerproject.github.io/luafilesystem)
 * [LPeg](http://www.inf.puc-rio.br/~roberto/lpeg)

Projects which inspired l65:
 * [nimble65](https://bitbucket.org/kylearan/nimble65)
 * [k65](http://devkk.net/wiki/index.php?title=K65)

## License

l65 is licensed under the MIT License, see LICENSE for more information.
