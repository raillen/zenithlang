package.path = package.path .. ";./src/?.lua;./tests/?.lua;./?/init.lua;./src/?/init.lua"

local SourceText = require("src.source.source_text")
local Parser = require("src.syntax.parser.parser")
local Binder = require("src.semantic.binding.binder")
local LuaCodegen = require("src.backend.lua.lua_codegen")
local Lowerer = require("src.lowering.lowerer")
local ModuleManager = require("src.semantic.binding.module_manager")

local function transpile(code)
    local source = SourceText.new(code, "validate_test.zt")
    local unit, diags = Parser.parse_string(code, "validate_test.zt")
    if diags:has_errors() then
        error("Parser error: " .. diags:format(source))
    end

    local binder = Binder.new(diags, ModuleManager.new("."))
    binder:bind(unit, "validate_test")
    if diags:has_errors() then
        error("Binder error: " .. diags:format(source))
    end

    unit = Lowerer.new(diags):lower(unit)

    local codegen = LuaCodegen.new()
    return codegen:generate_body(unit)
end

local function run_lua(lua_code, entrypoint)
    local loader = loadstring or load
    local chunk, err = loader(lua_code .. "\nreturn " .. entrypoint .. "()")
    if not chunk then
        print("--- LUA CODE ---")
        print(lua_code)
        print("----------------")
        error("Lua Load Error: " .. err)
    end
    return chunk()
end

print("--- Rodando Teste Focado de Lowering do validate ---")

local code = [[
import std.validation as validation

struct Item
    pub quantidade: int validate validation.min_value(0) where it <= 10
end

func run_test_ok() -> int
    var item: Item = Item { quantidade: 4 }
    return item.quantidade
end

func run_test_fail() -> text
    var msg: text = "ok"
    attempt
        var item: Item = Item { quantidade: 11 }
    rescue e
        msg = "error"
    end
    return msg
end
]]

local lua_code = transpile(code)

if not lua_code:match("validation%.min_value%(_val,%s*0%)") then
    print("--- LUA CODE ---")
    print(lua_code)
    print("----------------")
    error("Esperava validate lowered para validation.min_value(_val, 0)")
end

if not lua_code:match("_val%s*<=%s*10") then
    print("--- LUA CODE ---")
    print(lua_code)
    print("----------------")
    error("Esperava where lowered com _val <= 10")
end

if lua_code:match("[^_%w]it%s*<=%s*10") then
    print("--- LUA CODE ---")
    print(lua_code)
    print("----------------")
    error("A condicao combinada ainda vazou com it para o codegen")
end

local ok_value = run_lua(lua_code, "run_test_ok")
local fail_value = run_lua(lua_code, "run_test_fail")

if ok_value ~= 4 then
    error("Esperava run_test_ok() == 4, obtive " .. tostring(ok_value))
end

if fail_value ~= "error" then
    error("Esperava run_test_fail() == 'error', obtive " .. tostring(fail_value))
end

print("validate lowering: OK")
