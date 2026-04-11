-- Transpilado por Zenith Ascension (Native v0.4)
local zt = require("src.backend.lua.runtime.zenith_rt")

function test_control()
    local sum = 0
    for _ = 1, 5 do
        sum = (sum + 1)
    end
    local list = {10, 20, 30}
    for _, x in zt.iter(list) do
        sum = (sum + x)
    end
    local val = 20
    local res = 0
    do
        local _m = val
        if (_m == 10) then
            res = 1
        elseif (_m == 20) or (_m == 30) then
            res = 2
        end
    end
    return (sum + res)
end
local total = test_control()
