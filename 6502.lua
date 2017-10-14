local M = {}

local symbols={} M.symbols=symbols
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
                symbols[section.label] = rorg(position)
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
                    symbols[section.label] = rorg(section.org)
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
            symbols[section.label] = section.location.rorg(section.org)
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
        if t == 'table' and type(v.resolve) == 'function' then symbols[k]=v.resolve() count=count+1 end
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

-- write a DASM symbol file for debuggers
M.writesym = function(filename)
    if not filename then filename = 'main.sym' end
    local f = assert(io.open(filename, "wb"), "failed to open " .. filename .. " for writing")
    table.sort(symbols)
    local ins,fmt,rep = table.insert,string.format,string.rep
    local s,sym_rev = {'--- Symbol List'},{}
    for k,v in pairs(symbols) do if type(v) == 'number' then ins(sym_rev,k) end end
    table.sort(sym_rev, function(a,b) local x,y=symbols[a],symbols[b] if x==y then return a<b end return x<y end)
    for _,v in ipairs(sym_rev) do
        local k=symbols[v]
        local u=v:match'.*()_' if u then -- change _ to . in local labels
            local parent=v:sub(1,u-1) if symbols[parent] then v = parent..'.'..v:sub(u+1) end
        end
        ins(s, fmt("%s%s %04x", v, rep(' ',24-#v), k))
    end
    s[#s+1] = '--- End of Symbol List.'
    f:write(table.concat(s, '\n'))
    f:close()
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
    label.resolve = function() return rorg(section.org + offset) end
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
local size_dc = function(v)
    if type(v) == 'function' then
        local r,x = M.pcall(v)
        if not r or not x then return v end
    end
    size_ref(v)
    return v
end
local size_op = function(late, early)
    if type(late) == 'function' then
        local r,x = M.pcall(late, early or 0, op_resolve)
        if not r or not x then return late,early end
        late=x early=nil
    end
    size_ref(late) size_ref(early)
    return late,early
end

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
    for c in s:gmatch'.' do t[c]=f(i) i=i+1 end
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

local op,cycles_def,xcross_def
op = function(code, cycles, extra_on_crosspage)
    return { opc=code, cycles=cycles or cycles_def, xcross=extra_on_crosspage or xcross_def }
end
local op_eval = function(late, early)
    local x = early or 0
    return type(late) == 'function' and late(x,op_resolve) or x+op_resolve(late)
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
        table.insert(M.section_current.instructions, { size=1, cycles=v.cycles, bin=v.opc })
    end
end
cycles_def=2 xcross_def=0 local opimm={
    adc=op(0x69), ['and']=op(0x29), cmp=op(0xc9), cpx=op(0xe0), cpy=op(0xc0), eor=op(0x49), lda=op(0xa9), ldx=op(0xa2),
    ldy=op(0xa0), ora=op(0x09), sbc=op(0xe9),
    anc=op(0x0b), ane=op(0x8b), arr=op(0x6b), asr=op(0x4b), jam=op(0x12,0), lax=op(0xab), nop=op(0x80), sbx=op(0xcb),
} M.opimm = opimm
for k,v in pairs(opimm) do
    M[k .. 'imm'] = function(late, early)
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local size = function() late,early = size_op(late,early) return 2 end
        local bin = function() local l65dbg=l65dbg return { v.opc, op_eval_byte(late,early) } end
        table.insert(M.section_current.instructions, { size=size, cycles=2, bin=bin })
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
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local size = function() late,early = size_op(late,early) return 2 end
        local bin = function() local l65dbg=l65dbg return { v.opc, op_eval_byte(late,early) } end
        table.insert(M.section_current.instructions, { size=size, cycles=v.cycles, bin=bin })
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
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local size = function() late,early = size_op(late,early) return 3 end
        local bin = function() local l65dbg=l65dbg 
            local x = op_eval_word(late,early)
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
            if x >= -128 and x <= 0xff then return M[k .. 'zpg'](late, early) end
            if x >= -32768 and x <= 0xffff then return M[k .. 'abs'](late, early) end
            error("value out of word range: " .. x)
        end
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local abs = opabs[k]
        local ins = { cycles=abs.cycles }
        ins.size = function() local l65dbg=l65dbg 
            local r,x = M.pcall_za(op_eval, late, early)
            if not r then return 3 end
            size_ref(x)
            x = word_normalize(x)
            local zpg = opzpg[k]
            if x <= 0xff and zpg then
                ins.size = 2
                ins.cycles = zpg.cycles
                ins.bin = function() return { zpg.opc, x } end
                return 2
            end
            ins.size = 3
            ins.bin = function() return { abs.opc, x&0xff, x>>8 } end
            return 3
        end
        ins.bin = function() local l65dbg=l65dbg 
            local x = word_normalize(op_eval(late, early))
            -- since we assumed absolute on link phase, we must generate absolute in binary
            if x <= 0xff and opzpg[k] then io.stderr:write("warning: forcing abs on zpg operand for opcode " .. k .. "\n") end
            return { abs.opc, x&0xff, x>>8 }
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
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local size = function() late,early = size_op(late,early) return 2 end
        local bin = function() local l65dbg=l65dbg return { v.opc, op_eval_byte(late,early) } end
        table.insert(M.section_current.instructions, { size=size, cycles=v.cycles, bin=bin })
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
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local size = function() late,early = size_op(late,early) return 3 end
        local bin = function() local l65dbg=l65dbg 
            local x = op_eval_word(late,early)
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
            if x >= -128 and x <= 0xff then return M[k .. 'zpx'](late, early) end
            if x >= -32768 and x <= 0xffff then return M[k .. 'abx'](late, early) end
            error("value out of word range: " .. x)
        end
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local abx = opabx[k]
        local ins = { cycles=abx.cycles }
        ins.size = function() local l65dbg=l65dbg 
            local r,x = M.pcall_za(op_eval, late, early)
            if not r then return 3 end
            size_ref(x)
            x = word_normalize(x)
            local zpx = opzpx[k]
            if x <= 0xff and zpx then
                ins.size = 2
                ins.cycles = zpx.cycles
                ins.bin = function() return { zpx.opc, x } end
                return 2
            end
            ins.size = 3
            ins.bin = function() return { abx.opc, x&0xff, x>>8 } end
            return 3
        end
        ins.bin = function() local l65dbg=l65dbg
            local x = word_normalize(op_eval(late, early))
            -- since we assumed absolute on link phase, we must generate absolute in binary
            if x <= 0xff and opzpx[k] then io.stderr:write("warning: forcing abx on zpx operand for opcode " .. k .. "\n") end
            return { abx.opc, x&0xff, x>>8 }
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
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local size = function() late,early = size_op(late,early) return 2 end
        local bin = function() local l65dbg=l65dbg return { v.opc, op_eval_byte(late,early) } end
        table.insert(M.section_current.instructions, { size=size, cycles=v.cycles, bin=bin })
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
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local size = function() late,early = size_op(late,early) return 3 end
        local bin = function() local l65dbg=l65dbg
            local x = op_eval_word(late,early)
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
            if x >= -128 and x <= 0xff then return M[k .. 'zpy'](late, early) end
            if x >= -32768 and x <= 0xffff then return M[k .. 'aby'](late, early) end
            error("value out of word range: " .. x)
        end
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local aby = opaby[k]
        local ins = { cycles=aby.cycles }
        ins.size = function() local l65dbg=l65dbg
            local r,x = M.pcall_za(op_eval, late, early)
            if not r then return 3 end
            size_ref(x)
            x = word_normalize(x)
            local zpy = opzpy[k]
            if x <= 0xff and zpy then
                ins.size = 2
                ins.cycles = zpy.cycles
                ins.bin = function() return { zpy.opc, x } end
                return 2
            end
            ins.size = 3
            ins.bin = function() return { aby.opc, x&0xff, x>>8 } end
            return 3
        end
        ins.bin = function() local l65dbg=l65dbg
            local x = word_normalize(op_eval(late, early))
            -- since we assumed absolute on link phase, we must generate absolute in binary
            if x <= 0xff and opzpy[k] then io.stderr:write("warning: forcing aby on zpy operand for opcode " .. k .. "\n") end
            return { aby.opc, x&0xff, x>>8 }
        end
        table.insert(M.section_current.instructions, ins)
    end
end
cycles_def=2 xcross_def=0 local oprel={
    bcc=op(0x90), bcs=op(0xb0), beq=op(0xf0), bmi=op(0x30), bne=op(0xd0), bpl=op(0x10), bvc=op(0x50), bvs=op(0x70),
} M.oprel = oprel
for k,v in pairs(oprel) do
    M[k .. 'rel'] = function(label)
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local parent,offset = M.label_current
        local section,rorg = M.section_current,M.location_current.rorg
        local op = { cycles=2 }
        op.size = function()
            offset = section.size
            label = size_dc(label)
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
    jmp=op(0x6c),
    jam=op(0xd2,0),
} M.opind = opind
for k,v in pairs(opind) do
    M[k .. 'ind'] = function(late, early)
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local size = function() late,early = size_op(late,early) return 3 end
        local bin = function() local l65dbg=l65dbg
            local x = op_eval_word(late,early)
            return { v.opc, x&0xff, x>>8 }
        end
        table.insert(M.section_current.instructions, { size=size, cycles=v.cycles, bin=bin })
    end
end
cycles_def=6 xcross_def=0 local opinx={
    adc=op(0x61), ['and']=op(0x21), cmp=op(0xc1), eor=op(0x41), lda=op(0xa1), ora=op(0x01), sbc=op(0xe1), sta=op(0x81),
    dcp=op(0xc3,8), isb=op(0xe3,8), jam=op(0x52,0), lax=op(0xa3), rla=op(0x23,8), rra=op(0x63,8), sax=op(0x83), slo=op(0x03,8),
    sre=op(0x43,8),
} M.opinx = opinx
for k,v in pairs(opinx) do
    M[k .. 'inx'] = function(late, early)
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local size = function() late,early = size_op(late,early) return 2 end
        local bin = function() local l65dbg=l65dbg return { v.opc, op_eval_byte(late,early) } end
        table.insert(M.section_current.instructions, { size=size, cycles=v.cycles, bin=bin })
    end
end
cycles_def=5 xcross_def=1 local opiny={
    adc=op(0x71), ['and']=op(0x31), cmp=op(0xd1), eor=op(0x51), lda=op(0xb1), ora=op(0x11), sbc=op(0xf1), sta=op(0x91,6),
    dcp=op(0xd3,8), isb=op(0xf3,8), jam=op(0x62,0,0), lax=op(0xb3), rla=op(0x33,8), rra=op(0x73,8), sha=op(0x93,6), slo=op(0x13,8),
    sre=op(0x53,8),
}
for k,v in pairs(opiny) do
    M[k .. 'iny'] = function(late, early)
        local l65dbg = { info=debug.getinfo(2, 'Sl'), trace=debug.traceback(nil, 1) }
        local size = function() late,early = size_op(late,early) return 2 end
        local bin = function() local l65dbg=l65dbg return { v.opc, op_eval_byte(late,early) } end
        table.insert(M.section_current.instructions, { size=size, cycles=v.cycles, bin=bin })
    end
end

return M
