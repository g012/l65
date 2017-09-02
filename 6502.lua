adressing = {
    imm=0x09, zp=0x05, zpx=0x15, ab=0x0d, abx=0x1d, aby=0x19, inx=0x01, iny=0x11,
    acc=0x09,
}
encode = {
    adc=0x60,
    ['and']=0x20,
    asl=0x01,
    lda=0xa0,
    sta=0x80,
    --lda = { imm=0xa9, zp=0xa5, zpx=0xb5, ab=0xad, abx=0xbd, aby=0xb9, inx=0xa1, iny=0xb1 },
}


--[=[
adressages:
    lda #10     lda(10)                                                   lda"#10"
    lda 10      lda{10}                                                   lda"10"
    lda 10,x    lda{10,"x"}      ldax{10}      lda{x=10}                  lda"10,x"
    lda $1000   lda{0x1000}
    lda $1000,x lda{0x1000,"x"}  ldax{0x1000}
    lda $1000,y lda{0x1000,"y"}  lday{0x1000}
    lda (10,x)  lda{10,"(x)"}    lda_x{10}     a=(10,x)  a=mem[zp[10+x]]  lda"(10,x)"
    lda (10),y  lda{10,"(y)"}    lda_y{10}     a=(10),y  a=mem[zp[10]+y]  lda"(10),y"

    sta 10      sta{10}
    sta 10,x    sta{10,"x"}
    sta $1000   sta{0x1000}
    sta $1000,x sta{0x1000,"x"}
    sta $1000,y sta{0x1000,"y"}
    sta (10,x)  sta{10,"(x)"}
    sta (10),y  sta{10,"(y)"}
]=]

local function setfenv(fn, env)
    local i = 1
    while true do
        local name = debug.getupvalue(fn, i)
        if name == "_ENV" then
            debug.upvaluejoin(fn, i, (function() return env end), 1)
            break
        elseif not name then break end
        i = i + 1
    end

    return fn
end

local function getfenv(fn)
    local i = 1
    while true do
        local name, val = debug.getupvalue(fn, i)
        if name == "_ENV" then return val
        elseif not name then break end
        i = i + 1
    end
end

asm6502_env = {
    lda = function(v)
        emit(0xa9, v)
    end,
}
function asm(f, ...)
    setfenv(f, asm6502_env)
    f(...)
end

a,x,y = 0,0,0
do
    local regs = { a="a", x="x", y="y" }
    setmetatable(_G, {
        __index = function(t,k,v)
            if regs[k] then assert(loadstring("ld" .. k .. "(" .. v .. ")"))() end
            return t[k]
        end,
        __newindex = function(t,k,v)
            if regs[k] then assert(loadstring("st" .. k .. "(" .. v .. ")"))() end
            t[k] = v
        end,
    })
end
