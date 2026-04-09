-- ============================================================================
-- Zenith Compiler — Debug Prelude
-- ============================================================================

package.path = package.path .. ";./?.lua"

local ModuleManager   = require("src.semantic.binding.module_manager")
local Binder          = require("src.semantic.binding.binder")
local LuaCodegen      = require("src.backend.lua.lua_codegen")
local DiagnosticBag   = require("src.diagnostics.diagnostic_bag")

local function test_prelude()
    local manager = ModuleManager.new(".")
    local diags = DiagnosticBag.new()
    local binder = Binder.new(diags, manager)
    
    print("--- 1. Carregando Teste de Prelude ---")
    local unit, err = manager:get_or_load_ast("tests.stdlib.test_prelude")
    if not unit then
        print("ERRO:", err)
        return
    end
    
    print("--- 2. Binding (Deve injetar std.core) ---")
    local scope, binder_diags = binder:bind(unit, "tests.stdlib.test_prelude")
    
    if binder_diags:has_errors() then
        print("\n❌ Erros de Binding:")
        for _, d in ipairs(binder_diags.diagnostics) do
            local line = (d.span and d.span.start_line) or 0
            local col = (d.span and d.span.start_col) or 0
            print(string.format("[%s] %s em L%d:C%d", d.code, d.message, line, col))
        end
    else
        print("\n✅ Prelude injetado com sucesso!")
        
        print("\n--- 3. Geração de Código Lua ---")
        local codegen = LuaCodegen.new()
        local lua_code = codegen:generate(unit)
        print(lua_code)
    end
end

test_prelude()
