-- ============================================================================
-- Zenith Compiler — Lowerer (IR Transformation)
-- Transforma a AST rica em uma IR (Lowered AST) mais simples e otimizada.
-- ============================================================================

local SK = require("src.syntax.ast.syntax_kind")
local ExprSyntax = require("src.syntax.ast.expr_syntax")
local StmtSyntax = require("src.syntax.ast.stmt_syntax")
local DeclSyntax = require("src.syntax.ast.decl_syntax")

local Lowerer = {}
Lowerer.__index = Lowerer

function Lowerer.new(diagnostics)
    local self = setmetatable({}, Lowerer)
    self.diagnostics = diagnostics
    return self
end

--- Ponto de entrada: transforma a compilation unit.
function Lowerer:lower(unit)
    return self:_lower_node(unit)
end

function Lowerer:_lower_node(node)
    if not node then return nil end
    
    local method_name = "_lower_" .. node.kind:lower()
    local lowered
    
    if self[method_name] then
        lowered = self[method_name](self, node)
    else
        -- Fallback seguro: clona o nó preservando campos básicos
        lowered = { kind = node.kind, span = node.span }
        for k, v in pairs(node) do
            if k ~= "kind" and k ~= "span" and type(v) ~= "table" then
                lowered[k] = v
            end
        end
    end
    
    -- Se o visitante retornou um bloco de desugaring, o chamador (como _lower_block) 
    -- deve estar preparado para achatar os statements.
    if type(lowered) == "table" and lowered.kind == "BLOCK" and lowered.statements then
        return lowered
    end

    -- Garantir preservação de metadados semânticos (Merge final no resultado)
    if type(lowered) == "table" and type(node) == "table" then
        local fields_to_preserve = {
            "symbol", "type_info", "is_ufcs", "is_capture", 
            "resolved_args", "is_pub", "is_async", "params", "return_type",
            "target_type"
        }
        for _, field in ipairs(fields_to_preserve) do
            if node[field] ~= nil and lowered[field] == nil then
                lowered[field] = node[field]
            end
        end


        -- SINCRONIZAÇÃO CRÍTICA: Se o nó tem um símbolo, o símbolo deve agora
        -- apontar para esta nova versão (lowered) da declaração.
        if lowered.symbol and (lowered.kind:match("_DECL$") or lowered.kind == SK.FUNC_DECL) then
            lowered.symbol.declaration = lowered
        end
    end
    
    return lowered
end

-- ============================================================================
-- Visitantes de Declaração
-- ============================================================================

function Lowerer:_lower_compilation_unit(node)
    local decls = {}
    for _, d in ipairs(node.declarations) do
        local lowered = self:_lower_node(d)
        if lowered then table.insert(decls, lowered) end
    end
    return { kind = SK.COMPILATION_UNIT, declarations = decls, span = node.span }
end

function Lowerer:_lower_func_decl(node)
    local body = nil
    local initial_stmts = {}
    
    -- Se houver destruturação em parâmetros, gera stmts iniciais
    for _, p in ipairs(node.params or {}) do
        if p.pattern and p.pattern.kind ~= SK.IDENTIFIER_EXPR then
            local _, bindings = self:_gen_pattern_logic(p.pattern, p.name)
            for _, b in ipairs(bindings) do table.insert(initial_stmts, b) end
        end
    end

    if node.body then
        body = {}
        -- Injeta destruturações primeiro
        for _, s in ipairs(initial_stmts) do table.insert(body, self:_lower_node(s)) end
        -- Corpo original
        for _, s in ipairs(node.body) do
            table.insert(body, self:_lower_node(s))
        end
    end
    
    local params = {}
    for _, p in ipairs(node.params or {}) do
        table.insert(params, self:_lower_node(p))
    end

    return {
        kind = node.kind,
        name = node.name,
        params = params,
        return_type = node.return_type,
        body = body,
        is_async = node.is_async,
        is_pub = node.is_pub,
        span = node.span
    }
end

function Lowerer:_lower_param_node(node)
    return {
        kind = node.kind,
        name = node.name,
        type_node = node.type_node,
        default_value = self:_lower_node(node.default_value),
        span = node.span
    }
end

function Lowerer:_lower_var_decl(node)
    return {
        kind = node.kind,
        name = node.name,
        pattern = self:_lower_node(node.pattern),
        type_node = node.type_node,
        initializer = self:_lower_node(node.initializer),
        is_pub = node.is_pub,
        span = node.span
    }
end

function Lowerer:_lower_const_decl(node) return self:_lower_var_decl(node) end
function Lowerer:_lower_global_decl(node) return self:_lower_var_decl(node) end
function Lowerer:_lower_state_decl(node) return self:_lower_var_decl(node) end

function Lowerer:_lower_computed_decl(node)
    return {
        kind = SK.COMPUTED_DECL,
        name = node.name,
        expression = self:_lower_node(node.expression),
        is_pub = node.is_pub,
        span = node.span
    }
end

function Lowerer:_lower_struct_decl(node)
    local methods = {}
    for _, m in ipairs(node.methods) do
        table.insert(methods, self:_lower_node(m))
    end
    return {
        kind = SK.STRUCT_DECL,
        name = node.name,
        fields = node.fields,
        methods = methods,
        is_pub = node.is_pub,
        span = node.span
    }
end

function Lowerer:_lower_apply_decl(node)
    local methods = {}
    for _, m in ipairs(node.methods or {}) do
        table.insert(methods, self:_lower_node(m))
    end
    return {
        kind = SK.APPLY_DECL,
        trait_name = node.trait_name,
        struct_name = node.struct_name,
        methods = methods,
        span = node.span
    }
end

function Lowerer:_lower_type_alias_decl(node)
    return {
        kind = SK.TYPE_ALIAS_DECL,
        name = node.name,
        target_type = node.target_type, -- Tipo não muda no lowering
        is_pub = node.is_pub,
        span = node.span
    }
end

function Lowerer:_lower_union_decl(node)
    return {
        kind = SK.UNION_DECL,
        name = node.name,
        union_type = node.union_type,
        is_pub = node.is_pub,
        span = node.span
    }
end

-- ============================================================================
-- Visitantes de Statement
-- ============================================================================

function Lowerer:_lower_expr_stmt(node)
    return { kind = SK.EXPR_STMT, expression = self:_lower_node(node.expression), span = node.span }
end

function Lowerer:_lower_watch_stmt(node)
    return {
        kind = SK.WATCH_STMT,
        body = self:_lower_block(node.body),
        span = node.span
    }
end

function Lowerer:_lower_return_stmt(node)
    return {
        kind = SK.RETURN_STMT,
        expression = self:_lower_node(node.expression),
        span = node.span
    }
end

function Lowerer:_lower_assign_stmt(node)
    return {
        kind = SK.ASSIGN_STMT,
        target = self:_lower_node(node.target or node.left),
        value = self:_lower_node(node.value or node.right),
        span = node.span
    }
end

function Lowerer:_lower_attempt_stmt(node)
    local rescue = nil
    if node.rescue_clause then
        rescue = {
            kind = node.rescue_clause.kind,
            error_name = node.rescue_clause.error_name,
            body = self:_lower_block(node.rescue_clause.body),
            span = node.rescue_clause.span
        }
    end
    return {
        kind = SK.ATTEMPT_STMT,
        body = self:_lower_block(node.body),
        rescue_clause = rescue,
        span = node.span
    }
end

function Lowerer:_lower_match_stmt(node)
    -- Desugaring: match x vira { local _m = x; if cond then ... end }
    -- Para v1.0-alpha, vamos retornar um bloco ou um IF_STMT direto se possível.
    -- No Zenith, o Lowerer pode retornar uma lista de statements para substituir um único.
    
    local expr = self:_lower_node(node.expression)
    local match_var = "_m" -- Variável temporária para o valor do match
    
    local first_if = nil
    local current_if = nil
    
    for i, case in ipairs(node.cases) do
        local cond_expr = nil
        local all_bindings = {}
        
        for _, pattern in ipairs(case.patterns) do
            local p_cond, p_bindings = self:_gen_pattern_logic(pattern, match_var)
            if not cond_expr then cond_expr = p_cond
            else cond_expr = self:_lower_binary(cond_expr, "or", p_cond) end
            for _, b in ipairs(p_bindings) do table.insert(all_bindings, b) end
        end
        
        local body = {}
        -- Adicionar os bindings no início do corpo do case
        for _, b in ipairs(all_bindings) do table.insert(body, b) end
        -- Adicionar o corpo original
        for _, s in ipairs(self:_lower_block(case.body)) do table.insert(body, s) end
        
        if i == 1 then
            first_if = StmtSyntax.if_stmt(cond_expr, body, {}, nil, case.span)
            current_if = first_if
        else
            table.insert(first_if.elif_clauses, StmtSyntax.elif_clause(cond_expr, body, case.span))
        end
    end
    
    if node.else_clause then
        first_if.else_clause = StmtSyntax.else_clause(self:_lower_block(node.else_clause.body), node.else_clause.span)
    end
    
    -- Retornamos um bloco que declara a variável temporária e então executa o IF
    return {
        kind = "BLOCK",
        statements = {
            DeclSyntax.var_decl(match_var, nil, expr, false, node.span),
            first_if
        }
    }
end

--- Auxiliar para criar BINARY_EXPR
function Lowerer:_lower_binary(left, op_lexeme, right)
    local Token = require("src.syntax.tokens.token")
    local TokenKind = require("src.syntax.tokens.token_kind")
    local op_kind = op_lexeme == "or" and TokenKind.KW_OR or (op_lexeme == "and" and TokenKind.KW_AND or TokenKind.EQUAL_EQUAL)
    return ExprSyntax.binary(left, Token.new(op_kind, op_lexeme, nil, left.span), right, left.span:merge(right.span))
end

--- Gera a lógica AST para um padrão.
function Lowerer:_gen_pattern_logic(pattern, access_path_node)
    -- Se o caminho de acesso for uma string, transforma em Identifier
    if type(access_path_node) == "string" then
        access_path_node = ExprSyntax.identifier(access_path_node, pattern.span)
    end

    if pattern.kind == SK.IDENTIFIER_EXPR then
        if pattern.name == "_" then
            return ExprSyntax.literal(true, "bool", pattern.span), {}
        elseif pattern.is_capture then
            -- local x = access_path
            local b = StmtSyntax.assign(ExprSyntax.identifier(pattern.name, pattern.span), access_path_node, pattern.span)
            return ExprSyntax.literal(true, "bool", pattern.span), { b }
        else
            -- Enum member ou valor fixo: access_path == pattern
            return self:_lower_binary(access_path_node, "==", pattern), {}
        end

    elseif pattern.kind == SK.LITERAL_EXPR then
        return self:_lower_binary(access_path_node, "==", pattern), {}

    elseif pattern.kind == SK.LIST_EXPR then
        -- 1. Checar se é tabela: zt.is(access_path, "table")
        local is_table = ExprSyntax.call(
            ExprSyntax.identifier("zt.is", pattern.span), -- Acesso direto via identificador para evitar : no Codegen
            { access_path_node, ExprSyntax.literal("table", "text", pattern.span) },
            pattern.span
        )
        
        local cond = is_table
        local bindings = {}
        
        for i, el in ipairs(pattern.elements) do
            if el.kind == SK.REST_EXPR then
                -- ..resto
                if el.expression.kind == SK.IDENTIFIER_EXPR then
                    -- local resto = zt.slice(access_path, i-1, #access_path)
                    local slice_call = ExprSyntax.call(
                        ExprSyntax.identifier("zt.slice", el.span),
                        { 
                            access_path_node, 
                            ExprSyntax.literal(i - 1, "int", el.span),
                            ExprSyntax.len(access_path_node, el.span)
                        },
                        el.span
                    )
                    table.insert(bindings, StmtSyntax.assign(el.expression, slice_call, el.span))
                end
            else
                -- access_path[i]
                local sub_path = ExprSyntax.member(access_path_node, ExprSyntax.literal(i, "int", el.span), el.span)
                local sub_cond, sub_bindings = self:_gen_pattern_logic(el, sub_path)
                cond = self:_lower_binary(cond, "and", sub_cond)
                for _, b in ipairs(sub_bindings) do table.insert(bindings, b) end
            end
        end
        
        return cond, bindings

    elseif pattern.kind == SK.STRUCT_INIT_EXPR then
        -- Checar tipo da struct (simplificado: zt.is(access_path, "table"))
        local cond = ExprSyntax.call(
            ExprSyntax.identifier("zt.is", pattern.span),
            { access_path_node, ExprSyntax.literal("table", "text", pattern.span) },
            pattern.span
        )
        local bindings = {}
        
        for _, f in ipairs(pattern.fields) do
            -- access_path.field
            local sub_path = ExprSyntax.member(access_path_node, f.name, f.value.span)
            local sub_cond, sub_bindings = self:_gen_pattern_logic(f.value, sub_path)
            cond = self:_lower_binary(cond, "and", sub_cond)
            for _, b in ipairs(sub_bindings) do table.insert(bindings, b) end
        end
        
        return cond, bindings
    end

    return ExprSyntax.literal(true, "bool", pattern.span), {}
end

function Lowerer:_lower_check_stmt(node)
    return {
        kind = SK.CHECK_STMT,
        condition = self:_lower_node(node.condition),
        else_body = self:_lower_block(node.else_body),
        span = node.span
    }
end

function Lowerer:_lower_if_stmt(node)
    local elifs = {}
    for _, e in ipairs(node.elif_clauses or {}) do
        table.insert(elifs, {
            kind = e.kind,
            condition = self:_lower_node(e.condition),
            body = self:_lower_block(e.body),
            span = e.span
        })
    end
    local else_c = nil
    if node.else_clause then
        else_c = {
            kind = node.else_clause.kind,
            body = self:_lower_block(node.else_clause.body),
            span = node.else_clause.span
        }
    end
    return {
        kind = SK.IF_STMT,
        condition = self:_lower_node(node.condition),
        body = self:_lower_block(node.body),
        elif_clauses = elifs,
        else_clause = else_c,
        span = node.span
    }
end

function Lowerer:_lower_while_stmt(node)
    return {
        kind = SK.WHILE_STMT,
        condition = self:_lower_node(node.condition),
        body = self:_lower_block(node.body),
        span = node.span
    }
end

function Lowerer:_lower_for_in_stmt(node)
    return {
        kind = SK.FOR_IN_STMT,
        variables = node.variables,
        iterable = self:_lower_node(node.iterable),
        body = self:_lower_block(node.body),
        span = node.span
    }
end

function Lowerer:_lower_compound_assign_stmt(node)
    return {
        kind = SK.COMPOUND_ASSIGN_STMT,
        target = self:_lower_node(node.target or node.left),
        value = self:_lower_node(node.value or node.right),
        operator = node.operator,
        span = node.span
    }
end

function Lowerer:_lower_block(node)
    local stmts = {}
    for _, s in ipairs(node or {}) do
        local lowered = self:_lower_node(s)
        if lowered then
            if lowered.kind == "BLOCK" and lowered.statements then
                for _, sub in ipairs(lowered.statements) do
                    table.insert(stmts, sub)
                end
            else
                table.insert(stmts, lowered)
            end
        end
    end
    return stmts
end

function Lowerer:_lower_group_expr(node)
    return self:_lower_node(node.expression)
end

function Lowerer:_lower_is_expr(node)
    return {
        kind = SK.IS_EXPR,
        expression = self:_lower_node(node.expression),
        target_type = node.target_type, -- Preservar explicitamente
        type_node = node.type_node,
        is_not = node.is_not,
        span = node.span
    }
end

function Lowerer:_lower_as_expr(node)
    return {
        kind = SK.AS_EXPR,
        expression = self:_lower_node(node.expression),
        target_type = node.target_type, -- Preservar explicitamente
        type_node = node.type_node,
        span = node.span
    }
end

-- ============================================================================
-- Otimizações: Constant Folding
-- ============================================================================

function Lowerer:_lower_binary_expr(node)
    local left = self:_lower_node(node.left)
    local right = self:_lower_node(node.right)
    local op = node.operator.lexeme

    -- Constant Folding para Números
    if left.kind == SK.LITERAL_EXPR and right.kind == SK.LITERAL_EXPR then
        local lv = left.value
        local rv = right.value
        
        if type(lv) == "number" and type(rv) == "number" then
            if op == "+" then return ExprSyntax.literal(lv + rv, "int", node.span) end
            if op == "-" then return ExprSyntax.literal(lv - rv, "int", node.span) end
            if op == "*" then return ExprSyntax.literal(lv * rv, "int", node.span) end
            if op == "/" and rv ~= 0 then return ExprSyntax.literal(lv / rv, "float", node.span) end
        end
        
        -- Constant Folding para Strings (Concatenação)
        if op == "+" and type(lv) == "string" and type(rv) == "string" then
            return ExprSyntax.literal(lv .. rv, "text", node.span)
        end
    end

    return { 
        kind = SK.BINARY_EXPR, 
        left = left, 
        right = right, 
        operator = node.operator, 
        span = node.span
    }
end

function Lowerer:_lower_unary_expr(node)
    local operand = self:_lower_node(node.operand)
    local op = node.operator.lexeme
    
    if operand.kind == SK.LITERAL_EXPR then
        local v = operand.value
        if op == "-" and type(v) == "number" then return ExprSyntax.literal(-v, operand.literal_type, node.span) end
        if op == "not" and type(v) == "boolean" then return ExprSyntax.literal(not v, "bool", node.span) end
    end
    
    return { kind = SK.UNARY_EXPR, operand = operand, operator = node.operator, span = node.span }
end

function Lowerer:_lower_call_expr(node)
    local callee = self:_lower_node(node.callee)
    local args = {}
    local res_args = node.resolved_args or node.arguments
    for _, arg in ipairs(res_args) do
        if type(arg) == "table" and arg.kind == "NAMED" then
            table.insert(args, { kind = "NAMED", name = arg.name, value = self:_lower_node(arg.value), span = arg.span })
        else
            table.insert(args, self:_lower_node(arg))
        end
    end
    return { 
        kind = SK.CALL_EXPR, 
        callee = callee, 
        arguments = args,
        generic_args = node.generic_args, -- Preservar
        span = node.span
    }
end

function Lowerer:_lower_bang_expr(node)
    return {
        kind = SK.BANG_EXPR,
        expression = self:_lower_node(node.expression),
        span = node.span
    }
end

function Lowerer:_lower_rest_expr(node)
    return {
        kind = SK.REST_EXPR,
        expression = self:_lower_node(node.expression),
        span = node.span
    }
end

function Lowerer:_lower_len_expr(node)
    return {
        kind = SK.LEN_EXPR,
        expression = self:_lower_node(node.expression),
        span = node.span
    }
end

function Lowerer:_lower_list_expr(node)
    local elements = {}
    for _, el in ipairs(node.elements or {}) do
        table.insert(elements, self:_lower_node(el))
    end
    return {
        kind = SK.LIST_EXPR,
        elements = elements,
        span = node.span
    }
end

function Lowerer:_lower_struct_init_expr(node)
    local fields = {}
    for _, f in ipairs(node.fields or {}) do
        table.insert(fields, {
            name = f.name,
            value = self:_lower_node(f.value)
        })
    end
    return {
        kind = SK.STRUCT_INIT_EXPR,
        type_name = node.type_name,
        fields = fields,
        span = node.span
    }
end

function Lowerer:_lower_map_expr(node)
    local pairs = {}
    for _, p in ipairs(node.pairs or {}) do
        table.insert(pairs, {
            key = self:_lower_node(p.key),
            value = self:_lower_node(p.value)
        })
    end
    return {
        kind = SK.MAP_EXPR,
        pairs = pairs,
        span = node.span
    }
end

function Lowerer:_lower_member_expr(node)
    return {
        kind = SK.MEMBER_EXPR,
        object = self:_lower_node(node.object),
        member_name = node.member_name,
        span = node.span
    }
end

function Lowerer:_lower_lambda_expr(node)
    local body = nil
    local initial_stmts = {}
    
    for _, p in ipairs(node.params or {}) do
        if p.pattern and p.pattern.kind ~= SK.IDENTIFIER_EXPR then
            local _, bindings = self:_gen_pattern_logic(p.pattern, p.name)
            for _, b in ipairs(bindings) do table.insert(initial_stmts, b) end
        end
    end

    if type(node.body) == "table" and node.body.kind then
        -- Expressão única: se houver destruturação, vira bloco
        local expr = self:_lower_node(node.body)
        if #initial_stmts > 0 then
            body = {}
            for _, s in ipairs(initial_stmts) do table.insert(body, self:_lower_node(s)) end
            table.insert(body, StmtSyntax.return_stmt(expr, expr.span))
        else
            body = expr
        end
    else
        -- Bloco
        body = {}
        for _, s in ipairs(initial_stmts) do table.insert(body, self:_lower_node(s)) end
        for _, s in ipairs(node.body or {}) do
            table.insert(body, self:_lower_node(s))
        end
    end
    
    local params = {}
    for _, p in ipairs(node.params or {}) do
        table.insert(params, self:_lower_node(p))
    end

    return {
        kind = SK.LAMBDA_EXPR,
        params = params,
        body = body,
        span = node.span
    }
end

function Lowerer:_lower_identifier_expr(node)
    return {
        kind = SK.IDENTIFIER_EXPR,
        name = node.name,
        span = node.span
    }
end

return Lowerer
