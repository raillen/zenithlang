-- Transpilado por Zenith v0.2.5
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure

local c_reset, c_bold, c_green, c_red, c_cyan, c_gray, c_yellow, main

local fs = require("src/stdlib/fs")

local io = require("src/stdlib/io")

local os = require("src/stdlib/os")

local text = require("src/stdlib/text")

local time = require("src/stdlib/time")

function c_reset()
    return "[0m"
end

function c_bold()
    return "[1m"
end

function c_green()
    return "[32m"
end

function c_red()
    return "[31m"
end

function c_cyan()
    return "[36m"
end

function c_gray()
    return "[90m"
end

function c_yellow()
    return "[33m"
end

function main()
    local filter = ""
    local export_report = false
    -- native lua
    
 for i , v in ipairs ( arg or { } ) do 
 if v == "--grep" and arg [ i + 1 ] then 
 filter = arg [ i + 1 ] 
 elseif v == "--report" then 
 export_report = true 
 end 
 end 
 
    io.write_line(zt.add(zt.add(zt.add(c_bold(), c_cyan()), "[TEST] ZENITH TEST ORCHESTRATOR (ZTest)"), c_reset()))
    io.write_line(zt.add(zt.add(c_gray(), "===================================="), c_reset()))
    local tests_dir = "tests"
    if not fs.exists(tests_dir) then
        io.write_line(zt.add(zt.add(c_red(), "Erro: Diretorio /tests nao encontrado."), c_reset()))
        os.exit_process(1)
    end
    local files = {}
    -- native lua
    
 local folders = { "tests/core" , "tests/stdlib" , "tests/integration" , "tests/ascension" } 
 local is_windows = package . config : sub ( 1 , 1 ) == "\\" 
 
 for _ , folder in ipairs ( folders ) do 
 local cmd = is_windows and ( "dir /b " .. folder : gsub ( "/" , "\\" ) .. " 2>nul" ) or ( "ls " .. folder .. " 2>/dev/null" ) 
 local handle = _G . io . popen ( cmd ) 
 if handle then 
 for line in handle : lines ( ) do 
 
 if line : find ( "_test%.zt$" ) or line : find ( "^test_.*%.zt$" ) then 
 local path = folder .. "/" .. line 
 if filter == "" or path : find ( filter ) then 
 table . insert ( files , path ) 
 end 
 end 
 end 
 handle : close ( ) 
 end 
 end 
 
    if (#(files) == 0) then
        io.write_line(zt.add(zt.add(c_yellow(), "Nenhum arquivo de teste correspondente encontrado."), c_reset()))
        return
    end
    local total_passed = 0
    local total_failed = 0
    local start_total = time.get_cpu_time()
    local platform = os.get_platform()
    local i = 1
    while (i <= #(files)) do
        local file = files[i]
        local display_name = text.truncate(file, 40)
        io.write(zt.add(zt.add(zt.add(zt.add(zt.add(zt.add(zt.add(zt.add(c_bold(), " ["), i), "/"), #(files)), "] "), c_reset()), display_name), " "))
        local start_suite = time.get_cpu_time()
        local compile_cmd = zt.add(zt.add("lua ztc.lua compile ", file), " out.lua > nul 2>&1")
        if (platform ~= "windows") then
            compile_cmd = zt.add(zt.add("lua ztc.lua compile ", file), " out.lua > /dev/null 2>&1")
        end
        local compile_res = os.run_command(compile_cmd)
        if compile_res:is_failure() then
            io.write_line(zt.add(zt.add(c_red(), "FALHA NA COMPILACAO"), c_reset()))
            total_failed = total_failed + 1
        else
            local env_flags = "set ZTEST_SILENT=1 && "
            if (platform ~= "windows") then
                env_flags = "ZTEST_SILENT=1 "
            end
            local run_res = os.run_command(zt.add(env_flags, "lua out.lua > nul 2>&1"))
            local end_suite = time.get_cpu_time()
            local duration = (end_suite - start_suite)
            local dur_text = ""
            -- native lua
            
 dur_text = string . format ( "%.3fs" , duration ) 
 
            if run_res:is_success() then
                io.write_line(zt.add(zt.add(zt.add(zt.add(zt.add(zt.add(zt.add(c_green(), "PASSOU"), c_reset()), c_gray()), " [ "), dur_text), " ]"), c_reset()))
                total_passed = total_passed + 1
            else
                io.write_line(zt.add(zt.add(zt.add(zt.add(zt.add(zt.add(zt.add(c_red(), "FALHOU"), c_reset()), c_gray()), " [ "), dur_text), " ]"), c_reset()))
                -- native lua
                
 _G . os . execute ( "lua out.lua" ) 
 
                total_failed = total_failed + 1
            end
        end
        i = i + 1
    end
    local end_total = time.get_cpu_time()
    local total_duration = (end_total - start_total)
    local total_dur_text = ""
    -- native lua
    
 total_dur_text = string . format ( "%.2fs" , total_duration ) 
 
    io.write_line(zt.add(zt.add(c_gray(), "===================================="), c_reset()))
    if (total_failed == 0) then
        io.write_line(zt.add(zt.add(zt.add(zt.add(zt.add(zt.add(zt.add(c_green(), c_bold()), " SUCESSO: "), total_passed), " suites passaram em "), total_dur_text), "!"), c_reset()))
    else
        io.write_line(zt.add(zt.add(zt.add(zt.add(zt.add(zt.add(zt.add(c_red(), c_bold()), " FALHA: "), total_failed), " suite(s) falharam. (Total: "), total_dur_text), ")"), c_reset()))
    end
    if export_report then
        local report = {["timestamp"] = time.get_timestamp(), ["total"] = #(files), ["passed"] = total_passed, ["failed"] = total_failed, ["duration"] = total_duration}
        -- native lua
        
 local json = require ( "src/stdlib/json" ) 
 local fs = require ( "src/stdlib/fs" ) 
 fs . write_text_file ( "docs/web/public/test-health.json" , json . stringify ( report , 4 ) ) 
 
        io.write_line(zt.add(zt.add(c_gray(), " Relatorio exportado para docs/web/public/test-health.json"), c_reset()))
    end
    if (total_failed > 0) then
        os.exit_process(1)
    end
end

-- Struct Methods

-- Auto-run main if not in a namespace
if not false then
    local status = main()
    if type(status) == 'table' and status.co then status = zt.drive(status) end
    if type(status) == 'number' then os.exit(status) end
end

return {
    main = main,
}