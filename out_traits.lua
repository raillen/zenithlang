-- Transpilado por Zenith v0.2.5
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty   = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure
local Outcome  = zt.Outcome
local Optional = zt.Optional

local Printable, Greetable, Player, main

-- Namespace: test_traits

-- Trait: Printable
local Printable = {}
Printable.__index = Printable
Printable._is_trait = true
Printable._name = "Printable"
Printable._required = {"to_text"}

-- Trait: Greetable
local Greetable = {}
Greetable.__index = Greetable
Greetable._is_trait = true
Greetable._name = "Greetable"
Greetable._required = {}
function Greetable:greet()
    return "Ola, eu sou printavel!"
end

local Player = {}
Player.__index = Player
Player._metadata = {
    name = "Player",
    fields = {
        { name = "nome", type = "any" },
        { name = "vida", type = "any" },
    },
    methods = {
    }
}

function Player.new(fields)
    local self = setmetatable({}, Player)
    self.nome = fields.nome or nil
    self.vida = fields.vida or nil
    return self
end

-- apply Printable to Player
function Player:to_text()
    return zt.add(zt.add("Player(", self.nome), ")")
end
zt.register_trait(Player, Printable)

-- apply Greetable to Player
zt.register_trait(Player, Greetable)

function main()
    local p = Player.new({["nome"] = "Raillen", ["vida"] = 80})
    print(p:to_text())
    print(p:greet())
end

-- Struct Methods
function Player:greet()
    return "Ola, eu sou printavel!"
end

return {
    Printable = Printable,
    Greetable = Greetable,
    Player = Player,
    main = main,
}