-- ============================================================================
-- Zenith Compiler — Statement Parser
-- Funções para parsing de statements (if, while, match, etc).
-- ============================================================================

local StmtSyntax = require("src.syntax.ast.stmt_syntax")
local ExprSyntax = require("src.syntax.ast.expr_syntax")
local TokenKind = require("src.syntax.tokens.token_kind")
local ParseExpressions = require("src.syntax.parser.parse_expressions")

local ParseStatements = {}

-- Lista de tokens que abrem blocos que OBRIGATORIAMENTE terminam com KW_END
local block_openers = {
    [TokenKind.KW_IF] = true,
    [TokenKind.KW_WHILE] = true,
    [TokenKind.KW_FOR] = true,
    [TokenKind.KW_REPEAT] = true,
    [TokenKind.KW_MATCH] = true,
    [TokenKind.KW_ATTEMPT] = true,
    [TokenKind.KW_WATCH] = true,
    [TokenKind.KW_GROUP] = true,
    [TokenKind.KW_TEST] = true,
    [TokenKind.KW_STRUCT] = true,
    [TokenKind.KW_ENUM] = true,
    [TokenKind.KW_TRAIT] = true,
    [TokenKind.KW_APPLY] = true,
    [TokenKind.KW_FUNC] = true,
    [TokenKind.KW_ASYNC] = true,
}

function ParseStatements.parse_statement(ctx)
    local k = ctx:peek().kind
    
    if k == TokenKind.KW_IF then
        return ParseStatements._parse_if(ctx)
    elseif k == TokenKind.KW_WHILE then
        return ParseStatements._parse_while(ctx)
    elseif k == TokenKind.KW_FOR then
        return ParseStatements._parse_for(ctx)
    elseif k == TokenKind.KW_REPEAT then
        return ParseStatements._parse_repeat(ctx)
    elseif k == TokenKind.KW_RETURN then
        return ParseStatements._parse_return(ctx)
    elseif k == TokenKind.KW_BREAK then
        return StmtSyntax.break_stmt(ctx:advance().span)
    elseif k == TokenKind.KW_CONTINUE then
        return StmtSyntax.continue_stmt(ctx:advance().span)
    elseif k == TokenKind.KW_MATCH then
        return ParseStatements._parse_match(ctx)
    elseif k == TokenKind.KW_ATTEMPT then
        return ParseStatements._parse_attempt(ctx)
    elseif k == TokenKind.KW_WATCH then
        return ParseStatements._parse_watch(ctx)
    elseif k == TokenKind.KW_THROW then
        return ParseStatements._parse_throw(ctx)
    elseif k == TokenKind.KW_CHECK then
        return ParseStatements._parse_check(ctx)
    elseif k == TokenKind.KW_AFTER then
        return ParseStatements._parse_after(ctx)
    elseif k == TokenKind.KW_NATIVE then
        return ParseStatements._parse_native_lua(ctx, true)
    end

    -- Expression Statement ou Atribuição
    local expr = ParseExpressions.parse_expression(ctx)
    
    -- Atribuição composta
    if ctx:match(TokenKind.PLUS_EQUAL) or ctx:match(TokenKind.MINUS_EQUAL) or
       ctx:match(TokenKind.STAR_EQUAL) or ctx:match(TokenKind.SLASH_EQUAL) then
        local op = ctx:peek(-1)
        local value = ParseExpressions.parse_expression(ctx)
        return StmtSyntax.compound_assign(expr, op, value, expr.span:merge(value.span))
    end
    
    -- Atribuição simples
    if ctx:match(TokenKind.EQUAL) then
        local value = ParseExpressions.parse_expression(ctx)
        return StmtSyntax.assign(expr, value, expr.span:merge(value.span))
    end

    return StmtSyntax.expr_stmt(expr, expr.span)
end

function ParseStatements._parse_attempt(ctx)
    local start = ctx:advance() -- 'attempt'
    ctx:skip_newlines()
    local body = ParseStatements.parse_block(ctx, {TokenKind.KW_RESCUE, TokenKind.KW_END})
    
    local rescue_clause = nil
    if ctx:match(TokenKind.KW_RESCUE) then
        local err_name = "e"
        if ctx:check(TokenKind.IDENTIFIER) then
            err_name = ctx:advance().lexeme
        end
        
        local err_type = nil
        if ctx:match(TokenKind.COLON) then
            local ParseTypes = require("src.syntax.parser.parse_types")
            err_type = ParseTypes.parse_type(ctx)
        end
        
        ctx:skip_newlines()
        local rescue_body = ParseStatements.parse_block(ctx, TokenKind.KW_END)
        rescue_clause = StmtSyntax.rescue_clause(err_name, err_type, rescue_body, start.span)
    end
    
    local end_token = ctx:expect(TokenKind.KW_END, "esperado 'end' após attempt")
    return StmtSyntax.attempt_stmt(body, rescue_clause, start.span:merge(end_token.span))
end

function ParseStatements._parse_watch(ctx)
    local start = ctx:advance() -- 'watch'
    
    local target = nil
    -- Se o próximo token não for uma nova linha nem o início de um bloco, tenta parsear expressão
    if not ctx:check(TokenKind.NEWLINE) and not ctx:check(TokenKind.KW_END) then
        target = ParseExpressions.parse_expression(ctx)
    end
    
    ctx:skip_newlines()
    local body = ParseStatements.parse_block(ctx, TokenKind.KW_END)
    local end_token = ctx:expect(TokenKind.KW_END, "esperado 'end' após watch")
    return StmtSyntax.watch_stmt(target, body, start.span:merge(end_token.span))
end

function ParseStatements.parse_block(ctx, end_kind)
    local statements = {}

    local function is_end(k)
        if type(end_kind) == "table" then
            for _, ek in ipairs(end_kind) do
                if k == ek then return true end
            end
            return false
        else
            return k == end_kind
        end
    end

    while not ctx:is_at_end() do
        ctx:skip_newlines()
        local k = ctx:peek().kind
        if is_end(k) then break end

        local ParseDeclarations = require("src.syntax.parser.parse_declarations")
        local stmt = ParseDeclarations.parse_declaration_or_statement(ctx)
        if stmt then table.insert(statements, stmt) end
        ctx:skip_newlines()
    end
    return statements
end

function ParseStatements._parse_if(ctx)
    local start = ctx:advance() -- 'if'
    local condition = ParseExpressions.parse_expression(ctx)
    ctx:skip_newlines()
    
    local body = ParseStatements.parse_block(ctx, { TokenKind.KW_ELIF, TokenKind.KW_ELSE, TokenKind.KW_END })
    
    local elif_clauses = {}
    while ctx:match(TokenKind.KW_ELIF) do
        local elif_cond = ParseExpressions.parse_expression(ctx)
        ctx:skip_newlines()
        local elif_body = ParseStatements.parse_block(ctx, { TokenKind.KW_ELIF, TokenKind.KW_ELSE, TokenKind.KW_END })
        table.insert(elif_clauses, { condition = elif_cond, body = elif_body })
    end
    
    local else_clause = nil
    if ctx:match(TokenKind.KW_ELSE) then
        ctx:skip_newlines()
        local else_body = ParseStatements.parse_block(ctx, TokenKind.KW_END)
        else_clause = { body = else_body }
    end
    
    local end_t = ctx:expect(TokenKind.KW_END, "esperado 'end' após if")
    return StmtSyntax.if_stmt(condition, body, elif_clauses, else_clause, start.span:merge(end_t.span))
end

function ParseStatements._parse_while(ctx)
    local start = ctx:advance() -- 'while'
    local condition = ParseExpressions.parse_expression(ctx)
    ctx:skip_newlines()
    local body = ParseStatements.parse_block(ctx, TokenKind.KW_END)
    local end_t = ctx:expect(TokenKind.KW_END, "esperado 'end' após while")
    return StmtSyntax.while_stmt(condition, body, start.span:merge(end_t.span))
end

function ParseStatements._parse_repeat(ctx)
    local start = ctx:advance() -- 'repeat'
    local count_expr = ParseExpressions.parse_expression(ctx)
    ctx:expect(TokenKind.KW_TIMES, "esperado 'times' após expressão no repeat")
    ctx:skip_newlines()
    local body = ParseStatements.parse_block(ctx, TokenKind.KW_END)
    local end_t = ctx:expect(TokenKind.KW_END, "esperado 'end' após repeat-times")
    return StmtSyntax.repeat_times(count_expr, body, start.span:merge(end_t.span))
end

function ParseStatements._parse_for(ctx)
    local start = ctx:advance() -- 'for'
    
    local variables = {}
    repeat
        local id = ctx:expect(TokenKind.IDENTIFIER, "esperado nome de variável no for-in")
        table.insert(variables, { name = id.lexeme, span = id.span })
    until not ctx:match(TokenKind.COMMA)
    
    ctx:expect(TokenKind.KW_IN, "esperado 'in' após variáveis no for")
    local iterable = ParseExpressions.parse_expression(ctx)
    ctx:skip_newlines()
    
    local body = ParseStatements.parse_block(ctx, TokenKind.KW_END)
    local end_t = ctx:expect(TokenKind.KW_END, "esperado 'end' após for")
    return StmtSyntax.for_in(variables, iterable, body, start.span:merge(end_t.span))
end

function ParseStatements._parse_return(ctx)
    local start = ctx:advance() -- 'return'
    local value = nil
    if not ctx:check(TokenKind.NEWLINE) and not ctx:check(TokenKind.KW_END) and 
       not ctx:check(TokenKind.KW_CASE) and not ctx:check(TokenKind.KW_ELSE) and not ctx:is_at_end() then
        value = ParseExpressions.parse_expression(ctx)
    end
    return StmtSyntax.return_stmt(value, start.span:merge(value and value.span or start.span))
end

function ParseStatements._parse_match(ctx)
    local start = ctx:advance() -- 'match'
    local expr = ParseExpressions.parse_expression(ctx)
    ctx:skip_newlines()
    
    local cases = {}
    local else_clause = nil
    
    while not ctx:check(TokenKind.KW_END) and not ctx:is_at_end() do
        ctx:skip_newlines()
        if ctx:match(TokenKind.KW_CASE) then
            local patterns = {}
            repeat
                local ParseDeclarations = require("src.syntax.parser.parse_declarations")
                table.insert(patterns, ParseDeclarations._parse_pattern(ctx, false))
            until not ctx:match(TokenKind.COMMA)
            
            local guard = nil
            if ctx:check(TokenKind.IDENTIFIER) and ctx:peek().lexeme == "when" then
                ctx:advance() -- consume 'when'
                guard = ParseExpressions.parse_expression(ctx)
            end
            
            if not ctx:match(TokenKind.COLON) and not ctx:match(TokenKind.FAT_ARROW) then
                ctx:expect(TokenKind.COLON, "esperado ':' ou '=>' após padrão do case (ou guard)")
            end
            ctx:skip_newlines()
            
            local body = ParseStatements.parse_block(ctx, { TokenKind.KW_CASE, TokenKind.KW_ELSE, TokenKind.KW_END })
            
            local case_node = StmtSyntax.match_case(patterns, body, patterns[1].span:merge(ctx:peek(-1).span))
            case_node.guard = guard
            table.insert(cases, case_node)
        elseif ctx:match(TokenKind.KW_ELSE) then
            ctx:match(TokenKind.COLON) -- Opcional no else
            ctx:skip_newlines()
            local else_body = ParseStatements.parse_block(ctx, TokenKind.KW_END)
            else_clause = StmtSyntax.else_clause(else_body, ctx:peek(-1).span)
            break
        else
            if not ctx:check(TokenKind.KW_END) then ctx:advance() else break end
        end
        ctx:skip_newlines()
    end
    
    local end_t = ctx:expect(TokenKind.KW_END, "esperado 'end' para fechar match")
    return StmtSyntax.match_stmt(expr, cases, else_clause, start.span:merge(end_t.span))
end

function ParseStatements._parse_throw(ctx)
    local start = ctx:advance() -- 'throw'
    local expr = ParseExpressions.parse_expression(ctx)
    return StmtSyntax.throw_stmt(expr, start.span:merge(expr.span))
end

function ParseStatements._parse_check(ctx)
    local start = ctx:advance() -- 'check'
    local condition = ParseExpressions.parse_expression(ctx)
    
    local else_body = {}
    if ctx:match(TokenKind.KW_ELSE) then
        ctx:skip_newlines()
        else_body = ParseStatements.parse_block(ctx, TokenKind.KW_END)
    end
    
    local end_t = ctx:expect(TokenKind.KW_END, "esperado 'end' para fechar check")
    
    return StmtSyntax.check_stmt(condition, else_body, start.span:merge(end_t.span))
end

function ParseStatements._parse_after(ctx)
    local start = ctx:advance() -- 'after'
    local body
    if ctx:match(TokenKind.KW_DO) then
        body = ParseStatements.parse_block(ctx, TokenKind.KW_END)
        local end_t = ctx:expect(TokenKind.KW_END, "esperado 'end' após block no after")
        return StmtSyntax.after_stmt(body, start.span:merge(end_t.span))
    else
        local stmt = ParseStatements.parse_statement(ctx)
        return StmtSyntax.after_stmt({stmt}, start.span:merge(stmt.span))
    end
end

function ParseStatements._parse_native_lua(ctx, is_stmt)
    local start = ctx:advance() -- 'native'
    ctx:expect(TokenKind.IDENTIFIER, "esperado 'lua' após 'native'")
    
    local content = {}
    local depth = 1
    
    while not ctx:is_at_end() do
        local t = ctx:peek()
        if block_openers[t.kind] or (t.kind == TokenKind.IDENTIFIER and t.lexeme == "function") then
            depth = depth + 1
        elseif t.kind == TokenKind.KW_END then
            depth = depth - 1
            if depth == 0 then break end
        end
        
        table.insert(content, ctx:advance().lexeme)
        table.insert(content, " ")
    end
    
    local end_t = ctx:expect(TokenKind.KW_END, "esperado 'end' após bloco native lua")
    local lua_code = table.concat(content)
    
    if is_stmt then
        return StmtSyntax.native_lua(lua_code, start.span:merge(end_t.span))
    else
        return ExprSyntax.native_lua(lua_code, start.span:merge(end_t.span))
    end
end

return ParseStatements
