-- Zenith Compiler CLI (ztc.lua)
local Parser        = require("src.syntax.parser.parser")
local Binder        = require("src.semantic.binding.binder")
local ModuleManager = require("src.semantic.binding.module_manager")
local Lowerer       = require("src.lowering.lowerer")
local LuaCodegen    = require("src.backend.lua.lua_codegen")
local DiagnosticBag = require("src.diagnostics.diagnostic_bag")
local ProjectParser = require("src.project_system.project_parser")
local SourceText    = require("src.source.source_text")

local function compile_file(file_path, out_path, target_platform)
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
    local binder = Binder.new(diags, mm, target_platform)
    binder:bind(unit, "main")
    if diags:has_errors() then return nil, diags:format(source) end

    print("--- 2.5 Lowering and Otimizacao (" .. (target_platform or "host") .. ") ---")
    local lowerer = Lowerer.new(diags, target_platform)
    unit = lowerer:lower(unit)

    print("--- 3. Codegen ---")
    local codegen = LuaCodegen.new()
    local lua_code = codegen:generate(unit)
    
    -- Criar diretorios pai se necessario
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
        print("Zenith Compiler v0.3.0 (Ascension)")
        print("Uso:")
        print("  zt <arquivo.zt> [--run] [--target <os>]")
        print("  zt build [--standalone | --bundle] [--target <os>]")
        print("  zt test [--grep <pattern>]")
        return
    end

    local command = args[1]
    local target = "windows" -- Default
    local mode = "normal"
    local run_after = false

    -- Parse de flags
    for i, v in ipairs(args) do
        if v == "--target" and args[i+1] then target = args[i+1]
        elseif v == "--standalone" then mode = "standalone"
        elseif v == "--bundle" then mode = "bundle"
        elseif v == "--run" then run_after = true
        end
    end

    local lua_code, err, out_path

    if command == "test" then
        print("[TEST] Iniciando Zenith Test Orchestrator...")
        lua_code, err, out_path = compile_file("src/cli/ztest.zt", "ztest.lua", target)
        if not err then
            local cmd_args = ""
            for i=2, #args do cmd_args = cmd_args .. " " .. args[i] end
            os.execute("lua ztest.lua" .. cmd_args)
            return
        end
    elseif command == "zpm" then
        print("[ZPM] Iniciando Zenith Package Manager...")
        lua_code, err, out_path = compile_file("src/cli/zpm.zt", "zpm_engine.lua", target)
        if not err then
            local cmd_args = ""
            for i=2, #args do cmd_args = cmd_args .. " " .. args[i] end
            os.execute("lua zpm_engine.lua" .. cmd_args)
            return
        end
    elseif command == "build" then
        print("[BUILD] Iniciando build soberano (" .. mode .. ") para " .. target .. "...")
        local config, p_err = ProjectParser.parse("zenith.ztproj")
        if not config then
            print("Erro no arquivo de projeto: " .. tostring(p_err))
            return
        end
        
        local final_out = config.build.out
        if mode == "bundle" then
            print("Preparando bundle em ./dist...")
            os.execute("mkdir dist 2>nul")
            os.execute("mkdir dist\\native 2>nul")
            final_out = "dist/" .. (config.build.out or "app.lua")
        end

        lua_code, err, out_path = compile_file(config.build.main, final_out, target)
        
        if not err and mode == "standalone" then
            print("Gerando standalone para " .. target .. "...")
            -- Simulação de bundling da VM
            print("Aviso: Standalone funcional requer zt-seed tool. Gerando wrapper Lua...")
        end
    elseif command == "compile" then
        local filename = args[2]
        if not filename then
            print("Erro: compile exige um arquivo.zt")
            return
        end
        lua_code, err, out_path = compile_file(filename, args[3] or "out.lua", target)
        if not err then return end -- Finaliza sem executar
    else
        if not command:match("%.zt$") then
            print("Erro: Comando ou arquivo desconhecido: " .. command)
            return
        end
        lua_code, err, out_path = compile_file(command, "out.lua", target)
    end

    if err then
        print(err)
        return
    end

    print("\n--- Build concluido com sucesso: " .. (out_path or "out.lua") .. " ---")
    
    -- Rodar opcionalmente
    if run_after or (command ~= "build" and command ~= "test" and command ~= "compile") then
        print("\n--- Executando ---")
        local script_args = {}
        for i=2, #args do table.insert(script_args, args[i]) end
        local env = setmetatable({ arg = script_args }, { __index = _G })
        local chunk, load_err = loadfile(out_path)
        if not chunk then
            print("Erro ao carregar Lua transpilado: " .. load_err)
        else
            setfenv(chunk, env)
            local ok, exports = pcall(chunk)
            if not ok then
                print("Erro de execucao (chunk): " .. tostring(exports))
            else
                local main_func = (type(exports) == "table" and exports.main) or env.main
                if main_func then
                    local ok2, run_err2 = pcall(main_func)
                    if not ok2 then
                        print("Erro de execucao (main): " .. tostring(run_err2))
                    end
                end
            end
        end
    end
end

compiler_main(arg)
