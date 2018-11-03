local M = {}

local symbols,symbolsorg={},{} M.symbols,M.symbolsorg=symbols,symbolsorg
local locations={} M.locations=locations
local sections={} M.sections=sections
local relations={} M.relations=relations
local stats={} M.stats=stats setmetatable(stats, stats)

local before_link={} M.before_link=before_link

M.strip = true  -- set to false to disable dead stripping of relocatable sections
M.strip_empty = false -- set to true to strip empty sections: their label will then not resolve
M.pcall = pcall -- set to empty function returning false to disable eval during compute_size()
-- set to pcall directly if you want to keep ldazab/x/y eval during compute_size() even if
-- disabled for other parts (required to distinguish automatically between zp/abs addressing)
M.pcall_za = function(...) return M.pcall(...) end

M.__index = M
M.__newindex = function(t,k,v)
    local kk = k
    if type(k) == 'string' and k:sub(1,1) == '_' and M.label_current then
        kk = M.label_current .. k
    end
    if symbols[kk] then error("attempt to modify symbol " .. k) end
    rawset(t,k,v)
end
symbols.__index = symbols
setmetatable(M, symbols)

local id_ = 0
local id = function() id_=id_+1 return id_ end M.id=id

M.link = function()
    if stats.unused then return end

    for _,v in ipairs(before_link) do v() end

    if M.strip then
        symbols.__index = function(tab,key)
            local val = rawget(symbols, key)
            if type(val) == 'table' and val.type == 'label' then
                val.section.refcount = val.section.refcount + 1
            end
            return val
        end
    end
    for _,section in ipairs(sections) do
        section:compute_size()
    end
    symbols.__index = symbols

    local chunk_reserve = function(section, chunk_ix)
        local chunks = section.location.chunks
        local chunk = chunks[chunk_ix]
        local holes = section.holes
        local new_chunks,ins = {},table.insert

        local chunk1 = { id=id(), start=chunk.start, size=section.org-chunk.start }
        local hole_ix = 1
        local hole1 = holes[1]
        if hole1 and hole1.start==0 then
            chunk1.size = chunk1.size + hole1.size
            hole_ix = 2
        end
        if chunk1.size > 0 then ins(new_chunks, chunk1) end
        while hole_ix <= #holes do
            local hole = holes[hole_ix]
            local chunki = { id=id(), start=section.org+hole.start, size=hole.size }
            ins(new_chunks, chunki)
            hole_ix = hole_ix + 1
        end
        local chunkl = { id=id(), start=section.org+section.size, size=chunk.start+chunk.size-(section.org+section.size) }
        local chunkn = new_chunks[#new_chunks]
        if chunkn and chunkn.start+chunkn.size==chunkl.start then
            chunkn.size = chunkn.size + chunkl.size
        elseif chunkl.size > 0 then
            ins(new_chunks, chunkl)
        end

        table.remove(chunks, chunk_ix)
        for i=chunk_ix,chunk_ix+#new_chunks-1 do ins(chunks, i, new_chunks[i-chunk_ix+1]) end
    end

    local chunk_from_address = function(section, address)
        local chunks,rorg = section.location.chunks,section.location.rorg
        for i,chunk in ipairs(chunks) do
            if address >= chunk.start and address+section.size <= chunk.start+chunk.size then
                return chunk,i
            end
        end
    end

    local check_section_position = function(section, address, chunk)
        local chunk = chunk_from_address(section, address)
        if not chunk then return end
        local rorg = section.location.rorg
        if section.align then
            local raddress = rorg(address)
            if section.offset then raddress = raddress - section.offset end
            if raddress % section.align ~= 0 then return end
        end
        for _,constraint in ipairs(section.constraints) do
            local cstart, cfinish = address+constraint.start, address+constraint.finish
            if rorg(cstart) // 0x100 == rorg(cfinish) // 0x100 then
                if constraint.type == 'crosspage' then return end
            else
                if constraint.type == 'samepage' then return end
            end
        end
        local address_end = address+section.size
        local waste = math.min(address - chunk.start, chunk.size - (address_end - chunk.start))
        local raddress,raddress_end = rorg(address),rorg(address_end)
        local align,cross=0x100,0
        repeat
            local cross_count = (raddress_end+align-1)//align - (raddress+align-1)//align
            if raddress&(align-1) == 0 then cross_count=cross_count+1 end
            cross = cross + align * align * cross_count
            align = align>>1
        until align==1
        local lsb=0
        for i=0,15 do if raddress&(1<<i) == 0 then lsb=lsb+1 else break end end
        return waste, cross, lsb
    end

    local position_section = function(section, constrain)
        local location = section.location
        local chunks,rorg = location.chunks,location.rorg
        table.sort(chunks, function(a,b) if a.size==b.size then return a.id<b.id end return a.size<b.size end)
        for chunk_ix,chunk in ipairs(chunks) do if chunk.size >= section.size then
            local waste,cross,lsb,position = math.maxinteger,math.maxinteger,math.maxinteger
            local usage_lowest = function(start, finish)
                local inc=1
                if section.align then
                    local rstart = rorg(start)
                    local arstart = (rstart + section.align - 1) // section.align * section.align
                    if section.offset then arstart = arstart + section.offset end
                    start = start + arstart-rstart
                    inc = section.align
                end
                for address=start,finish,inc do
                    local nwaste, ncross, nlsb = check_section_position(section, address, chunk)
                    if nwaste then
                        if constrain then
                            nwaste, ncross, nlsb = constrain(address, nwaste, ncross, nlsb)
                            if not nwaste then goto skip end
                        end
                        if nwaste > waste then goto skip end
                        if nwaste == waste then
                            -- if waste is the same, keep the one that uses the least amount of aligned addresses
                            if ncross > cross then goto skip end
                            if ncross == cross then
                                -- if cross count is same, take the one with the most set LSB count (eg. select 11 over 10)
                                if nlsb > lsb then goto skip end
                            end
                        end
                        position,waste,cross,lsb = address,nwaste,ncross,nlsb
                        ::skip::
                    end
                end
            end
            local finish = math.min(chunk.start + 0xff, chunk.start + chunk.size - section.size)
            usage_lowest(chunk.start, finish)
            if chunk.size ~= math.huge then
                local start = math.max(chunk.start + chunk.size - section.size - 0xff, chunk.start)
                usage_lowest(start, chunk.start + chunk.size - section.size)
            end
            if position then
                section.org = position
                chunk_reserve(section, chunk_ix)
                --print(section.label, string.format("%04X\t%d", position, section.size))
                --for k,v in ipairs(location.chunks) do print(string.format("  %04X  %04X  %d", v.start, v.size+v.start-1, v.size)) end
                return position
            end
        end end
    end

    stats.used = 0
    stats.unused = 0
    stats.cycles = 0
    local related_sections = {}
    for _,location in ipairs(locations) do
        local sections,rorg = location.sections,location.rorg

        -- filter sections list
        local position_independent_sections = {}
        local symbols_to_remove = {}
        local section_count = #sections
        location.cycles=0 location.used=0
        for ix,section in ipairs(sections) do
            location.cycles = location.cycles + section.cycles
            location.used = location.used + section.size
            if section.size == 0 then
                if M.strip_empty or section.weak then
                    sections[ix]=nil
                    if not section.org then table.insert(symbols_to_remove, section.label) end
                else
                    section.org = location.start
                end
            elseif not section.org then
                if M.strip and not section.refcount and not section.strong then 
                    sections[ix]=nil
                    table.insert(symbols_to_remove, section.label)
                elseif section.related then
                    table.insert(related_sections, section)
                else
                    table.insert(position_independent_sections, section)
                end
            end
        end
        do local j=0 for i=1,section_count do
            if sections[i] ~= nil then j=j+1 sections[j],sections[i] = sections[i],sections[j] end
        end end
        for _,v in ipairs(symbols_to_remove) do symbols[v] = nil end
        location.position_independent_sections = position_independent_sections
        stats.cycles = stats.cycles + location.cycles
        stats.used = stats.used + location.used

        -- fixed position sections
        for section_ix,section in ipairs(sections) do if section.org then
            if section.org < location.start or section.org > (location.finish or math.huge) then
                error("ORG section " .. section.label .. " starts outside container location")
            end
            for chunk_ix,chunk in ipairs(location.chunks) do
                if chunk.start <= section.org and chunk.size - (section.org - chunk.start) >= section.size then
                    chunk_reserve(section, chunk_ix)
                    goto chunk_located
                end
            end
            error("ORG section " .. section.label .. " overflows its location")
            ::chunk_located::
        end end
    end

    table.sort(related_sections, function(a,b) if a.size==b.size then return a.id<b.id end return a.size>b.size end)
    for _,section in ipairs(related_sections) do if not section.org then
        local related,ins = {},table.insert
        local function collect(section_parent, offset)
            local relatives = relations[section_parent]
            if relatives then
                for relative,relative_offset in pairs(relatives) do
                    if not related[relative] and relative ~= section then
                        relative_offset = relative_offset + offset
                        related[relative] = relative_offset
                        collect(relative, relative_offset)
                    end
                end
            end
        end
        collect(section, 0)
        local location_start = section.location.start
        local position = position_section(section, function(address, waste, cross, lsb)
            local waste, cross, lsb = 0, 0, 0
            for section,offset in pairs(related) do
                local section_address = address + (section.location.start - location_start) + offset
                local nwaste, ncross, nlsb = check_section_position(section, section_address)
                if not nwaste then return end
                waste, cross, lsb = waste+nwaste, cross+ncross, lsb+nlsb
            end
            return waste, cross, lsb
        end)
        if not position then
            error("unable to find space for section " .. section.label)
        end
        for section,offset in pairs(related) do
            section.org = position + (section.location.start - location_start) + offset
            local chunk,chunk_ix = chunk_from_address(section, section.org)
            chunk_reserve(section, chunk_ix)
        end
    end end

    for _,location in ipairs(locations) do
        local position_independent_sections = location.position_independent_sections
        table.sort(position_independent_sections, function(a,b) if a.size==b.size then return a.label>b.label end return a.size>b.size end)
        for _,section in ipairs(position_independent_sections) do
            if not position_section(section) then
                error("unable to find space for section " .. section.label)
            end
        end

        -- unused space stats
        local unused = 0
        for _,chunk in ipairs(location.chunks) do
            if chunk.size ~= math.huge then
                unused = unused + chunk.size
            else
                location.stops_at = chunk.start-1
            end
        end
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
        if t == 'table' and type(v.resolve) == 'function' then symbols[k],symbolsorg[k]=v.resolve() count=count+1 end
        if t == 'string' and symbols[v] then symbols[k]=symbols[v] count=count+1 end
        stats.resolved_count = stats.resolved_count + count
    end end until count == 0

    -- set local label references resolver
    local llresolver = { __index = function(tab,key)
        if type(key) ~= 'string' or key:sub(1,1) ~= '_' or not M.label_current then return nil end
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
    local fill
    for _,location in ipairs(locations) do
        if location.start < #bin then
            error(string.format("location [%04x,%04x] overlaps another", location.start, location.finish or location.stops_at))
        end
        if fill then for i=#bin+of0,location.start-1 do ins(bin, filler) end end
        M.size=0 M.cycles=0
        local sections = location.sections
        table.sort(sections, function(a,b) return a.org < b.org end)
        for _,section in ipairs(sections) do
            for i=#bin+of0,section.org-1 do ins(bin, filler) end
            local bin_offset = math.min(#bin, section.org-of0)+1
            for _,instruction in ipairs(section.instructions) do
                local b,o = instruction.bin
                if type(b) == 'function' then b,o = b(filler) end
                if type(b) == 'table' then mov(b,1,#b,bin_offset,bin) bin_offset=bin_offset+#b
                elseif b then bin[bin_offset]=b bin_offset=bin_offset+1 end
                if o then
                    bin_offset=bin_offset+o
                    for i=#bin,bin_offset-1 do ins(bin, filler) end
                end
                M.size=#bin M.cycles=M.cycles+(instruction.cycles or 0)
            end
        end
        fill = not location.nofill
        if location.finish and fill then
            for i=#bin+of0,location.finish do ins(bin, filler) end
        end
    end
    stats.bin_size = #bin
    return bin
end

M.writebin = function(filename, bin)
    if not filename then filename = 'main.bin' end
    if not bin then bin = M.genbin() end
    local f = assert(io.open(filename, "wb"), "failed to open " .. filename .. " for writing")
    f:write(string.char(table.unpack(bin)))
    f:close()
end

-- return a table of entry(address, label)
M.getsym = function(entry)
    local ins = table.insert
    local s,sym_rev = {},{}
    for k,v in pairs(symbols) do if type(v) == 'number' then ins(sym_rev,k) end end
    table.sort(sym_rev, function(a,b) local x,y=symbols[a],symbols[b] if x==y then return a<b end return x<y end)
    for _,v in ipairs(sym_rev) do
        local k,vorg=symbols[v],v
        local u=v:match'.*()_' if u then -- change _ to . in local labels
            local parent=v:sub(1,u-1) if symbols[parent] then v = parent..'.'..v:sub(u+1) end
        end
        local e = entry(k,v,vorg) if e then
            if type(e) == 'table' then for _,ev in ipairs(e) do ins(s, ev) end
            else ins(s, e) end
        end
    end
    return s
end
M.getsym_as = {
    lua = function() -- .lua
        local fmt,rep = string.format,string.rep
        local s = M.getsym(function(a,l) return fmt("%s = 0x%04x", l, a) end)
        return table.concat(s, '\n')
    end,
    dasm = function() -- .sym
        local fmt,rep = string.format,string.rep
        local s = M.getsym(function(a,l) return fmt("%s%s %04x", l, rep(' ',24-#l), a) end)
        table.insert(s, 1, '--- Symbol List')
        s[#s+1] = '--- End of Symbol List.'
        return table.concat(s, '\n')
    end,
}
-- write a symbol file for debuggers, using specified format (defaults to DASM)
M.writesym = function(filename, format)
    assert(filename)
    local s = M.getsym_as[format or 'dasm'](filename)
    if s then
        local f = assert(io.open(filename, "wb"), "failed to open " .. filename .. " for writing")
        f:write(s) f:close()
    end
end

stats.__tostring = function()
    local s,ins={},table.insert
    ins(s, "                Free  Used  Size     Area")
    for _,location in ipairs(locations) do
        local name = (location.name or ''):sub(1,14)
        name = string.rep(' ', 14-#name) .. name
        local fmt = "%s  %5d %5d %5d [%04X-%04X]"
        if location.finish then
            local size = location.finish-location.start+1
            ins(s, string.format(fmt, name,
                location.unused, size-location.unused, size, location.start, location.finish))
        else
            ins(s, string.format(fmt, name,
                location.unused, location.used, location.stops_at-location.start+1, location.start, location.stops_at))
        end
    end
    if #locations > 1 then
        ins(s, string.format(" --- Total ---  %5d %5d %5d", stats.unused, stats.used, stats.bin_size))
    end
    return table.concat(s, '\n')
end

M.location = function(start, finish)
    local location
    if type(start) ~= 'table' then
        location = { start=start, finish=finish }
    else
        if start.type == 'location' then
            for _,v in ipairs(locations) do if v == start then
                M.location_current = start
                return start
            end end
            error("unable to find reference to location [" .. (start.start or '?') .. ", " .. (start.finish or '?') .. "]")
        end
        location = start
        location.start = start[1]
        location.finish = start[2]
        if type(location.rorg) == 'number' then
            local offset = location.rorg - location.start
            location.rorg = function(x) return x+offset end
        end
    end
    location.type = 'location'
    location.sections = {}
    if not location.rorg then location.rorg = function(x) return x end end
    local size = (location.finish or math.huge) - location.start + 1
    location.chunks={ { id=id(), start=location.start, size=size } }
    locations[#locations+1] = location
    M.location_current = location
    return location
end

M.section = function(t)
    local section = {}
    local name = t or 'S'..id()
    if type(name) ~= 'string' then
        assert(type(t) == 'table', "invalid arguments for section")
        if t.type == 'section' then
            for _,v in ipairs(sections) do if v == t then
                M.location_current = t.location
                M.section_current = t
                return t
            end end
            error("unable to find reference to section " .. (t.label or '?'))
        end
        section=t name=t[1] or 'S'..id() section[1]=nil
        if section.offset and not section.align then error("section " .. name .. " has offset, but no align") end
    end
    table.insert(M.location_current.sections, section)
    table.insert(M.sections, section)
    section.location = M.location_current
    M.section_current = section
    section.type = 'section'
    section.id = id()
    section.constraints = {}
    section.instructions = {}
    assert(name:sub(1,1) ~= '_', "sections can't be named with a local label")
    section.label = M.label(name)
    section.holes = {}
    section.refcount = 0
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
            constraint.finish = constraint.to==#instructions and self.size or instructions[constraint.to+1].offset
        end
    end
    return section
end

-- relate(section1, section2 [, [offset1,] offset2])
-- Add a position relationship between 'section1' and 'section2', with 'offset1'
-- bytes from selected position for 'section2', and 'offset2' bytes from selec-
-- -ted positon for 'section1'.
-- If offset1 is omitted, -offset2 is used.
M.relate = function(section1, section2, offset, offset2)
    assert(section1.type == 'section', "section1 is not a section")
    assert(section2.type == 'section', "section2 is not a section")
    local rel1 = relations[section1] or {}
    rel1[section2] = (offset2 or offset) or 0
    relations[section1] = rel1
    local rel2 = relations[section2] or {}
    rel2[section1] = (offset2 and offset) or -rel1[section2]
    relations[section2] = rel2
    section1.related = true
    section2.related = true
end

M.label = function(name)
    local label,offset
    local section,rorg = M.section_current,M.location_current.rorg
    label = { type='label', section=section }
    if not name then name='_L'..id() end
    if name:sub(1,1) == '_' then -- local label
        name = M.label_current .. name
    else
        M.label_current = name
        label.bin = function() M.label_current = name end
    end
    if symbols[name] then error("duplicate symbol: " .. name) end
    symbols[name] = label
    label.label = name
    label.size = function()
        offset = section.size
        label.size = 0
        return 0
    end
    label.resolve = function()
        local o = section.org + offset
        return rorg(o),o
    end
    table.insert(section.instructions, label)
    return name,label
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

-- skip(bytes)
-- Insert a hole in the section of 'bytes' bytes, which can be used by other
-- relocatable sections.
M.skip = function(bytes)
    local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
    local ins,section = {},M.section_current
    ins.size = function()
        table.insert(section.holes, { start=ins.offset, size=bytes })
        return bytes
    end
    ins.bin = function(filler) return nil,bytes end
    table.insert(section.instructions, ins)
end

-- sleep(cycles [, noillegal])
-- Waste 'cycles' cycles. If 'noillegal' is true, trashes NZ flags.
M.sleep = function(cycles, noillegal)
    assert(cycles > 1, "can't sleep for less than 2 cycles")
    if cycles & 1 ~= 0 then
        if noillegal then bitzpg(0) else nopzpg(0) end
        cycles = cycles - 3
    end
    for i=1,cycles/2 do nopimp() end
end

local op_resolve = function(v)
    if type(v) == 'function' then v=v() end
    if type(v) == 'table' and v.label then v = symbols[v.label] end
    if type(v) == 'string' then v = symbols[v] end
    if type(v) ~= 'number' then error("unresolved symbol: " .. tostring(v)) end
    return v
end M.op_resolve = op_resolve

local size_ref = function(v)
    if type(v) == 'string' then v=symbols[v] end
    if type(v) == 'table' and v.type == 'label' then v.section.refcount = 1 + (v.section.refcount or 0) end
end
M.size_ref = size_ref

local size_dc = function(v)
    if type(v) == 'function' then
        local r,x = M.pcall(v)
        if not r or not x then return v end
    end
    size_ref(v)
    return v
end
M.size_dc = size_dc

local size_op = function(late, early)
    if type(late) == 'function' then
        local r,x = M.pcall(late, early or 0, op_resolve)
        if not r or not x then return late,early end
        late=x early=nil
    end
    size_ref(late) size_ref(early)
    return late,early
end
M.size_op = size_op

local byte_normalize = function(v)
    if v < -0x80 or v > 0xFF then error("value out of byte range: " .. v) end
    if v < 0 then v = v + 0x100 end
    return v & 0xff
end
M.byte_normalize = byte_normalize

local word_normalize = function(v)
    if v < -0x8000 or v > 0xFFFF then error("value out of word range: " .. v) end
    if v < 0 then v = v + 0x10000 end
    return v & 0xffff
end
M.word_normalize = word_normalize

local long_normalize = function(v)
    if v < -0x80000000 or v > 0xFFFFFFFF then error("value out of word range: " .. v) end
    if v < 0 then v = v + 0x100000000 end
    return v & 0xffffffff
end
M.long_normalize = long_normalize

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
    local t,i={},0
    for c in s:gmatch'.' do local v=i t[c]=function() return f(v) end i=i+1 end
    M.cs=t
    return t
end

M.byte_impl = function(args, nrm)
    local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
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
    local size = function()
        for i,v in ipairs(data) do data[i] = size_dc(v) end
        return #data
    end
    local bin = function() local l65dbg=l65dbg
        local b={}
        for k,v in ipairs(data) do
            if type(v) == 'function' then v = v() end
            local vt = type(v)
            if vt == 'table' and v.label then v = symbols[v.label]
            elseif vt == 'string' then v = symbols[v] end
            if type(v) ~= 'number' then error("unresolved symbol for dc.b, index " .. k) end 
            b[#b+1] = nrm(v)
        end
        return b
    end
    table.insert(M.section_current.instructions, { data=data, size=size, bin=bin })
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
        local vt = type(v)
        if vt == 'string' or vt == 'table' and (v.type == 'section' or v.type == 'label') then
            args[k] = function() return v end
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
    local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
    local args = {...}
    local data = {}
    for k,v in ipairs(args) do
        local t = type(v)
        if t == 'number' or t == 'function' or t == 'string' then data[#data+1] = v
        elseif t == 'table' then
            if v.type == 'section' or v.type == 'label' then data[#data+1] = function() return v end
            else table.move(v,1,#v,#data+1,data) end
        else error("unsupported type for word() argument: " .. t .. ", value: " .. v)
        end
    end
    local size = function()
        for i,v in ipairs(data) do data[i] = size_dc(v) end
        return #data*2
    end
    local bin = function() local l65dbg=l65dbg
        local b={}
        for k,v in ipairs(data) do
            if type(v) == 'function' then v = v() end
            local vt = type(v)
            if vt == 'table' and v.label then v = symbols[v.label]
            elseif vt == 'string' then v = symbols[v] end
            if type(v) ~= 'number' then error("unresolved symbol for dc.w, index " .. k) end 
            v = word_normalize(v)
            b[#b+1] = v&0xff
            b[#b+1] = v>>8
        end
        return b
    end
    table.insert(M.section_current.instructions, { data=data, size=size, bin=bin })
end

M.long = function(...)
    local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
    local args = {...}
    local data = {}
    for k,v in ipairs(args) do
        local t = type(v)
        if t == 'number' or t == 'function' or t == 'string' then data[#data+1] = v
        elseif t == 'table' then
            if v.type == 'section' or v.type == 'label' then data[#data+1] = function() return v end
            else table.move(v,1,#v,#data+1,data) end
        else error("unsupported type for long() argument: " .. t .. ", value: " .. v)
        end
    end
    local size = function()
        for i,v in ipairs(data) do data[i] = size_dc(v) end
        return #data*4
    end
    local bin = function() local l65dbg=l65dbg
        local b={}
        for k,v in ipairs(data) do
            if type(v) == 'function' then v = v() end
            local vt = type(v)
            if vt == 'table' and v.label then v = symbols[v.label]
            elseif vt == 'string' then v = symbols[v] end
            if type(v) ~= 'number' then error("unresolved symbol for dc.l, index " .. k) end 
            v = long_normalize(v)
            b[#b+1] = v&0xff
            b[#b+1] = (v>>8)&0xff
            b[#b+1] = (v>>16)&0xff
            b[#b+1] = v>>24
        end
        return b
    end
    table.insert(M.section_current.instructions, { data=data, size=size, bin=bin })
end

local op = function(code, cycles, extra_on_crosspage)
    return { opc=code, cycles=cycles or cycles_def, xcross=extra_on_crosspage or xcross_def }
end
M.op = op

local op_eval = function(late, early)
    local x = early or 0
    return type(late) == 'function' and late(x,op_resolve) or x+op_resolve(late)
end
M.op_eval = op_eval

local op_eval_byte = function(late, early, nozp)
    local v = op_eval(late, early)
    local zpv = zeropage(v)
    if not nozp and zpv then return zpv end
    return byte_normalize(v)
end
M.op_eval_byte = op_eval_byte

local op_eval_word = function(late, early) return word_normalize(op_eval(late, early)) end
M.op_eval_word = op_eval_word

return M