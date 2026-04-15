package.path = package.path .. ";./src/?.lua;./tests/?.lua;./?/init.lua;./src/?/init.lua"

local SourceText = require("src.source.source_text")
local Parser = require("src.syntax.parser.parser")
local Binder = require("src.semantic.binding.binder")
local LuaCodegen = require("src.backend.lua.lua_codegen")
local Lowerer = require("src.lowering.lowerer")
local ModuleManager = require("src.semantic.binding.module_manager")

local function transpile(code)
    local source = SourceText.new(code, "index_assignment_test.zt")
    local unit, diags = Parser.parse_string(code, "index_assignment_test.zt")
    if diags:has_errors() then
        error("Parser error: " .. diags:format(source))
    end

    local binder = Binder.new(diags, ModuleManager.new("."))
    binder:bind(unit, "index_assignment_test")
    if diags:has_errors() then
        error("Binder error: " .. diags:format(source))
    end

    unit = Lowerer.new(diags):lower(unit)

    local codegen = LuaCodegen.new()
    return codegen:generate_body(unit)
end

local function run_lua(lua_code)
    local loader = loadstring or load
    local chunk, err = loader(lua_code .. "\nreturn main()")
    if not chunk then
        print("--- LUA CODE ---")
        print(lua_code)
        print("----------------")
        error("Lua Load Error: " .. err)
    end
    return chunk()
end

print("--- Rodando Teste Focado de Atribuicao por Indice ---")

local code = [[
func main() -> int
    var xs: any = [10, 20]
    xs[1] = 99
    xs[2] += 1
    return xs[1] + xs[2]
end
]]

local lua_code = transpile(code)

if lua_code:find("zt.index_any(xs, 1) =", 1, true) then
    print("--- LUA CODE ---")
    print(lua_code)
    print("----------------")
    error("Lvalue indexado foi emitido como chamada de leitura invalida")
end

local result = run_lua(lua_code)
if result ~= 120 then
    error("Esperava main() == 120, obtive " .. tostring(result))
end

print("✅ Atribuicao por indice: OK")
