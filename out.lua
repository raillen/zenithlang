-- Transpilado por Zenith v0.2.0
local zt = require("src.backend.lua.runtime.zenith_rt")

local AppStatus, test_enum, main

local AppStatus = {
    Success = function(p1)
        return { _tag = "Success",
            value = p1,
            _1 = p1,
        }
    end,
    Error = function(p1)
        return { _tag = "Error",
            msg = p1,
            _1 = p1,
        }
    end,
    Loading = { _tag = "Loading" },
}

function test_enum(status)
    if type(status) == "table" and not getmetatable(status) then
        local _is_named = false
        if status.status ~= nil then _is_named = true end
        if _is_named then
            local _args = status
            status = _args.status
        end
    end
    local _m = status
    if ((_m._tag == "Success") and true) then
        local v = _m._1
        zt.print(zt.add("Sucesso: ", v))
    elseif ((_m._tag == "Error") and true) then
        local m = _m._1
        zt.print(zt.add("Erro: ", m))
    elseif (_m._tag == "Loading") then
        zt.print("Carregando...")
    end
end

function main()
    local s1 = AppStatus.Success(100)
    local s2 = AppStatus.Error("Falha critica")
    local s3 = AppStatus.Loading
    test_enum(s1)
    test_enum(s2)
    test_enum(s3)
end

-- Struct Methods
-- Entry Point
if main then main() end
-- Exports
return {
    main = main,
}