-- Implementação Nativa da std.os para Zenith
local zt = require("src.backend.lua.runtime.zenith_rt")

local OS = {}

-- 📂 ENUMS
OS.SpecialPath = {
    Config = "Config",
    Temp = "Temp",
    Desktop = "Desktop",
    Documents = "Documents"
}

-- 🔎 DETECÇÃO DE PLATAFORMA
local is_windows = package.config:sub(1,1) == "\\"
local platform_name = is_windows and "windows" or "linux" -- simplificado

-- 💎 PROPRIEDADES
OS.version = "0.2.0"
OS.platform = platform_name
OS.arch = os.getenv("PROCESSOR_ARCHITECTURE") or "x64"
OS.project_root = os.getenv("ZENITH_PROJECT_ROOT") or "."

-- 🏗️ FUNÇÕES

function OS.get_env_variable(name)
    local val = os.getenv(name)
    if val then
        return zt.Optional.Present(val)
    else
        return zt.Optional.Empty
    end
end

function OS.get_all_env_variables()
    local envs = {}
    -- No Windows/Lua 5.1 não há forma nativa fácil sem popen
    -- Vamos usar uma aproximação via set (cmd)
    if is_windows then
        local handle = io.popen("set")
        for line in handle:lines() do
            local k, v = line:match("^(.-)=(.*)$")
            if k then envs[k] = v end
        end
        handle:close()
    end
    return envs
end

function OS.get_system_info()
    local info = {
        name = platform_name,
        arch = OS.arch,
        username = os.getenv(is_windows and "USERNAME" or "USER") or "unknown",
        home_dir = os.getenv(is_windows and "USERPROFILE" or "HOME") or "."
    }
    -- No Zenith, retornamos a struct (tabela com metatabela se necessário)
    return info
end

function OS.get_hardware_info()
    local cpu = "Unknown CPU"
    local ram = 0.0
    
    if is_windows then
        -- 1. Tentar CPU via WMIC
        local h_cpu = io.popen("wmic cpu get name 2>nul")
        if h_cpu then
            h_cpu:read("*l") -- header
            local res = h_cpu:read("*l")
            if res and res:match("%S") then cpu = res:gsub("^%s*(.-)%s*$", "%1") end
            h_cpu:close()
        end

        -- Fallback CPU: Variável de Ambiente
        if cpu == "Unknown CPU" then
            cpu = os.getenv("PROCESSOR_IDENTIFIER") or "Unknown Intel/AMD"
        end

        -- 2. Tentar RAM via WMIC
        local h_ram = io.popen("wmic OS get TotalVisibleMemorySize 2>nul")
        if h_ram then
            h_ram:read("*l") -- header
            local ram_str = h_ram:read("*l")
            if ram_str and tonumber(ram_str) then
                ram = tonumber(ram_str) / (1024 * 1024)
            end
            h_ram:close()
        end

        -- Fallback RAM: SYSTEMINFO (mais lento, mas garantido)
        if ram == 0.0 then
            local h_sys = io.popen("systeminfo | findstr /C:\"Total Physical Memory\"")
            if h_sys then
                local res = h_sys:read("*l")
                if res then
                    -- Ex: "Total Physical Memory:     16,312 MB"
                    local num = res:match("[%d%,%.]+")
                    if num then
                        num = num:gsub("[,%.]", "")
                        ram = tonumber(num) / 1024
                    end
                end
                h_sys:close()
            end
        end
    end

    return {
        cpu = cpu,
        ram_gb = ram,
        battery = nil,
        is_charging = false
    }
end

function OS.run_command(cmd)
    local ok, kind, code = os.execute(cmd)
    if is_windows then return ok end -- No 5.1/Windows retorna o código direto
    return code or 0
end

function OS.get_working_dir()
    if is_windows then
        local h = io.popen("cd")
        local path = h:read("*l")
        h:close()
        return path
    else
        local h = io.popen("pwd")
        local path = h:read("*l")
        h:close()
        return path
    end
end

function OS.get_special_path(kind)
    -- kind vem como string do Enum ("Desktop", "Config", etc)
    if is_windows then
        if kind == "Desktop" then return os.getenv("USERPROFILE") .. "\\Desktop"
        elseif kind == "Documents" then return os.getenv("USERPROFILE") .. "\\Documents"
        elseif kind == "Config" then return os.getenv("APPDATA")
        elseif kind == "Temp" then return os.getenv("TEMP")
        end
    end
    return "."
end

function OS.is_admin()
    if is_windows then
        -- Truque comum: tentar escrever em pasta do sistema
        local success = os.execute("net session >nul 2>&1")
        return success == 0
    end
    return false
end

function OS.exit(code)
    os.exit(code)
end

return OS
