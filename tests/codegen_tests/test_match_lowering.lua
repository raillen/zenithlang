package.path = package.path .. ";./src/?.lua;./tests/?.lua;./?/init.lua;./src/?/init.lua"

local SourceText = require("src.source.source_text")
local Parser = require("src.syntax.parser.parser")
local Binder = require("src.semantic.binding.binder")
local LuaCodegen = require("src.backend.lua.lua_codegen")
local Lowerer = require("src.lowering.lowerer")
local ModuleManager = require("src.semantic.binding.module_manager")

local function transpile(code)
    local source = SourceText.new(code, "match_test.zt")
    local unit, diags = Parser.parse_string(code, "match_test.zt")
    if diags:has_errors() then
        error("Parser error: " .. diags:format(source))
    end

    local binder = Binder.new(diags, ModuleManager.new("."))
    binder:bind(unit, "match_test")
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

print("--- Rodando Teste Focado de Lowering do match ---")

local code = [[
struct Player
    pub nome: text
    pub vida: int
end

func main() -> int
    var lista: any = [10, 20, 30, 40]
    var resto_len: int = 0
    var nome_ok: bool = false

    match lista
        case [10, a, ..resto]:
            check a == 20 else error("a deveria ser 20") end
            resto_len = #resto
        else:
            error("Deveria casar com a lista")
    end

    var p: Player = Player { nome: "Zenith", vida: 100 }
    match p
        case Player { nome, vida: 100 }:
            nome_ok = nome == "Zenith"
        else:
            error("Deveria casar com Player")
    end

    check resto_len == 2 else error("resto deveria ter tamanho 2") end
    check nome_ok else error("nome deveria casar") end
    return 0
end
]]

local lua_code = transpile(code)

if not lua_code:match("zt%.slice%(_m, 2%)") then
    print("--- LUA CODE ---")
    print(lua_code)
    print("----------------")
    error("Esperava lowering do rest pattern para zt.slice(_m, 2)")
end

local result = run_lua(lua_code)
if result ~= 0 then
    error("Esperava main() == 0, obtive " .. tostring(result))
end

print("✅ Lowering de match: OK")
