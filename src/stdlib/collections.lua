-- Implementação Nativa da std.collections para Zenith
local zt = require("src.backend.lua.runtime.zenith_rt")

local COLLECTIONS = {}

-- 🛠️ HELPERS DE SEGURANÇA
local function check_lock(obj)
    if obj._locks and obj._locks > 0 then
        error("Erro de Concorrência: Coleção não pode ser modificada durante a iteração.", 3)
    end
end

-- 📦 SET (Conjunto Único)

function COLLECTIONS.new_set()
    local self = { _data = {}, _size = 0, _locks = 0 }
    
    function self:add_item(item)
        check_lock(self)
        if not self._data[item] then
            self._data[item] = true
            self._size = self._size + 1
        end
    end
    
    function self:remove_item(item)
        check_lock(self)
        if self._data[item] then
            self._data[item] = nil
            self._size = self._size - 1
        end
    end
    
    function self:contains_item(item)
        return self._data[item] == true
    end
    
    function self:clear_all()
        check_lock(self)
        self._data = {}
        self._size = 0
    end

    function self:iterator()
        self._locks = self._locks + 1
        local keys = {}
        for k in pairs(self._data) do table.insert(keys, k) end
        local i = 0
        return function()
            i = i + 1
            if i <= #keys then return zt.Optional.Present(keys[i]) end
            self._locks = self._locks - 1
            return zt.Optional.Empty
        end
    end
    
    return self
end

-- 📦 QUEUE (Fila)

function COLLECTIONS.new_queue()
    local self = { _data = {}, _first = 1, _last = 0, _locks = 0 }
    
    function self:enqueue_item(item)
        check_lock(self)
        self._last = self._last + 1
        self._data[self._last] = item
    end
    
    function self:dequeue_item()
        check_lock(self)
        if self._first > self._last then return zt.Optional.Empty end
        local val = self._data[self._first]
        self._data[self._first] = nil
        self._first = self._first + 1
        return zt.Optional.Present(val)
    end
    
    function self:peek_front()
        if self._first > self._last then return zt.Optional.Empty end
        return zt.Optional.Present(self._data[self._first])
    end
    
    return self
end

-- 📦 STACK (Pilha)

function COLLECTIONS.new_stack()
    local self = { _data = {}, _locks = 0 }
    
    function self:push_item(item)
        check_lock(self)
        table.insert(self._data, item)
    end
    
    function self:pop_item()
        check_lock(self)
        if #self._data == 0 then return zt.Optional.Empty end
        return zt.Optional.Present(table.remove(self._data))
    end
    
    function self:peek_top()
        if #self._data == 0 then return zt.Optional.Empty end
        return zt.Optional.Present(self._data[#self._data])
    end
    
    return self
end

-- 📦 GRID (Matriz 2D Eficiente)

function COLLECTIONS.new_grid(width, height, default)
    return zt.grid_new(width, height, default)
end

-- 🏗️ UTILITÁRIOS FUNCIONAIS (Embutidos)

function COLLECTIONS.filter_items(list, predicate)
    local res = {}
    for _, v in ipairs(list) do
        if predicate(v) then table.insert(res, v) end
    end
    return res
end

function COLLECTIONS.map_items(list, transform)
    local res = {}
    for _, v in ipairs(list) do
        table.insert(res, transform(v))
    end
    return res
end

function COLLECTIONS.distinct_items(list)
    local seen = {}
    local res = {}
    for _, v in ipairs(list) do
        if not seen[v] then
            seen[v] = true
            table.insert(res, v)
        end
    end
    return res
end

function COLLECTIONS.chunk_list(list, size)
    local res = {}
    for i = 1, #list, size do
        local chunk = {}
        for j = i, i + size - 1 do
            if list[j] then table.insert(chunk, list[j]) end
        end
        table.insert(res, chunk)
    end
    return res
end

function COLLECTIONS.find_first(list, predicate)
    for _, v in ipairs(list) do
        if predicate(v) then return zt.Optional.Present(v) end
    end
    return zt.Optional.Empty
end

return COLLECTIONS
