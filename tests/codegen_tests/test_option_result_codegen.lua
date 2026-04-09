-- tests/codegen_tests/test_option_result_codegen.lua
package.path = package.path .. ";./src/?.lua;./tests/?.lua;./?/init.lua;./src/?/init.lua"

local SourceText = require("src.source.source_text")
local Parser = require("src.syntax.parser.parser")
local Binder = require("src.semantic.binding.binder")
local Lowerer = require("src.lowering.lowerer")
local LuaCodegen = require("src.backend.lua.lua_codegen")
local ModuleManager = require("src.semantic.binding.module_manager")

local function transpile(code)
    local source = SourceText.new(code, "option_result_test.zt")
    local unit, diags = Parser.parse_string(code, "option_result_test.zt")
    if diags:has_errors() then
        error("Parser error: " .. diags:format(source))
    end

    local mm = ModuleManager.new(".")
    local binder = Binder.new(diags, mm)
    binder:bind(unit, "option_result_test")
    if diags:has_errors() then
        error("Binder error: " .. diags:format(source))
    end

    local lowerer = Lowerer.new(diags)
    unit = lowerer:lower(unit)
    if diags:has_errors() then
        error("Lowerer error: " .. diags:format(source))
    end

    local codegen = LuaCodegen.new()
    return codegen:generate_body(unit)
end

local function run_lua(lua_code)
    local loader = loadstring or load
    local f, err = loader(lua_code)
    if not f then
        print("--- LUA CODE ---")
        print(lua_code)
        print("----------------")
        error("Lua Load Error: " .. err)
    end
    return f()
end

print("--- Rodando Testes de Optional/Outcome (End-to-End) ---")

local code = [[
func divide(a: int, b: int) -> Optional<int>
    if b == 0
        return Empty
    end
    return Present(a / b)
end

func calculate_ok() -> Optional<int>
    var x = divide(10, 2)?
    var y = divide(x, 1)?
    return Present(y + 5)
end

func calculate_fail() -> Optional<int>
    var x = divide(10, 2)?
    var y = divide(x, 0)?
    return Present(y + 5)
end
]]

local lua_code = transpile(code)

local ok_value = run_lua(lua_code .. "\nreturn calculate_ok()._1")
if ok_value ~= 10 then
    print(lua_code)
    error("Esperado 10 em calculate_ok(), recebido " .. tostring(ok_value))
end

local fail_tag = run_lua(lua_code .. "\nreturn calculate_fail()._tag")
if fail_tag ~= "Empty" then
    print(lua_code)
    error("Esperado tag 'Empty' em calculate_fail(), recebido " .. tostring(fail_tag))
end

print("✅ Optional/Outcome + try operator: OK")
