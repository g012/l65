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

#pragma syntax6502 off
lda = 5 if lda < 6 then print('yep') end
#pragma syntax6502 on

--@@data samepage byte(1, 2) crosspage byte(3, 4)

--section{ "toto", align = 256, offset = 16 }
--section{ "toto", org = 0xf100 }
section "waitForIntim" --alt short syntax when no other option: @@waitForIntim ?
    -- n_{ a=INTIM } ?
    --lda(INTIM) -- or a=INTIM
    --bne "waitForIntim"
    ldx #0xf0
    ldx #0b1101
    ldy #0xAB - 16 + 0b11011 & 3 | 6 ~ 0xf >> ~3 << 1 // 5

    lda #0xac
    lda #INTIM
    lda 0xbeef
    lda INTIM
    lda.w INTIM
    lda INTIM,x
    lda INTIM,y
    lda (INTIM,x)
    lda (INTIM),y

    asl
    asl INTIM
    asl
@.toto
    bne "test"
    bne waitForIntim
    bne f()
    bne .toto

    jam asl lsr ldx #16 ldy 0xf0f0

    rts

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
