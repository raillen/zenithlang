-- Transpilado por Zenith v0.2.5
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty   = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure
local Outcome  = zt.Outcome
local Optional = zt.Optional

local main

function main()
    local l = zt.list({})
    l:push(1)
    if l:is_empty() then
        print("vazia")
    end
    print(l:len())
end

-- Struct Methods

-- Auto-run main
if true then
    local status = main(arg)
    if type(status) == 'table' and status.co then status = zt.drive(status) end
    if type(status) == 'number' then os.exit(status) end
end

return {
    main = main,
}