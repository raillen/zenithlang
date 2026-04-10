-- Implementação Nativa da std.math para Zenith
local m = {}

m.PI = math.pi
m.E  = 2.718281828459045
m.TAU = math.pi * 2

m.abs = math.abs
m.sqrt = math.sqrt
m.pow = math.pow or function(b, e) return b ^ e end
m.floor = math.floor
m.ceil = math.ceil

function m.round(x)
    return math.floor(x + 0.5)
end

m.sin = math.sin
m.cos = math.cos
m.tan = math.tan
m.asin = math.asin
m.acos = math.acos
m.atan2 = math.atan2 or math.atan

function m.rad(deg) return deg * (math.pi / 180.0) end
function m.deg(rad) return rad * (180.0 / math.pi) end

function m.clamp(val, min, max)
    if val < min then return min end
    if val > max then return max end
    return val
end

function m.lerp(a, b, t)
    return a + (b - a) * t
end

function m.sign(x)
    if x > 0 then return 1
    elseif x < 0 then return -1
    else return 0 end
end

return m
