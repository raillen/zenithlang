-- Transpilado por Zenith Ascension (Transcendencia v0.10)
local zt = require("src.backend.lua.runtime.zenith_rt")

local Test = {}
Test.__index = Test
function Test.new(fields)
    local self = setmetatable({}, Test)
    return self
end
function Test:test(n, struct_name)
    return n
end

return {
}
