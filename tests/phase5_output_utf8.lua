-- Transpilado por Zenith Ascension (Native v0.4)
local zt = require("src.backend.lua.runtime.zenith_rt")

function test_gate()
    local list = {1, 2, 3}
    local map = {["status"] = 200}
    local ok = ((not (1 == 2)) and (list[0] == 1))
    if (ok) then
        return map["status"]
    end
    return 0
end
local final_status = test_gate()
