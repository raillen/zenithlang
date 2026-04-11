-- Transpilado por Zenith v0.2.5
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure

local main

local io = require("src/stdlib/io")

local os = require("src/stdlib/os")

local time = require("src/stdlib/time")

function main()
    io.write_line("🕒 Teste de Compatibilidade (Fase 8) — Zenith")
    local start = time.get_cpu_time()
    local timestamp = time.get_timestamp()
    local date_str = time.format_date(timestamp, "%d/%m/%Y %H:%M:%S")
    io.write_line(zt.add("Timestamp: ", timestamp))
    io.write_line(zt.add("Data: ", date_str))
    local platform = os.get_platform()
    io.write_line(zt.add("Plat: ", platform))
    io.write_line("Pausa de 300ms...")
    os.sleep(300)
    local end_time = time.get_cpu_time()
    io.write_line(zt.add(zt.add("✅ Delta CPU: ", (end_time - start)), "s"))
    local path_opt = os.get_env("PATH")
    -- native lua
    
 local opt = path_opt 
 if opt and opt . _tag == "Present" then 
 print ( "✅ PATH encontrado (Via Native Lua Bridge)" ) 
 else 
 print ( "❌ PATH não encontrado" ) 
 end 
 
end

-- Struct Methods
main()


-- Auto-run main if not in a namespace
if not false then
    local status = main()
    if type(status) == 'table' and status.co then status = zt.drive(status) end
    if type(status) == 'number' then os.exit(status) end
end

return {
    main = main,
}