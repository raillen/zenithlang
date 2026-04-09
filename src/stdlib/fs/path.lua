-- Implementação Nativa de std.fs.path
local PATH = {}

local sep = package.config:sub(1,1)

function PATH.join(parts)
    -- Zenith passa uma lista (tabela Lua)
    return table.concat(parts, sep)
end

function PATH.extension(p)
    return p:match("^.+(%.[^%.]+)$") or ""
end

function PATH.basename(p)
    return p:match("^.+" .. sep .. "(.+)$") or p:match("^(.+)$") or ""
end

function PATH.dirname(p)
    return p:match("^(.*)" .. sep) or "."
end

function PATH.is_absolute(p)
    if sep == "\\" then
        return p:match("^%a:\\") ~= nil
    else
        return p:sub(1,1) == "/"
    end
end

function PATH.normalize(p)
    -- Versão simplificada
    p = p:gsub("/", sep):gsub("\\", sep)
    return p
end

return PATH
