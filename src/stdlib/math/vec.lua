-- Implementação Nativa de std.math.vec
local v = {}

local Vec2 = {}
Vec2.__index = Vec2

function Vec2.new(fields)
    local self = setmetatable({
        x = fields.x or 0.0,
        y = fields.y or 0.0
    }, Vec2)
    return self
end

function Vec2:length()
    return math.sqrt(self.x^2 + self.y^2)
end

function Vec2:normalize()
    local len = self:length()
    if len == 0 then return Vec2.new({x=0, y=0}) end
    return Vec2.new({x = self.x / len, y = self.y / len})
end

function Vec2:dot(other)
    return (self.x * other.x) + (self.y * other.y)
end

function v.vec2(x, y)
    return Vec2.new({x = x, y = y})
end

-- Exporta a classe para o Binder
v.Vec2 = Vec2

return v
