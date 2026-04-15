-- ============================================================================
-- Zenith Compiler — Syntax Kind
-- Enum de todos os tipos de nó da AST.
-- ============================================================================

local SyntaxKind = {
    -- Raiz
    COMPILATION_UNIT        = "COMPILATION_UNIT",

    -- ================================================================
    -- Expressões
    -- ================================================================
    LITERAL_EXPR            = "LITERAL_EXPR",
    IDENTIFIER_EXPR         = "IDENTIFIER_EXPR",
    BINARY_EXPR             = "BINARY_EXPR",
    UNARY_EXPR              = "UNARY_EXPR",
    CALL_EXPR               = "CALL_EXPR",
    MEMBER_EXPR             = "MEMBER_EXPR",
    INDEX_EXPR              = "INDEX_EXPR",
    RANGE_EXPR              = "RANGE_EXPR",
    CHECK_EXPR              = "CHECK_EXPR",            -- check expr
    BANG_EXPR               = "BANG_EXPR",
    REST_EXPR               = "REST_EXPR",             -- .. (rest/spread)
    LEN_EXPR                = "LEN_EXPR",              -- #
    MATCH_EXPR              = "MATCH_EXPR",
    LAMBDA_EXPR             = "LAMBDA_EXPR",           -- (params) => body
    GROUP_EXPR              = "GROUP_EXPR",            -- (expr)

    LIST_EXPR               = "LIST_EXPR",             -- [a, b, c]
    MAP_EXPR                = "MAP_EXPR",              -- { "k": v }
    SELF_EXPR               = "SELF_EXPR",             -- self
    IT_EXPR                 = "IT_EXPR",               -- it
    SELF_FIELD_EXPR         = "SELF_FIELD_EXPR",       -- @field (sugar for self.field)
    STRUCT_INIT_EXPR        = "STRUCT_INIT_EXPR",      -- Type { field: value }
    AWAIT_EXPR              = "AWAIT_EXPR",            -- await expr
    IS_EXPR                 = "IS_EXPR",               -- expr is Type
    AS_EXPR                 = "AS_EXPR",               -- expr as Type
    TRY_EXPR                = "TRY_EXPR",              -- expr?
    NATIVE_LUA_EXPR         = "NATIVE_LUA_EXPR",       -- native lua ... end

    -- ================================================================
    -- Statements
    -- ================================================================
    ASSIGN_STMT             = "ASSIGN_STMT",
    COMPOUND_ASSIGN_STMT    = "COMPOUND_ASSIGN_STMT",  -- += -= *= /=
    IF_STMT                 = "IF_STMT",
    ELIF_CLAUSE             = "ELIF_CLAUSE",
    ELSE_CLAUSE             = "ELSE_CLAUSE",
    WHILE_STMT              = "WHILE_STMT",
    FOR_IN_STMT             = "FOR_IN_STMT",
    REPEAT_TIMES_STMT       = "REPEAT_TIMES_STMT",
    BREAK_STMT              = "BREAK_STMT",
    CONTINUE_STMT           = "CONTINUE_STMT",
    RETURN_STMT             = "RETURN_STMT",
    WATCH_STMT              = "WATCH_STMT",
    ATTEMPT_STMT            = "ATTEMPT_STMT",
    RESCUE_CLAUSE           = "RESCUE_CLAUSE",
    EXPR_STMT               = "EXPR_STMT",             -- expressão como statement
    MATCH_STMT              = "MATCH_STMT",            -- match expr: ... end
    THROW_STMT              = "THROW_STMT",            -- throw expr
    CHECK_STMT              = "CHECK_STMT",            -- check expr else ... end
    AFTER_STMT              = "AFTER_STMT",            -- after ...
    NATIVE_LUA_STMT         = "NATIVE_LUA_STMT",       -- native lua ... end

    -- ================================================================
    -- Declarações
    -- ================================================================
    VAR_DECL                = "VAR_DECL",
    CONST_DECL              = "CONST_DECL",
    GLOBAL_DECL             = "GLOBAL_DECL",
    STATE_DECL              = "STATE_DECL",
    COMPUTED_DECL           = "COMPUTED_DECL",
    FUNC_DECL               = "FUNC_DECL",

    ASYNC_FUNC_DECL         = "ASYNC_FUNC_DECL",
    STRUCT_DECL             = "STRUCT_DECL",
    ENUM_DECL               = "ENUM_DECL",
    TYPE_ALIAS_DECL         = "TYPE_ALIAS_DECL",
    UNION_DECL              = "UNION_DECL",
    TRAIT_DECL              = "TRAIT_DECL",
    APPLY_DECL              = "APPLY_DECL",
    NAMESPACE_DECL          = "NAMESPACE_DECL",
    IMPORT_DECL             = "IMPORT_DECL",
    EXPORT_DECL             = "EXPORT_DECL",
    REDO_DECL               = "REDO_DECL",
    EXTERN_DECL             = "EXTERN_DECL",

    -- Subnós de declaração
    PARAM_NODE              = "PARAM_NODE",            -- name: Type
    FIELD_NODE              = "FIELD_NODE",             -- pub? name: Type = default
    ATTRIBUTE_NODE          = "ATTRIBUTE_NODE",         -- @name(args)
    ENUM_MEMBER_NODE        = "ENUM_MEMBER_NODE",
    VARIANT_PATTERN         = "VARIANT_PATTERN",        -- Variant(a, b) em match

    -- ================================================================
    -- Tipos
    -- ================================================================
    NAMED_TYPE              = "NAMED_TYPE",             -- int, text, Player
    NULLABLE_TYPE           = "NULLABLE_TYPE",          -- T?
    UNION_TYPE              = "UNION_TYPE",             -- A | B
    GENERIC_TYPE            = "GENERIC_TYPE",           -- list<int>, map<text, int>
    MODIFIED_TYPE           = "MODIFIED_TYPE",          -- uniq list<int>
    FUNC_TYPE               = "FUNC_TYPE",              -- func(int, text) -> bool
    STRUCT_TYPE             = "STRUCT_TYPE",            -- struct { fields }

    -- ================================================================
    -- Match
    -- ================================================================
    MATCH_CASE              = "MATCH_CASE",
    MATCH_ELSE              = "MATCH_ELSE",

    -- ================================================================
    -- Testes
    -- ================================================================
    GROUP_DECL              = "GROUP_DECL",
    TEST_DECL               = "TEST_DECL",
    ASSERT_STMT             = "ASSERT_STMT",

    -- Erro
    BAD_NODE                = "BAD_NODE",
}

return SyntaxKind
