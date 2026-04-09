-- ============================================================================
-- Zenith Compiler — Type Syntax
-- Factory functions para nós de tipo da AST.
-- ============================================================================

local SyntaxNode = require("src.syntax.ast.syntax_node")
local SK = require("src.syntax.ast.syntax_kind")

local TypeSyntax = {}

--- Tipo nomeado: int, text, Player
function TypeSyntax.named(name, span)
    return SyntaxNode.new(SK.NAMED_TYPE, {
        name = name,
    }, span)
end

--- Tipo nullable: T?
function TypeSyntax.nullable(base_type, span)
    return SyntaxNode.new(SK.NULLABLE_TYPE, {
        base_type = base_type,
    }, span)
end

--- Union type: A | B
function TypeSyntax.union(types, span)
    return SyntaxNode.new(SK.UNION_TYPE, {
        types = types,  -- lista de type nodes
    }, span)
end

--- Tipo genérico: list<int>, map<text, int>
function TypeSyntax.generic(base_name, type_args, span)
    return SyntaxNode.new(SK.GENERIC_TYPE, {
        base_name = base_name,
        type_args = type_args,
    }, span)
end

--- Tipo de função: (int, int) => bool
function TypeSyntax.func(params, return_type, span)
    return SyntaxNode.new(SK.FUNC_TYPE, {
        params = params, -- lista de type nodes
        return_type = return_type, -- type node ou nil
    }, span)
end

--- Tipo modificado: uniq list<int>
function TypeSyntax.modified(modifier, base_type, span)
    return SyntaxNode.new(SK.MODIFIED_TYPE, {
        modifier = modifier,    -- string ou TokenKind
        base_type = base_type,  -- type node
    }, span)
end

return TypeSyntax
