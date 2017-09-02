--[[
  6502 assembler macros.
]]

local M = require 'macro'

local val = function(s)
end

local op_imm = {}

local op = {
    lda = function(get,put)
        get:expecting 'space'
        local t,v = get:next()
        if t ~= '#' then return nil,true end
        t,v = get:block(nil, {['\n']=true,['--']=true})
        return put 'lda.imm(' :tokenlist t ')' :token v
        --v = get:upto '\n' --'[;\n(--)]'
        --return ('lda.imm(%s)\n'):format(v)
    end,
}

for k,v in pairs(op) do M.define(k,v) end
