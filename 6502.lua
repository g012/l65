local M = {}

local byte_normalize = function(v)
    if v < -128 or v > 255 then error("value out of byte range: " .. v) end
    if v < 0 then v = v + 0x100 end
    return v & 0xff
end
local word_normalize = function(v)
    if v < -32768 or v > 65535 then error("value out of word range: " .. v) end
    if v < 0 then v = v + 0x10000 end
    return v & 0xffff
end

local byte_emit = function(v, bin)
    assert(v >=0 and v <= 0xff)
    bin[#bin+1] = v
end
local word_emit = function(v, bin)
    assert(v >=0 and v <= 0xffff)
    bin[#bin+1] = 0xff & (v % 0x100)
    bin[#bin+1] = 0xff & (v // 0x100)
end

M.byte = function(...)
    local data = {...}
    for _,v in ipairs(data) do byte_emit(byte_normalize(v)) end
end
M.word = function(...)
    local data = {...}
    for _,v in ipairs(data) do word_emit(word_normalize(v)) end
end

M.section = function(t)
    local s = {}
    if (type(t) == 'string') then s.name = t
    else
        assert(type(t) == 'table')
        assert(type(t.name) == 'string' and string.len(t.name) > 0)
        s = t
    end
    s.instructions = {}
end

return M

--[===[


adressing = {
    imm=0x09, zp=0x05, zpx=0x15, ab=0x0d, abx=0x1d, aby=0x19, inx=0x01, iny=0x11,
    acc=0x09,
}
encode = {
    adc=0x60,
    ['and']=0x20,
    asl=0x01,
    lda=0xa0,
    sta=0x80,
    --lda = { imm=0xa9, zp=0xa5, zpx=0xb5, ab=0xad, abx=0xbd, aby=0xb9, inx=0xa1, iny=0xb1 },
}

]===]
