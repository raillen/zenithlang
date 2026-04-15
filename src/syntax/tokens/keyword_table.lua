-- ============================================================================
-- Zenith Compiler â€” Keyword Table
-- Mapa de strings para TokenKind para palavras-chave.
-- ============================================================================

local TokenKind = require("src.syntax.tokens.token_kind")

local KeywordTable = {}

--- Mapa de keyword â†’ TokenKind.
KeywordTable.keywords = {
    -- DeclaraÃ§Ãµes de variÃ¡veis
    ["var"]      = TokenKind.KW_VAR,
    ["const"]    = TokenKind.KW_CONST,
    ["global"]   = TokenKind.KW_GLOBAL,
    ["state"]    = TokenKind.KW_STATE,
    ["computed"] = TokenKind.KW_COMPUTED,
    ["func"]     = TokenKind.KW_FUNC,
    ["async"]    = TokenKind.KW_ASYNC,
    ["await"]    = TokenKind.KW_AWAIT,
    ["pub"]      = TokenKind.KW_PUB,

    -- Tipos e Estruturas
    ["struct"]   = TokenKind.KW_STRUCT,
    ["enum"]      = TokenKind.KW_ENUM,
    ["trait"]     = TokenKind.KW_TRAIT,
    ["apply"]     = TokenKind.KW_APPLY,
    ["type"]      = TokenKind.KW_TYPE,
    ["union"]     = TokenKind.KW_UNION,
    ["namespace"] = TokenKind.KW_NAMESPACE,
    ["redo"]      = TokenKind.KW_REDO,

    -- Fluxo
    ["if"]       = TokenKind.KW_IF,
    ["elif"]     = TokenKind.KW_ELIF,
    ["else"]     = TokenKind.KW_ELSE,
    ["end"]      = TokenKind.KW_END,
    ["while"]    = TokenKind.KW_WHILE,
    ["for"]      = TokenKind.KW_FOR,
    ["in"]       = TokenKind.KW_IN,
    ["repeat"]   = TokenKind.KW_REPEAT,
    ["times"]    = TokenKind.KW_TIMES,
    ["return"]   = TokenKind.KW_RETURN,
    ["break"]    = TokenKind.KW_BREAK,
    ["continue"] = TokenKind.KW_CONTINUE,
    ["attempt"]  = TokenKind.KW_ATTEMPT,
    ["rescue"]   = TokenKind.KW_RESCUE,
    ["check"]    = TokenKind.KW_CHECK,
    ["throw"]    = TokenKind.KW_THROW,
    ["watch"]    = TokenKind.KW_WATCH,
    ["do"]       = TokenKind.KW_DO,
    ["as"]       = TokenKind.KW_AS,
    ["is"]       = TokenKind.KW_IS,
    ["_"]        = TokenKind.UNDERSCORE,

    -- Literais
    ["true"]     = TokenKind.KW_TRUE,
    ["false"]    = TokenKind.KW_FALSE,
    ["null"]     = TokenKind.KW_NULL,
    ["self"]     = TokenKind.KW_SELF,
    ["it"]       = TokenKind.KW_IT,

    -- MÃ³dulos
    ["import"]    = TokenKind.KW_IMPORT,
    ["export"]    = TokenKind.KW_EXPORT,

    -- LÃ³gicos e ValidaÃ§Ã£o
    ["and"]       = TokenKind.KW_AND,
    ["or"]        = TokenKind.KW_OR,
    ["not"]       = TokenKind.KW_NOT,
    ["where"]     = TokenKind.KW_WHERE,
    ["validate"]  = TokenKind.KW_VALIDATE,

    -- DSL de testes
    ["group"]     = TokenKind.KW_GROUP,
    ["test"]      = TokenKind.KW_TEST,
    ["assert"]    = TokenKind.KW_ASSERT,
    
    -- Tipos e Modificadores
    ["grid"]      = TokenKind.KW_GRID,
    ["uniq"]      = TokenKind.KW_UNIQ,

    -- Controle de Fluxo extra
    ["match"]     = TokenKind.KW_MATCH,
    ["case"]      = TokenKind.KW_CASE,
    ["after"]     = TokenKind.KW_AFTER,

    -- ConversÃ£o
    ["to"]        = TokenKind.KW_TO,
    ["native"]    = TokenKind.KW_NATIVE,
    ["extern"]    = TokenKind.KW_EXTERN,
}

--- Verifica se uma string Ã© keyword e retorna o TokenKind correspondente.
--- @param word string
--- @return string|nil TokenKind ou nil se nÃ£o for keyword
function KeywordTable.lookup(word)
    return KeywordTable.keywords[word]
end

--- Verifica se uma string Ã© keyword.
--- @param word string
--- @return boolean
function KeywordTable.is_keyword(word)
    return KeywordTable.keywords[word] ~= nil
end

return KeywordTable
