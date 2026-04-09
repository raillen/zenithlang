-- ============================================================================
-- Zenith Compiler — Token Kind
-- Enum de todos os tipos de token da linguagem.
-- ============================================================================

local TokenKind = {
    -- Fim de arquivo
    EOF                 = "EOF",

    -- Literais básicos
    INTEGER_LITERAL     = "INTEGER_LITERAL",
    FLOAT_LITERAL       = "FLOAT_LITERAL",
    STRING_LITERAL      = "STRING_LITERAL",
    IDENTIFIER          = "IDENTIFIER",

    -- Palavras-chave (KW_ prefix)
    KW_VAR              = "KW_VAR",
    KW_CONST            = "KW_CONST",
    KW_GLOBAL           = "KW_GLOBAL",
    KW_STATE            = "KW_STATE",
    KW_COMPUTED         = "KW_COMPUTED",
    KW_WATCH            = "KW_WATCH",
    KW_PUB              = "KW_PUB",
    KW_NAMESPACE        = "KW_NAMESPACE",
    KW_REDO             = "KW_REDO",
    KW_IMPORT           = "KW_IMPORT",
    KW_EXPORT           = "KW_EXPORT",
    KW_AS               = "KW_AS",
    KW_IS               = "KW_IS",
    KW_DO               = "KW_DO",
    KW_FROM             = "KW_FROM",
    
    KW_STRUCT           = "KW_STRUCT",
    KW_ENUM             = "KW_ENUM",
    KW_TRAIT            = "KW_TRAIT",
    KW_APPLY            = "KW_APPLY",
    KW_TYPE             = "KW_TYPE",
    KW_UNION            = "KW_UNION",

    
    KW_FUNC             = "KW_FUNC",
    KW_ASYNC            = "KW_ASYNC",
    KW_AWAIT            = "KW_AWAIT",
    KW_RETURN           = "KW_RETURN",
    
    KW_IF               = "KW_IF",
    KW_ELIF             = "KW_ELIF",
    KW_ELSE             = "KW_ELSE",
    KW_END              = "KW_END",
    KW_WHILE            = "KW_WHILE",
    KW_FOR              = "KW_FOR",
    KW_IN               = "KW_IN",
    KW_REPEAT           = "KW_REPEAT",
    KW_TIMES            = "KW_TIMES",
    KW_BREAK            = "KW_BREAK",
    KW_CONTINUE         = "KW_CONTINUE",
    KW_MATCH            = "KW_MATCH",
    KW_CASE             = "KW_CASE",
    
    KW_TRY              = "KW_TRY",
    KW_CATCH            = "KW_CATCH",
    KW_ATTEMPT          = "KW_ATTEMPT",
    KW_RESCUE           = "KW_RESCUE",
    KW_CHECK            = "KW_CHECK",
    KW_THROW            = "KW_THROW",
    KW_FINALLY          = "KW_FINALLY",
    
    KW_TRUE             = "KW_TRUE",
    KW_FALSE            = "KW_FALSE",
    KW_NULL             = "KW_NULL",
    KW_SELF             = "KW_SELF",
    KW_IT               = "KW_IT",
    
    KW_AND              = "KW_AND",
    KW_OR               = "KW_OR",
    KW_NOT              = "KW_NOT",
    KW_WHERE            = "KW_WHERE",
    KW_GRID             = "KW_GRID",
    KW_UNIQ             = "KW_UNIQ",
    KW_TO               = "KW_TO",
    
    KW_GROUP            = "KW_GROUP",
    KW_TEST             = "KW_TEST",
    KW_ASSERT           = "KW_ASSERT",

    -- Operadores aritméticos
    PLUS                = "PLUS",           -- +
    MINUS               = "MINUS",          -- -
    STAR                = "STAR",           -- *
    SLASH               = "SLASH",          -- /
    PERCENT             = "PERCENT",        -- %
    CARET               = "CARET",          -- ^

    -- Operadores de comparação
    EQUAL_EQUAL         = "EQUAL_EQUAL",    -- ==
    BANG_EQUAL          = "BANG_EQUAL",     -- !=
    LESS                = "LESS",           -- <
    LESS_EQUAL          = "LESS_EQUAL",     -- <=
    GREATER             = "GREATER",        -- >
    GREATER_EQUAL       = "GREATER_EQUAL",  -- >=

    -- Operadores de atribuição
    EQUAL               = "EQUAL",          -- =
    PLUS_EQUAL          = "PLUS_EQUAL",     -- +=
    MINUS_EQUAL         = "MINUS_EQUAL",    -- -=
    STAR_EQUAL          = "STAR_EQUAL",     -- *=
    SLASH_EQUAL         = "SLASH_EQUAL",    -- /=

    -- Delimitadores
    LPAREN              = "LPAREN",         -- (
    RPAREN              = "RPAREN",         -- )
    LBRACKET            = "LBRACKET",       -- [
    RBRACKET            = "RBRACKET",       -- ]
    LBRACE              = "LBRACE",         -- {
    RBRACE              = "RBRACE",         -- }

    -- Pontuação
    DOT                 = "DOT",            -- .
    DOT_DOT             = "DOT_DOT",        -- ..
    COMMA               = "COMMA",          -- ,
    COLON               = "COLON",          -- :
    SEMICOLON           = "SEMICOLON",      -- ;
    ARROW               = "ARROW",          -- ->
    FAT_ARROW           = "FAT_ARROW",      -- =>
    DOT                 = "DOT",            -- .
    PIPE                = "PIPE",           -- |
    BANG                = "BANG",           -- !
    QUESTION            = "QUESTION",       -- ?
    HASH                = "HASH",           -- #
    AT                  = "AT",             -- @
    UNDERSCORE          = "UNDERSCORE",     -- _
    
    NEWLINE             = "NEWLINE",
    BAD_TOKEN           = "BAD_TOKEN",
}

return TokenKind
