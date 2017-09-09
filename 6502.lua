local M = {}

local symbols={} M.symbols=symbols
local locations={} M.locations=locations
local stats={} M.stats=stats

local section_current -- cache of last location's last section, for faster access

M.link = function()
    assert(not stats.unused, "can't link twice")

    stats.unused = 0
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
        for ix,section in ipairs(sections) do
            if symbols[section.label] then error("duplicate symbol: " .. section.label) end
            symbols[section.label] = section
            section:compute_size()
            if section.size == 0 then
                sections[ix]=nil
                if not section.org then table.insert(symbols_to_remove, section.label) end
            elseif not section.org then table.insert(position_independent_sections, section) end
        end
        for _,v in ipairs(symbols_to_remove) do symbols[v] = nil end

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

M.genbin = function(filler)
    if not filler then filler = 0xff end
    if not stats.unused then M.link() end
    local bin = {}
    local ins = table.insert
    table.sort(locations, function(a,b) return a.start < b.start end)
    local position = 0
    for _,location in ipairs(locations) do
        if location.start < position then
            error(string.format("location [%04x,%04x] overlaps another",
                location.start, math.type(location.size) == 'integer' and (location.size + location.start) or 0xffff))
        end
        for i=position,location.start do ins(bin, filler) end
        position = location.start
        local sections = location.sections
        table.sort(sections, function(a,b) return a.start < b.start end)
        for _,section in sections do
            assert(section.org >= position)
            for i=position,section.org do ins(bin, filler) end
            for _,instruction in ipairs(section.instructions) do
                -- TODO
            end
        end
        if math.type(location.size) == 'integer' then
            local endpos = location.size+location.start
            for i=position,endpos do ins(bin, filler) end
            position = endpos
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
    local size = (finish or math.huge) - start
    locations[#locations+1] = { start=start, finish=finish, chunks={ { start=start, size=size } } }
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
    table.insert(location[#locations].sections, section)
    section_current = section
    section.constraints = {}
    section.instructions = {}
    function section:compute_size()
        local instructinos = self.instructions
        local size = 0
        for _,instruction in ipairs(instructions) do
            instruction.offset = size
            size = size + #instruction.data
        end
        self.size = size
        -- set start and finish fields of constraints
        for _,constraint in ipairs(self.constraints) do
            constraint.start = instructions[constraint.from].offset
            constraint.finish =  instructions[constraint.to].offset
        end
    end
end

M.samepage = function()
    local section = section_current
    table.insert(section.constraints, { type='samepage', from=#section.instructions+1 })
end
M.crosspage = function()
    local section = section_current
    table.insert(section.constraints, { type='crosspage', from=#section.instructions+1 })
end
M.endpage = function()
    local section = section_current
    local constraint = section.constraints[#section.constraints]
    assert(constraint and not constraint.finish, "closing constraint, but no constraint is open")
    constraint.to = #section.instructions
end

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

M.byte = function(...)
    local data = {...}
    for i=1,#data do data[i] = byte_normalize(data[i]) end
    table.insert(section_current.instructions, { data=data })
end
M.word = function(...)
    local src,data = {...}, {}
    for i=1,#src do
        local v = word_normalize(data[i])
        table.insert(data, v & 0xff) table.insert(data, v >> 8)
    end
    table.insert(section_current.instructions, { data=data })
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
