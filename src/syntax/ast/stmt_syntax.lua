-- ============================================================================
-- Zenith Compiler — Statement Syntax
-- Factory functions para nós de statement da AST.
-- ============================================================================

local SyntaxNode = require("src.syntax.ast.syntax_node")
local SK = require("src.syntax.ast.syntax_kind")

local StmtSyntax = {}

--- Atribuição: x = valor
function StmtSyntax.assign(target, value, span)
    return SyntaxNode.new(SK.ASSIGN_STMT, {
        target = target,  -- expressão (identifier, member, index)
        value = value,
    }, span)
end

--- Atribuição composta: x += valor
function StmtSyntax.compound_assign(target, operator, value, span)
    return SyntaxNode.new(SK.COMPOUND_ASSIGN_STMT, {
        target = target,
        operator = operator,  -- Token do operador (+=, -=, etc.)
        value = value,
    }, span)
end

--- If / elif / else / end
function StmtSyntax.if_stmt(condition, body, elif_clauses, else_clause, span)
    return SyntaxNode.new(SK.IF_STMT, {
        condition = condition,
        body = body,                -- lista de statements
        elif_clauses = elif_clauses or {},  -- lista de elif nodes
        else_clause = else_clause,  -- else node ou nil
    }, span)
end

--- Elif clause
function StmtSyntax.elif_clause(condition, body, span)
    return SyntaxNode.new(SK.ELIF_CLAUSE, {
        condition = condition,
        body = body,
    }, span)
end

--- Else clause
function StmtSyntax.else_clause(body, span)
    return SyntaxNode.new(SK.ELSE_CLAUSE, {
        body = body,
    }, span)
end

--- While / end
function StmtSyntax.while_stmt(condition, body, span)
    return SyntaxNode.new(SK.WHILE_STMT, {
        condition = condition,
        body = body,
    }, span)
end

--- For item in collection / end
--- For index, item in collection / end
function StmtSyntax.for_in(variables, iterable, body, span)
    return SyntaxNode.new(SK.FOR_IN_STMT, {
        variables = variables,  -- lista de { name = string, span = span }
        iterable = iterable,    -- expressão
        body = body,
    }, span)
end

--- Repeat N times / end
function StmtSyntax.repeat_times(count_expr, body, span)
    return SyntaxNode.new(SK.REPEAT_TIMES_STMT, {
        count_expr = count_expr,
        body = body,
    }, span)
end

--- Break
function StmtSyntax.break_stmt(span)
    return SyntaxNode.new(SK.BREAK_STMT, {}, span)
end

--- Continue
function StmtSyntax.continue_stmt(span)
    return SyntaxNode.new(SK.CONTINUE_STMT, {}, span)
end

--- Return (com expressão opcional)
function StmtSyntax.return_stmt(expression, span)
    return SyntaxNode.new(SK.RETURN_STMT, {
        expression = expression,  -- pode ser nil
    }, span)
end

--- Watch state: ... end
function StmtSyntax.watch_stmt(target, body, span)
    return SyntaxNode.new(SK.WATCH_STMT, {
        target = target,  -- nome do state
        body = body,
    }, span)
end

--- Attempt / rescue / end
function StmtSyntax.attempt_stmt(body, rescue_clause, span)
    return SyntaxNode.new(SK.ATTEMPT_STMT, {
        body = body,
        rescue_clause = rescue_clause,
    }, span)
end

--- Rescue clause
function StmtSyntax.rescue_clause(error_name, error_type, body, span)
    return SyntaxNode.new(SK.RESCUE_CLAUSE, {
        error_name = error_name,  -- string (nome da variável de erro)
        error_type = error_type,  -- nó de tipo (pode ser nil)
        body = body,
    }, span)
end

--- Expression statement (chamada de função como statement, etc.)
function StmtSyntax.expr_stmt(expression, span)
    return SyntaxNode.new(SK.EXPR_STMT, {
        expression = expression,
    }, span)
end

--- Assert statement
function StmtSyntax.assert_stmt(expression, span)
    return SyntaxNode.new(SK.ASSERT_STMT, {
        expression = expression,
    }, span)
end

--- Match statement
function StmtSyntax.match_stmt(expression, cases, else_clause, span)
    return SyntaxNode.new(SK.MATCH_STMT or "MATCH_STMT", {
        expression = expression,
        cases = cases,        -- lista de MATCH_CASE
        else_clause = else_clause,
    }, span)
end

--- Match case
function StmtSyntax.match_case(patterns, body, span)
    return SyntaxNode.new(SK.MATCH_CASE, {
        patterns = patterns,   -- lista de expressões
        body = body,
    }, span)
end

--- Throw statement
function StmtSyntax.throw_stmt(expression, span)
    return SyntaxNode.new(SK.THROW_STMT, {
        expression = expression,
    }, span)
end

function StmtSyntax.check_stmt(condition, else_body, span)
    return SyntaxNode.new(SK.CHECK_STMT, {
        condition = condition,
        else_body = else_body,
    }, span)
end

function StmtSyntax.native_lua(lua_code, span)
    return SyntaxNode.new(SK.NATIVE_LUA_STMT, {
        lua_code = lua_code,
    }, span)
end

return StmtSyntax
