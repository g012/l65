M = require "asm"

-- Return a value in rage [0x00, 0xff] if x is to use zeropage addressing mode. Defaults to range [0x0000-0x00ff].
M.zeropage = function(x) if x >= -128 and x <= 0xff then return M.byte_normalize(x) end end

local op_eval_byte = function(late, early, nozp)
    local v = op_eval(late, early)
    local zpv = zeropage(v)
    if not nozp and zpv then return zpv end
    return byte_normalize(v)
end
M.op_eval_byte = op_eval_byte

local op_eval_word = function(late, early) return word_normalize(op_eval(late, early)) end
M.op_eval_word = op_eval_word

local cycles_def,xcross_def

cycles_def=2 xcross_def=0 local opimp={
    asl=M.op(0x0a), brk=M.op(0x00,7), clc=M.op(0x18), cld=M.op(0xd8), cli=M.op(0x58), clv=M.op(0xb8), dex=M.op(0xca), dey=M.op(0x88),
    inx=M.op(0xe8), iny=M.op(0xc8), lsr=M.op(0x4a), nop=M.op(0xea), pha=M.op(0x48,3), php=M.op(0x08,3), pla=M.op(0x68,4), plp=M.op(0x28,4),
    rol=M.op(0x2a), ror=M.op(0x6a), rti=M.op(0x40,6), rts=M.op(0x60,6), sec=M.op(0x38), sei=M.op(0x78), tax=M.op(0xaa), tay=M.op(0xa8),
    tsx=M.op(0xba), txa=M.op(0x8a), txs=M.op(0x9a), tya=M.op(0x98),
    jam=M.op(0x02,0),
} M.opimp = opimp
for k,v in pairs(opimp) do
    M[k .. 'imp'] = function()
        table.insert(M.section_current.instructions, { size=1, cycles=v.cycles, bin=v.opc })
    end
end

cycles_def=2 xcross_def=0 local opimm={
    adc=M.op(0x69), ['and']=M.op(0x29), cmp=M.op(0xc9), cpx=M.op(0xe0), cpy=M.op(0xc0), eor=M.op(0x49), lda=M.op(0xa9), ldx=M.op(0xa2),
    ldy=M.op(0xa0), ora=M.op(0x09), sbc=M.op(0xe9),
    anc=M.op(0x0b), ane=M.op(0x8b), arr=M.op(0x6b), asr=M.op(0x4b), jam=M.op(0x12,0), lax=M.op(0xab), nop=M.op(0x80), sbx=M.op(0xcb),
} M.opimm = opimm
for k,v in pairs(opimm) do
    M[k .. 'imm'] = function(late, early)
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local size = function() late,early = M.size_op(late,early) return 2 end
        local bin = function() local l65dbg=l65dbg return { v.opc, M.op_eval_byte(late,early,true) } end
        table.insert(M.section_current.instructions, { size=size, cycles=2, bin=bin })
    end
end

cycles_def=3 xcross_def=0 local opzpg={
    adc=M.op(0x65), ['and']=M.op(0x25), asl=M.op(0x06,5), bit=M.op(0x24), cmp=M.op(0xc5), cpx=M.op(0xe4), cpy=M.op(0xc4), dec=M.op(0xc6,5),
    eor=M.op(0x45), inc=M.op(0xe6,5), lda=M.op(0xa5), ldx=M.op(0xa6), ldy=M.op(0xa4), lsr=M.op(0x46,5), ora=M.op(0x05), rol=M.op(0x26,5),
    ror=M.op(0x66,5), sbc=M.op(0xe5), sta=M.op(0x85), stx=M.op(0x86), sty=M.op(0x84), 
    dcp=M.op(0xc7,5), isb=M.op(0xe7,5), jam=M.op(0x22,0), lax=M.op(0xa7), nop=M.op(0x04), rla=M.op(0x27,5), rra=M.op(0x67,5), sax=M.op(0x87),
    slo=M.op(0x07,5), sre=M.op(0x47,5),
} M.opzpg = opzpg
for k,v in pairs(opzpg) do
    M[k .. 'zpg'] = function(late, early)
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local size = function() late,early = M.size_op(late,early) return 2 end
        local bin = function() local l65dbg=l65dbg return { v.opc, M.op_eval_byte(late,early) } end
        table.insert(M.section_current.instructions, { size=size, cycles=v.cycles, bin=bin })
    end
end

cycles_def=4 xcross_def=0 local opabs={
    adc=M.op(0x6d), ['and']=M.op(0x2d), asl=M.op(0x0e,6), bit=M.op(0x2c), cmp=M.op(0xcd), cpx=M.op(0xec), cpy=M.op(0xcc), dec=M.op(0xce,6),
    eor=M.op(0x4d), inc=M.op(0xee,6), jmp=M.op(0x4c,3), jsr=M.op(0x20,6), lda=M.op(0xad), ldx=M.op(0xae), ldy=M.op(0xac), lsr=M.op(0x4e,6),
    ora=M.op(0x0d), rol=M.op(0x2e,6), ror=M.op(0x6e,6), sbc=M.op(0xed), sta=M.op(0x8d), stx=M.op(0x8e), sty=M.op(0x8c),
    dcp=M.op(0xcf,6), isb=M.op(0xef,6), jam=M.op(0x72,0), lax=M.op(0xaf), nop=M.op(0x0c), rla=M.op(0x2f,6), rra=M.op(0x6f,6), sax=M.op(0x8f),
    slo=M.op(0x0f,6), sre=M.op(0x4f,6),
} M.opabs = opabs
for k,v in pairs(opabs) do
    M[k .. 'abs'] = function(late, early)
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local size = function() late,early = M.size_op(late,early) return 3 end
        local bin = function() local l65dbg=l65dbg 
            local x = M.op_eval_word(late,early)
            return { v.opc, x&0xff, x>>8 }
        end
        table.insert(M.section_current.instructions, { size=size, cycles=v.cycles, bin=bin })
    end
end
local opzab={} M.opabs = opabs
for k,_ in pairs(opzpg) do if opabs[k] then opzab[k]=true end end
for k,_ in pairs(opzab) do
    M[k .. 'zab'] = function(late, early)
        if type(late) ~= 'function' then
            local x = (early or 0) + late
            if zeropage(x) then return M[k .. 'zpg'](late, early) end
            if x >= -32768 and x <= 0xffff then return M[k .. 'abs'](late, early) end
            error("value out of word range: " .. x)
        end
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local abs = opabs[k]
        local ins = { cycles=abs.cycles }
        ins.size = function() local l65dbg=l65dbg 
            local r,x = M.pcall_za(M.op_eval, late, early)
            if not r then return 3 end
            M.size_ref(x)
            x = M.word_normalize(x)
            local zpg,zpv = opzpg[k], zeropage(x)
            if zpv and zpg then
                ins.size = 2
                ins.cycles = zpg.cycles
                ins.bin = function() return { zpg.opc, zpv } end
                return 2
            end
            ins.size = 3
            ins.bin = function() return { abs.opc, x&0xff, x>>8 } end
            return 3
        end
        ins.bin = function() local l65dbg=l65dbg 
            local x = M.word_normalize(M.op_eval(late, early))
            -- since we assumed absolute on link phase, we must generate absolute in binary
            if zeropage(x) and opzpg[k] then io.stderr:write("warning: forcing abs on zpg operand for opcode " .. k .. "\n") end
            return { abs.opc, x&0xff, x>>8 }
        end
        table.insert(M.section_current.instructions, ins)
    end
end

cycles_def=4 xcross_def=0 local opzpx={
    adc=M.op(0x75), ['and']=M.op(0x35), asl=M.op(0x16,6), cmp=M.op(0xd5), dec=M.op(0xd6,6), eor=M.op(0x55), inc=M.op(0xf6,6), lda=M.op(0xb5),
    ldy=M.op(0xb4), lsr=M.op(0x56,6), ora=M.op(0x15), rol=M.op(0x36,6), ror=M.op(0x76,6), sbc=M.op(0xf5), sta=M.op(0x95), sty=M.op(0x94),
    dcp=M.op(0xd7,6), isb=M.op(0xf7,6), jam=M.op(0x32,0), nop=M.op(0x14), rla=M.op(0x37,6), rra=M.op(0x77,6), slo=M.op(0x17,6), sre=M.op(0x57,6),
} M.opzpx = opzpx
for k,v in pairs(opzpx) do
    M[k .. 'zpx'] = function(late, early)
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local size = function() late,early = M.size_op(late,early) return 2 end
        local bin = function() local l65dbg=l65dbg return { v.opc, M.op_eval_byte(late,early) } end
        table.insert(M.section_current.instructions, { size=size, cycles=v.cycles, bin=bin })
    end
end

cycles_def=4 xcross_def=1 local opabx={
    adc=M.op(0x7d), ['and']=M.op(0x3d), asl=M.op(0x1e,7,0), cmp=M.op(0xdd), dec=M.op(0xde,7,0), eor=M.op(0x5d), inc=M.op(0xfe,7,0), lda=M.op(0xbd),
    ldy=M.op(0xbc), lsr=M.op(0x5e,7,0), ora=M.op(0x1d), rol=M.op(0x3e,7,0), ror=M.op(0x7e,7,0), sbc=M.op(0xfd), sta=M.op(0x9d,5,0),
    dcp=M.op(0xdf,7,0), isb=M.op(0xff,7,0), jam=M.op(0x92,0,0), nop=M.op(0x1c), rla=M.op(0x3f,7,0), rra=M.op(0x7f,7,0), shy=M.op(0x9c,5,0), slo=M.op(0x1f,7,0),
    sre=M.op(0x5f,7,0),
} M.opabx = opabx
for k,v in pairs(opabx) do
    M[k .. 'abx'] = function(late, early)
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local size = function() late,early = M.size_op(late,early) return 3 end
        local bin = function() local l65dbg=l65dbg 
            local x = M.op_eval_word(late,early)
            return { v.opc, x&0xff, x>>8 }
        end
        table.insert(M.section_current.instructions, { size=size, cycles=v.cycles, bin=bin })
    end
end
local opzax={} M.opabx = opabx
for k,_ in pairs(opzpx) do if opabx[k] then opzax[k]=true end end
for k,_ in pairs(opzax) do
    M[k .. 'zax'] = function(late, early)
        if type(late) ~= 'function' then
            local x = (early or 0) + late
            if zeropage(x) then return M[k .. 'zpx'](late, early) end
            if x >= -32768 and x <= 0xffff then return M[k .. 'abx'](late, early) end
            error("value out of word range: " .. x)
        end
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local abx = opabx[k]
        local ins = { cycles=abx.cycles }
        ins.size = function() local l65dbg=l65dbg 
            local r,x = M.pcall_za(M.op_eval, late, early)
            if not r then return 3 end
            M.size_ref(x)
            x = M.word_normalize(x)
            local zpx,zpv = opzpx[k], zeropage(x)
            if zpv and zpx then
                ins.size = 2
                ins.cycles = zpx.cycles
                ins.bin = function() return { zpx.opc, zpv } end
                return 2
            end
            ins.size = 3
            ins.bin = function() return { abx.opc, x&0xff, x>>8 } end
            return 3
        end
        ins.bin = function() local l65dbg=l65dbg
            local x = M.word_normalize(M.op_eval(late, early))
            -- since we assumed absolute on link phase, we must generate absolute in binary
            if zeropage(x) and opzpx[k] then io.stderr:write("warning: forcing abx on zpx operand for opcode " .. k .. "\n") end
            return { abx.opc, x&0xff, x>>8 }
        end
        table.insert(M.section_current.instructions, ins)
    end
end

cycles_def=4 xcross_def=0 local opzpy={
    ldx=M.op(0xb6), stx=M.op(0x96),
    jam=M.op(0x42,0), lax=M.op(0xb7), sax=M.op(0x97),
} M.opzpy = opzpy
for k,v in pairs(opzpy) do
    M[k .. 'zpy'] = function(late, early)
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local size = function() late,early = M.size_op(late,early) return 2 end
        local bin = function() local l65dbg=l65dbg return { v.opc, M.op_eval_byte(late,early) } end
        table.insert(M.section_current.instructions, { size=size, cycles=v.cycles, bin=bin })
    end
end

cycles_def=4 xcross_def=1 local opaby={
    adc=M.op(0x79), ['and']=M.op(0x39), cmp=M.op(0xd9), eor=M.op(0x59), lda=M.op(0xb9), ldx=M.op(0xbe), ora=M.op(0x19), sbc=M.op(0xf9),
    sta=M.op(0x99,5,0), 
    dcp=M.op(0xdb,7,0), isb=M.op(0xfb,7,0), jam=M.op(0xb2,0,0), las=M.op(0xbb), lax=M.op(0xbf), rla=M.op(0x3b,7,0), rra=M.op(0x7b,7,0), sha=M.op(0x9f,5,0),
    shs=M.op(0x9b,5,0), shx=M.op(0x9e,5,0), slo=M.op(0x1b,7,0), sre=M.op(0x5b,7,0),
} M.opaby = opaby
for k,v in pairs(opaby) do
    M[k .. 'aby'] = function(late, early)
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local size = function() late,early = M.size_op(late,early) return 3 end
        local bin = function() local l65dbg=l65dbg
            local x = M.op_eval_word(late,early)
            return { v.opc, x&0xff, x>>8 }
        end
        table.insert(M.section_current.instructions, { size=size, cycles=v.cycles, bin=bin })
    end
end
local opzay={} M.opaby = opaby
for k,_ in pairs(opzpy) do if opaby[k] then opzay[k]=true end end
for k,_ in pairs(opzay) do
    M[k .. 'zay'] = function(late, early)
        if type(late) ~= 'function' then
            local x = (early or 0) + late
            if zeropage(x) then return M[k .. 'zpy'](late, early) end
            if x >= -32768 and x <= 0xffff then return M[k .. 'aby'](late, early) end
            error("value out of word range: " .. x)
        end
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local aby = opaby[k]
        local ins = { cycles=aby.cycles }
        ins.size = function() local l65dbg=l65dbg
            local r,x = M.pcall_za(M.op_eval, late, early)
            if not r then return 3 end
            M.size_ref(x)
            x = M.word_normalize(x)
            local zpy,zpv = opzpy[k], zeropage(x)
            if zpv and zpy then
                ins.size = 2
                ins.cycles = zpy.cycles
                ins.bin = function() return { zpy.opc, zpv } end
                return 2
            end
            ins.size = 3
            ins.bin = function() return { aby.opc, x&0xff, x>>8 } end
            return 3
        end
        ins.bin = function() local l65dbg=l65dbg
            local x = M.word_normalize(M.op_eval(late, early))
            -- since we assumed absolute on link phase, we must generate absolute in binary
            if zeropage(x) and opzpy[k] then io.stderr:write("warning: forcing aby on zpy operand for opcode " .. k .. "\n") end
            return { aby.opc, x&0xff, x>>8 }
        end
        table.insert(M.section_current.instructions, ins)
    end
end

cycles_def=2 xcross_def=0 local oprel={
    bcc=M.op(0x90), bcs=M.op(0xb0), beq=M.op(0xf0), bmi=M.op(0x30), bne=M.op(0xd0), bpl=M.op(0x10), bvc=M.op(0x50), bvs=M.op(0x70),
} M.oprel = oprel
for k,v in pairs(oprel) do
    M[k .. 'rel'] = function(label)
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local parent,offset = M.label_current
        local section,rorg = M.section_current,M.location_current.rorg
        local op = { cycles=2 }
        op.size = function()
            offset = section.size
            label = M.size_dc(label)
            return 2
        end
        op.bin = function() local l65dbg=l65dbg 
            local x,l = label,label
            if type(x) == 'function' then x=x() end
            if type(x) == 'string' then
                if x:sub(1,1) == '_' then x=parent..x l=x end
                x = symbols[x]
            end
            if type(x) ~= 'number' then error("unresolved branch target: " .. tostring(x)) end
            x = x-2 - offset - rorg(section.org)
            if x < -128 or x > 127 then error("branch target out of range for " .. l .. ": " .. x) end
            return { v.opc, x&0xff }
        end
        table.insert(M.section_current.instructions, op)
    end
end

cycles_def=5 xcross_def=0 local opind={
    jmp=M.op(0x6c),
    jam=M.op(0xd2,0),
} M.opind = opind
for k,v in pairs(opind) do
    M[k .. 'ind'] = function(late, early)
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local size = function() late,early = M.size_op(late,early) return 3 end
        local bin = function() local l65dbg=l65dbg
            local x = M.op_eval_word(late,early)
            return { v.opc, x&0xff, x>>8 }
        end
        table.insert(M.section_current.instructions, { size=size, cycles=v.cycles, bin=bin })
    end
end

cycles_def=6 xcross_def=0 local opinx={
    adc=M.op(0x61), ['and']=M.op(0x21), cmp=M.op(0xc1), eor=M.op(0x41), lda=M.op(0xa1), ora=M.op(0x01), sbc=M.op(0xe1), sta=M.op(0x81),
    dcp=M.op(0xc3,8), isb=M.op(0xe3,8), jam=M.op(0x52,0), lax=M.op(0xa3), rla=M.op(0x23,8), rra=M.op(0x63,8), sax=M.op(0x83), slo=M.op(0x03,8),
    sre=M.op(0x43,8),
} M.opinx = opinx
for k,v in pairs(opinx) do
    M[k .. 'inx'] = function(late, early)
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local size = function() late,early = M.size_op(late,early) return 2 end
        local bin = function() local l65dbg=l65dbg return { v.opc, M.op_eval_byte(late,early) } end
        table.insert(M.section_current.instructions, { size=size, cycles=v.cycles, bin=bin })
    end
end

cycles_def=5 xcross_def=1 local opiny={
    adc=M.op(0x71), ['and']=M.op(0x31), cmp=M.op(0xd1), eor=M.op(0x51), lda=M.op(0xb1), ora=M.op(0x11), sbc=M.op(0xf1), sta=M.op(0x91,6),
    dcp=M.op(0xd3,8), isb=M.op(0xf3,8), jam=M.op(0x62,0,0), lax=M.op(0xb3), rla=M.op(0x33,8), rra=M.op(0x73,8), sha=M.op(0x93,6), slo=M.op(0x13,8),
    sre=M.op(0x53,8),
}
for k,v in pairs(opiny) do
    M[k .. 'iny'] = function(late, early)
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local size = function() late,early = M.size_op(late,early) return 2 end
        local bin = function() local l65dbg=l65dbg return { v.opc, M.op_eval_byte(late,early) } end
        table.insert(M.section_current.instructions, { size=size, cycles=v.cycles, bin=bin })
    end
end

return M
