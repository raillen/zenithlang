-- Transpilado por Zenith v0.2.5
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure

local test_list_safety, test_text_safety, main

function test_list_safety()
    local l = {10, 20, 30}
    local i = 1
    print(zt.index_seq(l, i))
    local zero = 0
    print(zt.index_seq(l, zero))
end

function test_text_safety()
    local t = "Zenith"
    local i = 1
    print(zt.index_text(t, i))
    local s = zt.slice(t, 0, 10)
end

function main()
    print("Iniciando testes de segurança...")
    test_list_safety()
    test_text_safety()
    print("Fase 11 (Runtime) validada com sucessos parciais (limites seguros).")
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