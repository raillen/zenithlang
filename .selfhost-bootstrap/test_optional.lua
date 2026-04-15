-- Transpilado por Zenith v0.2.5
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure

local main

local core = require("src/stdlib/core")

function main()
    local present = Present("Zenith")
    if not zt.is_present(present) then
        panic("Present deve ser marcado como presente")
    end
    if zt.is_empty(present) then
        panic("Present nao pode ser vazio")
    end
    if (zt.unwrap(present) ~= "Zenith") then
        panic("unwrap deve retornar o valor interno")
    end
    if (zt.unwrap_or(present, "fallback") ~= "Zenith") then
        panic("unwrap_or nao deve consumir fallback em Present")
    end
    local empty = Empty
    if zt.is_present(empty) then
        panic("Empty nao pode ser marcado como presente")
    end
    if not zt.is_empty(empty) then
        panic("Empty deve ser vazio")
    end
    if (zt.unwrap_or(empty, "fallback") ~= "fallback") then
        panic("unwrap_or deve usar fallback em Empty")
    end
    local ok = Success("ok")
    if not zt.is_success(ok) then
        panic("Success deve ser marcado como sucesso")
    end
    if zt.is_failure(ok) then
        panic("Success nao pode ser falha")
    end
    if (zt.unwrap_or(ok, "fallback") ~= "ok") then
        panic("Outcome.unwrap_or nao deve consumir fallback em Success")
    end
    local fail = Failure("boom")
    if zt.is_success(fail) then
        panic("Failure nao pode ser sucesso")
    end
    if not zt.is_failure(fail) then
        panic("Failure deve ser marcado como falha")
    end
    if (zt.unwrap_or(fail, "fallback") ~= "fallback") then
        panic("Outcome.unwrap_or deve usar fallback em Failure")
    end
    print("Optional semantics OK")
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
}