local macro = require "macro"

function addressing(get)
    local t,v = get:next()
end

macro.define('lda', addressing)
