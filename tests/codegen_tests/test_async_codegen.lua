package.path = package.path .. ";./src/?.lua;./tests/?.lua;./?/init.lua;./src/?/init.lua"

local SourceText = require("src.source.source_text")
local Parser = require("src.syntax.parser.parser")
local Binder = require("src.semantic.binding.binder")
local LuaCodegen = require("src.backend.lua.lua_codegen")
local Lowerer = require("src.lowering.lowerer")
local ModuleManager = require("src.semantic.binding.module_manager")

local function transpile(code)
    local source = SourceText.new(code, "async_test.zt")
    local unit, diags = Parser.parse_string(code, "async_test.zt")
    if diags:has_errors() then
        error("Parser error: " .. diags:format(source))
    end

    local binder = Binder.new(diags, ModuleManager.new("."))
    binder:bind(unit, "async_test")
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

print("--- Rodando Teste Focado de Async/Await ---")

local code = [[
async func fetch() -> int
    return 10
end

async func calc() -> int
    var a: int = await fetch()
    var twice = async (x: int) => x * 2
    return await twice(a + 11)
end

func main() -> int
    return calc()
end
]]

local lua_code = transpile(code)

if not lua_code:match("zt%.async%(") then
    print("--- LUA CODE ---")
    print(lua_code)
    print("----------------")
    error("Esperava emissão de zt.async no codegen")
end

if not lua_code:match("zt%.await%(") then
    print("--- LUA CODE ---")
    print(lua_code)
    print("----------------")
    error("Esperava await lowered para zt.await")
end

local result = run_lua(lua_code)
if result ~= 42 then
    error("Esperava main() == 42, obtive " .. tostring(result))
end

print("✅ Async/Await: OK")
