-- ============================================================================
-- Zenith Compiler — Parse Declarations
-- Parsing de declarações: var, const, func, struct, trait, enum, etc.
-- ============================================================================

local TokenKind        = require("src.syntax.tokens.token_kind")
local DeclSyntax       = require("src.syntax.ast.decl_syntax")
local ExprSyntax       = require("src.syntax.ast.expr_syntax")
local SK               = require("src.syntax.ast.syntax_kind")
local ParseTypes       = require("src.syntax.parser.parse_types")
local ParseExpressions = require("src.syntax.parser.parse_expressions")
local ParseStatements  = require("src.syntax.parser.parse_statements")

local ParseDeclarations = {}

--- Ponto de entrada: decide se é declaração ou statement.
function ParseDeclarations.parse_declaration_or_statement(ctx)
    ctx:skip_newlines()
    if ctx:is_at_end() then return nil end

    local k = ctx:peek().kind

    -- Namespaces e Imports
    if k == TokenKind.KW_NAMESPACE then
        return ParseDeclarations._parse_namespace(ctx)
    elseif k == TokenKind.KW_IMPORT then
        return ParseDeclarations._parse_import(ctx)
    elseif k == TokenKind.KW_EXPORT then
        return ParseDeclarations._parse_export(ctx)
    elseif k == TokenKind.KW_PUB then
        return ParseDeclarations._parse_pub_decl(ctx)
    elseif k == TokenKind.KW_REDO then
        return ParseDeclarations._parse_redo(ctx)
    end

    -- Variáveis e Constantes
    if k == TokenKind.KW_VAR or k == TokenKind.KW_CONST or k == TokenKind.KW_GLOBAL or
       k == TokenKind.KW_STATE or k == TokenKind.KW_COMPUTED then
        return ParseDeclarations._parse_var(ctx)
    end

    -- Funções
    if k == TokenKind.KW_FUNC or k == TokenKind.KW_ASYNC then
        return ParseDeclarations._parse_func(ctx, false)
    end

    -- Tipos Compostos
    if k == TokenKind.KW_STRUCT then
        return ParseDeclarations._parse_struct(ctx, false)
    elseif k == TokenKind.KW_ENUM then
        return ParseDeclarations._parse_enum(ctx, false)
    elseif k == TokenKind.KW_TRAIT then
        return ParseDeclarations._parse_trait(ctx, false)
    elseif k == TokenKind.KW_APPLY then
        return ParseDeclarations._parse_apply(ctx)
    elseif k == TokenKind.KW_TYPE then
        return ParseDeclarations._parse_type_alias(ctx, false)
    elseif k == TokenKind.KW_UNION then
        return ParseDeclarations._parse_union(ctx, false)
    end

    -- Testes
    if k == TokenKind.KW_GROUP or k == TokenKind.KW_TEST then
        return ParseDeclarations._parse_test(ctx)
    end

    -- Se não for uma declaração, tenta um statement
    return ParseStatements.parse_statement(ctx)
end

function ParseDeclarations._parse_pattern(ctx)
    local k = ctx:peek().kind
    
    if k == TokenKind.LBRACKET then
        local start = ctx:advance()
        local elements = {}
        if not ctx:check(TokenKind.RBRACKET) then
            repeat
                table.insert(elements, ParseDeclarations._parse_pattern(ctx))
            until not ctx:match(TokenKind.COMMA)
        end
        local end_t = ctx:expect(TokenKind.RBRACKET, "esperado ']'")
        return ExprSyntax.list(elements, start.span:merge(end_t.span))
        
    elseif k == TokenKind.LBRACE then
        local start = ctx:advance()
        local fields = {}
        if not ctx:check(TokenKind.RBRACE) then
            repeat
                ctx:skip_newlines()
                local name
                local val
                if ctx:match(TokenKind.AT) then
                    local id = ctx:expect(TokenKind.IDENTIFIER, "esperado nome do campo após '@'")
                    name = id.lexeme
                    val = ExprSyntax.self_field(id.lexeme, id.span)
                else
                    local name_t = ctx:expect(TokenKind.IDENTIFIER, "esperado nome do campo")
                    name = name_t.lexeme
                    if ctx:match(TokenKind.COLON) then
                        val = ParseDeclarations._parse_pattern(ctx)
                    else
                        val = ExprSyntax.identifier(name, name_t.span)
                    end
                end
                table.insert(fields, { name = name, value = val })
            until not ctx:match(TokenKind.COMMA)
        end
        local end_t = ctx:expect(TokenKind.RBRACE, "esperado '}'")
        return ExprSyntax.struct_init("any", fields, start.span:merge(end_t.span)) -- Simplificado para pattern

    elseif k == TokenKind.UNDERSCORE then
        local t = ctx:advance()
        return ExprSyntax.identifier("_", t.span)

    elseif k == TokenKind.IDENTIFIER then
        local id = ctx:advance()
        
        -- Caso especial: Variant Pattern Variant(a, b)
        if ctx:match(TokenKind.LPAREN) then
            local sub_patterns = {}
            if not ctx:check(TokenKind.RPAREN) then
                repeat
                    table.insert(sub_patterns, ParseDeclarations._parse_pattern(ctx))
                until not ctx:match(TokenKind.COMMA)
            end
            local end_t = ctx:expect(TokenKind.RPAREN, "esperado ')'")
            local node = ExprSyntax.call(ExprSyntax.identifier(id.lexeme, id.span), sub_patterns, id.span:merge(end_t.span))
            node.kind = SK.VARIANT_PATTERN
            return node
        end

        -- Caso especial: identificador seguido de { é um Struct Pattern
        if ctx:check(TokenKind.LBRACE) then
            local fields_node = ParseDeclarations._parse_pattern(ctx)
            fields_node.type_name = id.lexeme
            fields_node.span = id.span:merge(fields_node.span)
            return fields_node
        end
        
        local node = ExprSyntax.identifier(id.lexeme, id.span)
        -- Suporte a anotação de tipo no padrão: n: int
        if ctx:match(TokenKind.COLON) then
            local type_node = ParseTypes.parse_type(ctx)
            node.type_annotation = type_node
            node.span = node.span:merge(type_node.span)
        end
        return node
    else
        -- Fallback para literais no match
        return ParseExpressions.parse_expression(ctx)
    end
end

function ParseDeclarations._parse_var(ctx, is_pub)
    local start = ctx:advance()
    local kind = start.kind
    
    local pattern = ParseDeclarations._parse_pattern(ctx)
    local type_node = nil
    
    -- Tipo global da declaração (opcional se houver no padrão ou inicializador)
    if ctx:match(TokenKind.COLON) then
        type_node = ParseTypes.parse_type(ctx)
    end
    
    -- Se não tem tipo global, mas o padrão é um identificador com tipo, move o tipo para o topo
    if not type_node and pattern.kind == SK.IDENTIFIER_EXPR and pattern.type_annotation then
        type_node = pattern.type_annotation
    end
    
    local initializer = nil
    if ctx:match(TokenKind.EQUAL) then
        ctx:skip_newlines()
        initializer = ParseExpressions.parse_expression(ctx)
    end

    local span = start.span:merge(initializer and initializer.span or (type_node and type_node.span or pattern.span))

    -- Helper para criar o nó correto
    local function create_decl(factory)
        local node = factory(pattern.kind == SK.IDENTIFIER_EXPR and pattern.name or nil, type_node, initializer, is_pub, span)
        node.pattern = pattern
        return node
    end

    if kind == TokenKind.KW_VAR then return create_decl(DeclSyntax.var_decl)
    elseif kind == TokenKind.KW_CONST then return create_decl(DeclSyntax.const_decl)
    elseif kind == TokenKind.KW_GLOBAL then return create_decl(DeclSyntax.global_decl)
    elseif kind == TokenKind.KW_STATE then return create_decl(DeclSyntax.state_decl)
    elseif kind == TokenKind.KW_COMPUTED then return create_decl(DeclSyntax.computed_decl)
    end
end

function ParseDeclarations._parse_func(ctx, is_pub, is_prototype)
    local is_async = ctx:match(TokenKind.KW_ASYNC)
    local start = ctx:expect(TokenKind.KW_FUNC, "esperado 'func'")
    
    local name_token = ctx:peek()
    if name_token.kind == TokenKind.IDENTIFIER or name_token.kind:match("^KW_") then
        ctx:advance()
    else
        ctx.diagnostics:report_error("ZT-P001", "esperado nome da função", name_token.span)
    end

    local generic_params = ParseDeclarations._parse_generic_params(ctx)

    ctx:expect(TokenKind.LPAREN, "esperado '('")
    local params = ParseDeclarations._parse_params(ctx)
    ctx:expect(TokenKind.RPAREN, "esperado ')' após parâmetros")
    
    local return_type = nil
    if ctx:match(TokenKind.COLON) or ctx:match(TokenKind.ARROW) then
        return_type = ParseTypes.parse_type(ctx)
    end
    
    ctx:skip_newlines()
    
    local body = nil
    local end_t = ctx:peek(-1)
    
    -- Se for protótipo, o corpo é opcional. Se não for, é obrigatório.
    if not is_prototype then
        ctx:match(TokenKind.KW_DO) -- Opcional por enquanto para compatibilidade
        body = ParseStatements.parse_block(ctx, TokenKind.KW_END)
        end_t = ctx:expect(TokenKind.KW_END, "esperado 'end' após corpo da função")
    else
        -- Heurística para default implementation em Trait: 
        -- Se o próximo token não for NEWLINE, ou se houver um bloco antes do próximo membro.
        -- Simplificado: se houver algo que não seja o fim do trait ou o início de outro membro.
        local next_k = ctx:peek().kind
        if next_k ~= TokenKind.KW_END and next_k ~= TokenKind.KW_FUNC and next_k ~= TokenKind.KW_ASYNC and next_k ~= TokenKind.KW_PUB then
             body = ParseStatements.parse_block(ctx, {TokenKind.KW_END, TokenKind.KW_FUNC, TokenKind.KW_ASYNC, TokenKind.KW_PUB})
             if ctx:match(TokenKind.KW_END) then
                end_t = ctx:peek(-1)
             end
        end
    end
    
    local span = start.span:merge(end_t.span)
    local fn = is_async 
        and DeclSyntax.async_func_decl(name_token.lexeme, params, return_type, body, is_pub, span, generic_params)
        or DeclSyntax.func_decl(name_token.lexeme, params, return_type, body, is_pub, span, generic_params)
    return fn
end

function ParseDeclarations._parse_params(ctx)
    local params = {}
    if ctx:check(TokenKind.RPAREN) then return params end
    repeat
        table.insert(params, ParseDeclarations._parse_param(ctx))
    until not ctx:match(TokenKind.COMMA)
    return params
end

function ParseDeclarations._parse_param(ctx)
    -- Tenta parsear um padrão (identificador, lista ou struct)
    local pattern = ParseDeclarations._parse_pattern(ctx)
    
    local type_node = nil
    if ctx:match(TokenKind.COLON) then
        type_node = ParseTypes.parse_type(ctx)
    end
    
    -- Se o padrão for um identificador com anotação (ex: n: int), extraímos o tipo
    if not type_node and pattern.kind == SK.IDENTIFIER_EXPR and pattern.type_annotation then
        type_node = pattern.type_annotation
    end

    local default_value = nil
    if ctx:match(TokenKind.EQUAL) then
        ctx:skip_newlines()
        default_value = ParseExpressions.parse_expression(ctx)
    end
    
    local span = pattern.span:merge(default_value and default_value.span or (type_node and type_node.span or pattern.span))
    
    local node = DeclSyntax.param_node(pattern.kind == SK.IDENTIFIER_EXPR and pattern.name or nil, type_node, default_value, span)
    node.pattern = pattern
    return node
end

function ParseDeclarations._parse_pub_decl(ctx)
    ctx:advance() -- 'pub'
    local k = ctx:peek().kind
    if k == TokenKind.KW_FUNC or k == TokenKind.KW_ASYNC then
        return ParseDeclarations._parse_func(ctx, true)
    elseif k == TokenKind.KW_VAR or k == TokenKind.KW_CONST or k == TokenKind.KW_GLOBAL or
           k == TokenKind.KW_STATE or k == TokenKind.KW_COMPUTED then
        return ParseDeclarations._parse_var(ctx, true)
    elseif k == TokenKind.KW_STRUCT then
        return ParseDeclarations._parse_struct(ctx, true)
    elseif k == TokenKind.KW_ENUM then
        return ParseDeclarations._parse_enum(ctx, true)
    elseif k == TokenKind.KW_TRAIT then
        return ParseDeclarations._parse_trait(ctx, true)
    elseif k == TokenKind.KW_TYPE then
        return ParseDeclarations._parse_type_alias(ctx, true)
    elseif k == TokenKind.KW_UNION then
        return ParseDeclarations._parse_union(ctx, true)
    end
    return ParseDeclarations.parse_declaration_or_statement(ctx)
end

function ParseDeclarations._parse_struct(ctx, is_pub)
    local start = ctx:advance() -- 'struct'
    local id = ctx:expect(TokenKind.IDENTIFIER)
    
    local generic_params = ParseDeclarations._parse_generic_params(ctx)
    
    local fields, methods = {}, {}
    
    while not ctx:check(TokenKind.KW_END) and not ctx:is_at_end() do
        ctx:skip_newlines()
        -- print("DEBUG: Parsing struct member, next token:", ctx:peek().kind)
        if ctx:check(TokenKind.KW_END) then break end
        
        -- Atributos: @name(args)
        local attributes = {}
        while ctx:match(TokenKind.AT) do
            local attr_start = ctx:peek(-1)
            local attr_name = ctx:expect(TokenKind.IDENTIFIER, "esperado nome do atributo após '@'")
            local args = {}
            if ctx:match(TokenKind.LPAREN) then
                if not ctx:check(TokenKind.RPAREN) then
                    repeat
                        table.insert(args, ParseExpressions.parse_expression(ctx))
                    until not ctx:match(TokenKind.COMMA)
                end
                ctx:expect(TokenKind.RPAREN, "esperado ')' após argumentos do atributo")
            end
            table.insert(attributes, DeclSyntax.attribute_node(attr_name.lexeme, args, attr_start.span:merge(ctx:peek(-1).span)))
            ctx:skip_newlines()
        end

        local member_pub = ctx:match(TokenKind.KW_PUB)
        
        if ctx:check(TokenKind.KW_FUNC) then
            -- OBS: atributos em métodos podem ser suportados futuramente
            table.insert(methods, ParseDeclarations._parse_func(ctx, member_pub))
        else
            local f_id = ctx:expect(TokenKind.IDENTIFIER, "esperado nome do campo")
            ctx:expect(TokenKind.COLON, "esperado ':'")
            local f_type = ParseTypes.parse_type(ctx)
            
            local def = nil
            if ctx:match(TokenKind.EQUAL) then 
                ctx:skip_newlines()
                def = ParseExpressions.parse_expression(ctx) 
            end
            
            -- Validação: where expression
            local condition = nil
            if ctx:match(TokenKind.KW_WHERE) then
                ctx:skip_newlines()
                condition = ParseExpressions.parse_expression(ctx)
            end

            local field_span = f_id.span:merge(ctx:peek(-1).span)
            table.insert(fields, DeclSyntax.field_node(
                f_id.lexeme, f_type, def, member_pub, 
                attributes, condition, field_span
            ))
        end
        ctx:skip_newlines()
    end
    
    local end_t = ctx:expect(TokenKind.KW_END)
    return DeclSyntax.struct_decl(id.lexeme, fields, methods, is_pub, start.span:merge(end_t.span), generic_params)
end

function ParseDeclarations._parse_enum(ctx, is_pub)
    local start = ctx:advance()
    local id = ctx:expect(TokenKind.IDENTIFIER)
    local members = {}
    while not ctx:check(TokenKind.KW_END) and not ctx:is_at_end() do
        ctx:skip_newlines()
        if ctx:check(TokenKind.KW_END) then break end
        
        local m_id = ctx:expect(TokenKind.IDENTIFIER)
        local val = nil
        local params = nil
        
        -- Sum Type: Member(T, U) ou Member(name: T)
        if ctx:match(TokenKind.LPAREN) then
            params = {}
            if not ctx:check(TokenKind.RPAREN) then
                repeat
                    local p_name = nil
                    local p_type = nil
                    -- Suporta nome opcional: Success(value: T) ou Success(T)
                    if ctx:check(TokenKind.IDENTIFIER) and ctx:peek(1).kind == TokenKind.COLON then
                        p_name = ctx:advance().lexeme
                        ctx:advance() -- consume ':'
                        p_type = ParseTypes.parse_type(ctx)
                    else
                        p_type = ParseTypes.parse_type(ctx)
                    end
                    table.insert(params, { name = p_name, type = p_type })
                until not ctx:match(TokenKind.COMMA)
            end
            ctx:expect(TokenKind.RPAREN, "esperado ')' após parâmetros do membro de enum")
        elseif ctx:match(TokenKind.EQUAL) then 
            val = ParseExpressions.parse_expression(ctx) 
        end
        
        table.insert(members, DeclSyntax.enum_member(m_id.lexeme, val, params, m_id.span))
        ctx:skip_newlines()
    end
    local end_t = ctx:expect(TokenKind.KW_END)
    return DeclSyntax.enum_decl(id.lexeme, members, is_pub, start.span:merge(end_t.span))
end

function ParseDeclarations._parse_trait(ctx, is_pub)
    local start = ctx:advance()
    local id = ctx:expect(TokenKind.IDENTIFIER)
    local methods = {}
    while not ctx:check(TokenKind.KW_END) and not ctx:is_at_end() do
        ctx:skip_newlines()
        if ctx:check(TokenKind.KW_END) then break end
        if ctx:match(TokenKind.KW_PUB) or ctx:check(TokenKind.KW_FUNC) or ctx:check(TokenKind.KW_ASYNC) then
            -- Note: na v1.0-alpha, traits podem ter apenas assinaturas (is_prototype = true)
            local m_pub = ctx:match(TokenKind.KW_PUB)
            table.insert(methods, ParseDeclarations._parse_func(ctx, m_pub, true))
        else
            ctx:advance() -- Evitar loop infinito
        end
        ctx:skip_newlines()
    end
    local end_t = ctx:expect(TokenKind.KW_END)
    return DeclSyntax.trait_decl(id.lexeme, methods, is_pub, start.span:merge(end_t.span))
end

function ParseDeclarations._parse_apply(ctx)
    local start = ctx:advance() -- 'apply'
    local trait_id = ctx:expect(TokenKind.IDENTIFIER, "esperado nome da trait")
    ctx:expect(TokenKind.KW_TO, "esperado 'to' após o nome da trait")
    local struct_id = ctx:expect(TokenKind.IDENTIFIER, "esperado nome da struct")
    
    local methods = {}
    ctx:skip_newlines()
    
    while not ctx:check(TokenKind.KW_END) and not ctx:is_at_end() do
        ctx:skip_newlines()
        if ctx:check(TokenKind.KW_END) then break end
        
        if ctx:check(TokenKind.KW_PUB) or ctx:check(TokenKind.KW_FUNC) or ctx:check(TokenKind.KW_ASYNC) then
            local is_pub = ctx:match(TokenKind.KW_PUB)
            table.insert(methods, ParseDeclarations._parse_func(ctx, is_pub))
        else
            ctx:advance() -- Consumir token inesperado para evitar loop infinito
            ctx.diagnostics:report_error("ZT-P001", "apenas funções podem ser declaradas dentro de um bloco 'apply'", ctx:peek(-1).span)
        end
        ctx:skip_newlines()
    end
    
    local end_t = ctx:expect(TokenKind.KW_END, "esperado 'end' ao final do bloco 'apply'")
    return DeclSyntax.apply_decl(trait_id.lexeme, struct_id.lexeme, methods, start.span:merge(end_t.span))
end
function ParseDeclarations._parse_type_alias(ctx, is_pub)
    local start = ctx:expect(TokenKind.KW_TYPE, "esperado 'type'")
    local name = ctx:expect(TokenKind.IDENTIFIER, "esperado nome do alias").lexeme

    local generic_params = ParseDeclarations._parse_generic_params(ctx)

    ctx:expect(TokenKind.EQUAL, "esperado '=' após nome do alias")
    local target = ParseTypes.parse_type(ctx)

    return DeclSyntax.type_alias_decl(name, target, is_pub, start.span:merge(target.span), generic_params)
end

function ParseDeclarations._parse_union(ctx, is_pub)
    local start = ctx:expect(TokenKind.KW_UNION, "esperado 'union'")
    local name = ctx:expect(TokenKind.IDENTIFIER, "esperado nome da união").lexeme

    local generic_params = ParseDeclarations._parse_generic_params(ctx)

    ctx:expect(TokenKind.EQUAL, "esperado '=' após nome da união")
    local target = ParseTypes.parse_type(ctx)

    return DeclSyntax.union_decl(name, target, is_pub, start.span:merge(target.span), generic_params)
end

function ParseDeclarations._parse_generic_params(ctx)
    if not ctx:match(TokenKind.LESS) then return nil end
    
    local params = {}
    repeat
        ctx:skip_newlines()
        local name_t = ctx:expect(TokenKind.IDENTIFIER, "esperado nome do parâmetro genérico")
        local constraint = nil
        
        -- Suporte a restrição: T where T is Trait
        if ctx:match(TokenKind.KW_WHERE) then
            local t_id = ctx:expect(TokenKind.IDENTIFIER, "esperado nome do parâmetro genérico após 'where'")
            if t_id.lexeme ~= name_t.lexeme then
                ctx.diagnostics:report_error("ZT-P001", "nome do parâmetro no 'where' deve coincidir com o declarado", t_id.span)
            end
            ctx:expect(TokenKind.KW_IS, "esperado 'is' após o nome do parâmetro no 'where'")
            constraint = ParseTypes.parse_type(ctx)
        end
        
        table.insert(params, { name = name_t.lexeme, constraint = constraint, span = name_t.span })
    until not ctx:match(TokenKind.COMMA)
    
    ctx:expect(TokenKind.GREATER, "esperado '>' após parâmetros genéricos")
    return params
end

function ParseDeclarations._parse_redo(ctx)
    local start = ctx:advance()
    ctx:expect(TokenKind.KW_FUNC)
    local t_id = ctx:expect(TokenKind.IDENTIFIER)
    ctx:expect(TokenKind.DOT)
    local m_id = ctx:expect(TokenKind.IDENTIFIER)
    ctx:expect(TokenKind.LPAREN)
    local params = ParseDeclarations._parse_params(ctx)
    ctx:expect(TokenKind.RPAREN)
    local ret = nil
    if ctx:match(TokenKind.COLON) then ret = ParseTypes.parse_type(ctx) end
    
    ctx:skip_newlines()
    local body = ParseStatements.parse_block(ctx, TokenKind.KW_END)
    local end_t = ctx:expect(TokenKind.KW_END)
    
    return DeclSyntax.redo_decl(t_id.lexeme, m_id.lexeme, params, ret, body, start.span:merge(end_t.span))
end

function ParseDeclarations._parse_import(ctx)
    local start = ctx:advance()
    local path = ""
    if ctx:match(TokenKind.STRING_LITERAL) then path = ctx:peek(-1).value
    else
        local parts = {}
        repeat table.insert(parts, ctx:expect(TokenKind.IDENTIFIER).lexeme)
        until not ctx:match(TokenKind.DOT)
        path = table.concat(parts, ".")
    end
    local alias = nil
    if ctx:match(TokenKind.KW_AS) then alias = ctx:expect(TokenKind.IDENTIFIER).lexeme end
    return DeclSyntax.import_decl(path, alias, start.span:merge(ctx:peek(-1).span))
end

function ParseDeclarations._parse_namespace(ctx)
    local start = ctx:advance()
    local parts = {}
    repeat table.insert(parts, ctx:expect(TokenKind.IDENTIFIER).lexeme)
    until not ctx:match(TokenKind.DOT)
    return DeclSyntax.namespace_decl(table.concat(parts, "."), start.span:merge(ctx:peek(-1).span))
end

function ParseDeclarations._parse_export(ctx)
    local start = ctx:advance()
    local decl = ParseDeclarations.parse_declaration_or_statement(ctx)
    return DeclSyntax.export_decl(decl, start.span:merge(decl.span))
end

function ParseDeclarations._parse_test(ctx)
    local start = ctx:advance()
    local id = ctx:expect(TokenKind.STRING_LITERAL)
    ctx:skip_newlines()
    local body = ParseStatements.parse_block(ctx, TokenKind.KW_END)
    local end_t = ctx:expect(TokenKind.KW_END)
    if start.kind == TokenKind.KW_GROUP then return DeclSyntax.group_decl(id.value, body, start.span:merge(end_t.span))
    else return DeclSyntax.test_decl(id.value, body, start.span:merge(end_t.span)) end
end

return ParseDeclarations
