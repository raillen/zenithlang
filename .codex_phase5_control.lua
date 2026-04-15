-- Transpilado por Zenith v0.2.5
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure

local test_control, total

function test_control()
    local sum = 0
    for _ = 1, 5 do
        sum = (zt.add(sum, 1))
    end
    local list = {10, 20, 30}
    for x in zt.iter(list) do
        sum = (zt.add(sum, x))
    end
    local val = 20
    local res = 0
    -- [Aviso: Match não desaçucarado]
    return (zt.add(sum, res))
end

-- Struct Methods
total = test_control()


return {
}