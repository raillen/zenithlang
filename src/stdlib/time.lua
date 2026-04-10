-- Implementação Nativa da std.time para Zenith
local zt = require("src.backend.lua.runtime.zenith_rt")

local TIME = {}

-- 📂 ENUMS
TIME.Weekday = {
    Sunday = 1, Monday = 2, Tuesday = 3, Wednesday = 4, Thursday = 5, Friday = 6, Saturday = 7
}

local weekday_map = {
    ["Sunday"] = 1, ["Monday"] = 2, ["Tuesday"] = 3, ["Wednesday"] = 4, ["Thursday"] = 5, ["Friday"] = 6, ["Saturday"] = 7
}

-- 🏗️ CONSTRUTORES

function TIME.seconds(s)
    return { seconds = s, _tag = "Duration" }
end

function TIME.ms(m)
    return { seconds = m / 1000, _tag = "Duration" }
end

function TIME.duration(h, m, s, ms)
    local total = (h or 0) * 3600 + (m or 0) * 60 + (s or 0) + (ms or 0) / 1000
    return { seconds = total, _tag = "Duration" }
end

-- 🚀 CORE

function TIME.now()
    local t = os.time()
    local d = os.date("*t", t)
    return {
        year = d.year, month = d.month, day = d.day,
        hour = d.hour, minute = d.min, second = d.sec,
        millisecond = math.floor((os.clock() % 1) * 1000),
        weekday = d.wday,
        unix = t,
        _tag = "DateTime"
    }
end

function TIME.sleep(d)
    local s = d.seconds
    if package.config:sub(1,1) == "\\" then
        -- Windows: truque do ping (não consome 100% CPU)
        os.execute("ping -n " .. math.floor(s + 1) .. " 127.0.0.1 > nul")
    else
        -- Linux/Posix
        os.execute("sleep " .. s)
    end
end

function TIME.wait(d)
    -- No Zenith, wait assíncrono usa o runtime de corrotinas
    return zt.async_run(function()
        local start = os.clock()
        while os.clock() - start < d.seconds do
            coroutine.yield() -- Libera p/ outras tarefas
        end
    end)
end

-- 🕒 STOPWATCH

function TIME.stopwatch()
    local sw = { running = false, start_t = 0, elapsed = 0 }
    
    function sw:start()
        if not self.running then
            self.start_t = os.clock()
            self.running = true
        end
    end
    
    function sw:stop()
        if self.running then
            self.elapsed = self.elapsed + (os.clock() - self.start_t)
            self.running = false
        end
        return { seconds = self.elapsed, _tag = "Duration" }
    end
    
    function sw:reset()
        self.elapsed = 0
        self.running = false
    end
    
    return sw
end

-- 📝 FORMATAÇÃO

local function format_date(dt, pattern)
    -- Mapeamento simples de exemplos para tokens do os.date
    local map = {
        ["YYYY"] = "%Y", ["YY"] = "%y",
        ["MM"] = "%m",
        ["DD"] = "%d",
        ["HH"] = "%H", ["mm"] = "%M", ["ss"] = "%S"
    }
    local res = pattern
    for k, v in pairs(map) do
        res = res:gsub(k, v)
    end
    return os.date(res, dt.unix)
end

-- Injetar métodos nas tabelas que o Zenith usará como instâncias
-- (No v0.2 o Codegen lida com isso se definirmos na metatabela ou no objeto)

return TIME
