-- Transpilado por Zenith v0.2.5
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure

local test_gate, final_status

function test_gate()
    local list = {1, 2, 3}
    local status_by_name = {["status"] = 200}
    local ok = (not ((1 == 2)) and ((zt.index_seq(list, 1) == 1)))
    if ok then
        return status_by_name["status"]
    end
    return 0
end

-- Struct Methods
final_status = test_gate()

print(final_status)


return {
}