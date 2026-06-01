local M = require "asm"
M.gameboy = false

local function die(msg)
    error(msg, 3)
end

local function eval(v)
    if type(v) == "function" then v = v() end
    if type(v) == "string" then
        local s = M.symbols[v]
        if s ~= nil then v = s end
    end
    return v
end

local function byte(v)
    v = eval(v)
    if type(v) == "string" or type(v) == "table" then v = M.op_resolve(v) end
    return M.byte_normalize(v)
end

local function signed_byte(v, sign)
    v = eval(v)
    if type(v) == "string" or type(v) == "table" then v = M.op_resolve(v) end
    if sign == "-" then v = -v end
    if v < -128 or v > 127 then die("signed byte out of range: " .. v) end
    return v & 0xff
end

local function word(v)
    v = eval(v)
    if type(v) == "string" or type(v) == "table" then v = M.op_resolve(v) end
    return M.word_normalize(v)
end

local function wordle(v)
    v = word(v)
    return v & 0xff, v >> 8
end

local function emit(size, bin, cycles)
    table.insert(M.section_current.instructions, {
        size = function() return size end,
        cycles = cycles,
        bin = type(bin) == "function" and bin or function() return bin end,
    })
end

local function emit_rel(opc, target, cycles)
    local parent, offset = M.label_current
    local section, rorg = M.section_current, M.location_current.rorg
    local ins = { cycles = cycles or 12 }
    ins.size = function()
        offset = section.size
        return 2
    end
    ins.bin = function()
        local x, label = eval(target), target
        if type(label) == "string" and label:sub(1, 1) == "_" then
            x = parent .. label
        end
        if type(x) == "string" or type(x) == "table" then x = M.op_resolve(x) end
        x = x - 2 - offset - rorg(section.org)
        if x < -128 or x > 127 then die("relative branch out of range: " .. x) end
        return { opc, x & 0xff }
    end
    table.insert(M.section_current.instructions, ins)
end

function M.imm(v) return { kind = "imm", value = v } end
function M.mem(v) return { kind = "mem", value = v } end
function M.spoff(sign, v) return { kind = "spoff", sign = sign, value = v } end
function M.idx(r, sign, disp)
    local d = byte(disp or 0)
    if sign == "-" then d = (-d) & 0xff end
    return { kind = "idx", reg = r, disp = d }
end

local r8 = { b = 0, c = 1, d = 2, e = 3, h = 4, l = 5, a = 7 }
local rp = { bc = 0, de = 1, hl = 2, sp = 3 }
local rp2 = { bc = 0, de = 1, hl = 2, af = 3 }
local cc = { nz = 0, z = 1, nc = 2, c = 3, po = 4, pe = 5, p = 6, m = 7 }
local alu = { add = 0x80, adc = 0x88, sub = 0x90, sbc = 0x98, ["and"] = 0xa0, xor = 0xa8, ["or"] = 0xb0, cp = 0xb8 }

local function is_mem(o) return type(o) == "table" and o.kind == "mem" end
local function is_idx(o) return type(o) == "table" and o.kind == "idx" end
local function is_imm(o) return type(o) == "table" and o.kind == "imm" end
local function is_spoff(o) return type(o) == "table" and o.kind == "spoff" end
local function imm_value(o)
    if not is_imm(o) then die("immediate operand requires #") end
    return o.value
end
local function condition_code(c)
    local code = cc[c]
    if code == nil then return end
    if M.gameboy and code > 3 then die("condition is not available on Game Boy") end
    return code
end
local function prefix_for(r)
    if M.gameboy and (r == "ix" or r == "iy") then die("IX/IY are not available on Game Boy") end
    return r == "ix" and 0xdd or r == "iy" and 0xfd
end

local function r8code(o)
    if type(o) == "string" and r8[o] then return r8[o] end
    if is_mem(o) and o.value == "hl" then return 6 end
    if is_idx(o) then return 6, prefix_for(o.reg), o.disp end
end

local function emit_r8(prefix, opc, disp)
    if prefix then emit(3, { prefix, opc, disp }) else emit(1, { opc }) end
end

local function emit_r8_n(prefix, opc, disp, n)
    if prefix then emit(4, function() return { prefix, opc, disp, byte(n) } end) else emit(2, function() return { opc, byte(n) } end) end
end

local function rpcode(o, allow_index)
    if type(o) ~= "string" then return end
    if allow_index and (o == "ix" or o == "iy") then return 2, prefix_for(o) end
    return rp[o]
end

local function rp2code(o, allow_index)
    if type(o) ~= "string" then return end
    if allow_index and (o == "ix" or o == "iy") then return 2, prefix_for(o) end
    return rp2[o]
end

function M.ld(a, b)
    a, b = eval(a), eval(b)
    if is_spoff(b) then
        if not M.gameboy or a ~= "hl" then die("unsupported ld") end
        return emit(2, function() return { 0xf8, signed_byte(b.value, b.sign) } end)
    end
    if a == "sp" and (b == "hl" or b == "ix" or b == "iy") then
        local p = prefix_for(b)
        return emit(p and 2 or 1, p and { p, 0xf9 } or { 0xf9 })
    end
    local ac, ap, ad = r8code(a)
    local bc, bp, bd = r8code(b)
    if ac and bc then
        if ap and bp then die("cannot use two indexed operands") end
        local p, d = ap or bp, ad or bd
        return emit_r8(p, 0x40 | (ac << 3) | bc, d)
    end
    if ac and not is_mem(b) then return emit_r8_n(ap, 0x06 | (ac << 3), ad, imm_value(b)) end
    if a == "a" and is_mem(b) and (b.value == "bc" or b.value == "de") then
        return emit(1, { b.value == "bc" and 0x0a or 0x1a })
    end
    if is_mem(a) and (a.value == "bc" or a.value == "de") and b == "a" then
        return emit(1, { a.value == "bc" and 0x02 or 0x12 })
    end
    if M.gameboy and a == "a" and is_mem(b) and b.value == "c" then
        return emit(1, { 0xf2 })
    end
    if M.gameboy and is_mem(a) and a.value == "c" and b == "a" then
        return emit(1, { 0xe2 })
    end
    if a == "a" and is_mem(b) then
        return emit(3, function() local lo, hi = wordle(b.value); return { M.gameboy and 0xfa or 0x3a, lo, hi } end)
    end
    if is_mem(a) and b == "a" then
        return emit(3, function() local lo, hi = wordle(a.value); return { M.gameboy and 0xea or 0x32, lo, hi } end)
    end
    if (a == "i" or a == "r") and b == "a" then return emit(2, { 0xed, a == "i" and 0x47 or 0x4f }) end
    if a == "a" and (b == "i" or b == "r") then return emit(2, { 0xed, b == "i" and 0x57 or 0x5f }) end

    local rc, p = rpcode(a, true)
    if rc and not is_mem(b) then
        return emit(p and 4 or 3, function()
            local lo, hi = wordle(imm_value(b))
            return p and { p, 0x01 | (rc << 4), lo, hi } or { 0x01 | (rc << 4), lo, hi }
        end)
    end
    if is_mem(a) then
        rc, p = rpcode(b, true)
        if M.gameboy and b == "sp" then
            return emit(3, function() local lo, hi = wordle(a.value); return { 0x08, lo, hi } end)
        end
        if M.gameboy then die("unsupported ld") end
        if rc then
            return emit((p or b ~= "hl") and 4 or 3, function()
                local lo, hi = wordle(a.value)
                if p then return { p, 0x22, lo, hi } end
                if b == "hl" then return { 0x22, lo, hi } end
                return { 0xed, 0x43 | (rc << 4), lo, hi }
            end)
        end
    end
    if is_mem(b) then
        if M.gameboy then die("unsupported ld") end
        rc, p = rpcode(a, true)
        if rc then
            return emit((p or a ~= "hl") and 4 or 3, function()
                local lo, hi = wordle(b.value)
                if p then return { p, 0x2a, lo, hi } end
                if a == "hl" then return { 0x2a, lo, hi } end
                return { 0xed, 0x4b | (rc << 4), lo, hi }
            end)
        end
    end
    die("unsupported ld")
end

-- Operator assignments use the shortest Game Boy encoding available for
-- absolute loads through A. Mnemonic ld/ldh remain explicit.
function M.ld_auto(a, b)
    a, b = eval(a), eval(b)
    if not M.gameboy then return M.ld(a, b) end

    local address, absolute_opcode, high_opcode
    if a == "a" and is_mem(b) and type(b.value) ~= "string" then
        address, absolute_opcode, high_opcode = b.value, 0xfa, 0xf0
    elseif is_mem(a) and type(a.value) ~= "string" and b == "a" then
        address, absolute_opcode, high_opcode = a.value, 0xea, 0xe0
    else
        return M.ld(a, b)
    end

    local ins = {}
    ins.size = function()
        local ok, value = M.pcall_za(word, address)
        if not ok then return 3 end
        if value >= 0xff00 then
            ins.size = 2
            ins.bin = function() return { high_opcode, value & 0xff } end
            return 2
        end
        ins.size = 3
        ins.bin = function() return { absolute_opcode, value & 0xff, value >> 8 } end
        return 3
    end
    ins.bin = function()
        local value = word(address)
        return { absolute_opcode, value & 0xff, value >> 8 }
    end
    table.insert(M.section_current.instructions, ins)
end

local function alu_a(name, a, b)
    a, b = eval(a), eval(b)
    if b ~= nil then
        if a ~= "a" then die(name .. " only supports A as explicit accumulator") end
        a = b
    end
    local rc, p, d = r8code(a)
    if rc then return emit_r8(p, alu[name] | rc, d) end
    emit(2, function() return { alu[name] | 0x46, byte(imm_value(a)) } end)
end

function M.add(a, b)
    if M.gameboy and a == "sp" and b ~= nil then
        return emit(2, function() return { 0xe8, byte(imm_value(eval(b))) } end)
    end
    if b ~= nil and (a == "hl" or a == "ix" or a == "iy") then
        local rc = rpcode(b, a == "ix" or a == "iy")
        if rc == nil then die("unsupported add") end
        local p = prefix_for(a)
        return emit(p and 2 or 1, p and { p, 0x09 | (rc << 4) } or { 0x09 | (rc << 4) })
    end
    return alu_a("add", a, b)
end

for _, name in ipairs{ "adc", "sbc" } do
    M[name] = function(a, b)
        if b ~= nil and a == "hl" then
            if M.gameboy then die(name .. " hl is not available on Game Boy") end
            local rc = rpcode(b)
            if rc == nil then die("unsupported " .. name) end
            return emit(2, { 0xed, (name == "adc" and 0x4a or 0x42) | (rc << 4) })
        end
        return alu_a(name, a, b)
    end
end

for _, name in ipairs{ "sub", "and", "or", "xor", "cp" } do
    M[name] = function(a) return alu_a(name, a) end
end
M.ana = M["and"]
M.ora = M["or"]

local function incdec(name, o)
    o = eval(o)
    local rc, p, d = r8code(o)
    if rc then return emit_r8(p, (name == "inc" and 0x04 or 0x05) | (rc << 3), d) end
    local r, rpfx = rpcode(o, true)
    if r then return emit(rpfx and 2 or 1, rpfx and { rpfx, (name == "inc" and 0x03 or 0x0b) | (r << 4) } or { (name == "inc" and 0x03 or 0x0b) | (r << 4) }) end
    die("unsupported " .. name)
end
function M.inc(o) return incdec("inc", o) end
function M.dec(o) return incdec("dec", o) end

function M.jp(a, b)
    a, b = eval(a), eval(b)
    if b then
        local c = condition_code(a)
        if not c then die("invalid jp condition") end
        return emit(3, function() local lo, hi = wordle(b); return { 0xc2 | (c << 3), lo, hi } end)
    end
    if is_mem(a) and (a.value == "hl" or a.value == "ix" or a.value == "iy") then
        local p = prefix_for(a.value)
        return emit(p and 2 or 1, p and { p, 0xe9 } or { 0xe9 })
    end
    emit(3, function() local lo, hi = wordle(a); return { 0xc3, lo, hi } end)
end

function M.jr(a, b)
    a, b = eval(a), eval(b)
    if b then
        local c = ({ nz = 0x20, z = 0x28, nc = 0x30, c = 0x38 })[a]
        if not c then die("invalid jr condition") end
        return emit_rel(c, b, 12)
    end
    emit_rel(0x18, a, 12)
end
function M.djnz(a) emit_rel(0x10, a, 13) end

function M.call(a, b)
    a, b = eval(a), eval(b)
    if b then
        local c = condition_code(a)
        if not c then die("invalid call condition") end
        return emit(3, function() local lo, hi = wordle(b); return { 0xc4 | (c << 3), lo, hi } end)
    end
    emit(3, function() local lo, hi = wordle(a); return { 0xcd, lo, hi } end)
end

function M.ret(c)
    c = eval(c)
    if c then
        local code = condition_code(c)
        if not code then die("invalid ret condition") end
        return emit(1, { 0xc0 | (code << 3) })
    end
    emit(1, { 0xc9 })
end
function M.reti() emit(2, { 0xed, 0x4d }) end
function M.retn()
    if M.gameboy then die("retn is not available on Game Boy") end
    emit(2, { 0xed, 0x45 })
end
function M.rst(n)
    emit(1, function()
        n = byte(imm_value(eval(n)))
        if n & 0xc7 ~= 0 or n > 0x38 then die("invalid rst vector") end
        return { 0xc7 | n }
    end)
end

function M.push(r)
    r = eval(r)
    local c, p = rp2code(r, true)
    if c == nil then die("unsupported push") end
    emit(p and 2 or 1, p and { p, 0xc5 | (c << 4) } or { 0xc5 | (c << 4) })
end
function M.pop(r)
    r = eval(r)
    local c, p = rp2code(r, true)
    if c == nil then die("unsupported pop") end
    emit(p and 2 or 1, p and { p, 0xc1 | (c << 4) } or { 0xc1 | (c << 4) })
end

local rotbase = { rlc = 0x00, rrc = 0x08, rl = 0x10, rr = 0x18, sla = 0x20, sra = 0x28, srl = 0x38 }
for name, base in pairs(rotbase) do
    M[name] = function(o)
        o = eval(o)
        local rc, p, d = r8code(o)
        if not rc then die("unsupported " .. name) end
        return emit(p and 4 or 2, p and { p, 0xcb, d, base | rc } or { 0xcb, base | rc })
    end
end
for name, base in pairs{ bit = 0x40, res = 0x80, set = 0xc0 } do
    M[name] = function(bit, o)
        bit, o = eval(bit), eval(o)
        local rc, p, d = r8code(o)
        if not rc then die("unsupported " .. name) end
        local opc = base | (byte(imm_value(bit)) << 3) | rc
        return emit(p and 4 or 2, p and { p, 0xcb, d, opc } or { 0xcb, opc })
    end
end

function M.swap(o)
    o = eval(o)
    local rc, p, d = r8code(o)
    if not rc then die("unsupported swap") end
    if not M.gameboy then die("swap is only available in Game Boy mode") end
    return emit(p and 4 or 2, p and { p, 0xcb, d, 0x30 | rc } or { 0xcb, 0x30 | rc })
end

function M.ex(a, b)
    if M.gameboy then die("ex is not available on Game Boy") end
    a, b = eval(a), eval(b)
    if a == "de" and b == "hl" then return emit(1, { 0xeb }) end
    if a == "af" and b == "af2" then return emit(1, { 0x08 }) end
    if is_mem(a) and a.value == "sp" and (b == "hl" or b == "ix" or b == "iy") then
        local p = prefix_for(b)
        return emit(p and 2 or 1, p and { p, 0xe3 } or { 0xe3 })
    end
    die("unsupported ex")
end

function M.im(n)
    if M.gameboy then die("im is not available on Game Boy") end
    emit(2, function() return { 0xed, ({ [0] = 0x46, 0x56, 0x5e })[byte(imm_value(eval(n)))] or die("invalid interrupt mode") } end)
end

M["in"] = function(a, b)
    if M.gameboy then die("in is not available on Game Boy") end
    a, b = eval(a), eval(b)
    if a == "a" and is_mem(b) and b.value ~= "c" then return emit(2, function() return { 0xdb, byte(b.value) } end) end
    local rc = r8[a]
    if rc and is_mem(b) and b.value == "c" then return emit(2, { 0xed, 0x40 | (rc << 3) }) end
    die("unsupported in")
end
function M.out(a, b)
    if M.gameboy then die("out is not available on Game Boy") end
    a, b = eval(a), eval(b)
    if is_mem(a) and a.value ~= "c" and b == "a" then return emit(2, function() return { 0xd3, byte(a.value) } end) end
    local rc = r8[b]
    if is_mem(a) and a.value == "c" and rc then return emit(2, { 0xed, 0x41 | (rc << 3) }) end
    die("unsupported out")
end

for name, opc in pairs{
    ccf=0x3f, cpl=0x2f, daa=0x27, di=0xf3, ei=0xfb, exx=0xd9, halt=0x76, nop=0x00,
    rla=0x17, rlca=0x07, rra=0x1f, rrca=0x0f, scf=0x37,
} do M[name] = function()
    if M.gameboy and name == "exx" then die("exx is not available on Game Boy") end
    emit(1, { opc })
end end

function M.stop()
    if not M.gameboy then die("stop is only available in Game Boy mode") end
    emit(2, { 0x10, 0x00 })
end

for name, opc in pairs{
    neg=0x44, rld=0x6f, rrd=0x67, ldi=0xa0, ldir=0xb0, ldd=0xa8, lddr=0xb8,
    cpi=0xa1, cpir=0xb1, cpd=0xa9, cpdr=0xb9, ini=0xa2, inir=0xb2, ind=0xaa, indr=0xba,
    outi=0xa3, otir=0xb3, outd=0xab, otdr=0xbb,
} do M[name] = function()
    if M.gameboy then die(name .. " is not available on Game Boy") end
    emit(2, { 0xed, opc })
end end

do
    local z80_reti = M.reti
    M.reti = function()
        if M.gameboy then return emit(1, { 0xd9 }) end
        return z80_reti()
    end
end

function M.ldi(a, b)
    a, b = eval(a), eval(b)
    if not M.gameboy then return emit(2, { 0xed, 0xa0 }) end
    if is_mem(a) and a.value == "hl" and b == "a" then return emit(1, { 0x22 }) end
    if a == "a" and is_mem(b) and b.value == "hl" then return emit(1, { 0x2a }) end
    die("unsupported ldi")
end

function M.ldd(a, b)
    a, b = eval(a), eval(b)
    if not M.gameboy then return emit(2, { 0xed, 0xa8 }) end
    if is_mem(a) and a.value == "hl" and b == "a" then return emit(1, { 0x32 }) end
    if a == "a" and is_mem(b) and b.value == "hl" then return emit(1, { 0x3a }) end
    die("unsupported ldd")
end

function M.ldh(a, b)
    a, b = eval(a), eval(b)
    if not M.gameboy then die("ldh is only available in Game Boy mode") end
    local high_byte = function(v)
        v = word(v)
        if v >= 0xff00 and v <= 0xffff then return v & 0xff end
        return M.byte_normalize(v)
    end
    if is_mem(a) and a.value == "c" and b == "a" then return emit(1, { 0xe2 }) end
    if a == "a" and is_mem(b) and b.value == "c" then return emit(1, { 0xf2 }) end
    if is_mem(a) and b == "a" then return emit(2, function() return { 0xe0, high_byte(a.value) } end) end
    if a == "a" and is_mem(b) then return emit(2, function() return { 0xf0, high_byte(b.value) } end) end
    die("unsupported ldh")
end

return M
