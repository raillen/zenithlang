-- Implementação Nativa da std.reflect para Zenith
local zt = require("src.backend.lua.runtime.zenith_rt")

local REFLECT = {}

function REFLECT.type_of(obj)
    if obj == nil then return "null" end
    local t = type(obj)
    if t == "number" then return "float" -- simplificado
    elseif t == "string" then return "text"
    elseif t == "boolean" then return "bool"
    elseif t == "table" then
        local mt = getmetatable(obj)
        if mt and mt._metadata then
            return mt._metadata.name
        end
        return "map"
    end
    return t
end

function REFLECT.inspect(obj)
    if type(obj) ~= "table" then return zt.Optional.Empty end
    local mt = getmetatable(obj)
    local meta = (mt and mt._metadata) or (obj._metadata)
    
    if not meta then return zt.Optional.Empty end

    local fields = {}
    for _, f in ipairs(meta.fields or {}) do
        table.insert(fields, { name = f.name, type_name = f.type })
    end

    local methods = {}
    for _, m_name in ipairs(meta.methods or {}) do
        table.insert(methods, { name = m_name })
    end

    return zt.Optional.Present({
        name = meta.name,
        fields = fields,
        methods = methods
    })
end

function REFLECT.get_value(obj, name)
    if type(obj) == "table" then
        return obj[name]
    end
    return nil
end

function REFLECT.set_value(obj, name, value)
    if type(obj) ~= "table" then
        return zt.Outcome.Failure("Objeto não é uma struct/mapa")
    end
    
    -- No futuro, validar tipo aqui usando meta.fields
    obj[name] = value
    return zt.Outcome.Success(nil)
end

return REFLECT
