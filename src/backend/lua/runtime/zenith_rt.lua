-- ============================================================================
-- Zenith Runtime (zenith_rt.lua)
-- Suporte para tipos e operações fundamentais do Zenith no Lua.
-- ============================================================================

local zt = {}

-- ----------------------------------------------------------------------------
-- TIPOS CORE
-- ----------------------------------------------------------------------------

zt.Optional = {
    Present = function(value)
        local obj = { _tag = "Present", value = value, _1 = value }
        setmetatable(obj, {
            __tostring = function() return tostring(value) end,
            __index = {
                unwrap_or = function(self, default) return self.value end,
                is_present = function(self) return true end,
                is_empty = function(self) return false end,
            }
        })
        return obj
    end,
    Empty = { _tag = "Empty" },
}
setmetatable(zt.Optional.Empty, {
    __tostring = function() return "null" end,
    __index = {
        unwrap_or = function(self, default) return default end,
        is_present = function(self) return false end,
        is_empty = function(self) return true end,
    }
})

zt.Outcome = {
    Success = function(value)
        local obj = { _tag = "Success", value = value, _1 = value }
        setmetatable(obj, {
            __tostring = function() return tostring(value) end,
            __index = {
                unwrap_or = function(self, default) return self.value end,
                is_success = function(self) return true end,
                is_failure = function(self) return false end,
            }
        })
        return obj
    end,
    Failure = function(err)
        local obj = { _tag = "Failure", error = err, _1 = err }
        setmetatable(obj, {
            __tostring = function() return "Error: " .. tostring(err) end,
            __index = {
                unwrap_or = function(self, default) return default end,
                is_success = function(self) return false end,
                is_failure = function(self) return true end,
            }
        })
        return obj
    end,
}

--- Operador Bang (!): Garante que um valor não é nulo.
function zt.bang(val, msg)
    if val == nil then
        error(msg or "acesso a valor nulo (null!)", 2)
    end
    return val
end

--- Vinculação FFI (Foreign Function Interface)
function zt.ffi_bind(name)
    local ok, ffi = pcall(require, "ffi")
    if not ok then
        return function() error("FFI não suportado nesta VM (use LuaJIT)") end
    end
    -- Heurística simples: tenta buscar no C global
    return ffi.C[name]
end

--- Verifica uma condição ou resultado.
function zt.check(val, msg)
    if val == nil or val == false then
        error(msg or "verificação de 'check' falhou", 2)
    end
    return val
end

-- ----------------------------------------------------------------------------
-- OPERAÇÕES
-- ----------------------------------------------------------------------------

function zt.add(a, b)
    if type(a) == "string" or type(b) == "string" then
        return tostring(a) .. tostring(b)
    end
    return a + b
end

function zt.unwrap_or(val, default)
    if type(val) == "table" and val._tag then
        if val._tag == "Present" or val._tag == "Success" then
            return val.value
        end
        return default
    end
    return val or default
end

function zt.is(val, target_type)
    local t = type(val)
    if t == target_type then return true end
    if target_type == "int" and t == "number" and math.floor(val) == val then return true end
    if target_type == "float" and t == "number" then return true end
    if target_type == "text" and t == "string" then return true end
    if target_type == "bool" and t == "boolean" then return true end
    if t == "table" and val._metadata and val._metadata.name == target_type then return true end
    return false
end

function zt.slice(obj, start_v, end_v)
    if type(obj) == "string" then
        return obj:sub(start_v or 1, end_v or -1)
    elseif type(obj) == "table" then
        local res = {}
        for i = (start_v or 1), (end_v or #obj) do
            table.insert(res, obj[i])
        end
        return res
    end
    return obj
end

function zt.iter(obj)
    if type(obj) == "table" then
        if obj._iter then return obj:_iter() end
        return ipairs(obj)
    end
    error("objeto não iterável")
end

function zt.async_run(fn, ...)
    local co = coroutine.create(fn)
    local task = { co = co, status = "running" }
    local ok, res = coroutine.resume(co, ...)
    if not ok then error(res) end
    return task
end

function zt.await(task)
    if type(task) ~= "table" or not task.co then return task end
    while coroutine.status(task.co) ~= "dead" do
        coroutine.yield()
    end
    return task
end

-- ----------------------------------------------------------------------------
-- MOTOR GRID (Flat Array 2D)
-- ----------------------------------------------------------------------------

local GridMT = {}
GridMT.__index = GridMT

function GridMT:get(x, y)
    if x < 0 or x >= self.width or y < 0 or y >= self.height then
        return zt.Optional.Empty
    end
    local idx = (y * self.width) + x + 1 -- Lua é 1-indexed
    return zt.Optional.Present(self.data[idx])
end

function GridMT:set(x, y, val)
    if x < 0 or x >= self.width or y < 0 or y >= self.height then
        return false
    end
    local idx = (y * self.width) + x + 1
    self.data[idx] = val
    return true
end

function GridMT:fill(val)
    for i = 1, #self.data do self.data[i] = val end
end

function zt.grid_new(w, h, default)
    local data = {}
    local total = w * h
    for i = 1, total do data[i] = default end
    
    local obj = {
        width = w,
        height = h,
        data = data,
        _tag = "Grid"
    }
    return setmetatable(obj, GridMT)
end

return zt
