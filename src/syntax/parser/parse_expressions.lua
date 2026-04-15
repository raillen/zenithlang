-- ============================================================================
-- Zenith Compiler — Parse Expressions
-- Pratt Parser para expressões com precedência e associatividade.
-- ============================================================================

local TokenKind     = require("src.syntax.tokens.token_kind")
local ExprSyntax    = require("src.syntax.ast.expr_syntax")
local SK            = require("src.syntax.ast.syntax_kind")
local OperatorTable = require("src.syntax.tokens.operator_table")

local ParseExpressions = {}

--- Heurística para distinguir '<' (menor que) de '<T>' (genéricos).
function ParseExpressions._is_generic_start(ctx)
    local i = 0
    local balance = 1
    while true do
        local t = ctx:peek(i)
        if t.kind == TokenKind.EOF or t.kind == TokenKind.SEMICOLON or t.kind == TokenKind.NEWLINE then
            return false
        end
        if t.kind == TokenKind.LESS then
            balance = balance + 1
        elseif t.kind == TokenKind.GREATER then
            balance = balance - 1
            if balance == 0 then
                local next_t = ctx:peek(i + 1)
                return next_t.kind == TokenKind.LPAREN or next_t.kind == TokenKind.LBRACE
            end
        end
        i = i + 1
        if i > 30 then return false end
    end
end

--- Extrai o nome de um tipo de uma expressão (ex: 'SourceText' ou 'source.SourceText').
function ParseExpressions._extract_type_name(expr)
    if not expr then return nil end
    if expr.kind == SK.IDENTIFIER_EXPR then
        return expr.name
    elseif expr.kind == SK.MEMBER_EXPR then
        local left = ParseExpressions._extract_type_name(expr.object)
        if left then
            return left .. "." .. expr.member_name
        end
    end
    return nil
end

--- Ponto de entrada para parsing de expressões.
function ParseExpressions.parse_expression(ctx, precedence)
    precedence = precedence or 0
    local left = nil
    
    local initial_kind = ctx:peek().kind
    
    -- Operadores Prefixos
    if OperatorTable.is_unary_prefix(initial_kind) then
        local operator = ctx:advance()
        local operand = ParseExpressions.parse_expression(ctx, OperatorTable.Precedence.UNARY)
        
        if operator.kind == TokenKind.KW_CHECK then
            left = ExprSyntax.check_expr(operand, operator.span:merge(operand.span))
        elseif operator.kind == TokenKind.KW_AWAIT then
            left = ExprSyntax.await_expr(operand, operator.span:merge(operand.span))
        elseif operator.kind == TokenKind.DOT_DOT then
            left = ExprSyntax.rest(operand, operator.span:merge(operand.span))
        elseif operator.kind == TokenKind.HASH then
            left = ExprSyntax.len(operand, operator.span:merge(operand.span))
        else
            left = ExprSyntax.unary(operator, operand, operator.span:merge(operand.span))
        end
    else
        left = ParseExpressions._parse_primary(ctx)
    end

    if not left then
        local t = ctx:peek()
        ctx.diagnostics:report_error("ZT-P002", "expressão esperada, encontrado " .. t.kind, t.span)
        ctx:advance() -- Garante progresso!
        return ExprSyntax.literal(nil, "error", t.span)
    end

    -- Loop de Operadores Pós-fixos e Binários
    while true do
        local kind = ctx:peek().kind
        
        -- 1. Operadores Pós-fixos (ex: !)
        if OperatorTable.is_unary_postfix(kind) then
            local operator = ctx:advance()
            if operator.kind == TokenKind.BANG then
                left = ExprSyntax.bang(left, left.span:merge(operator.span))
            elseif operator.kind == TokenKind.QUESTION then
                left = ExprSyntax.try(left, left.span:merge(operator.span))
            end
            -- Continua para checar mais pós-fixos ou binários
        
        -- 2. Operadores Binários
        else
            local prec = OperatorTable.binary_precedence(kind)
            if prec == 0 or prec <= precedence then
                break
            end

            -- Importante: consumimos o operador agora
            local operator = ctx:advance()
            
            -- Casos Especiais de Operadores Binários
            if kind == TokenKind.DOT or kind == TokenKind.QUESTION_DOT then
                local member = ctx:expect_field_name("esperado nome do campo após '" .. operator.lexeme .. "'")
                left = ExprSyntax.member(left, member.lexeme, left.span:merge(member.span), kind == TokenKind.QUESTION_DOT)
            
            elseif kind == TokenKind.LESS and ParseExpressions._is_generic_start(ctx) then
                -- Genéricos: func<T>() ou Struct<T> {}
                local generic_args = {}
                repeat
                    local ParseTypes = require("src.syntax.parser.parse_types")
                    table.insert(generic_args, ParseTypes.parse_type(ctx))
                until not ctx:match(TokenKind.COMMA)
                ctx:expect(TokenKind.GREATER, "esperado '>' após argumentos genéricos")
                
                if ctx:check(TokenKind.LPAREN) then
                    local start_paren = ctx:advance()
                    local args = {}
                    if not ctx:check(TokenKind.RPAREN) then
                        repeat
                            ctx:skip_newlines()
                            if ctx:check(TokenKind.IDENTIFIER) and ctx:peek(1).kind == TokenKind.COLON then
                                local name_t = ctx:advance()
                                ctx:advance() -- consume ':'
                                local val = ParseExpressions.parse_expression(ctx)
                                table.insert(args, { name = name_t.lexeme, value = val, kind = "NAMED" })
                            else
                                table.insert(args, ParseExpressions.parse_expression(ctx))
                            end
                        until not ctx:match(TokenKind.COMMA)
                    end
                    local end_token = ctx:expect(TokenKind.RPAREN, "esperado ')'")
                    left = ExprSyntax.call(left, args, left.span:merge(end_token.span), generic_args)
                elseif ctx:check(TokenKind.LBRACE) then
                    local type_name = ParseExpressions._extract_type_name(left)
                    local start_brace = ctx:advance()
                    local fields = {}
                    if not ctx:check(TokenKind.RBRACE) then
                        repeat
                            ctx:skip_newlines()
                            if ctx:check(TokenKind.RBRACE) then break end
                            local name_t = ctx:expect_field_name("esperado nome do campo")
                            local f_val
                            if ctx:match(TokenKind.COLON) then
                                f_val = ParseExpressions.parse_expression(ctx)
                            else
                                f_val = ExprSyntax.identifier(name_t.lexeme, name_t.span)
                            end
                            table.insert(fields, { name = name_t.lexeme, value = f_val })
                            ctx:match(TokenKind.COMMA)
                            ctx:skip_newlines()
                        until ctx:check(TokenKind.RBRACE) or ctx:is_at_end()
                    end
                    local end_token = ctx:expect(TokenKind.RBRACE, "esperado '}'")
                    left = ExprSyntax.struct_init(type_name, fields, left.span:merge(end_token.span), generic_args)
                else
                    ctx.diagnostics:report_error("ZT-P001", "esperado '(' ou '{' após argumentos genéricos", ctx:peek().span)
                    break
                end

            elseif kind == TokenKind.LPAREN then
                local args = {}
                if not ctx:check(TokenKind.RPAREN) then
                    repeat
                        ctx:skip_newlines()
                        if ctx:check(TokenKind.IDENTIFIER) and ctx:peek(1).kind == TokenKind.COLON then
                            local name_t = ctx:advance()
                            ctx:advance() -- consume ':'
                            local val = ParseExpressions.parse_expression(ctx)
                            table.insert(args, { name = name_t.lexeme, value = val, kind = "NAMED" })
                        else
                            table.insert(args, ParseExpressions.parse_expression(ctx))
                        end
                    until not ctx:match(TokenKind.COMMA)
                end
                local end_token = ctx:expect(TokenKind.RPAREN, "esperado ')'")
                left = ExprSyntax.call(left, args, left.span:merge(end_token.span))

            elseif kind == TokenKind.LBRACKET then
                local index = ParseExpressions.parse_expression(ctx)
                local end_token = ctx:expect(TokenKind.RBRACKET, "esperado ']'")
                left = ExprSyntax.index(left, index, left.span:merge(end_token.span))

            elseif kind == TokenKind.LBRACE then
                -- Struct Init padrão
                local type_name = ParseExpressions._extract_type_name(left)
                local fields = {}
                if not ctx:check(TokenKind.RBRACE) then
                    repeat
                        ctx:skip_newlines()
                        if ctx:check(TokenKind.RBRACE) then break end
                        local name_t = ctx:expect_field_name("esperado nome do campo")
                        local f_val
                        if ctx:match(TokenKind.COLON) then
                            f_val = ParseExpressions.parse_expression(ctx)
                        else
                            f_val = ExprSyntax.identifier(name_t.lexeme, name_t.span)
                        end
                        table.insert(fields, { name = name_t.lexeme, value = f_val })
                        ctx:match(TokenKind.COMMA)
                        ctx:skip_newlines()
                    until ctx:check(TokenKind.RBRACE) or ctx:is_at_end()
                end
                local end_token = ctx:expect(TokenKind.RBRACE, "esperado '}'")
                left = ExprSyntax.struct_init(type_name, fields, left.span:merge(end_token.span), nil)

            elseif operator.kind == TokenKind.KW_AS then
                local ParseTypes = require("src.syntax.parser.parse_types")
                local type_node = ParseTypes.parse_type(ctx)
                left = ExprSyntax.as_expr(left, type_node, left.span:merge(type_node.span))
            
            elseif operator.kind == TokenKind.KW_IS then
                local is_not = ctx:match(TokenKind.KW_NOT)
                local ParseTypes = require("src.syntax.parser.parse_types")
                local type_node = ParseTypes.parse_type(ctx)
                left = ExprSyntax.is_expr(left, type_node, is_not, left.span:merge(type_node.span))
            
            elseif kind == TokenKind.DOT_DOT then
                -- Range: 0..5 → RANGE_EXPR com start_expr/end_expr
                local right = ParseExpressions.parse_expression(ctx, prec)
                if not right then
                    ctx.diagnostics:report_error("ZT-P002", "esperada expressão após '..'", operator.span)
                    right = ExprSyntax.literal(nil, "error", operator.span)
                end
                left = ExprSyntax.range(left, right, left.span:merge(right.span))

            else
                -- Operadores Binários Comuns (+, -, *, / etc)
                local assoc = OperatorTable.binary_assoc(kind)
                local right = ParseExpressions.parse_expression(ctx, prec - (assoc == "RIGHT" and 1 or 0))
                
                if not right then
                    ctx.diagnostics:report_error("ZT-P002", "esperada expressão após o operador '" .. operator.lexeme .. "'", operator.span)
                    right = ExprSyntax.literal(nil, "error", operator.span)
                end

                left = ExprSyntax.binary(left, operator, right, left.span:merge(right.span))
            end
        end
    end

    return left
end

function ParseExpressions._parse_lambda(ctx, is_async)
    local start = ctx:peek()
    if not is_async then
        is_async = ctx:match(TokenKind.KW_ASYNC)
    end
    
    ctx:expect(TokenKind.LPAREN, "esperado '(' na lambda")
    local ParseDeclarations = require("src.syntax.parser.parse_declarations")
    local params = ParseDeclarations._parse_params(ctx)
    ctx:expect(TokenKind.RPAREN, "esperado ')' na lambda")
    ctx:expect(TokenKind.FAT_ARROW, "esperado '=>' na lambda")
    
    local body
    local span_end
    if ctx:match(TokenKind.KW_DO) then
        local ParseStatements = require("src.syntax.parser.parse_statements")
        body = ParseStatements.parse_block(ctx, TokenKind.KW_END)
        local end_t = ctx:expect(TokenKind.KW_END, "esperado 'end' ao final do bloco da lambda")
        span_end = end_t.span
    else
        body = ParseExpressions.parse_expression(ctx, OperatorTable.Precedence.ASSIGNMENT)
        span_end = body.span
    end
    return ExprSyntax.lambda(params, body, is_async, start.span:merge(span_end))
end

function ParseExpressions._parse_primary(ctx)
    local k = ctx:peek().kind

    if k == TokenKind.IDENTIFIER or k == TokenKind.KW_GRID or k == TokenKind.KW_TEST or k == TokenKind.UNDERSCORE then
        local id = ctx:advance()
        local node = ExprSyntax.identifier(id.lexeme, id.span)
        return node
    elseif k == TokenKind.KW_SELF then
        return ExprSyntax.self_ref(ctx:advance().span)
    elseif k == TokenKind.KW_IT then
        return ExprSyntax.it_ref(ctx:advance().span)
    elseif k == TokenKind.INTEGER_LITERAL or k == TokenKind.FLOAT_LITERAL or 
           k == TokenKind.STRING_LITERAL or k == TokenKind.KW_TRUE or 
           k == TokenKind.KW_FALSE or k == TokenKind.KW_NULL then
        local t = ctx:advance()
        local ltype = "any"
        local value = t.value
        if t.kind == TokenKind.INTEGER_LITERAL then ltype = "int"
        elseif t.kind == TokenKind.FLOAT_LITERAL then ltype = "float"
        elseif t.kind == TokenKind.STRING_LITERAL then ltype = "text"
        elseif t.kind == TokenKind.KW_TRUE or t.kind == TokenKind.KW_FALSE then ltype = "bool" 
        elseif t.kind == TokenKind.KW_NULL then
            ltype = "null"
            value = nil
        end
        return ExprSyntax.literal(value, ltype, t.span)
    
    elseif k == TokenKind.LPAREN or k == TokenKind.KW_ASYNC then
        local start = ctx:peek()
        local is_async = ctx:match(TokenKind.KW_ASYNC)
        
        local is_lambda = false
        if ctx:check(TokenKind.LPAREN) then
            local i = 1
            if ctx:peek(i).kind == TokenKind.RPAREN and ctx:peek(i+1).kind == TokenKind.FAT_ARROW then
                is_lambda = true
            elseif ctx:peek(i).kind == TokenKind.IDENTIFIER then
                local next_k = ctx:peek(i+1).kind
                if next_k == TokenKind.COLON or next_k == TokenKind.COMMA or next_k == TokenKind.LBRACE then is_lambda = true end
            elseif ctx:peek(i).kind == TokenKind.LBRACKET then is_lambda = true end
        end

        if is_lambda or is_async then 
            return ParseExpressions._parse_lambda(ctx, is_async) 
        end

        ctx:advance() -- (
        local expr = ParseExpressions.parse_expression(ctx)
        ctx:expect(TokenKind.RPAREN, "esperado ')' após expressão")
        return ExprSyntax.group(expr, start.span:merge(ctx:peek(-1).span))
    
    elseif k == TokenKind.LBRACKET then
        local start = ctx:advance()
        local elements = {}
        if not ctx:check(TokenKind.RBRACKET) then
            repeat
                ctx:skip_newlines()
                if ctx:check(TokenKind.RBRACKET) then break end
                table.insert(elements, ParseExpressions.parse_expression(ctx))
                ctx:match(TokenKind.COMMA)
                ctx:skip_newlines()
            until ctx:check(TokenKind.RBRACKET) or ctx:is_at_end()
        end
        local end_t = ctx:expect(TokenKind.RBRACKET, "esperado ']'")
        return ExprSyntax.list(elements, start.span:merge(end_t.span))

    elseif k == TokenKind.LBRACE then
        local start = ctx:advance()
        local entries = {}
        if not ctx:check(TokenKind.RBRACE) then
            repeat
                ctx:skip_newlines()
                if ctx:check(TokenKind.RBRACE) then break end
                if ctx:check(TokenKind.DOT_DOT) then
                    table.insert(entries, ParseExpressions.parse_expression(ctx))
                else
                    local key = ParseExpressions.parse_expression(ctx)
                    ctx:expect(TokenKind.COLON, "esperado ':' no par chave-valor")
                    local val = ParseExpressions.parse_expression(ctx)
                    table.insert(entries, { key = key, value = val })
                end
                ctx:match(TokenKind.COMMA)
                ctx:skip_newlines()
            until ctx:check(TokenKind.RBRACE) or ctx:is_at_end()
        end
        local end_t = ctx:expect(TokenKind.RBRACE, "esperado '}'")
        return ExprSyntax.map(entries, start.span:merge(end_t.span))

    elseif k == TokenKind.AT then
        local start = ctx:advance()
        local id = ctx:expect_field_name("esperado nome do campo após '@'")
        return ExprSyntax.self_field(id.lexeme, start.span:merge(id.span))
    
    elseif k == TokenKind.KW_CHECK then
        local ParseStatements = require("src.syntax.parser.parse_statements")
        return ParseStatements._parse_check(ctx)
    elseif k == TokenKind.KW_NATIVE then
        local ParseStatements = require("src.syntax.parser.parse_statements")
        return ParseStatements._parse_native_lua(ctx, false)
    end

    return nil
end

return ParseExpressions
