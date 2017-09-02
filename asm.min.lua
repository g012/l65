dofile "vcs.lua"
TIM_OVERSCAN = 50
TIM_VBLANK = 61
TIM_KERNEL = 17
location(0xf000, 0xffff)
section "waitForIntim"
ldx_immediate(0xf0)
lda_immediate(0xac)
lda_immediate(INTIM)
lda_absolute(0xbeef)
lda_absolute(INTIM)
lda_absolute_x(INTIM)
lda_absolute_x(INTIM)
lda_indirect_x(INTIM)
lda_indirect_y(INTIM)
asl_implied()
asl_absolute(INTIM)
asl_implied()
bne_relative("test")
bne_relative("waitForIntim")
bne_relative(f())
bne_relative("_toto")
rts_implied()

