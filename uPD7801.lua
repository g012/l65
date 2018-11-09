M = require "asm"

local op_eval_byte = function(late, early) return M.byte_normalize(M.op_eval(late, early)) end
M.op_eval_byte = op_eval_byte

local op_eval_word = function(late, early) return M.word_normalize(M.op_eval(late, early)) end
M.op_eval_word = op_eval_word

local opimp={
    block=M.op(0x31,13),
    calb=M.op(0x63,13),
    daa=M.op(0x61,4),
    ex=M.op(0x10,4),
    exx=M.op(0x11,4),
    halt=M.op(0x01,6),
	jb=M.op(0x73,4),
    nop=M.op(0x00,4),
    ret=M.op(0x08,11),
    reti=M.op(0x62,15),
    rets=M.op(0x18,11),
    sio=M.op(0x09,4),
    softi=M.op(0x72,19),
    stm=M.op(0x19,4),
    ['table']=M.op(0x21,19)
} M.opimp = opimp
for k,v in pairs(opimp) do
    M[k .. 'imp' ] = function()
        table.insert(M.section_current.instructions, { size=1, cycles=v.cycles, bin=v.opc })
    end
end

local opa={
    dcr=M.op(0x51,4),
    inr=M.op(0x41,4),
} M.opa = opa
for k,v in pairs(opa) do
    M[k .. 'a'] = function()
        table.insert(M.section_current.instructions, { size=1, cycles=v.cycles, bin=v.opc })
    end
end

local opb={
    dcr=M.op(0x52,4),
    inr=M.op(0x42,4),
} M.opb = opb
for k,v in pairs(opb) do
    M[k .. 'b'] = function()
        table.insert(M.section_current.instructions, { size=1, cycles=v.cycles, bin=v.opc })
    end
end

local opc={
    dcr=M.op(0x53,4),
    inr=M.op(0x43,4),
} M.opc = opc
for k,v in pairs(opc) do
    M[k .. 'c'] = function()
        table.insert(M.section_current.instructions, { size=1, cycles=v.cycles, bin=v.opc })
    end
end

local opsp={
    dcx=M.op(0x03,7),
    inx=M.op(0x02,7)
} M.opsp = opsp
for k,v in pairs(opsp) do
    M[k .. 'sp'] = function()
        table.insert(M.section_current.instructions, { size=1, cycles=v.cycles, bin=v.opc })
    end
end

local opregxx ={
    mvib=M.op(0x6a,7),
    mvic=M.op(0x6b,7),
    mvid=M.op(0x6c,7),
    mvie=M.op(0x6d,7),
    mvih=M.op(0x6e,7),
    mvil=M.op(0x6f,7),
    mviv=M.op(0x68,7)
} M.opregxx = opregxx
for k,v in pairs(opregxx) do
    M[k] = function(late, early)
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local size = function() late,early = M.size_op(late,early) return 2 end
        local bin = function() local l65dbg=l65dbg return { v.opc, M.op_eval_byte(late,early) } end
        table.insert(M.section_current.instructions, { size=size, cycles=v.cycles, bin=bin })
    end
end

local opaxx ={
    aci=M.op(0x56,7),
    adi=M.op(0x46,7),
    adinc=M.op(0x26,7),
    ani=M.op(0x07,7),
    eqi=M.op(0x77,7),    
    gti=M.op(0x27,7),
    lti=M.op(0x37,7),
    mvi=M.op(0x69,7),
    nei=M.op(0x67,7),
    offi=M.op(0x57,7),
    oni=M.op(0x47,7),
    ori=M.op(0x17,7),
    sbi=M.op(0x76,7),
    sui=M.op(0x66,7),
    suinb=M.op(0x36,7),
    xri=M.op(0x16,7)
} M.opaxx = opaxx
for k,v in pairs(opaxx) do
    M[k .. 'a'] = function(late, early)
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local size = function() late,early = M.size_op(late,early) return 2 end
        local bin = function() local l65dbg=l65dbg return { v.opc, M.op_eval_byte(late,early) } end
        table.insert(M.section_current.instructions, { size=size, cycles=v.cycles, bin=bin })
    end
end

local opr8r8 ={
    movab=M.op(0x0a,4), movac=M.op(0x0b,4),
    movad=M.op(0x0c,4), movae=M.op(0x0d,4),
    movah=M.op(0x0e,4), moval=M.op(0x0f,4),
    movba=M.op(0x1a,4), movca=M.op(0x1b,4),
    movda=M.op(0x1c,4), movea=M.op(0x1d,4),
    movha=M.op(0x1e,4), movla=M.op(0x1f,4),
} M.opr8r8 = opr8r8
for k,v in pairs(opr8r8) do
    M[k] = function()
        table.insert(M.section_current.instructions, { size=1, cycles=v.cycles, bin=v.opc })
    end
end

local opw = {
    call=M.op(0x44,16),
    jmp=M.op(0x54,10),
    lxisp=M.op(0x04,10),
    lxibc=M.op(0x14,10),
    lxide=M.op(0x24,10),
    lxihl=M.op(0x34,10)    
} M.opw = opw
for k,v in pairs(opw) do
    M[k] = function(late, early)
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local size = function() late,early = M.size_op(late,early) return 3 end
        local bin = function() local l65dbg=l65dbg 
            local x = M.op_eval_word(late,early)
            return { v.opc, x&0xff, x>>8 }
        end
        table.insert(M.section_current.instructions, { size=size, cycles=v.cycles, bin=bin })
    end
end

M.calt = function(late, early)
    local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
    local op = { cycles=19 }
    op.size = function() late,early = M.size_op(late,early) return 1 end
    op.bin = function() 
        local l65dbg=l65dbg
        local x = M.op_eval_byte(late,early)
        if (x%2 == 1) then error("offset should be even : " .. x) end
        if x < 0x80 or x > 0xfe then error("offset out of range : " .. x) end
        x = (x>>1) + 0x40
        return x
    end
    table.insert(M.section_current.instructions, op)
end

M.calf = function(late, early)
    local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
    local op = { cycles=16 }
    op.size = function() late,early = M.size_op(late,early) return 2 end
    op.bin = function() local l65dbg=l65dbg 
        local x = 0 + M.op_eval_word(late,early)
        if x < 0x0800 or x > 0xffff then error("subroutine address out of range [0x0800-0xffff]: " .. x) end
        x = x - 0x0800;
        return { 0x78 | ((x>>8) & 0x07), x&0xff }
    end
    table.insert(M.section_current.instructions, op)
end

M.jr = function(label)
    local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
    local parent,offset = M.label_current
    local section,rorg = M.section_current,M.location_current.rorg
    local op = { cycles=13 }
    op.size = function()
        offset = section.size
        label = M.size_dc(label)
        return 1
    end
    op.bin = function() 
        local l65dbg=l65dbg 
        local x,l = label,label
        if type(x) == 'function' then x=x() end
        if type(x) == 'string' then
            if x:sub(1,1) == '_' then x=parent..x l=x end
            x = symbols[x]
        end
        if type(x) ~= 'number' then error("unresolved branch target: " .. tostring(x)) end
        x = x-1 - offset - rorg(section.org)
        if x < -32 or x > 0x32 then error("branch target out of range for " .. l .. ": " .. x)
        elseif x >= 0 then
            x = 0xc0 + x
            return 
        else
            return x & 0xff
        end
    end
    table.insert(M.section_current.instructions, op)
end

local op48imp = {
    ei=M.op(0x20,8),
    di=M.op(0x24,8),
    clc=M.op(0x2a,8),
    pen=M.op(0x2c,11),
    per=M.op(0x3c,11),
    pex=M.op(0x2d,11),
    rld=M.op(0x38,17),
    rrd=M.op(0x39,17),
    stc=M.op(0x2b,8),
} M.op48imp = op48imp
for k,v in pairs(op48imp) do
    M[k .. 'imp'] = function()
        table.insert(M.section_current.instructions, { size=2, cycles=v.cycles, bin={ 0x48, v.opc } })
    end
end

return M

--[[ [todo]
8 bits instructions:
    JRE+ 0x4e xx 17
    JRE- 0x4f xx 17
    
    -- d
    ldaxm=M.op(0x2e,7),
    ldaxp=M.op(0x2c,7), 
    staxm=M.op(0x3e,7),
    staxp=M.op(0x3c,7)
    
    -- h
    ldaxm=M.op(0x2f,7),
    ldaxp=M.op(0x2d,7),
    staxm=M.op(0x3f,7),
    staxp=M.op(0x3d,7)

    -- wa = v+xx
    inrw=M.op(0x20,13),
    ldaw=M.op(0x28,10),
    dcrw=M.op(0x30,13),
    staw=M.op(0x38,10),
    bit0=M.op(0x58,10),
    bit1=M.op(0x59,10),
    bit2=M.op(0x5a,10),
    bit3=M.op(0x5b,10),
    bit4=M.op(0x5c,10),
    bit5=M.op(0x5d,10),
    bit6=M.op(0x5e,10),
    bit7=M.op(0x5f,10)

    -- hhll
    call=M.op(0x44,16),
    jmp=M.op(0x54,10),
    lxisp=M.op(0x04,10),
    lxibc=M.op(0x14,10),
    lxide=M.op(0x24,10),
    lxihl=M.op(0x34,10)    

    -- (r16),hhll
    mvixbc=M.op(0x49,10),
    mvixde=M.op(0x4a,10),
    mvixhl=M.op(0x4b,10)

    - (r16)
    dcxbc=M.op(0x13,7),
    dcxde=M.op(0x23,7),
    dcxhl=M.op(0x33,7),
    inxbc=M.op(0x12,7),
    inxde=M.op(0x22,7),
    inxhl=M.op(0x32,7),
    ldaxbc=M.op(0x29,7), 
    ldaxde=M.op(0x2a,7),
    ldaxhl=M.op(0x2b,7),
    staxbc=M.op(0x39,7),
    staxde=M.op(0x3a,7),
    staxhl=M.op(0x3b,7),


16 bits instructions:
    0x48xx 
    0x4cxx
    0x4dxx
    0x60xx
    0x64xx
    0x70xx
    0x74xx
]]--
