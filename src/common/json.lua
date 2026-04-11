-- zenith-json (Ponte rítmica para v0.3.x)
local json = {}

local function escape_str(s)
    local names = {
        ['"']  = '\\"', ['\\'] = '\\\\', ['\b'] = '\\b',
        ['\f'] = '\\f', ['\n'] = '\\n', ['\r'] = '\\r',
        ['\t'] = '\\t'
    }
    return s:gsub('["\\\b\f\n\r\t]', names):gsub('[\0-\31]', function(c)
        return string.format('\\u%04x', string.byte(c))
    end)
end

function json.encode(val)
    local t = type(val)
    if t == "nil" then return "null"
    elseif t == "boolean" then return tostring(val)
    elseif t == "number" then return tostring(val)
    elseif t == "string" then return '"' .. escape_str(val) .. '"'
    elseif t == "table" then
        if val[0] ~= nil or val[1] ~= nil then
            local res = {}
            for i=0, #val do res[i+1] = json.encode(val[i]) end
            -- Ajuste para arrays base-0 do Zenith
            if #res == 0 and val[0] == nil then return "[]" end
            return "[" .. table.concat(res, ",") .. "]"
        else
            local res = {}
            for k, v in pairs(val) do table.insert(res, json.encode(tostring(k)) .. ":" .. json.encode(v)) end
            return "{" .. table.concat(res, ",") .. "}"
        end
    else
        return "null"
    end
end

-- Parser simplificado para o bootstrap
function json.decode(s)
    -- Para o patch v0.3.x, usaremos um parser rítmico via load string seguro
    -- ou uma regex controlada.
    local function parse_val(str, pos)
        str = str:sub(pos)
        local c = str:sub(1,1)
        if c == '"' then
            local val = str:match('^"([^"]*)"')
            return val, pos + #val + 2
        elseif c == "{" then
            local res = {}
            pos = pos + 1
            while true do
                local k, v
                k, pos = parse_val(s, pos)
                pos = s:find(":", pos) + 1
                v, pos = parse_val(s, pos)
                res[k] = v
                pos = s:find("[,}]", pos)
                if s:sub(pos,pos) == "}" then break end
                pos = pos + 1
            end
            return res, pos + 1
        elseif c == "[" then
            local res = {}
            pos = pos + 1
            local i = 0
            while true do
                local v
                v, pos = parse_val(s, pos)
                res[i] = v
                i = i + 1
                pos = s:find("[,%]]", pos)
                if s:sub(pos,pos) == "]" then break end
                pos = pos + 1
            end
            return res, pos + 1
        elseif str:match("^true") then return true, pos + 4
        elseif str:match("^false") then return false, pos + 5
        elseif str:match("^null") then return nil, pos + 4
        else
            local n = str:match("^[%d%.%-]+")
            return tonumber(n), pos + #n
        end
    end
    
    local ok, res = pcall(function() return parse_val(s:gsub("%s+", ""), 1) end)
    return res
end

return json
