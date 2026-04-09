-- ============================================================================
-- Zenith Compiler — Zenith Type
-- Representação de tipos no modelo semântico.
-- ============================================================================

local ZenithType = {}
ZenithType.__index = ZenithType

 ZenithType.Kind = {
    PRIMITIVE = "PRIMITIVE", -- int, float, bool, text, void
    STRUCT    = "STRUCT",
    ENUM      = "ENUM",
    TRAIT     = "TRAIT",
    FUNC      = "FUNC",
    NULLABLE  = "NULLABLE",
    UNION     = "UNION",
    GENERIC   = "GENERIC",
    NULL      = "NULL",
    TYPE_PARAM = "TYPE_PARAM",
    ERROR     = "ERROR",     -- Tipo sentinela para erros de compilação
}

--- Cria um novo ZenithType.
function ZenithType.new(kind, name, data)
    local self = setmetatable({}, ZenithType)
    self.kind = kind
    self.name = name
    
    if data then
        for k, v in pairs(data) do
            self[k] = v
        end
    end
    
    return self
end

--- Verifica se este tipo é compatível com outro.
function ZenithType:is_assignable_to(other)
    if self.kind == ZenithType.Kind.ERROR or other.kind == ZenithType.Kind.ERROR then
        return true -- Evita cascata de erros
    end

    if self == other then return true end

    -- Lógica de Traits: Struct S -> Trait T
    if other.kind == ZenithType.Kind.TRAIT then
        if self.kind == ZenithType.Kind.STRUCT then
            local struct_sym = self.symbol
            if struct_sym and struct_sym.implements then
                for _, trait_sym in ipairs(struct_sym.implements) do
                    if trait_sym.name == other.name then return true end
                end
            end
        end
        return false
    end

    -- Lógica de Nullable: T -> T? ou null -> T?
    if other.kind == ZenithType.Kind.NULLABLE then
        if self.kind == ZenithType.Kind.NULL then return true end
        return self:is_assignable_to(other.base_type)
    end

    -- Lógica de União (Origem): A | B -> T
    if self.kind == ZenithType.Kind.UNION then
        for _, t in ipairs(self.types) do
            if not t:is_assignable_to(other) then return false end
        end
        return true
    end

    -- Lógica de União (Alvo): T -> A | B
    if other.kind == ZenithType.Kind.UNION then
        for _, t in ipairs(other.types) do
            if self:is_assignable_to(t) then return true end
        end
        return false
    end

    -- Lógica de Genéricos: list<int> -> list<int>
    if self.kind == ZenithType.Kind.GENERIC and other.kind == ZenithType.Kind.GENERIC then
        if self.base_name ~= other.base_name then return false end
        if #self.type_args ~= #other.type_args then return false end
        for i = 1, #self.type_args do
            -- Invariância por padrão para Zenith v1.0-alpha
            if not self.type_args[i]:is_assignable_to(other.type_args[i]) or
               not other.type_args[i]:is_assignable_to(self.type_args[i]) then
                return false
            end
        end
        return true
    end

    -- Promoção automática: int -> float (Zenith é pragmático)
    local BuiltinTypes = require("src.semantic.types.builtin_types")
    if self == BuiltinTypes.INT and other == BuiltinTypes.FLOAT then
        return true
    end

    -- Fallback por nome para tipos primários e structs simples
    return self.name == other.name and self.kind == other.kind
end

function ZenithType:__tostring()
    if self.kind == ZenithType.Kind.GENERIC then
        local args = {}
        for _, arg in ipairs(self.type_args or {}) do
            table.insert(args, tostring(arg))
        end
        return string.format("%s<%s>", self.base_name, table.concat(args, ", "))
    elseif self.kind == ZenithType.Kind.NULLABLE then
        return tostring(self.base_type) .. "?"
    elseif self.kind == ZenithType.Kind.UNION then
        local parts = {}
        for _, t in ipairs(self.types or {}) do
            table.insert(parts, tostring(t))
        end
        return table.concat(parts, " | ")
    end
    return self.name
end

return ZenithType
