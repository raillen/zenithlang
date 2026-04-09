-- ============================================================================
-- Zenith Compiler — Parse Types
-- Parsing de tipos: int, text, list<int>, int?, etc.
-- ============================================================================

local TokenKind  = require("src.syntax.tokens.token_kind")
local TypeSyntax = require("src.syntax.ast.type_syntax")

local ParseTypes = {}

function ParseTypes.parse_type(ctx)
    -- Modificadores: uniq list<T>
    if ctx:match(TokenKind.KW_UNIQ) then
        local modifier = ctx:peek(-1)
        local base = ParseTypes.parse_type(ctx)
        return TypeSyntax.modified(modifier.lexeme, base, modifier.span:merge(base.span))
    end

    local type_node = ParseTypes._parse_primary_type(ctx)

    -- Union: A | B
    while ctx:match(TokenKind.PIPE) do
        local other = ParseTypes._parse_primary_type(ctx)
        local types = { type_node, other }
        type_node = TypeSyntax.union(types, type_node.span:merge(other.span))
    end

    -- Nullable: type?
    if ctx:match(TokenKind.QUESTION) then
        type_node = TypeSyntax.nullable(type_node, type_node.span:merge(ctx:peek(-1).span))
    end

    return type_node
end

function ParseTypes._parse_primary_type(ctx)
    -- Tipo de Função: (int, int) => bool
    if ctx:match(TokenKind.LPAREN) then
        local start = ctx:peek(-1)
        local params = {}
        if not ctx:check(TokenKind.RPAREN) then
            repeat
                table.insert(params, ParseTypes.parse_type(ctx))
            until not ctx:match(TokenKind.COMMA)
        end
        ctx:expect(TokenKind.RPAREN, "esperado ')' após tipos dos parâmetros")
        ctx:expect(TokenKind.FAT_ARROW, "esperado '=>' para definir tipo de retorno da função")
        local ret = ParseTypes.parse_type(ctx)
        return TypeSyntax.func(params, ret, start.span:merge(ret.span))
    end

    -- it como tipo
    if ctx:match(TokenKind.KW_IT) then
        return TypeSyntax.named("it", ctx:peek(-1).span)
    end

    -- Grid: grid<T>
    if ctx:match(TokenKind.KW_GRID) then
        local grid_token = ctx:peek(-1)
        local span = grid_token.span
        local args = {}
        if ctx:match(TokenKind.LESS) then
            repeat
                table.insert(args, ParseTypes.parse_type(ctx))
            until not ctx:match(TokenKind.COMMA)
            local end_token = ctx:expect(TokenKind.GREATER, "esperado '>' após grid")
            span = span:merge(end_token.span)
        end
        return TypeSyntax.generic("grid", args, span)
    end

    local id = ctx:expect(TokenKind.IDENTIFIER, "esperado nome do tipo")
    local span = id.span

    -- Genérico: Name<Args>
    if ctx:match(TokenKind.LESS) then
        local args = {}
        repeat
            table.insert(args, ParseTypes.parse_type(ctx))
        until not ctx:match(TokenKind.COMMA)
        local end_token = ctx:expect(TokenKind.GREATER, "esperado '>' após argumentos genéricos")
        return TypeSyntax.generic(id.lexeme, args, span:merge(end_token.span))
    end

    return TypeSyntax.named(id.lexeme, span)
end

return ParseTypes
