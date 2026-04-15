-- Transpilado por Zenith v0.2.5
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty   = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure
local Outcome  = zt.Outcome
local Optional = zt.Optional

local say_hello, main

-- Namespace: test_ufcs

function say_hello(nome)
    print(zt.add("hello ", nome))
end

function main()
    local person = "Carlos"
    say_hello(person)
    print(zt.add("O tamanho do nome é: ", tostring(zt.len(person))))
end

-- Struct Methods

return {
    say_hello = say_hello,
    main = main,
}