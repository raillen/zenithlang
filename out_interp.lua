-- Transpilado por Zenith v0.2.5
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty   = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure
local Outcome  = zt.Outcome
local Optional = zt.Optional

local main

-- Namespace: test_interp

function main()
    local nome = "Raillen"
    local hp = 80
    local msg = zt.add(zt.add(zt.add(zt.add("Jogador: ", (nome)), ", HP: "), (hp)), "")
    print(msg)
    local x = 10
    local y = 5
    print(zt.add(zt.add(zt.add(zt.add("Soma: ", (zt.add(x, y))), ", Produto: "), ((x * y))), ""))
    print(zt.add(zt.add("Resultado: ", ((hp * 2))), " pontos"))
end

-- Struct Methods

return {
    main = main,
}