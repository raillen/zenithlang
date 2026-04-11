-- Transpilado por Zenith Ascension (Native v0.4)
local zt = require("src.backend.lua.runtime.zenith_rt")

function soma(a, b)
    return zt.add(a, b)
end
function soma(x)
    return x
end
function main()
    local x = 10
    local y = zt.add(z, x)
    if ((x > 0)) then
        io.print("positivo")
    end
end

-- Auto-run main
local _s = main()
if type(_s) == 'table' and _s.co then zt.drive(_s) end

return {
}
