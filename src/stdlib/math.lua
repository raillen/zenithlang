-- Standard Library Zenith — Math (Native)
local m = {}

m.PI = math.pi
m.abs = math.abs
m.floor = math.floor
m.ceil = math.ceil
m.sin = math.sin
m.cos = math.cos
m.tan = math.tan
m.sqrt = math.sqrt
m.random = math.random

function m.rad(deg)
    return deg * (math.pi / 180.0)
end

function m.deg(rad)
    return rad * (180.0 / math.pi)
end

return m
