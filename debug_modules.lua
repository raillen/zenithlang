-- ============================================================================
-- Zenith Compiler — Debug Modules
-- Testa o carregamento de múltiplos módulos e resolução de namespaces.
-- ============================================================================

package.path = package.path .. ";./?.lua"

local ModuleManager   = require("src.semantic.binding.module_manager")
local Binder          = require("src.semantic.binding.binder")
local LuaCodegen      = require("src.backend.lua.lua_codegen")
local DiagnosticBag   = require("src.diagnostics.diagnostic_bag")

local function run_test(module_path)
    local manager = ModuleManager.new(".")
    local diags = DiagnosticBag.new()
    local binder = Binder.new(diags, manager)
    
    print("\n" .. string.rep("=", 60))
    print("TESTE: " .. module_path)
    print(string.rep("=", 60))
    
    print("--- 1. Carregando Módulo (" .. module_path .. ") ---")
    local main_unit, err = manager:get_or_load_ast(module_path)
    if not main_unit then
        print("ERRO:", err)
        return
    end
    
    print("--- 2. Binding (Resolução Semântica) ---")
    local scope, binder_diags = binder:bind(main_unit, module_path)
    
    if binder_diags:has_errors() then
        print("\n❌ Erros de Binding:")
        for _, d in ipairs(binder_diags.diagnostics) do
            local line = (d.span and d.span.start_line) or 0
            local col = (d.span and d.span.start_col) or 0
            print(string.format("[%s] %s em L%d:C%d", d.code, d.message, line, col))
        end
    else
        print("\n✅ Binding concluído com sucesso!")
        
        print("\n--- 3. Geração de Código Lua ---")
        local codegen = LuaCodegen.new()
        local lua_code = codegen:generate(main_unit)
        print(lua_code)
    end
end

run_test("tests.modules.main")
run_test("tests.modules.std_test")
