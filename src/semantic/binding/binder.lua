-- ============================================================================
-- Zenith Compiler — Binder
-- Analisador Semântico: Resolve nomes e verifica tipos.
-- ============================================================================

local SK            = require("src.syntax.ast.syntax_kind")
local DeclSyntax    = require("src.syntax.ast.decl_syntax")
local ExprSyntax    = require("src.syntax.ast.expr_syntax")
local StmtSyntax    = require("src.syntax.ast.stmt_syntax")
local Symbol        = require("src.semantic.symbols.symbol")
local Scope         = require("src.semantic.symbols.scope")
local BuiltinTypes  = require("src.semantic.types.builtin_types")
local ZenithType    = require("src.semantic.types.zenith_type")
local TypeChecker   = require("src.semantic.types.type_checker")
local DiagnosticBag = require("src.diagnostics.diagnostic_bag")

local Binder = {}
Binder.__index = Binder

function Binder.new(diagnostics, module_manager)
    local self = setmetatable({}, Binder)
    self.diagnostics = diagnostics or DiagnosticBag.new()
    self.module_manager = module_manager
    self.skip_prelude = false -- Flag para evitar recursão no std.core
    
    -- Escopo raiz inicial
    self.scope = Scope.new(Scope.Kind.GLOBAL, nil)
    
    self.current_func = nil
    self.current_struct = nil
    self.current_alias = nil
    self.current_namespace = nil
    return self
end

--- Ponto de entrada: resolve nomes e vincula tipos na CompilationUnit.
function Binder:bind(unit, module_name)
    -- Discovery: carrega prelude primeiro para popular o escopo global
    if not self.skip_prelude and self.module_manager then
        local prelude_scope = self.module_manager:get_prelude_scope()
        if prelude_scope then
            -- Copia símbolos da prelude para o escopo global para simplicidade
            for name, sym in pairs(prelude_scope.symbols) do
                self.scope:define(sym)
            end
        end
    end

    -- Se não houver namespace explícito, usamos o nome do módulo/arquivo
    self.current_namespace = module_name
    
    -- Passagem 1: Registrar declarações globais e descobrir Namespace
    for _, decl in ipairs(unit.declarations) do
        if decl.kind == SK.NAMESPACE_DECL then
            self.current_namespace = decl.name
        end
    end
    
    -- Se tivermos um namespace, criamos um escopo para ele no GLOBAL
    if self.current_namespace then
        local ns_scope = Scope.new(Scope.Kind.NAMESPACE, self.scope)
        self.scope = ns_scope
    end

    -- Registrar declarações globais (Passagem 1)
    for _, decl in ipairs(unit.declarations) do
        self:_declare(decl)
    end

    -- Passagem 1.2: Vincular assinaturas de membros (Campos de Structs, Métodos de Traits, Aliases)
    -- Importante para que o 'apply' e 'generic constraints' conheçam a estrutura dos tipos.
    for _, decl in ipairs(unit.declarations) do
        if decl.kind == SK.STRUCT_DECL or decl.kind == SK.TRAIT_DECL or 
           decl.kind == SK.ENUM_DECL or decl.kind == SK.TYPE_ALIAS_DECL or 
           decl.kind == SK.UNION_DECL then
            self.only_signatures = true
            self:_bind_node(decl)
            self.only_signatures = false
        end
    end

    -- Passagem 1.5: Registrar implementações de Traits (apply)
    for _, decl in ipairs(unit.declarations) do
        if decl.kind == SK.APPLY_DECL then
            self:_bind_node(decl)
        end
    end
    
    -- Passagem 2: Verificar corpos de funções e inicializadores
    local last_type = BuiltinTypes.VOID
    for _, decl in ipairs(unit.declarations) do
        -- Pula Aliases e Uniões na passagem 2 (já tratados ou dinâmicos)
        if decl.kind == SK.TYPE_ALIAS_DECL or decl.kind == SK.UNION_DECL then
            -- Skip
        elseif decl.kind == SK.FUNC_DECL or decl.kind == SK.ASYNC_FUNC_DECL or 
           decl.kind == SK.VAR_DECL or decl.kind == SK.CONST_DECL or
           decl.kind == SK.GLOBAL_DECL or decl.kind == SK.STATE_DECL or
           decl.kind == SK.COMPUTED_DECL then
            last_type = self:_bind_node(decl)
        elseif decl.kind == SK.STRUCT_DECL then
            -- Agora vincula os corpos dos métodos da struct
            self.skip_fields = true
            last_type = self:_bind_node(decl)
            self.skip_fields = false
        end
    end
    
    -- Regista no manager se houver
    if self.module_manager and self.current_namespace then
        self.module_manager:register_scope(self.current_namespace, self.scope)
    end

    return self.scope, self.diagnostics, last_type
end

-- ============================================================================
-- Passagem 1 — Declaração (Coleta de Símbolos)
-- ============================================================================

function Binder:_declare(node)
    local symbol
    if node.kind == SK.FUNC_DECL or node.kind == SK.ASYNC_FUNC_DECL then
        local return_type = self:_resolve_type(node.return_type)
        symbol = Symbol.func(node.name, node.params, return_type, node.span)
        
    elseif node.kind == SK.STRUCT_DECL then
        symbol = Symbol.struct(node.name, {}, {}, node.span)
        symbol.type_info = ZenithType.new(ZenithType.Kind.STRUCT, node.name, { symbol = symbol })
    
    elseif node.kind == SK.ENUM_DECL then
        symbol = Symbol.enum(node.name, node.members, node.span)
        symbol.type_info = ZenithType.new(ZenithType.Kind.ENUM, node.name, { symbol = symbol })

    elseif node.kind == SK.TRAIT_DECL then
        symbol = Symbol.trait(node.name, node.methods, node.span)
        symbol.type_info = ZenithType.new(ZenithType.Kind.TRAIT, node.name, { symbol = symbol })

    elseif node.kind == SK.TYPE_ALIAS_DECL or node.kind == SK.UNION_DECL then
        -- Criamos o símbolo vazio. A resolução real será na Passagem 1.2
        symbol = Symbol.alias(node.name, BuiltinTypes.ANY, node.is_pub, node.span)
        symbol.generic_params = node.generic_params

    elseif node.kind == SK.VAR_DECL or node.kind == SK.CONST_DECL or node.kind == SK.GLOBAL_DECL or
           node.kind == SK.STATE_DECL or node.kind == SK.COMPUTED_DECL then
        -- Não resolve o tipo ainda (será feito no _bind_var_decl na passagem 2)
        local type_info = BuiltinTypes.ANY
        symbol = (node.kind == SK.CONST_DECL) 
            and Symbol.constant(node.name, type_info, node.is_pub, node.span)
            or Symbol.variable(node.name, type_info, node.is_pub, node.span)
            
    elseif node.kind == SK.IMPORT_DECL then
        -- O nome no escopo local será o alias ou a última parte do path
        local parts = {}
        for part in node.path:gmatch("[^%.]+") do table.insert(parts, part) end
        local name = node.alias or parts[#parts]
        symbol = Symbol.new(Symbol.Kind.MODULE, name, { span = node.span })
    end

    if symbol then
        symbol.declaration = node
        symbol.is_pub = node.is_pub or false
        if not self.scope:define(symbol) then
            self.diagnostics:report_error("ZT-S001", 
                string.format("redefinição de '%s'", symbol.name), 
                node.span)
        end
    end
end

-- ============================================================================
-- Passagem 2 — Vinculação (Análise Profunda)
-- ============================================================================

function Binder:_bind_node(node)
    if not node then return BuiltinTypes.VOID end
    
    local handlers = {
        [SK.FUNC_DECL]       = self._bind_func_decl,
        [SK.VAR_DECL]        = self._bind_var_decl,
        [SK.CONST_DECL]      = self._bind_var_decl,
        [SK.GLOBAL_DECL]     = self._bind_var_decl,
        [SK.STATE_DECL]      = self._bind_var_decl,
        [SK.COMPUTED_DECL]   = self._bind_var_decl,
        [SK.ASSIGN_STMT]     = self._bind_assign_stmt,
        [SK.EXPR_STMT]       = self._bind_expr_stmt,
        [SK.BINARY_EXPR]     = self._bind_binary_expr,
        [SK.LITERAL_EXPR]    = self._bind_literal_expr,
        [SK.IDENTIFIER_EXPR] = self._bind_identifier_expr,
        [SK.CALL_EXPR]       = self._bind_call_expr,
        [SK.IF_STMT]         = self._bind_if_stmt,
        [SK.RETURN_STMT]     = self._bind_return_stmt,
        [SK.WHILE_STMT]      = self._bind_while_stmt,
        [SK.RANGE_EXPR]      = self._bind_range_expr,
        [SK.GROUP_EXPR]      = self._bind_group_expr,
        [SK.STRUCT_DECL]     = self._bind_struct_decl,
        [SK.SELF_FIELD_EXPR] = self._bind_self_field_expr,
        [SK.IT_EXPR]         = self._bind_it_ref_expr,
        [SK.SELF_EXPR]       = self._bind_self_ref_expr,
        [SK.MEMBER_EXPR]     = self._bind_member_expr,
        [SK.STRUCT_INIT_EXPR]= self._bind_struct_init_expr,
        [SK.UNARY_EXPR]      = self._bind_unary_expr,
        [SK.BANG_EXPR]       = self._bind_bang_expr,
        [SK.LIST_EXPR]       = self._bind_list_expr,
        [SK.MAP_EXPR]        = self._bind_map_expr,
        [SK.ATTEMPT_STMT]    = self._bind_attempt_stmt,
        [SK.WATCH_STMT]      = self._bind_watch_stmt,
        [SK.MATCH_STMT]      = self._bind_match_stmt,
        [SK.LAMBDA_EXPR]     = self._bind_lambda_expr,
        [SK.IS_EXPR]         = self._bind_is_expr,
        [SK.AS_EXPR]         = self._bind_as_expr,
        [SK.CHECK_EXPR]      = self._bind_check_expr,
        [SK.AWAIT_EXPR]      = self._bind_await_expr,
        [SK.THROW_STMT]      = self._bind_throw_stmt,
        [SK.ASSERT_STMT]     = self._bind_assert_stmt,
        [SK.FOR_IN_STMT]     = self._bind_for_in_stmt,
        [SK.REPEAT_TIMES_STMT] = self._bind_repeat_stmt,
        [SK.COMPOUND_ASSIGN_STMT] = self._bind_compound_assign_stmt,
        [SK.IMPORT_DECL]     = self._bind_import_decl,
        [SK.TYPE_ALIAS_DECL] = self._bind_type_alias_decl,
        [SK.UNION_DECL]      = self._bind_union_decl,
        [SK.TRAIT_DECL]      = self._bind_trait_decl,
        [SK.APPLY_DECL]      = self._bind_apply_decl,
        [SK.NAMESPACE_DECL]  = function() end, -- Já tratado no bind()
    }
    
    local handler = handlers[node.kind]
    if handler then
        return handler(self, node)
    else
        -- Fallback: recursão para nós desconhecidos
        -- self.diagnostics:report_warning("ZT-S000", "nó não verificado pelo Binder: " .. node.kind, node.span)
        return BuiltinTypes.ANY
    end
end

function Binder:_bind_func_decl(node)
    local symbol = self.scope:lookup_local(node.name)
    
    -- Suporte a funções aninhadas
    if not symbol then
        -- Criamos o símbolo sem resolver o tipo de retorno ainda, pois pode depender de genéricos
        symbol = Symbol.func(node.name, node.params, BuiltinTypes.ANY, node.span, node.is_async)
        symbol.is_pub = node.is_pub or false
        if not self.scope:define(symbol) then
            self.diagnostics:report_error("ZT-S001", string.format("redefinição de '%s'", symbol.name), node.span)
            return BuiltinTypes.ERROR
        end
    end
    
    local prev_func = self.current_func
    self.current_func = symbol
    
    node.symbol = symbol
    symbol.declaration = node
    
    -- Sub-escopo para função (parâmetros + corpo)
    local func_scope = Scope.new(Scope.Kind.FUNCTION, self.scope)
    local prev_scope = self.scope
    self.scope = func_scope

    -- Se estivermos em uma struct, injeta 'self'
    if self.current_struct then
        local self_sym = Symbol.parameter("self", self.current_struct.type_info, nil, node.span)
        self.scope:define(self_sym)
    end

    -- 1. Vincular parâmetros genéricos (CRÍTICO: antes da assinatura)
    local resolved_generic_params = {}
    for _, p in ipairs(node.generic_params or {}) do
        local constraint = p.constraint and self:_resolve_type(p.constraint) or nil
        local param_sym = Symbol.generic_param(p.name, constraint, p.span)
        if not self.scope:define(param_sym) then
            self.diagnostics:report_error("ZT-S001", string.format("parâmetro genérico '%s' já declarado", p.name), p.span)
        end
        table.insert(resolved_generic_params, param_sym)
    end
    symbol.generic_params = resolved_generic_params

    -- 2. Resolver tipo de retorno e parâmetros reais (agora T é conhecido)
    symbol.return_type = self:_resolve_type(node.return_type)

    for i, param in ipairs(node.params) do
        local type_info = self:_resolve_type(param.type_node)
        
        -- Se não tem nome, é destruturação: func f(Player { x })
        if not param.name and param.pattern then
            param.name = "_p" .. i -- Nome interno para o Lua
            self:_bind_pattern(param.pattern, type_info)
        end

        local param_symbol = Symbol.parameter(param.name, type_info, param.default_value, param.span)
        if not self.scope:define(param_symbol) then
            self.diagnostics:report_error("ZT-S001", 
                string.format("parâmetro '%s' já declarado", param.name), 
                param.span)
        end
    end
    
    -- 3. Corpo
    for _, stmt in ipairs(node.body) do
        self:_bind_node(stmt)
    end
    
    self.scope = prev_scope
    self.current_func = prev_func
    return symbol.type_info or BuiltinTypes.VOID
end

function Binder:_bind_struct_decl(node)
    local symbol = self.scope:lookup(node.name)
    if not symbol then return BuiltinTypes.ERROR end

    node.symbol = symbol
    local prev_struct = self.current_struct
    self.current_struct = symbol
    
    local struct_scope = symbol.members_scope
    local prev_scope = self.scope

    if not struct_scope then
        -- Primeira vez visitando (Passagem 1.2): Criar escopo e vincular campos
        struct_scope = Scope.new(Scope.Kind.STRUCT, self.scope)
        symbol.members_scope = struct_scope
        self.scope = struct_scope
        
        -- Vincular parâmetros genéricos
        local resolved_generic_params = {}
        for _, p in ipairs(node.generic_params or {}) do
            local constraint = p.constraint and self:_resolve_type(p.constraint) or nil
            local param_sym = Symbol.generic_param(p.name, constraint, p.span)
            if not self.scope:define(param_sym) then
                self.diagnostics:report_error("ZT-S001", string.format("parâmetro genérico '%s' já declarado", p.name), p.span)
            end
            table.insert(resolved_generic_params, param_sym)
        end
        symbol.generic_params = resolved_generic_params

        -- Vincular campos
        local resolved_fields = {}
        for _, field in ipairs(node.fields) do
            local type_info = self:_resolve_type(field.type_node)
            local field_sym = Symbol.field(field.name, type_info, field.is_pub, field.default_value, field.span)
            if not self.scope:define(field_sym) then
                self.diagnostics:report_error("ZT-S001", 
                    string.format("campo '%s' já declarado no struct", field.name), field.span)
            end
            table.insert(resolved_fields, field_sym)
        end
        symbol.fields = resolved_fields
        -- Vincular assinaturas de métodos (SEM o corpo ainda)
        local resolved_methods = {}
        for _, method in ipairs(node.methods) do
            local return_type = self:_resolve_type(method.return_type)
            local method_sym = Symbol.func(method.name, method.params, return_type, method.span)
            method_sym.declaration = method
            table.insert(resolved_methods, method_sym)
        end
        symbol.methods = resolved_methods
    end

    -- Se estamos apenas registrando assinaturas, paramos aqui
    if self.only_signatures then
        self.scope = prev_scope
        self.current_struct = prev_struct
        return symbol.type_info
    end

    -- Segunda vez visitando (Passagem 2): Vincular corpos de métodos
    self.scope = struct_scope
    for _, method_sym in ipairs(symbol.methods or {}) do
        if method_sym.declaration then
            self:_bind_node(method_sym.declaration)
        end
    end
    
    self.scope = prev_scope
    self.current_struct = prev_struct
    return symbol.type_info or BuiltinTypes.VOID
end

function Binder:_bind_trait_decl(node)
    local symbol = self.scope:lookup_local(node.name)
    if not symbol then return BuiltinTypes.ERROR end
    
    local prev_scope = self.scope
    self.scope = Scope.new(Scope.Kind.STRUCT, self.scope)
    
    local resolved_methods = {}
    for _, method in ipairs(node.methods) do
        local return_type = self:_resolve_type(method.return_type)
        local method_sym = Symbol.func(method.name, method.params, return_type, method.span)
        method_sym.declaration = method
        table.insert(resolved_methods, method_sym)
    end
    symbol.methods = resolved_methods
    
    self.scope = prev_scope
    return BuiltinTypes.VOID
end

function Binder:_bind_apply_decl(node)
    local trait_sym = self.scope:lookup(node.trait_name)
    local struct_sym = self.scope:lookup(node.struct_name)
    
    if not trait_sym or trait_sym.kind ~= Symbol.Kind.TRAIT then
        self.diagnostics:report_error("ZT-S301", string.format("trait '%s' não encontrada", node.trait_name), node.span)
        return BuiltinTypes.ERROR
    end
    if not struct_sym or struct_sym.kind ~= Symbol.Kind.STRUCT then
        self.diagnostics:report_error("ZT-S302", string.format("struct '%s' não encontrada", node.struct_name), node.span)
        return BuiltinTypes.ERROR
    end
    
    -- Vincular métodos da implementação
    local prev_struct = self.current_struct
    self.current_struct = struct_sym
    
    -- O escopo de implementação herda do escopo de membros da struct para achar campos
    local struct_scope = Scope.new(Scope.Kind.STRUCT, struct_sym.members_scope or self.scope)
    local prev_scope = self.scope
    self.scope = struct_scope
    
    local implemented_methods = {}
    for _, method in ipairs(node.methods or {}) do
        self:_bind_node(method)
        if method.symbol then
            implemented_methods[method.name] = method.symbol
            -- Adiciona ao símbolo da struct se não existir (evita duplicatas se já declarado no struct)
            local existing = struct_sym:get_member(method.name)
            if not existing then
                table.insert(struct_sym.methods, method.symbol)
            end
        end
    end
    
    -- Verificação de Contrato e Injeção de Defaults
    for _, req_method in ipairs(trait_sym.methods) do
        local impl = implemented_methods[req_method.name] or struct_sym:get_member(req_method.name)
        if not impl then
            -- Se não tem implementação, checa se a trait tem um default (corpo no nó da declaração)
            local req_node = req_method.declaration
            if req_node and req_node.body then
                -- Injeta a implementação padrão na struct
                table.insert(struct_sym.methods, req_method)
            else
                self.diagnostics:report_error("ZT-S303", 
                    string.format("struct '%s' não implementa o método requerido '%s' da trait '%s'", 
                        struct_sym.name, req_method.name, trait_sym.name), node.span)
            end
        end
    end
    
    -- Registrar que a struct implementa a trait
    table.insert(struct_sym.implements, trait_sym)
    
    self.scope = prev_scope
    self.current_struct = prev_struct
    return BuiltinTypes.VOID
end

function Binder:_bind_type_alias_decl(node)
    local symbol = self.scope:lookup_local(node.name)
    if not symbol then return BuiltinTypes.ERROR end
    
    symbol.target_node = node.target_type
    
    -- Se NÃO for genérico, resolvemos agora para validar imediatamente
    if not node.generic_params or #node.generic_params == 0 then
        symbol.type_info = self:_resolve_type(node.target_type)
    else
        -- Se for genérico, deixamos ANY como base; a resolução real será dinâmica no momento do uso
        symbol.type_info = BuiltinTypes.ANY
        
        -- Mas ainda registramos os parâmetros genéricos no símbolo para a substituição futura
        local resolved_params = {}
        for _, p in ipairs(node.generic_params) do
            local p_sym = Symbol.generic_param(p.name, nil, p.span)
            table.insert(resolved_params, p_sym)
        end
        symbol.generic_params = resolved_params
    end
    
    node.symbol = symbol
    return symbol.type_info
end

function Binder:_bind_union_decl(node)
    local symbol = self.scope:lookup_local(node.name)
    if not symbol then return BuiltinTypes.ERROR end
    
    symbol.target_node = node.union_type
    
    if not node.generic_params or #node.generic_params == 0 then
        symbol.type_info = self:_resolve_type(node.union_type)
    else
        symbol.type_info = BuiltinTypes.ANY
        local resolved_params = {}
        for _, p in ipairs(node.generic_params) do
            local p_sym = Symbol.generic_param(p.name, nil, p.span)
            table.insert(resolved_params, p_sym)
        end
        symbol.generic_params = resolved_params
    end
    
    node.symbol = symbol
    return symbol.type_info
end

function Binder:_bind_var_decl(node)
    local type_info = BuiltinTypes.ANY
    if node.type_node then
        type_info = self:_resolve_type(node.type_node)
    end

    -- Se for um identificador simples, usamos a lógica antiga
    if node.name then
        local symbol = self.scope:lookup_local(node.name)
        if not symbol then
            if node.kind == SK.CONST_DECL then symbol = Symbol.constant(node.name, type_info, node.is_pub, node.span)
            elseif node.kind == SK.GLOBAL_DECL then symbol = Symbol.global_var(node.name, type_info, node.is_pub, node.span)
            elseif node.kind == SK.STATE_DECL then symbol = Symbol.state_var(node.name, type_info, node.span)
            elseif node.kind == SK.COMPUTED_DECL then symbol = Symbol.computed(node.name, type_info, node.span)
            else symbol = Symbol.variable(node.name, type_info, node.is_pub, node.span) end
            
            if not self.scope:define(symbol) then
                self.diagnostics:report_error("ZT-S001", string.format("redefinição de '%s'", node.name), node.span)
            end
            node.symbol = symbol
        end
    elseif node.pattern then
        -- Destruturação complexa: vincular sub-padrões
        self:_bind_pattern(node.pattern, type_info)
    else
        -- Erro fallback
        self.diagnostics:report_error("ZT-S101", "declaração inválida: nome ou padrão esperado", node.span)
    end
    
    -- Verificar inicializador
    local init_node = node.initializer or node.expression
    if init_node then
        local init_type = self:_bind_node(init_node)
        local TypeChecker = require("src.semantic.types.type_checker")
        
        -- Se o tipo não foi especificado, ele é inferido do inicializador na destruturação
        if not node.type_node then
            type_info = init_type
            -- Se for destruturação, precisamos re-vincular os tipos dos sub-padrões com a informação real
            if node.pattern then self:_bind_pattern(node.pattern, type_info) end
        end

        if not TypeChecker.is_assignable(init_type, type_info) then
            self.diagnostics:report_error("ZT-S100", 
                string.format("Tipo incompatível na atribuição: esperado '%s', encontrado '%s'", tostring(type_info), tostring(init_type)), 
                init_node.span)
        end
    elseif not node.pattern then
        -- Para declarações simples, o tipo é obrigatório se não houver inicializador (v0.2 rule)
        if not node.type_node then
            self.diagnostics:report_error("ZT-S101", "definição de tipo obrigatória para variável '" .. (node.name or "???") .. "'", node.span)
        end
    end

    return type_info
end

--- Vincula um padrão de destruturação.
function Binder:_bind_pattern(pattern, matched_type)
    if not pattern then return end
    
    if pattern.kind == SK.IDENTIFIER_EXPR then
        local name = pattern.name
        if name == "_" then return end
        
        -- Se for um Enum Member conhecido, não é destruturação, é comparação de valor
        local existing = self.scope:lookup(name)
        if existing and existing.kind == Symbol.Kind.ENUM_MEMBER then
            pattern.symbol = existing
            return
        end
        
        -- Caso contrário, é uma captura (cria nova variável local)
        local sym = Symbol.variable(name, matched_type, false, pattern.span)
        self.scope:define(sym)
        pattern.symbol = sym
        pattern.is_capture = true

    elseif pattern.kind == SK.LIST_EXPR then
        -- Assume que matched_type é list<T>
        local item_type = BuiltinTypes.ANY
        if matched_type.kind == ZenithType.Kind.GENERIC and matched_type.base_name == "list" then
            item_type = matched_type.type_args[1]
        end
        
        for _, el in ipairs(pattern.elements) do
            self:_bind_pattern(el, item_type)
        end

    elseif pattern.kind == SK.REST_EXPR then
        -- ..resto (matched_type deve ser o tipo da coleção)
        self:_bind_pattern(pattern.expression, matched_type)

    elseif pattern.kind == SK.STRUCT_INIT_EXPR then
        -- Player { nome, vida: 0 }
        local struct_sym = self.scope:lookup(pattern.type_name)
        if not struct_sym or struct_sym.kind ~= Symbol.Kind.STRUCT then
            self.diagnostics:report_error("ZT-S002", "struct '" .. pattern.type_name .. "' não encontrada no padrão", pattern.span)
            return
        end
        
        for _, f in ipairs(pattern.fields) do
            -- Busca o tipo do campo na struct
            local field_sym = struct_sym:get_member(f.name)
            local f_type = field_sym and field_sym.type_info or BuiltinTypes.ANY
            self:_bind_pattern(f.value, f_type)
        end

    else
        -- Literais e outros: apenas vincula normalmente para checar tipos
        local p_type = self:_bind_node(pattern)
        local TypeChecker = require("src.semantic.types.type_checker")
        if not TypeChecker.is_assignable(p_type, matched_type) then
            self.diagnostics:report_error("ZT-S100", 
                string.format("tipo do padrão '%s' incompatível com '%s'", tostring(p_type), tostring(matched_type)), 
                pattern.span)
        end
    end
end

-- ============================================================================
-- Handlers de Expressões
-- ============================================================================

function Binder:_bind_literal_expr(node)
    local map = {
        int = BuiltinTypes.INT,
        float = BuiltinTypes.FLOAT,
        text = BuiltinTypes.TEXT,
        bool = BuiltinTypes.BOOL,
        null = BuiltinTypes.NULL,
    }
    return map[node.literal_type] or BuiltinTypes.ERROR
end

function Binder:_bind_identifier_expr(node)
    local symbol = self.scope:lookup(node.name)
    if not symbol then
        self.diagnostics:report_error("ZT-S002", 
            string.format("símbolo '%s' não encontrado", node.name), node.span)
        return BuiltinTypes.ERROR
    end
    
    node.symbol = symbol
    -- Se for um reativo (STATE), o tipo resultante é o tipo base (transparente)
    node.is_reactive = (symbol.kind == Symbol.Kind.STATE or symbol.kind == Symbol.Kind.COMPUTED)
    
    return symbol.type_info or BuiltinTypes.ANY
end

function Binder:_bind_binary_expr(node)
    local left_type = self:_bind_node(node.left)
    local right_type = self:_bind_node(node.right)
    
    local result_type = TypeChecker.get_binary_result(left_type, node.operator.lexeme, right_type)
    
    if result_type == BuiltinTypes.ERROR then
        self.diagnostics:report_error("ZT-S101", 
            string.format("operador '%s' inválido para tipos '%s' e '%s'", 
                node.operator.lexeme, tostring(left_type), tostring(right_type)), node.span)
    end
    
    node.type_info = result_type
    return result_type
end

function Binder:_bind_unary_expr(node)
    local operand_type = self:_bind_node(node.operand)
    local result_type = TypeChecker.get_unary_result(node.operator.lexeme, operand_type)
    
    if result_type == BuiltinTypes.ERROR then
        self.diagnostics:report_error("ZT-S101", 
            string.format("operador '%s' inválido para tipo '%s'", 
                node.operator.lexeme, tostring(operand_type)), node.span)
    end
    
    return result_type
end

function Binder:_bind_call_expr(node)
    local callee_type = self:_bind_node(node.callee)
    
    -- Se o callee tem um símbolo, pegamos os parâmetros reais dele
    local symbol = node.callee.symbol
    if not symbol and node.callee.kind == SK.MEMBER_EXPR then
        symbol = node.callee.symbol -- Já deve ter sido setado se foi resolvido
    end

    -- Validação de Argumentos Genéricos Explícitos: func<int>(...)
    if node.generic_args and symbol and symbol.generic_params then
        local resolved_args = {}
        for _, arg_node in ipairs(node.generic_args) do
            table.insert(resolved_args, self:_resolve_type(arg_node))
        end
        node.resolved_generic_args = resolved_args
        
        if #resolved_args ~= #symbol.generic_params then
            self.diagnostics:report_error("ZT-S104", 
                string.format("número incorreto de argumentos genéricos para '%s' (esperado %d, recebido %d)", 
                    symbol.name, #symbol.generic_params, #resolved_args), node.span)
        else
            local TypeChecker = require("src.semantic.types.type_checker")
            for i, arg_type in ipairs(resolved_args) do
                local constraint = symbol.generic_params[i].constraint
                if constraint and not TypeChecker.is_assignable(arg_type, constraint) then
                    self.diagnostics:report_error("ZT-S100", 
                        string.format("argumento genérico '%s' não satisfaz a restrição '%s' de '%s'", 
                            tostring(arg_type), tostring(constraint), symbol.generic_params[i].name), 
                        node.generic_args[i].span)
                end
            end
        end
    end

    if callee_type.kind ~= ZenithType.Kind.FUNC and callee_type.kind ~= ZenithType.Kind.STRUCT then
        -- Se for um símbolo de função já resolvido (mesmo que o tipo de retorno seja VOID), é chamável
        if symbol and symbol.kind == Symbol.Kind.FUNCTION then
            -- OK
        elseif callee_type == BuiltinTypes.ANY then 
            -- Vincular argumentos mesmo que não possamos validar os parâmetros
            for _, arg in ipairs(node.arguments) do
                local arg_node = arg
                if type(arg) == "table" and arg.kind == "NAMED" then
                    arg_node = arg.value
                end
                self:_bind_node(arg_node)
            end
            return BuiltinTypes.ANY 
        else
            self.diagnostics:report_error("ZT-S103", 
                string.format("tipo '%s' não é chamável", tostring(callee_type)), node.callee.span)
            return BuiltinTypes.ERROR
        end
    end

    -- Validação de Chamada de Função
    local params = symbol and symbol.params or callee_type.params or {}
    
    -- Heurística Construtor: se callee_type é STRUCT, os parâmetros são seus campos (para o .new() implícito)
    if callee_type.kind == ZenithType.Kind.STRUCT and #params == 0 then
        local struct_sym = callee_type.symbol
        if struct_sym and struct_sym.fields then
            params = struct_sym.fields
        end
    end

    local args = node.arguments
    local effective_args = {} -- Mapeamento param_index -> arg_node
    
    -- 1. Suporte UFCS: O primeiro parâmetro é preenchido pelo objeto do MemberExpr
    local current_pos = 1
    if node.callee.kind == SK.MEMBER_EXPR and node.callee.is_ufcs then
        effective_args[1] = node.callee.object
        current_pos = 2
    end

    -- 2. Preencher argumentos posicionais e nomeados
    local has_spread = false
    for i, arg in ipairs(args) do
        if type(arg) == "table" and arg.kind == "NAMED" then
            -- ... (restante do código de NAMED mantido)
            local found = false
            for j, p in ipairs(params) do
                if p.name == arg.name then
                    if effective_args[j] then
                        self.diagnostics:report_error("ZT-S105", 
                            string.format("parâmetro '%s' já preenchido posicionalmente", arg.name), arg.span or node.span)
                    end
                    effective_args[j] = arg.value
                    found = true
                    break
                end
            end
            if not found then
                self.diagnostics:report_error("ZT-S105", 
                    string.format("parâmetro '%s' não existe na função", arg.name), arg.span or node.span)
            end
        else
            -- Posicional
            if arg.kind == SK.REST_EXPR then has_spread = true end
            
            while effective_args[current_pos] do current_pos = current_pos + 1 end
            if current_pos > #params and not has_spread then
                self.diagnostics:report_error("ZT-S105", "argumentos posicionais demais (esperado " .. #params .. ")", node.span)
            else
                effective_args[current_pos] = arg
                current_pos = current_pos + 1
            end
        end
    end

    -- 3. Verificar parâmetros obrigatórios faltantes e validar tipos
    local final_args = {}
    -- Se houver spread, a validação de "faltantes" é pulada pois o spread pode preencher tudo em runtime
    if has_spread then
        -- Apenas vincula todos os argumentos presentes
        for _, arg in ipairs(args) do
            local arg_node = arg
            if type(arg) == "table" and arg.kind == "NAMED" then arg_node = arg.value end
            self:_bind_node(arg_node)
            table.insert(final_args, arg)
        end
    else
        for i, p in ipairs(params) do
            local val = effective_args[i]
            if not val then
                if p.default_value then
                    val = p.default_value
                else
                    self.diagnostics:report_error("ZT-S105", "argumento faltante para o parâmetro '" .. p.name .. "'", node.span)
                end
            end
            if val then
                local arg_type = self:_bind_node(val)
                table.insert(final_args, val)
            end
        end
    end

    node.resolved_args = final_args
    local ret = symbol and symbol.return_type or callee_type.return_type
    if not ret and callee_type.kind == ZenithType.Kind.STRUCT then
        ret = callee_type
    end
    return ret or BuiltinTypes.ANY
end

function Binder:_bind_expr_stmt(node)
    self:_bind_node(node.expression)
    return BuiltinTypes.VOID
end

function Binder:_bind_assign_stmt(node)
    local target_type = self:_bind_node(node.target or node.left)
    local value_type = self:_bind_node(node.value or node.right)
    
    local TypeChecker = require("src.semantic.types.type_checker")
    if not TypeChecker.is_assignable(value_type, target_type) then
        self.diagnostics:report_error("ZT-S100", 
            string.format("Tipo incompatível na atribuição: esperado '%s', encontrado '%s'", tostring(target_type), tostring(value_type)), 
            (node.value or node.right).span)
    end
    
    return BuiltinTypes.VOID
end

function Binder:_bind_compound_assign_stmt(node)
    return self:_bind_assign_stmt(node)
end

function Binder:_bind_if_stmt(node)
    local cond_type = self:_bind_node(node.condition)
    if cond_type ~= BuiltinTypes.BOOL and cond_type ~= BuiltinTypes.ANY then
        self.diagnostics:report_error("ZT-S102", "condição do 'if' deve ser 'bool'", node.condition.span)
    end
    
    local if_scope = Scope.new(Scope.Kind.BLOCK, self.scope)
    local old_scope = self.scope
    self.scope = if_scope
    for _, stmt in ipairs(node.body) do self:_bind_node(stmt) end
    self.scope = old_scope
    
    for _, elif in ipairs(node.elif_clauses or {}) do
        local ec_type = self:_bind_node(elif.condition)
        if ec_type ~= BuiltinTypes.BOOL and ec_type ~= BuiltinTypes.ANY then
            self.diagnostics:report_error("ZT-S102", "condição do 'elif' deve ser 'bool'", elif.condition.span)
        end
        local elif_scope = Scope.new(Scope.Kind.BLOCK, self.scope)
        self.scope = elif_scope
        for _, stmt in ipairs(elif.body) do self:_bind_node(stmt) end
        self.scope = old_scope
    end
    
    if node.else_clause then
        local else_scope = Scope.new(Scope.Kind.BLOCK, self.scope)
        self.scope = else_scope
        for _, stmt in ipairs(node.else_clause.body) do self:_bind_node(stmt) end
        self.scope = old_scope
    end
    
    return BuiltinTypes.VOID
end

function Binder:_bind_for_in_stmt(node)
    local iterable_type = self:_bind_node(node.iterable)
    
    local for_scope = Scope.new(Scope.Kind.BLOCK, self.scope)
    local old_scope = self.scope
    self.scope = for_scope
    
    -- Descobrir tipo dos itens (v1.0-alpha assume ANY se não for Range)
    local item_type = BuiltinTypes.ANY
    if iterable_type.kind == ZenithType.Kind.GENERIC and iterable_type.base_name == "list" then
        item_type = iterable_type.type_args[1] or BuiltinTypes.ANY
    end

    for _, v in ipairs(node.variables) do
        local sym = Symbol.for_var(v.name, item_type, v.span)
        self.scope:define(sym)
    end
    
    for _, stmt in ipairs(node.body) do self:_bind_node(stmt) end
    
    self.scope = old_scope
    return BuiltinTypes.VOID
end

function Binder:_bind_repeat_stmt(node)
    self:_bind_node(node.count_expr)
    local repeat_scope = Scope.new(Scope.Kind.BLOCK, self.scope)
    local old_scope = self.scope
    self.scope = repeat_scope
    for _, stmt in ipairs(node.body) do self:_bind_node(stmt) end
    self.scope = old_scope
    return BuiltinTypes.VOID
end

function Binder:_bind_assert_stmt(node)
    self:_bind_node(node.expression)
    return BuiltinTypes.VOID
end

function Binder:_bind_attempt_stmt(node)
    local try_scope = Scope.new(Scope.Kind.BLOCK, self.scope)
    local old_scope = self.scope
    self.scope = try_scope
    for _, stmt in ipairs(node.body) do self:_bind_node(stmt) end
    self.scope = old_scope
    
    if node.rescue_clause then
        local rescue = node.rescue_clause
        local rescue_scope = Scope.new(Scope.Kind.BLOCK, self.scope)
        local old_scope_r = self.scope
        self.scope = rescue_scope
        
        if rescue.error_name then
            local err_sym = Symbol.new(Symbol.Kind.RESCUE_VAR, rescue.error_name, {
                type_info = BuiltinTypes.ANY,
                span = rescue.span
            })
            self.scope:define(err_sym)
        end

        for _, stmt in ipairs(rescue.body) do self:_bind_node(stmt) end
        self.scope = old_scope_r
    end

    return BuiltinTypes.VOID
end

function Binder:_bind_watch_stmt(node)
    if node.target then self:_bind_node(node.target) end
    local watch_scope = Scope.new(Scope.Kind.BLOCK, self.scope)
    local old_scope = self.scope
    self.scope = watch_scope
    for _, stmt in ipairs(node.body) do self:_bind_node(stmt) end
    self.scope = old_scope
    return BuiltinTypes.VOID
end

function Binder:_bind_return_stmt(node)
    local expr_type = node.expression and self:_bind_node(node.expression) or BuiltinTypes.VOID
    
    if self.current_func then
        local expected = self.current_func.return_type
        if not TypeChecker.is_assignable(expr_type, expected) then
            self.diagnostics:report_error("ZT-S100", 
                string.format("Tipo de retorno incompatível: esperado '%s', encontrado '%s'", tostring(expected), tostring(expr_type)), 
                node.expression and node.expression.span or node.span)
        end
    end
    
    return BuiltinTypes.VOID
end

function Binder:_bind_while_stmt(node)
    local cond_type = self:_bind_node(node.condition)
    if cond_type ~= BuiltinTypes.BOOL and cond_type ~= BuiltinTypes.ANY then
        self.diagnostics:report_error("ZT-S102", "condição do 'while' deve ser 'bool'", node.condition.span)
    end
    
    local while_scope = Scope.new(Scope.Kind.BLOCK, self.scope)
    local old_scope = self.scope
    self.scope = while_scope
    for _, stmt in ipairs(node.body) do self:_bind_node(stmt) end
    self.scope = old_scope
    return BuiltinTypes.VOID
end

function Binder:_bind_range_expr(node)
    local start_t = self:_bind_node(node.start_expr)
    local end_t = self:_bind_node(node.end_expr)
    
    if start_t ~= BuiltinTypes.INT or end_t ~= BuiltinTypes.INT then
        self.diagnostics:report_error("ZT-S102", "intervalos (ranges) devem ser compostos por inteiros", node.span)
    end
    
    return ZenithType.new(ZenithType.Kind.GENERIC, "list<int>", {
        base_name = "list",
        type_args = { BuiltinTypes.INT },
        span = node.span
    })
end

function Binder:_bind_group_expr(node)
    return self:_bind_node(node.expression)
end

function Binder:_bind_self_field_expr(node)
    if not self.current_struct then
        self.diagnostics:report_error("ZT-S201", 
            "atalho '@campo' só pode ser usado dentro de métodos de um struct/trait", node.span)
        return BuiltinTypes.ERROR
    end
    
    -- Resolve como 'self.campo'
    local sym = self.scope:lookup(node.field_name)
    
    -- Se não achou no escopo, tenta buscar diretamente no símbolo da struct (para casos de apply)
    if not sym and self.current_struct then
        sym = self.current_struct:get_member(node.field_name)
    end

    if not sym then
        self.diagnostics:report_error("ZT-S002", 
            string.format("campo '%s' não encontrado no struct '%s'", node.field_name, self.current_struct.name), node.span)
        return BuiltinTypes.ERROR
    end
    
    node.symbol = sym
    return sym.type_info or BuiltinTypes.ANY
end

function Binder:_bind_it_ref_expr(node)
    -- 'it' é resolvido dinamicamente ou injetado em contextos específicos (como where)
    return BuiltinTypes.ANY
end

function Binder:_bind_self_ref_expr(node)
    if not self.current_struct then
        self.diagnostics:report_error("ZT-S201", "'self' só pode ser usado dentro de métodos", node.span)
        return BuiltinTypes.ERROR
    end
    return self.current_struct.type_info
end

function Binder:_bind_member_expr(node)
    local object_type = self:_bind_node(node.object)
    
    -- Heurística para construtores: Struct.new(...) -> Struct
    if node.member_name == "new" and object_type.kind == ZenithType.Kind.STRUCT then
        return object_type
    end

    -- Caso 2: Acesso a Campo/Método de Struct ou Tipo Genérico (T)
    local target_type = object_type
    
    -- Se for um parâmetro genérico, buscamos na restrição (constraint) dele
    if object_type.kind == ZenithType.Kind.TYPE_PARAM and object_type.constraint then
        target_type = object_type.constraint
    end

    if target_type.kind == ZenithType.Kind.STRUCT or target_type.kind == ZenithType.Kind.TRAIT then
        local base_sym = target_type.symbol
        if base_sym then
            local member_sym = base_sym:get_member(node.member_name)
            if member_sym then
                node.symbol = member_sym
                return member_sym.type_info or member_sym.return_type or BuiltinTypes.ANY
            end
        end
    end

    -- Fallback UFCS: Procurar função global que aceite o objeto como primeiro parâmetro
    local ufcs_sym = self.scope:lookup(node.member_name)
    if ufcs_sym and ufcs_sym.kind == Symbol.Kind.FUNCTION then
        node.symbol = ufcs_sym
        node.is_ufcs = true
        return ufcs_sym.type_info or BuiltinTypes.ANY
    end

    -- Heurística para reatividade: state.get / state.set
    if node.member_name == "get" or node.member_name == "set" then
        return BuiltinTypes.ANY
    end

    -- Heurística para ANY: se nada acima funcionou e o objeto é ANY, aceitamos qualquer membro
    if object_type == BuiltinTypes.ANY then
        return BuiltinTypes.ANY
    end

    -- Erro: membro não encontrado
    self.diagnostics:report_error("ZT-S008", 
        string.format("membro '%s' não encontrado no tipo '%s'", 
            node.member_name, tostring(object_type)), node.span)
    
    return BuiltinTypes.ANY
end

function Binder:_bind_struct_init_expr(node)
    local type_sym = self.scope:lookup(node.type_name)
    if not type_sym or type_sym.kind ~= Symbol.Kind.STRUCT then
        self.diagnostics:report_error("ZT-S002", 
            string.format("tipo '%s' não encontrado para inicialização", node.type_name), node.span)
        return BuiltinTypes.ERROR
    end
    
    local resolved_type = type_sym.type_info
    
    -- Validar se a inicialização usa genéricos: Box<int> { ... }
    if node.generic_args then
        -- Criamos um nó temporário de GENERIC_TYPE para reutilizar a lógica de validação do _resolve_type
        resolved_type = self:_resolve_type({
            kind = SK.GENERIC_TYPE,
            base_name = node.type_name,
            type_args = node.generic_args,
            span = node.span
        })
    end

    -- Validar campos da inicialização
    for _, field in ipairs(node.fields) do
        self:_bind_node(field.value)
    end
    
    return resolved_type or type_sym.type_info or BuiltinTypes.ANY
end

-- ============================================================================
-- Helpers de Resolução
--- Resolve um nó de tipo da AST para um ZenithType real.
function Binder:_resolve_type(type_node)
    if not type_node then return BuiltinTypes.VOID end
    
    if type_node.kind == SK.NAMED_TYPE then
        local builtin = BuiltinTypes.lookup(type_node.name)
        if builtin then return builtin end
        
        -- Prioridade 1: Parâmetros genéricos do alias atual
        if self.current_alias and self.current_alias.generic_params then
            for _, p in ipairs(self.current_alias.generic_params) do
                if p.name == type_node.name then
                    return ZenithType.new(ZenithType.Kind.TYPE_PARAM, p.name, {
                        symbol = p,
                        constraint = p.constraint
                    })
                end
            end
        end

        -- Prioridade 2: Busca global
        local symbol = self.scope:lookup(type_node.name)
        if symbol then
            if symbol.kind == Symbol.Kind.STRUCT or 
               symbol.kind == Symbol.Kind.ENUM or 
               symbol.kind == Symbol.Kind.TRAIT or
               symbol.kind == Symbol.Kind.ALIAS then
                return symbol.type_info
            elseif symbol.kind == Symbol.Kind.GENERIC_PARAM then
                return ZenithType.new(ZenithType.Kind.TYPE_PARAM, type_node.name, {
                    symbol = symbol,
                    constraint = symbol.constraint
                })
            end
        end
        
        self.diagnostics:report_error("ZT-S002", 
            string.format("tipo desconhecido: '%s'", type_node.name), 
            type_node.span)
        return BuiltinTypes.ERROR
    end

    if type_node.kind == SK.GENERIC_TYPE then
        local base_sym = self.scope:lookup(type_node.base_name)
        local expected_params = {}
        
        -- Heurística: se não achou símbolo, checa se é builtin
        if not base_sym then
            local builtin = BuiltinTypes.lookup(type_node.base_name)
            if builtin then
                -- Para builtins, criamos parâmetros genéricos "fakes"
                local counts = { list = 1, grid = 1, map = 2 }
                local count = counts[type_node.base_name] or 0
                for i = 1, count do table.insert(expected_params, { name = "T"..i }) end
            else
                self.diagnostics:report_error("ZT-S002", string.format("tipo base '%s' não encontrado", type_node.base_name), type_node.span)
                return BuiltinTypes.ERROR
            end
        else
            expected_params = base_sym.generic_params or {}
        end

        local args = {}
        for i, arg_node in ipairs(type_node.type_args) do
            local arg_t = self:_resolve_type(arg_node)
            table.insert(args, arg_t)
        end
        
        -- Validar Constraints
        if #args ~= #expected_params then
            -- Se for Alias, a validação é flexível pois pode ser resolvido dinamicamente
            if not (base_sym and base_sym.kind == Symbol.Kind.ALIAS) then
                self.diagnostics:report_error("ZT-S104", 
                    string.format("número incorreto de argumentos genéricos para '%s' (esperado %d, recebido %d)", 
                        type_node.base_name, #expected_params, #args), type_node.span)
            end
        else
            local TypeChecker = require("src.semantic.types.type_checker")
            for i, arg_type in ipairs(args) do
                local constraint = expected_params[i].constraint
                if constraint and not TypeChecker.is_assignable(arg_type, constraint) then
                    self.diagnostics:report_error("ZT-S100", 
                        string.format("argumento genérico '%s' não satisfaz a restrição '%s' de '%s'", 
                            tostring(arg_type), tostring(constraint), expected_params[i].name), 
                        type_node.type_args[i].span)
                end
            end
        end
        
        -- Caso Especial: Alias Genérico (type Box<T> = list<T>)
        if base_sym and base_sym.kind == Symbol.Kind.ALIAS and #expected_params > 0 then
            local prev_scope = self.scope
            local temp_scope = Scope.new(Scope.Kind.BLOCK, self.scope) -- Escopo efêmero
            
            for i, p in ipairs(expected_params) do
                local arg_type = args[i] or BuiltinTypes.ANY
                local p_sym = Symbol.generic_param(p.name, nil, type_node.span)
                p_sym.type_info = arg_type
                temp_scope:define(p_sym)
            end
            
            self.scope = temp_scope
            local result = self:_resolve_type(base_sym.target_node)
            self.scope = prev_scope
            return result
        end

        return ZenithType.new(ZenithType.Kind.GENERIC, type_node.base_name, {
            base_name = type_node.base_name,
            type_args = args,
            span = type_node.span
        })
    end

    if type_node.kind == SK.FUNC_TYPE then
        local params = {}
        for _, p_node in ipairs(type_node.params or {}) do
            table.insert(params, self:_resolve_type(p_node))
        end
        local ret = self:_resolve_type(type_node.return_type)
        return ZenithType.new(ZenithType.Kind.FUNC, "func", {
            params = params,
            return_type = ret,
            span = type_node.span
        })
    end

    if type_node.kind == SK.UNION_TYPE then
        local types = {}
        for _, t_node in ipairs(type_node.types) do
            table.insert(types, self:_resolve_type(t_node))
        end
        return ZenithType.new(ZenithType.Kind.UNION, "union", {
            types = types,
            span = type_node.span
        })
    end

    if type_node.kind == SK.NULLABLE_TYPE then
        local base = self:_resolve_type(type_node.base_type)
        return ZenithType.new(ZenithType.Kind.NULLABLE, tostring(base).."?", {
            base_type = base,
            span = type_node.span
        })
    end

    return BuiltinTypes.ANY
end

function Binder:_bind_lambda_expr(node)
    local lambda_scope = Scope.new(Scope.Kind.FUNCTION, self.scope)
    local old_scope = self.scope
    self.scope = lambda_scope
    
    local params = {}
    for i, param in ipairs(node.params or {}) do
        local type_info = self:_resolve_type(param.type_node)
        
        -- Destruturação em Lambda
        if not param.name and param.pattern then
            param.name = "_lp" .. i
            self:_bind_pattern(param.pattern, type_info)
        end

        local param_symbol = Symbol.parameter(param.name, type_info, param.default_value, param.span)
        self.scope:define(param_symbol)
        table.insert(params, type_info)
    end
    
    local return_type = BuiltinTypes.ANY
    if type(node.body) == "table" and node.body.kind then
        -- Expressão única
        return_type = self:_bind_node(node.body)
    else
        -- Bloco de statements
        for _, stmt in ipairs(node.body or {}) do
            self:_bind_node(stmt)
        end
        -- TODO: Inferir tipo de retorno do bloco (por agora ANY)
    end
    
    self.scope = old_scope
    
    return ZenithType.new(ZenithType.Kind.FUNC, "lambda", {
        params = params,
        return_type = return_type
    })
end

function Binder:_bind_is_expr(node)
    self:_bind_node(node.expression)
    node.target_type = self:_resolve_type(node.type_node)
    return BuiltinTypes.BOOL
end

function Binder:_bind_as_expr(node)
    self:_bind_node(node.expression)
    node.target_type = self:_resolve_type(node.type_node)
    return node.target_type
end

function Binder:_bind_check_expr(node)
    self:_bind_node(node.expression)
    return BuiltinTypes.ANY
end

function Binder:_bind_await_expr(node)
    if not self.current_func or not self.current_func.is_async then
        self.diagnostics:report_error("ZT-S202", "'await' só pode ser usado dentro de uma função 'async'", node.span)
    end
    return self:_bind_node(node.expression)
end

function Binder:_bind_throw_stmt(node)
    self:_bind_node(node.expression)
    return BuiltinTypes.VOID
end

return Binder
