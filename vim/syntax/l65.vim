" Vim syntax file
" Language:	l65

" For version 5.x: Clear all syntax items
" For version 6.x: Quit when a syntax file was already loaded
if version < 600
  syntax clear
elseif exists("b:current_syntax")
  finish
endif

let s:cpo_save = &cpo
set cpo&vim

syn case match

" syncing method
syntax sync fromstart

" Comments
syn keyword luaTodo            contained TODO FIXME XXX
syn match   luaComment         "--.*$" contains=luaTodo,@Spell
syn region luaComment        matchgroup=luaComment start="--\[\z(=*\)\[" end="\]\z1\]" contains=luaTodo,@Spell

" First line may start with #!
syn match luaComment "\%^#!.*"

" catch errors caused by wrong parenthesis and wrong curly brackets or
" keywords placed outside their respective blocks
syn region luaParen      transparent                     start='(' end=')' contains=ALLBUT,luaParenError,luaTodo,luaSpecial,luaIfThen,luaElseifThen,luaElse,luaThenEnd,luaBlock,luaLoopBlock,luaIn,luaStatement
syn region luaTableBlock transparent matchgroup=luaTable start="{" end="}" contains=ALLBUT,luaBraceError,luaTodo,luaSpecial,luaIfThen,luaElseifThen,luaElse,luaThenEnd,luaBlock,luaLoopBlock,luaIn,luaStatement

syn match  luaParenError ")"
syn match  luaBraceError "}"
syn match  luaError "\<\%(end\|else\|elseif\|then\|until\|in\)\>"

" function ... end
syn region luaFunctionBlock transparent matchgroup=luaFunction start="\<function\>" end="\<end\>" contains=ALLBUT,luaTodo,luaSpecial,luaElseifThen,luaElse,luaThenEnd,luaIn

" if ... then
syn region luaIfThen transparent matchgroup=luaCond start="\<if\>" end="\<then\>"me=e-4           contains=ALLBUT,luaTodo,luaSpecial,luaElseifThen,luaElse,luaIn nextgroup=luaThenEnd skipwhite skipempty

" then ... end
syn region luaThenEnd contained transparent matchgroup=luaCond start="\<then\>" end="\<end\>" contains=ALLBUT,luaTodo,luaSpecial,luaThenEnd,luaIn

" elseif ... then
syn region luaElseifThen contained transparent matchgroup=luaCond start="\<elseif\>" end="\<then\>" contains=ALLBUT,luaTodo,luaSpecial,luaElseifThen,luaElse,luaThenEnd,luaIn

" else
syn keyword luaElse contained else

" do ... end
syn region luaBlock transparent matchgroup=luaStatement start="\<do\>" end="\<end\>"          contains=ALLBUT,luaTodo,luaSpecial,luaElseifThen,luaElse,luaThenEnd,luaIn

" samepage ... end
syn region luaBlock transparent matchgroup=luaStatement start="\<samepage\>" end="\<end\>"          contains=ALLBUT,luaTodo,luaSpecial,luaElseifThen,luaElse,luaThenEnd,luaIn
" crosspage ... end
syn region luaBlock transparent matchgroup=luaStatement start="\<crosspage\>" end="\<end\>"          contains=ALLBUT,luaTodo,luaSpecial,luaElseifThen,luaElse,luaThenEnd,luaIn

" repeat ... until
syn region luaLoopBlock transparent matchgroup=luaRepeat start="\<repeat\>" end="\<until\>"   contains=ALLBUT,luaTodo,luaSpecial,luaElseifThen,luaElse,luaThenEnd,luaIn

" while ... do
syn region luaLoopBlock transparent matchgroup=luaRepeat start="\<while\>" end="\<do\>"me=e-2 contains=ALLBUT,luaTodo,luaSpecial,luaIfThen,luaElseifThen,luaElse,luaThenEnd,luaIn nextgroup=luaBlock skipwhite skipempty

" for ... do and for ... in ... do
syn region luaLoopBlock transparent matchgroup=luaRepeat start="\<for\>" end="\<do\>"me=e-2   contains=ALLBUT,luaTodo,luaSpecial,luaIfThen,luaElseifThen,luaElse,luaThenEnd nextgroup=luaBlock skipwhite skipempty

syn keyword luaIn contained in

" other keywords
syn keyword luaStatement return local break goto
syn match luaLabel "::\I\i*::"
syn match l65Label "@@\k\+"
syn match l65Label "@\k\+"
syn match l65SChar "[#!\\]"
syn match l65SFunc "[\\]"
syn keyword luaOperator and or not
syn keyword luaConstant nil true false

syn region	l65PreProc	start="^#\s*\(pragma\>\)" skip="\\$" end="$" keepend
  
" Strings
syn match  luaSpecial contained #\\[\\abfnrtvz'"]\|\\b[01]\{2}\|\\x[[:xdigit:]]\{2}\|\\[[:digit:]]\{,3}#
syn region luaString2 matchgroup=luaString start="\[\z(=*\)\[" end="\]\z1\]" contains=@Spell
syn region luaString  start=+'+ end=+'+ skip=+\\\\\|\\'+ contains=luaSpecial,@Spell
syn region luaString  start=+"+ end=+"+ skip=+\\\\\|\\"+ contains=luaSpecial,@Spell

" integer number
syn match luaNumber "\<\d\+\>"
" hex numbers
syn match luaNumber "\<0[xX][[:xdigit:].]\+\%([pP][-+]\=\d\+\)\=\>"
" bin numbers
syn match luaNumber "\<0[bB][01]\+\>"
" floating point number, with dot, optional exponent
syn match luaNumber  "\<\d\+\.\d*\%([eE][-+]\=\d\+\)\=\>"
" floating point number, starting with a dot, optional exponent
syn match luaNumber  "\.\d\+\%([eE][-+]\=\d\+\)\=\>"
" floating point number, without dot, with exponent
syn match luaNumber  "\<\d\+[eE][-+]\=\d\+\>"

syn keyword luaFunc assert collectgarbage dofile error next
syn keyword luaFunc print rawget rawset tonumber tostring type _VERSION

syn keyword luaFunc getmetatable setmetatable
syn keyword luaFunc ipairs pairs
syn keyword luaFunc pcall xpcall
syn keyword luaFunc _G loadfile rawequal require
syn keyword luaFunc load select
syn match   luaFunc /\<package\.cpath\>/
syn match   luaFunc /\<package\.loaded\>/
syn match   luaFunc /\<package\.loadlib\>/
syn match   luaFunc /\<package\.path\>/
syn keyword luaFunc _ENV rawlen
syn match   luaFunc /\<package\.config\>/
syn match   luaFunc /\<package\.preload\>/
syn match   luaFunc /\<package\.searchers\>/
syn match   luaFunc /\<package\.searchpath\>/
syn match   luaFunc /\<bit32\.arshift\>/
syn match   luaFunc /\<bit32\.band\>/
syn match   luaFunc /\<bit32\.bnot\>/
syn match   luaFunc /\<bit32\.bor\>/
syn match   luaFunc /\<bit32\.btest\>/
syn match   luaFunc /\<bit32\.bxor\>/
syn match   luaFunc /\<bit32\.extract\>/
syn match   luaFunc /\<bit32\.lrotate\>/
syn match   luaFunc /\<bit32\.lshift\>/
syn match   luaFunc /\<bit32\.replace\>/
syn match   luaFunc /\<bit32\.rrotate\>/
syn match   luaFunc /\<bit32\.rshift\>/
syn match luaFunc /\<coroutine\.running\>/
syn match   luaFunc /\<coroutine\.create\>/
syn match   luaFunc /\<coroutine\.resume\>/
syn match   luaFunc /\<coroutine\.status\>/
syn match   luaFunc /\<coroutine\.wrap\>/
syn match   luaFunc /\<coroutine\.yield\>/
syn match   luaFunc /\<string\.byte\>/
syn match   luaFunc /\<string\.char\>/
syn match   luaFunc /\<string\.dump\>/
syn match   luaFunc /\<string\.find\>/
syn match   luaFunc /\<string\.format\>/
syn match   luaFunc /\<string\.gsub\>/
syn match   luaFunc /\<string\.len\>/
syn match   luaFunc /\<string\.lower\>/
syn match   luaFunc /\<string\.rep\>/
syn match   luaFunc /\<string\.sub\>/
syn match   luaFunc /\<string\.upper\>/
syn match luaFunc /\<string\.gmatch\>/
syn match luaFunc /\<string\.match\>/
syn match luaFunc /\<string\.reverse\>/
syn match luaFunc /\<table\.pack\>/
syn match luaFunc /\<table\.unpack\>/
syn match   luaFunc /\<table\.concat\>/
syn match   luaFunc /\<table\.sort\>/
syn match   luaFunc /\<table\.insert\>/
syn match   luaFunc /\<table\.remove\>/
syn match   luaFunc /\<math\.abs\>/
syn match   luaFunc /\<math\.acos\>/
syn match   luaFunc /\<math\.asin\>/
syn match   luaFunc /\<math\.atan\>/
syn match   luaFunc /\<math\.atan2\>/
syn match   luaFunc /\<math\.ceil\>/
syn match   luaFunc /\<math\.sin\>/
syn match   luaFunc /\<math\.cos\>/
syn match   luaFunc /\<math\.tan\>/
syn match   luaFunc /\<math\.deg\>/
syn match   luaFunc /\<math\.exp\>/
syn match   luaFunc /\<math\.floor\>/
syn match   luaFunc /\<math\.log\>/
syn match   luaFunc /\<math\.max\>/
syn match   luaFunc /\<math\.min\>/
syn match luaFunc /\<math\.huge\>/
syn match luaFunc /\<math\.fmod\>/
syn match luaFunc /\<math\.modf\>/
syn match luaFunc /\<math\.cosh\>/
syn match luaFunc /\<math\.sinh\>/
syn match luaFunc /\<math\.tanh\>/
syn match   luaFunc /\<math\.pow\>/
syn match   luaFunc /\<math\.rad\>/
syn match   luaFunc /\<math\.sqrt\>/
syn match   luaFunc /\<math\.frexp\>/
syn match   luaFunc /\<math\.ldexp\>/
syn match   luaFunc /\<math\.random\>/
syn match   luaFunc /\<math\.randomseed\>/
syn match   luaFunc /\<math\.pi\>/
syn match   luaFunc /\<io\.close\>/
syn match   luaFunc /\<io\.flush\>/
syn match   luaFunc /\<io\.input\>/
syn match   luaFunc /\<io\.lines\>/
syn match   luaFunc /\<io\.open\>/
syn match   luaFunc /\<io\.output\>/
syn match   luaFunc /\<io\.popen\>/
syn match   luaFunc /\<io\.read\>/
syn match   luaFunc /\<io\.stderr\>/
syn match   luaFunc /\<io\.stdin\>/
syn match   luaFunc /\<io\.stdout\>/
syn match   luaFunc /\<io\.tmpfile\>/
syn match   luaFunc /\<io\.type\>/
syn match   luaFunc /\<io\.write\>/
syn match   luaFunc /\<os\.clock\>/
syn match   luaFunc /\<os\.date\>/
syn match   luaFunc /\<os\.difftime\>/
syn match   luaFunc /\<os\.execute\>/
syn match   luaFunc /\<os\.exit\>/
syn match   luaFunc /\<os\.getenv\>/
syn match   luaFunc /\<os\.remove\>/
syn match   luaFunc /\<os\.rename\>/
syn match   luaFunc /\<os\.setlocale\>/
syn match   luaFunc /\<os\.time\>/
syn match   luaFunc /\<os\.tmpname\>/
syn match   luaFunc /\<debug\.debug\>/
syn match   luaFunc /\<debug\.gethook\>/
syn match   luaFunc /\<debug\.getinfo\>/
syn match   luaFunc /\<debug\.getlocal\>/
syn match   luaFunc /\<debug\.getupvalue\>/
syn match   luaFunc /\<debug\.setlocal\>/
syn match   luaFunc /\<debug\.setupvalue\>/
syn match   luaFunc /\<debug\.sethook\>/
syn match   luaFunc /\<debug\.traceback\>/
syn match luaFunc /\<debug\.getmetatable\>/
syn match luaFunc /\<debug\.setmetatable\>/
syn match luaFunc /\<debug\.getregistry\>/
syn match luaFunc /\<debug\.getuservalue\>/
syn match luaFunc /\<debug\.setuservalue\>/
syn match luaFunc /\<debug\.upvalueid\>/
syn match luaFunc /\<debug\.upvaluejoin\>/

synt match  luaSemiCol ";"

syn match l65Keyword /\<location\>/
syn match l65Keyword /\<section\>/
syn match l65Keyword /\<charset\>/
syn match l65Keyword /\<byte\>/
syn match l65Keyword /\<byte_hi\>/
syn match l65Keyword /\<byte_lo\>/
syn match l65Keyword /\<word\>/

syn match l65Keyword /\<adcimm\>/
syn match l65Keyword /\<adczpg\>/
syn match l65Keyword /\<adczpx\>/
syn match l65Keyword /\<adcabs\>/
syn match l65Keyword /\<adcabx\>/
syn match l65Keyword /\<adcaby\>/
syn match l65Keyword /\<adczab\>/
syn match l65Keyword /\<adczax\>/
syn match l65Keyword /\<adcinx\>/
syn match l65Keyword /\<adciny\>/
syn match l65Keyword /\<andimm\>/
syn match l65Keyword /\<andzpg\>/
syn match l65Keyword /\<andzpx\>/
syn match l65Keyword /\<andabs\>/
syn match l65Keyword /\<andabx\>/
syn match l65Keyword /\<andaby\>/
syn match l65Keyword /\<andzab\>/
syn match l65Keyword /\<andzax\>/
syn match l65Keyword /\<andinx\>/
syn match l65Keyword /\<andiny\>/
syn match l65Keyword /\<aslimp\>/
syn match l65Keyword /\<aslzpg\>/
syn match l65Keyword /\<aslzpx\>/
syn match l65Keyword /\<aslabs\>/
syn match l65Keyword /\<aslabx\>/
syn match l65Keyword /\<aslzab\>/
syn match l65Keyword /\<aslzax\>/
syn match l65Keyword /\<bccrel\>/
syn match l65Keyword /\<bcsrel\>/
syn match l65Keyword /\<beqrel\>/
syn match l65Keyword /\<bitzpg\>/
syn match l65Keyword /\<bitabs\>/
syn match l65Keyword /\<bitzab\>/
syn match l65Keyword /\<bmirel\>/
syn match l65Keyword /\<bnerel\>/
syn match l65Keyword /\<bplrel\>/
syn match l65Keyword /\<brkimp\>/
syn match l65Keyword /\<bvcrel\>/
syn match l65Keyword /\<bvsrel\>/
syn match l65Keyword /\<clcimp\>/
syn match l65Keyword /\<cldimp\>/
syn match l65Keyword /\<cliimp\>/
syn match l65Keyword /\<clvimp\>/
syn match l65Keyword /\<cmpimm\>/
syn match l65Keyword /\<cmpzpg\>/
syn match l65Keyword /\<cmpzpx\>/
syn match l65Keyword /\<cmpabs\>/
syn match l65Keyword /\<cmpabx\>/
syn match l65Keyword /\<cmpaby\>/
syn match l65Keyword /\<cmpzab\>/
syn match l65Keyword /\<cmpzax\>/
syn match l65Keyword /\<cmpinx\>/
syn match l65Keyword /\<cmpiny\>/
syn match l65Keyword /\<cpximm\>/
syn match l65Keyword /\<cpxzpg\>/
syn match l65Keyword /\<cpxabs\>/
syn match l65Keyword /\<cpxzab\>/
syn match l65Keyword /\<cpyimm\>/
syn match l65Keyword /\<cpyzpg\>/
syn match l65Keyword /\<cpyabs\>/
syn match l65Keyword /\<cpyzab\>/
syn match l65Keyword /\<deczpg\>/
syn match l65Keyword /\<deczpx\>/
syn match l65Keyword /\<decabs\>/
syn match l65Keyword /\<decabx\>/
syn match l65Keyword /\<deczab\>/
syn match l65Keyword /\<deczax\>/
syn match l65Keyword /\<deximp\>/
syn match l65Keyword /\<deyimp\>/
syn match l65Keyword /\<eorimm\>/
syn match l65Keyword /\<eorzpg\>/
syn match l65Keyword /\<eorzpx\>/
syn match l65Keyword /\<eorabs\>/
syn match l65Keyword /\<eorabx\>/
syn match l65Keyword /\<eoraby\>/
syn match l65Keyword /\<eorzab\>/
syn match l65Keyword /\<eorzax\>/
syn match l65Keyword /\<eorinx\>/
syn match l65Keyword /\<eoriny\>/
syn match l65Keyword /\<inczpg\>/
syn match l65Keyword /\<inczpx\>/
syn match l65Keyword /\<incabs\>/
syn match l65Keyword /\<incabx\>/
syn match l65Keyword /\<inczab\>/
syn match l65Keyword /\<inczax\>/
syn match l65Keyword /\<inximp\>/
syn match l65Keyword /\<inyimp\>/
syn match l65Keyword /\<jmpabs\>/
syn match l65Keyword /\<jmpind\>/
syn match l65Keyword /\<jsrabs\>/
syn match l65Keyword /\<ldaimm\>/
syn match l65Keyword /\<ldazpg\>/
syn match l65Keyword /\<ldazpx\>/
syn match l65Keyword /\<ldaabs\>/
syn match l65Keyword /\<ldaabx\>/
syn match l65Keyword /\<ldaaby\>/
syn match l65Keyword /\<ldazab\>/
syn match l65Keyword /\<ldazax\>/
syn match l65Keyword /\<ldainx\>/
syn match l65Keyword /\<ldainy\>/
syn match l65Keyword /\<ldximm\>/
syn match l65Keyword /\<ldxzpg\>/
syn match l65Keyword /\<ldxzpy\>/
syn match l65Keyword /\<ldxabs\>/
syn match l65Keyword /\<ldxaby\>/
syn match l65Keyword /\<ldxzab\>/
syn match l65Keyword /\<ldxzay\>/
syn match l65Keyword /\<ldyzpg\>/
syn match l65Keyword /\<ldyzpx\>/
syn match l65Keyword /\<ldyabs\>/
syn match l65Keyword /\<ldyabx\>/
syn match l65Keyword /\<ldyzab\>/
syn match l65Keyword /\<ldyzax\>/
syn match l65Keyword /\<lsrimp\>/
syn match l65Keyword /\<lsrzpg\>/
syn match l65Keyword /\<lsrzpx\>/
syn match l65Keyword /\<lsrabs\>/
syn match l65Keyword /\<lsrabx\>/
syn match l65Keyword /\<lsrzab\>/
syn match l65Keyword /\<lsrzax\>/
syn match l65Keyword /\<nopimp\>/
syn match l65Keyword /\<oraimm\>/
syn match l65Keyword /\<orazpg\>/
syn match l65Keyword /\<orazpx\>/
syn match l65Keyword /\<oraabs\>/
syn match l65Keyword /\<oraabx\>/
syn match l65Keyword /\<oraaby\>/
syn match l65Keyword /\<orazab\>/
syn match l65Keyword /\<orazax\>/
syn match l65Keyword /\<orainx\>/
syn match l65Keyword /\<orainy\>/
syn match l65Keyword /\<phaimp\>/
syn match l65Keyword /\<phpimp\>/
syn match l65Keyword /\<plaimp\>/
syn match l65Keyword /\<plpimp\>/
syn match l65Keyword /\<rolimp\>/
syn match l65Keyword /\<rolzpg\>/
syn match l65Keyword /\<rolzpx\>/
syn match l65Keyword /\<rolabs\>/
syn match l65Keyword /\<rolabx\>/
syn match l65Keyword /\<rolzab\>/
syn match l65Keyword /\<rolzax\>/
syn match l65Keyword /\<rorimp\>/
syn match l65Keyword /\<rorzpg\>/
syn match l65Keyword /\<rorzpx\>/
syn match l65Keyword /\<rorabs\>/
syn match l65Keyword /\<rorabx\>/
syn match l65Keyword /\<rorzab\>/
syn match l65Keyword /\<rorzax\>/
syn match l65Keyword /\<rtiimp\>/
syn match l65Keyword /\<rtsimp\>/
syn match l65Keyword /\<sbcimm\>/
syn match l65Keyword /\<sbczpg\>/
syn match l65Keyword /\<sbczpx\>/
syn match l65Keyword /\<sbcabs\>/
syn match l65Keyword /\<sbcabx\>/
syn match l65Keyword /\<sbcaby\>/
syn match l65Keyword /\<sbczab\>/
syn match l65Keyword /\<sbczax\>/
syn match l65Keyword /\<sbcinx\>/
syn match l65Keyword /\<sbciny\>/
syn match l65Keyword /\<secimp\>/
syn match l65Keyword /\<sedimp\>/
syn match l65Keyword /\<seiimp\>/
syn match l65Keyword /\<stazpg\>/
syn match l65Keyword /\<stazpx\>/
syn match l65Keyword /\<staabs\>/
syn match l65Keyword /\<staabx\>/
syn match l65Keyword /\<staaby\>/
syn match l65Keyword /\<stazab\>/
syn match l65Keyword /\<stazax\>/
syn match l65Keyword /\<stainx\>/
syn match l65Keyword /\<stainy\>/
syn match l65Keyword /\<stxzpg\>/
syn match l65Keyword /\<stxzpy\>/
syn match l65Keyword /\<stxabs\>/
syn match l65Keyword /\<stxzab\>/
syn match l65Keyword /\<styzpg\>/
syn match l65Keyword /\<styzpx\>/
syn match l65Keyword /\<styabs\>/
syn match l65Keyword /\<styzab\>/
syn match l65Keyword /\<taximp\>/
syn match l65Keyword /\<tayimp\>/
syn match l65Keyword /\<tsximp\>/
syn match l65Keyword /\<txaimp\>/
syn match l65Keyword /\<txsimp\>/
syn match l65Keyword /\<tyaimp\>/

syn match l65Keyword /\<ancimm\>/
syn match l65Keyword /\<aneimm\>/
syn match l65Keyword /\<arrimm\>/
syn match l65Keyword /\<asrimm\>/
syn match l65Keyword /\<dcpzpg\>/
syn match l65Keyword /\<dcpzpx\>/
syn match l65Keyword /\<dcpabs\>/
syn match l65Keyword /\<dcpabx\>/
syn match l65Keyword /\<dcpaby\>/
syn match l65Keyword /\<dcpinx\>/
syn match l65Keyword /\<dcpiny\>/
syn match l65Keyword /\<isbzpg\>/
syn match l65Keyword /\<isbzpx\>/
syn match l65Keyword /\<isbabs\>/
syn match l65Keyword /\<isbabx\>/
syn match l65Keyword /\<isbaby\>/
syn match l65Keyword /\<isbinx\>/
syn match l65Keyword /\<isbiny\>/
syn match l65Keyword /\<jamimp\>/
syn match l65Keyword /\<lasaby\>/
syn match l65Keyword /\<laxzpg\>/
syn match l65Keyword /\<laxzpy\>/
syn match l65Keyword /\<laxabs\>/
syn match l65Keyword /\<laxaby\>/
syn match l65Keyword /\<laxinx\>/
syn match l65Keyword /\<laxiny\>/
syn match l65Keyword /\<nopimm\>/
syn match l65Keyword /\<nopzpg\>/
syn match l65Keyword /\<nopzpx\>/
syn match l65Keyword /\<nopabs\>/
syn match l65Keyword /\<nopabx\>/
syn match l65Keyword /\<rlazpg\>/
syn match l65Keyword /\<rlazpx\>/
syn match l65Keyword /\<rlaabs\>/
syn match l65Keyword /\<rlaabx\>/
syn match l65Keyword /\<rlaaby\>/
syn match l65Keyword /\<rlainx\>/
syn match l65Keyword /\<rlainy\>/
syn match l65Keyword /\<rrazpg\>/
syn match l65Keyword /\<rrazpx\>/
syn match l65Keyword /\<rraabs\>/
syn match l65Keyword /\<rraabx\>/
syn match l65Keyword /\<rraaby\>/
syn match l65Keyword /\<rrainx\>/
syn match l65Keyword /\<rrainy\>/
syn match l65Keyword /\<saxzpg\>/
syn match l65Keyword /\<saxzpy\>/
syn match l65Keyword /\<saxabs\>/
syn match l65Keyword /\<saxinx\>/
syn match l65Keyword /\<sbximm\>/
syn match l65Keyword /\<shaaby\>/
syn match l65Keyword /\<shainy\>/
syn match l65Keyword /\<shsaby\>/
syn match l65Keyword /\<shxaby\>/
syn match l65Keyword /\<shyabx\>/
syn match l65Keyword /\<slozpg\>/
syn match l65Keyword /\<slozpx\>/
syn match l65Keyword /\<sloabs\>/
syn match l65Keyword /\<sloabx\>/
syn match l65Keyword /\<sloaby\>/
syn match l65Keyword /\<sloinx\>/
syn match l65Keyword /\<sloiny\>/
syn match l65Keyword /\<srezpg\>/
syn match l65Keyword /\<srezpx\>/
syn match l65Keyword /\<sreabs\>/
syn match l65Keyword /\<sreabx\>/
syn match l65Keyword /\<sreaby\>/
syn match l65Keyword /\<sreinx\>/
syn match l65Keyword /\<sreiny\>/


syn match l65Opcode /\<adc\%(.[bw]\)\=\>/
syn match l65Opcode /\<and\%(.[bw]\)\=\>/
syn match l65Opcode /\<asl\%(.[bw]\)\=\>/
syn match l65Opcode /\<bcc\>/
syn match l65Opcode /\<bcs\>/
syn match l65Opcode /\<beq\>/
syn match l65Opcode /\<bit\%(.[bw]\)\=\>/
syn match l65Opcode /\<bmi\>/
syn match l65Opcode /\<bne\>/
syn match l65Opcode /\<bpl\>/
syn match l65Opcode /\<brk\>/
syn match l65Opcode /\<bvc\>/
syn match l65Opcode /\<bvs\>/
syn match l65Opcode /\<clc\>/
syn match l65Opcode /\<cld\>/
syn match l65Opcode /\<cli\>/
syn match l65Opcode /\<clv\>/
syn match l65Opcode /\<cmp\%(.[bw]\)\=\>/
syn match l65Opcode /\<cpx\%(.[bw]\)\=\>/
syn match l65Opcode /\<cpy\%(.[bw]\)\=\>/
syn match l65Opcode /\<dec\%(.[bw]\)\=\>/
syn match l65Opcode /\<dex\>/
syn match l65Opcode /\<dey\>/
syn match l65Opcode /\<eor\%(.[bw]\)\=\>/
syn match l65Opcode /\<inc\%(.[bw]\)\=\>/
syn match l65Opcode /\<inx\>/
syn match l65Opcode /\<iny\>/
syn match l65Opcode /\<jmp\>/
syn match l65Opcode /\<jsr\>/
syn match l65Opcode /\<lda\%(.[bw]\)\=\>/
syn match l65Opcode /\<ldx\%(.[bw]\)\=\>/
syn match l65Opcode /\<ldy\%(.[bw]\)\=\>/
syn match l65Opcode /\<lsr\%(.[bw]\)\=\>/
syn match l65Opcode /\<nop\%(.[bw]\)\=\>/
syn match l65Opcode /\<ora\%(.[bw]\)\=\>/
syn match l65Opcode /\<pha\>/
syn match l65Opcode /\<php\>/
syn match l65Opcode /\<pla\>/
syn match l65Opcode /\<plp\>/
syn match l65Opcode /\<rol\%(.[bw]\)\=\>/
syn match l65Opcode /\<ror\%(.[bw]\)\=\>/
syn match l65Opcode /\<rti\>/
syn match l65Opcode /\<rts\>/
syn match l65Opcode /\<sbc\%(.[bw]\)\=\>/
syn match l65Opcode /\<sec\>/
syn match l65Opcode /\<sed\>/
syn match l65Opcode /\<sei\>/
syn match l65Opcode /\<sta\%(.[bw]\)\=\>/
syn match l65Opcode /\<stx\%(.[bw]\)\=\>/
syn match l65Opcode /\<sty\%(.[bw]\)\=\>/
syn match l65Opcode /\<tax\>/
syn match l65Opcode /\<tay\>/
syn match l65Opcode /\<tsx\>/
syn match l65Opcode /\<txa\>/
syn match l65Opcode /\<txs\>/
syn match l65Opcode /\<tya\>/

syn match l65Opcode /\<anc\>/
syn match l65Opcode /\<ane\>/
syn match l65Opcode /\<arr\>/
syn match l65Opcode /\<asr\>/
syn match l65Opcode /\<dcp\%(.[bw]\)\=\>/
syn match l65Opcode /\<isb\%(.[bw]\)\=\>/
syn match l65Opcode /\<jam\>/
syn match l65Opcode /\<las\>/
syn match l65Opcode /\<lax\%(.[bw]\)\=\>/
syn match l65Opcode /\<rla\%(.[bw]\)\=\>/
syn match l65Opcode /\<rra\%(.[bw]\)\=\>/
syn match l65Opcode /\<sax\%(.[bw]\)\=\>/
syn match l65Opcode /\<sbx\>/
syn match l65Opcode /\<sha\>/
syn match l65Opcode /\<shs\>/
syn match l65Opcode /\<shx\>/
syn match l65Opcode /\<shy\>/
syn match l65Opcode /\<slo\%(.[bw]\)\=\>/
syn match l65Opcode /\<sre\%(.[bw]\)\=\>/

" Define the default highlighting.
" For version 5.7 and earlier: only when not done already
" For version 5.8 and later: only when an item doesn't have highlighting yet
if version >= 508 || !exists("did_l65_syntax_inits")
  if version < 508
    let did_l65_syntax_inits = 1
    command -nargs=+ HiLink hi link <args>
  else
    command -nargs=+ HiLink hi def link <args>
  endif

  HiLink luaStatement		Statement
  HiLink luaRepeat		Repeat
  HiLink luaFor			Repeat
  HiLink luaString		String
  HiLink luaString2		String
  HiLink luaNumber		Number
  HiLink luaOperator		Operator
  HiLink luaIn			Operator
  HiLink luaConstant		Constant
  HiLink luaCond		Conditional
  HiLink luaElse		Conditional
  HiLink luaFunction		Function
  HiLink luaComment		Comment
  HiLink luaTodo		Todo
  HiLink luaTable		Structure
  HiLink luaError		Error
  HiLink luaParenError		Error
  HiLink luaBraceError		Error
  HiLink luaSpecial		SpecialChar
  HiLink luaFunc		Identifier
  HiLink luaLabel		Label

  HiLink luaSemiCol             Delimiter
  
  HiLink l65PreProc		PreProc
  HiLink l65Label		Special
  HiLink l65Keyword		Identifier
  HiLink l65Opcode		Type
  HiLink l65SChar		Operator
  HiLink l65SFunc		Function

  delcommand HiLink
endif

let b:current_syntax = "l65"

let &cpo = s:cpo_save
unlet s:cpo_save
" vim: et ts=8 sw=2
