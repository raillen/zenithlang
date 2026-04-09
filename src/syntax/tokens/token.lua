-- ============================================================================
-- Zenith Compiler — Token
-- Estrutura de um token: tipo, lexema, valor, span.
-- ============================================================================

local Token = {}
Token.__index = Token

--- Cria um novo Token.
--- @param kind string TokenKind
--- @param lexeme string Texto bruto do token
--- @param value any Valor computado (número, string sem aspas, etc.)
--- @param span table Span no código-fonte
--- @return table
function Token.new(kind, lexeme, value, span)
    local self = setmetatable({}, Token)
    self.kind = kind
    self.lexeme = lexeme
    self.value = value
    self.span = span
    return self
end

--- Representação textual para debug.
--- @return string
function Token:__tostring()
    if self.value ~= nil then
        return string.format("Token(%s, %q, %s)",
            self.kind, self.lexeme, tostring(self.value))
    else
        return string.format("Token(%s, %q)", self.kind, self.lexeme)
    end
end

return Token
