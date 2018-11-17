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

local opr16={
    dcxbc=M.op(0x13,7),
    dcxde=M.op(0x23,7),
    dcxhl=M.op(0x33,7),
    inxbc=M.op(0x12,7),
    inxde=M.op(0x22,7),
    inxhl=M.op(0x32,7),
    ldaxbc=M.op(0x29,7), 
    ldaxdde=M.op(0x2e,7),
    ldaxde=M.op(0x2a,7),
    ldaxhl=M.op(0x2b,7),
    ldaxide=M.op(0x2c,7), 
    ldaxdhl=M.op(0x2f,7),
    ldaxihl=M.op(0x2d,7),
    staxbc=M.op(0x39,7),
    staxde=M.op(0x3a,7),
    staxdde=M.op(0x3e,7),
    staxdhl=M.op(0x3f,7),
    staxhl=M.op(0x3b,7),
    staxide=M.op(0x3c,7),
    staxihl=M.op(0x3d,7),
} M.opr16 = opr16
for k,v in pairs(opr16) do
    M[k] = function()
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
    mviv=M.op(0x68,7),
    mvixbc=M.op(0x49,10),
    mvixde=M.op(0x4a,10),
    mvixhl=M.op(0x4b,10),
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
} M.opw = opw
for k,v in pairs(opw) do
    M[k .. 'imm'] = function(late, early)
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local size = function() late,early = M.size_op(late,early) return 3 end
        local bin = function() local l65dbg=l65dbg 
            local x = M.op_eval_word(late,early)
            return { v.opc, x&0xff, x>>8 }
        end
        table.insert(M.section_current.instructions, { size=size, cycles=v.cycles, bin=bin })
    end
end

local opr16w = {
    lxisp=M.op(0x04,10),
    lxibc=M.op(0x14,10),
    lxide=M.op(0x24,10),
    lxihl=M.op(0x34,10)    
} M.opr16w = opr16w
for k,v in pairs(opr16w) do
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


M['calt' .. 'imm'] = function(late, early)
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

M['calf' .. 'imm'] = function(late, early)
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

M.jre = function(label)
    local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
    local parent,offset = M.label_current
    local section,rorg = M.section_current,M.location_current.rorg
    local op = { cycles=17 }
    op.size = function()
        offset = section.size
        label = M.size_dc(label)
        return 2
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
        x = x-2 - offset - rorg(section.org)
        if x < -128 or x > 127 then error("branch target out of range for " .. l .. ": " .. x) end
        local opcode = x >= 0 and 0x4e or 0x4f 
        return { opcode, x&0xff }
    end
    table.insert(M.section_current.instructions, op)
end

local opwa = {
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
} M.opwa = opwa
for k,v in pairs(opwa) do
    M[k .. 'wa'] = function(late, early)
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local size = function() late,early = M.size_op(late,early) return 2 end
        local bin = function() local l65dbg=l65dbg return { v.opc, M.op_eval_byte(late,early) } end
        table.insert(M.section_current.instructions, { size=size, cycles=v.cycles, bin=bin })
    end
end

local opwaxx = {
    mviw=M.op(0x71,3,13),
    eqiw=M.op(0x75,3,13),
} M.opwaxx = opwaxx
for k,v in pairs(opwaxx) do
    M[k .. 'waxx'] = function(late_offset, late_data, early_offset, early_data)
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local size = function() 
            late_offset,early_offset = M.size_op(late_offset,early_offset) 
            late_data,early_data = M.size_op(late_data,early_data) 
            return 3 
        end
        local bin = function() 
            local l65dbg=l65dbg 
            return { v.opc, M.op_eval_byte(late_offset,early_offset), M.op_eval_byte(late_data,early_data) }
        end
        table.insert(M.section_current.instructions, { size=size, cycles=v.cycles, bin=bin })
    end
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
    skc=M.op(0x0a,8),
    skz=M.op(0x0c,8),
    sknc=M.op(0x1a,8),
    sknz=M.op(0x1c,8),    
    stc=M.op(0x2b,8),
} M.op48imp = op48imp
for k,v in pairs(op48imp) do
    M[k .. 'imp'] = function()
        table.insert(M.section_current.instructions, { size=2, cycles=v.cycles, bin={ 0x48, v.opc } })
    end
end

local op48r8={    
    rlla=M.op(0x30,8),
    rlra=M.op(0x31,8),
    rllc=M.op(0x32,8),
    rlrc=M.op(0x33,8),
    slla=M.op(0x34,8),
    slra=M.op(0x35,8),
    sllc=M.op(0x36,8),
    slrc=M.op(0x37,8),
} M.op48r8 = op48r8
for k,v in pairs(op48r8) do
    M[k] = function()
        table.insert(M.section_current.instructions, { size=2, cycles=v.cycles, bin={ 0x48, v.opc } })
    end
end

local op48r16={
    pushbc=M.op(0x1e,17),
    pushde=M.op(0x2e,17),
    pushhl=M.op(0x3e,17),
    pushva=M.op(0x0e,17),
    popbc=M.op(0x1f,15),
    popde=M.op(0x2f,15),
    pophl=M.op(0x3f,15),
    popva=M.op(0x0f,15),
} M.op48r16 = op48r16
for k,v in pairs(op48r16) do
    M[k] = function()
        table.insert(M.section_current.instructions, { size=2, cycles=v.cycles, bin={ 0x48, v.opc } })
    end
end

local op48int={
    skitf0=M.op(0x00,8),
    skitft=M.op(0x01,8),
    skitf1=M.op(0x02,8),
    skitf2=M.op(0x03,8),
    skitfs=M.op(0x04,8),
    sknitf0=M.op(0x10,8),
    sknitft=M.op(0x11,8),
    sknitf1=M.op(0x12,8),
    sknitf2=M.op(0x13,8),
    sknitfs=M.op(0x14,8),
} M.op48int = op48int
for k,v in pairs(op48int) do
    M[k] = function()
        table.insert(M.section_current.instructions, { size=2, cycles=v.cycles, bin={ 0x48, v.opc } })
    end
end

-- IN/OUT
local opinout={
    ['in']=M.op(0x4c,10),
    out=M.op(0x4d,10),
} M.opinout = op4inout
for k,v in pairs(opinout) do
    M[k .. 'imm'] = function(late, early)
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local op = { cycles=v.cycles }
        op.size = function() late,early = M.size_op(late,early) return 2 end
        op.bin = function() local l65dbg=l65dbg 
            local x = 0x00 + M.op_eval_byte(late,early)
            return { v.opc, 0x00, x }
        end
        table.insert(M.section_current.instructions, op)
    end
end


local op4car8={
    movapa=M.op(0xc0,10),
    movapb=M.op(0xc1,10),
    movapc=M.op(0xc2,10),
    movamk=M.op(0xc3,10),
    movamb=M.op(0xc4,10),
    movamc=M.op(0xc5,10),
    movatm0=M.op(0xc6,10),
    movatm1=M.op(0xc7,10),
    movas=M.op(0xc8,10),
} M.op4car8 = op4car8
for k,v in pairs(op4car8) do
    M[k] = function()
        table.insert(M.section_current.instructions, { size=2, cycles=v.cycles, bin={ 0x4c, v.opc } })
    end
end

local op4dr8a={
    movpaa=M.op(0xc0,10),
    movpba=M.op(0xc1,10),
    movpca=M.op(0xc2,10),
    movmka=M.op(0xc3,10),
    movmba=M.op(0xc4,10),
    movmca=M.op(0xc5,10),
    movtm0a=M.op(0xc6,10),
    movtm1a=M.op(0xc7,10),
    movsa=M.op(0xc8,10),
} M.op4dr8a = op4dr8a
for k,v in pairs(op4dr8a) do
    M[k] = function()
        table.insert(M.section_current.instructions, { size=2, cycles=v.cycles, bin={ 0x4d, v.opc } })
    end
end

local op60names = {'ana','xra','ora','addnc','gta','subnb','lta','add','','adc','','sub','nea','sbb','eqa'}
local register_names = {'v','a','b','c','d','e','h','l'}
local k = 0x08
for i,o in ipairs(op60names) do
    if o == '' then
        k = k + #register_names
    else
        for j,r in ipairs(register_names) do
            local l = k
            M[o .. r .. 'a'] = function()
                table.insert(M.section_current.instructions, { size=2, cycles=8, bin={ 0x60, l } })
            end
            k = k + 1
        end
    end
end

k = 0x88
op60names[9] = 'ona'
op60names[11] = 'offa'
for i,o in ipairs(op60names) do
    if o == '' then
        k = k + #register_names
    else
        for j,r in ipairs(register_names) do
            local l = k
            local name = o .. 'a' .. r
            if not M[name] then
                M[name] = function()
                    table.insert(M.section_current.instructions, { size=2, cycles=8, bin={ 0x60, l } })
                end
            end
            k = k + 1
        end
    end
end

k = 0x08
local op64names = { 'ani','xri','ori','adinc','gti','suinb','lti','adi','oni','aci','offi','sui','nei','sbi','eqi' }
for i,o in ipairs(op64names) do
    for j,r in ipairs(register_names) do
        local name = o .. r
        if not M[name] then
            local l = k
            M[name] = function(late,early)
                local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
                local op = { cycles=11 }
                op.size = function() late,early = M.size_op(late,early) return 3 end
                op.bin = function() local l65dbg=l65dbg 
                    local x = 0x00 + l;
                    local y = 0x00 + M.op_eval_byte(late,early)
                    return { 0x64, x, y }
                end
                table.insert(M.section_current.instructions, op)
            end
        end
        k = k + 1
    end
end

k = 0x88
local ex_register_names = {'pa','pb','pc','mk'}
for i,o in ipairs(op64names) do
    for j,r in ipairs(ex_register_names) do
        local name = o .. r
        if not M[name] then
            local l = k
            M[name] = function(late,early)
                local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
                local op = { cycles=11 }
                op.size = function() late,early = M.size_op(late,early) return 3 end
                op.bin = function() local l65dbg=l65dbg 
                    local x = 0x00 + l;
                    local y = 0x00 + M.op_eval_byte(late,early)
                    return { 0x64, x, y }
                end
                table.insert(M.section_current.instructions, op)
            end
        end
        k = k + 1
    end
    k = k + 4
end

local op74wa = {
    anaw=M.op(0x88,14),
    xraw=M.op(0x90,14),
    oraw=M.op(0x98,14),
    addncw=M.op(0xa0,14),
    gtaw=M.op(0xa8,14),
    subnbw=M.op(0xb0,14),
    ltaw=M.op(0xb8,14),
    addw=M.op(0xc0,14),
    onaw=M.op(0xc8,14),
    adcw=M.op(0xd0,14),
    offaw=M.op(0xd8,14),
    subw=M.op(0xe0,14),
    neaw=M.op(0xe8,14),
    sbbw=M.op(0xf0,14),
    eqaw=M.op(0xf8,14),
} M.op74wa = op74wa
for k,v in pairs(op74wa) do
    M[k .. 'wa'] = function(late, early)
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local size = function() late,early = M.size_op(late,early) return 3 end
        local bin = function() local l65dbg=l65dbg return { 0x74, v.opc, M.op_eval_byte(late,early) } end
        table.insert(M.section_current.instructions, { size=size, cycles=v.cycles, bin=bin })
    end
end

local op70ind = {
    sspd=M.op(0x0e,20),
    lspd=M.op(0x0f,20),
    sbcd=M.op(0x1e,20),
    lbcd=M.op(0x1f,20),
    sded=M.op(0x2e,20),
    lded=M.op(0x2f,20),
    shld=M.op(0x3e,20),
    lhld=M.op(0x3f,20),
} M.op70ind = op70ind
for k,v in pairs(op70ind) do
    M[k] = function(late, early)
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local size = function() late,early = M.size_op(late,early) return 4 end
        local bin = function() local l65dbg=l65dbg 
            local x = M.op_eval_word(late,early)
            return { 0x70, v.opc, x&0xff, x>>8 }
        end
        table.insert(M.section_current.instructions, { size=size, cycles=v.cycles, bin=bin })
    end
end

local op70indr8 = {
    movindv=M.op(0x78,17),
    movinda=M.op(0x79,17),
    movindb=M.op(0x7a,17),
    movindc=M.op(0x7b,17),
    movindd=M.op(0x7c,17),
    movinde=M.op(0x7d,17),
    movindh=M.op(0x7e,17),
    movindl=M.op(0x7f,17),
} M.op70indr8 = op70indr8
for k,v in pairs(op70indr8) do
    M[k] = function(late, early)
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local size = function() late,early = M.size_op(late,early) return 4 end
        local bin = function() local l65dbg=l65dbg 
            local x = M.op_eval_word(late,early)
            return { 0x70, v.opc, x&0xff, x>>8 }
        end
        table.insert(M.section_current.instructions, { size=size, cycles=v.cycles, bin=bin })
    end
end

local op70r8ind = {
    movvind=M.op(0x68,17),
    movaind=M.op(0x69,17),
    movbind=M.op(0x6a,17),
    movcind=M.op(0x6b,17),
    movdind=M.op(0x6c,17),
    moveind=M.op(0x6d,17),
    movhind=M.op(0x6e,17),
    movlind=M.op(0x6f,17),
} M.op70r8ind = op70r8ind
for k,v in pairs(op70r8ind) do
    M[k] = function(late, early)
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local size = function() late,early = M.size_op(late,early) return 4 end
        local bin = function() local l65dbg=l65dbg 
            local x = M.op_eval_word(late,early)
            return { 0x70, v.opc, x&0xff, x>>8 }
        end
        table.insert(M.section_current.instructions, { size=size, cycles=v.cycles, bin=bin })
    end
end

return M

--[[ [todo]
    	
16 bits instructions:
    0x70xx
        11
		case 0x89: my_stprintf_s(buffer, buffer_len, _T("anax b")); break;
		case 0x8a: my_stprintf_s(buffer, buffer_len, _T("anax d")); break;
		case 0x8b: my_stprintf_s(buffer, buffer_len, _T("anax h")); break;
		case 0x8c: my_stprintf_s(buffer, buffer_len, _T("anax d+")); break;
		case 0x8d: my_stprintf_s(buffer, buffer_len, _T("anax h+")); break;
		case 0x8e: my_stprintf_s(buffer, buffer_len, _T("anax d-")); break;
		case 0x8f: my_stprintf_s(buffer, buffer_len, _T("anax h-")); break;
		case 0x91: my_stprintf_s(buffer, buffer_len, _T("xrax b")); break;
		case 0x92: my_stprintf_s(buffer, buffer_len, _T("xrax d")); break;
		case 0x93: my_stprintf_s(buffer, buffer_len, _T("xrax h")); break;
		case 0x94: my_stprintf_s(buffer, buffer_len, _T("xrax d+")); break;
		case 0x95: my_stprintf_s(buffer, buffer_len, _T("xrax h+")); break;
		case 0x96: my_stprintf_s(buffer, buffer_len, _T("xrax d-")); break;
		case 0x97: my_stprintf_s(buffer, buffer_len, _T("xrax h-")); break;
		case 0x99: my_stprintf_s(buffer, buffer_len, _T("orax b")); break;
		case 0x9a: my_stprintf_s(buffer, buffer_len, _T("orax d")); break;
		case 0x9b: my_stprintf_s(buffer, buffer_len, _T("orax h")); break;
		case 0x9c: my_stprintf_s(buffer, buffer_len, _T("orax d+")); break;
		case 0x9d: my_stprintf_s(buffer, buffer_len, _T("orax h+")); break;
		case 0x9e: my_stprintf_s(buffer, buffer_len, _T("orax d-")); break;
		case 0x9f: my_stprintf_s(buffer, buffer_len, _T("orax h-")); break;
		case 0xa1: my_stprintf_s(buffer, buffer_len, _T("addncx b")); break;
		case 0xa2: my_stprintf_s(buffer, buffer_len, _T("addncx d")); break;
		case 0xa3: my_stprintf_s(buffer, buffer_len, _T("addncx h")); break;
		case 0xa4: my_stprintf_s(buffer, buffer_len, _T("addncx d+")); break;
		case 0xa5: my_stprintf_s(buffer, buffer_len, _T("addncx h+")); break;
		case 0xa6: my_stprintf_s(buffer, buffer_len, _T("addncx d-")); break;
		case 0xa7: my_stprintf_s(buffer, buffer_len, _T("addncx h-")); break;
		case 0xa9: my_stprintf_s(buffer, buffer_len, _T("gtax b")); break;
		case 0xaa: my_stprintf_s(buffer, buffer_len, _T("gtax d")); break;
		case 0xab: my_stprintf_s(buffer, buffer_len, _T("gtax h")); break;
		case 0xac: my_stprintf_s(buffer, buffer_len, _T("gtax d+")); break;
		case 0xad: my_stprintf_s(buffer, buffer_len, _T("gtax h+")); break;
		case 0xae: my_stprintf_s(buffer, buffer_len, _T("gtax d-")); break;
		case 0xaf: my_stprintf_s(buffer, buffer_len, _T("gtax h-")); break;
		case 0xb1: my_stprintf_s(buffer, buffer_len, _T("subnbx b")); break;
		case 0xb2: my_stprintf_s(buffer, buffer_len, _T("subnbx d")); break;
		case 0xb3: my_stprintf_s(buffer, buffer_len, _T("subnbx h")); break;
		case 0xb4: my_stprintf_s(buffer, buffer_len, _T("subnbx d+")); break;
		case 0xb5: my_stprintf_s(buffer, buffer_len, _T("subnbx h+")); break;
		case 0xb6: my_stprintf_s(buffer, buffer_len, _T("subnbx d-")); break;
		case 0xb7: my_stprintf_s(buffer, buffer_len, _T("subnbx h-")); break;
		case 0xb9: my_stprintf_s(buffer, buffer_len, _T("ltax b")); break;
		case 0xba: my_stprintf_s(buffer, buffer_len, _T("ltax d")); break;
		case 0xbb: my_stprintf_s(buffer, buffer_len, _T("ltax h")); break;
		case 0xbc: my_stprintf_s(buffer, buffer_len, _T("ltax d+")); break;
		case 0xbd: my_stprintf_s(buffer, buffer_len, _T("ltax h+")); break;
		case 0xbe: my_stprintf_s(buffer, buffer_len, _T("ltax d-")); break;
		case 0xbf: my_stprintf_s(buffer, buffer_len, _T("ltax h-")); break;
		case 0xc1: my_stprintf_s(buffer, buffer_len, _T("addx b")); break;
		case 0xc2: my_stprintf_s(buffer, buffer_len, _T("addx d")); break;
		case 0xc3: my_stprintf_s(buffer, buffer_len, _T("addx h")); break;
		case 0xc4: my_stprintf_s(buffer, buffer_len, _T("addx d+")); break;
		case 0xc5: my_stprintf_s(buffer, buffer_len, _T("addx h+")); break;
		case 0xc6: my_stprintf_s(buffer, buffer_len, _T("addx d-")); break;
		case 0xc7: my_stprintf_s(buffer, buffer_len, _T("addx h-")); break;
		case 0xc9: my_stprintf_s(buffer, buffer_len, _T("onax b")); break;
		case 0xca: my_stprintf_s(buffer, buffer_len, _T("onax d")); break;
		case 0xcb: my_stprintf_s(buffer, buffer_len, _T("onax h")); break;
		case 0xcc: my_stprintf_s(buffer, buffer_len, _T("onax d+")); break;
		case 0xcd: my_stprintf_s(buffer, buffer_len, _T("onax h+")); break;
		case 0xce: my_stprintf_s(buffer, buffer_len, _T("onax d-")); break;
		case 0xcf: my_stprintf_s(buffer, buffer_len, _T("onax h-")); break;
		case 0xd1: my_stprintf_s(buffer, buffer_len, _T("adcx b")); break;
		case 0xd2: my_stprintf_s(buffer, buffer_len, _T("adcx d")); break;
		case 0xd3: my_stprintf_s(buffer, buffer_len, _T("adcx h")); break;
		case 0xd4: my_stprintf_s(buffer, buffer_len, _T("adcx d+")); break;
		case 0xd5: my_stprintf_s(buffer, buffer_len, _T("adcx h+")); break;
		case 0xd6: my_stprintf_s(buffer, buffer_len, _T("adcx d-")); break;
		case 0xd7: my_stprintf_s(buffer, buffer_len, _T("adcx h-")); break;
		case 0xd9: my_stprintf_s(buffer, buffer_len, _T("offax b")); break;
		case 0xda: my_stprintf_s(buffer, buffer_len, _T("offax d")); break;
		case 0xdb: my_stprintf_s(buffer, buffer_len, _T("offax h")); break;
		case 0xdc: my_stprintf_s(buffer, buffer_len, _T("offax d+")); break;
		case 0xdd: my_stprintf_s(buffer, buffer_len, _T("offax h+")); break;
		case 0xde: my_stprintf_s(buffer, buffer_len, _T("offax d-")); break;
		case 0xdf: my_stprintf_s(buffer, buffer_len, _T("offax h-")); break;
		case 0xe1: my_stprintf_s(buffer, buffer_len, _T("subx b")); break;
		case 0xe2: my_stprintf_s(buffer, buffer_len, _T("subx d")); break;
		case 0xe3: my_stprintf_s(buffer, buffer_len, _T("subx h")); break;
		case 0xe4: my_stprintf_s(buffer, buffer_len, _T("subx d+")); break;
		case 0xe5: my_stprintf_s(buffer, buffer_len, _T("subx h+")); break;
		case 0xe6: my_stprintf_s(buffer, buffer_len, _T("subx d-")); break;
		case 0xe7: my_stprintf_s(buffer, buffer_len, _T("subx h-")); break;
		case 0xe9: my_stprintf_s(buffer, buffer_len, _T("neax b")); break;
		case 0xea: my_stprintf_s(buffer, buffer_len, _T("neax d")); break;
		case 0xeb: my_stprintf_s(buffer, buffer_len, _T("neax h")); break;
		case 0xec: my_stprintf_s(buffer, buffer_len, _T("neax d+")); break;
		case 0xed: my_stprintf_s(buffer, buffer_len, _T("neax h+")); break;
		case 0xee: my_stprintf_s(buffer, buffer_len, _T("neax d-")); break;
		case 0xef: my_stprintf_s(buffer, buffer_len, _T("neax h-")); break;
		case 0xf1: my_stprintf_s(buffer, buffer_len, _T("sbbx b")); break;
		case 0xf2: my_stprintf_s(buffer, buffer_len, _T("sbbx d")); break;
		case 0xf3: my_stprintf_s(buffer, buffer_len, _T("sbbx h")); break;
		case 0xf4: my_stprintf_s(buffer, buffer_len, _T("sbbx d+")); break;
		case 0xf5: my_stprintf_s(buffer, buffer_len, _T("sbbx h+")); break;
		case 0xf6: my_stprintf_s(buffer, buffer_len, _T("sbbx d-")); break;
		case 0xf7: my_stprintf_s(buffer, buffer_len, _T("sbbx h-")); break;
		case 0xf9: my_stprintf_s(buffer, buffer_len, _T("eqax b")); break;
		case 0xfa: my_stprintf_s(buffer, buffer_len, _T("eqax d")); break;
		case 0xfb: my_stprintf_s(buffer, buffer_len, _T("eqax h")); break;
		case 0xfc: my_stprintf_s(buffer, buffer_len, _T("eqax d+")); break;
		case 0xfd: my_stprintf_s(buffer, buffer_len, _T("eqax h+")); break;
		case 0xfe: my_stprintf_s(buffer, buffer_len, _T("eqax d-")); break;
		case 0xff: my_stprintf_s(buffer, buffer_len, _T("eqax h-")); break;
]]--
