-- Transpilado por Zenith v0.2.5
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure

local User, get_user, main

local User = {}
User.__index = User
User._metadata = {
    name = "User",
    fields = {
    },
    methods = {
    }
}

function User.new(fields)
    local self = setmetatable({}, User)
    self.name = fields.name or nil
    self.age = fields.age or nil
    return self
end

function get_user(id)
    if (id == 1) then
        return Optional:Present(User.new({["name"] = "Raillen", ["age"] = 25}))
    end
    return Optional.Empty
end

function main(args)
    local __afters = {}
    local ready
    nil = true
    function save(_p1)
        ("Salvando perigosamente...")
    end
    if -- [[Expr:TRY_EXPR]] then
        save()
    end
    local name = "Zenith"
    local version = 0.4
    print(zt.add(zt.add(zt.add(zt.add("Bem-vindo ao ", (name)), " v"), (version)), "!"))
    print(zt.add(zt.add("Soma: ", (zt.add(1, 2))), ""))
    local u = get_user(1)
    local name_safe = zt.unwrap_or(zt.safe_get(u, "name"), "Anonimo")
    print(zt.add(zt.add("User: ", (name_safe)), ""))
    local u2 = get_user(2)
    local name_none = zt.unwrap_or(zt.safe_get(u2, "name"), "Desconhecido")
    print(zt.add(zt.add("User 2: ", (name_none)), ""))
    print("Inicio da tarefa")
    table.insert(__afters, function()
        print("Fim da tarefa (via after)")
    end)
    table.insert(__afters, function()
        print("Limpando recursos...")
        print("Fechando conexoes...")
    end)
    function fail()
        for i=#__afters,1,-1 do __afters[i]() end
        return Optional.Empty
    end
    print("Antes do check")
    print("Teste concluido com sucesso!")
    for i=#__afters,1,-1 do __afters[i]() end
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
    main = main,
}