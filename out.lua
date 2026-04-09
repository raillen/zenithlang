-- Transpilado por Zenith v0.2.0
local zt = require("src.backend.lua.runtime.zenith_rt")

local Greetable, Person, p

local Greetable = { _is_trait = true }

local Person = {}
Person.__index = Person

function Person.new(fields, ...)
    if type(fields) ~= "table" or getmetatable(fields) then
        local args = { fields, ... }
        fields = {}
        fields.name = args[1]
    end
    local self = setmetatable({}, Person)
    local _val = fields.name or nil
    self.name = _val
    return self
end


-- Struct Methods
function Person:greet()
    zt.print(zt.add("Ola, eu sou ", self.name))
end

-- Implementation: Greetable for Person

local p = Person.new({ name = "Zenith" })

p:greet()
