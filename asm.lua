dofile "vcs.lua"

TIM_OVERSCAN    = 50 -- TIM64T,  3200 cycles = ~ 42 scanlines
TIM_VBLANK      = 61 -- TIM64T,  3904 cycles = ~ 51 scanlines
TIM_KERNEL      = 17 -- T1024T, 17408 cycles = ~229 scanlines
location(0xf000, 0xffff)

section "waitForIntim"
    -- n_{ a=INTIM } ?
    --lda(INTIM) -- or a=INTIM
    --bne "waitForIntim"
    ldx #0xf0

    lda #0xac
    lda #INTIM
    lda 0xbeef
    lda INTIM
    lda INTIM,x
    lda INTIM,y
    lda (INTIM,x)
    lda (INTIM),y

    asl
    asl INTIM
    asl

    bne "test"
    bne waitForIntim
    bne f()
    bne _toto

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
