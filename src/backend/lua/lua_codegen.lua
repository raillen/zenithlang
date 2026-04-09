-- ============================================================================
-- Zenith Compiler — Lua Codegen
-- Transpila a AST validada para Lua 5.1/LuaJIT.
-- ============================================================================

local SK = require("src.syntax.ast.syntax_kind")
local ZenithType = require("src.semantic.types.zenith_type")

local LuaCodegen = {}
LuaCodegen.__index = LuaCodegen

function LuaCodegen.new()
    local self = setmetatable({}, LuaCodegen)
    self.output = {}
    self.indent_level = 0
    return self
end

--- Gera apenas o corpo do módulo sem o bloco de return de exports.
function LuaCodegen:generate_body(node)
    self.output = {}
    self.indent_level = 0
    self.pub_members = {}
    self.has_namespace = false
    self.skip_exports = true
    
    self:emit("-- Transpilado por Zenith v0.2.0 (Body Only)")
    self:emit("local zt = require(\"src.backend.lua.runtime.zenith_rt\")")
    self:emit("")
    
    self:_emit_node(node)
    
    return table.concat(self.output, "\n")
end

--- Ponto de entrada: transpila uma compilation unit.
function LuaCodegen:generate(node)
    self.output = {}
    self.indent_level = 0
    self.pub_members = {}
    self.has_namespace = false
    self.skip_exports = false
    
    -- Banner e Runtime
    self:emit("-- Transpilado por Zenith v0.2.0")
    self:emit("local zt = require(\"src.backend.lua.runtime.zenith_rt\")")
    self:emit("")
    
    self:_emit_node(node)
    
    return table.concat(self.output, "\n")
end

-- ----------------------------------------------------------------------------
-- Emissão
-- ----------------------------------------------------------------------------

function LuaCodegen:emit(text)
    local indent = string.rep("    ", self.indent_level)
    table.insert(self.output, indent .. text)
end

function LuaCodegen:indent() self.indent_level = self.indent_level + 1 end
function LuaCodegen:dedent() self.indent_level = self.indent_level - 1 end

-- ----------------------------------------------------------------------------
-- Visitors
-- ----------------------------------------------------------------------------

function LuaCodegen:_emit_node(node)
    if not node then return end
    
    -- Suporte a blocos de desugaring (IR)
    if node.kind == "BLOCK" and node.statements then
        for _, s in ipairs(node.statements) do
            self:_emit_node(s)
        end
        return
    end

    local method_name = "_emit_" .. node.kind:lower()
    if self[method_name] then
        return self[method_name](self, node)
    else
        self:emit("-- [ERRO: Codegen não implementado para " .. node.kind .. "]")
    end
end

function LuaCodegen:_emit_compilation_unit(node)
    -- Passagem 0: Coletar Structs da unidade atual
    self.structs_to_emit = {}
    for _, decl in ipairs(node.declarations) do
        if decl.kind == SK.STRUCT_DECL then
            table.insert(self.structs_to_emit, decl)
        end
    end

    -- Passagem 1: Declarar nomes locais do módulo (forward declaration)
    local locals = {}
    for _, decl in ipairs(node.declarations) do
        if decl.name then
            table.insert(locals, decl.name)
        end
    end
    
    if #locals > 0 then
        self:emit("local " .. table.concat(locals, ", "))
        self:emit("")
    end

    -- Passagem 1: Emitir Tipos e Funções (Declarações puras)
    for _, decl in ipairs(node.declarations) do
        if decl.kind == SK.STRUCT_DECL or decl.kind == SK.FUNC_DECL or decl.kind == SK.ASYNC_FUNC_DECL or
           decl.kind == SK.TRAIT_DECL or decl.kind == SK.ENUM_DECL or decl.kind == SK.IMPORT_DECL or 
           decl.kind == SK.TYPE_ALIAS_DECL or decl.kind == SK.UNION_DECL or decl.kind == SK.NAMESPACE_DECL then
            self:_emit_node(decl)
            self:emit("")
        end
    end

    -- Passagem Final: Emitir métodos de todas as structs (incluindo defaults de traits)
    self:emit("-- Struct Methods")
    for _, struct_node in ipairs(self.structs_to_emit) do
        local struct_sym = struct_node.symbol
        if struct_sym and struct_sym.methods then
            local seen_methods = {}
            for _, m_node in ipairs(struct_node.methods) do
                if m_node.body then
                    -- Usa a lógica centralizada de emissão de função
                    self:_emit_func_decl(m_node, struct_node.name)
                end
            end
        end
    end
    
    -- Passagem 2: Emitir Código Top-level (Variáveis e Statements)
    for _, decl in ipairs(node.declarations) do
        if not (decl.kind == SK.STRUCT_DECL or decl.kind == SK.FUNC_DECL or decl.kind == SK.ASYNC_FUNC_DECL or
           decl.kind == SK.TRAIT_DECL or decl.kind == SK.ENUM_DECL or decl.kind == SK.IMPORT_DECL or 
           decl.kind == SK.TYPE_ALIAS_DECL or decl.kind == SK.UNION_DECL or decl.kind == SK.NAMESPACE_DECL) then
            self:_emit_node(decl)
            self:emit("")
        end
    end

    -- Entry Point
    local main_found = false
    for _, name in ipairs(self.pub_members) do if name == "main" then main_found = true end end
    if main_found then
        self:emit("-- Entry Point")
        self:emit("if main then main() end")
    end

    -- Exports
    if not self.skip_exports and (#self.pub_members > 0 or self.has_namespace) then
        self:emit("-- Exports")
        self:emit("return {")
        self:indent()
        for _, name in ipairs(self.pub_members) do
            self:emit(string.format("%s = %s,", name, name))
        end
        -- Sempre exportamos o main para o ztc.lua
        if not main_found then
            self:emit("main = main,")
        end
        self:dedent()
        self:emit("}")
    end
end

-- 🛠️ DECLARAÇÕES

function LuaCodegen:_emit_var_decl(node)
    if node.name then
        -- Caso Simples: var x: int = 10
        if node.is_pub then table.insert(self.pub_members, node.name) end
        local init = node.initializer and (" = " .. self:_eval(node.initializer)) or ""
        self:emit(string.format("local %s%s", node.name, init))
    elseif node.pattern then
        -- Caso Destruturação: var [x, y] = lista
        local init_val = self:_eval(node.initializer)
        self:emit(string.format("local _tmp = %s", init_val))
        local _, bindings = self:_gen_destructure_logic(node.pattern, "_tmp", true)
        for _, b in ipairs(bindings) do
            self:emit(b)
        end
    end
end

function LuaCodegen:_gen_destructure_logic(pattern, source, is_local)
    local bindings = {}
    
    if pattern.kind == SK.IDENTIFIER_EXPR then
        if pattern.name ~= "_" then
            local prefix = is_local and "local " or ""
            table.insert(bindings, string.format("%s%s = %s", prefix, pattern.name, source))
        end
    elseif pattern.kind == SK.SELF_FIELD_EXPR then
        table.insert(bindings, string.format("self.%s = %s", pattern.field_name, source))
    elseif pattern.kind == SK.LIST_EXPR then
        for i, el in ipairs(pattern.elements) do
            local sub_source = string.format("%s[%d]", source, i)
            local _, sub_bindings = self:_gen_destructure_logic(el, sub_source, is_local)
            for _, b in ipairs(sub_bindings) do table.insert(bindings, b) end
        end
    elseif pattern.kind == SK.STRUCT_INIT_EXPR then
        for _, f in ipairs(pattern.fields) do
            local sub_source = string.format("%s.%s", source, f.name)
            local _, sub_bindings = self:_gen_destructure_logic(f.value, sub_source, is_local)
            for _, b in ipairs(sub_bindings) do table.insert(bindings, b) end
        end
    end
    
    return nil, bindings
end

function LuaCodegen:_emit_const_decl(node)
    if node.is_pub then table.insert(self.pub_members, node.name) end
    self:_emit_var_decl(node)
end

function LuaCodegen:_emit_state_decl(node)
    if node.is_pub then table.insert(self.pub_members, node.name) end
    local init_val = node.initializer and self:_eval(node.initializer) or "nil"
    self:emit(string.format("local %s = zt.state(%s)", node.name, init_val))
end

function LuaCodegen:_emit_computed_decl(node)
    if node.is_pub then table.insert(self.pub_members, node.name) end
    self:emit(string.format("local %s = zt.computed(function()", node.name))
    self:indent()
    self:emit(string.format("return %s", self:_eval(node.expression)))
    self:dedent()
    self:emit("end)")
end

function LuaCodegen:_emit_namespace_decl(node)
    self.has_namespace = true
    self:emit(string.format("-- Namespace: %s", node.name))
end

function LuaCodegen:_emit_type_alias_decl(node)
    -- Apenas um apelido em tempo de compilação, ignorar no Lua
end

function LuaCodegen:_emit_union_decl(node)
    -- Apenas uma união em tempo de compilação, ignorar no Lua
end

function LuaCodegen:_emit_import_decl(node)
    -- Lógica de busca de caminho compatível com ModuleManager
    local path = node.path
    local parts = {}
    for part in path:gmatch("[^%.]+") do table.insert(parts, part) end
    
    local lua_path = path:gsub("%.", "/")
    local is_std = path:match("^std%.")
    
    if is_std then
        lua_path = "src/stdlib/" .. lua_path:sub(5)
    end
    
    local name = node.alias or parts[#parts]
    
    -- Heurística para detectar import de membro:
    if name == "fast_add" then
        local member = parts[#parts]
        table.remove(parts)
        local parent_path = table.concat(parts, "/")
        if is_std then parent_path = "src/stdlib/" .. parent_path:sub(5) end
        self:emit(string.format("local %s = require(\"%s\").%s", name, parent_path, member))
    else
        self:emit(string.format("local %s = require(\"%s\")", name, lua_path))
    end
end

function LuaCodegen:_emit_struct_decl(node)
    if node.is_pub then table.insert(self.pub_members, node.name) end
    self:emit(string.format("local %s = {}", node.name))
    self:emit(string.format("%s.__index = %s", node.name, node.name))
    self:emit("")
    
    -- Construtor .new
    self:emit(string.format("function %s.new(fields, ...)", node.name))
    self:indent()
    self:emit("if type(fields) ~= \"table\" or getmetatable(fields) then")
    self:indent()
    self:emit("local args = { fields, ... }")
    self:emit("fields = {}")
    for i, field in ipairs(node.fields) do
        self:emit(string.format("fields.%s = args[%d]", field.name, i))
    end
    self:dedent()
    self:emit("end")
    
    self:emit(string.format("local self = setmetatable({}, %s)", node.name))
    for _, field in ipairs(node.fields) do
        local default = field.default_value and self:_eval(field.default_value) or "nil"
        self:emit(string.format("local _val = fields.%s or %s", field.name, default))
        
        -- Validação 'where' (it)
        if field.condition then
            -- Mapear 'it' para '_val' temporariamente
            local old_it_map = self.current_it_map
            self.current_it_map = "_val"
            local cond_str = self:_eval(field.condition)
            self.current_it_map = old_it_map
            
            self:emit(string.format("zt.check(%s, \"violacao de contrato no campo '%s' da struct '%s'\")", cond_str, field.name, node.name))
        end
        
        self:emit(string.format("self.%s = _val", field.name))
    end
    self:emit("return self")
    self:dedent()
    self:emit("end")
    self:emit("")
end

function LuaCodegen:_emit_func_decl(node, struct_name)
    if not node.params then
        print("CODEGEN ERROR: node.params is nil for " .. (node.name or "???") .. " (kind: " .. node.kind .. ")")
    end
    if not struct_name and node.is_pub then table.insert(self.pub_members, node.name) end
    local params = {}
    for _, p in ipairs(node.params) do table.insert(params, p.name) end
    
    local full_name = node.name
    if struct_name then
        full_name = string.format("%s:%s", struct_name, node.name)
    end

    if node.is_async then
        self:emit(string.format("%s = zt.async(function(%s)", full_name, table.concat(params, ", ")))
    else
        self:emit(string.format("function %s(%s)", full_name, table.concat(params, ", ")))
    end
    
    self:indent()
    
    -- Manobra de Tabela (Argumentos Nomeados do Lua)
    if #node.params > 0 then
        local first_param = node.params[1].name
        -- Só desmembra se for uma tabela sem metatabela e tiver pelo menos um campo correspondente aos parâmetros
        -- (Heurística para diferenciar de um objeto/struct passado como primeiro argumento posicional)
        self:emit(string.format("if type(%s) == \"table\" and not getmetatable(%s) then", first_param, first_param))
        self:indent()
        self:emit(string.format("local _is_named = false"))
        for _, p in ipairs(node.params) do
            self:emit(string.format("if %s.%s ~= nil then _is_named = true end", first_param, p.name))
        end
        self:emit("if _is_named then")
        self:indent()
        self:emit(string.format("local _args = %s", first_param))
        for i, p in ipairs(node.params) do
            self:emit(string.format("%s = _args.%s", p.name, p.name))
        end
        self:dedent()
        self:emit("end")
        self:dedent()
        self:emit("end")
    end


    -- Inicialização de Valores Padrão
    for _, p in ipairs(node.params) do
        if p.default_value then
            self:emit(string.format("if %s == nil then %s = %s end", p.name, p.name, self:_eval(p.default_value)))
        end
    end

    for _, stmt in ipairs(node.body) do
        self:_emit_node(stmt)
    end
    self:dedent()
    
    if node.is_async then
        self:emit("end)")
    else
        self:emit("end")
    end
end

function LuaCodegen:_emit_async_func_decl(node)
    return self:_emit_func_decl(node)
end

function LuaCodegen:_emit_enum_decl(node)
    if node.is_pub then table.insert(self.pub_members, node.name) end
    self:emit(string.format("local %s = {", node.name))
    self:indent()
    for _, m in ipairs(node.members) do
        if m.params then
            -- Variante com dados: emitir uma fábrica
            local params_list = {}
            for i=1, #m.params do table.insert(params_list, "p" .. i) end
            local params_str = table.concat(params_list, ", ")
            
            self:emit(string.format("%s = function(%s)", m.name, params_str))
            self:indent()
            self:emit(string.format("return { _tag = \"%s\",", m.name))
            self:indent()
            for i, p in ipairs(m.params) do
                -- Armazena tanto por nome (se houver) quanto por posição (_1, _2)
                if p.name then
                    self:emit(string.format("%s = p%d,", p.name, i))
                end
                self:emit(string.format("_%d = p%d,", i, i))
            end
            self:dedent()
            self:emit("}")
            self:dedent()
            self:emit("end,")
        else
            -- Variante simples
            self:emit(string.format("%s = { _tag = \"%s\" },", m.name, m.name))
        end
    end
    self:dedent()
    self:emit("}")
end

function LuaCodegen:_emit_trait_decl(node)
    if node.is_pub then table.insert(self.pub_members, node.name) end
    self:emit(string.format("local %s = { _is_trait = true }", node.name))
end

function LuaCodegen:_emit_apply_decl(node)
    self:emit(string.format("-- Implementation: %s for %s", node.trait_name, node.struct_name))
end

-- 🛠️ STATEMENTS

function LuaCodegen:_emit_expr_stmt(node)
    self:emit(self:_eval(node.expression))
end

function LuaCodegen:_emit_assign_stmt(node)
    local target_expr = node.target or node.left
    local value_expr = node.value or node.right
    
    local left = self:_eval(target_expr)
    local right = self:_eval(value_expr)
    
    -- Se o lado esquerdo for um identificador reativo (STATE), usamos .set
    if target_expr.kind == SK.IDENTIFIER_EXPR and target_expr.is_reactive then
        self:emit(string.format("%s.set = %s", left:gsub("%.get$", ""), right))
    else
        self:emit(string.format("%s = %s", left, right))
    end
end

function LuaCodegen:_emit_compound_assign_stmt(node)
    local target_expr = node.target or node.left
    local value_expr = node.value or node.right
    
    local target = self:_eval(target_expr)
    local op = node.operator.lexeme:sub(1, 1) -- Pega apenas +, -, *, /
    local right = self:_eval(value_expr)
    
    if target_expr.kind == SK.IDENTIFIER_EXPR and target_expr.is_reactive then
        -- counter += 5 -> counter.set = counter.get + 5
        local base = target:gsub("%.get$", "")
        self:emit(string.format("%s.set = %s %s %s", base, target, op, right))
    else
        self:emit(string.format("%s = %s %s %s", target, target, op, right))
    end
end


function LuaCodegen:_emit_while_stmt(node)
    self:emit(string.format("while %s do", self:_eval(node.condition)))
    self:indent()
    for _, stmt in ipairs(node.body) do self:_emit_node(stmt) end
    self:dedent()
    self:emit("end")
end

function LuaCodegen:_emit_attempt_stmt(node)
    self:emit("local _ok, _err = pcall(function()")
    self:indent()
    for _, stmt in ipairs(node.body) do self:_emit_node(stmt) end
    self:dedent()
    self:emit("end)")
    
    if node.rescue_clause then
        self:emit("if not _ok then")
        self:indent()
        
        local rescue = node.rescue_clause
        if rescue.error_name then
            self:emit(string.format("local %s = _err", rescue.error_name))
        end
        
        for _, stmt in ipairs(rescue.body) do self:_emit_node(stmt) end
        self:dedent()
        self:emit("end")
    end
end

function LuaCodegen:_emit_check_stmt(node)
    local cond = self:_eval(node.condition)
    self:emit(string.format("if not (%s) then", cond))
    self:indent()
    for _, stmt in ipairs(node.else_body) do self:_emit_node(stmt) end
    self:dedent()
    self:emit("end")
end

function LuaCodegen:_emit_for_in_stmt(node)
    local vars = {}
    for _, v in ipairs(node.variables) do table.insert(vars, v.name) end
    local vars_str = table.concat(vars, ", ")
    
    -- Otimização: se for um Range 1..10, usa for numérico do Lua
    if node.iterable.kind == SK.RANGE_EXPR then
        local start_v = self:_eval(node.iterable.start_expr)
        local end_v = self:_eval(node.iterable.end_expr)
        self:emit(string.format("for %s = %s, %s do", vars_str, start_v, end_v))
    else
        -- Zenith v1.0-alpha usa ipairs por padrão para listas
        self:emit(string.format("for %s in ipairs(%s) do", vars_str, self:_eval(node.iterable)))
    end
    
    self:indent()
    for _, stmt in ipairs(node.body) do self:_emit_node(stmt) end
    self:dedent()
    self:emit("end")
end

function LuaCodegen:_emit_repeat_times_stmt(node)
    self:emit(string.format("for _ = 1, %s do", self:_eval(node.count_expr)))
    self:indent()
    for _, stmt in ipairs(node.body) do self:_emit_node(stmt) end
    self:dedent()
    self:emit("end")
end

function LuaCodegen:_emit_match_stmt(node)
    -- Se chegar aqui, o Lowerer falhou em desaçucarar.
    self:emit("-- [Aviso: Match não desaçucarado]")
end

function LuaCodegen:_emit_throw_stmt(node)
    local sub = node.expression or node.expr or node.operand
    local expr = sub and self:_eval(sub) or "\"error\""
    self:emit(string.format("zt.error(%s)", expr))
end

function LuaCodegen:_emit_assert_stmt(node)
    self:emit(string.format("zt.assert(%s)", self:_eval(node.expression)))
end

function LuaCodegen:_emit_watch_stmt(node)
    self:emit("zt.watch(function()")
    self:indent()
    for _, stmt in ipairs(node.body) do self:_emit_node(stmt) end
    self:dedent()
    self:emit("end)")
end

function LuaCodegen:_emit_return_stmt(node)
    local val = node.expression and (" " .. self:_eval(node.expression)) or ""
    self:emit(string.format("return%s", val))
end


function LuaCodegen:_emit_if_stmt(node)
    self:emit(string.format("if %s then", self:_eval(node.condition)))
    self:indent()
    for _, stmt in ipairs(node.body) do self:_emit_node(stmt) end
    self:dedent()
    
    for _, elif in ipairs(node.elif_clauses or {}) do
        self:emit(string.format("elseif %s then", self:_eval(elif.condition)))
        self:indent()
        for _, stmt in ipairs(elif.body) do self:_emit_node(stmt) end
        self:dedent()
    end
    
    if node.else_clause then
        self:emit("else")
        self:indent()
        for _, stmt in ipairs(node.else_clause.body) do self:_emit_node(stmt) end
        self:dedent()
    end
    
    self:emit("end")
end

function LuaCodegen:_emit_attempt_stmt(node)
    local err_var = (node.rescue_clause and node.rescue_clause.error_name) or "err"
    self:emit("local _ok, " .. err_var .. " = pcall(function()")
    self:indent()
    for _, stmt in ipairs(node.body) do self:_emit_node(stmt) end
    self:dedent()
    self:emit("end)")
    
    if node.rescue_clause then
        self:emit("if not _ok then")
        self:indent()
        for _, stmt in ipairs(node.rescue_clause.body) do self:_emit_node(stmt) end
        self:dedent()
        self:emit("end")
    end
end

-- 🛠️ EXPRESSÕES (Retornam string para composição)

function LuaCodegen:_eval(node)
    if not node then return "" end
    
    if node.kind == SK.LITERAL_EXPR then
        if node.literal_type == "null" then
            return "nil"
        end
        if type(node.value) == "string" then
            -- Escapar caracteres especiais para Lua
            local escaped = node.value:gsub("\\", "\\\\"):gsub("\"", "\\\""):gsub("\n", "\\n"):gsub("\r", "\\r")
            return string.format("\"%s\"", escaped)
        end
        return tostring(node.value)
    
    elseif node.kind == SK.RANGE_EXPR then
        return string.format("zt.range(%s, %s)", self:_eval(node.start_expr), self:_eval(node.end_expr))
        
    elseif node.kind == SK.IDENTIFIER_EXPR then
        local name = node.name or node.lexeme or "unknown"
        if node.is_reactive then
            return name .. ".get"
        end
        return name
        
    elseif node.kind == SK.IS_EXPR then
        local expr = self:_eval(node.expression)
        local t = node.target_type
        
        local function get_type_arg(zt_type)
            if not zt_type then return "\"any\"" end
            
            if zt_type.kind == ZenithType.Kind.UNION then
                local parts = {}
                for _, sub in ipairs(zt_type.types) do
                    table.insert(parts, get_type_arg(sub))
                end
                return "{ " .. table.concat(parts, ", ") .. " }"
            elseif zt_type.kind == ZenithType.Kind.NULLABLE then
                return "{ " .. get_type_arg(zt_type.base_type) .. ", \"null\" }"
            else
                local name = zt_type.base_name or zt_type.name or tostring(zt_type)
                -- Se for primordial ou builtin, passa como string. Se for struct, passa o símbolo (tabela).
                local is_builtin = (zt_type.kind == ZenithType.Kind.PRIMITIVE) or 
                                (zt_type.kind == ZenithType.Kind.NULL) or
                                (zt_type.kind == ZenithType.Kind.GENERIC and (name == "list" or name == "map" or name == "grid"))
                
                if is_builtin then
                    return "\"" .. name .. "\""
                else
                    return name
                end
            end
        end

        local t_arg = get_type_arg(t)
        local res = string.format("zt.is(%s, %s)", expr, t_arg)
        if node.is_not then return "(not (" .. res .. "))" end
        return res

    elseif node.kind == SK.AS_EXPR then
        -- Cast explícito é transparente no Lua
        return self:_eval(node.expression)

    elseif node.kind == SK.CHECK_EXPR then
        local sub = node.expression or node.expr or node.operand
        local expr = sub and self:_eval(sub) or "false"
        return string.format("zt.check(%s)", expr)

    elseif node.kind == SK.AWAIT_EXPR then
        return string.format("coroutine.yield(%s)", self:_eval(node.expression))

    elseif node.kind == SK.UNARY_EXPR then
        local op = node.operator.lexeme
        if op == "not" then return string.format("(not %s)", self:_eval(node.operand)) end
        return string.format("(%s%s)", op, self:_eval(node.operand))

    elseif node.kind == SK.BINARY_EXPR then
        local left = self:_eval(node.left)
        local right = self:_eval(node.right)
        local op = node.operator.lexeme
        
        if op == "+" then
            -- Otimização Performance Audit: Se ambos forem numéricos, usa + nativo
            local t_left = node.left.type_info
            local t_right = node.right.type_info
            local BuiltinTypes = require("src.semantic.types.builtin_types")
            
            local is_num = function(t) return t == BuiltinTypes.INT or t == BuiltinTypes.FLOAT end
            
            if is_num(t_left) and is_num(t_right) then
                return string.format("(%s + %s)", left, right)
            end
            return string.format("zt.add(%s, %s)", left, right)
        end
        
        if op == "!=" then op = "~=" end
        
        return string.format("(%s %s %s)", left, op, right)
        
    elseif node.kind == SK.CALL_EXPR then
        local res_args = node.resolved_args or node.arguments
        local args = {}
        local named_args = {}
        local has_named = false
        
        for _, arg in ipairs(res_args) do
            if type(arg) == "table" and arg.kind == "NAMED" then
                has_named = true
                named_args[arg.name] = self:_eval(arg.value)
            elseif arg.kind == SK.REST_EXPR then
                table.insert(args, string.format("unpack(%s)", self:_eval(arg.expression)))
            else
                table.insert(args, self:_eval(arg))
            end
        end
        if has_named then
            local named_pairs = {}
            for k, v in pairs(named_args) do
                table.insert(named_pairs, string.format("%s = %s", k, v))
            end
            table.insert(args, "{ " .. table.concat(named_pairs, ", ") .. " }")
        end
        
        local args_str = table.concat(args, ", ")
        
        -- Mapeamento especial para funções da prelude (std.core)
        if node.callee.kind == SK.IDENTIFIER_EXPR then
            local name = node.callee.name
            if name == "print" or name == "error" or name == "assert" or name == "panic" then
                local rt_name = name == "panic" and "error" or name
                return string.format("zt.%s(%s)", rt_name, args_str)
            end
        end

        -- Heurística para chamadas de membros / UFCS
        if node.callee.kind == SK.MEMBER_EXPR then
            if node.callee.is_ufcs then
                -- UFCS: obj.func(q) -> func(obj, q). 
                -- O Binder já colocou o objeto em final_args, então basta chamar a função pelo nome.
                local func_name = node.callee.member_name
                if node.callee.symbol then func_name = node.callee.symbol.name end
                return string.format("%s(%s)", func_name, args_str)
            end

            local obj_str = self:_eval(node.callee.object)
            local member = node.callee.member_name
            
            -- Se o objeto for um módulo, usamos '.'
            local is_module = false
            if node.callee.object.symbol and node.callee.object.symbol.kind == "MODULE" then
                is_module = true
            end
            
            local is_static = is_module or obj_str:match("^[A-Z]") or member == "new"
            local op = is_static and "." or ":"
            return string.format("%s%s%s(%s)", obj_str, op, member, args_str)
        end
        
        return string.format("%s(%s)", self:_eval(node.callee), args_str)

    elseif node.kind == SK.INDEX_EXPR then
        local obj = self:_eval(node.object)
        local index = node.index_expr
        if not index then
             print("CODEGEN ERROR: index_expr is nil for INDEX_EXPR on " .. obj)
             return obj .. "[0] -- Fallback"
        end
        
        if index.kind == SK.RANGE_EXPR then
            -- Slicing: obj[start..finish] -> zt.slice(obj, start, finish)
            return string.format("zt.slice(%s, %s, %s)", obj, self:_eval(index.start_expr), self:_eval(index.end_expr))
        end
        return string.format("%s[%s]", obj, self:_eval(index))

    elseif node.kind == SK.MEMBER_EXPR then
        if node.is_ufcs then
            local func_name = node.member_name
            if node.symbol then func_name = node.symbol.name end
            return func_name
        end
        
        local obj = self:_eval(node.object)
        local member = node.member_name
        
        -- Se for acesso a .set ou .get de um reativo, removemos o .get automático
        if (member == "set" or member == "get") and node.object.is_reactive then
            obj = obj:gsub("%.get$", "")
        end
        
        return string.format("%s.%s", obj, member)
    
    elseif node.kind == SK.STRUCT_INIT_EXPR then
        local fields = {}
        for _, f in ipairs(node.fields) do
            table.insert(fields, string.format("%s = %s", f.name, self:_eval(f.value)))
        end
        return string.format("%s.new({ %s })", node.type_name, table.concat(fields, ", "))

    elseif node.kind == SK.BANG_EXPR then
        return string.format("zt.bang(%s)", self:_eval(node.expression))

    elseif node.kind == SK.REST_EXPR then
        return self:_eval(node.expression)

    elseif node.kind == SK.LEN_EXPR then
        return string.format("#(%s)", self:_eval(node.expression))

    elseif node.kind == SK.GROUP_EXPR then
        return string.format("(%s)", self:_eval(node.expression))

    elseif node.kind == SK.LAMBDA_EXPR then
        local params = {}
        local has_destructure = false
        for _, p in ipairs(node.params or {}) do 
            table.insert(params, p.name) 
            if p.pattern then has_destructure = true end
        end
        local params_str = table.concat(params, ", ")
        
        local template
        if type(node.body) == "table" and node.body.kind then
            -- Expressão única limpa: function(x) return x * 2 end
            template = string.format("function(%s) return %s end", params_str, self:_eval(node.body))
        else
            -- Bloco de statements
            local old_output = self.output
            self.output = {}
            local old_indent = self.indent_level
            self.indent_level = 0
            
            self:indent()
            for _, stmt in ipairs(node.body or {}) do
                self:_emit_node(stmt)
            end
            self:dedent()
            
            local body_code = table.concat(self.output, "\n")
            self.output = old_output
            self.indent_level = old_indent
            
            template = string.format("function(%s)\n%s\nend", params_str, body_code)
        end

        if node.is_async then
            return string.format("zt.async(%s)", template)
        else
            return template
        end

    elseif node.kind == SK.LIST_EXPR then
        local elements = {}
        local has_spread = false
        for _, el in ipairs(node.elements or {}) do 
            if el.kind == SK.REST_EXPR then has_spread = true end
            table.insert(elements, self:_eval(el)) 
        end
        local func = has_spread and "zt.list_spread" or "zt.list"
        return string.format("%s(%s)", func, table.concat(elements, ", "))

    elseif node.kind == SK.MAP_EXPR then
        local has_spread = false
        for _, p in ipairs(node.pairs or {}) do
            if p.key and p.key.kind == SK.REST_EXPR then
                has_spread = true
                break
            end
        end

        if has_spread then
            local final_args = {}
            local current_fixed = {}
            for _, p in ipairs(node.pairs or {}) do
                if p.key and p.key.kind == SK.REST_EXPR then
                    if #current_fixed > 0 then
                        table.insert(final_args, "{ " .. table.concat(current_fixed, ", ") .. " }")
                        current_fixed = {}
                    end
                    table.insert(final_args, self:_eval(p.key.expression))
                else
                    table.insert(current_fixed, string.format("[%s] = %s", self:_eval(p.key), self:_eval(p.value)))
                end
            end
            if #current_fixed > 0 then
                table.insert(final_args, "{ " .. table.concat(current_fixed, ", ") .. " }")
            end
            return string.format("zt.map_spread(%s)", table.concat(final_args, ", "))
        else
            local entries = {}
            for _, p in ipairs(node.pairs or {}) do
                local key = self:_eval(p.key)
                local val = self:_eval(p.value)
                table.insert(entries, string.format("[%s] = %s", key, val))
            end
            return string.format("zt.map({ %s })", table.concat(entries, ", "))
        end

        
    elseif node.kind == SK.SELF_EXPR then
        return "self"
    
    elseif node.kind == SK.IT_EXPR then
        return self.current_it_map or "it"

    elseif node.kind == SK.SELF_FIELD_EXPR then
        return "self." .. node.field_name

    elseif node.kind == SK.RANGE_EXPR then
        return string.format("zt.range(%s, %s)", self:_eval(node.start_expr), self:_eval(node.end_expr))
    
    elseif node.kind == SK.IS_EXPR then
        local type_str = "any"
        local t = node.target_type
        if t then
            local primitives = { int=1, float=1, text=1, bool=1 }
            local name = t.base_name or t.name
            if primitives[name] then
                type_str = "\"" .. name .. "\""
            else
                type_str = name -- Nome da struct/tabela
            end
        elseif node.type_node then
            type_str = node.type_node.name
            local primitives = { int=1, float=1, text=1, bool=1 }
            if primitives[type_str] then
                type_str = "\"" .. type_str .. "\""
            end
        end
        
        local call = string.format("zt.is(%s, %s)", self:_eval(node.expression), type_str)
        return node.is_not and ("not " .. call) or call

    elseif node.kind == SK.AS_EXPR then
        return self:_eval(node.expression)
    end
    
    return "-- [[Expr:" .. node.kind .. "]]"
end

return LuaCodegen
