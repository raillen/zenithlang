-- Implementação Nativa de std.math.random
local r = {}

function r.seed(val)
    math.randomseed(val)
end

function r.float(min, max)
    min = min or 0.0
    max = max or 1.0
    return min + (math.random() * (max - min))
end

function r.int(min, max)
    return math.random(min, max)
end

function r.bool()
    return math.random() > 0.5
end

return r
