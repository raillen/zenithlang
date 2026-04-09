package.path = package.path .. ";./src/?.lua;./tests/?.lua;./?/init.lua;./src/?/init.lua"

local SourceText = require("src.source.source_text")
local Parser = require("src.syntax.parser.parser")
local Binder = require("src.semantic.binding.binder")
local Lowerer = require("src.lowering.lowerer")
local LuaCodegen = require("src.backend.lua.lua_codegen")
local ModuleManager = require("src.semantic.binding.module_manager")

local function transpile(code)
    local source = SourceText.new(code, "native_test.zt")
    local unit, diags = Parser.parse_string(code, "native_test.zt")
    if diags:has_errors() then
        error("Parser error: " .. diags:format(source))
    end

    local mm = ModuleManager.new(".")
    local binder = Binder.new(diags, mm)
    binder:bind(unit, "native_test")
    if diags:has_errors() then
        error("Binder error: " .. diags:format(source))
    end

    local lowerer = Lowerer.new(diags)
    unit = lowerer:lower(unit)

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

print("--- Rodando Teste de Blocos Native Lua ---")

local code = [[
func test_native_stmt() -> int
    var x = 0
    native lua
        x = 10
    end
    return x
end

func test_native_expr() -> int
    var y = (native lua 20 + 22 end) as int
    return y
end
]]

local lua_code = transpile(code)
print(lua_code)

local stmt_val = run_lua(lua_code .. "\nreturn test_native_stmt()")
if stmt_val ~= 10 then
    error("Esperado 10 em test_native_stmt(), recebido " .. tostring(stmt_val))
end

local expr_val = run_lua(lua_code .. "\nreturn test_native_expr()")
if expr_val ~= 42 then
    error("Esperado 42 em test_native_expr(), recebido " .. tostring(expr_val))
end

print("✅ Blocos Native Lua: OK")
