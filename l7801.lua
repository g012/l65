#!/usr/bin/env lua

local function lookupify(tb, dst, not_val)
    if not dst then dst = tb end
    local val = not not_val
    for _, v in pairs(tb) do
        dst[v] = val
    end
    return tb
end

local WhiteChars = lookupify{' ', '\n', '\t', '\r'}
local Spaces = lookupify{' ', '\t'}
local EscapeLookup = {['\r'] = '\\r', ['\n'] = '\\n', ['\t'] = '\\t', ['"'] = '\\"', ["'"] = "\\'"}
local LowerChars = lookupify{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
                             'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
                             's', 't', 'u', 'v', 'w', 'x', 'y', 'z'}
local UpperChars = lookupify{'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
                             'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
                             'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'}
local Digits = lookupify{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'}
local HexDigits = lookupify{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                            'A', 'a', 'B', 'b', 'C', 'c', 'D', 'd', 'E', 'e', 'F', 'f'}
local BinDigits = lookupify{'0', '1'}

local Symbols = lookupify{'+', '-', '*', '/', '^', '%', ',', '{', '}', '[', ']', '(', ')', ';', '#', '&', '|', '!'}

local Keywords = lookupify{
    'and', 'break', 'do', 'else', 'elseif',
    'end', 'false', 'for', 'function', 'goto', 'if',
    'in', 'local', 'nil', 'not', 'or', 'repeat',
    'return', 'then', 'true', 'until', 'while',
};

local Keywords_control = {
}
local Keywords_data = {
    'dc',
}
local Keywords_7801 = {
    'aci','adi','adinc','ani',
    'block','calb','calf','call','calt','clc','ei','eqi','daa','di','dcr','dcx',
    'ex','exx','gti','halt','inr','inx','jb','jmp','jr','lti','lxi','mvi','nei','nop',
    'offi','oni','ori','pen','per','pex','ret','reti','rets','rld','rrd','sio','softi','stc','stm',
    'sbi','sui','suinb','table','xri',
}

local Registers_7801 = {
    a=8,b=8,c=8,d=8,e=8,h=8,l=8,v=8,
    bc=16,de=16,hl=16,sp=16
}

local function syntax7801(on)
    syntax7801_on = on
    lookupify(Keywords_control, Keywords, not on)
    lookupify(Keywords_data, Keywords, not on)
    lookupify(Keywords_7801, Keywords, not on)
end
syntax7801(true)

local opcode_arg_encapsulate_on
local function opcode_arg_encapsulate(on)
    opcode_arg_encapsulate_on = not opcode_arg_encapsulate_on
end
opcode_arg_encapsulate(true)

local opcode_encapsulate = {} -- additionnal opcode, to have basic encapsulation (function(a) return a end)
local opcode_alias = {} -- alternate user names for opcodes
local opcode_implied = lookupify{
    'block','calb','clc','ei','daa','dcr','di','ex','exx','halt','inr','jb','nop','pen',
    'per','pex','ret','reti','rets','rld','rrd','sio','softi','stc','stm','table'
}

local opcode_immediate = lookupify{
    'calf', 'calt','call','jmp'
}
local opcode_relative = lookupify{
    'jr',
}
local opcode_reg = lookupify{
    'dcr','dcx','inr','inx'
}
local opcode_regb = lookupify{
    'aci','adi','adinc','ani','eqi','gti','lti','mvi','nei','offi','oni','ori','sbi','sui','suinb','xri',
}
local opcode_regw = lookupify{
    'lxi'
}
local opcode_reg_list = {
    a = lookupify{'aci','adi','adinc','ani','dcr','inr','eqi','gti','lti','mvi','nei','offi','oni','ori','sbi','sui','suinb','xri'},
    b = lookupify{'dcr','inr','mvi'},
    c = lookupify{'dcr','inr','mvi'},
    d = lookupify{'mvi'},
    e = lookupify{'mvi'},
    h = lookupify{'mvi'},
    l = lookupify{'mvi'},
    v = lookupify{'mvi'},
    bc = lookupify{'lxi'},
    de = lookupify{'lxi'},
    hl = lookupify{'lxi'},
    sp = lookupify{'dcx','inx','lxi'},
}

local addressing_map = {
    imp = opcode_implied,
    imm = opcode_immediate,
    rel = opcode_relative,
    reg = opcode_reg,
    regb = opcode_regb,
    regw = opcode_regw,
}

local Scope = {
    new = function(self, parent)
        local s = {
            Parent = parent,
            Locals = { },
            Globals = { },
            Children = { },
        }
        
        if parent then
            table.insert(parent.Children, s)
        end
        
        return setmetatable(s, { __index = self })
    end,
    
    AddLocal = function(self, v)
        table.insert(self.Locals, v)
    end,
    
    AddGlobal = function(self, v)
        table.insert(self.Globals, v)
    end,
    
    CreateLocal = function(self, name)
        local v
        v = self:GetLocal(name)
        if v then return v end
        v = { }
        v.Scope = self
        v.Name = name
        v.IsGlobal = false
        self:AddLocal(v)
        return v
    end,
    
    GetLocal = function(self, name)
        for k, var in pairs(self.Locals) do
            if var.Name == name then return var end
        end
        
        if self.Parent then
            return self.Parent:GetLocal(name)
        end
    end,
    
    CreateGlobal = function(self, name)
        local v
        v = self:GetGlobal(name)
        if v then return v end
        v = { }
        v.Scope = self
        v.Name = name
        v.IsGlobal = true
        self:AddGlobal(v)
        return v
    end, 
    
    GetGlobal = function(self, name)
        for k, v in pairs(self.Globals) do
            if v.Name == name then return v end
        end
        
        if self.Parent then
            return self.Parent:GetGlobal(name)
        end
    end,
}

local function LexLua(src)
    --token dump
    local tokens = {}

    local st, err = pcall(function()
        --line / char / pointer tracking
        local p = 1
        local line = 1
        local char = 1

        --get / peek functions
        local function get()
            local c = src:sub(p,p)
            if c == '\n' then
                char = 1
                line = line + 1
            else
                char = char + 1
            end
            p = p + 1
            return c
        end
        local function get_n(count) for i=1,count do get() end end
        local function peek(n)
            n = n or 0
            return src:sub(p+n,p+n)
        end
        local function peek_n(sz) return src:sub(p, p+sz-1) end
        local function consume(chars)
            local c = peek()
            for i = 1, #chars do
                if c == chars:sub(i,i) then return get() end
            end
        end
        local function get_word(spaces)
            if not spaces then spaces = Spaces end
            local c
            repeat get() c = peek() until not spaces[c]
            local start = p
            repeat get() c = peek() until not (UpperChars[c] or LowerChars[c] or Digits[c] or c == '_')
            return src:sub(start, p-1)
        end
        local function peek_ident(i, spaces)
            if not spaces then spaces = Spaces end
            local c
            while true do c=peek(i) if not spaces[c] then break end i=i+1 end
            if not (UpperChars[c] or LowerChars[c] or c == '_') then return end
            local start = p+i
            repeat i=i+1 c = peek(i) until not (UpperChars[c] or LowerChars[c] or Digits[c] or c == '_')
            return src:sub(start, p+i-1),i,c
        end

        --shared stuff
        local function generateError(err)
            return error(">> :"..line..":"..char..": "..err, 0)
        end

        local function tryGetLongString()
            local start = p
            if peek() == '[' then
                local equalsCount = 0
                local depth = 1
                while peek(equalsCount+1) == '=' do
                    equalsCount = equalsCount + 1
                end
                if peek(equalsCount+1) == '[' then
                    --start parsing the string. Strip the starting bit
                    for _ = 0, equalsCount+1 do get() end

                    --get the contents
                    local contentStart = p
                    while true do
                        --check for eof
                        if peek() == '' then
                            generateError("Expected ']"..string.rep('=', equalsCount).."]' near <eof>.", 3)
                        end

                        --check for the end
                        local foundEnd = true
                        if peek() == ']' then
                            for i = 1, equalsCount do
                                if peek(i) ~= '=' then foundEnd = false end
                            end
                            if peek(equalsCount+1) ~= ']' then
                                foundEnd = false
                            end
                        else
                            --[[
                            if peek() == '[' then
                                -- is there an embedded long string?
                                local embedded = true
                                for i = 1, equalsCount do
                                    if peek(i) ~= '=' then
                                        embedded = false
                                        break
                                    end
                                end
                                if peek(equalsCount + 1) == '[' and embedded then
                                    -- oh look, there was
                                    depth = depth + 1
                                    for i = 1, (equalsCount + 2) do
                                        get()
                                    end
                                end
                            end
                            ]]
                            foundEnd = false
                        end
                        --
                        if foundEnd then
                            depth = depth - 1
                            if depth == 0 then
                                break
                            else
                                for i = 1, equalsCount + 2 do
                                    get()
                                end
                            end
                        else
                            get()
                        end
                    end

                    --get the interior string
                    local contentString = src:sub(contentStart, p-1)

                    --found the end. Get rid of the trailing bit
                    for i = 0, equalsCount+1 do get() end

                    --get the exterior string
                    local longString = src:sub(start, p-1)

                    --return the stuff
                    return contentString, longString
                else
                    return nil
                end
            else
                return nil
            end
        end

        --main token emitting loop
        while true do
            --get leading whitespace. The leading whitespace will include any comments
            --preceding the token. This prevents the parser needing to deal with comments
            --separately.
            local leading = { }
            local leadingWhite = ''
            local longStr = false
            while true do
                local c = peek()
                if c == '#' and peek(1) == '!' and line == 1 then
                    -- #! shebang for linux scripts
                    get()
                    get()
                    leadingWhite = "#!"
                    while peek() ~= '\n' and peek() ~= '' do
                        leadingWhite = leadingWhite .. get()
                    end
                    local token = {
                        Type = 'Comment',
                        CommentType = 'Shebang',
                        Data = leadingWhite,
                        Line = line,
                        Char = char
                    }
                    token.Print = function()
                        return "<"..(token.Type .. string.rep(' ', 7-#token.Type)).."  "..(token.Data or '').." >"
                    end
                    leadingWhite = ""
                    table.insert(leading, token)
                end
                if c == ' ' or c == '\t' then
                    --whitespace
                    --leadingWhite = leadingWhite..get()
                    local c2 = get() -- ignore whitespace
                    table.insert(leading, { Type = 'Whitespace', Line = line, Char = char, Data = c2 })
                elseif c == '\n' or c == '\r' then
                    local nl = get()
                    if leadingWhite ~= "" then
                        local token = {
                            Type = 'Comment',
                            CommentType = longStr and 'LongComment' or 'Comment',
                            Data = leadingWhite,
                            Line = line,
                            Char = char,
                        }
                        token.Print = function()
                            return "<"..(token.Type .. string.rep(' ', 7-#token.Type)).."  "..(token.Data or '').." >"
                        end
                        table.insert(leading, token)
                        leadingWhite = ""
                    end
                    table.insert(leading, { Type = 'Whitespace', Line = line, Char = char, Data = nl })
                elseif c == '-' and peek(1) == '-' then
                    --comment
                    get()
                    get()
                    leadingWhite = leadingWhite .. '--'
                    local _, wholeText = tryGetLongString()
                    if wholeText then
                        leadingWhite = leadingWhite..wholeText
                        longStr = true
                    else
                        while peek() ~= '\n' and peek() ~= '' do
                            leadingWhite = leadingWhite..get()
                        end
                    end
                else
                    break
                end
            end
            if leadingWhite ~= "" then
                local token = {
                    Type = 'Comment',
                    CommentType = longStr and 'LongComment' or 'Comment',
                    Data = leadingWhite,
                    Line = line,
                    Char = char,
                }
                token.Print = function()
                    return "<"..(token.Type .. string.rep(' ', 7-#token.Type)).."  "..(token.Data or '').." >"
                end
                table.insert(leading, token)
            end

            --get the initial char
            local thisLine = line
            local thisChar = char
            local errorAt = ":"..line..":"..char..":> "
            local c = peek()

            --symbol to emit
            local toEmit = nil

            --pragma
            if char == 1 and peek_n(7) == '#pragma' then
                get_n(7)
                local dat,opt = get_word()
                local onoff = function(f, noerr)
                    opt = get_word()
                    if opt == 'on' then f(true)
                    elseif opt == 'off' then f(false)
                    elseif not noerr then generateError("invalid option for pragma " .. dat .. ", expected: [on,off]")
                    end
                    return opt
                end
                if dat == 'encapsulate' then
                    local opcode = onoff(function() end, true)
                    if opcode then
                        opcode_encapsulate[opcode] = get_word()
                        table.insert(Keywords_7801, opcode)
                        Keywords[opcode] = syntax7801_on
                        toEmit = {Type = 'Symbol', Data = ';'}
                    else
                        toEmit = {Type = 'Keyword', Data = 'encapsulate_' .. opt}
                    end
                elseif dat == 'add_opcode' then
                    local opcode,addressing = get_word(),get_word()
                    local map = addressing_map[addressing]
                    if not map then generateError("invalid addressing for pragma add_opcode: " .. addressing .. " (opcode: " .. opcode .. ")") end
                    map[opcode] = true
                    table.insert(Keywords_7801, opcode)
                    Keywords[opcode] = syntax7801_on
                    toEmit = {Type = 'Symbol', Data = ';'}
                elseif dat == 'alias' then
                    local org,new = get_word(),get_word()
                    opcode_alias[new] = org
                    table.insert(Keywords_7801, new)
                    Keywords[new] = syntax7801_on
                    toEmit = {Type = 'Symbol', Data = ';'}
                else generateError("unknown pragma: " .. dat)
                end

            --branch on type
            elseif c == '' then
                --eof
                toEmit = { Type = 'Eof' }

            elseif UpperChars[c] or LowerChars[c] or c == '_' then
                --ident or keyword
                local start = p
                repeat
                    get()
                    c = peek()
                until not (UpperChars[c] or LowerChars[c] or Digits[c] or c == '_')
                local dat = src:sub(start, p-1)
                if Keywords[dat] then
                    toEmit = {Type = 'Keyword', Data = dat}
                else
                    toEmit = {Type = 'Ident', Data = dat}
                end

            elseif Digits[c] or (peek() == '.' and Digits[peek(1)]) then
                --number const
                local start = p
                local data_override
                if c == '0' and peek(1) == 'x' then
                    get();get()
                    while HexDigits[peek()] do get() end
                    if consume('Pp') then
                        consume('+-')
                        while Digits[peek()] do get() end
                    end
                elseif c == '0' and peek(1) == 'b' then
                    get();get()
                    while BinDigits[peek()] do get() end
                    local pstart, val_s, val = p, src:sub(start+2, p-1), 0
                    for i=1,#val_s do
                        if val_s:sub(i,i) == '1' then val = val + (1<<(#val_s-i)) end
                    end
                    if consume('Pp') then
                        consume('+-')
                        while Digits[peek()] do get() end
                    end
                    data_override = val .. src:sub(pstart, p-1)
                else
                    while Digits[peek()] do get() end
                    if consume('.') then
                        while Digits[peek()] do get() end
                    end
                    if consume('Ee') then
                        consume('+-')
                        while Digits[peek()] do get() end
                    end
                end
                toEmit = {Type = 'Number', Data = data_override or src:sub(start, p-1)}

            elseif c == '\'' or c == '\"' then
                local start = p
                --string const
                local delim = get()
                local contentStart = p
                while true do
                    local c = get()
                    if c == '\\' then
                        get() --get the escape char
                    elseif c == delim then
                        break
                    elseif c == '' then
                        generateError("Unfinished string near <eof>")
                    end
                end
                local content = src:sub(contentStart, p-2)
                local constant = src:sub(start, p-1)
                toEmit = {Type = 'String', Data = constant, Constant = content}

            elseif c == '[' then
                local content, wholetext = tryGetLongString()
                if wholetext then
                    toEmit = {Type = 'String', Data = wholetext, Constant = content}
                else
                    get()
                    toEmit = {Type = 'Symbol', Data = '['}
                end

            elseif c == '<' and peek(1) == c then
                get() get()
                toEmit = {Type = 'Symbol', Data = '<<'}
            elseif c == '>' and peek(1) == c then
                get() get()
                toEmit = {Type = 'Symbol', Data = '>>'}

            elseif consume('>=<') then
                toEmit = {Type = 'Symbol', Data = consume('=') and c..'=' or c}

            elseif consume('~') then
                toEmit = {Type = 'Symbol', Data = consume('=') and '~=' or '~'}

            elseif consume('.') then
                if consume('.') then
                    if consume('.') then
                        toEmit = {Type = 'Symbol', Data = '...'}
                    else
                        toEmit = {Type = 'Symbol', Data = '..'}
                    end
                else
                    toEmit = {Type = 'Symbol', Data = '.'}
                end

            elseif consume(':') then
                toEmit = {Type = 'Symbol', Data = consume(':') and '::' or ':'}
            elseif consume('/') then
                toEmit = {Type = 'Symbol', Data = consume('/') and '//' or '/'}

            elseif syntax7801_on and consume('@') then
                if consume('@') then
                    toEmit = {Type = 'Symbol', Data = '@@'}
                else
                    toEmit = {Type = 'Symbol', Data = '@'}
                end

            elseif syntax7801_on and consume('\\') then
                toEmit = {Type = 'Symbol', Data = '\\'}

            elseif Symbols[c] then
                get()
                toEmit = {Type = 'Symbol', Data = c}

            else
                local contents, all = tryGetLongString()
                if contents then
                    toEmit = {Type = 'String', Data = all, Constant = contents}
                else
                    generateError("Unexpected Symbol '"..c.."' in source.", 2)
                end
            end

            if toEmit[1] then
                toEmit[1].LeadingWhite = leading
                for k,v in ipairs(toEmit) do
                    v.Line = thisLine
                    v.Char = thisChar
                    v.Print = function()
                        return "<"..(v.Type..string.rep(' ', 7-#v.Type)).."  "..(v.Data or '').." >"
                    end
                    tokens[#tokens+1] = v
                end
            else
                --add the emitted symbol, after adding some common data
                toEmit.LeadingWhite = leading -- table of leading whitespace/comments
                --for k, tok in pairs(leading) do
                --  tokens[#tokens + 1] = tok
                --end

                toEmit.Line = thisLine
                toEmit.Char = thisChar
                toEmit.Print = function()
                    return "<"..(toEmit.Type..string.rep(' ', 7-#toEmit.Type)).."  "..(toEmit.Data or '').." >"
                end
                tokens[#tokens+1] = toEmit

                --halt after eof has been emitted
                if toEmit.Type == 'Eof' then break end
            end
        end
    end)
    if not st then
        return false, err
    end

    --public interface:
    local tok = {}
    local savedP = {}
    local p = 1
    
    function tok:getp()
        return p
    end
    
    function tok:setp(n)
        p = n
    end
    
    function tok:getTokenList()
        return tokens
    end
    
    --getters
    function tok:Peek(n)
        n = n or 0
        return tokens[math.min(#tokens, p+n)]
    end
    function tok:Get(tokenList)
        local t = tokens[p]
        p = math.min(p + 1, #tokens)
        if tokenList then
            table.insert(tokenList, t)
        end
        return t
    end
    function tok:Is(t)
        return tok:Peek().Type == t
    end

    --save / restore points in the stream
    function tok:Save()
        savedP[#savedP+1] = p
    end
    function tok:Commit()
        savedP[#savedP] = nil
    end
    function tok:Restore()
        p = savedP[#savedP]
        savedP[#savedP] = nil
    end

    --either return a symbol if there is one, or return true if the requested
    --symbol was gotten.
    function tok:ConsumeSymbol(symb, tokenList)
        local t = self:Peek()
        if t.Type == 'Symbol' then
            if symb then
                if t.Data == symb then
                    self:Get(tokenList)
                    return true
                else
                    return nil
                end
            else
                self:Get(tokenList)
                return t
            end
        else
            return nil
        end
    end

    function tok:ConsumeKeyword(kw, tokenList)
        local t = self:Peek()
        if t.Type == 'Keyword' and t.Data == kw then
            self:Get(tokenList)
            return true
        else
            return nil
        end
    end

    function tok:IsKeyword(kw)
        local t = tok:Peek()
        return t.Type == 'Keyword' and t.Data == kw
    end

    function tok:IsSymbol(s)
        local t = tok:Peek()
        return t.Type == 'Symbol' and t.Data == s
    end

    function tok:IsEof()
        return tok:Peek().Type == 'Eof'
    end

    return true, tok
end


local function ParseLua(src, src_name)
    local st, tok
    if type(src) ~= 'table' then
        st, tok = LexLua(src)
    else
        st, tok = true, src
    end
    if not st then
        return false, tok
    end
    --
    local function GenerateError(msg)
        local err = (src_name or '=(string)') .. ":"..tok:Peek().Line..":"..tok:Peek().Char..": "..msg.."\n "
        --find the line
        local lineNum = 0
        if type(src) == 'string' then
            for line in src:gmatch("[^\n]*\n?") do
                if line:sub(-1,-1) == '\n' then line = line:sub(1,-2) end
                lineNum = lineNum+1
                if lineNum == tok:Peek().Line then
                    err = err..line:gsub('\t','    ').."\n"
                    for i = 1, tok:Peek().Char do
                        local c = line:sub(i,i)
                        if c == '\t' then
                            err = err..'    '
                        else
                            err = err..' '
                        end
                    end
                    err = err.."^^^^"
                    break
                end
            end
        end
        return err
    end
    --
    local VarUid = 0
    local function CreateScope(parent)
        local scope = Scope:new(parent)
        scope.Print = function() return "<Scope>" end
        return scope
    end

    local ParseExpr
    local ParseStatementList
    local ParseSimpleExpr, 
            ParseSubExpr,
            ParsePrimaryExpr,
            ParseSuffixedExpr

    local function ParseFunctionArgsAndBody(scope, tokenList)
        local funcScope = CreateScope(scope)
        if not tok:ConsumeSymbol('(', tokenList) then
            return false, GenerateError("'(' expected")
        end

        --arg list
        local argList = {}
        local isVarArg = false
        while not tok:ConsumeSymbol(')', tokenList) do
            if tok:Is('Ident') then
                local arg = funcScope:CreateLocal(tok:Get(tokenList).Data)
                argList[#argList+1] = arg
                if not tok:ConsumeSymbol(',', tokenList) then
                    if tok:ConsumeSymbol(')', tokenList) then
                        break
                    else
                        return false, GenerateError("')' expected")
                    end
                end
            elseif tok:ConsumeSymbol('...', tokenList) then
                isVarArg = true
                if not tok:ConsumeSymbol(')', tokenList) then
                    return false, GenerateError("'...' must be the last argument of a function")
                end
                break
            else
                return false, GenerateError("Argument name or '...' expected")
            end
        end

        --body
        local st, body = ParseStatementList(funcScope)
        if not st then return false, body end

        --end
        if not tok:ConsumeKeyword('end', tokenList) then
            return false, GenerateError("'end' expected after function body")
        end
        local nodeFunc = {}
        nodeFunc.AstType   = 'Function'
        nodeFunc.Scope     = funcScope
        nodeFunc.Arguments = argList
        nodeFunc.Body      = body
        nodeFunc.VarArg    = isVarArg
        nodeFunc.Tokens    = tokenList
        --
        return true, nodeFunc
    end


    function ParsePrimaryExpr(scope)
        local tokenList = {}

        if tok:ConsumeSymbol('(', tokenList) then
            local st, ex = ParseExpr(scope)
            if not st then return false, ex end
            if not tok:ConsumeSymbol(')', tokenList) then
                return false, GenerateError("')' expected")
            end
            if false then
                --save the information about parenthesized expressions somewhere
                ex.ParenCount = (ex.ParenCount or 0) + 1
                return true, ex
            else
                local parensExp = {}
                parensExp.AstType   = 'Parentheses'
                parensExp.Inner     = ex
                parensExp.Tokens    = tokenList
                return true, parensExp
            end

        elseif tok:Is('Ident') then
            local id = tok:Get(tokenList)
            local var = scope:GetLocal(id.Data)
            if not var then
                var = scope:GetGlobal(id.Data)
                if not var then
                    var = scope:CreateGlobal(id.Data)
                end
            end
            --
            local nodePrimExp = {}
            nodePrimExp.AstType   = 'VarExpr'
            nodePrimExp.Name      = id.Data
            nodePrimExp.Variable  = var
            nodePrimExp.Tokens    = tokenList
            --
            return true, nodePrimExp

        else
            return false, GenerateError("Primary expression expected")
        end
    end

    function ParseSuffixedExpr(scope, onlyDotColon)
        --base primary expression
        local st, prim = ParsePrimaryExpr(scope)
        if not st then return false, prim end
        --
        while true do
            local tokenList = {}

            if tok:IsSymbol('.') or tok:IsSymbol(':') then
                local symb = tok:Get(tokenList).Data
                if not tok:Is('Ident') then
                    return false, GenerateError("Identifier expected")
                end
                local id = tok:Get(tokenList)
                local nodeIndex = {}
                nodeIndex.AstType  = 'MemberExpr'
                nodeIndex.Base     = prim
                nodeIndex.Indexer  = symb
                nodeIndex.Ident    = id
                nodeIndex.Tokens   = tokenList
                --
                prim = nodeIndex

            elseif not onlyDotColon and tok:ConsumeSymbol('[', tokenList) then
                local st, ex = ParseExpr(scope)
                if not st then return false, ex end
                if not tok:ConsumeSymbol(']', tokenList) then
                    return false, GenerateError("']' expected")
                end
                local nodeIndex = {}
                nodeIndex.AstType  = 'IndexExpr'
                nodeIndex.Base     = prim
                nodeIndex.Index    = ex
                nodeIndex.Tokens   = tokenList
                --
                prim = nodeIndex

            elseif not onlyDotColon and tok:ConsumeSymbol('(', tokenList) then
                local args = {}
                while not tok:ConsumeSymbol(')', tokenList) do
                    local st, ex = ParseExpr(scope)
                    if not st then return false, ex end
                    args[#args+1] = ex
                    if not tok:ConsumeSymbol(',', tokenList) then
                        if tok:ConsumeSymbol(')', tokenList) then
                            break
                        else
                            return false, GenerateError("')' expected")
                        end
                    end
                end
                local nodeCall = {}
                nodeCall.AstType   = 'CallExpr'
                nodeCall.Base      = prim
                nodeCall.Arguments = args
                nodeCall.Tokens    = tokenList
                --
                prim = nodeCall

            elseif not onlyDotColon and tok:Is('String') then
                --string call
                local nodeCall = {}
                nodeCall.AstType    = 'StringCallExpr'
                nodeCall.Base       = prim
                nodeCall.Arguments  = { tok:Get(tokenList) }
                nodeCall.Tokens     = tokenList
                --
                prim = nodeCall

            elseif not onlyDotColon and tok:IsSymbol('{') then
                --table call
                local st, ex = ParseSimpleExpr(scope)
                -- FIX: ParseExpr(scope) parses the table AND and any following binary expressions.
                -- We just want the table
                if not st then return false, ex end
                local nodeCall = {}
                nodeCall.AstType   = 'TableCallExpr'
                nodeCall.Base      = prim
                nodeCall.Arguments = { ex }
                nodeCall.Tokens    = tokenList
                --
                prim = nodeCall

            else
                break
            end
        end
        return true, prim
    end


    function ParseSimpleExpr(scope)
        local tokenList = {}

        if tok:Is('Number') then
            local nodeNum = {}
            nodeNum.AstType = 'NumberExpr'
            nodeNum.Value   = tok:Get(tokenList)
            nodeNum.Tokens  = tokenList
            return true, nodeNum

        elseif tok:Is('String') then
            local nodeStr = {}
            nodeStr.AstType = 'StringExpr'
            nodeStr.Value   = tok:Get(tokenList)
            nodeStr.Tokens  = tokenList
            return true, nodeStr

        elseif tok:ConsumeKeyword('nil', tokenList) then
            local nodeNil = {}
            nodeNil.AstType = 'NilExpr'
            nodeNil.Tokens  = tokenList
            return true, nodeNil

        elseif tok:IsKeyword('false') or tok:IsKeyword('true') then
            local nodeBoolean = {}
            nodeBoolean.AstType = 'BooleanExpr'
            nodeBoolean.Value   = (tok:Get(tokenList).Data == 'true')
            nodeBoolean.Tokens  = tokenList
            return true, nodeBoolean

        elseif tok:ConsumeSymbol('...', tokenList) then
            local nodeDots = {}
            nodeDots.AstType  = 'DotsExpr'
            nodeDots.Tokens   = tokenList
            return true, nodeDots

        elseif tok:ConsumeSymbol('{', tokenList) then
            local v = {}
            v.AstType = 'ConstructorExpr'
            v.EntryList = {}
            --
            while true do
                if tok:IsSymbol('[', tokenList) then
                    --key
                    tok:Get(tokenList)
                    local st, key = ParseExpr(scope)
                    if not st then
                        return false, GenerateError("Key expression expected")
                    end
                    if not tok:ConsumeSymbol(']', tokenList) then
                        return false, GenerateError("']' expected")
                    end
                    if not tok:ConsumeSymbol('=', tokenList) then
                        return false, GenerateError("'=' expected")
                    end
                    local st, value = ParseExpr(scope)
                    if not st then
                        return false, GenerateError("Value expression expected")
                    end
                    v.EntryList[#v.EntryList+1] = {
                        Type  = 'Key';
                        Key   = key;
                        Value = value;
                    }

                elseif tok:Is('Ident') then
                    --value or key
                    local lookahead = tok:Peek(1)
                    if lookahead.Type == 'Symbol' and lookahead.Data == '=' then
                        --we are a key
                        local key = tok:Get(tokenList)
                        if not tok:ConsumeSymbol('=', tokenList) then
                            return false, GenerateError("'=' expected")
                        end
                        local st, value = ParseExpr(scope)
                        if not st then
                            return false, GenerateError("Value expression expected")
                        end
                        v.EntryList[#v.EntryList+1] = {
                            Type  = 'KeyString';
                            Key   = key.Data;
                            Value = value;
                        }

                    else
                        --we are a value
                        local st, value = ParseExpr(scope)
                        if not st then
                            return false, GenerateError("Value expected")
                        end
                        v.EntryList[#v.EntryList+1] = {
                            Type = 'Value';
                            Value = value;
                        }

                    end
                elseif tok:ConsumeSymbol('}', tokenList) then
                    break

                else
                    --value
                    local st, value = ParseExpr(scope)
                    v.EntryList[#v.EntryList+1] = {
                        Type = 'Value';
                        Value = value;
                    }
                    if not st then
                        return false, GenerateError("Value expected")
                    end
                end

                if tok:ConsumeSymbol(';', tokenList) or tok:ConsumeSymbol(',', tokenList) then
                    --all is good
                elseif tok:ConsumeSymbol('}', tokenList) then
                    break
                else
                    return false, GenerateError("'}' or table entry expected")
                end
            end
            v.Tokens  = tokenList
            return true, v

        elseif tok:ConsumeKeyword('function', tokenList) then
            local st, func = ParseFunctionArgsAndBody(scope, tokenList)
            if not st then return false, func end
            --
            func.IsLocal = true
            return true, func

        elseif tok:ConsumeSymbol('\\', tokenList) then -- short lambdas \params(expr)
            local funcScope = CreateScope(scope)

            local argList = {}
            if not tok:ConsumeSymbol('(', tokenList) then while true do
                if not tok:Is('Ident') then return false, GenerateError("Identifier expected") end
                argList[#argList+1] = funcScope:CreateLocal(tok:Get(tokenList).Data)
                if tok:ConsumeSymbol('(', tokenList) then break end
                if not tok:ConsumeSymbol(',', tokenList) then return false, GenerateError("'(' expected") end
            end end
            local st, body = ParseExpr(funcScope)
            if not st then return false, body end
            if not tok:ConsumeSymbol(')', tokenList) then return false, GenerateError("')' expected after lambda body") end

            local last_tok = body.Tokens[1]
            local last_char,last_line = last_tok.Char,last_tok.Line
            local space = { Char=last_char, Line=last_line, Data=' ', Type='Whitespace' }
            local ret = { AstType='ReturnStatement', Arguments={body}, TrailingWhite=true, Tokens={
                { Char=last_char, Line=last_line, Print=function() return '<Keyword  return >' end, LeadingWhite={space}, Type='Keyword', Data='return' }
            }}
            local statList = { AstType='Statlist', Scope=CreateScope(funcScope), Tokens={}, Body={ret} }
            
            local tok_first = tokenList[1]
            tok_first.Type = 'Keyword'
            tok_first.Data = 'function'
            tok_first.Print=function() return '<Keyword  function >' end

            local ntokl = {}
            local paren_ix = 2 + math.max(0, #argList*2-1)
            table.insert(ntokl, tokenList[1])
            table.insert(ntokl, tokenList[paren_ix])
            for i=2,paren_ix-1 do table.insert(ntokl, tokenList[i]) end
            table.insert(ntokl, tokenList[#tokenList])
            table.insert(ntokl, { Char=last_char, Line=last_line, Type='Keyword', Data='end', Print=function() return '<Keyword  end >' end, LeadingWhite={space} })

            local func = { AstType='Function', Scope=funcScope, Arguments=argList, Body=statList, VarArg=false, Tokens=ntokl, isLocal=true }
            return true, func

        else
            return ParseSuffixedExpr(scope)
        end
    end


    local unops = lookupify{'-', 'not', '#', '~'}
    local unopprio = 12
    local priority = {
        ['^'] = {14,13};
        ['%'] = {11,11};
        ['//'] = {11,11};
        ['/'] = {11,11};
        ['*'] = {11,11};
        ['+'] = {10,10};
        ['-'] = {10,10};
        ['..'] = {9,8};
        ['>>'] = {7,7};
        ['<<'] = {7,7};
        ['&'] = {6,6};
        ['~'] = {5,5};
        ['|'] =  {4,4};
        ['=='] = {3,3};
        ['<'] = {3,3};
        ['<='] = {3,3};
        ['~='] = {3,3};
        ['>'] = {3,3};
        ['>='] = {3,3};
        ['and'] = {2,2};
        ['or'] = {1,1};
    }
    function ParseSubExpr(scope, level)
        --base item, possibly with unop prefix
        local st, exp
        if unops[tok:Peek().Data] then
            local tokenList = {}
            local op = tok:Get(tokenList).Data
            st, exp = ParseSubExpr(scope, unopprio)
            if not st then return false, exp end
            local nodeEx = {}
            nodeEx.AstType = 'UnopExpr'
            nodeEx.Rhs     = exp
            nodeEx.Op      = op
            nodeEx.OperatorPrecedence = unopprio
            nodeEx.Tokens  = tokenList
            exp = nodeEx
        else
            st, exp = ParseSimpleExpr(scope)
            if not st then return false, exp end
        end

        --next items in chain
        while true do
            local prio = priority[tok:Peek().Data]
            if prio and prio[1] > level then
                local tokenList = {}
                local op = tok:Get(tokenList).Data
                local st, rhs = ParseSubExpr(scope, prio[2])
                if not st then return false, rhs end
                local nodeEx = {}
                nodeEx.AstType = 'BinopExpr'
                nodeEx.Lhs     = exp
                nodeEx.Op      = op
                nodeEx.OperatorPrecedence = prio[1]
                nodeEx.Rhs     = rhs
                nodeEx.Tokens  = tokenList
                --
                exp = nodeEx
            else
                break
            end
        end

        return true, exp
    end


    ParseExpr = function(scope)
        return ParseSubExpr(scope, 0)
    end


    local pragma_map = {
        encapsulate_on = function() opcode_arg_encapsulate(true) end,
        encapsulate_off = function() opcode_arg_encapsulate(false) end,
        syntax7801k_on = function() syntax7801k(true) end,
        syntax7801k_off = function() syntax7801k(false) end,
    }
    local function ParseStatement(scope)
        local stat = nil
        local tokenList = {}
        local commaTokenList = {}
        local l,c = function() return tokenList[1].Line end, function() return tokenList[1].Char end
        local p = function(t,n) return function() return '<' .. t .. string.rep(' ', 7-#t) .. ' ' .. n .. ' >' end end
        local t = function(t,s,w) return { Type=t, Data=s, Print=p(t,s), Char=c(), Line=l(), LeadingWhite=w or {} } end
        local no_encapsulation = { Function=true, NumberExpr=true, StringExpr=true }
        local reg = function(e) local t=e.Tokens[1] if t then return Registers_7801[t.Data] end end

        local function emit_call(params)
            local name,args,inverse_encapsulate = params.name, params.args or {}, params.inverse_encapsulate
            if not params.func_white then params.func_white = tokenList[1].LeadingWhite end
            local space = { Char=c(), Line=l(), Data=' ', Type='Whitespace' }
            local op_var = {
                AstType='VarExpr', Name=name, Variable={ IsGlobal=true, Name=name, Scope=CreateScope(scope) }, Tokens = { t('Ident', name, params.func_white) }
            }
            local encapsulate = params.encapsulate
            if encapsulate == nil then encapsulate = opcode_arg_encapsulate_on end
            if type(inverse_encapsulate) == 'table' then
                -- this is a list of arguments, where each can be encapsulated
                for arg_ix,arg in ipairs(args) do
                    if ( (encapsulate and not inverse_encapsulate[arg_ix]) or (not encapsulate and inverse_encapsulate[arg_ix]) ) and not no_encapsulation[arg.AstType] then
                        local inner_call_scope = CreateScope(op_var.Variable.Scope)
                        local inner_call_body = {
                            AstType='StatList', Scope=CreateScope(inner_call_scope), Tokens={}, Body={
                                { AstType='ReturnStatement', Arguments={arg}, Tokens={ t('Keyword', 'return', {space}) } }
                            }
                        }
                        local inner_call = {
                            AstType='Function', VarArg=false, IsLocal=true, Scope=inner_call_scope, Body=inner_call_body, Arguments={},
                            Tokens={ t('Keyword', 'function'), t('Symbol', '('), t('Symbol', ')'), t('Keyword', 'end', {space}) }
                        }
                        if #arg.Tokens[1].LeadingWhite == 0 then table.insert(arg.Tokens[1].LeadingWhite, space) end
                        args[arg_ix] = inner_call
                    end
                end
            elseif #args > 0 and ( (encapsulate and not inverse_encapsulate) or (not encapsulate and inverse_encapsulate) ) and not no_encapsulation[args[1].AstType] then
                -- opcode arguments of type (late, early), where only late is to be encapsulated with _o parameter to be set to early and added to _f(late)
                local inner_call_scope,inner_call = CreateScope(op_var.Variable.Scope)
                if params.basic then
                    local inner_call_body = {
                        AstType='StatList', Scope=CreateScope(inner_call_scope), Tokens={}, Body={
                            { AstType='ReturnStatement', Arguments={args[1]}, Tokens={ t('Keyword', 'return', {space}) } }
                        }
                    }
                    inner_call = {
                        AstType='Function', VarArg=false, IsLocal=true, Scope=inner_call_scope, Body=inner_call_body, Arguments={},
                        Tokens={ t('Keyword', 'function'), t('Symbol', '('), t('Symbol', ')'), t('Keyword', 'end', {space}) }
                    }
                else
                    local fvarexpr = {
                        AstType='VarExpr', Name='_f', Variable={ Name='_f', Scope=CreateScope(inner_call_scope) }, Tokens = { t('Ident', '_f') }
                    }
                    local inner_add = {
                        AstType='BinopExpr', Op='+', OperatorPrecedence=10, Tokens={ t('Symbol', '+') },
                        Lhs = {
                            AstType='VarExpr', Name='_o', Variable={ IsGlobal=false, Name='_o', Scope=inner_call_scope }, Tokens={ t('Ident', '_o', {space}) }
                        },
                        Rhs = {
                            AstType = 'CallExpr', Base = fvarexpr, Arguments = {args[1]}, Tokens = { t('Symbol', '('), t('Symbol', ')') }
                        }
                    }
                    local inner_call_body = {
                        AstType='StatList', Scope=CreateScope(inner_call_scope), Tokens={}, Body={
                            { AstType='ReturnStatement', Arguments={inner_add}, Tokens={ t('Keyword', 'return', {space}) } }
                        }
                    }
                    inner_call = {
                        AstType='Function', VarArg=false, IsLocal=true, Scope=inner_call_scope, Body=inner_call_body,
                        Arguments={
                            { IsGlobal=false, Name='_o', Scope=inner_call_scope },
                            { IsGlobal=false, Name='_f', Scope=inner_call_scope },
                        },
                        Tokens={ t('Keyword', 'function'), t('Symbol', '('), t('Ident', '_o'), t('Symbol', ','), t('Ident', '_f'), t('Symbol', ')'), t('Keyword', 'end', {space}) }
                    }
                end
                args[1] = inner_call
            end
            local exp_call = {
                AstType = 'CallExpr', Base = op_var, Arguments = args, Tokens = {
                    t('Symbol', '(', params.paren_open_white), t('Symbol', ')', params.paren_close_white)
                }
            }
            do
                local j=#commaTokenList
                for i=2,#args do
                    assert(j > 0)
                    table.insert(exp_call.Tokens, i, commaTokenList[j])
                    j = j - 1
                end
            end
            return { AstType = 'CallStatement', Expression = exp_call, Tokens = {} }
        end

        local function as_string_expr(expr, s)
            local ss = '"'..s..'"'
            local lw = expr.Tokens and #expr.Tokens > 0 and expr.Tokens[1].LeadingWhite or {}
            local p = function() return '<String   '..s..' >' end
            local v = { LeadingWhite=lw, Type='String', Data=ss, Constant=s, Char=c(), Line=l(), Print=p }
            return { AstType = 'StringExpr', Value = v, Tokens = {v} }
        end

        -- parser pragmas
        do
            ::search_pragma::
            for k,v in pairs(pragma_map) do
                if tok:ConsumeKeyword(k) then v() goto search_pragma end
            end
        end

        -- label declarations
        if not stat then
        if tok:ConsumeSymbol('@@', tokenList) then
            if not tok:Is('Ident') then return false, GenerateError("Identifier expected") end
            local label_name = tok:Get(tokenList)
            label_name = as_string_expr(label_name, label_name.Data)
            stat = emit_call{name = 'section', args = {label_name}, encapsulate=false}
        elseif tok:ConsumeSymbol('@', tokenList) then
            if not tok:Is('Ident') then return false, GenerateError("Identifier expected") end
            local label_name = tok:Get(tokenList)
            label_name = as_string_expr(label_name, label_name.Data)
            stat = emit_call{name = 'label', args = {label_name}, encapsulate=false}
        end end

        -- new statements
        if not stat then
            local pagestat = function(fpage)
                local st, nodeBlock = ParseStatementList(scope)
                if not st then return false, nodeBlock end
                if not tok:ConsumeKeyword('end', tokenList) then
                    return false, GenerateError("'end' expected")
                end

                local nodeDoStat = {}
                nodeDoStat.AstType = 'DoStatement'
                nodeDoStat.Body    = nodeBlock
                nodeDoStat.Tokens  = tokenList
                stat = nodeDoStat

                tokenList[1].Data = 'do'

                local space = {{ Char=c(), Line=l(), Data=' ', Type='Whitespace' }}
                local opencall,closecall = emit_call{name=fpage,func_white=space},emit_call{name='endpage',func_white=space}
                table.insert(nodeBlock.Body, 1, opencall)
                table.insert(nodeBlock.Body, closecall)
            end
            if tok:ConsumeKeyword('samepage', tokenList) then pagestat('samepage')
            elseif tok:ConsumeKeyword('crosspage', tokenList) then pagestat('crosspage')
            end
        end

        -- declare data
        if not stat then
        if tok:ConsumeKeyword('dc', tokenList) then
            if not tok:ConsumeSymbol('.', tokenList) then return false, GenerateError("'.' expected") end
            local suffix_list = { b='byte', w='word', l='long' }
            local func = suffix_list[tok:Get(tokenList).Data]
            if not func then return false, GenerateError("'b', 'w' or 'l' expected") end
            local inverse_encapsulate={}
            inverse_encapsulate[1] = tok:ConsumeSymbol('!', tokenList)
            local st, expr = ParseExpr(scope)
            if not st then return false, expr end
            if inverse_encapsulate[1] then
                local d = expr.Tokens[1].LeadingWhite
                for _,v in ipairs(tokenList[#tokenList].LeadingWhite) do table.insert(d, v) end
            end
            local exprs = { expr }
            while tok:ConsumeSymbol(',', tokenList) do
                commaTokenList[#commaTokenList+1] = tokenList[#tokenList]
                inverse_encapsulate[#exprs+1] = tok:ConsumeSymbol('!', tokenList)
                local st, expr = ParseExpr(scope)
                if not st then return false, expr end
                if inverse_encapsulate[#exprs+1] then
                    local d = expr.Tokens[1].LeadingWhite
                    for _,v in ipairs(tokenList[#tokenList].LeadingWhite) do table.insert(d, v) end
                end
                exprs[#exprs+1] = expr
            end
            stat = emit_call{name=func, args=exprs, inverse_encapsulate=inverse_encapsulate}
        end end

        -- 7801 opcodes
        if not stat then
            local mod_st, mod_expr, inverse_encapsulate
        for _,op in pairs(Keywords_7801) do
            if tok:ConsumeKeyword(op, tokenList) then
                if opcode_relative[op] then
                    local st, expr = ParseExpr(scope) if not st then return false, expr end
                    if expr.AstType == 'VarExpr' and expr.Variable.IsGlobal then
                        expr = as_string_expr(expr, expr.Name)
                    end
                    stat = emit_call{name=op, args={expr}, encapsulate=false} break
                end
                if opcode_immediate[op] then
                    inverse_encapsulate = tok:ConsumeSymbol('!', tokenList)
                    local st, expr = ParseExpr(scope) if not st then return false, expr end
                    local paren_open_whites = {}
                    if inverse_encapsulate then for _,v in ipairs(tokenList[#tokenList-1].LeadingWhite) do table.insert(paren_open_whites, v) end end
                    for _,v in ipairs(tokenList[#tokenList].LeadingWhite) do table.insert(paren_open_whites, v) end
                    if tok:ConsumeSymbol(',', tokenList) then
                        commaTokenList[1] = tokenList[#tokenList]
                        mod_st, mod_expr = ParseExpr(scope)
                        if not mod_st then return false, mod_expr end
                    end
                    stat = emit_call{name=op, args={expr, mod_expr}, inverse_encapsulate=inverse_encapsulate, paren_open_white=paren_open_whites} break
                end
                if opcode_reg[op] or opcode_regb[op] or opcode_regw[op] then
                    tok:Save()
                    local register_name = tok:Get(tokenList).Data
                    local call_args = {name=op..register_name}
                    if not Registers_7801[register_name] then tok:Restore()
                        return false, GenerateError(register_name .. " is not a valid register")
                    end
                    if not opcode_reg_list[register_name] and opcode_reg_list[register_name][op] then tok:Restore() 
                        return false, GenerateError("Opcode " .. op .. " doesn't support this addressing mode")
                    end
                    if opcode_regw[op] or opcode_regb[op] then
                        if not tok:ConsumeSymbol(',', tokenList) then tok:Restore()
                            return false, GenerateError("Opcode " .. op .. " doesn't support this addressing mode")
                        end
                        local inverse_encapsulate = tok:ConsumeSymbol('!', tokenList)
                        local st, expr = ParseExpr(scope) if not st then tok:Restore()
                            return false, expr
                        end
                        local paren_open_whites = {}
                        if inverse_encapsulate then for _,v in ipairs(tokenList[#tokenList-1].LeadingWhite) do table.insert(paren_open_whites, v) end end
                        for _,v in ipairs(tokenList[#tokenList].LeadingWhite) do table.insert(paren_open_whites, v) end
                        if tok:ConsumeSymbol(',', tokenList) then
                            commaTokenList[1] = tokenList[#tokenList]
                            mod_st, mod_expr = ParseExpr(scope)
                            if not mod_st then tok:Restore()
                                return false, mod_expr
                            end
                        end
                        call_args.args={expr, mod_expr}
                        call_args.inverse_encapsulate=inverse_encapsulate 
                        call_args.paren_open_white=paren_open_whites
                    end
                    tok:Commit()
                    stat = emit_call(call_args) break
                end
                if opcode_implied[op] then stat = emit_call{name=op .. 'imp'} break end
                error("internal error: unable to find addressing of valid opcode " .. op) -- should not happen
            end
        end end

        if stat then -- nothing
        elseif tok:ConsumeKeyword('if', tokenList) then
            --setup
            local nodeIfStat = {}
            nodeIfStat.AstType = 'IfStatement'
            nodeIfStat.Clauses = {}

            --clauses
            repeat
                local st, nodeCond = ParseExpr(scope)
                if not st then return false, nodeCond end
                if not tok:ConsumeKeyword('then', tokenList) then
                    return false, GenerateError("'then' expected")
                end
                local st, nodeBody = ParseStatementList(scope)
                if not st then return false, nodeBody end
                nodeIfStat.Clauses[#nodeIfStat.Clauses+1] = {
                    Condition = nodeCond;
                    Body = nodeBody;
                }
            until not tok:ConsumeKeyword('elseif', tokenList)

            --else clause
            if tok:ConsumeKeyword('else', tokenList) then
                local st, nodeBody = ParseStatementList(scope)
                if not st then return false, nodeBody end
                nodeIfStat.Clauses[#nodeIfStat.Clauses+1] = {
                    Body = nodeBody;
                }
            end

            --end
            if not tok:ConsumeKeyword('end', tokenList) then
                return false, GenerateError("'end' expected")
            end

            nodeIfStat.Tokens = tokenList
            stat = nodeIfStat

        elseif tok:ConsumeKeyword('while', tokenList) then
            --setup
            local nodeWhileStat = {}
            nodeWhileStat.AstType = 'WhileStatement'

            --condition
            local st, nodeCond = ParseExpr(scope)
            if not st then return false, nodeCond end

            --do
            if not tok:ConsumeKeyword('do', tokenList) then
                return false, GenerateError("'do' expected")
            end

            --body
            local st, nodeBody = ParseStatementList(scope)
            if not st then return false, nodeBody end

            --end
            if not tok:ConsumeKeyword('end', tokenList) then
                return false, GenerateError("'end' expected")
            end

            --return
            nodeWhileStat.Condition = nodeCond
            nodeWhileStat.Body      = nodeBody
            nodeWhileStat.Tokens    = tokenList
            stat = nodeWhileStat

        elseif tok:ConsumeKeyword('do', tokenList) then
            --do block
            local st, nodeBlock = ParseStatementList(scope)
            if not st then return false, nodeBlock end
            if not tok:ConsumeKeyword('end', tokenList) then
                return false, GenerateError("'end' expected")
            end

            local nodeDoStat = {}
            nodeDoStat.AstType = 'DoStatement'
            nodeDoStat.Body    = nodeBlock
            nodeDoStat.Tokens  = tokenList
            stat = nodeDoStat

        elseif tok:ConsumeKeyword('for', tokenList) then
            --for block
            if not tok:Is('Ident') then
                return false, GenerateError("Identifier expected")
            end
            local baseVarName = tok:Get(tokenList)
            if tok:ConsumeSymbol('=', tokenList) then
                --numeric for
                local forScope = CreateScope(scope)
                local forVar = forScope:CreateLocal(baseVarName.Data)
                --
                local st, startEx = ParseExpr(scope)
                if not st then return false, startEx end
                if not tok:ConsumeSymbol(',', tokenList) then
                    return false, GenerateError("',' expected")
                end
                local st, endEx = ParseExpr(scope)
                if not st then return false, endEx end
                local st, stepEx;
                if tok:ConsumeSymbol(',', tokenList) then
                    st, stepEx = ParseExpr(scope)
                    if not st then return false, stepEx end
                end
                if not tok:ConsumeKeyword('do', tokenList) then
                    return false, GenerateError("'do' expected")
                end
                --
                local st, body = ParseStatementList(forScope)
                if not st then return false, body end
                if not tok:ConsumeKeyword('end', tokenList) then
                    return false, GenerateError("'end' expected")
                end
                --
                local nodeFor = {}
                nodeFor.AstType  = 'NumericForStatement'
                nodeFor.Scope    = forScope
                nodeFor.Variable = forVar
                nodeFor.Start    = startEx
                nodeFor.End      = endEx
                nodeFor.Step     = stepEx
                nodeFor.Body     = body
                nodeFor.Tokens   = tokenList
                stat = nodeFor
            else
                --generic for
                local forScope = CreateScope(scope)
                --
                local varList = { forScope:CreateLocal(baseVarName.Data) }
                while tok:ConsumeSymbol(',', tokenList) do
                    if not tok:Is('Ident') then
                        return false, GenerateError("for variable expected.")
                    end
                    varList[#varList+1] = forScope:CreateLocal(tok:Get(tokenList).Data)
                end
                if not tok:ConsumeKeyword('in', tokenList) then
                    return false, GenerateError("'in' expected")
                end
                local generators = {}
                local st, firstGenerator = ParseExpr(scope)
                if not st then return false, firstGenerator end
                generators[#generators+1] = firstGenerator
                while tok:ConsumeSymbol(',', tokenList) do
                    local st, gen = ParseExpr(scope)
                    if not st then return false, gen end
                    generators[#generators+1] = gen
                end
                if not tok:ConsumeKeyword('do', tokenList) then
                    return false, GenerateError("'do' expected")
                end
                local st, body = ParseStatementList(forScope)
                if not st then return false, body end
                if not tok:ConsumeKeyword('end', tokenList) then
                    return false, GenerateError("'end' expected")
                end
                --
                local nodeFor = {}
                nodeFor.AstType      = 'GenericForStatement'
                nodeFor.Scope        = forScope
                nodeFor.VariableList = varList
                nodeFor.Generators   = generators
                nodeFor.Body         = body
                nodeFor.Tokens       = tokenList
                stat = nodeFor
            end

        elseif tok:ConsumeKeyword('repeat', tokenList) then
            local st, body = ParseStatementList(scope)
            if not st then return false, body end
            --
            if not tok:ConsumeKeyword('until', tokenList) then
                return false, GenerateError("'until' expected")
            end
            -- FIX: Used to parse in parent scope
            -- Now parses in repeat scope
            local st, cond = ParseExpr(body.Scope)
            if not st then return false, cond end
            --
            local nodeRepeat = {}
            nodeRepeat.AstType   = 'RepeatStatement'
            nodeRepeat.Condition = cond
            nodeRepeat.Body      = body
            nodeRepeat.Tokens    = tokenList
            stat = nodeRepeat

        elseif tok:ConsumeKeyword('function', tokenList) then
            if not tok:Is('Ident') then
                return false, GenerateError("function name expected")
            end
            local st, name = ParseSuffixedExpr(scope, true) --true => only dots and colons
            if not st then return false, name end
            --
            local st, func = ParseFunctionArgsAndBody(scope, tokenList)
            if not st then return false, func end
            --
            func.IsLocal = false
            func.Name    = name
            stat = func

        elseif tok:ConsumeKeyword('local', tokenList) then
            if tok:Is('Ident') then
                local varList = { tok:Get(tokenList).Data }
                while tok:ConsumeSymbol(',', tokenList) do
                    if not tok:Is('Ident') then
                        return false, GenerateError("local var name expected")
                    end
                    varList[#varList+1] = tok:Get(tokenList).Data
                end

                local initList = {}
                if tok:ConsumeSymbol('=', tokenList) then
                    repeat
                        local st, ex = ParseExpr(scope)
                        if not st then return false, ex end
                        initList[#initList+1] = ex
                    until not tok:ConsumeSymbol(',', tokenList)
                end

                --now patch var list
                --we can't do this before getting the init list, because the init list does not
                --have the locals themselves in scope.
                for i, v in pairs(varList) do
                    varList[i] = scope:CreateLocal(v)
                end

                local nodeLocal = {}
                nodeLocal.AstType   = 'LocalStatement'
                nodeLocal.LocalList = varList
                nodeLocal.InitList  = initList
                nodeLocal.Tokens    = tokenList
                --
                stat = nodeLocal

            elseif tok:ConsumeKeyword('function', tokenList) then
                if not tok:Is('Ident') then
                    return false, GenerateError("function name expected")
                end
                local name = tok:Get(tokenList).Data
                local localVar = scope:CreateLocal(name)
                --
                local st, func = ParseFunctionArgsAndBody(scope, tokenList)
                if not st then return false, func end
                --
                func.Name         = localVar
                func.IsLocal      = true
                stat = func

            else
                return false, GenerateError("local var or function def expected")
            end

        elseif tok:ConsumeSymbol('::', tokenList) then
            if not tok:Is('Ident') then
                return false, GenerateError('Label name expected')
            end
            local label = tok:Get(tokenList).Data
            if not tok:ConsumeSymbol('::', tokenList) then
                return false, GenerateError("'::' expected")
            end
            local nodeLabel = {}
            nodeLabel.AstType = 'LabelStatement'
            nodeLabel.Label   = label
            nodeLabel.Tokens  = tokenList
            stat = nodeLabel

        elseif tok:ConsumeKeyword('return', tokenList) then
            local exList = {}
            if not tok:IsKeyword('end') then
                local st, firstEx = ParseExpr(scope)
                if st then
                    exList[1] = firstEx
                    while tok:ConsumeSymbol(',', tokenList) do
                        local st, ex = ParseExpr(scope)
                        if not st then return false, ex end
                        exList[#exList+1] = ex
                    end
                end
            end

            local nodeReturn = {}
            nodeReturn.AstType   = 'ReturnStatement'
            nodeReturn.Arguments = exList
            nodeReturn.Tokens    = tokenList
            stat = nodeReturn

        elseif tok:ConsumeKeyword('break', tokenList) then
            local nodeBreak = {}
            nodeBreak.AstType = 'BreakStatement'
            nodeBreak.Tokens  = tokenList
            stat = nodeBreak

        elseif tok:ConsumeKeyword('goto', tokenList) then
            if not tok:Is('Ident') then
                return false, GenerateError("Label expected")
            end
            local label = tok:Get(tokenList).Data
            local nodeGoto = {}
            nodeGoto.AstType = 'GotoStatement'
            nodeGoto.Label   = label
            nodeGoto.Tokens  = tokenList
            stat = nodeGoto

        elseif tok:IsSymbol(';') then
            stat = { AstType='SemicolonStatement', Tokens={} }

        else

            --statementParseExpr
            local st, suffixed = ParseSuffixedExpr(scope)
            if not st then return false, suffixed end

            --assignment or call?
            if tok:IsSymbol(',') or tok:IsSymbol('=') then
                --check that it was not parenthesized, making it not an lvalue
                if (suffixed.ParenCount or 0) > 0 then
                    return false, GenerateError("Can not assign to parenthesized expression, is not an lvalue")
                end

                --more processing needed
                local lhs = { suffixed }
                while tok:ConsumeSymbol(',', tokenList) do
                    local st, lhsPart = ParseSuffixedExpr(scope)
                    if not st then return false, lhsPart end
                    lhs[#lhs+1] = lhsPart
                end

                --equals
                if not tok:ConsumeSymbol('=', tokenList) then
                    return false, GenerateError("'=' expected")
                end

                --rhs
                local rhs = {}
                local st, firstRhs = ParseExpr(scope)
                if not st then return false, firstRhs end
                rhs[1] = firstRhs
                while tok:ConsumeSymbol(',', tokenList) do
                    local st, rhsPart = ParseExpr(scope)
                    if not st then return false, rhsPart end
                    rhs[#rhs+1] = rhsPart
                end

                --done
                local nodeAssign = {}
                nodeAssign.AstType = 'AssignmentStatement'
                nodeAssign.Lhs     = lhs
                nodeAssign.Rhs     = rhs
                nodeAssign.Tokens  = tokenList
                stat = nodeAssign

            elseif suffixed.AstType == 'CallExpr' or
                   suffixed.AstType == 'TableCallExpr' or
                   suffixed.AstType == 'StringCallExpr'
            then
                --it's a call statement
                local nodeCall = {}
                nodeCall.AstType    = 'CallStatement'
                nodeCall.Expression = suffixed
                nodeCall.Tokens     = tokenList
                stat = nodeCall
            else
                return false, GenerateError("Assignment statement expected")
            end
        end

        if tok:IsSymbol(';') then
            stat.Semicolon = tok:Get( stat.Tokens )
        end
        return true, stat
    end


    local statListCloseKeywords = lookupify{'end', 'else', 'elseif', 'until'}

    ParseStatementList = function(scope)
        local nodeStatlist   = {}
        nodeStatlist.Scope   = CreateScope(scope)
        nodeStatlist.AstType = 'Statlist'
        nodeStatlist.Body    = { }
        nodeStatlist.Tokens  = { }
        --
        --local stats = {}
        --
        while not statListCloseKeywords[tok:Peek().Data] and not tok:IsEof() do
            local st, nodeStatement = ParseStatement(nodeStatlist.Scope)
            if not st then return false, nodeStatement end
            --stats[#stats+1] = nodeStatement
            nodeStatlist.Body[#nodeStatlist.Body + 1] = nodeStatement
        end

        if tok:IsEof() then
            local nodeEof = {}
            nodeEof.AstType = 'Eof'
            nodeEof.Tokens  = { tok:Get() }
            nodeStatlist.Body[#nodeStatlist.Body + 1] = nodeEof
        end

        --
        --nodeStatlist.Body = stats
        return true, nodeStatlist
    end


    local function mainfunc()
        local topScope = CreateScope()
        return ParseStatementList(topScope)
    end

    local st, main = mainfunc()
    return st, main
end


local function Format65(ast)
    local function splitLines(str)
        if str:match("\n") then
            local lines = {}
            for line in str:gmatch("[^\n]*") do 
                table.insert(lines, line)
            end
            assert(#lines > 0)
            return lines
        else
            return { str }
        end
    end

    local formatStatlist, formatExpr
    local out = {
        rope = {},  -- List of strings
        line = 1,
        char = 1,

        appendStr = function(self, str)
            table.insert(self.rope, str)

            local lines = splitLines(str)
            if #lines == 1 then
                self.char = self.char + #str
            else
                self.line = self.line + #lines - 1
                local lastLine = lines[#lines]
                self.char = #lastLine
            end
        end,

        appendToken = function(self, token)
            self:appendWhite(token)
            self:appendStr(token.Data)
        end,

        appendTokens = function(self, tokens)
            for _,token in ipairs(tokens) do
                self:appendToken( token )
            end
        end,

        appendWhite = function(self, token)
            if token.LeadingWhite then
                self:appendTokens( token.LeadingWhite )
            end
        end
    }
    formatExpr = function(expr)
        local tok_it = 1
        local function appendNextToken(str)
            local tok = expr.Tokens[tok_it];
            if str and tok.Data ~= str then
                error("Expected token '" .. str .. "'.")
            end
            out:appendToken( tok )
            tok_it = tok_it + 1
        end
        local function appendToken(token)
            out:appendToken( token )
            tok_it = tok_it + 1
        end
        local function appendWhite()
            local tok = expr.Tokens[tok_it];
            if not tok then error("Missing token") end
            out:appendWhite( tok )
            tok_it = tok_it + 1
        end
        local function appendStr(str)
            appendWhite()
            out:appendStr(str)
        end
        local function peek()
            if tok_it < #expr.Tokens then
                return expr.Tokens[tok_it].Data
            end
        end
        local function appendComma(mandatory, seperators)
            seperators = seperators or { "," }
            seperators = lookupify( seperators )
            if not mandatory and not seperators[peek()] then
                return
            end
            assert(seperators[peek()], "Missing comma or semicolon")
            appendNextToken()
        end

        if expr.AstType == 'VarExpr' then
            if expr.Variable then
                appendStr( expr.Variable.Name )
            else
                appendStr( expr.Name )
            end

        elseif expr.AstType == 'NumberExpr' then
            appendToken( expr.Value )

        elseif expr.AstType == 'StringExpr' then
            appendToken( expr.Value )

        elseif expr.AstType == 'BooleanExpr' then
            appendNextToken( expr.Value and "true" or "false" )

        elseif expr.AstType == 'NilExpr' then
            appendNextToken( "nil" )

        elseif expr.AstType == 'BinopExpr' then
            formatExpr(expr.Lhs)
            appendStr( expr.Op )
            formatExpr(expr.Rhs)

        elseif expr.AstType == 'UnopExpr' then
            appendStr( expr.Op )
            formatExpr(expr.Rhs)

        elseif expr.AstType == 'DotsExpr' then
            appendNextToken( "..." )

        elseif expr.AstType == 'CallExpr' then
            formatExpr(expr.Base)
            appendNextToken( "(" )
            for i,arg in ipairs( expr.Arguments ) do
                formatExpr(arg)
                appendComma( i ~= #expr.Arguments )
            end
            appendNextToken( ")" )

        elseif expr.AstType == 'TableCallExpr' then
            formatExpr( expr.Base )
            formatExpr( expr.Arguments[1] )

        elseif expr.AstType == 'StringCallExpr' then
            formatExpr(expr.Base)
            appendToken( expr.Arguments[1] )

        elseif expr.AstType == 'IndexExpr' then
            formatExpr(expr.Base)
            appendNextToken( "[" )
            formatExpr(expr.Index)
            appendNextToken( "]" )

        elseif expr.AstType == 'MemberExpr' then
            formatExpr(expr.Base)
            appendNextToken()  -- . or :
            appendToken(expr.Ident)

        elseif expr.AstType == 'Function' then
            -- anonymous function
            appendNextToken( "function" )
            appendNextToken( "(" )
            if #expr.Arguments > 0 then
                for i = 1, #expr.Arguments do
                    appendStr( expr.Arguments[i].Name )
                    if i ~= #expr.Arguments then
                        appendNextToken(",")
                    elseif expr.VarArg then
                        appendNextToken(",")
                        appendNextToken("...")
                    end
                end
            elseif expr.VarArg then
                appendNextToken("...")
            end
            appendNextToken(")")
            formatStatlist(expr.Body)
            appendNextToken("end")

        elseif expr.AstType == 'ConstructorExpr' then
            appendNextToken( "{" )
            for i = 1, #expr.EntryList do
                local entry = expr.EntryList[i]
                if entry.Type == 'Key' then
                    appendNextToken( "[" )
                    formatExpr(entry.Key)
                    appendNextToken( "]" )
                    appendNextToken( "=" )
                    formatExpr(entry.Value)
                elseif entry.Type == 'Value' then
                    formatExpr(entry.Value)
                elseif entry.Type == 'KeyString' then
                    appendStr(entry.Key)
                    appendNextToken( "=" )
                    formatExpr(entry.Value)
                end
                appendComma( i ~= #expr.EntryList, { ",", ";" } )
            end
            appendNextToken( "}" )

        elseif expr.AstType == 'Parentheses' then
            appendNextToken( "(" )
            formatExpr(expr.Inner)
            appendNextToken( ")" )

        else
            error(string.format("Unknown AST Type: %s\n", tostring(statement.AstType)))
        end

        assert(tok_it == #expr.Tokens + 1)
    end


    local formatStatement = function(statement)
        local tok_it = 1
        local function appendNextToken(str)
            local tok = statement.Tokens[tok_it];
            assert(tok, string.format("Not enough tokens for %q. First token at %i:%i",
                str, statement.Tokens[1].Line, statement.Tokens[1].Char))
            assert(tok.Data == str,
                string.format('Expected token %q, got %q', str, tok.Data))
            out:appendToken( tok )
            tok_it = tok_it + 1
        end
        local function appendToken(token)
            out:appendToken( str )
            tok_it = tok_it + 1
        end
        local function appendWhite()
            local tok = statement.Tokens[tok_it];
            out:appendWhite( tok )
            tok_it = tok_it + 1
        end
        local function appendStr(str)
            appendWhite()
            out:appendStr(str)
        end
        local function appendComma(mandatory)
            if mandatory
               or (tok_it < #statement.Tokens and statement.Tokens[tok_it].Data == ",") then
               appendNextToken( "," )
            end
        end

        if statement.AstType == 'AssignmentStatement' then
            for i,v in ipairs(statement.Lhs) do
                formatExpr(v)
                appendComma( i ~= #statement.Lhs )
            end
            if #statement.Rhs > 0 then
                appendNextToken( "=" )
                for i,v in ipairs(statement.Rhs) do
                    formatExpr(v)
                    appendComma( i ~= #statement.Rhs )
                end
            end

        elseif statement.AstType == 'CallStatement' then
            formatExpr(statement.Expression)

        elseif statement.AstType == 'LocalStatement' then
            appendNextToken( "local" )
            for i = 1, #statement.LocalList do
                appendStr( statement.LocalList[i].Name )
                appendComma( i ~= #statement.LocalList )
            end
            if #statement.InitList > 0 then
                appendNextToken( "=" )
                for i = 1, #statement.InitList do
                    formatExpr(statement.InitList[i])
                    appendComma( i ~= #statement.InitList )
                end
            end

        elseif statement.AstType == 'IfStatement' then
            appendNextToken( "if" )
            formatExpr( statement.Clauses[1].Condition )
            appendNextToken( "then" )
            formatStatlist( statement.Clauses[1].Body )
            for i = 2, #statement.Clauses do
                local st = statement.Clauses[i]
                if st.Condition then
                    appendNextToken( "elseif" )
                    formatExpr(st.Condition)
                    appendNextToken( "then" )
                else
                    appendNextToken( "else" )
                end
                formatStatlist(st.Body)
            end
            appendNextToken( "end" )

        elseif statement.AstType == 'WhileStatement' then
            appendNextToken( "while" )
            formatExpr(statement.Condition)
            appendNextToken( "do" )
            formatStatlist(statement.Body)
            appendNextToken( "end" )

        elseif statement.AstType == 'DoStatement' then
            appendNextToken( "do" )
            formatStatlist(statement.Body)
            appendNextToken( "end" )

        elseif statement.AstType == 'ReturnStatement' then
            appendNextToken( "return" )
            if statement.TrailingWhite then out:appendStr(' ') end
            for i = 1, #statement.Arguments do
                formatExpr(statement.Arguments[i])
                appendComma( i ~= #statement.Arguments )
            end

        elseif statement.AstType == 'BreakStatement' then
            appendNextToken( "break" )

        elseif statement.AstType == 'RepeatStatement' then
            appendNextToken( "repeat" )
            formatStatlist(statement.Body)
            appendNextToken( "until" )
            formatExpr(statement.Condition)

        elseif statement.AstType == 'Function' then
            if statement.IsLocal then
                appendNextToken( "local" )
            end
            appendNextToken( "function" )

            if statement.IsLocal then
                appendStr(statement.Name.Name)
            else
                formatExpr(statement.Name)
            end

            appendNextToken( "(" )
            if #statement.Arguments > 0 then
                for i = 1, #statement.Arguments do
                    appendStr( statement.Arguments[i].Name )
                    appendComma( i ~= #statement.Arguments or statement.VarArg )
                    if i == #statement.Arguments and statement.VarArg then
                        appendNextToken( "..." )
                    end
                end
            elseif statement.VarArg then
                appendNextToken( "..." )
            end
            appendNextToken( ")" )

            formatStatlist(statement.Body)
            appendNextToken( "end" )

        elseif statement.AstType == 'GenericForStatement' then
            appendNextToken( "for" )
            for i = 1, #statement.VariableList do
                appendStr( statement.VariableList[i].Name )
                appendComma( i ~= #statement.VariableList )
            end
            appendNextToken( "in" )
            for i = 1, #statement.Generators do
                formatExpr(statement.Generators[i])
                appendComma( i ~= #statement.Generators )
            end
            appendNextToken( "do" )
            formatStatlist(statement.Body)
            appendNextToken( "end" )

        elseif statement.AstType == 'NumericForStatement' then
            appendNextToken( "for" )
            appendStr( statement.Variable.Name )
            appendNextToken( "=" )
            formatExpr(statement.Start)
            appendNextToken( "," )
            formatExpr(statement.End)
            if statement.Step then
                appendNextToken( "," )
                formatExpr(statement.Step)
            end
            appendNextToken( "do" )
            formatStatlist(statement.Body)
            appendNextToken( "end" )

        elseif statement.AstType == 'LabelStatement' then
            appendNextToken( "::" )
            appendStr( statement.Label )
            appendNextToken( "::" )

        elseif statement.AstType == 'GotoStatement' then
            appendNextToken( "goto" )
            appendStr( statement.Label )

        elseif statement.AstType == 'Eof' then
            appendWhite()

        elseif statement.AstType == 'SemicolonStatement' then

        else
            error(string.format("Unknown AST Type: %s\n", tostring(statement.AstType)))
        end

        if statement.Semicolon then
            appendNextToken(";")
        end

        assert(tok_it == #statement.Tokens + 1)
    end

    formatStatlist = function(statList)
        for _, stat in ipairs(statList.Body) do
            formatStatement(stat)
        end
    end

    formatStatlist(ast)
    
    return table.concat(out.rope)
end

local dirsep = package.config:sub(1,1)
local dirl65 = (string.match(arg[0], "(.*[\\/]).*") or ''):gsub('/',dirsep)
local searchl65 = ''
if #dirl65 > 0 then
    searchl65 = string.format(";%s?;%s?.l65", dirl65, dirl65)
    package.path = package.path .. string.format(";%s?.lua", dirl65)
end
l65_def = {
    parse = ParseLua,
    format = Format65,
    searcher_index = 2,
    search_path = string.format(".%s?;.%s?.l65%s", dirsep, dirsep, searchl65),
    load_org = load,
    loadfile_org = loadfile,
    dofile_org = dofile,
}
if not l65 then l65 = l65_def else for k,v in pairs(l65_def) do l65[k]=v end end
l65.report = function(success, ...)
    if success then return success,... end
    local message=... io.stderr:write(tostring(message)..'\n')
    os.exit(-1)
end
l65.msghandler = function(msg)
    local o = function(s) io.stderr:write(s) end

    msg = tostring(msg)
    msg = msg:gsub('%[string "(.-%.l65)"%]', '%1') -- [string "xxx.l65"] -> xxx.l65
    local trace_cur = debug.traceback(nil, 2)
    trace_cur = trace_cur:gsub('%[string "(.-%.l65)"%]', '%1') -- [string "xxx.l65"] -> xxx.l65
    trace_cur = trace_cur:gsub('stack traceback:', '')

    local i=2
    while debug.getinfo(i) do
        local j = 1
        while true do
            local n,v = debug.getlocal(i, j)
            if not n then break end
            if n == 'l65dbg' then
                o(string.format("%s\n", msg))
                if trace_cur:find("in local 'late'") then
                    local lines = {}
                    for line in trace_cur:gmatch("[^\n]+") do
                        if line:find("in local 'late'") then break end
                        table.insert(lines, line)
                    end
                    o(table.concat(lines,'\n'))
                end
                local trace = v.trace:match(".-\n(.*)\n.-'xpcall'")
                trace = trace:gsub('%[string "(.-%.l65)"%]', '%1')
                trace = trace:gsub('stack traceback:', '')
                o(trace .. '\n')
                os.exit(-2)
            end
            j = j + 1
        end
        i = i + 1
    end

    trace_cur = trace_cur:match(".-\n(.*)\n.-'xpcall'")
    o(string.format("%s\n%s\n", msg, trace_cur))
    os.exit(-3)
end
do
    local getembedded = type(arg[-1]) == 'function' and arg[-1]
    l65.load_embedded = function(name)
        if not getembedded then return end
        local src,isl65 = getembedded(name)
        if not src then return end
        if isl65 then
            name = name .. '.l65'
            local st, ast = l65.report(l65.parse(src, name))
            src = l65.format(ast)
        else
            name = name .. '.lua'
        end
        local bc = assert(l65.load_org(src, name))
        return bc, name
    end
end
l65.searcher = function(name)
    local filename,err = package.searchpath(name, l65.search_path, '.', '.')
    if not filename then return err end
    local file = assert(io.open(filename, 'rb'))
    local src = file:read('*a')
    file:close()
    local st, ast = l65.report(l65.parse(src, filename))
    local bc = assert(l65.load_org(l65.format(ast), filename))
    return bc, filename
end
l65.load = function(chunk, chunkname, mode, ...)
    local chunk_t,s = type(chunk)
    if chunk_t == 'string' then s = chunk
    elseif chunk_t == 'function' then
        s={} for x in chunk do if #x==0 then break end s[#s+1]=x end
        s = table.concat(s)
    else return nil, string.format("invalid type for chunk %s: %s", chunkname or "=(load)", chunk_t)
    end
    if s:sub(1,4) == "\x1bLua" then -- a binary file
        return l65.load_org(s, chunkname, mode, ...)
    end
    local st, ast = l65.report(l65.parse(s, chunkname or "=(load)"))
    return l65.load_org(l65.format(ast), chunkname, 't', ...)
end
l65.loadfile = function(filename, mode, ...)
    local s
    if not filename then s = io.read('*a')
    else
        local file = io.open(filename, 'rb')
        if not file then return nil,"failed to open " .. filename .. " for reading" end
        s = file:read('*a')
        file:close()
    end
    return l65.load(s, filename, mode, ...)
end
l65.dofile = function(filename)
    local f = l65.report(l65.loadfile(filename))
    return f()
end
l65.installhooks = function()
    if package.searchers[l65.searcher_index] ~= l65.searcher then
        table.insert(package.searchers, l65.searcher_index, l65.searcher)
    end
    if not l65.hooks_installed then l65.hooks_installed = true
        load = l65.load
        loadfile = l65.loadfile
        dofile = l65.dofile
    end
end
l65.uninstallhooks = function()
    for k,v in ipairs(package.searchers) do
        if v == l65.searcher then table.remove(package.searchers, k) break end
    end
    if l65.hooks_installed then l65.hooks_installed = nil
        load = l65.load_org
        loadfile = l65.loadfile_org
        dofile = l65.dofile_org
    end
end
table.insert(package.searchers, l65.searcher_index, l65.load_embedded)
l65.installhooks()

function getopt(optstring, ...)
	local opts = { }
	local args = { ... }
	for optc, optv in optstring:gmatch"(%a)(:?)" do
		opts[optc] = { hasarg = optv == ":" }
	end
	return coroutine.wrap(function()
		local yield = coroutine.yield
		local i = 1
		while i <= #args do
			local arg,argi = args[i], i
			i = i + 1
			if arg == "--" then
				break
			elseif arg:sub(1, 1) == "-" then
				for j = 2, #arg do
					local opt = arg:sub(j, j)
					if opts[opt] then
						if opts[opt].hasarg then
							if j == #arg then
								if args[i] then yield(opt, args[i], argi) i=i+1
                                elseif optstring:sub(1, 1) == ":" then yield(':', opt, argi)
                                else yield('?', opt, argi) end
							else yield(opt, arg:sub(j + 1), argi) end
							break
						else yield(opt, false, argi) end
					else yield('?', opt, argi) end
				end
			else yield(false, arg, argi) end
		end
		for i = i, #args do yield(false, args[i], i) end
	end)
end

local cfg=require"l65cfg" l65.cfg=cfg
local version = function()
    print(string.format("l65 %s", cfg.version))
end
local usage = function(f)
    if not f then f = io.stdout end
    f:write(string.format([[
Usage: %s [options] file [args]
Options:
  -d <file>        Dump the Lua code after l65 parsing into file
  -h               Display this information
  -v               Display the release version
]], arg[0]))
end
local invalid_usage = function()
    io.stderr:write("Invalid usage.\n")
    usage(io.stderr)
end

local inf,dump,optix
for opt,arg,i in getopt("d:hv", ...) do
    if opt == '?' then return invalid_usage() end
    if opt == 'h' then return usage() end
    if opt == 'v' then return version() end
    if opt == 'd' then dump = arg end
    if opt == false then inf=arg optix=i+1 break end
end
if not inf then return invalid_usage() end
if dump then l65.format = function(ast)
    local s=Format65(ast) l65.format = Format65
    local f = assert(io.open(dump, 'wb')) f:write(s) f:close()
    return s
end end

local fn='' for i=#inf,1,-1 do local c=inf:sub(i,i) if c==dirsep or c=='/' then break end fn=c..fn if c=='.' then fn='' end end filename=fn
local f = l65.report(l65.loadfile(inf))
return xpcall(f, l65.msghandler, select(optix, ...))
