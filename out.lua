-- Transpilado por Zenith v0.2.5
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure

local main

local time = require("src/stdlib/time")

function main()
    print("--- Testando Módulo std.time ---")
    local agora = time.now()
    print(((((("Agora: " .. agora.day) .. "/") .. agora.month) .. "/") .. agora.year))
    print(((((("Hora:  " .. agora.hour) .. ":") .. agora.minute) .. ":") .. agora.second))
    print("\
Aguardando 2 segundos (Síncrono)...")
    local d2 = time.seconds(2)
    local sw = time.stopwatch()
    sw:start()
    time.sleep(d2)
    local decorrido = sw:stop()
    print((("Tempo real decorrido: " .. decorrido.seconds) .. "s"))
    local d_complexa = time.duration(1, 30)
    print(("\
1m 30s em segundos: " .. d_complexa.seconds))
    print("\
--- Fim dos testes de TIME ---")
    return 0
end

-- Struct Methods

-- Auto-run main if not in a namespace
if not false then
    local status = main()
    if type(status) == 'number' then os.exit(status) end
end

return {
    main = main,
}