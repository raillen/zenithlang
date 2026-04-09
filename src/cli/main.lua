-- ============================================================================
-- Zenith Compiler — Entry Point
-- ============================================================================

local VERSION = "1.0.0-alpha"
local CODENAME = "Zenith"

local SourceText  = require("src.source.source_text")
local Parser      = require("src.syntax.parser.parser")
local Binder      = require("src.semantic.binding.binder")
local LuaCodegen  = require("src.backend.lua.lua_codegen")

--- Imprime a ajuda do CLI.
local function print_help()
    print(string.format("Zenith Language Compiler v%s", VERSION))
    print("")
    print("Uso: zt <comando> [opções]")
    print("")
    print("Comandos:")
    print("  build              Compila o projeto completo")
    print("  run                Compila e executa com Lua")
    print("  check              Valida sem emitir Lua")
    print("  transpile <file>   Transpila um arquivo específico")
    print("  test               Executa testes (*_test.zt)")
    print("  version            Mostra a versão")
    print("  help               Mostra esta ajuda")
    print("")
end

--- Imprime a versão.
local function print_version()
    print(string.format("%s v%s", CODENAME, VERSION))
end

--- Entry point principal.
local function main(args)
    if #args == 0 then
        print_help()
        return 0
    end

    local command = args[1]

    if command == "version" or command == "--version" or command == "-v" then
        print_version()
        return 0
    elseif command == "help" or command == "--help" or command == "-h" then
        print_help()
        return 0
    elseif command == "build" then
        print("⚠️  Comando 'build' ainda não implementado.")
        return 1
    elseif command == "run" then
        print("⚠️  Comando 'run' ainda não implementado.")
        return 1
    elseif command == "check" then
        print("⚠️  Comando 'check' ainda não implementado.")
        return 1
    elseif command == "transpile" then
        local filename = args[2]
        if not filename then
            print("❌ Erro: transpile exige um arquivo de entrada.")
            print("   Exemplo: zt transpile main.zt")
            return 1
        end

        -- Ler arquivo
        local f = io.open(filename, "r")
        if not f then
            print(string.format("❌ Erro: arquivo '%s' não encontrado.", filename))
            return 1
        end
        local code = f:read("*a")
        f:close()

        local source = SourceText.new(code, filename)
        
        -- 1. Parsing
        local unit, diagnostics = Parser.parse(source)
        if diagnostics:has_errors() then
            print(diagnostics:format(source))
            return 1
        end

        -- 2. Binding
        local binder = Binder.new()
        local _, binder_diags = binder:bind(unit)
        if binder_diags:has_errors() then
            print(binder_diags:format(source))
            return 1
        end

        -- 3. Codegen
        local codegen = LuaCodegen.new()
        local lua_code = codegen:generate(unit)

        -- 4. Gravar Output
        local out_name = filename:gsub("%.zt$", "") .. ".lua"
        if out_name == filename then out_name = filename .. ".lua" end
        
        local out_f = io.open(out_name, "w")
        if out_f then
            out_f:write(lua_code)
            out_f:close()
            print(string.format("✅ Sucesso: '%s' -> '%s'", filename, out_name))
            return 0
        else
            print(string.format("❌ Erro ao gravar arquivo de saída '%s'", out_name))
            return 1
        end
    elseif command == "test" then
        print("⚠️  Comando 'test' ainda não implementado.")
        return 1
    else
        print(string.format("❌ Comando desconhecido: '%s'", command))
        print("   Use 'zt help' para ver os comandos disponíveis.")
        return 1
    end
end

-- Executa
local exit_code = main(arg or {})
os.exit(exit_code)
