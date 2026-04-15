-- ============================================================================
-- Zenith Compiler — Expression Syntax
-- Factory functions para nós de expressão da AST.
-- ============================================================================

local SyntaxNode = require("src.syntax.ast.syntax_node")
local SK = require("src.syntax.ast.syntax_kind")

local ExprSyntax = {}

--- Literal: 10, 3.14, "text", true, false, null
function ExprSyntax.literal(value, literal_type, span)
    return SyntaxNode.new(SK.LITERAL_EXPR, {
        value = value,
        literal_type = literal_type, -- "int", "float", "text", "bool", "null"
    }, span)
end

--- Identificador: nome de variável
function ExprSyntax.identifier(name, span)
    return SyntaxNode.new(SK.IDENTIFIER_EXPR, {
        name = name,
    }, span)
end

--- Expressão binária: a + b, a == b, a and b
function ExprSyntax.binary(left, operator, right, span)
    return SyntaxNode.new(SK.BINARY_EXPR, {
        left = left,
        operator = operator,  -- Token do operador
        right = right,
    }, span)
end

--- Expressão unária (prefixo): not x, -x
function ExprSyntax.unary(operator, operand, span)
    return SyntaxNode.new(SK.UNARY_EXPR, {
        operator = operator,
        operand = operand,
    }, span)
end

--- Chamada de função: func<T>(args)
function ExprSyntax.call(callee, arguments, span, generic_args)
    return SyntaxNode.new(SK.CALL_EXPR, {
        callee = callee,
        arguments = arguments,  -- lista de expressões
        generic_args = generic_args, -- lista de type_nodes ou nil
    }, span)
end

--- Acesso a membro: obj.campo ou obj?.campo
function ExprSyntax.member(object, member_name, span, is_safe)
    return SyntaxNode.new(SK.MEMBER_EXPR, {
        object = object,
        member_name = member_name,  -- string
        is_safe = is_safe or false,
    }, span)
end

--- Acesso por índice: list[i]
function ExprSyntax.index(object, index_expr, span)
    return SyntaxNode.new(SK.INDEX_EXPR, {
        object = object,
        index_expr = index_expr,
    }, span)
end

--- Range: 0..5
function ExprSyntax.range(start_expr, end_expr, span)
    return SyntaxNode.new(SK.RANGE_EXPR, {
        start_expr = start_expr,
        end_expr = end_expr,
    }, span)
end

--- Expressão agrupada: (expr)
function ExprSyntax.group(expression, span)
    return SyntaxNode.new(SK.GROUP_EXPR, {
        expression = expression,
    }, span)
end

--- Lista: [a, b, c]
function ExprSyntax.list(elements, span)
    return SyntaxNode.new(SK.LIST_EXPR, {
        elements = elements,
    }, span)
end

--- Map: { "k": v, ... }
function ExprSyntax.map(pairs, span)
    return SyntaxNode.new(SK.MAP_EXPR, {
        pairs = pairs,  -- lista de { key = expr, value = expr }
    }, span)
end

--- Self: self
function ExprSyntax.self_ref(span)
    return SyntaxNode.new(SK.SELF_EXPR, {}, span)
end

--- It: it
function ExprSyntax.it_ref(span)
    return SyntaxNode.new(SK.IT_EXPR, {}, span)
end

--- Self field sugar: .field → self.field
function ExprSyntax.self_field(field_name, span)
    return SyntaxNode.new(SK.SELF_FIELD_EXPR, {
        field_name = field_name,
    }, span)
end

--- Struct init: Type { field: value }
function ExprSyntax.struct_init(type_name, fields, span, generic_args)
    return SyntaxNode.new(SK.STRUCT_INIT_EXPR, {
        type_name = type_name,
        fields = fields,  -- lista de { name = string, value = expr }
        generic_args = generic_args, -- lista de type_nodes ou nil
    }, span)
end

--- Operador Bang (!)
function ExprSyntax.bang(expression, span)
    return SyntaxNode.new(SK.BANG_EXPR, {
        expression = expression,
    }, span)
end

--- Operador Rest/Spread (..)
function ExprSyntax.rest(expression, span)
    return SyntaxNode.new(SK.REST_EXPR, {
        expression = expression,
    }, span)
end

--- Operador de Tamanho (#)
function ExprSyntax.len(expression, span)
    return SyntaxNode.new(SK.LEN_EXPR, {
        expression = expression,
    }, span)
end


--- Lambdas: (params) => body
function ExprSyntax.lambda(params, body, is_async, span)
    return SyntaxNode.new(SK.LAMBDA_EXPR, {
        params = params,   -- lista de PARAM_NODE
        body = body,       -- expressão ou lista de statements (se usar do...end)
        is_async = is_async or false,
    }, span)
end

--- Await: await expr
function ExprSyntax.await_expr(expression, span)
    return SyntaxNode.new(SK.AWAIT_EXPR, {
        expression = expression,
    }, span)
end

--- Is: expr is Type
function ExprSyntax.is_expr(expression, type_node, is_not, span)
    return SyntaxNode.new(SK.IS_EXPR, {
        expression = expression,
        type_node = type_node,
        is_not = is_not or false,
    }, span)
end

--- As: expr as Type
function ExprSyntax.as_expr(expression, type_node, span)
    return SyntaxNode.new(SK.AS_EXPR, {
        expression = expression,
        type_node = type_node,
    }, span)
end

--- Check: check expr
function ExprSyntax.check_expr(expression, span)
    return SyntaxNode.new(SK.CHECK_EXPR, {
        expression = expression,
    }, span)
end

--- Try: expr?
function ExprSyntax.try(expression, span)
    return SyntaxNode.new(SK.TRY_EXPR, {
        expression = expression,
    }, span)
end

function ExprSyntax.native_lua(lua_code, span)
    return SyntaxNode.new(SK.NATIVE_LUA_EXPR, {
        lua_code = lua_code,
    }, span)
end

return ExprSyntax
