-- Implementação Nativa da std.events para Zenith
local zt = require("src.backend.lua.runtime.zenith_rt")

local EVENTS = {}

-- 🛠️ HELPERS
local function generate_id()
    local charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
    local res = ""
    for i = 1, 6 do
        local r = math.random(1, #charset)
        res = res .. charset:sub(r, r)
    end
    return "L-" .. res
end

-- 📦 EVENT EMITTER

function EVENTS.EventEmitter()
    local self = {
        _listeners = {}, -- Map<event, List<Listener>>
        _id_map = {}     -- Map<id, {event, index}>
    }

    function self:on(event, callback)
        if not self._listeners[event] then
            self._listeners[event] = {}
        end

        local id = generate_id()
        local listener = {
            id = id,
            callback = callback,
            once = false
        }

        table.insert(self._listeners[event], listener)
        self._id_map[id] = { event = event, listener = listener }
        
        return id
    end

    function self:once(event, callback)
        local id = self:on(event, callback)
        self._id_map[id].listener.once = true
        return id
    end

    function self:off(id)
        local entry = self._id_map[id]
        if not entry then return false end

        local event = entry.event
        local listeners = self._listeners[event]
        if listeners then
            for i, l in ipairs(listeners) do
                if l.id == id then
                    table.remove(listeners, i)
                    break
                end
            end
        end

        self._id_map[id] = nil
        return true
    end

    function self:emit(event, data)
        local listeners = self._listeners[event]
        if not listeners or #listeners == 0 then
            return zt.Outcome.Success(nil)
        end

        local errors = {}
        local to_remove = {}

        -- Clonamos a lista para evitar problemas se alguém chamar 'off' durante o processamento
        local targets = {}
        for _, l in ipairs(listeners) do table.insert(targets, l) end

        for _, l in ipairs(targets) do
            local ok, err = pcall(l.callback, data)
            if not ok then
                table.insert(errors, tostring(err))
            end
            if l.once then
                table.insert(to_remove, l.id)
            end
        end

        -- Limpeza de listeners 'once'
        for _, id in ipairs(to_remove) do
            self:off(id)
        end

        if #errors > 0 then
            return zt.Outcome.Failure(errors)
        end

        return zt.Outcome.Success(nil)
    end

    function self:count(event)
        if event then
            return self._listeners[event] and #self._listeners[event] or 0
        end
        
        local total = 0
        for _, list in pairs(self._listeners) do
            total = total + #list
        end
        return total
    end

    function self:clear(event)
        if event then
            local list = self._listeners[event]
            if list then
                for _, l in ipairs(list) do
                    self._id_map[l.id] = nil
                end
                self._listeners[event] = nil
            end
        else
            self._listeners = {}
            self._id_map = {}
        end
    end

    function self:wait_for(event)
        return zt.async_run(function()
            local received = false
            local result = nil
            
            local id
            id = self:once(event, function(data)
                result = data
                received = true
            end)

            while not received do
                coroutine.yield()
            end

            return result
        end)
    end

    return self
end

return EVENTS
