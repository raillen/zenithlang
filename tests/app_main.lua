-- Transpilado por Zenith Ascension (Native v0.4)
local zt = require("src.backend.lua.runtime.zenith_rt")
local math = require("tests.math_utils")

function main()
    local x = 10
    local y = 20
    local r = math.add(x, y)
    print(("Resultado modular: " + r))
end

return {
}
