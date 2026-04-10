-- Transpilado por Zenith v0.2.0
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure

local main

local json = require("src/stdlib/json")

function main()
    print("--- Testando Módulo std.json ---")
    local raw_json = "{ \"nome\": \"Zenith\", \"versao\": 0.2 }"
    print("Testando parse genérico...")
    local res_parse = json.parse(raw_json)
    local _m = res_parse
    if ((_m._tag == "Success") and true) then
        local obj = _m._1
        local mapa = obj
        print(("Nome: " .. mapa.nome))
    elseif ((_m._tag == "Failure") and true) then
        local e = _m._1
        print("Erro no parse")
    end
    print("\
Testando stringify:")
    local data = {["id"] = 1, ["autor"] = "Rafael"}
    local s = json.stringify(data, 0)
    print(s)
    print("\
Testando beautify:")
    local res_b = json.beautify(s, 2)
    local _m = res_b
    if ((_m._tag == "Success") and true) then
        local b = _m._1
        print(b)
    elseif ((_m._tag == "Failure") and true) then
        local e = _m._1
        print("Erro no beautify")
    end
    print("\
--- Fim dos testes ---")
    return 0
end

-- Struct Methods

-- Auto-run main if not in a namespace
if not false then
    local status = main()
    if type(status) == 'number' then os.exit(status) end
end

return {
    main = main,
}