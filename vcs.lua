-- set cpu to 6502
cpu = require "6502"
setmetatable(_ENV, cpu)

vcs = {
    -- TIA write only
    VSYNC   = 0x00, -- ......1.  vertical sync set-clear
    VBLANK  = 0x01, -- 11....1.  vertical blank set-clear
    WSYNC   = 0x02, -- <strobe>  wait for leading edge of horizontal blank
    RSYNC   = 0x03, -- <strobe>  reset horizontal sync counter
    NUSIZ0  = 0x04, -- ..111111  number-size player-missile 0
    NUSIZ1  = 0x05, -- ..111111  number-size player-missile 1
    COLUP0  = 0x06, -- 1111111.  color-lum player 0 and missile 0
    COLUP1  = 0x07, -- 1111111.  color-lum player 1 and missile 1
    COLUPF  = 0x08, -- 1111111.  color-lum playfield and ball
    COLUBK  = 0x09, -- 1111111.  color-lum background
    CTRLPF  = 0x0a, -- ..11.111  control playfield ball size & collisions
    REFP0   = 0x0b, -- ....1...  reflect player 0
    REFP1   = 0x0c, -- ....1...  reflect player 1
    PF0     = 0x0d, -- 1111....  playfield register byte 0
    PF1     = 0x0e, -- 11111111  playfield register byte 1
    PF2     = 0x0f, -- 11111111  playfield register byte 2
    RESP0   = 0x10, -- <strobe>  reset player 0
    RESP1   = 0x11, -- <strobe>  reset player 1
    RESM0   = 0x12, -- <strobe>  reset missile 0
    RESM1   = 0x13, -- <strobe>  reset missile 1
    RESBL   = 0x14, -- <strobe>  reset ball
    AUDC0   = 0x15, -- ....1111  audio control 0
    AUDC1   = 0x16, -- ....1111  audio control 1
    AUDF0   = 0x17, -- ...11111  audio frequency 0
    AUDF1   = 0x18, -- ...11111  audio frequency 1
    AUDV0   = 0x19, -- ....1111  audio volume 0
    AUDV1   = 0x1a, -- ....1111  audio volume 1
    GRP0    = 0x1b, -- 11111111  graphics player 0
    GRP1    = 0x1c, -- 11111111  graphics player 1
    ENAM0   = 0x1d, -- ......1.  graphics (enable) missile 0
    ENAM1   = 0x1e, -- ......1.  graphics (enable) missile 1
    ENABL   = 0x1f, -- ......1.  graphics (enable) ball
    HMP0    = 0x20, -- 1111....  horizontal motion player 0
    HMP1    = 0x21, -- 1111....  horizontal motion player 1
    HMM0    = 0x22, -- 1111....  horizontal motion missile 0
    HMM1    = 0x23, -- 1111....  horizontal motion missile 1
    HMBL    = 0x24, -- 1111....  horizontal motion ball
    VDELP0  = 0x25, -- .......1  vertical delay player 0
    VDELP1  = 0x26, -- .......1  vertical delay player 1
    VDELBL  = 0x27, -- .......1  vertical delay ball
    RESMP0  = 0x28, -- ......1.  reset missile 0 to player 0
    RESMP1  = 0x29, -- ......1.  reset missile 1 to player 1
    HMOVE   = 0x2a, -- <strobe>  apply horizontal motion
    HMCLR   = 0x2b, -- <strobe>  clear horizontal motion registers
    CXCLR   = 0x2c, -- <strobe>  clear collision latches= 0x20

    -- TIA read only
    CXM0P   = 0x30, -- 11......  read collision M0-P1, M0-P0 (Bit 7 --6)
    CXM1P   = 0x31, -- 11......  read collision M1-P0, M1-P1
    CXP0FB  = 0x32, -- 11......  read collision P0-PF, P0-BL
    CXP1FB  = 0x33, -- 11......  read collision P1-PF, P1-BL
    CXM0FB  = 0x34, -- 11......  read collision M0-PF, M0-BL
    CXM1FB  = 0x35, -- 11......  read collision M1-PF, M1-BL
    CXBLPF  = 0x36, -- 1.......  read collision BL-PF, unused
    CXPPMM  = 0x37, -- 11......  read collision P0-P1, M0-M1
    INPT0   = 0x38, -- 1.......  read pot port
    INPT1   = 0x39, -- 1.......  read pot port
    INPT2   = 0x3a, -- 1.......  read pot port
    INPT3   = 0x3b, -- 1.......  read pot port
    INPT4   = 0x3c, -- 1.......  read input
    INPT5   = 0x3d, -- 1.......  read input

    -- PIA
    SWCHA   = 0x280, -- 11111111  Port A= 0x280 -- input or output  (read or write)
    SWACNT  = 0x281, -- 11111111  Port A DDR, 0= input, 1=output
    SWCHB   = 0x282, -- 11111111  Port B= 0x280 -- console switches (read only)
    SWBCNT  = 0x283, -- 11111111  Port B DDR (hardwired as input)
    INTIM   = 0x284, -- 11111111  Timer output (read only)
    INSTAT  = 0x285, -- 11......  Timer Status (read only, undocumented)

    TIM1T   = 0x294, -- 11111111  set 1 clock interval (838 nsec/interval)
    TIM8T   = 0x295, -- 11111111  set 8 clock interval (6.7 usec/interval)
    TIM64T  = 0x296, -- 11111111  set 64 clock interval (53.6 usec/interval)
    T1024T  = 0x297, -- 11111111  set 1024 clock interval (858.2 usec/interval)
}
do
    local symbols = cpu.symbols
    for k,v in pairs(vcs) do symbols[k] = v end
end

--[[ TODO enable this when lua load() is changed

mappers = {}

mappers['2K'] = function()
    rom0 = location(0xf800, 0xffff)
    section{"vectors", org=0xfffc} word(main,main)
end
mappers.CV = mappers['2K']

mappers['4K'] = function()
    rom0 = location(0xf000, 0xffff)
    section{"vectors", org=0xfffc} word(main,main)
end

local bank_stubs = function(count, hotspot)
    function switchrom(i) assert(i>=0 and i<count) bit 0x1000+hotspot+i end
    local base = 0x10000 - (count << 12)
    for bi=0,count-1 do
        local o = base+(bi<<12)
        _ENV['rom' .. bi] = location{o, o+0xfff, rorg=0xf000}
        local start=section{"entry"..bi, org=o+hotspot-6} switchrom(0) if bi==0 then jmp main end
        section{"switchtab"..bi, org=o+hotspot} for i=1,count do byte(0) end
        section{"vectors"..bi, org=o+0xffc} word(start,start)
    end
end
mappers.FA = function() bank_stubs(3, 0xff8) end
mappers.F8 = function() bank_stubs(2, 0xff8) end
mappers.F6 = function() bank_stubs(4, 0xff6) end
mappers.F4 = function() bank_stubs(8, 0xff4) end

mappers.FE = function()
    rom0 = location(0xd000, 0xdfff)
    section{"vectors0", org=0xdffc} word(main,main)
    location{0xe000, 0xefff, nofill=true}
    rom1 = location(0xf000, 0xffff)
    section{"vectors1", org=0xfffc} word(main,main)
end

mappers.E0 = function(map)
    function switchrom0(i) assert(map[i]==0) bit 0x1fe0+i end
    function switchrom1(i) assert(map[i]==1) bit 0x1fe8+i end
    function switchrom2(i) assert(map[i]==2) bit 0x1ff0+i end
    map[7]=3
    for bi=0,7 do
        local o = bi*0x400
        _ENV["rom"..bi] = location{0xe000+o, 0xe3ff+o, rorg=0xf000+map[bi]*0x400}
    end
    section{"switchtab", org=0xffe0} for i=1,8*3 do byte(0) end
    section{"vectors", org=0xfffc} word(main,main)
end

-- rom0 refers to the last one in ROM, ie the one always mapped to 0xF800-0xFFFF,
-- such that changing the rom count does not change its index
mappers['3F'] = function(count)
    function switchrom(i) assert(i>=0 and i<count-1) lda#i sta 0x3f end
    local symbols=cpu.symbols for k,v in pairs(vcs) do -- remap TIA to 0x40-0x7F
        if v<0x40 then v=v+0x40 vcs[k]=v symbols[k]=v end
    end
    bi=0,count-2 do
        local o = 0x800*bi + 0x10000
        _ENV["rom"..(count-1-bi)] = location{o, 0x7ff+o, rorg=0xf000}
    end
    local o = 0x800 * (count-1) + 0x10000
    rom0 = location{o, 0x7ff+o, rorg=0xf800}
    section{"vectors", org=0xfffc} word(main,main)
end

mappers.E7 = function()
    function switchrom(i) assert(i>=0 and i<7) bit 0x1fe0+i end
    function enableram() bit 0x1fe7 end
    function switchram(i) assert(i>=0 and i<4) bit 0x1fe8+i end
    for bi=0,6 do
        local o = bi*0x800
        _ENV['rom' .. bi] = location{o+0xc000, o+0xc7ff, rorg=0xf000}
    end
    rom7 = location{0xf800, 0xffff, rorg=0xf800}
    section{"switchtab", org=0xffe0} for i=1,12 do byte(0) end
    section{"vectors", org=0xfffc} word(main,main)
end

mappers.F0 = function()
    function switchrom() lda 0x1ff0 end
    for bi=0,15 do
        local o = bi*0x1000 + 0x1000
        _ENV['rom' .. bi] = location{o, o+0xfff, rorg=0xf000}
        local start=section{"entry"..bi}
            @_loop switchrom() bne _loop
            if bi==0 then jmp main end
        section{"switchtab"..bi, org=o+0xff0} byte(0)
        section{"vectors"..bi, org=o+0xffc} word(start,start)
    end
end

local function bank_stubs2 = function(hotspot0, hotspot1)
    function switchrom(i)
        if i==0 then bit hotspot0
        elseif i==1 then bit hotspot1
        else error("invalid rom index: " .. i) end
    end
    local base = 0xe000
    for bi=0,1 do
        local o = base+(bi<<12)
        _ENV['rom' .. bi] = location{o, o+0xfff, rorg=0xf000}
        local start=section{"entry"..bi, org=o+0xffc-6} switchrom(0) if bi==0 then jmp main end
        section{"vectors"..bi, org=o+0xffc} word(start,start)
    end
end
mappers.UA = function() bank_stubs2(0x220, 0x240) end
mappers.['0840'] = function() bank_stubs2(0x800, 0x840) end

mappers['3E'] = function(rom_count, ram_count)
    mappers['3F'](rom_count)
    function switchram(i) assert(i>=0 and i<ram_count-1) lda#i sta 0x3E end
end

]]
