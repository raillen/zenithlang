-- Zenith Compiler CLI (ztc.lua)
local Parser        = require("src.syntax.parser.parser")
local Binder        = require("src.semantic.binding.binder")
local ModuleManager = require("src.semantic.binding.module_manager")
local Lowerer       = require("src.lowering.lowerer")
local LuaCodegen    = require("src.backend.lua.lua_codegen")
local DiagnosticBag = require("src.diagnostics.diagnostic_bag")
local ProjectParser = require("src.project_system.project_parser")
local SourceText    = require("src.source.source_text")

local function compile_file(file_path, out_path)
    local file = io.open(file_path, "r")
    if not file then
        return nil, "Erro: Arquivo não encontrado: " .. file_path
    end
    local content = file:read("*all")
    file:close()

    local diags = DiagnosticBag.new()
    
    print("--- 1. Parsing (" .. file_path .. ") ---")
    local unit, p_diags, source = Parser.parse_string(content, file_path)
    if p_diags:has_errors() then return nil, p_diags:format(source) end

    print("--- 2. Binding ---")
    local mm = ModuleManager.new(".")
    local binder = Binder.new(diags, mm)
    binder:bind(unit, "main")
    if diags:has_errors() then return nil, diags:format(source) end

    print("--- 2.5 Lowering & Otimização ---")
    local lowerer = Lowerer.new(diags)
    unit = lowerer:lower(unit)

    print("--- 3. Codegen ---")
    local codegen = LuaCodegen.new()
    local lua_code = codegen:generate(unit)
    
    -- Criar diretórios pai se necessário
    local dir = out_path:match("(.*[/\\])")
    if dir then 
        -- Comando multiplataforma (mkdir -p no linux/mac, mkdir no win)
        os.execute("mkdir " .. dir:gsub("/", "\\") .. " 2>nul") 
    end

    local out_file = io.open(out_path or "out.lua", "w")
    out_file:write(lua_code)
    out_file:close()
    
    return lua_code, nil, out_path or "out.lua"
end

local function compiler_main(args)
    if not args or #args == 0 then
        print("Zenith Compiler v0.2.0")
        print("Uso: lua ztc.lua <arquivo.zt> | build")
        return
    end

    local command = args[1]
    local lua_code, err, out_path

    if command == "build" then
        print("­ƒôª Iniciando build do projeto Zenith...")
        local config, p_err = ProjectParser.parse("zenith.ztproj")
        if not config then
            print("Erro no arquivo de projeto: " .. tostring(p_err))
            return
        end
        print("Projeto: " .. config.info.name .. " v" .. config.info.version)
        lua_code, err, out_path = compile_file(config.build.main, config.build.out)
    else
        lua_code, err, out_path = compile_file(command, "out.lua")
    end

    if err then
        print(err)
        return
    end

    print("\n--- Build conclu├¡do com sucesso: " .. out_path .. " ---")
    
    -- Rodar opcionalmente
    print("\n--- Executando ---")
    local env = setmetatable({ arg = {} }, { __index = _G })
    local chunk, load_err = loadfile(out_path)
    if not chunk then
        print("Erro ao carregar Lua transpilado: " .. load_err)
    else
        setfenv(chunk, env)
        local ok, exports = pcall(chunk)
        if not ok then
            print("Erro de execu├º├úo (chunk): " .. tostring(exports))
        else
            local main_func = (type(exports) == "table" and exports.main) or env.main
            if main_func then
                local ok2, run_err2 = pcall(main_func)
                if not ok2 then
                    print("Erro de execu├º├úo (main): " .. tostring(run_err2))
                end
            end
        end
    end
end

compiler_main(arg)
