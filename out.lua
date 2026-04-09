-- Transpilado por Zenith v0.2.0
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure

local test_check, test_named_defaults, test_unions, Item, test_where, test_interpolation, test_panic, main

function test_check(n)
    if type(n) == "table" and not getmetatable(n) then
        local _is_named = false
        if n.n ~= nil then _is_named = true end
        if _is_named then
            local _args = n
            n = _args.n
        end
    end
    if not ((n > 0)) then
        zt.print("n deve ser positivo")
        return
    end
    zt.print("n ok")
end

function test_named_defaults(x, y, z)
    if type(x) == "table" and not getmetatable(x) then
        local _is_named = false
        if x.x ~= nil then _is_named = true end
        if x.y ~= nil then _is_named = true end
        if x.z ~= nil then _is_named = true end
        if _is_named then
            local _args = x
            x = _args.x
            y = _args.y
            z = _args.z
        end
    end
    if y == nil then y = 10 end
    if z == nil then z = 20 end
    zt.print(zt.add(zt.add(zt.add(zt.add(zt.add("x=", x), ", y="), y), ", z="), z))
end

function test_unions(val)
    if type(val) == "table" and not getmetatable(val) then
        local _is_named = false
        if val.val ~= nil then _is_named = true end
        if _is_named then
            local _args = val
            val = _args.val
        end
    end
    if zt.is(val, "int") then
        zt.print(zt.add("int: ", val))
    elseif zt.is(val, "text") then
        zt.print(zt.add("text: ", val))
    else
        zt.print(zt.add("bool: ", val))
    end
end

local Item = {}
Item.__index = Item

function Item.new(fields, ...)
    if type(fields) ~= "table" or getmetatable(fields) then
        local args = { fields, ... }
        fields = {}
        fields.nome = args[1]
        fields.quantidade = args[2]
    end
    local self = setmetatable({}, Item)
    local _val = fields.nome or nil
    self.nome = _val
    local _val = fields.quantidade or nil
    zt.check((_val >= 0), "violacao de contrato no campo 'quantidade' da struct 'Item'")
    self.quantidade = _val
    return self
end


function test_where()
    zt.print("--- Teste Where (Contratos) ---")
    local i1 = Item.new({ nome = "Espada", quantidade = 10 })
    zt.print(zt.add("Item 1 ok: ", i1.nome))
    local _ok, e = pcall(function()
        local i2 = Item.new({ nome = "Erro", quantidade = -5 })
    end)
    if not _ok then
        zt.print(zt.add("Sucesso: Capturou violação de contrato: ", e.message))
    end
end

function test_interpolation()
    zt.print("--- Teste Interpolação Avançada ---")
    local a = 10
    local b = 20
    local msg = zt.add(zt.add(zt.add(zt.add(zt.add(zt.add("A soma de ", a), " e "), b), " é "), (a + b)), "")
    zt.print(msg)
    if not ((msg == "A soma de 10 e 20 é 30")) then
        zt.error("Interpolação falhou!")
    end
end

function test_panic()
    zt.print("--- Teste Panic (Prelude) ---")
    local _ok, e = pcall(function()
        zt.error("Erro fatal de teste")
    end)
    if not _ok then
        zt.print(zt.add("Capturou panic: ", e.message))
    end
end

function main()
    test_panic()
    test_where()
    test_interpolation()
    zt.print("--- Teste Check ---")
    test_check(1)
    test_check((-1))
    zt.print("--- Teste Named/Defaults ---")
    test_named_defaults(1, 10, 20)
    test_named_defaults(1, 2, 20)
    test_named_defaults(5, 10, 100)
    zt.print("--- Teste Unions ---")
    test_unions(42)
    test_unions("hello")
    test_unions(true)
    zt.print("Refinement tests passed!")
    return 0
end

-- Struct Methods
-- Entry Point
if main then main() end
-- Exports
return {
    test_check = test_check,
    test_named_defaults = test_named_defaults,
    test_unions = test_unions,
    test_where = test_where,
    test_interpolation = test_interpolation,
    test_panic = test_panic,
    main = main,
}