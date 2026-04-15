-- Transpilado por Zenith v0.2.5
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure

local print, check, Person, falar, gritar, main

function print(msg)
end

function check(cond)
end

local Person = {}
Person.__index = Person
Person._metadata = {
    name = "Person",
    fields = {
    },
    methods = {
    }
}

function Person.new(fields)
    local self = setmetatable({}, Person)
    self.nome = fields.nome or nil
    return self
end

function falar(p)
    print(zt.add("GLOBAL: ", p.nome))
end

function gritar(p)
    print(zt.add(p.nome, " GRITA!!!"))
end

function main()
    local p = Person.new({["nome"] = "Zenith"})
    p:falar()
    gritar(p)
    local lista = {"a", "b", "c"}
    local tam = zt.len(lista)
    print(zt.add("Tamanho da lista: ", tam))
    local s = "zenith"
    local tam_s = zt.len(s)
    print(zt.add("Tamanho da string: ", tam_s))
    local partes = zt.split("a,b,c")
    print(zt.add("Partes: ", zt.len(partes)))
    local config = {["debug"] = "true"}
    local chaves = zt.keys(config)
    print(zt.add("Chaves: ", zt.len(chaves)))
    print("UFCS Hardening Test Passed!")
end

-- Struct Methods
function Person:falar()
    print(zt.add(self.nome, " diz oi"))
end

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