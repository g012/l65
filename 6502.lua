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
        local sections,rorg = location.sections,location.rorg

        local chunk_reserve = function(chunk_ix, chunk, start, size)
            if start == chunk.start then
                if size == chunk.size then location.chunks[chunk_ix] = nil
                else chunk.start=start+size chunk.size=chunk.size-size end
            else
                if chunk.size - (start - chunk.start) == size then chunk.size = chunk.size - size
                else
                    local sz = start - chunk.start
                    table.insert(location.chunks, chunk_ix+1, { start=start+size, size=chunk.size-(sz+size) })
                    chunk.size = sz
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
                    symbols[section.label] = rorg(section.org)
                    goto chunk_located
                end
            end
            error("ORG section " .. section.label .. " overflows its location")
            ::chunk_located::
        end end

        -- position independent sections
        table.sort(position_independent_sections, function(a,b) return a.size==b.size and a.label>b.label or a.size>b.size end)
        for _,section in ipairs(position_independent_sections) do
            local chunks = {}
            for _,chunk in ipairs(location.chunks) do
                if chunk.size >= section.size then chunks[#chunks+1] = chunk end
            end
            table.sort(chunks, function(a,b) return a.size < b.size end)
            for chunk_ix,chunk in ipairs(chunks) do
                local waste,position,position_end = math.maxinteger
                local usage_lowest = function(start, finish)
                    local inc=1
                    if section.align then
                        start = (start + section.align - 1) // section.align * section.align
                        if section.offset then start = start + section.offset end
                        inc = section.align
                    end
                    for address=start,finish,inc do
                        for _,constraint in ipairs(section.constraints) do
                            local cstart, cfinish = address+constraint.start, address+constraint.finish
                            if cstart // 0x100 == cfinish // 0x100 then
                                if constraint.type == 'crosspage' then goto constraints_not_met end
                            else
                                if constraint.type == 'samepage' then goto constraints_not_met end
                            end
                        end
                        local address_end = address+section.size
                        local w = math.min(address - chunk.start, chunk.size - (address_end - chunk.start))
                        if w > waste then goto constraints_not_met end
                        if w==waste then
                            -- if waste is the same, keep the one that uses the least amount of aligned addresses
                            local align=0x100
                            repeat
                                local cross_count_cur = (position_end+align-1)//align - (position+align-1)//align
                                if position&(align-1) == 0 then cross_count_cur=cross_count_cur+1 end
                                local cross_count_new = (address_end+align-1)//align - (address+align-1)//align
                                if address&(align-1) == 0 then cross_count_new=cross_count_new+1 end
                                if cross_count_new < cross_count_cur then goto select_pos end
                                align = align>>1
                            until align==1
                            -- if cross count is same, take the one with the most LSB count
                            local lsb_cur,lsb_new=0,0
                            for i=0,15 do if position&(1<<i) == 0 then lsb_cur=lsb_cur+1 else break end end
                            for i=0,15 do if address&(1<<i) == 0 then lsb_new=lsb_new+1 else break end end
                            if lsb_cur >= lsb_new then goto constraints_not_met end
                        end
                        ::select_pos::
                        waste=w position=address position_end=address_end
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
                    symbols[section.label] = rorg(position)
                    --print(section.label, string.format("%04X\t%d", position, section.size))
                    --for k,v in ipairs(location.chunks) do print(string.format("  %04X  %04X  %d", v.start, v.size+v.start-1, v.size)) end
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
    repeat local count = 0
    for k,v in pairs(symbols) do if k ~= '__index' then
        local t = type(v)
        if t == 'function' then v=v() t=type(v) symbols[k]=v count=count+1 end
        if t == 'table' and type(v.resolve) == 'function' then symbols[k]=v.resolve() count=count+1 end
        if t == 'string' and symbols[v] then symbols[k]=symbols[v] count=count+1 end
        stats.resolved_count = stats.resolved_count + count
    end end until count == 0

    -- set local label references resolver
    local llresolver = { __index = function(tab,key)
        if type(key) ~= 'string' or key:sub(1,1) ~= '_' then return nil end
        return symbols[M.label_current .. key]
    end }
    setmetatable(symbols, llresolver)
end

M.genbin = function(filler)
    if #locations == 0 then return end
    if not filler then filler = 0 end -- brk opcode
    M.resolve()
    local bin = {}
    local ins,mov = table.insert,table.move
    table.sort(locations, function(a,b) return a.start < b.start end)
    local of0 = locations[1].start
    for _,location in ipairs(locations) do
        if location.start < #bin then
            error(string.format("location [%04x,%04x] overlaps another", location.start, location.finish))
        end
        for i=#bin+of0,location.start-1 do ins(bin, filler) end
        M.size=0 M.cycles=0
        local sections = location.sections
        table.sort(sections, function(a,b) return a.org < b.org end)
        for _,section in ipairs(sections) do
            assert(section.org >= #bin+of0)
            for i=#bin+of0,section.org-1 do ins(bin, filler) end
            M.label_current = section.label
            for _,instruction in ipairs(section.instructions) do
                local b,f = instruction.bin,instruction.asbin
                if b then mov(b,1,#b,#bin+1,bin)
                elseif f then f(bin) end
                M.size=#bin M.cycles=M.cycles+(instruction.cycles or 0)
            end
        end
        if location.finish then
            for i=#bin+of0,location.finish do ins(bin, filler) end
        end
    end
    return bin
end

M.writebin = function(filename, bin)
    if not filename then filename = 'main.bin' end
    if not bin then bin = M.genbin() end
    local f = assert(io.open(filename, "wb"), "failed to open " .. filename .. " for writing")
    f:write(string.char(table.unpack(bin)))
    f:close()
end

M.writesym = function(filename)
    if not filename then filename = 'main.sym' end
    local f = assert(io.open(filename, "wb"), "failed to open " .. filename .. " for writing")
    table.sort(symbols)
    local ins,fmt,rep = table.insert,string.format,string.rep
    local s ={'--- Symbol List'}
    local sym_rev = {}
    for k,v in pairs(symbols) do if type(v) == 'number' then ins(sym_rev,k) end end
    table.sort(sym_rev, function(a,b) local x,y=symbols[a],symbols[b] return x==y and a<b or x<y end)
    for _,v in ipairs(sym_rev) do local k=symbols[v] ins(s, fmt("%s%s %04x", v, rep(' ',24-#v), k)) end
    s[#s+1] = '--- End of Symbol List.'
    f:write(table.concat(s, '\n'))
    f:close()
end

M.getstats = function()
    return 'TODO' -- TODO
end

M.location = function(start, finish)
    local location = { type='location', start=start, finish=finish, sections={} }
    if type(start) == 'table' then
        if start.type == 'location' then
            for _,v in ipairs(locations) do if v == start then
                M.location_current = start
                return start
            end end
            error("unable to find reference to location [" .. (start.start or '?') .. ", " .. (start.finish or '?') .. "]")
        end
        location.start = start[1]
        location.finish = start[2]
        location.rorg = start.rorg
        if type(location.rorg) == 'number' then
            local offset = location.rorg - location.start
            location.rorg = function(x) return x+offset end
        end
    end
    if not location.rorg then location.rorg = function(x) return x end end
    local size = (location.finish or math.huge) - location.start + 1
    location.chunks={ { start=location.start, size=size } }
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
            instruction.offset = self.size
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
    local label,offset
    local section,rorg = M.section_current,M.location_current.rorg
    label = { type='label' }
    if name:sub(1,1) == '_' then -- local label
        name = M.label_current .. name
    else
        M.label_current = name
        label.asbin = function() M.label_current = name end
    end
    if symbols[name] then error("duplicate symbol: " .. name) end
    symbols[name] = label
    label.size = function()
        offset = section.size
        label.size = 0
        return 0
    end
    label.resolve = function() return rorg(section.org + offset) end
    table.insert(section.instructions, label)
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
    assert(constraint and not constraint.to, "closing constraint, but no constraint is open")
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
                for c in v:gmatch'.' do
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
            local vt = type(v)
            if vt == 'table' and v.label then v = symbols[v.label]
            elseif vt == 'string' then v = symbols[v] end
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
    return args
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
            local vt = type(v)
            if vt == 'table' and v.label then v = symbols[v.label]
            elseif vt == 'string' then v = symbols[v] end
            v = word_normalize(v)
            b[#b+1] = v&0xff
            b[#b+1] = v>>8
        end
    end
    table.insert(M.section_current.instructions, { data=data, size=#data*2, asbin=asbin })
end

local op,cycles_def,xcross_def
op = function(code, cycles, extra_on_crosspage)
    return { opc=code, cycles=cycles or cycles_def, xcross=extra_on_crosspage or xcross_def }
end
local op_eval = function(late, early)
    local x = early or 0
    return type(late) == 'function' and late(x) or x+late
end
local op_eval_byte = function(late, early) return byte_normalize(op_eval(late, early)) end
local op_eval_word = function(late, early) return word_normalize(op_eval(late, early)) end
cycles_def=2 xcross_def=0 local opimp={
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
cycles_def=2 xcross_def=0 local opimm={
    adc=op(0x69), ['and']=op(0x29), cmp=op(0xc9), cpx=op(0xe0), cpy=op(0xc0), eor=op(0x49), lda=op(0xa9), ldx=op(0xa2),
    ldy=op(0xa0), ora=op(0x09), sbc=op(0xe9),
    anc=op(0x0b), ane=op(0x8b), arr=op(0x6b), asr=op(0x4b), jam=op(0x12,0), lax=op(0xab), nop=op(0x80), sbx=op(0xcb),
} M.opimm = opimm
for k,v in pairs(opimm) do
    M[k .. 'imm'] = function(late, early)
        local asbin = function(b) b[#b+1]=v.opc b[#b+1]=op_eval_byte(late,early) end
        table.insert(M.section_current.instructions, { size=2, cycles=2, asbin=asbin })
    end
end
cycles_def=3 xcross_def=0 local opzpg={
    adc=op(0x65), ['and']=op(0x25), asl=op(0x06,5), bit=op(0x24), cmp=op(0xc5), cpx=op(0xe4), cpy=op(0xc4), dec=op(0xc6,5),
    eor=op(0x45), inc=op(0xe6,5), lda=op(0xa5), ldx=op(0xa6), ldy=op(0xa4), lsr=op(0x46,5), ora=op(0x05), rol=op(0x26,5),
    ror=op(0x66,5), sbc=op(0xe5), sta=op(0x85), stx=op(0x86), sty=op(0x84), 
    dcp=op(0xc7,5), isb=op(0xe7,5), jam=op(0x22,0), lax=op(0xa7), nop=op(0x04), rla=op(0x27,5), rra=op(0x67,5), sax=op(0x87),
    slo=op(0x07,5), sre=op(0x47,5),
} M.opzpg = opzpg
for k,v in pairs(opzpg) do
    M[k .. 'zpg'] = function(late, early)
        local asbin = function(b) b[#b+1]=v.opc b[#b+1]=op_eval_byte(late,early) end
        table.insert(M.section_current.instructions, { size=2, cycles=v.cycles, asbin=asbin })
    end
end
cycles_def=4 xcross_def=0 local opabs={
    adc=op(0x6d), ['and']=op(0x2d), asl=op(0x0e,6), bit=op(0x2c), cmp=op(0xcd), cpx=op(0xec), cpy=op(0xcc), dec=op(0xce,6),
    eor=op(0x4d), inc=op(0xee,6), jmp=op(0x4c,3), jsr=op(0x20,6), lda=op(0xad), ldx=op(0xae), ldy=op(0xac), lsr=op(0x4e,6),
    ora=op(0x0d), rol=op(0x2e,6), ror=op(0x6e,6), sbc=op(0xed), sta=op(0x8d), stx=op(0x8e), sty=op(0x8c),
    dcp=op(0xcf,6), isb=op(0xef,6), jam=op(0x72,0), lax=op(0xaf), nop=op(0x0c), rla=op(0x2f,6), rra=op(0x6f,6), sax=op(0x8f),
    slo=op(0x0f,6), sre=op(0x4f,6),
} M.opabs = opabs
for k,v in pairs(opabs) do
    M[k .. 'abs'] = function(late, early)
        local asbin = function(b)
            local x = op_eval_word(late,early)
            b[#b+1]=v.opc b[#b+1]=x&0xff b[#b+1]=x>>8
        end
        table.insert(M.section_current.instructions, { size=3, cycles=v.cycles, asbin=asbin })
    end
end
local opzab={} M.opabs = opabs
for k,_ in pairs(opzpg) do if opabs[k] then opzab[k]=true end end
for k,_ in pairs(opzab) do
    M[k .. 'zab'] = function(late, early)
        if type(late) ~= 'function' then
            local x = (early or 0) + late
            if x >= -128 and x <= 0xff then return M[k .. 'zpg'](late, early) end
            if x >= -32768 and x <= 0xffff then return M[k .. 'abs'](late, early) end
            error("value out of word range: " .. x)
        end
        local abs = opabs[k]
        local ins = { cycles=abs.cycles }
        ins.size = function()
            local r,x = pcall(late, early or 0)
            if not r then return 3 end
            x = word_normalize(x)
            local zpg = opzpg[k]
            if x <= 0xff and zpg then
                ins.size = 2
                ins.cycles = zpg.cycles
                ins.asbin = function(b) b[#b+1]=zpg.opc b[#b+1]=x end
                return 2
            end
            ins.size = 3
            ins.asbin = function(b) b[#b+1]=abs.opc b[#b+1]=x&0xff b[#b+1]=x>>8 end
            return 3
        end
        ins.asbin = function(b)
            local x = word_normalize(late(early or 0))
            -- since we assumed absolute on link phase, we must generate absolute in binary
            if x <= 0xff and opzpg[k] then print("warning: forcing abs on zpg operand for opcode " .. k) end
            b[#b+1]=abs.opc b[#b+1]=x&0xff b[#b+1]=x>>8
        end
        table.insert(M.section_current.instructions, ins)
    end
end
cycles_def=4 xcross_def=0 local opzpx={
    adc=op(0x75), ['and']=op(0x35), asl=op(0x16,6), cmp=op(0xd5), dec=op(0xd6,6), eor=op(0x55), inc=op(0xf6,6), lda=op(0xb5),
    ldy=op(0xb4), lsr=op(0x56,6), ora=op(0x15), rol=op(0x36,6), ror=op(0x76,6), sbc=op(0xf5), sta=op(0x95), sty=op(0x94),
    dcp=op(0xd7,6), isb=op(0xf7,6), jam=op(0x32,0), nop=op(0x14), rla=op(0x37,6), rra=op(0x77,6), slo=op(0x17,6), sre=op(0x57,6),
} M.opzpx = opzpx
for k,v in pairs(opzpx) do
    M[k .. 'zpx'] = function(late, early)
        local asbin = function(b) b[#b+1]=v.opc b[#b+1]=op_eval_byte(late,early) end
        table.insert(M.section_current.instructions, { size=2, cycles=v.cycles, asbin=asbin })
    end
end
cycles_def=4 xcross_def=1 local opabx={
    adc=op(0x7d), ['and']=op(0x3d), asl=op(0x1e,7,0), cmp=op(0xdd), dec=op(0xde,7,0), eor=op(0x5d), inc=op(0xfe,7,0), lda=op(0xbd),
    ldy=op(0xbc), lsr=op(0x5e,7,0), ora=op(0x1d), rol=op(0x3e,7,0), ror=op(0x7e,7,0), sbc=op(0xfd), sta=op(0x9d,5,0),
    dcp=op(0xdf,7,0), isb=op(0xff,7,0), jam=op(0x92,0,0), nop=op(0x1c), rla=op(0x3f,7,0), rra=op(0x7f,7,0), shy=op(0x9c,5,0), slo=op(0x1f,7,0),
    sre=op(0x5f,7,0),
} M.opabx = opabx
for k,v in pairs(opabx) do
    M[k .. 'abx'] = function(late, early)
        local asbin = function(b)
            local x = op_eval_word(late,early)
            b[#b+1]=v.opc b[#b+1]=x&0xff b[#b+1]=x>>8
        end
        table.insert(M.section_current.instructions, { size=3, cycles=v.cycles, asbin=asbin })
    end
end
local opzax={} M.opabx = opabx
for k,_ in pairs(opzpx) do if opabx[k] then opzax[k]=true end end
for k,_ in pairs(opzax) do
    M[k .. 'zax'] = function(late, early)
        if type(late) ~= 'function' then
            local x = (early or 0) + late
            if x >= -128 and x <= 0xff then return M[k .. 'zpx'](late, early) end
            if x >= -32768 and x <= 0xffff then return M[k .. 'abx'](late, early) end
            error("value out of word range: " .. x)
        end
        local abx = opabx[k]
        local ins = { cycles=abx.cycles }
        ins.size = function()
            local r,x = pcall(late, early or 0)
            if not r then return 3 end
            x = word_normalize(x)
            local zpx = opzpx[k]
            if x <= 0xff and zpx then
                ins.size = 2
                ins.cycles = zpx.cycles
                ins.asbin = function(b) b[#b+1]=zpx.opc b[#b+1]=x end
                return 2
            end
            ins.size = 3
            ins.asbin = function(b) b[#b+1]=abx.opc b[#b+1]=x&0xff b[#b+1]=x>>8 end
            return 3
        end
        ins.asbin = function(b)
            local x = word_normalize(late(early or 0))
            -- since we assumed absolute on link phase, we must generate absolute in binary
            if x <= 0xff and opzpx[k] then print("warning: forcing abx on zpx operand for opcode " .. k) end
            b[#b+1]=abx.opc b[#b+1]=x&0xff b[#b+1]=x>>8
        end
        table.insert(M.section_current.instructions, ins)
    end
end
cycles_def=4 xcross_def=0 local opzpy={
    ldx=op(0xb6), stx=op(0x96),
    jam=op(0x42,0), lax=op(0xb7), sax=op(0x97),
} M.opzpy = opzpy
for k,v in pairs(opzpy) do
    M[k .. 'zpy'] = function(late, early)
        local asbin = function(b) b[#b+1]=v.opc b[#b+1]=op_eval_byte(late,early) end
        table.insert(M.section_current.instructions, { size=2, cycles=v.cycles, asbin=asbin })
    end
end
cycles_def=4 xcross_def=1 local opaby={
    adc=op(0x79), ['and']=op(0x39), cmp=op(0xd9), eor=op(0x59), lda=op(0xb9), ldx=op(0xbe), ora=op(0x19), sbc=op(0xf9),
    sta=op(0x99,5,0), 
    dcp=op(0xdb,7,0), isb=op(0xfb,7,0), jam=op(0xb2,0,0), las=op(0xbb), lax=op(0xbf), rla=op(0x3b,7,0), rra=op(0x7b,7,0), sha=op(0x9f,5,0),
    shs=op(0x9b,5,0), shx=op(0x9e,5,0), slo=op(0x1b,7,0), sre=op(0x5b,7,0),
} M.opaby = opaby
for k,v in pairs(opaby) do
    M[k .. 'aby'] = function(late, early)
        local asbin = function(b)
            local x = op_eval_word(late,early)
            b[#b+1]=v.opc b[#b+1]=x&0xff b[#b+1]=x>>8
        end
        table.insert(M.section_current.instructions, { size=3, cycles=v.cycles, asbin=asbin })
    end
end
local opzay={} M.opaby = opaby
for k,_ in pairs(opzpy) do if opaby[k] then opzay[k]=true end end
for k,_ in pairs(opzay) do
    M[k .. 'zay'] = function(late, early)
        if type(late) ~= 'function' then
            local x = (early or 0) + late
            if x >= -128 and x <= 0xff then return M[k .. 'zpy'](late, early) end
            if x >= -32768 and x <= 0xffff then return M[k .. 'aby'](late, early) end
            error("value out of word range: " .. x)
        end
        local aby = opaby[k]
        local ins = { cycles=aby.cycles }
        ins.size = function()
            local r,x = pcall(late, early or 0)
            if not r then return 3 end
            x = word_normalize(x)
            local zpy = opzpy[k]
            if x <= 0xff and zpy then
                ins.size = 2
                ins.cycles = zpy.cycles
                ins.asbin = function(b) b[#b+1]=zpy.opc b[#b+1]=x end
                return 2
            end
            ins.size = 3
            ins.asbin = function(b) b[#b+1]=aby.opc b[#b+1]=x&0xff b[#b+1]=x>>8 end
            return 3
        end
        ins.asbin = function(b)
            local x = word_normalize(late(early or 0))
            -- since we assumed absolute on link phase, we must generate absolute in binary
            if x <= 0xff and opzpy[k] then print("warning: forcing aby on zpy operand for opcode " .. k) end
            b[#b+1]=aby.opc b[#b+1]=x&0xff b[#b+1]=x>>8
        end
        table.insert(M.section_current.instructions, ins)
    end
end
cycles_def=2 xcross_def=0 local oprel={
    bcc=op(0x90), bcs=op(0xb0), beq=op(0xf0), bmi=op(0x30), bne=op(0xd0), bpl=op(0x10), bvc=op(0x50), bvs=op(0x70),
} M.oprel = oprel
for k,v in pairs(oprel) do
    M[k .. 'rel'] = function(label)
        local parent,offset = M.label_current
        local section,rorg = M.section_current,M.location_current.rorg
        local op = { cycles=2 }
        op.size = function()
            offset = section.size
            op.size=2
            return 2
        end
        op.asbin = function(b)
            local x,l = label,label
            if type(x) == 'function' then x=x() end
            if type(x) == 'string' then
                if x:sub(1,1) == '_' then x=parent..x l=x end
                x = symbols[x]
            end
            if type(x) ~= 'number' then error("unresolved branch target: " .. tostring(x)) end
            x = x - offset - rorg(section.org)
            if x < -128 or x > 127 then error("branch target out of range for " .. l .. ": " .. x) end
            b[#b+1]=v.opc b[#b+1]=x&0xff
        end
        table.insert(M.section_current.instructions, op)
    end
end
cycles_def=5 xcross_def=0 local opind={
    jmp=op(0x6c),
    jam=op(0xd2,0),
} M.opind = opind
for k,v in pairs(opind) do
    M[k .. 'ind'] = function(late, early)
        local asbin = function(b)
            local x = op_eval_word(late,early)
            b[#b+1]=v.opc b[#b+1]=x&0xff b[#b+1]=x>>8
        end
        table.insert(M.section_current.instructions, { size=3, cycles=v.cycles, asbin=asbin })
    end
end
cycles_def=6 xcross_def=0 local opinx={
    adc=op(0x61), ['and']=op(0x21), cmp=op(0xc1), eor=op(0x41), lda=op(0xa1), ora=op(0x01), sbc=op(0xe1), sta=op(0x81),
    dcp=op(0xc3,8), isb=op(0xe3,8), jam=op(0x52,0), lax=op(0xa3), rla=op(0x23,8), rra=op(0x63,8), sax=op(0x83), slo=op(0x03,8),
    sre=op(0x43,8),
} M.opinx = opinx
for k,v in pairs(opinx) do
    M[k .. 'inx'] = function(late, early)
        local asbin = function(b) b[#b+1]=v.opc b[#b+1]=op_eval_byte(late,early) end
        table.insert(M.section_current.instructions, { size=2, cycles=v.cycles, asbin=asbin })
    end
end
cycles_def=5 xcross_def=1 local opiny={
    adc=op(0x71), ['and']=op(0x31), cmp=op(0xd1), eor=op(0x51), lda=op(0xb1), ora=op(0x11), sbc=op(0xf1), sta=op(0x91,6),
    dcp=op(0xd3,8), isb=op(0xf3,8), jam=op(0x62,0,0), lax=op(0xb3), rla=op(0x33,8), rra=op(0x73,8), sha=op(0x93,6), slo=op(0x13,8),
    sre=op(0x53,8),
}
for k,v in pairs(opiny) do
    M[k .. 'iny'] = function(late, early)
        local asbin = function(b) b[#b+1]=v.opc b[#b+1]=op_eval_byte(late,early) end
        table.insert(M.section_current.instructions, { size=2, cycles=v.cycles, asbin=asbin })
    end
end

return M
