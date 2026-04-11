-- Implementação Nativa da std.text para Zenith
local zt = require("src.backend.lua.runtime.zenith_rt")

local TEXT = {}

-- 📂 TABELAS DE SUPORTE
local accent_map = {
    ["á"]="a", ["à"]="a", ["â"]="a", ["ã"]="a", ["ä"]="a",
    ["é"]="e", ["è"]="e", ["ê"]="e", ["ë"]="e",
    ["í"]="i", ["ì"]="i", ["î"]="i", ["ï"]="i",
    ["ó"]="o", ["ò"]="o", ["ô"]="o", ["õ"]="o", ["ö"]="o",
    ["ú"]="u", ["ù"]="u", ["û"]="u", ["ü"]="u",
    ["ç"]="c", ["ñ"]="n",
    ["Á"]="A", ["À"]="A", ["Â"]="A", ["Ã"]="A", ["Ä"]="A",
    ["É"]="E", ["È"]="E", ["Ê"]="E", ["Ë"]="E",
    ["Í"]="I", ["Ì"]="I", ["Î"]="I", ["Ï"]="I",
    ["Ó"]="O", ["Ò"]="O", ["Ô"]="O", ["Õ"]="O", ["Ö"]="O",
    ["Ú"]="U", ["Ù"]="U", ["Û"]="U", ["Ü"]="U",
    ["Ç"]="C", ["Ñ"]="N"
}

-- 🔡 TRANSFORMAÇÃO

function TEXT.to_upper(t) return t:upper() end -- Lua: ok p/ ASCII, limitado p/ UTF8
function TEXT.to_lower(t) return t:lower() end

function TEXT.trim(t)
    return t:gsub("^%s*(.-)%s*$", "%1")
end

function TEXT.capitalize(t)
    if #t == 0 then return t end
    -- Pega o primeiro char (podendo ser multibyte)
    local first, rest = t:match("^([%z\1-\127\194-\244][\128-\191]*)(.*)$")
    return (first or ""):upper() .. (rest or "")
end

function TEXT.reverse(t)
    local runes = {}
    for rune in t:gmatch("[%z\1-\127\194-\244][\128-\191]*") do
        table.insert(runes, 1, rune)
    end
    return table.concat(runes)
end

function TEXT.clean_accents(t)
    local res = t:gsub("[%z\1-\127\194-\244][\128-\191]*", function(c)
        return accent_map[c] or c
    end)
    return res
end

function TEXT.slugify(t)
    local s = TEXT.clean_accents(t):lower()
    s = s:gsub("[%s%p]+", "-") -- troca espaços e pontuação por hifen
    s = s:gsub("^%-+", ""):gsub("%-+$", "") -- remove hifens das pontas
    return s
end

function TEXT.to_snake_case(t)
    local s = t:gsub("(%l)(%u)", "%1_%2"):gsub("[%s%-]+", "_"):lower()
    return s
end

function TEXT.to_camel_case(t)
    local s = TEXT.to_snake_case(t)
    return s:gsub("(_%l)", function(v) return v:sub(2):upper() end)
end

-- 🔎 BUSCA E PREDICADOS

function TEXT.count_chars(t)
    local _, count = t:gsub("[%z\1-\127\194-\244][\128-\191]*", "")
    return count
end

function TEXT.contains(t, search) return t:find(search, 1, true) ~= nil end
function TEXT.starts_with(t, prefix) return t:sub(1, #prefix) == prefix end
function TEXT.ends_with(t, suffix) return t:sub(-#suffix) == suffix end

function TEXT.is_numeric(t) return t:match("^%d+$") ~= nil end

function TEXT.is_emoji(t)
    -- Heurística simples: caracteres de 4 bytes costumam ser emojis
    for rune in t:gmatch("[\240-\244][\128-\191]*") do
        return true
    end
    return false
end

-- 🔪 DIVISÃO E UNIÃO

function TEXT.split(t, sep)
    local parts = {}
    local pattern = string.format("([^%s]+)", sep)
    t:gsub(pattern, function(c) table.insert(parts, c) end)
    return parts
end

function TEXT.join(parts, sep)
    return table.concat(parts, sep)
end

function TEXT.lines(t)
    local l = {}
    for line in t:gmatch("([^\n\r]+)") do table.insert(l, line) end
    return l
end

function TEXT.slice(t, start_idx, end_idx)
    return t:sub(start_idx or 1, end_idx or -1)
end

-- 🛡️ SEGURANÇA E UI

function TEXT.mask(t, v_start, v_end, char)
    char = char or "*"
    local len = TEXT.count_chars(t)
    if len <= (v_start + v_end) then return t end
    
    local runes = {}
    for rune in t:gmatch("[%z\1-\127\194-\244][\128-\191]*") do table.insert(runes, rune) end
    
    local res = ""
    for i=1, #runes do
        if i <= v_start or i > (#runes - v_end) then
            res = res .. runes[i]
        else
            res = res .. char
        end
    end
    return res
end

function TEXT.truncate(t, max_len, suffix)
    suffix = suffix or "..."
    local len = TEXT.count_chars(t)
    if len <= max_len then return t end
    
    local count = 0
    local res = ""
    for rune in t:gmatch("[%z\1-\127\194-\244][\128-\191]*") do
        res = res .. rune
        count = count + 1
        if count >= max_len then break end
    end
    return res .. suffix
end

function TEXT.format(fmt, args)
    if type(args) == "table" then
        return string.format(fmt, unpack(args))
    else
        return string.format(fmt, args)
    end
end

return TEXT
