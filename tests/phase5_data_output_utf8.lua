-- Transpilado por Zenith Ascension (Native v0.4)
local zt = require("src.backend.lua.runtime.zenith_rt")

local Status = {
    Active = "Active",
    Inactive = "Inactive",
}
local Point = {}
Point.__index = Point
function Point.new(fields)
    local self = setmetatable({}, Point)
    self.x = fields.x or 0
    self.y = fields.y or 0
    return self
end
function Point:move(dx, dy)
    self.x = (self.x + dx)
    self.y = (self.y + dy)
end
function test_data()
    local p = Point.new({["x"] = 10, ["y"] = 20})
    p.move(5, 5)
    local s = Status.Active
    if (((s == "Active") and (p.x == 15))) then
        return p.y
    end
    return 0
end
local res = test_data()
