-- ============================================================================
-- Zenith Compiler — Module Manager
-- Gerencia o carregamento de arquivos, cache de módulos e resolução de caminhos.
-- ============================================================================

local Parser     = require("src.syntax.parser.parser")
local SourceText = require("src.source.source_text")

local ModuleManager = {}
ModuleManager.__index = ModuleManager

function ModuleManager.new(project_root)
    local self = setmetatable({}, ModuleManager)
    self.project_root = project_root or "."
    self.cache = {} -- module_name -> { ast, scope, path }
    self.stdlib_path = "src/stdlib" -- Caminho base da stdlib
    return self
end

--- Resolve o nome de um módulo para um caminho de arquivo.
function ModuleManager:resolve_path(module_name)
    -- Se começa com std., busca na biblioteca padrão
    if module_name:match("^std%.") then
        local sub = module_name:sub(5)
        return self.stdlib_path .. "/" .. sub:gsub("%.", "/") .. ".zt"
    end
    
    -- Caso contrário, busca no diretório do projeto
    return self.project_root .. "/" .. module_name:gsub("%.", "/") .. ".zt"
end

--- Carrega um módulo (parse apenas).
function ModuleManager:get_or_load_ast(module_name)
    if self.cache[module_name] then
        return self.cache[module_name].ast
    end
    
    local path = self:resolve_path(module_name)
    local file = io.open(path, "r")
    if not file then
        return nil, "Módulo não encontrado: " .. module_name .. " em " .. path
    end
    
    local content = file:read("*all")
    file:close()
    
    local source = SourceText.new(content, path)
    local unit, diags = Parser.parse(source)
    
    if diags:has_errors() then
        return nil, "Erro de parser no módulo " .. module_name .. ":\n" .. diags:format(source)
    end

    -- DESCOBERTA DE ZDOC (v0.3.1)
    local zdoc_path = path:gsub("%.zt$", ".zdoc")
    local zdoc_file = io.open(zdoc_path, "r")
    if zdoc_file then
        local zdoc_content = zdoc_file:read("*all")
        zdoc_file:close()
        local zdoc_source = SourceText.new(zdoc_content, zdoc_path)
        local zdoc_unit, zdoc_diags = Parser.parse(zdoc_source)
        
        if not zdoc_diags:has_errors() then
            -- Mescla doc_comments do .zdoc no unit principal
            for _, dc in ipairs(zdoc_unit.doc_comments or {}) do
                table.insert(unit.doc_comments, dc)
            end
        end
    end
    
    self.cache[module_name] = {
        ast = unit,
        source = source,
        path = path,
        name = module_name
    }
    
    return unit
end

--- Registra o escopo já processado de um módulo.
function ModuleManager:register_scope(module_name, scope)
    if self.cache[module_name] then
        self.cache[module_name].scope = scope
    end
end

function ModuleManager:get_scope(module_name)
    return self.cache[module_name] and self.cache[module_name].scope
end

--- Retorna o escopo do std.core (prelude).
function ModuleManager:get_prelude_scope()
    local prelude_name = "std.core"
    local scope = self:get_scope(prelude_name)
    if scope then return scope end
    
    -- Se não estiver no cache, carrega e faz o bind
    local ast, err = self:get_or_load_ast(prelude_name)
    if not ast then
        error("FALHA CRÍTICA: Não foi possível carregar a Prelude (std.core): " .. tostring(err))
    end
    
    local Binder = require("src.semantic.binding.binder")
    local DiagnosticBag = require("src.diagnostics.diagnostic_bag")
    
    -- Criamos um binder especial para o std.core (sem prelude recursivo!)
    local dummy_diags = DiagnosticBag.new()
    local core_binder = Binder.new(dummy_diags, self)
    core_binder.skip_prelude = true -- Evita recursão infinita
    
    local core_scope, b_diags = core_binder:bind(ast, prelude_name)
    if dummy_diags:has_errors() then
        local core_source = self.cache[prelude_name] and self.cache[prelude_name].source
        error("FALHA CRÍTICA: Erros semânticos na Prelude (std.core):\n" .. dummy_diags:format(core_source))
    end

    -- APLICAR LOWERING NA PRELUDE TAMBÉM
    local Lowerer = require("src.lowering.lowerer")
    local lowerer = Lowerer.new(dummy_diags)
    ast = lowerer:lower(ast)
    self.cache[prelude_name].ast = ast -- Atualiza no cache com a versão otimizada

    return core_scope
end

return ModuleManager
