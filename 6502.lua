local M = {}

local symbols={} M.symbols=symbols
local locations={} M.locations=locations
local stats={} M.stats=stats

M.__index = M
symbols.__index = symbols
setmetatable(M, symbols)

M.link = function()
    if stats.unused then return end

    stats.unused = 0
    stats.cycles = 0
    for _,location in ipairs(locations) do
        local sections = location.sections

        local chunk_reserve = function(chunk_ix, chunk, start, size)
            if start == chunk.start then
                if size == chunk.size then location.chunks[chunk_ix] = nil
                else chunk.start=start+size chunk.size=chunk.size-size end
            else
                if chunk.size - (start - chunk.start) == size then chunk.size = chunk.size - size
                else
                    chunk.size = start - chunk.start
                    table.insert(location.chunks, chunk_ix+1, { start=start+size, size=chunk.start+chunk.size-(start+size) })
                end
            end
        end

        -- filter sections list
        local position_independent_sections = {}
        local symbols_to_remove = {}
        location.cycles=0
        for ix,section in ipairs(sections) do
            section:compute_size()
            location.cycles = location.cycles + section.cycles
            if section.size == 0 then
                sections[ix]=nil
                if not section.org then table.insert(symbols_to_remove, section.label) end
            elseif not section.org then table.insert(position_independent_sections, section) end
        end
        for _,v in ipairs(symbols_to_remove) do symbols[v] = nil end
        stats.cycles = stats.cycles + location.cycles

        -- fixed position sections
        for section_ix,section in ipairs(sections) do if section.org then
            if section.org < location.start or section.org > location.finish then
                error("ORG section " .. section.label .. " starts outside container location")
            end
            for chunk_ix,chunk in ipairs(location.chunks) do
                if chunk.start <= section.org and chunk.size - (section.org - chunk.start) >= section.size then
                    chunk_reserve(chunk_ix, chunk, section.org, section.size)
                    goto chunk_located
                end
            end
            error("ORG section " .. section.label .. " overflows its location")
            ::chunk_located::
        end end

        -- position independent sections
        table.sort(position_independent_sections, function(a,b) return a.size < b.size end)
        while #position_independent_sections > 0 do
            local section = position_independent_sections[1]
            local chunks = {}
            for _,chunk in ipairs(location.chunks) do
                if chunk.size >= section.size then chunks[#chunks+1] = chunk end
            end
            table.sort(chunks, function(a,b) return a.size < b.size end)
            for chunk_ix,chunk in ipairs(chunks) do
                local waste,position = math.maxinteger
                local usage_lowest = function(start, finish)
                    local inc=1
                    if section.align then
                        start = (start + section.align - 1) // section.align * section.align
                        if section.offset then start = start + section.offset end
                        inc = section.align
                    end
                    for address=start,finish,inc do
                        for _,constraint in ipairs(section.constraints) do
                            local start, finish = address+contraint.start, address+constraint.finish
                            if start // 0x100 == finish // 0x100 then
                                if constraint.type == 'crosspage' then goto constraints_not_met end
                            else
                                if constraint.type == 'samepage' then goto constraints_not_met end
                            end
                        end
                        local w = math.min(address - chunk.start, chunk.size - (address - chunk.start))
                        if w < waste then waste=w position=address end
                        ::constraints_not_met::
                    end
                end
                local finish = math.min(chunk.start + 0xff, chunk.start + chunk.size - section.size)
                usage_lowest(chunk.start, finish)
                if chunk.size ~= math.huge then
                    local start = math.max(chunk.start + chunk.size - section.size - 0xff, chunk.start)
                    usage_lowest(start, chunk.start + chunk.size - section.size)
                end
                if position then
                    chunk_reserve(chunk_ix, chunk, position, section.size)
                    section.org = position
                    symbols[section.label] = position
                    goto chunk_located
                end
            end
            error("unable to find space for section " .. section.label)
            ::chunk_located::
        end

        -- unused space stats
        local unused = 0
        for _,chunk in ipairs(location.chunks) do unused = unused + chunk.size - chunk.start end
        location.unused = unused
        stats.unused = stats.unused + unused

    end
end

M.resolve = function()
    if stats.resolved_count then return end
    M.link()

    stats.resolved_count = 0
    local count = 0
    for k,v in pairs(symbols) do
        local t = type(v)
        if v == 'function' then symbols[k] = v() count=count+1
        elseif v == 'table' and type(v.resolve) == 'function' then symbols[k] = v.resolve() count=count+1 end
    end
    stats.resolved_count = count
end

M.genbin = function(filler)
    if not filler then filler = 0xff end
    M.resolve()
    local bin = {}
    local ins = table.insert
    table.sort(locations, function(a,b) return a.start < b.start end)
    for _,location in ipairs(locations) do
        if location.start < #bin then
            error(string.format("location [%04x,%04x] overlaps another",
                location.start, math.type(location.size) == 'integer' and (location.size + location.start) or 0xffff))
        end
        for i=#bin,location.start do ins(bin, filler) end
        M.size=0 M.cycles=0
        local sections = location.sections
        table.sort(sections, function(a,b) return a.start < b.start end)
        for _,section in sections do
            assert(section.org >= #bin)
            for i=#bin,section.org do ins(bin, filler) end
            for _,instruction in ipairs(section.instructions) do
                if instruction.bin then for _,b in ipairs(instruction.bin) do ins(bin, b) end
                elseif instruction.asbin then instruction.asbin(bin) end
                M.size=#bin M.cycles=M.cycles+(instruction.cycles or 0)
            end
        end
        if math.type(location.size) == 'integer' then
            local endpos = location.size+location.start
            for i=#bin,endpos do ins(bin, filler) end
        end
    end
    return bin
end

M.writebin = function(filename, bin)
    if not filename then filename = 'main.bin' end
    if not bin then bin = M.genbin() end
    local f = assert(io.open(filename, "wb"), "failed to open " .. filename .. " for writing")
    local sconv,s=string.char,{} for i=1,#bin do s[i] = sconv(bin[i]) end
    f:write(table.concat(s))
    f:close()
end

M.writesym = function(filename)
    if not filename then filename = 'main.sym' end
    local f = assert(io.open(filename, "wb"), "failed to open " .. filename .. " for writing")
    table.sort(symbols)
    local ins,fmt,rep = table.insert,string.format,string.rep
    local s ={'--- Symbol List'}
    for k,v in pairs(symbols) do ins(s, fmt("%s%s %04x", k, rep(' ',24-#k), type(v)=='table' and v.org or v)) end
    s[#s+1] = '--- End of Symbol List.'
    f:write(table.concat(s, '\n'))
    f:close()
end

M.location = function(start, finish)
    if type(start) == 'table' then
        for _,v in ipairs(locations) do if v == start then
            M.location_current = start
            return start
        end end
        error("unable to find reference to location [" .. (start.start or '?') .. ", " .. (start.finish or '?') .. "]")
    end
    local size = (finish or math.huge) - start
    local location = { start=start, finish=finish, chunks={ { start=start, size=size } } }
    locations[#locations+1] = location
    M.location_current = location
    return location
end

M.section = function(t)
    local section = {}
    if (type(t) == 'string') then section.label = t
    else
        assert(type(t) == 'table')
        assert(type(t[1]) == 'string' and string.len(t[1]) > 0)
        section=t section.label=t[1] section[1]=nil
        if section.offset and not section.align then error("section " .. section.label .. " has offset, but no align") end
    end
    table.insert(M.location_current.sections, section)
    if symbols[section.label] then error("duplicate symbol: " .. section.label) end
    symbols[section.label] = section
    M.label_current = section.label
    M.section_current = section
    section.type = 'section'
    section.constraints = {}
    section.instructions = {}
    function section:compute_size()
        local instructions = self.instructions
        self.size=0 self.cycles=0
        for _,instruction in ipairs(instructions) do
            instruction.offset = size
            local ins_sz = instruction.size or 0
            if type(ins_sz) == 'function' then
                -- evaluation is needed to get the size (distinguish zpg/abs)
                -- labels and sections are not resolved at this point, so
                -- evaluation will fail if the size is not explicitly stated (.b/.w);
                -- in that case, assume max size
                ins_sz = ins_sz()
            end
            self.size = self.size + ins_sz
            self.cycles = self.cycles + (instruction.cycles or 0)
        end
        for _,constraint in ipairs(self.constraints) do
            constraint.start = instructions[constraint.from].offset
            constraint.finish =  instructions[constraint.to].offset
        end
    end
    return section
end

M.label = function(name)
    local eval,resolve,label,offset
    label = { type='label', size=eval, resolve=resolve }
    if name:sub(1,1) == '_' then -- local label
        name = M.label_current .. name
    else
        M.label_current = name
    end
    if symbols[name] then error("duplicate symbol: " .. name) end
    symbols[name] = label
    eval = function()
        offset = M.section_current.size
        label.size = 0
        return 0
    end
    resolve = function() return M.section_current.org + offset end
    table.insert(M.section_current.instructions, label)
    return label
end

M.samepage = function()
    local section = M.section_current
    table.insert(section.constraints, { type='samepage', from=#section.instructions+1 })
end
M.crosspage = function()
    local section = M.section_current
    table.insert(section.constraints, { type='crosspage', from=#section.instructions+1 })
end
M.endpage = function()
    local section = M.section_current
    local constraint = section.constraints[#section.constraints]
    assert(constraint and not constraint.finish, "closing constraint, but no constraint is open")
    constraint.to = #section.instructions
end

local byte_normalize = function(v)
    if v < -128 or v > 255 then error("value out of byte range: " .. v) end
    if v < 0 then v = v + 0x100 end
    return v & 0xff
end
M.byte_normalize = byte_normalize

local word_normalize = function(v)
    if v < -32768 or v > 65535 then error("value out of word range: " .. v) end
    if v < 0 then v = v + 0x10000 end
    return v & 0xffff
end
M.word_normalize = word_normalize

-- charset([s] [, f])
-- Set a new charset to be used for next string data in byte().
-- Without argument, revert to Lua charset.
-- s: string of all letters of charset
-- f: letter index offset or function to transform the letter index
M.charset = function(s, f)
    local st = type(s)
    if st == 'nil' then M.cs = nil return s end
    if st == 'table' then M.cs = s return s end
    if not f then f = function(v) return v end
    elseif type(f) == 'number' then f = function(v) return v + f end end
    local t={}
    for c in s:gmatch'.' do t[c]=f(#t) end
    M.cs=t
    return t
end

M.byte_impl = function(args, nrm)
    local data,cs = {},M.cs
    for k,v in ipairs(args) do
        local t = type(v)
        if t == 'number' or t == 'function' then data[#data+1] = v
        elseif t == 'table' then table.move(v,1,#v,#data+1,data)
        elseif t == 'string' then
            if cs then
                for _,c in v:gmatch'.' do
                    local i=cs[c]
                    if not i then error("character " .. c .. " is not part of current charset") end
                    data[#data+1]=i
                end
            else
                local s = {v:byte(1,#v)}
                table.move(s, 1, #s, #data+1, data)
            end
        else error("unsupported type for byte() argument: " .. t .. ", value: " .. v)
        end
    end
    local asbin = function(b)
        for _,v in ipairs(data) do
            if type(v) == 'function' then v = v() end
            b[#b+1] = nrm(v)
        end
    end
    table.insert(M.section_current.instructions, { data=data, size=#data, asbin=asbin })
end
-- byte(...)
-- Declare bytes to go into the binary stream.
-- Each argument can be either:
--  * a number resolving to a valid range byte
--  * a string, converted to bytes using the charset previously defined,
--    or Lua's charset if none was defined
--  * a table, with each entry resolving to a valid range byte
--  * a function, resolving to exactly one valid range byte, evaluated
--    after symbols have been resolved
M.byte = function(...)
    return M.byte_impl({...}, byte_normalize)
end
local byte_encapsulate = function(args)
    for k,v in ipairs(args) do
        if type(v) == 'table' and (v.type == 'section' or v.type == 'label') then
            args[k] = function() return symbols[v.label] end
        end
    end
end
M.byte_hi = function(...)
    return M.byte_impl(byte_encapsulate{...}, function(v) return (v>>8)&0xff end)
end
M.byte_lo = function(...)
    return M.byte_impl(byte_encapsulate{...}, function(v) return v&0xff end)
end

-- word(...)
-- Declare words to go into the binary stream.
-- Each argument can be either:
--  * a section or a label
--  * a number resolving to a valid range word
--  * a table, with each entry resolving to a valid range word
--  * a function, resolving to exactly one valid range word, evaluated
--    after symbols have been resolved
M.word = function(...)
    local args = {...}
    local data = {}
    for k,v in ipairs(args) do
        local t = type(v)
        if t == 'number' or t == 'function' then data[#data+1] = v
        elseif t == 'table' then
            if v.type == 'section' or v.type == 'label' then data[#data+1] = function() return symbols[v.label] end
            else table.move(v,1,#v,#data+1,data) end
        else error("unsupported type for word() argument: " .. t .. ", value: " .. v)
        end
    end
    local asbin = function(b)
        for _,v in ipairs(data) do
            if type(v) == 'function' then v = v() end
            v = word_normalize(v)
            b[#b+1] = v&0xff
            b[#b+1] = v>>8
        end
    end
    table.insert(M.section_current.instructions, { data=data, size=#data*2, asbin=asbin })
end

local op,cycles_def = function(code, cycles, extra_on_crosspage)
    return { opc=code, cycles=cycles or cycles_def, xcross=extra_on_crosspage or 0 }
end
cycles_def=2 local opimp={
    asl=op(0x0a), brk=op(0x00,7), clc=op(0x18), cld=op(0xd8), cli=op(0x58), clv=op(0xb8), dex=op(0xca), dey=op(0x88),
    inx=op(0xe8), iny=op(0xc8), lsr=op(0x4a), nop=op(0xea), pha=op(0x48,3), php=op(0x08,3), pla=op(0x68,4), plp=op(0x28,4),
    rol=op(0x2a), ror=op(0x6a), rti=op(0x40,6), rts=op(0x60,6), sec=op(0x38), sei=op(0x78), tax=op(0xaa), tay=op(0xa8),
    tsx=op(0xba), txa=op(0x8a), txs=op(0x9a), tya=op(0x98),
    jam=op(0x02,0),
} M.opimp = opimp
for k,v in pairs(opimp) do
    M[k .. 'imp'] = function()
        local asbin = function(b) b[#b+1] = v.opc end
        table.insert(M.section_current.instructions, { size=1, cycles=v.cycles, asbin=asbin })
    end
end
cycles_def=2 local opimm={
    adc=op(0x69), ['and']=op(0x29), cmp=op(0xc9), cpx=op(0xe0), cpy=op(0xc0), eor=op(0x49), lda=op(0xa9), ldx=op(0xa2),
    ldy=op(0xa0), ora=op(0x09), sbc=op(0xe9),
    anc=op(0x0b), ane=op(0x8b), arr=op(0x6b), asr=op(0x4b), jam=op(0x12,0), lax=op(0xab), nop=op(0x80), sbx=op(0xcb),
} M.opimm = opimm
for k,v in pairs(opimm) do
    M[k .. 'imm'] = function(late, early)
        local asbin = function(b)
            local x = early or 0
            x = byte_normalize(type(late) == 'function' and late(x) or x+late)
            b[#b+1]=v.opc b[#b+1]=x
        end
        table.insert(M.section_current.instructions, { size=2, cycles=2, asbin=asbin })
    end
end
cycles_def=3 local opzpg={
    adc=op(0x65), ['and']=op(0x25), asl=op(0x06,5), bit=op(0x24), cmp=op(0xc5), cpx=op(0xe4), cpy=op(0xc4), dec=op(0xc6,5),
    eor=op(0x45), inc=op(0xe6,5), lda=op(0xa5), ldx=op(0xa6), ldy=op(0xa4), lsr=op(0x46,5), ora=op(0x05), rol=op(0x26,5),
    ror=op(0x66,5), sbc=op(0xe5), sta=op(0x85), stx=op(0x86), sty=op(0x84), 
    dcp=op(0xc7,5), isb=op(0xe7,5), jam=op(0x22,0), lax=op(0xa7), nop=op(0x04), rla=op(0x27,5), rra=op(0x67,5), sax=op(0x87),
    slo=op(0x07,5), sre=op(0x47,5),
} M.opzpg = opzpg
for k,v in pairs(opzpg) do
    M[k .. 'zpg'] = function(late, early)
        local asbin = function(b)
            local x = early or 0
            x = byte_normalize(type(late) == 'function' and late(x) or x+late)
            b[#b+1]=v.opc b[#b+1]=x
        end
        table.insert(M.section_current.instructions, { size=2, cycles=v.cycles, asbin=asbin })
    end
end
cycles_def=4 local opabs={
    adc=op(0x6d), ['and']=op(0x2d), asl=op(0x0e,6), bit=op(0x2c), cmp=op(0xcd), cpx=op(0xec), cpy=op(0xcc), dec=op(0xce,6),
    eor=op(0x4d), inc=op(0xee,6), jmp=op(0x4c,3), jsr=op(0x20,6), lda=op(0xad), ldx=op(0xae), ldy=op(0xac), lsr=op(0x4e,6),
    ora=op(0x0d), rol=op(0x2e,6), ror=op(0x6e,6), sbc=op(0xed), sta=op(0x8d), stx=op(0x8e), sty=op(0x8c),
    dcp=op(0xcf,6), isb=op(0xef,6), jam=op(0x72,0), lax=op(0xaf), nop=op(0x0c), rla=op(0x2f,6), rra=op(0x6f,6), sax=op(0x8f),
    slo=op(0x0f,6), sre=op(0x4f,6),
} M.opabs = opabs
for k,v in pairs(opabs) do
    M[k .. 'abs'] = function(late, early)
        local asbin = function(b)
            local x = early or 0
            x = word_normalize(type(late) == 'function' and late(x) or x+late)
            b[#b+1]=v.opc b[#b+1]=x&0xff b[#b+1]=x>>8
        end
        table.insert(M.section_current.instructions, { size=3, cycles=v.cycles, asbin=asbin })
    end
    M[k .. 'zab'] = function(late, early)
        if type(late) ~= 'function' then
            local x = (early or 0) + late
            if x >= -128 and x <= 0xff then return M[k .. 'zpg'](late, early) end
            if x >= -32768 and x <= 0xffff then return M[k .. 'abs'](late, early) end
            error("value out of word range: " .. x)
        end
        local eval, asbin
        local ins = { size=eval, cycles=v.cycles, asbin=asbin }
        eval = function()
            local r,x = pcall(late(early or 0))
            if not r then return 3 end
            x = word_normalize(x)
            local op = opzpg[k]
            if x <= 0xff and op then
                ins.size = 2
                ins.cycles = op.cycles
                ins.asbin = function(b) b[#b+1]=op.opc b[#b+1]=x end
                return 2
            end
            ins.size = 3
            ins.asbin = function(b) b[#b+1]=v.opc b[#b+1]=x&0xff b[#b+1]=x>>8 end
            return 3
        end
        asbin = function(b)
            local x = word_normalize(late(early or 0))
            -- since we assumed absolute on link phase, we must generate absolute in binary
            if x <= 0xff and opzpg[k] then print("warning: forcing abs on zpg operand for opcode " .. k) end
            b[#b+1]=v.opc b[#b+1]=x&0xff b[#b+1]=x>>8
        end
        table.insert(M.section_current.instructions, ins)
    end
end

return M
