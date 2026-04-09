-- ============================================================================
-- Zenith Compiler — Parser Context
-- Estado compartilhado do parser: tokens, posição, diagnósticos.
-- ============================================================================

local TokenKind     = require("src.syntax.tokens.token_kind")
local Span          = require("src.source.span")
local DiagnosticBag = require("src.diagnostics.diagnostic_bag")

local ParserContext = {}
ParserContext.__index = ParserContext

--- Cria um novo contexto de parser.
--- @param tokens table Lista de tokens do lexer
--- @param source_text table SourceText original
--- @return table
function ParserContext.new(tokens, source_text)
    local self = setmetatable({}, ParserContext)
    self.tokens = tokens
    self.source = source_text
    self.pos = 1
    self.diagnostics = DiagnosticBag.new()
    return self
end

--- Retorna o token atual sem avançar.
--- @return table Token
function ParserContext:current()
    if self.pos > #self.tokens then
        return self.tokens[#self.tokens]  -- EOF
    end
    return self.tokens[self.pos]
end

--- Retorna o token N posições à frente (sem avançar).
--- @param offset number
--- @return table Token
function ParserContext:peek(offset)
    local p = self.pos + (offset or 0)
    if p > #self.tokens then
        return self.tokens[#self.tokens]
    end
    if p < 1 then
        return self.tokens[1]
    end
    return self.tokens[p]
end

--- Retorna o tipo do token atual.
--- @return string TokenKind
function ParserContext:current_kind()
    return self:current().kind
end

--- Verifica se o token atual é do tipo fornecido.
--- @param kind string TokenKind
--- @return boolean
function ParserContext:check(kind)
    return self:current_kind() == kind
end

--- Verifica se o token atual é algum dos tipos fornecidos.
--- @param ... string TokenKinds
--- @return boolean
function ParserContext:check_any(...)
    local current = self:current_kind()
    for _, kind in ipairs({...}) do
        if current == kind then return true end
    end
    return false
end

--- Avança e retorna o token anterior.
--- @return table Token (o token consumido)
function ParserContext:advance()
    local token = self:current()
    if self.pos <= #self.tokens then
        self.pos = self.pos + 1
    end
    return token
end

--- Retrocede o cursor do parser.
--- @param count number
function ParserContext:rewind(count)
    self.pos = self.pos - (count or 1)
    if self.pos < 1 then self.pos = 1 end
end

--- Consome um token do tipo esperado. Se não for o esperado, gera erro.
--- @param kind string TokenKind esperado
--- @param error_msg string Mensagem de erro se não encontrar
--- @return table Token
function ParserContext:expect(kind, error_msg)
    if self:check(kind) then
        return self:advance()
    end

    -- Erro: token inesperado
    local current = self:current()
    self.diagnostics:report_error(
        "ZT-P001",
        error_msg or string.format(
            "esperado '%s', encontrado '%s'",
            kind, current.kind
        ),
        current.span,
        string.format("token encontrado: '%s'", current.lexeme)
    )

    -- Retorna um token "fantasma" para continuar parsing
    return {
        kind = kind,
        lexeme = "",
        value = nil,
        span = current.span,
    }
end

--- Consome o token se for do tipo esperado. Retorna true/false.
--- @param kind string TokenKind
--- @return boolean
function ParserContext:match(kind)
    if self:check(kind) then
        self:advance()
        return true
    end
    return false
end

--- Verifica se atingiu o fim dos tokens.
--- @return boolean
function ParserContext:is_at_end()
    return self:current_kind() == TokenKind.EOF
end

--- Pula tokens NEWLINE ou SEMICOLON (separadores de statement).
function ParserContext:skip_newlines()
    while self:check(TokenKind.NEWLINE) or self:check(TokenKind.SEMICOLON) do
        self:advance()
    end
end

--- Pula tokens NEWLINE opcionais (um ou mais se houver).
function ParserContext:skip_optional_newlines()
    self:skip_newlines()
end

--- Espera pelo menos um NEWLINE ou EOF (terminador de statement).
--- Se não encontrar, reporta erro mas continua.
function ParserContext:expect_newline_or_eof()
    if self:check(TokenKind.NEWLINE) or self:check(TokenKind.EOF) then
        self:skip_newlines()
        return
    end
    -- Pode estar no mesmo line — reportar aviso
    -- (tolerante: não bloqueia o parsing)
end

--- Cria um span que vai do início de start_token ao final de end_token.
--- @param start_token table Token
--- @param end_token table Token
--- @return table Span
function ParserContext:span_from(start_token, end_token)
    if not end_token then
        end_token = self.tokens[self.pos - 1] or start_token
    end
    return start_token.span:merge(end_token.span)
end

--- Sincroniza após erro: avança até encontrar ponto seguro.
function ParserContext:synchronize()
    while not self:is_at_end() do
        local kind = self:current_kind()

        -- Pontos de sincronização
        if kind == TokenKind.KW_END
            or kind == TokenKind.KW_FUNC
            or kind == TokenKind.KW_STRUCT
            or kind == TokenKind.KW_TRAIT
            or kind == TokenKind.KW_ENUM
            or kind == TokenKind.KW_VAR
            or kind == TokenKind.KW_CONST
            or kind == TokenKind.KW_IF
            or kind == TokenKind.KW_WHILE
            or kind == TokenKind.KW_FOR
            or kind == TokenKind.KW_RETURN
            or kind == TokenKind.KW_IMPORT
            or kind == TokenKind.KW_EXPORT then
            return
        end

        self:advance()
    end
end

return ParserContext
