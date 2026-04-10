-- Implementação Nativa da std.log para Zenith
local zt = require("src.backend.lua.runtime.zenith_rt")

local LOG = {}

-- 📂 ENUMS
LOG.LogLevel = {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warning = 3,
    Error = 4,
    Panic = 5
}

LOG.LogFormat = {
    Text = "Text",
    Json = "Json"
}

-- ⚙️ CONFIGURAÇÃO GLOBAL
local current_level = LOG.LogLevel.Info
local current_format = LOG.LogFormat.Text

-- 🎨 CORES AURA (ANSI)
local COLORS = {
    Trace  = "\27[90m",    -- Cinza
    Debug  = "\27[34m",    -- Azul
    Info   = "\27[36m",    -- Ciano (Zenith Blue)
    Warn   = "\27[33m",    -- Amarelo
    Error  = "\27[31m",    -- Vermelho
    Panic  = "\27[41;37m", -- Branco sobre Vermelho
    Reset  = "\27[0m",
    Dim    = "\27[2m"      -- Esmaecido
}

-- 🛠️ HELPERS
local function get_timestamp()
    return os.date("%H:%M:%S")
end

local function get_caller_info()
    local info = debug.getinfo(4, "Sl") -- Nível 4 para pular o wrapper e o log.func
    if info then
        local file = info.short_src:match("([^/\\]+)$") or info.short_src
        return string.format("%s:%d", file, info.currentline)
    end
    return "unknown"
end

local function write_log(level_name, level_val, msg)
    if level_val < current_level then return end
    
    local timestamp = get_timestamp()
    local caller = get_caller_info()
    
    if current_format == LOG.LogFormat.Json then
        local json = require("src/stdlib/json")
        local entry = {
            timestamp = timestamp,
            level = level_name:upper(),
            caller = caller,
            message = tostring(msg)
        }
        -- Usar indentação conforme solicitado (Pretty Print)
        print(json.stringify(entry, 4))
    else
        local color = COLORS[level_name] or ""
        local reset = COLORS.Reset
        local dim = COLORS.Dim
        
        local prefix = string.format("%s[%s]%s", color, level_name:upper(), reset)
        local meta = string.format("%s[%s] [%s]%s", dim, timestamp, caller, reset)
        
        print(string.format("%s %s %s", prefix, meta, tostring(msg)))
    end
    
    if level_val == LOG.LogLevel.Panic then
        os.exit(1)
    end
end

-- 🏗️ FUNÇÕES PÚBLICAS

function LOG.set_level(level)
    current_level = level
end

function LOG.set_format(format)
    current_format = format
end

function LOG.trace(msg) write_log("Trace", LOG.LogLevel.Trace, msg) end
function LOG.debug(msg) write_log("Debug", LOG.LogLevel.Debug, msg) end
function LOG.info(msg)  write_log("Info",  LOG.LogLevel.Info,  msg) end
function LOG.warn(msg)  write_log("Warn",  LOG.LogLevel.Warning, msg) end
function LOG.err(msg)   write_log("Error", LOG.LogLevel.Error, msg) end
function LOG.panic(msg) write_log("Panic", LOG.LogLevel.Panic, msg) end

return LOG
