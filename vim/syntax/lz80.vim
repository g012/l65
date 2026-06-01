" Vim syntax file
" Language: lz80

if exists("b:current_syntax")
  finish
endif

runtime! syntax/l65.vim
unlet! b:current_syntax

" Lua comments require whitespace or punctuation before -- in lz80 because an
" attached -- is the postfix decrement operator (for example, c-- or [hl]--).
syn clear luaComment
syn match luaComment "\%(\<\%(a\|b\|c\|d\|e\|h\|l\|i\|r\|af\|bc\|de\|hl\|sp\|ix\|iy\|af2\)\>\|\]\)\@<!--.*$" contains=luaTodo,@Spell
syn region luaComment matchgroup=luaComment start="--\[\z(=*\)\[" end="\]\z1\]" contains=luaTodo,@Spell
syn match luaComment "\%^#!.*"

syn keyword l65Opcode adc add ana and bit call ccf cp cpd cpdr cpi cpir cpl daa dec di djnz ei ex exx halt im in inc ind indr
syn keyword l65Opcode ini inir jp jr ld ldd lddr ldh ldi ldir neg nop or ora otdr otir out outd outi pop push res ret reti retn
syn keyword l65Opcode rl rla rlc rlca rld rr rra rrc rrca rrd rst sbc scf set sla sra srl stop sub swap xor

syn keyword l65Keyword a b c d e h l i r af bc de hl sp ix iy af2 nz z nc po pe p m imm mem idx

" Dotted conditions retain the normal mnemonic and condition highlighting.
syn match lz80Operator "\%(\<jr\|\<jp\|\<call\)\@<=\.\ze\%(nz\|z\|nc\|c\)\>"

" Assignment and arithmetic operators.
syn match lz80Operator "\%(:=\|+=\|-=\|&=\||=\|\^=\)"

" Attached -- is decrement; a spaced -- remains a Lua comment.
syn match lz80Operator "\%(\<\%(a\|b\|c\|d\|e\|h\|l\|i\|r\|af\|bc\|de\|hl\|sp\|ix\|iy\|af2\)\>\|\]\)\@<=++"
syn match lz80Operator "\%(\<\%(a\|b\|c\|d\|e\|h\|l\|i\|r\|af\|bc\|de\|hl\|sp\|ix\|iy\|af2\)\>\|\]\)\@<=--"

" Auto-incrementing/decrementing HL memory operands.
syn match lz80Operator "[+-]\ze\s*\]"

hi def link lz80Operator Operator

let b:current_syntax = "lz80"
