-- Transpilado por Zenith v0.2.5
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty   = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure
local Outcome  = zt.Outcome
local Optional = zt.Optional

local Point, main

local io = require("src/stdlib/io")

local Point = {}
Point.__index = Point
Point._metadata = {
    name = "Point",
    fields = {
    },
    methods = {
    }
}

function Point.new(fields)
    local self = setmetatable({}, Point)
    self.x = fields.x or nil
    self.y = fields.y or nil
    return self
end

function main()
    local p = Point.new({["x"] = 10, ["y"] = 20})
    local __match_val_1 = p
    if type(__match_val_1) == 'table' and __match_val_1.x == 10 then
        local y_val = __match_val_1.y
        io.println("Point com x=10 encontrado!")
    elseif true then
        io.println("Outro point")
    end
end

-- Struct Methods
main()


-- Auto-run main
if true then
    local status = main(arg)
    if type(status) == 'table' and status.co then status = zt.drive(status) end
    if type(status) == 'number' then os.exit(status) end
end

return {
    main = main,
}