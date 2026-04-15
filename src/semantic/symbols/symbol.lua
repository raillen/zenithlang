-- ============================================================================
-- Zenith Compiler — Symbol
-- Representa um símbolo no modelo semântico: variável, função, struct, etc.
-- ============================================================================

local Symbol = {}
Symbol.__index = Symbol

--- Tipos de símbolo.
Symbol.Kind = {
    VARIABLE  = "VARIABLE",
    CONSTANT  = "CONSTANT",
    GLOBAL    = "GLOBAL",
    STATE     = "STATE",
    COMPUTED  = "COMPUTED",
    FUNCTION  = "FUNCTION",
    PARAMETER = "PARAMETER",
    STRUCT    = "STRUCT",
    ENUM      = "ENUM",
    TRAIT     = "TRAIT",
    MODULE    = "MODULE",
    NAMESPACE = "NAMESPACE",
    FIELD     = "FIELD",
    METHOD    = "METHOD",
    ENUM_MEMBER = "ENUM_MEMBER",
    FOR_VAR   = "FOR_VAR",       -- variável de loop
    RESCUE_VAR = "RESCUE_VAR",   -- variável de rescue
    ALIAS      = "ALIAS",        -- type Name = T ou union Name = A | B
    GENERIC_PARAM = "GENERIC_PARAM", -- Parâmetro genérico <T>
}

--- Cria um novo símbolo.
--- @param kind string Symbol.Kind
--- @param name string Nome do símbolo
--- @param data table Dados específicos do símbolo
--- @return table
function Symbol.new(kind, name, data)
    local self = setmetatable({}, Symbol)
    self.kind = kind
    self.name = name
    self.type_info = data and data.type_info or nil  -- ZenithType
    self.span = data and data.span or nil
    self.is_mutable = data and data.is_mutable or false
    self.is_pub = data and data.is_pub or false
    self.declaration = data and data.declaration or nil  -- nó AST original

    -- Dados específicos por tipo de símbolo
    if data then
        for k, v in pairs(data) do
            if not self[k] then
                self[k] = v
            end
        end
    end

    return self
end

--- Cria símbolo de variável.
function Symbol.variable(name, type_info, is_pub, span)
    return Symbol.new(Symbol.Kind.VARIABLE, name, {
        type_info = type_info,
        span = span,
        is_mutable = true,
        is_pub = is_pub or false,
    })
end

--- Cria símbolo de constante.
function Symbol.constant(name, type_info, is_pub, span)
    return Symbol.new(Symbol.Kind.CONSTANT, name, {
        type_info = type_info,
        span = span,
        is_mutable = false,
        is_pub = is_pub or false,
    })
end

--- Cria símbolo global.
function Symbol.global_var(name, type_info, is_pub, span)
    return Symbol.new(Symbol.Kind.GLOBAL, name, {
        type_info = type_info,
        span = span,
        is_mutable = true,
        is_pub = is_pub or false,
    })
end

--- Cria símbolo de parâmetro.
function Symbol.parameter(name, type_info, default_value, span)
    return Symbol.new(Symbol.Kind.PARAMETER, name, {
        type_info = type_info,
        default_value = default_value, -- expressão AST
        span = span,
        is_mutable = false,
    })
end

--- Cria símbolo de função.
function Symbol.func(name, params, return_type, span, is_async)
    local ZenithType = require("src.semantic.types.zenith_type")
    return Symbol.new(Symbol.Kind.FUNCTION, name, {
        params = params,           -- lista de { name, type_info }
        return_type = return_type,  -- ZenithType ou nil
        is_async = is_async or false,
        type_info = ZenithType.new(ZenithType.Kind.FUNC, name, { 
            return_type = return_type,
            params = params 
        }),
        span = span,
        is_mutable = false,
    })
end

function Symbol.method(name, params, return_type, span, is_async)
    local ZenithType = require("src.semantic.types.zenith_type")
    return Symbol.new(Symbol.Kind.METHOD, name, {
        params = params,
        return_type = return_type,
        is_async = is_async or false,
        type_info = ZenithType.new(ZenithType.Kind.FUNC, name, { 
            return_type = return_type,
            params = params 
        }),
        span = span,
        is_mutable = false,
    })
end

--- Cria símbolo de struct.
function Symbol.struct(name, fields, methods, span)
    return Symbol.new(Symbol.Kind.STRUCT, name, {
        fields = fields or {},
        methods = methods or {},
        implements = {}, -- lista de trait symbols
        span = span,
    })
end

--- Cria símbolo de campo.
function Symbol.field(name, type_info, is_pub, default_value, span)
    return Symbol.new(Symbol.Kind.FIELD, name, {
        type_info = type_info,
        is_pub = is_pub,
        default_value = default_value,
        span = span,
    })
end

--- Cria símbolo de enum.
function Symbol.enum(name, members, span)
    return Symbol.new(Symbol.Kind.ENUM, name, {
        members = members,
        span = span,
    })
end

--- Cria símbolo de membro de enum.
function Symbol.enum_member(name, parent_enum, params, span)
    return Symbol.new(Symbol.Kind.ENUM_MEMBER, name, {
        parent_enum = parent_enum, -- símbolo do Enum pai
        params = params,           -- lista de { name, type_info } (para Sum Types)
        span = span,
    })
end

--- Cria símbolo de trait.
function Symbol.trait(name, methods, span, generic_params)
    return Symbol.new(Symbol.Kind.TRAIT, name, {
        methods = methods,
        span = span,
        generic_params = generic_params or {},
    })
end

--- Cria símbolo de state.
function Symbol.state_var(name, type_info, span)
    return Symbol.new(Symbol.Kind.STATE, name, {
        type_info = type_info,
        span = span,
        is_mutable = true,
    })
end

--- Cria símbolo de computed.
function Symbol.computed(name, type_info, span)
    return Symbol.new(Symbol.Kind.COMPUTED, name, {
        type_info = type_info,
        span = span,
        is_mutable = false, 
    })
end

--- Cria símbolo de apelido de tipo (type/union).
function Symbol.alias(name, target_type, is_pub, span)
    return Symbol.new(Symbol.Kind.ALIAS, name, {
        type_info = target_type, -- O tipo para o qual ele aponta
        is_pub = is_pub or false,
        span = span,
    })
end

--- Cria símbolo de parâmetro genérico <T where T is Constraint>
function Symbol.generic_param(name, constraint, span)
    return Symbol.new(Symbol.Kind.GENERIC_PARAM, name, {
        type_info = nil, -- Será preenchido na instanciação
        constraint = constraint, -- ZenithType ou nil
        span = span,
    })
end

--- Cria símbolo de variável de loop.
function Symbol.for_var(name, type_info, span)
    return Symbol.new(Symbol.Kind.FOR_VAR, name, {
        type_info = type_info,
        span = span,
        is_mutable = false,
    })
end

--- Busca um membro (campo ou método) neste símbolo (se for STRUCT).
function Symbol:get_member(name)
    if self.kind == Symbol.Kind.MODULE then
        return self.members_scope and self.members_scope:lookup_local(name) or nil
    end

    if self.kind == Symbol.Kind.ENUM then
        if self.member_symbols then
            for _, m in ipairs(self.member_symbols) do
                if m.name == name then return m end
            end
        end
        return nil
    end

    if self.kind ~= Symbol.Kind.STRUCT and self.kind ~= Symbol.Kind.TRAIT then return nil end
    
    if self.members_scope then
        local sym = self.members_scope:lookup_local(name)
        if sym then return sym end
    end

    if self.fields then
        for _, f in ipairs(self.fields) do
            if f.name == name then return f end
        end
    end
    
    if self.methods or self.members then
        local members = self.methods or self.members
        for _, m in ipairs(members) do
            if m.name == name then return m end
        end
    end
    
    return nil
end

--- Verifica se o símbolo é mutável.
function Symbol:is_reassignable()
    return self.kind == Symbol.Kind.VARIABLE
        or self.kind == Symbol.Kind.GLOBAL
        or self.kind == Symbol.Kind.STATE
end

--- Representação textual.
function Symbol:__tostring()
    local t = self.type_info and tostring(self.type_info) or "?"
    return string.format("Symbol(%s, %s: %s)", self.kind, self.name, t)
end

--- Cria símbolo de alias (type Name = T).
function Symbol.alias(name, target_node, is_pub, span)
    return Symbol.new(Symbol.Kind.ALIAS, name, {
        target_node = target_node,
        is_pub = is_pub or false,
        span = span,
    })
end

--- Cria símbolo de parâmetro genérico (<T>).
function Symbol.generic_param(name, constraint, span)
    return Symbol.new(Symbol.Kind.GENERIC_PARAM, name, {
        constraint = constraint,
        span = span,
    })
end

return Symbol
