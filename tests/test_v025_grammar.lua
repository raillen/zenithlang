-- Transpilado por Zenith v0.2.5
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure

local Probe, test

local Probe = {}
Probe.__index = Probe
Probe._metadata = {
    name = "Probe",
    fields = {
        { name = "a", type = "any" },
        { name = "b", type = "any" },
    },
    methods = {
    }
}

function Probe.new(fields)
    local self = setmetatable({}, Probe)
    self.a = fields.a or nil
    self.b = fields.b or nil
    return self
end

function test()
    local p = Probe.new({["a"] = 1, ["b"] = 2})
    local i = 0
    while (i < 10) do
        i = zt.add(i, 1)
    end
end

-- Struct Methods

return {
    Probe = Probe,
}