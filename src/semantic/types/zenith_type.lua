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

    -- Por padrão, assumimos que é uma instância se for um tipo primitivo
    -- ou se for explicitamente marcado. Tipos puros (como Structs acessadas pelo nome) 
    -- devem marcar is_instance = false.
    if self.is_instance == nil then
        self.is_instance = true
    end
    
    return self
end

--- Cria uma união de tipos, achatando uniões aninhadas.
function ZenithType.create_union(types)
    local flattened = {}
    local seen = {}
    
    local function collect(t)
        if not t then return end
        if t.kind == ZenithType.Kind.UNION then
            for _, sub in ipairs(t.types) do collect(sub) end
        else
            local key = tostring(t)
            if not seen[key] then
                table.insert(flattened, t)
                seen[key] = true
            end
        end
    end
    
    for _, t in ipairs(types) do collect(t) end
    
    if #flattened == 0 then return ZenithType.new(ZenithType.Kind.ERROR, "<error>") end
    if #flattened == 1 then return flattened[1] end
    
    return ZenithType.new(ZenithType.Kind.UNION, "union", { types = flattened })
end

--- Verifica se este tipo é compatível com outro.
function ZenithType:is_assignable_to(other)
    if self.kind == ZenithType.Kind.ERROR or other.kind == ZenithType.Kind.ERROR then
        return true -- Evita cascata de erros
    end

    -- Se um dos lados for ANY, aceitamos (pragmatismo)
    if self.name == "any" or other.name == "any" then
        return true
    end

    if self == other then return true end

    -- Parâmetros genéricos: T só é compatível com T ou Any
    if self.kind == ZenithType.Kind.TYPE_PARAM and other.kind == ZenithType.Kind.TYPE_PARAM then
        return self.name == other.name
    end

    -- Um parâmetro genérico T pode ser atribuído a um Trait se ele tiver a restrição
    if self.kind == ZenithType.Kind.TYPE_PARAM and other.kind == ZenithType.Kind.TRAIT then
        if self.constraint and self.constraint:is_assignable_to(other) then
            return true
        end
    end

    if self.kind == ZenithType.Kind.TYPE_PARAM or other.kind == ZenithType.Kind.TYPE_PARAM then
        return false
    end

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

    -- Lógica de Funções: () => T -> () => T
    if self.kind == ZenithType.Kind.FUNC and other.kind == ZenithType.Kind.FUNC then
        -- Se um deles for ANY (sentinela), aceitamos
        if self.name == "any" or other.name == "any" then return true end

        -- Checar retorno (Covariante)
        local s_ret = self.return_type or BuiltinTypes.VOID
        local o_ret = other.return_type or BuiltinTypes.VOID
        
        if not s_ret:is_assignable_to(o_ret) then
            return false
        end
        -- Checar parâmetros (Contravariante)
        local s_params = self.params or {}
        local o_params = other.params or {}
        if #s_params ~= #o_params then return false end
        for i = 1, #s_params do
            local sp = s_params[i].type_info or s_params[i]
            local op = o_params[i].type_info or o_params[i]
            if not op:is_assignable_to(sp) then return false end
        end
        return true
    end

    -- Promoção automática: int -> float (Zenith é pragmático)
    local BuiltinTypes = require("src.semantic.types.builtin_types")
    if self == BuiltinTypes.INT and other == BuiltinTypes.FLOAT then
        return true
    end

    -- Fallback por nome para tipos primários e structs simples
    -- Se forem funções, a lógica estrutural acima já decidiu.
    if self.kind == ZenithType.Kind.FUNC and other.kind == ZenithType.Kind.FUNC then
        return false -- Se chegou aqui, falhou na lógica estrutural acima
    end

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
