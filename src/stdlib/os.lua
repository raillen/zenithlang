local zt = require("src.backend.lua.runtime.zenith_rt")

local OS = {}

OS.version = "0.3.1"
OS.platform = package.config:sub(1,1) == "\\" and "windows" or "unix"
OS.arch = "x64"

-- ENUMS
OS.SpecialPath = {
    Config = "Config", Temp = "Temp", Desktop = "Desktop", Documents = "Documents"
}

function OS.get_env_variable(name)
    local val = os.getenv(name)
    return val and zt.Optional.Present(val) or zt.Optional.Empty
end
OS.get_env = OS.get_env_variable


function OS.get_all_env_variables()
    -- Mock simples para compatibilidade
    return { ["USER"] = os.getenv("USERNAME") or os.getenv("USER") or "unknown" }
end

function OS.get_hardware_info()
    return { cpu = "Zenith Optimized CPU", ram_gb = 16 }
end

function OS.get_special_path(kind)
    if OS.platform == "windows" then
        return os.getenv("USERPROFILE") or "C:\\"
    else
        return os.getenv("HOME") or "/"
    end
end

function OS.get_working_dir()
    return os.getenv("PWD") or "."
end

function OS.is_admin()
    return false
end

function OS.run_command(command)
    local res1, res2, res3 = os.execute(command)
    local is_success = false
    local exit_code = 0
    if type(res1) == "boolean" then
        is_success = res1
        exit_code = res3 or 0
    else
        is_success = (res1 == 0)
        exit_code = res1 or 0
    end
    
    if is_success then
        return zt.Outcome.Success(exit_code)
    else
        return zt.Outcome.Failure("Comando falhou com codigo " .. tostring(exit_code))
    end
end

function OS.exit_process(code)
    os.exit(code)
end

OS.exit = OS.exit_process

function OS.sleep(ms)
    local start = os.clock()
    while os.clock() - start < ms/1000 do end
end

function OS.get_args()
    local args = {}
    for i, v in ipairs(_G.arg or {}) do
        table.insert(args, v)
    end
    return args
end

function OS.get_platform()
    return OS.platform
end

return OS
