-- PB8/PB16 encoders
-- Copyright 2018, 2019 Damian Yerrick
--
-- This Lua port is an altered version of the original Python encoders.
--
-- This software is provided 'as-is', without any express or implied
-- warranty. In no event will the authors be held liable for any damages
-- arising from the use of this software.
--
-- Permission is granted to anyone to use this software for any purpose,
-- including commercial applications, and to alter it and redistribute it
-- freely, subject to the following restrictions:
--
-- 1. The origin of this software must not be misrepresented; you must not
--    claim that you wrote the original software. If you use this software
--    in a product, an acknowledgment in the product documentation would be
--    appreciated but is not required.
-- 2. Altered source versions must be plainly marked as such, and must not be
--    misrepresented as being the original software.
-- 3. This notice may not be removed or altered from any source distribution.

local pb = {}

local function input_bytes(data)
    local bytes = {}
    if type(data) == "string" then
        for i = 1, #data do bytes[i] = string.byte(data, i) end
    else
        assert(type(data) == "table", "PB input must be a string or byte table")
        for i = 1, #data do bytes[i] = data[i] end
    end

    for i, value in ipairs(bytes) do
        assert(type(value) == "number" and value % 1 == 0
            and value >= 0 and value <= 0xff,
            "invalid PB input byte at index " .. i)
    end
    return bytes
end

local function read_file(filename)
    local file = assert(io.open(filename, "rb"))
    local data = assert(file:read("*all"))
    file:close()
    return data
end

-- Encode bytes as PB8. Each packet expands to eight bytes and starts with an
-- MSB-first control byte: 1 repeats the previous byte and 0 reads a literal.
function pb.pb8(data)
    local input = input_bytes(data)
    local output = {}
    local previous = 0

    for first = 1, #input, 8 do
        local chunk = {}
        for i = first, math.min(first + 7, #input) do
            chunk[#chunk + 1] = input[i]
        end
        while #chunk < 8 do chunk[#chunk + 1] = chunk[#chunk] end

        local control_index = #output + 1
        output[control_index] = 0
        for i, value in ipairs(chunk) do
            if value == previous then
                output[control_index] = output[control_index] | (0x80 >> (i - 1))
            else
                output[#output + 1] = value
                previous = value
            end
        end
    end
    return output
end

-- Encode bytes as PB16. This is PB8 over two interleaved streams, so a set
-- control bit repeats the byte two positions back. Partial packets preserve
-- the two streams and are padded to exactly eight uncompressed bytes.
function pb.pb16(data)
    local input = input_bytes(data)
    local output = {}
    local previous = { 0, 0 }

    for first = 1, #input, 8 do
        local chunk = {}
        for i = first, math.min(first + 7, #input) do
            chunk[#chunk + 1] = input[i]
        end

        if #chunk == 1 then
            chunk[2] = previous[2]
        elseif #chunk % 2 ~= 0 then
            chunk[#chunk + 1] = chunk[#chunk - 1]
        end
        local plane0, plane1 = chunk[#chunk - 1], chunk[#chunk]
        while #chunk < 8 do
            chunk[#chunk + 1] = plane0
            chunk[#chunk + 1] = plane1
        end

        local control_index = #output + 1
        output[control_index] = 0
        for i, value in ipairs(chunk) do
            local stream = ((i - 1) % 2) + 1
            if value == previous[stream] then
                output[control_index] = output[control_index] | (0x80 >> (i - 1))
            else
                output[#output + 1] = value
                previous[stream] = value
            end
        end
    end
    return output
end

function pb.pb8_file(filename)
    return pb.pb8(read_file(filename))
end

function pb.pb16_file(filename)
    return pb.pb16(read_file(filename))
end

return pb
