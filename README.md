# l65
[![Build Status](https://travis-ci.org/g012/l65.svg?branch=master)](https://travis-ci.org/g012/l65)

l65 is a 6502 assembler, operating from within Lua and written in Lua. This means assembler mnemonics become regular Lua statements anywhere in the middle of Lua code.

Table of Contents
=================

  * [Concept - How It Works](#concept---how-it-works)
  * [Command Line Usage](#command-line-usage)
  * [Samples - Getting Started](#samples---getting-started)
  * [API](#api)
     * [Lua Extensions](#lua-extensions)
        * [Lambda Functions](#lambda-functions)
        * [Binary Numbers](#binary-numbers)
     * [Globals](#globals)
        * [getopt(optstring, ...)](#getoptoptstring-)
     * [Instruction List](#instruction-list)
        * [Regular Opcodes](#regular-opcodes)
        * [Undocumented Opcodes](#undocumented-opcodes)
        * [Deriving Function Names](#deriving-function-names)
        * [Late and Early Operands](#late-and-early-operands)
        * [Toggling Encapsulation](#toggling-encapsulation)
     * [Data Definition](#data-definition)
        * [dc.b ... ; byte(...) ; byte_hi(...) ; byte_lo(...)](#dcb---byte--byte_hi--byte_lo)
        * [dc.w ... ; word(...)](#dcw---word)
        * [dc.l ... ; long(...)](#dcl---long)
        * [charset([s] [, f])](#charsets--f)
        * [byte_normalize(v)](#byte_normalizev)
        * [word_normalize(v)](#word_normalizev)
        * [long_normalize(v)](#long_normalizev)
     * [Assembler Functions (6502.lua)](#assembler-functions-6502lua)
        * [Module Properties](#module-properties)
        * [location(start [, finish]) ; location(opt)](#locationstart--finish--locationopt)
           * [Other Fields](#other-fields)
        * [@@name ; section(name) ; section(opt)](#name--sectionname--sectionopt)
           * [Other Fields](#other-fields-1)
        * [@name ; label(name)](#name--labelname)
           * [Other Fields](#other-fields-2)
        * [samepage ... end](#samepage--end)
        * [crosspage ... end](#crosspage--end)
        * [skip(bytes)](#skipbytes)
        * [relate(section1, section2 [, [offset1,] offset2])](#relatesection1-section2--offset1-offset2)
        * [sleep(cycles [, noillegal])](#sleepcycles--noillegal)
        * [op_resolve(v)](#op_resolvev)
        * [link()](#link)
        * [resolve()](#resolve)
        * [genbin([filler])](#genbinfiller)
        * [writebin(filename)](#writebinfilename)
        * [writesym(filename)](#writesymfilename)
     * [Parser Functions](#parser-functions)
        * [Pragmas](#pragmas)
           * [syntax6502 on|off](#syntax6502-onoff)
           * [encapsulate on|off](#encapsulate-onoff)
           * [encapsulate opcode funcname](#encapsulate-opcode-funcname)
           * [add_opcode opcode addressing](#add_opcode-opcode-addressing)
           * [alias opcode newname](#alias-opcode-newname)
        * [l65 Module](#l65-module)
           * [parse(src [, src_name])](#parsesrc--src_name)
           * [format(ast)](#formatast)
           * [searcher_index](#searcher_index)
           * [search_path](#search_path)
           * [report(success, ...)](#reportsuccess-)
           * [msghandler(msg)](#msghandlermsg)
           * [load_embedded(name)](#load_embeddedname)
           * [searcher(name)](#searchername)
           * [load, loadfile, dofile ; installhooks(), uninstallhooks()](#load-loadfile-dofile--installhooks-uninstallhooks)
           * [image(filename)](#imagefilename)
     * [VCS Module](#vcs-module)
  * [Building](#building)
     * [Windows](#windows)
     * [Linux](#linux)
  * [Vim files installation](#vim-files-installation)
  * [TODO List](#todo-list)
  * [Credits](#credits)
  * [License](#license)

## Concept - How It Works

l65 hooks into Lua `require` and related functionalities to add a parsing pass before Lua compilation. This pass transforms 6502 statements into regular Lua function calls. It applies only to files with extension '.l65', not '.lua'.

Since l65 features a linker, every symbol referenced has to be resolved after link, not at the time the opcode function gets executed. As such, the operand is encapsulated into a function for delayed execution: `rts` translates to `rtsimp()`, a global function call, which by default resolves to the `6502.lua` module set as the `__index` field of the metatable of the l65 source file's `_ENV`. Every function call translates to a name comprised of the mnemonic followed by a 3 characters addressing mode.

Spatial arrangement follows [nimble65](https://bitbucket.org/kylearan/nimble65) naming: *locations* define ROM areas, containing *sections* with some positional constraints, made of a sequence of instructions and/or data. Sections which are not fixed in space are then positioned during link phase within the locations. Then the resolve phase sets numeric values into the `symbols` table, and `genbin` creates a byte table representing the final ROM.

## Command Line Usage

```
Usage: l65 [options] file [args]
Options:
  -d <file>        Dump the Lua code after l65 parsing into file
  -h               Display this information
  -v               Display the release version
```

`args` are passed as arguments to the l65 script in `file`. The global function [getopt](#getoptoptstring-) is available to parse them.

## Samples - Getting Started

Have a look at these files in the `samples` folder to get started with l65:
 * `vcs_basic.l65`: a 2Kb VCS ROM which just shows color bands.
 * `vcs_hooks.l65`: shows how to insert your own instructions into the pipeline, here to get the cycle count and size of the kernel. Also, it does not use the main loop helpers nor mapper functions to show the actual code involved.
 * `vcs_hello.l65`: a VCS 'Hello World' using the playfield.
 * `vcs_banks.l65`: demonstrates 2 methods to call functions across banks.
 * `vcs_flush.l65`: a port of flewww's famous flush logo from the demo [.bin](http://www.pouet.net/prod.php?which=69666)
 * `vcs_spr48.l65`: a port of [.bin](http://www.pouet.net/prod.php?which=69666) title 48 pixels sprite animation.
 * `vcs_music.l65`: imports and plays back a TIATracker .ttt file directly.
 * `vcs_mcart.l65`: imports all TIATracker .ttt files in the folder supplied as arg, or current directory, and builds a music cart.

There's also `vcspal.act`, a palette file for authoring software for VCS. Use this palette or a similar one to create 8b PNG for `l65.image` and helper loaders depending on it. You can generate such a palette, or a GPL one for GIMP using [vcsconv](https://github.com/g012/vcsconv) `authpalette` command.

## API

### Lua Extensions

#### Lambda Functions

Since anonymous functions are core to l65, the extension syntax such as `\a,b(a+b)` is available to define them: it expands to `function(a,b) return a+b end`.

#### Binary Numbers

A simple parsing of number literals in the form of `0b11011` converts this base-2 / binary number into a decimal number after the l65 pass. There is no support for them in other places though - no format type for `string.format` or anything else.

### Globals

#### getopt(optstring, ...)

Taken from [Posix Get Opt](http://lua-users.org/wiki/PosixGetOpt).
Parse args in `...` using `optstring`, the same way as `getopt` from Posix.

Usage:

```lua
--
-- opt is either the option character, "?", or ":".
--
-- If opt is the option character, arg is either a string or false--if optstring
-- did not specify a required argument.
--
-- If opt is either "?" or ":", then arg is the unknown option or the option
-- lacking the required argument. Like the standard says, there's really only
-- one scenario for ":" to ever be returned. And per the standard, unless
-- optstring begins with a colon, "?" is returned instead.
--
for opt, arg in getopt(":a:b", ...) do
        print("opt:", opt, "arg:", arg)
end
```

### Instruction List

#### Regular Opcodes

|   |   |   |   |   |   |   |   |
|---|---|---|---|---|---|---|---|
|adc|and|asl|bcc|bcs|beq|bit|bmi|
|bne|bpl|brk|bvc|bvs|clc|cld|cli|
|clv|cmp|cpx|cpy|dec|dex|dey|eor|
|inc|inx|iny|jmp|jsr|lda|ldx|ldy|
|lsr|nop|ora|pha|php|pla|plp|rol|
|ror|rti|rts|sbc|sec|sed|sei|sta|
|stx|sty|tax|tay|tsx|txa|txs|tya|

#### Undocumented Opcodes

|   |   |   |   |   |   |   |   |
|---|---|---|---|---|---|---|---|
|anc|ane|arr|asr|dcp|isb|jam|las|
|lax|rla|rra|sax|sbx|sha|shs|shx|
|shy|slo|sre|

#### Deriving Function Names

Each opcode is parsed by l65 then transformed to a Lua function call inside the 6502 module by concatenating the name of the opcode with one of the following addressing mode:

|Mode|Code|
|----|----|
|implied|imp|
|immediate|imm|
|zeropage|zpg|
|zeropage,x|zpx|
|zeropage,y|zpy|
|absolute|abs|
|absolute,x|abx|
|absolute,y|aby|
|(indirect)|ind|
|(indirect,x)|inx|
|(indirect),y|iny|
|relative|rel|

So for instance, `cpy #6` becomes `cpyimm(6)`, and `dec 0x80,x` becomes `deczpx(0x80)`.
There's no `Accumulator` addressing mode, use `implied` instead.

For opcodes which have both a zeropage (byte size) addressing and a matching absolute (word size) addressing, the mode can be forced using `.b` or `.w` suffix on the opcode. This may be required if the first operand can't be resolved at the beginning of the link phase, but resolves to a zeropage (byte size) addressing after link: if resolve fails when computing the size of the instruction, absolute (word size) is assumed. These opcodes, without suffix, use the following addressing codes:

|Mode|Code|
|----|----|
|zeropage or absolute|zab|
|zeropage,x or absolute,x|zax|
|zeropage,y or absolute,y|zay|

#### Late and Early Operands

The first operand of each opcode is by default encapsulated into a function for late processing during link and binary generation phases, if it's not a simple number or string. So:

```lua
    sta WSYNC
-- translates to:
    stazab(function(_o,_f) return _o+_f( WSYNC) end)
```

An optional second operand can be specified, regardless of the addressing mode, which is not encapsulated and added later to the result of the first encapsulated parameter. The default function for the first operand simply adds the second operand to it, but any function can be specified instead.
In the example above, `_o` is the value of the second operand, and `_f` is a function used to resolve the value of the first operand (see [op_resolve](#op_resolve-v)).


```lua
    lda TIM1T,i
    lda (bla,5,x)
    lda (bla,4),y
-- translates to:
    ldazab(function(_o,_f) return _o+_f( TIM1T) end,i)
    ldainx (function(_o,_f) return _o+_f(bla) end,5)
    ldainy (function(_o,_f) return _o+_f(bla) end,4)
```

This is useful in case the value you want to use as operand is a reference to a global variable for instance. Since the function is evaluated later, the value might have changed in the meantime. Consider this:

```lua
    bla = 5
    lda (\o,f(print(o,bla) or o+bla),bla,x) -- for lda (bla,bla,x), or lda (bla+bla,x) in another assembler
    bla = 3
-- translates to:
    bla = 5
    ldainx (function(o,f) return print(o,bla) or o+bla end,bla)
    bla = 3
-- and prints:
5       3
```

So specifying `bla` in the first operand got us the value 3, ie the *last* value of the global `bla`, whereas using it in the second operand caused an immediate resolve of its value, which is 5 at the *ldainx function call*.

Note that the first operand function must be *reentrant*: it cannot have side effects, as it may be called more than once if its value can't be resolved at the beginning of the link phase. If you do need to specify side effects for it, consider hooking into the instructions before and after, to temporarily modify the `pcall` and/or `pcall_za` fields of the 6502.lua module used to attempt resolving before link is complete.

#### Toggling Encapsulation

Besides the use of `pragmas`, automatic encapsulation can be toggled from its current state for the current instruction using `!`. So if, as default, it is enabled:

```lua
    sta WSYNC
    sta !WSYNC
-- translates to:
    stazab(function(_o,_f) return _o+_f( WSYNC) end)
    stazab(WSYNC)
```

### Data Definition

#### dc.b ... ; byte(...) ; byte_hi(...) ; byte_lo(...)

Insert byte data into the current section.

Each argument is either a byte size number, a function evaluated after resolve phase and returning exactly one byte size number, a table of byte size numbers, or a string using the current [character set](#charsets--f).

`dc.b` encapsulates each of its arguments into a parameter-less function. For each argument, the current encapsulation state can be inverted using `!`.

`byte_hi` arguments can instead be any size, and the bits 8 to 15 are taken as the final byte value. Conversely, `byte_lo` keeps bits 0 to 7.

#### dc.w ... ; word(...)

Insert a word as two bytes in little-endian order.

Each argument is either a string to be used as key in the `symbols` table, a section, a label, a word size number, a function evaluated after resolve phase and returning exactly one word size number, or a table of word size numbers.

`dc.w` encapsulates each of its arguments into a parameter-less function. For each argument, the current encapsulation state can be inverted using `!`.

#### dc.l ... ; long(...)

Insert a long as four bytes in little-endian order.

Each argument is either a string to be used as key in the `symbols` table, a section, a label, a long size number, a function evaluated after resolve phase and returning exactly one long size number, or a table of long size numbers.

`dc.l` encapsulates each of its arguments into a parameter-less function. For each argument, the current encapsulation state can be inverted using `!`.

#### charset([s] [, f])

Set a new character set to be used for next string data in byte(). Without argument, revert to Lua charset.

`s`: string of all the letters of charset, or a table of the charset as returned by a previous `charset` call to set again, in which case `f` is ignored.

`f`: letter index offset, added to each letter index, or a function to transform the letter index.

Return the charset table.

#### byte_normalize(v)

Check if `v` is within byte range, call `error` if it is not, or convert negative values to positive values if it is.

#### word_normalize(v)

Check if `v` is within word range, call `error` if it is not, or convert negative values to positive values if it is.

#### long_normalize(v)

Check if `v` is within long range, call `error` if it is not, or convert negative values to positive values if it is.

### Assembler Functions (6502.lua)

#### Module Properties

`strip`: defaults to `true`. Set to `false` to disable dead stripping of relocatable sections.

`strip_empty`: defaults to `false`. Set to `true` to enable stripping of empty sections; otherwise, they are positioned at the start of the container location.

`pcall`: defaults to system's `pcall`. Set to an empty function returning `false` to disable early evaluation of expressions during link phase for computing the size of each section. This will force all opcodes without an explicit size to default to the largest possible size...

`pcall_za`: ...unless this field is set to system's `pcall`. Defaults to module's `pcall`. This field is used only by the `za*` (`zab`, `zax`, `zay`) virtual addressing modes, to discriminate between zeropage and absolute addressing.

`symbols`: list of symbols, resolved or not. Values can be anything before the resolve phase, but must be numbers after (except for the metatable fields). Set as the metatable of the 6502 module, which itself should be set as the metatable of the current `_ENV` environment.

`locations`:  list of all the registered locations.

`location_current`: the currently active location, in which any new section is added to.

`sections`: list of all the registered sections.

`section_current`: the currently active section, in which any new data or instruction is added to.

`label_current`: the previous non-local label name.

`relations`: a list of all the registered relationships.

`before_link`: a list of hook functions called just before the linking phase.

`id()`: return a new unique numeric identifier.

`stats`: a table of statistics regarding the build:
 * `cycles`: the total 6502 cycle count of the program, considering no branch is taken and no page is crossed.
 * `used`: the total ROM bytes actually used by the program.
 * `unused`: total empty ROM space.
 * `resolved_count`: number of symbols resolved during resolve phase.
 * `bin_size`: final binary size.
 * `__tostring()`: string conversion function, intended to print ROM information to the user.

#### location(start [, finish]) ; location(opt)

Create a ROM area starting at `start` or `opt[1]`. If `finish` or `opt[2]` is set, it is the last byte of the area, inclusive; otherwise, the area is unbounded.

If `opt` is an already existing location table, it sets this location as current and returns.

`opt.rorg` specifies a relocated origin: all references to symbols within this section are computed as if `start` had the value of `opt.rorg`. If it is a number, it's converted to a function adding `opt.rorg` - `opt[1]`.

If `opt.nofill` resolves to `true`, the optional gap between this location and the next is not filled up with the filler byte.

`opt.name` is displayed within `stats.__tostring()`.

When using the `opt` version, `opt` is first set as the actual location table, so custom properties are preserved.

Return the location table.

##### Other Fields

 * `type`: the string `'location'`.
 * `sections`: list of sections inside this location.
 * `chunks`: a list of unused memory sections of the location area. Manipulated during link phase. Each entry has `id`, `start`, and `size` fields.
 * `used`: memory within the section containing data or instructions.
 * `unused`: wasted memory within the section, containing nothing.
 * `stops_at`: for unbounded locations, position of the last used byte.

#### @@name ; section(name) ; section(opt)

Create a section within the current location, using name `name`, `opt[1]`, or a generated unique name. Also create a label with this name.

If `opt` is an already existing section table, it sets this section as current, its owner location as current, and returns.

If `opt.org` is set, the section is fixed and must start at this address in ROM; otherwise, it can be positioned anywhere within the location during link phase.

`opt.align` requests the section starts on an address multiple of `opt.align`. If `opt.offset` is specified, then it is the reference point from the start of the section which must be aligned, instead of the start of the section itself.

If `opt.strong` resolves to `true`, the section will not be stripped even if it has no reference and stripping is active. If `opt.weak` resolves to `true`, the section is stripped if its size is 0, and the references to it will fail to resolve.

When using the `opt` version, `opt` is first set as the actual section table, so custom properties are preserved.

Return the section table.

##### Other Fields

 * `type`: the string `'section'`.
 * `id`: a unique number identifier.
 * `label`: the name of this section.
 * `size`: the computed size of the section during link phase.
 * `cycles`: the sum of cycle count of the instructions within `instructions`, after link phase.
 * `refcount`: a positive number if this section is referenced.
 * `location`: the location containing this section, the currently active one at the point of the section function call.
 * `instructions`: the list of opcode functions contained within this section. Each instruction can contain:
   - `size`: a number or function returning the size of the opcode and its operands.
   - `cycles`: the number of cycles the instruction needs to execute.
   - `bin`: a byte or a function returning the binary representation of the opcode and its operands.
   - `offset`: the number of bytes from the start of the section at which this instruction is located; set during link phase.
 * `constraints`: the list of constraints within the section, filled by `samepage` and `crosspage` blocks. Each constraint has a `type` set to `'samepage'` or `'crosspage'`,  `from` and inclusive `to` indices into `instructions`, and after link phase a `start` and inclusive `finish` address position.
 * `holes`: a list of holes created by [skip](#skipbytes). Each hole has `start` index into `instructions` and a `size` number set to the parameter of `skip`.

#### @name ; label(name)

Create a label, that is a named reference to a position into the current section. If `name` starts with an `_`, the name is local to the previous one, and the final name is the previous non-local label name concatenated with this one; as such, a local label can be referenced outside its scope if named in full, with the parent prefix.

An entry with the label name referencing this label is added into the symbols table, and resolved during resolve phase.

Return the name of the label, and the label table.

##### Other Fields

 * `type`: the string `'label'`.
 * `section`: the section containing this label.
 * `label`: the name of this label.
 * `size`: a function grabbing this label offset into the section during link phase.
 * `resolve`: a function returning the address of this label, after link phase succeeded.
 * `bin`: a function setting the `label_current` to this label name, if it's not local.

#### samepage ... end

Add a constraint into the current section: all the instructions or data within the block must not cross a 256 bytes page.

#### crosspage ... end

Add a constraint into the current section: the instructions or data within the block must span over at least two 256 byte pages.

#### skip(bytes)

Insert a hole in the current section of size `bytes`, which can be used by another relocatable section.

#### relate(section1, section2 [, [offset1,] offset2])

Add a position relationship between `section1` and `section2`, with `offset1` bytes from selected position for `section2`, and `offset2` bytes from selected position for `section1`, in relation to their containing locations. If `offset1` is omitted, `-offset2` is used.

This is used to ensure two sections in different locations will start at the same or related offset. For instance, if the location of `section1` starts at 0xE000 and the location of `section2` starts at 0xF000, and the linker sets `section1` to 0xE100, then `relate(section1, section2)` implies that `section2` is at 0xF100.

Related sections are positioned before other relocatable sections during link phase.

#### sleep(cycles [, noillegal])

Generate instructions into the current section to waste `cycles` cycles. If `noillegal` resolves to `true`, trashes NZ flags; otherwise, it uses illegal opcodes to preserve the flags.

#### op_resolve(v)

Performs basic operations on `v` according to its type to attempt to turn it into a number. If it's a function, it's called, if it's a string, a label or a section, it gets its address from the `symbols` table, and otherwise it fails with a call to `error`.

#### link()

Link the sections, ie set their position into their parent locations. It triggers an early evaluation of operands, to determine their size, using the 6502 `pcall` and `pcall_za` fields.

#### resolve()

Resolve symbols to numeric address values: if the value is a function, it calls it; if it's a table and has a `resolve` function field, it calls it; if it's a string, it's an index into `symbols`.

It then sets a metatable for the table `symbols` used for resolving local label references during `genbin`, as they need to know the current parent label for that.

It calls `link` if needed.

#### genbin([filler])

Generate the binary as a table, using `filler` byte to fill gaps. `filler` defaults to 0 (`brk` opcode). It calls `resolve` first if needed.

Return a table, where each entry is a byte.

#### writebin(filename)

Write the final binary into `filename`.

#### writesym(filename)

Write a DASM symbol file into `filename` for debuggers. The last `_` in a label is turned into a `.`, to get stripping of the prefixed global label working in Stella. As such, it's best to avoid using `_` in local label names, after the initial one.

### Parser Functions

#### Pragmas

##### syntax6502 on|off

Enable or disable 6502 syntax parsing, besides pragma itself. Default is `on`.
For instance:

```lua
#pragma syntax6502 off
lda = 5 if lda < 6 then sta=7 end -- 'lda' is now available for use normally
#pragma syntax6502 on
lda #5 -- 'lda' is now an opcode again: generates 'ldaimm(5)'
```

##### encapsulate on|off

Enable or disable automatic encapsulation of first operand of opcodes. It also inverts the meaning of `!`, which toggles the encapsulation behaviour only for the current opcode. Default is `on`.

```lua
    lda !g(),v -- disable encapsulation of 'g()': ldazab(g(),v)
#pragma encapsulate off
    lda f,v -- 'f' is not encapsulated: ldazab(f,v)
    lda !_toto+15,16,x -- '_toto+15' is encapsulated: ldazax(function(_o,_f) return _o+_f(_toto+15) end,16)
    lda #15 -- '15' is never encapsulated, as it is a number: ldaimm(15)
#pragma encapsulate on
```

##### encapsulate opcode funcname

Add the following behaviour: Parse `opcode`, parse a single expression as operand, and encapsulate it as an argument to `funcname`. For instance:

```lua
-- translate 'far x' to 'farcall(function() return x end)', farcall is defined in vcs.l65
-- encapsulation allows referencing of yet undefined sections
#pragma encapsulate far farcall
    far kernel -- generates 'farcall(function() return kernel end)'
-- translate 'xsr x' to 'xcall(function() return x end)', xcall is defined in vcs.l65
#pragma encapsulate xsr xcall
    xsr myfunc -- generates 'xcall(function() return myfunc end)'
```


##### add_opcode opcode addressing

Add `opcode` with `addressing` code to the opcode tables. The resulting function must be globally available. For instance:

```lua
-- translate 'rtx' to 'rtximp()', rtximp is defined as global in vcs.l65
#pragma add_opcode rtx imp
    rtx -- generates 'rtximp()'
```

Operands parsing is done as per implied by the addressing mode, like other opcodes.

##### alias opcode newname

Create an alias `newname` for opcode `opcode`, retaining the original name as well. For instance:

```lua
#pragma alias and dna
    dna #0x3F -- same as and #0x3F
```

Note that aliasing `and` might be a good idea, since it's also a keyword for expressions in Lua. Consider this:

```lua
lda time; and#7 -- ';' is required here to prevent considering it as ldazab(time and #7)
lda time dna#7 -- using dna, no more ambiguity: ldazab(time) andimm(7)
```

#### l65 Module

##### parse(src [, src_name])

Parse `src` as l65, using `src_name` for error reporting.

Return `true` and an AST table on success; `false` and an error string otherwise.

##### format(ast)

Transform `ast` into a string.

Return a string of `ast` on success; raise an error otherwise.

##### searcher_index

The index at which to insert the l65 file searcher, followed by the embedded scripts searcher. Defaults to 2.

##### search_path

The search path used by the l65 file searcher. Defaults to filename if current directory, then with added '.l65' extension, then in l65 executable directory, then in l65 executable directory with '.l65' extension.

##### report(success, ...)

If `success` resolves to `true`, it returns all of its arguments. Otherwise, it writes the message, which is the first field of `...`, to `stderr` and exits the program.

##### msghandler(msg)

The function used for `xpcall` of the input file given on command line. It merges two stacktraces when an error happens in a late eval function.

##### load_embedded(name)

The searcher for scripts included into the l65 executable binary.

##### searcher(name)

The searcher for l65 files on disk.

##### load, loadfile, dofile ; installhooks(), uninstallhooks()

Hook functions to load l65 chunks and files automatically. Installed by default. Use `l65.installhooks()` and `l65.uninstallhooks()` to set the hooks or restore the original functions.

##### image(filename)

A C function, calling stb_image on `filename`, with only 8b palette-based PNG support. It does not translate colors to RGB triplets, but keeps the palette index as the value of the pixel.

Return `nil` and an error message on failure, or a table with the following fields on success:
 * `filename`: PNG filename
 * `width`: image width, in pixels
 * `height`: image height, in pixels
 * [1..width\*height]: the pixels, as palette indices (all bytes)

### VCS Module

`vcs.l65` is a helper file for developing on Atari 2600 VCS. It's embedded into the l65 executable. It sets the 6502.lua module as metatable of the current `_ENV`, defines all TIA and PIA symbols, some helper constants and functions, as well as mapper helpers and automatic cross bank call functions. See the samples directory for usage examples, and browse vcs.l65 directly for the list of self-explanatory helpers.

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

 * copy `vim/*` into `~/vimfiles/`
 * add `au BufRead,BufNewFile *.l65 set syntax=l65 filetype=l65` to `~/.vimrc` (or `~/_vimrc` on Windows)

Note that the syntax file includes some highlighting for features only activated via pragmas: `dna`, `xsr`, `rtx` and `far`. If you do not want to use these keywords, remove them from the syntax file.

## TODO List

 * [k65](http://devkk.net/wiki/index.php?title=K65) style syntax
 * helpers to inter-operate with cc/ca65 and dasm

## Credits

Developed by g012, using:
 * [Lua 5.3.4](https://www.lua.org)
 * [LuaMinify](https://github.com/stravant/LuaMinify)
 * [LuaFileSystem](https://keplerproject.github.io/luafilesystem)
 * [LPeg](http://www.inf.puc-rio.br/~roberto/lpeg)
 * [stb_image](https://github.com/nothings/stb)
 * [dkjson](http://dkolf.de/src/dkjson-lua.fsl)
 * Lua syntax file shipping with [vim](http://www.vim.org)
 * Lua indent file from [vim-lua](https://github.com/tbastos/vim-lua)

Projects which inspired l65:
 * [nimble65](https://bitbucket.org/kylearan/nimble65)
 * [k65](http://devkk.net/wiki/index.php?title=K65)

## License

l65 is licensed under the MIT License, see LICENSE for more information.
