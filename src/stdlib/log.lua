-- Implementação Nativa da std.log para Zenith
local zt = require("src.backend.lua.runtime.zenith_rt")

local LOG = {}

-- 📂 ENUMS
LOG.Level = {
    Trace = "Trace", Debug = "Debug", Info = "Info",
    Warn = "Warn", Error = "Error", Panic = "Panic"
}

-- 🎨 CORES AURA (ANSI)
local colors = {
    trace = "\27[38;5;244m", -- Cinza
    debug = "\27[38;5;39m",  -- Azul
    info  = "\27[38;5;51m",  -- Ciano
    warn  = "\27[38;5;226m", -- Amarelo
    error = "\27[38;5;196m", -- Vermelho
    panic = "\27[1;38;5;196m", -- Vermelho Negrito
    reset = "\27[0m",
    gray  = "\27[38;5;244m"
}

-- 📂 ESTADO INTERNO
local current_min_level = 3 -- Info por padrão (Trace=1, Debug=2...)
local global_context = {}
local sinks = {}

-- Níveis ordinais
local levels = { "Trace", "Debug", "Info", "Warn", "Error", "Panic" }
local level_map = { Trace=1, Debug=2, Info=3, Warn=4, Error=5, Panic=6 }

-- 🏗️ CONFIGURAÇÃO

function LOG.set_level(level_str)
    current_min_level = level_map[level_str] or 3
end

function LOG.with_context(data)
    for k, v in pairs(data or {}) do
        global_context[k] = v
    end
end

function LOG.add_sink(handler)
    table.insert(sinks, handler)
end

-- 🚀 EMISSÃO INTERNA

local function emit(level_str, msg)
    local level_idx = level_map[level_str]
    if level_idx < current_min_level then return end

    -- Criar entrada de log
    local entry = {
        level = level_str,
        message = msg,
        timestamp = os.clock(),
        context = {}
    }
    -- Mesclar contexto global
    for k, v in pairs(global_context) do entry.context[k] = v end

    -- 1. Sink Padrão: Console Aura
    local color = colors[level_str:lower()] or colors.reset
    local time_str = string.format("%.3f", entry.timestamp)
    print(string.format("%s[%s]%s %s%-5s%s %s", 
        colors.gray, time_str, colors.reset,
        color, level_str:upper(), colors.reset,
        msg))

    -- 2. Chamar Sinks Customizados
    for _, handler in ipairs(sinks) do
        pcall(handler, entry)
    end

    if level_str == "Panic" then
        error("PANIC: " .. msg)
    end
end

-- 🛰️ EXPORTS

function LOG.log_trace(msg) emit("Trace", msg) end
function LOG.log_debug(msg) emit("Debug", msg) end
function LOG.log_info(msg)  emit("Info", msg)  end
function LOG.log_warn(msg)  emit("Warn", msg)  end
function LOG.log_error(msg) emit("Error", msg) end
function LOG.log_panic(msg) emit("Panic", msg) end

function LOG.time_task(name, callback)
    local start = os.clock()
    callback()
    local duration = (os.clock() - start) * 1000
    LOG.log_info(string.format("%s concluído em %.2fms", name, duration))
end

return LOG
