-- ============================================================================
-- Zenith Compiler — Operator Table
-- Define operadores, precedência e associatividade.
-- ============================================================================

local TokenKind = require("src.syntax.tokens.token_kind")

local OperatorTable = {}

--- Associatividade.
OperatorTable.Assoc = {
    LEFT  = "LEFT",
    RIGHT = "RIGHT",
    NONE  = "NONE",
}

--- Precedências (menor número = menor precedência = liga mais fraco).
--- Baseado na tabela clássica com ajustes para Zenith.
OperatorTable.Precedence = {
    NONE        = 0,
    ASSIGNMENT  = 1,   -- = += -= *= /=
    OR          = 2,   -- or
    AND         = 3,   -- and
    EQUALITY    = 4,   -- == !=
    COMPARISON  = 5,   -- < <= > >=
    RANGE       = 6,   -- ..
    ADDITION    = 7,   -- + -
    MULTIPLY    = 8,   -- * / %
    POWER       = 9,   -- ^
    UNARY       = 10,  -- not - (prefixo)
    CALL        = 11,  -- () [] .
    POSTFIX     = 12,  -- ! ?
}

--- Tabela de operadores binários.
--- @type table<string, table>
OperatorTable.binary = {
    -- Aritméticos
    [TokenKind.PLUS]          = { prec = OperatorTable.Precedence.ADDITION,   assoc = OperatorTable.Assoc.LEFT },
    [TokenKind.MINUS]         = { prec = OperatorTable.Precedence.ADDITION,   assoc = OperatorTable.Assoc.LEFT },
    [TokenKind.STAR]          = { prec = OperatorTable.Precedence.MULTIPLY,   assoc = OperatorTable.Assoc.LEFT },
    [TokenKind.SLASH]         = { prec = OperatorTable.Precedence.MULTIPLY,   assoc = OperatorTable.Assoc.LEFT },
    [TokenKind.PERCENT]       = { prec = OperatorTable.Precedence.MULTIPLY,   assoc = OperatorTable.Assoc.LEFT },
    [TokenKind.CARET]         = { prec = OperatorTable.Precedence.POWER,      assoc = OperatorTable.Assoc.RIGHT },

    -- Comparação
    [TokenKind.EQUAL_EQUAL]   = { prec = OperatorTable.Precedence.EQUALITY,   assoc = OperatorTable.Assoc.NONE },
    [TokenKind.BANG_EQUAL]    = { prec = OperatorTable.Precedence.EQUALITY,   assoc = OperatorTable.Assoc.NONE },
    [TokenKind.LESS]          = { prec = OperatorTable.Precedence.COMPARISON, assoc = OperatorTable.Assoc.NONE },
    [TokenKind.LESS_EQUAL]    = { prec = OperatorTable.Precedence.COMPARISON, assoc = OperatorTable.Assoc.NONE },
    [TokenKind.GREATER]       = { prec = OperatorTable.Precedence.COMPARISON, assoc = OperatorTable.Assoc.NONE },
    [TokenKind.GREATER_EQUAL] = { prec = OperatorTable.Precedence.COMPARISON, assoc = OperatorTable.Assoc.NONE },

    -- Lógicos
    [TokenKind.KW_AND]        = { prec = OperatorTable.Precedence.AND,        assoc = OperatorTable.Assoc.LEFT },
    [TokenKind.KW_OR]         = { prec = OperatorTable.Precedence.OR,         assoc = OperatorTable.Assoc.LEFT },
    [TokenKind.PIPE]          = { prec = OperatorTable.Precedence.OR,         assoc = OperatorTable.Assoc.LEFT },

    -- Range
    [TokenKind.DOT_DOT]       = { prec = OperatorTable.Precedence.RANGE,      assoc = OperatorTable.Assoc.NONE },

    -- Acesso e Chamada (CALL)
    [TokenKind.DOT]           = { prec = OperatorTable.Precedence.CALL,       assoc = OperatorTable.Assoc.LEFT },
    [TokenKind.QUESTION_DOT]  = { prec = OperatorTable.Precedence.CALL,       assoc = OperatorTable.Assoc.LEFT },
    [TokenKind.LPAREN]        = { prec = OperatorTable.Precedence.CALL,       assoc = OperatorTable.Assoc.LEFT },
    [TokenKind.LBRACKET]      = { prec = OperatorTable.Precedence.CALL,       assoc = OperatorTable.Assoc.LEFT },
    [TokenKind.LBRACE]        = { prec = OperatorTable.Precedence.CALL,       assoc = OperatorTable.Assoc.LEFT },

    -- Tipos (is, as)
    [TokenKind.KW_IS]         = { prec = OperatorTable.Precedence.COMPARISON, assoc = OperatorTable.Assoc.NONE },
    [TokenKind.KW_AS]         = { prec = 9,                                   assoc = OperatorTable.Assoc.LEFT },
}

--- Tabela de operadores unários prefixo.
OperatorTable.unary_prefix = {
    [TokenKind.MINUS]   = { prec = OperatorTable.Precedence.UNARY },
    [TokenKind.KW_NOT]  = { prec = OperatorTable.Precedence.UNARY },
    [TokenKind.KW_CHECK] = { prec = OperatorTable.Precedence.UNARY },
    [TokenKind.KW_AWAIT] = { prec = OperatorTable.Precedence.UNARY },
    [TokenKind.DOT_DOT]  = { prec = OperatorTable.Precedence.UNARY }, -- rest/spread
    [TokenKind.HASH]     = { prec = OperatorTable.Precedence.UNARY }, -- length
}

--- Tabela de operadores unários pós-fixo.
OperatorTable.unary_postfix = {
    [TokenKind.BANG]     = { prec = OperatorTable.Precedence.POSTFIX },
    [TokenKind.QUESTION] = { prec = OperatorTable.Precedence.POSTFIX },
}

--- Tabela de operadores de atribuição composta.
OperatorTable.compound_assignment = {
    [TokenKind.PLUS_EQUAL]  = TokenKind.PLUS,
    [TokenKind.MINUS_EQUAL] = TokenKind.MINUS,
    [TokenKind.STAR_EQUAL]  = TokenKind.STAR,
    [TokenKind.SLASH_EQUAL] = TokenKind.SLASH,
}

--- Retorna a precedência de um operador binário.
--- @param kind string TokenKind
--- @return number Precedência (0 se não for operador binário)
function OperatorTable.binary_precedence(kind)
    local entry = OperatorTable.binary[kind]
    return entry and entry.prec or OperatorTable.Precedence.NONE
end

--- Retorna a associatividade de um operador binário.
--- @param kind string TokenKind
--- @return string Associatividade
function OperatorTable.binary_assoc(kind)
    local entry = OperatorTable.binary[kind]
    return entry and entry.assoc or OperatorTable.Assoc.NONE
end

--- Verifica se um token é operador unário prefixo.
--- @param kind string TokenKind
--- @return boolean
function OperatorTable.is_unary_prefix(kind)
    return OperatorTable.unary_prefix[kind] ~= nil
end

--- Verifica se um token é operador unário pós-fixo.
--- @param kind string TokenKind
--- @return boolean
function OperatorTable.is_unary_postfix(kind)
    return OperatorTable.unary_postfix[kind] ~= nil
end

--- Verifica se um token é atribuição composta.
--- @param kind string TokenKind
--- @return boolean
function OperatorTable.is_compound_assignment(kind)
    return OperatorTable.compound_assignment[kind] ~= nil
end

return OperatorTable
