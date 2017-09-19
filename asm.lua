dofile "vcs.lua"

TIM_OVERSCAN    = 50 -- TIM64T,  3200 cycles = ~ 42 scanlines
TIM_VBLANK      = 61 -- TIM64T,  3904 cycles = ~ 51 scanlines
TIM_KERNEL      = 17 -- T1024T, 17408 cycles = ~229 scanlines

location(0x000, 0xfff)

if toto ~= 15 then end

abc = 13 ~ 0x7
xyz = 1 << 2

x={ f=function() end }
x:f()
::lualabel::

;
lda = 5 if lda < 6 then sta=7 end
;

local function ptr_table(label, ...)
    local vals = {...}
    section{label .. "_lo", align=16} byte_lo(vals)
    section{label .. "_hi", align=16} byte_hi(vals)
end

charset(" abcdefghijklmnopqrstuvwxyz-")
section("message2") byte(4, "test", 0)
charset()
section("message") byte(4, "test", 0)

section("data")
    do crosspage()
        byte(1, 2) byte(3, 4) endpage()
    end
    word(0xf080) byte(16, 3, 4, 5, 6,
        24, 32)
    word(message2)
    byte_lo(message) byte_hi(message)
    byte(function() return message&0xff end, function() return message>>8 end)

ptr_table("ptrs", function() return message end, data, 0)

--section{ "toto", align = 256, offset = 16 }
--section{ "toto", org = 0xf100 }
--section "waitForIntim"
section("waitForIntim") --alt short syntax when no other option
    -- n_{ a=INTIM } ?
    --lda(INTIM) -- or a=INTIM
    --bne "waitForIntim"
    ldximm (function(o) return o+(0xf0) end)
    ldximm (function(o) return o+(13) end)
    ldyimm (function(o) return o+(0xAB - 16 + 27 & 3 | 6 ~ 0xf >> ~3 << 1 // 5) end)
    ldximm (function(o) return o+(15) end,3)

    local kernel_cycles,kernel_size
    table.insert(section_current.instructions, { asbin=function() kernel_cycles=cycles kernel_size=size end })

    ldazab(function(o) return o+( data) end)
    ldazab(function(o) return o+( data) end,5)
    ldazax(function(o) return o+( data) end,5)
    ldaaby(function(o) return o+( data) end,5)
    ldazab(function(o) return o+( data+3) end,12)
    ldazax(function(o) return o+( data+3) end,12)
    ldaaby(function(o) return o+( data+3) end,12)
    ldainx (function(o) return o+(VBLANK) end,5)
    ldainx (function(a) return a+2 end,VBLANK)
    ldainy (function(o) return o+(VBLANK) end,5)
    ldainy (function(a) return a&3 end,VBLANK)
    jmpind (function(o) return o+(VBLANK) end)
    jmpind (function(o) return o+(VBLANK) end,12)
    jmpind (function(o) return o+(VBLANK-4) end)

    -- cycles are counted without taking any branch
    table.insert(section_current.instructions, { asbin=function() print('kernel cycles: ', cycles-kernel_cycles, 'kernel size: ', size-kernel_size) end })

    ldazab( function(c) return data * c end, v)
    ldazab( function(c) return data*c end, v)
    local f = function(c) return data*c end v=5 ldazab(f,v) v=12 ldazab(f,v)
    local g = function() return function(c) return data * c end end

    ldazab(g(),v)
    ldazab( f,v)
    ldazax (function(o) return o+(_toto+15) end,16)
    ldaimm (15)

    do samepage()
        ldaimm (function(o) return o+(0xac) end)
        ldaimm (function(o) return o+(VBLANK) end)
        ldazab(function(o) return o+( 0xbeef) end)
        ldazab(function(o) return o+( VBLANK) end)
        ldaabs(function(o) return o+( VBLANK) end)
        ldazax(function(o) return o+( VBLANK) end)
        ldaaby(function(o) return o+( VBLANK) end)
        ldainx (function(o) return o+(VBLANK) end)
        ldainy (function(o) return o+(VBLANK) end) endpage()
    end

    aslimp()
    aslzab(function(o) return o+( VBLANK) end)
    aslimp()
label("_toto")
    bnerel( "_toto")
    bnerel( "waitForIntim")
    f=function() return "_toto" end bnerel( f())
    bnerel( "_toto")

    jamimp() aslimp() lsrimp() ldximm (function(o) return o+(16) end) ldyzab(function(o) return o+( 0xf0f0) end)
    rtsimp()

writebin()
writesym()

--[[
section "doOverscan"
    sta{WSYNC} -- WSYNC=a
    lda(2) sta{VBLANK} -- a=2 VBLANK=a
    lda(TIM_OVERSCAN) sta{TIM64T} -- a=TIM_OVERSCAN TIM64T=a
    jsr "waitForIntim"

section "doVBlank"
    lda(0x0e)
  label ".vsyncLoop" 
    sta{WSYNC}
    sta{VSYNC}
    lsr()
    bne ".vsyncLoop"
    lda(2)
    sta{VBLANK}
    lda(TIM_VBLANK)
    sta{TIM64T}
    jsr "waitForIntim"

section "doKernel"
    lda(TIM_KERNEL)
    sta{T1024T}
    jsr "waitForIntim"

section "start"
    -- clear zeropage
    cld()
    ldx(0)
    txa()
  label ".clearLoop"
    dex()
    tsx()
    pha()
    bne ".clearLoop"
    -- main
  label "mainLoop"
    jsr "doOverscan"
    jsr "doVBlank"
    jsr "doKernel"
    jmp "mainLoop"

section{ name="vectors", org=0xfffc }
    word{ "start", "start" }

]]
