-- ============================================================================
-- Zenith Runtime (zenith_rt.lua)
-- Suporte para reatividade e tipos do Zenith v0.2.0
-- ============================================================================

local zt = {}
local unpack = table.unpack or unpack

-- ----------------------------------------------------------------------------
-- Reatividade (Core do ZenEngine)
-- ----------------------------------------------------------------------------

local active_listener = nil
local listener_stack = {}

local function push_listener(listener)
    table.insert(listener_stack, active_listener)
    active_listener = listener
end

local function pop_listener()
    active_listener = table.remove(listener_stack)
end

--- Cria um novo estado reativo.
function zt.state(initial_value)
    local internal = {
        value = initial_value,
        observers = {}
    }
    
    local proxy = {}
    setmetatable(proxy, {
        __index = function(_, k)
            if k == "get" then
                if active_listener then
                    internal.observers[active_listener] = true
                end
                return internal.value
            end
        end,
        __newindex = function(_, k, v)
            if k == "set" then
                if internal.value ~= v then
                    internal.value = v
                    -- Notificar observadores
                    for obs, _ in pairs(internal.observers) do
                        obs()
                    end
                end
            end
        end,
        __tostring = function() return tostring(internal.value) end
    })
    
    return proxy
end

--- Cria um valor computado.
function zt.computed(fn)
    local internal = {
        value = nil,
        dirty = true,
        observers = {}
    }
    
    local function update()
        if not internal.dirty then
            internal.dirty = true
            -- Notificar observadores deste computed
            for obs, _ in pairs(internal.observers) do
                obs()
            end
        end
    end
    
    local proxy = {}
    setmetatable(proxy, {
        __index = function(_, k)
            if k == "get" then
                if internal.dirty then
                    push_listener(update)
                    internal.value = fn()
                    pop_listener()
                    internal.dirty = false
                end
                
                if active_listener then
                    internal.observers[active_listener] = true
                end
                
                return internal.value
            end
        end,
        __tostring = function() return tostring(internal.value) end
    })
    
    return proxy
end

--- Observa uma expressão ou valor reativo.
function zt.watch(fn)
    local function runner()
        push_listener(runner)
        fn()
        pop_listener()
    end
    runner() -- Execução inicial
end

--- Verificação de tipo 'is'
function zt.is(val, target_type)
    if val == nil then return target_type == "null" or target_type == "any" end
    if target_type == "any" then return true end

    local t = type(val)
    if target_type == "table" then return t == "table" end

    if target_type == "int" or target_type == "float" or target_type == "number" then

        return t == "number"
    elseif target_type == "text" or target_type == "string" then
        return t == "string"
    elseif target_type == "bool" or target_type == "boolean" then
        return t == "boolean"
    end
    
    -- Para structs, comparamos a metatabela
    if type(target_type) == "table" then
        return getmetatable(val) == target_type
    end
    
    return false
end

-- ----------------------------------------------------------------------------
-- Utilitários
-- ----------------------------------------------------------------------------

--- Operador Bang (!): Garante que um valor não é nulo.
function zt.bang(val, msg)
    if val == nil then
        error(msg or "acesso a valor nulo (null!)", 2)
    end
    return val
end

--- Verifica uma condição ou resultado.
--- Se for um booleano, false dispara erro.
--- Se for outro tipo, nil dispara erro (bubble up).
function zt.check(val, msg)
    if val == nil or val == false then
        zt.error(msg or "verificação de 'check' falhou")
    end
    return val
end

--- Helper para Listas (Indexação 1-based nativa do Lua)
function zt.list(...)
    return {...}
end

--- Cria uma nova lista espalhando elementos de outras listas (Spread)
function zt.list_spread(...)
    local res = {}
    local args = {...}
    for _, arg in ipairs(args) do
        if type(arg) == "table" and not getmetatable(arg) then
            for _, v in ipairs(arg) do table.insert(res, v) end
        else
            table.insert(res, arg)
        end
    end
    return res
end

--- Helper para Mapas
function zt.map(t)
    return t or {}
end

--- Cria um novo mapa espalhando entradas de outros mapas (Spread)
function zt.map_spread(...)
    local res = {}
    local args = {...}
    for _, arg in ipairs(args) do
        if type(arg) == "table" then
            for k, v in pairs(arg) do res[k] = v end
        end
    end
    return res
end

--- Retorna uma subparte de uma lista ou string (Slicing)
function zt.slice(obj, start, finish)
    if type(obj) == "string" then
        -- Lua strings são 1-based, mas no Zenith tentamos abstrair
        return obj:sub(start + 1, finish + 1)
    elseif type(obj) == "table" then
        local res = {}
        for i = start + 1, finish + 1 do
            table.insert(res, obj[i])
        end
        return res
    end
    return obj
end

--- Helper para Range (Retorna um iterador ou tabela)
function zt.range(start, finish)
    local current = start - 1
    return function()
        current = current + 1
        if current <= finish then
            return current
        end
    end
end

-- ----------------------------------------------------------------------------
-- Standard Library Support
-- ----------------------------------------------------------------------------

--- Implementação do print() do Zenith que retorna o valor impresso como texto.
function zt.print(...)
    print(...)
end

--- Implementação de assert()
function zt.assert(condition, message, ...)
    assert(condition, message, ...)
end

--- Suporte a Async/Await (Corrotinas Transparentes)
function zt.async(fn)
    return function(...)
        local co = coroutine.create(fn)
        local function execute(...)
            local args = {...}
            while coroutine.status(co) ~= "dead" do
                local ok, res = coroutine.resume(co, unpack(args))
                if not ok then
                    error(res, 0)
                end
                
                if coroutine.status(co) == "dead" then
                    return res
                end
                
                -- Se yieldou uma função (ex: resultado de outro zt.async), 
                -- precisamos esperar que ela termine para continuar este step.
                -- No Zenith v1.0-alpha, resolvemos imediatamente se não houver loop de eventos.
                if type(res) == "function" then
                    -- Simula o await resolvendo a função e pegando seu resultado
                    args = { res(function(val) return val end) }
                else
                    -- Se yieldou um valor puro, devolvemos pra coroutine no próximo loop
                    args = { res }
                end
            end
        end
        return execute(...)
    end
end

--- Implementação de + (sobrecarregado para soma e concatenação)
function zt.add(a, b)
    if type(a) == "number" and type(b) == "number" then
        return a + b
    end
    -- Se um for string ou não puder somar, concatena
    return tostring(a) .. tostring(b)
end

--- Lança um erro formatado.
function zt.error(msg)
    error({ message = msg }, 2)
end

return zt
