-- Transpilado por Zenith v0.2.5
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty   = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure
local Outcome  = zt.Outcome
local Optional = zt.Optional

local id, Box, main

local io = require("src/stdlib/io")

function id(x)
    return x
end

local Box = {}
Box.__index = Box
Box._metadata = {
    name = "Box",
    fields = {
    },
    methods = {
    }
}

function Box.new(fields)
    local self = setmetatable({}, Box)
    self.value = fields.value or nil
    return self
end

function main()
    local n1 = id(10)
    io.println(("n1 (explícito): " .. n1:to_text()))
    local n2 = id(20)
    io.println(("n2 (inferido): " .. n2:to_text()))
    local s1 = id("olá generics")
    io.println(("s1: " .. s1))
    local b1 = Box.new({["value"] = 100})
    io.println(("b1.value: " .. b1.value:to_text()))
    local b2 = Box.new({["value"] = "texto na box"})
    io.println(("b2.value: " .. b2.value))
    io.println("Todos os testes de generics (sucesso) passaram!")
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