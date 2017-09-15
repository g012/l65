dofile "vcs.lua"

TIM_OVERSCAN    = 50 -- TIM64T,  3200 cycles = ~ 42 scanlines
TIM_VBLANK      = 61 -- TIM64T,  3904 cycles = ~ 51 scanlines
TIM_KERNEL      = 17 -- T1024T, 17408 cycles = ~229 scanlines
location(0xf000, 0xffff)

if toto ~= 15 then end

abc = 13 ~ 0x7
xyz = 1 << 2

x:f()
::lualabel::

;
lda = 5 if lda < 6 then print('yep') end
;

--[[
local function ptr_table(label, ...)
    local vals = {...}
    section(label .. "_lo")
    for _,v in ipairs(vals) do byte_lo(v) end
    section(label .. "_hi")
    for _,v in ipairs(vals) do byte_hi(v) end
end
]]

--@@message byte(4) "test" byte(0)

section(function(o) return o+("data") end)
    do crosspage()
        byte(1, 2) byte(3, 4) endpage()
    end
    word(0xf080) byte(16, 3, 4, 5, 6,
        24, 32)
        --[[
    word(message)
    byte_lo(message) byte_hi(message)
    --]]
    byte(function() return message&0xff end, function() return message>>8 end)
    --[[

ptr_table("ptrs", message, data, 0)
--]]
--section{ "toto", align = 256, offset = 16 }
--section{ "toto", org = 0xf100 }
--section "waitForIntim"
section(function(o) return o+("waitForIntim") end) --alt short syntax when no other option
    -- n_{ a=INTIM } ?
    --lda(INTIM) -- or a=INTIM
    --bne "waitForIntim"
    ldx_immediate (function(o) return o+(0xf0) end)
    ldx_immediate (function(o) return o+(13) end)
    ldy_immediate (function(o) return o+(0xAB - 16 + 27 & 3 | 6 ~ 0xf >> ~3 << 1 // 5) end)

    lda_absolute(function(o) return o+( data) end)
    lda_absolute(function(o) return o+( data) end,5)
    lda_absolute_x(function(o) return o+( data) end,5)
    lda_absolute_y(function(o) return o+( data) end,5)
    lda_absolute(function(o) return o+( data+3) end,12)
    lda_absolute_x(function(o) return o+( data+3) end,12)
    lda_absolute_y(function(o) return o+( data+3) end,12)
    lda_indirect_x (function(o) return o+(INTIM) end,5)
    lda_indirect_x (function(o) return o+(INTIM) end,function(a) return a+2 end)
    lda_indirect_y (function(o) return o+(INTIM) end,5)
    lda_indirect_y (function(o) return o+(INTIM) end,function(a) return a+2 end)
    jmp_indirect (function(o) return o+(INTIM) end)
    jmp_indirect (function(o) return o+(INTIM) end,12)
    jmp_indirect (function(o) return o+(INTIM) end,function(a) return a-4 end)

    lda_absolute( function(c) return data * c end, v)
    lda_absolute( function(c) return data*c end, v)
    local f = function(c) return data*c end v=5 lda_absolute(f,v) v=12 lda_absolute(f,v)
    local g = function() return function(c) return data * c end end

    lda_absolute(g(),v)
    lda_absolute( f,v)
    lda_absolute_x (function(o) return o+(_toto+15) end,16)
    lda_immediate (15)

    do samepage()
        lda_immediate (function(o) return o+(0xac) end)
        lda_immediate (function(o) return o+(INTIM) end)
        lda_absolute(function(o) return o+( 0xbeef) end)
        lda_absolute(function(o) return o+( INTIM) end)
        lda_absolute_nozp(function(o) return o+( INTIM) end)
        lda_absolute_x(function(o) return o+( INTIM) end)
        lda_absolute_y(function(o) return o+( INTIM) end)
        lda_indirect_x (function(o) return o+(INTIM) end)
        lda_indirect_y (function(o) return o+(INTIM) end) endpage()
    end

    asl_implied()
    asl_absolute(function(o) return o+( INTIM) end)
    asl_implied()
label(function(o) return o+("_toto") end)
    bne_relative(function(o) return o+( "test") end)
    bne_relative(function(o) return o+( "waitForIntim") end)
    bne_relative(function(o) return o+( f()) end)
    bne_relative(function(o) return o+( "_toto") end)

    jam_implied() asl_implied() lsr_implied() ldx_immediate (function(o) return o+(16) end) ldy_absolute(function(o) return o+( 0xf0f0) end)

    rts_implied()

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
