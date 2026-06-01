" Vim syntax file
" Language: l7801

if exists("b:current_syntax")
  finish
endif

runtime! syntax/l65.vim
unlet! b:current_syntax

syn keyword l65Opcode aci adc adcw adcx adcxd adcxi add addnc addncw addncx addncxd addncxi addw addx addxd addxi adi
syn keyword l65Opcode ana anaw anax anaxd anaxi ani bit0 bit1 bit2 bit3 bit4 bit5 bit6 bit7 block calb calf call calt
syn keyword l65Opcode clc daa dcr dcrw dcx di ei eqa eqaw eqax eqaxd eqaxi eqi eqiw ex exx gta gtaw gtax gtaxd gtaxi
syn keyword l65Opcode halt in inr inrw inx jb jmp jr jre lbcd lded ldaw ldax ldaxd ldaxi lhld lspd lta ltaw ltax ltaxd ltaxi
syn keyword l65Opcode lxi mov mvi mviw mvix nea neaw neax neaxd neaxi nei nop offa offaw offax offaxd offaxi offi ona onaw
syn keyword l65Opcode onax onaxd onaxi oni ora oraw orax oraxd oraxi ori out pen per pex pop push ret reti rets rld rll rlr rrd
syn keyword l65Opcode sbb sbbw sbbx sbbxd sbbxi sbcd sbi sded shld sio skc skit sknc sknit sknz skz sll slr softi sspd staw
syn keyword l65Opcode stax staxd staxi stc stm sub subnb subnbw subnbx subnbxd subnbxi subw subx subxd subxi sui suinb table
syn keyword l65Opcode xra xraw xrax xraxd xraxi xri

syn keyword l65Keyword a b c d e h l v bc de hl sp va pa pb pc mk mb mc tm0 tm1 s

let b:current_syntax = "l7801"
