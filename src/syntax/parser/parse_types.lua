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

    -- Nullable: type? -> T | null (mantido como tipo nulável clássico)
    if ctx:match(TokenKind.QUESTION) then
        type_node = TypeSyntax.nullable(type_node, type_node.span:merge(ctx:peek(-1).span))
    end

    return type_node
end

function ParseTypes._parse_primary_type(ctx)
    -- Tipos Parentizados ou de Função: (A) ou (A, B) => C
    if ctx:match(TokenKind.LPAREN) then
        local start = ctx:peek(-1)
        local types = {}
        if not ctx:check(TokenKind.RPAREN) then
            repeat
                table.insert(types, ParseTypes.parse_type(ctx))
            until not ctx:match(TokenKind.COMMA)
        end
        ctx:expect(TokenKind.RPAREN, "esperado ')'")
        
        -- Se houver '=>', é uma função
        if ctx:match(TokenKind.FAT_ARROW) then
            local ret = ParseTypes.parse_type(ctx)
            return TypeSyntax.func(types, ret, start.span:merge(ret.span))
        end
        
        -- Caso contrário, deve ser apenas um tipo parentizado (A)
        if #types == 1 then
            return types[1]
        end
        
        -- Erro: múltiplos tipos sem =>
        ctx.diagnostics:report_error("ZT-P001", "esperado '=>' após lista de tipos em parênteses", start.span)
        return types[1] or TypeSyntax.named("error", start.span)
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

    -- Struct: struct { fields }
    if ctx:match(TokenKind.KW_STRUCT) then
        local start = ctx:peek(-1)
        ctx:expect(TokenKind.LBRACE, "esperado '{' após 'struct'")
        local fields = {}
        if not ctx:check(TokenKind.RBRACE) then
            repeat
                local f_name = ctx:expect(TokenKind.IDENTIFIER, "esperado nome do campo")
                ctx:expect(TokenKind.COLON, "esperado ':' após nome do campo")
                local f_type = ParseTypes.parse_type(ctx)
                table.insert(fields, { name = f_name.lexeme, type_node = f_type })
            until not ctx:match(TokenKind.COMMA)
        end
        local end_token = ctx:expect(TokenKind.RBRACE, "esperado '}'")
        return TypeSyntax.struct(fields, start.span:merge(end_token.span))
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
