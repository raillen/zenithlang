-- Transpilado por Zenith v1.0-alpha
local zt = require("src.backend.lua.runtime.zenith_rt")

local Point = {}
Point.__index = Point

function Point.new(fields)
    local self = setmetatable({}, Point)
    self.x = fields.x or nil
    self.y = fields.y or nil
    return self
end

function Point:sum(self)
    return (-- [[Expr:SELF_EXPR]].x + -- [[Expr:SELF_EXPR]].y)
end

