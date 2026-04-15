-- Transpilado por Zenith v0.2.5
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure

local print, Color, Pos, Sprite, main

function print(msg)
end

local Color = {
    Red = "Red",
    Green = "Green",
    Blue = "Blue",
}

local Pos = {}
Pos.__index = Pos
Pos._metadata = {
    name = "Pos",
    fields = {
    },
    methods = {
    }
}

function Pos.new(fields)
    local self = setmetatable({}, Pos)
    self.x = fields.x or nil
    self.y = fields.y or nil
    return self
end

local Sprite = {}
Sprite.__index = Sprite
Sprite._metadata = {
    name = "Sprite",
    fields = {
    },
    methods = {
    }
}

function Sprite.new(fields)
    local self = setmetatable({}, Sprite)
    self.pos = fields.pos or nil
    self.color = fields.color or nil
    return self
end

function main()
    local c = Color.Red
    -- [Aviso: Match não desaçucarado]
    -- [Aviso: Match não desaçucarado]
    local s = Sprite.new({["pos"] = Pos.new({["x"] = 10, ["y"] = 20}), ["color"] = Color.Blue})
    -- [Aviso: Match não desaçucarado]
    local lista = {1, 2, 3}
    -- [Aviso: Match não desaçucarado]
    print("Pattern Matching Hardening Test Passed!")
end

-- Struct Methods

-- Auto-run main if not in a namespace
if not false then
    local status = main()
    if type(status) == 'table' and status.co then status = zt.drive(status) end
    if type(status) == 'number' then os.exit(status) end
end

return {
    main = main,
    main = main,
}