-- tests/codegen_tests/test_codegen.lua
package.path = package.path .. ";./src/?.lua;./tests/?.lua;./?/init.lua;./src/?/init.lua"

local runner_mod = require("tests.test_runner")
local a = runner_mod.assert

local SourceText = require("src.source.source_text")
local Parser     = require("src.syntax.parser.parser")
local Binder     = require("src.semantic.binding.binder")
local LuaCodegen = require("src.backend.lua.lua_codegen")

local function transpile(code)
    local source = SourceText.new(code, "test.zt")
    local unit, diags = Parser.parse_string(code, "test.zt")
    if diags:has_errors() then error("Parser error: " .. diags:format(source)) end
    
    local ModuleManager = require("src.semantic.binding.module_manager")
    local mm = ModuleManager.new(".")
    local binder = Binder.new(diags, mm)
    binder:bind(unit, "test")
    if diags:has_errors() then error("Binder error: " .. diags:format(source)) end
    
    local Lowerer = require("src.lowering.lowerer")
    local lowerer = Lowerer.new(diags)
    unit = lowerer:lower(unit)

    local codegen = LuaCodegen.new()
    local lua = codegen:generate_body(unit)
    print("--- DEBUG: LUA GENERATED ---")
    print(lua)
    print("----------------------------")
    return lua
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

print("--- Rodando Testes de Codegen (End-to-End) ---")

-- 1. Teste de Reatividade (ZenEngine)
local code1 = [[
var counter: int = 10
state s_counter: int = 0
var result: any = 0

func run_test() -> int
    s_counter.set = 5
    watch
        result = s_counter.get * 2
    end
    s_counter.set = 10
    return result as int
end
]]

print("Executando Teste 1 (Reatividade)...")
local lua1 = transpile(code1)
local res1 = run_lua(lua1 .. "\nreturn run_test()")

if res1 == 20 then
    print("✅ Teste 1 (Reatividade): OK (20)")
else
    print("❌ Teste 1 (Reatividade): FALHOU (esperado 20, obtido " .. tostring(res1) .. ")")
    os.exit(1)
end

-- 2. Teste de Structs e Métodos
local code2 = [[
struct Point
    pub x: int
    pub y: int
    
    pub func sum() -> int
        return self.x + self.y
    end
end

func run_test() -> int
    var p: Point = Point.new(x: 10, y: 20)
    return p.sum()
end
]]

print("Executando Teste 2 (Structs)...")
local lua2 = transpile(code2)
local res2 = run_lua(lua2 .. "\nreturn run_test()")

if res2 == 30 then
    print("✅ Teste 2 (Structs): OK (30)")
else
    print("❌ Teste 2 (Structs): FALHOU (esperado 30, obtido " .. tostring(res2) .. ")")
    os.exit(1)
end

-- 3. Teste de Attempt/Rescue
local code3 = [[
func fail()
    var n: int? = Empty
    var x: int = n!
end

func run_test() -> text
    var msg: text = "ok"
    attempt
        fail()
    rescue e
        msg = "error"
    end
    return msg
end
]]

print("Executando Teste 3 (Attempt/Rescue)...")
local lua3 = transpile(code3)
local res3 = run_lua(lua3 .. "\nreturn run_test()")

if res3 == "error" then
    print("✅ Teste 3 (Attempt/Rescue): OK (error)")
else
    print("--- LUA CODE (TEST 3) ---")
    print(lua3)
    print("-------------------------")
    print("❌ Teste 3 (Attempt/Rescue): FALHOU (esperado 'error', obtido '" .. tostring(res3) .. "')")
    os.exit(1)
end

print("\n--- Testes Codegen Finalizados ---")
