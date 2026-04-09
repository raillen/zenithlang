package.path = package.path .. ";./src/?.lua;./tests/?.lua;./?/init.lua;./src/?/init.lua"

local SourceText = require("src.source.source_text")
local Parser = require("src.syntax.parser.parser")
local Binder = require("src.semantic.binding.binder")
local Lowerer = require("src.lowering.lowerer")
local LuaCodegen = require("src.backend.lua.lua_codegen")
local ModuleManager = require("src.semantic.binding.module_manager")

local function transpile(code)
    local source = SourceText.new(code, "iterable_test.zt")
    local unit, diags = Parser.parse_string(code, "iterable_test.zt")
    if diags:has_errors() then
        error("Parser error: " .. diags:format(source))
    end

    local mm = ModuleManager.new(".")
    local binder = Binder.new(diags, mm)
    binder:bind(unit, "iterable_test")
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

print("--- Rodando Teste de Trait Iterable ---")

local code = [[
struct Contador
    atual: int = 0
    limite: int = 0
end

apply Iterable<int> to Contador
    func iterator() -> () => Optional<int>
        return () => do
            native lua
                if self.atual < self.limite then
                    local val = self.atual
                    self.atual = self.atual + 1
                    return Present(val)
                end
                return Empty
            end
        end
    end
end

func run_iterable_test() -> int
    var c: Contador = Contador { atual: 0, limite: 5 }
    var soma = 0
    for i in c
        soma = soma + i
    end
    return soma
end
]]

local lua_code = transpile(code)
-- print(lua_code)

local soma = run_lua(lua_code .. "\nreturn run_iterable_test()")
if soma ~= 10 then -- 0 + 1 + 2 + 3 + 4 = 10
    error("Esperado soma 10, recebido " .. tostring(soma))
end

print("✅ Trait Iterable: OK")
