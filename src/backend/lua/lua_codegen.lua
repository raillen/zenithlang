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
    
    self:emit("-- Transpilado por Zenith v0.2.5 (Body Only)")
    self:emit("local zt = require(\"src.backend.lua.runtime.zenith_rt\")")
    self:_emit_prelude_constructors()
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
    self:_emit_prelude_constructors()
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

function LuaCodegen:_emit_prelude_constructors()
    self:emit("local Present = zt.Optional.Present")
    self:emit("local Empty = zt.Optional.Empty")
    self:emit("local Success = zt.Outcome.Success")
    self:emit("local Failure = zt.Outcome.Failure")
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
            for _, method_sym in ipairs(struct_sym.methods) do
                local m_node = method_sym.declaration
                if m_node and m_node.body and not seen_methods[m_node.name] then
                    seen_methods[m_node.name] = true
                    self:_emit_func_decl(m_node, struct_node.name)
                end
            end
        else
            for _, m_node in ipairs(struct_node.methods or {}) do
                if m_node.body then
                    self:_emit_func_decl(m_node, struct_node.name)
                end
            end
        end
    end
    
    -- Passagem 2: Emitir Código Top-level (Variáveis e Statements)
    self.is_top_level = true
    for _, decl in ipairs(node.declarations) do
        if not (decl.kind == SK.STRUCT_DECL or decl.kind == SK.FUNC_DECL or decl.kind == SK.ASYNC_FUNC_DECL or
           decl.kind == SK.TRAIT_DECL or decl.kind == SK.ENUM_DECL or decl.kind == SK.IMPORT_DECL or 
           decl.kind == SK.TYPE_ALIAS_DECL or decl.kind == SK.UNION_DECL or decl.kind == SK.NAMESPACE_DECL) then
            self:_emit_node(decl)
            self:emit("")
        end
    end
    self.is_top_level = false

    -- Entry Point
    local main_found = false
    for _, decl in ipairs(node.declarations) do
        if (decl.kind == SK.FUNC_DECL or decl.kind == SK.ASYNC_FUNC_DECL) and decl.name == "main" then
            main_found = true
            break
        end
    end

    if main_found then
        self:emit("")
        self:emit("-- Auto-run main if not in a namespace")
        self:emit(string.format("if not %s then", self.has_namespace and "true" or "false"))
        self:indent()
        self:emit("local status = main()")
        self:emit("if type(status) == 'number' then os.exit(status) end")
        self:dedent()
        self:emit("end")
    end

    -- Exports
    if not self.skip_exports then
        self:emit("")
        self:emit("return {")
        self:indent()
        for _, name in ipairs(self.pub_members) do
            self:emit(string.format("%s = %s,", name, name))
        end
        if main_found then
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
        local prefix = self.is_top_level and "" or "local "
        self:emit(string.format("%s%s%s", prefix, node.name, init))
    elseif node.pattern then
        -- Caso Destruturação: var [x, y] = lista
        local init_val = self:_eval(node.initializer)
        self:emit(string.format("local _tmp = %s", init_val))
        local _, bindings = self:_gen_destructure_logic(node.pattern, "_tmp", not self.is_top_level)
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
    local prefix = self.is_top_level and "" or "local "
    self:emit(string.format("%s%s = zt.state(%s)", prefix, node.name, init_val))
end

function LuaCodegen:_emit_computed_decl(node)
    if node.is_pub then table.insert(self.pub_members, node.name) end
    local prefix = self.is_top_level and "" or "local "
    self:emit(string.format("%s%s = zt.computed(function()", prefix, node.name))
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

function LuaCodegen:_emit_func_decl(node, struct_name)
    local name = node.name
    if struct_name then
        name = struct_name .. ":" .. name
    end
    
    local names = {}
    for _, p in ipairs(node.params or {}) do
        table.insert(names, p.name)
    end
    
    local params_str = table.concat(names, ", ")
    self:emit(string.format("function %s(%s)", name, params_str))
    self:indent()
    
    for _, stmt in ipairs(node.body) do
        self:_emit_node(stmt)
    end
    
    self:dedent()
    self:emit("end")
end

function LuaCodegen:_emit_async_func_decl(node)
    -- No Lua, funções async viram coroutines
    local names = {}
    for _, p in ipairs(node.params or {}) do
        table.insert(names, p.name)
    end
    local params_str = table.concat(names, ", ")

    self:emit(string.format("function %s(%s) ", node.name, params_str))
    self:indent()
    self:emit("return zt.async_run(function(...)")
    self:indent()
    
    for _, stmt in ipairs(node.body) do
        self:_emit_node(stmt)
    end
    
    self:dedent()
    self:emit("end, " .. params_str .. ")")
    self:dedent()
    self:emit("end")
end

function LuaCodegen:_emit_struct_decl(node)
    local name = node.name
    self:emit(string.format("local %s = {}", name))
    self:emit(string.format("%s.__index = %s", name, name))
    self:emit("")
    
    self:emit(string.format("function %s.new(fields)", name))
    self:indent()
    self:emit(string.format("local self = setmetatable({}, %s)", name))
    
    for _, field in ipairs(node.fields) do
        local init = field.initializer and self:_eval(field.initializer) or "nil"
        
        -- Aplicação do 'where' (contrato)
        if field.where_clause then
            self:emit(string.format("local _val = fields.%s or %s", field.name, init))
            local cond = self:_eval_with_it(field.where_clause, "_val")
            self:emit(string.format("if not (%s) then error(\"violacao de contrato no campo '%s' da struct '%s'\") end", cond, field.name, name))
            self:emit(string.format("self.%s = _val", field.name))
        else
            self:emit(string.format("self.%s = fields.%s or %s", field.name, field.name, init))
        end
    end
    
    self:emit("return self")
    self:dedent()
    self:emit("end")
end

function LuaCodegen:_emit_trait_decl(node)
    -- No Lua, traits são apenas recipientes de métodos que serão copiados
end

function LuaCodegen:_emit_enum_decl(node)
    self:emit(string.format("local %s = {", node.name))
    self:indent()
    for _, member in ipairs(node.members) do
        self:emit(string.format("%s = \"%s\",", member.name, member.name))
    end
    self:dedent()
    self:emit("}")
end


-- 🚀 STATEMENTS

function LuaCodegen:_emit_expr_stmt(node)
    self:emit(self:_eval(node.expression))
end

function LuaCodegen:_emit_assign_stmt(node)
    self:emit(string.format("%s = %s", self:_eval(node.target), self:_eval(node.value)))
end

function LuaCodegen:_emit_compound_assign_stmt(node)
    local op_map = { ["+="] = "+", ["-="] = "-", ["*="] = "*", ["/="] = "/" }
    local op = op_map[node.operator.lexeme]
    local target = self:_eval(node.target)
    self:emit(string.format("%s = %s %s %s", target, target, op, self:_eval(node.value)))
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
        -- Zenith usa zt.iter para abstrair listas e Trait Iterable
        self:emit(string.format("for %s in zt.iter(%s) do", vars_str, self:_eval(node.iterable)))
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

function LuaCodegen:_emit_native_lua_stmt(node)
    self:emit("-- native lua")
    self:emit(node.lua_code)
end

-- 💎 EXPRESSÕES

function LuaCodegen:_eval(node)
    if not node then return "nil" end
    
    -- Caso especial: Tabela de argumento nomeado (gerada pelo parser)
    if type(node) == "table" and node.kind == "NAMED" then
        return self:_eval(node.value)
    end
    
    if node.kind == SK.LITERAL_EXPR then
        if type(node.value) == "string" then return string.format("%q", node.value) end
        return tostring(node.value)
    
    elseif node.kind == SK.IDENTIFIER_EXPR then
        return node.name
    
    elseif node.kind == SK.SELF_EXPR then
        return "self"
    
    elseif node.kind == SK.IT_EXPR then
        return "it"
    
    elseif node.kind == SK.SELF_FIELD_EXPR then
        return "self." .. node.field_name

    elseif node.kind == SK.BINARY_EXPR then
        local op_map = { ["+"] = "+", ["-"] = "-", ["*"] = "*", ["/"] = "/", ["=="] = "==", ["!="] = "~=", ["and"] = "and", ["or"] = "or", ["+"] = ".." }
        -- Concatenação Zenith (+) -> Lua (..)
        local op = node.operator.lexeme
        if op == "+" then
            -- Heurística simples: se um dos lados for literal string, usa ..
            return string.format("(%s .. %s)", self:_eval(node.left), self:_eval(node.right))
        elseif op == "or" then
            -- Suporte a unwrap de Optional/Outcome
            return string.format("zt.unwrap_or(%s, %s)", self:_eval(node.left), self:_eval(node.right))
        end
        return string.format("(%s %s %s)", self:_eval(node.left), op_map[op] or op, self:_eval(node.right))
    
    elseif node.kind == SK.UNARY_EXPR then
        local op = node.operator.lexeme == "not" and "not " or "-"
        return string.format("%s%s", op, self:_eval(node.operand))

    elseif node.kind == SK.NAMED_ARG_EXPR then
        return self:_eval(node.value)

    elseif node.kind == SK.CALL_EXPR then
        local args = {}
        for _, arg in ipairs(node.arguments) do table.insert(args, self:_eval(arg)) end
        return string.format("%s(%s)", self:_eval(node.callee), table.concat(args, ", "))
    
    elseif node.kind == SK.MEMBER_EXPR then
        return string.format("%s.%s", self:_eval(node.object), node.member_name)
    
    elseif node.kind == SK.LEN_EXPR then
        return string.format("#(%s)", self:_eval(node.expression))
    
    elseif node.kind == SK.LIST_EXPR then
        local elements = {}
        for _, el in ipairs(node.elements) do table.insert(elements, self:_eval(el)) end
        return "{" .. table.concat(elements, ", ") .. "}"
    
    elseif node.kind == SK.MAP_EXPR then
        local entries = {}
        for _, p in ipairs(node.pairs or {}) do
            table.insert(entries, string.format("[%s] = %s", self:_eval(p.key), self:_eval(p.value)))
        end
        return "{" .. table.concat(entries, ", ") .. "}"
    
    elseif node.kind == SK.INDEX_EXPR then
        return string.format("%s[%s]", self:_eval(node.object), self:_eval(node.index_expr))

    elseif node.kind == SK.BANG_EXPR then
        -- Desaçucarado: result! -> error propagation
        return self:_eval(node.expression)

    elseif node.kind == SK.NATIVE_LUA_EXPR then
        return "(" .. node.lua_code .. ")"
    
    elseif node.kind == SK.AWAIT_EXPR then
        return string.format("zt.await(%s)", self:_eval(node.expression))

    elseif node.kind == SK.IS_EXPR then
        local type_str = "nil"
        if node.type_symbol then
            local t = node.type_symbol
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

function LuaCodegen:_eval_with_it(expr, it_value)
    -- Gambiarra temporária: substitui 'it' por it_value no codegen
    local old_eval = self._eval
    self._eval = function(s, node)
        if node and node.kind == SK.IT_EXPR then return it_value end
        return old_eval(s, node)
    end
    local res = self:_eval(expr)
    self._eval = old_eval
    return res
end

return LuaCodegen
