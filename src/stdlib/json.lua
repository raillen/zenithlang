-- Implementação Nativa da std.json para Zenith
local zt = require("src.backend.lua.runtime.zenith_rt")

local JSON = {}

-- ----------------------------------------------------------------------------
-- SERIALIZAÇÃO (Stringify)
-- ----------------------------------------------------------------------------

local function escape_str(s)
    local esc = { ["\""] = "\\\"", ["\\"] = "\\\\", ["\b"] = "\\b", ["\f"] = "\\f", ["\n"] = "\\n", ["\r"] = "\\r", ["\t"] = "\\t" }
    return s:gsub("[\"\\\b\f\n\r\t]", esc)
end

function JSON.stringify(val, indent_level)
    indent_level = indent_level or 0
    local current_indent = 0
    
    local function get_pad(depth)
        if indent_level == 0 then return "" end
        local char = (indent_level < 0) and "\t" or string.rep(" ", math.abs(indent_level))
        return "\n" .. string.rep(char, depth)
    end

    local function serialize(v, depth)
        local t = type(v)
        if t == "string" then return "\"" .. escape_str(v) .. "\""
        elseif t == "number" or t == "boolean" then return tostring(v)
        elseif t == "nil" then return "null"
        elseif t == "table" then
            -- Heurística: se tem _tag, é ADT (Optional/Outcome)
            if v._tag == "Present" or v._tag == "Success" then
                return serialize(v.value, depth)
            elseif v._tag == "Empty" or v._tag == "Failure" then
                return "null"
            end

            -- Verifica se é lista (array) ou mapa (object)
            local is_array = #v > 0
            local parts = {}
            if is_array then
                for _, item in ipairs(v) do
                    table.insert(parts, serialize(item, depth + 1))
                end
                local pad = get_pad(depth + 1)
                local end_pad = get_pad(depth)
                return "[" .. pad .. table.concat(parts, "," .. pad) .. end_pad .. "]"
            else
                for k, item in pairs(v) do
                    if type(k) == "string" then
                        table.insert(parts, "\"" .. escape_str(k) .. "\":" .. (indent_level ~= 0 and " " or "") .. serialize(item, depth + 1))
                    end
                end
                local pad = get_pad(depth + 1)
                local end_pad = get_pad(depth)
                return "{" .. pad .. table.concat(parts, "," .. pad) .. end_pad .. "}"
            end
        end
        return "null"
    end

    return serialize(val, 0)
end

-- ----------------------------------------------------------------------------
-- PARSING (Parse)
-- ----------------------------------------------------------------------------

function JSON.parse(str)
    local pos = 1
    
    local function skip_whitespace()
        while pos <= #str and str:sub(pos,pos):match("%s") do pos = pos + 1 end
    end

    local function parse_val()
        skip_whitespace()
        local char = str:sub(pos, pos)
        
        if char == "{" then return parse_obj()
        elseif char == "[" then return parse_arr()
        elseif char == "\"" then return parse_str()
        elseif char:match("[%d%-]") then return parse_num()
        elseif str:sub(pos, pos+3) == "true" then pos = pos + 4; return true
        elseif str:sub(pos, pos+4) == "false" then pos = pos + 5; return false
        elseif str:sub(pos, pos+3) == "null" then pos = pos + 4; return nil
        end
        error("Caractere inesperado '" .. char .. "' na posição " .. pos)
    end

    function parse_str()
        pos = pos + 1 -- "
        local start = pos
        while pos <= #str and str:sub(pos,pos) ~= "\"" do
            if str:sub(pos,pos) == "\\" then pos = pos + 1 end
            pos = pos + 1
        end
        local s = str:sub(start, pos - 1)
        pos = pos + 1 -- "
        -- Unescape simples
        return s:gsub("\\n", "\n"):gsub("\\\"", "\""):gsub("\\\\", "\\")
    end

    function parse_num()
        local start = pos
        while pos <= #str and str:sub(pos,pos):match("[%d%.eE%-]") do pos = pos + 1 end
        local n_str = str:sub(start, pos - 1)
        return tonumber(n_str)
    end

    function parse_obj()
        pos = pos + 1 -- {
        local obj = {}
        skip_whitespace()
        if str:sub(pos, pos) == "}" then pos = pos + 1; return obj end
        
        repeat
            skip_whitespace()
            local key = parse_str()
            skip_whitespace()
            if str:sub(pos, pos) ~= ":" then error("Esperado ':' na posição " .. pos) end
            pos = pos + 1
            obj[key] = parse_val()
            skip_whitespace()
            local sep = str:sub(pos, pos)
            if sep == "," then pos = pos + 1 end
        until sep == "}" or pos > #str
        
        pos = pos + 1
        return obj
    end

    function parse_arr()
        pos = pos + 1 -- [
        local arr = {}
        skip_whitespace()
        if str:sub(pos, pos) == "]" then pos = pos + 1; return arr end
        
        repeat
            table.insert(arr, parse_val())
            skip_whitespace()
            local sep = str:sub(pos, pos)
            if sep == "," then pos = pos + 1 end
        until sep == "]" or pos > #str
        
        pos = pos + 1
        return arr
    end

    local ok, res = pcall(parse_val)
    if ok then return zt.Outcome.Success(res) else return zt.Outcome.Failure(res) end
end

-- ----------------------------------------------------------------------------
-- EXTRAS
-- ----------------------------------------------------------------------------

function JSON.decode(str)
    -- Por enquanto, decode genérico. Mapeamento p/ Struct requer suporte do compilador.
    return JSON.parse(str)
end

function JSON.minify(str)
    local res = JSON.parse(str)
    if res._tag == "Failure" then return res end
    return zt.Outcome.Success(JSON.stringify(res.value, 0))
end

function JSON.beautify(str, indent)
    local res = JSON.parse(str)
    if res._tag == "Failure" then return res end
    return zt.Outcome.Success(JSON.stringify(res.value, indent or 4))
end

function JSON.read_file(path)
    local fs = require("src/stdlib/fs")
    local content_res = fs.read_text_file(path)
    if content_res._tag == "Failure" then return content_res end
    return JSON.parse(content_res.value)
end

function JSON.write_file(path, val, indent)
    local fs = require("src/stdlib/fs")
    local json_str = JSON.stringify(val, indent)
    return fs.write_text_file(path, json_str)
end

return JSON
