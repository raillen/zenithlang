-- Transpilado por Zenith v0.2.5
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty   = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure
local Outcome  = zt.Outcome
local Optional = zt.Optional

local divide, calcular, main

-- Namespace: test_try

function divide(a, b)
    if (b == 0) then
        return Failure("divisao por zero")
    end
    return Success((a / b))
end

function calcular(x, y)
    local __zt_try_1 = divide(x, y)
    if type(__zt_try_1) == "table" and (__zt_try_1._tag == "Failure" or __zt_try_1._tag == "Empty") then return __zt_try_1 end
    local resultado = (__zt_try_1.value or __zt_try_1._1)
    local dobro = (resultado * 2)
    return Success(dobro)
end

function main()
    local ok = calcular(10, 2)
    if zt.is(ok, Outcome) then
        print(zt.add(zt.add("Sucesso: ", (ok)), ""))
    end
    local err = calcular(5, 0)
    if zt.is(err, Outcome) then
        print(zt.add(zt.add("Erro: ", (err)), ""))
    end
    local val = zt.bang(Success(42))
    print(zt.add(zt.add("Valor desempacotado: ", (val)), ""))
end

-- Struct Methods

return {
    divide = divide,
    calcular = calcular,
    main = main,
}