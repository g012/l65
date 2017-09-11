local g = function(a) return a*3 end
local f = function(a) return a*3 end

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

section("data")
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
section("waitForIntim") --alt short syntax when no other option
    -- n_{ a=INTIM } ?
    --lda(INTIM) -- or a=INTIM
    --bne "waitForIntim"
    ldx_immediate(0xf0)
    ldx_immediate(13)
    ldy_immediate(0xAB - 16 + 27 & 3 | 6 ~ 0xf >> ~3 << 1 // 5)
    --[[
    lda data
    lda data,5
    lda data,function(final_address) return final_address & 3 end
    lda data,\a(a&3)
    lda (INTIM,5,x)
    lda (INTIM,5),y
    -- parse value list, si #list > 1 && list[#list-1] == 'x' ...
    ]]

    do samepage()
        lda_immediate(0xac)
        lda_immediate(INTIM)
        lda_absolute( 0xbeef)
        lda_absolute( INTIM)
        lda_absolute_nozp( INTIM)
        lda_absolute_x( INTIM)
        lda_absolute_x( INTIM)
        lda_indirect_x(INTIM)
        lda_indirect_y(INTIM) endpage()
    end

    asl_implied()
    asl_absolute( INTIM)
    asl_implied()
label_local("toto")
    bne_relative( "test")
    bne_relative( "waitForIntim")
    bne_relative( f())
    bne_relative_local("toto")

    jam_implied() asl_implied() lsr_implied() ldx_immediate(16) ldy_absolute( 0xf0f0)

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
