-- ============================================================================
-- Zenith Compiler — Scope
-- Escopo léxico com tabela de símbolos e escopo pai.
-- ============================================================================

local Scope = {}
Scope.__index = Scope

--- Tipos de escopo.
Scope.Kind = {
    GLOBAL    = "GLOBAL",
    FUNCTION  = "FUNCTION",
    BLOCK     = "BLOCK",
    STRUCT    = "STRUCT",
    TRAIT     = "TRAIT",
    LOOP      = "LOOP",
    NAMESPACE = "NAMESPACE",
}

--- Cria um novo escopo.
--- @param kind string Scope.Kind
--- @param parent table|nil Escopo pai
--- @return table
function Scope.new(kind, parent)
    local self = setmetatable({}, Scope)
    self.kind = kind
    self.parent = parent
    self.symbols = {}          -- name → Symbol
    self.children = {}         -- lista de escopos filhos
    self.depth = parent and (parent.depth + 1) or 0

    if parent then
        table.insert(parent.children, self)
    end

    return self
end

--- Define um símbolo no escopo atual.
--- @param symbol table Symbol
--- @return boolean true se ok, false se já existe
function Scope:define(symbol)
    if self.symbols[symbol.name] then
        return false  -- redefinição
    end
    self.symbols[symbol.name] = symbol
    return true
end

--- Busca um símbolo pelo nome no escopo atual.
--- @param name string
--- @return table|nil Symbol
function Scope:lookup_local(name)
    return self.symbols[name]
end

--- Busca um símbolo pelo nome, subindo pela cadeia de escopos.
--- @param name string
--- @return table|nil Symbol
function Scope:lookup(name)
    local symbol = self.symbols[name]
    if symbol then
        return symbol
    end
    if self.parent then
        return self.parent:lookup(name)
    end
    return nil
end

--- Verifica se estamos dentro de um loop (para validar break/continue).
--- @return boolean
function Scope:is_in_loop()
    if self.kind == Scope.Kind.LOOP then
        return true
    end
    if self.parent then
        return self.parent:is_in_loop()
    end
    return false
end

--- Verifica se estamos dentro de uma função (para validar return).
--- @return boolean
function Scope:is_in_function()
    if self.kind == Scope.Kind.FUNCTION then
        return true
    end
    if self.parent then
        return self.parent:is_in_function()
    end
    return false
end

--- Verifica se estamos dentro de um struct ou trait (para validar self).
--- @return boolean
function Scope:is_in_struct_or_trait()
    if self.kind == Scope.Kind.STRUCT or self.kind == Scope.Kind.TRAIT then
        return true
    end
    if self.parent then
        return self.parent:is_in_struct_or_trait()
    end
    return false
end

--- Retorna o nome de todos os símbolos (para sugestões de "você quis dizer?").
--- @return table Lista de nomes
function Scope:all_names()
    local names = {}
    local s = self
    while s do
        for name, _ in pairs(s.symbols) do
            table.insert(names, name)
        end
        s = s.parent
    end
    return names
end

--- Representação textual.
function Scope:__tostring()
    local count = 0
    for _ in pairs(self.symbols) do count = count + 1 end
    return string.format("Scope(%s, depth=%d, symbols=%d)",
        self.kind, self.depth, count)
end

return Scope
