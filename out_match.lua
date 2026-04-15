-- Transpilado por Zenith v0.2.5
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty   = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure
local Outcome  = zt.Outcome
local Optional = zt.Optional

local main

local io = require("src/stdlib/io")

function main()
    local x = 42
    local __match_val_1 = x
    if __match_val_1 == 10 then
        io.println("Era 10")
    elseif __match_val_1 == 42 then
        io.println("Era 42! Literal match funciona.")
    elseif true then
        io.println("Fallback hit!")
    end
    local resp = Success("Dados carregados com sucesso")
    local __match_val_2 = resp
    if (type(__match_val_2) == 'table' and __match_val_2._tag == "Success") then
        local val = (__match_val_2.value or __match_val_2._1)
        io.println(zt.add("Sucesso: ", val))
    elseif (type(__match_val_2) == 'table' and __match_val_2._tag == "Failure") then
        local err = (__match_val_2.error or __match_val_2.err or __match_val_2._1)
        io.println(zt.add("Erro: ", err))
    end
    local age = 15
    local __match_val_3 = age
    if (function() local n = __match_val_3;  return ((n < 18)) end)() then
        local n = __match_val_3
        io.println("Menor de idade!")
    elseif (function() local n = __match_val_3;  return ((n >= 18)) end)() then
        local n = __match_val_3
        io.println("Maior de idade!")
    end
end

-- Struct Methods
main()


-- Auto-run main
if true then
    local status = main(arg)
    if type(status) == 'table' and status.co then status = zt.drive(status) end
    if type(status) == 'number' then os.exit(status) end
end

return {
    main = main,
}