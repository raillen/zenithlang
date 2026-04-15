-- ============================================================================
-- Zenith Runtime (zenith_rt.lua)
-- Suporte para tipos e operações fundamentais do Zenith no Lua.
-- ============================================================================

local zt = {}

-- ----------------------------------------------------------------------------
-- TIPOS CORE
-- ----------------------------------------------------------------------------

zt.Optional = {
    Empty = { _tag = "Empty" },
}

local optional_methods = {
    unwrap_or = function(self, default) return self.value end,
    is_present = function(self) return true end,
    is_empty = function(self) return false end,
    unwrap = function(self, msg) 
        if self._tag == "Empty" then error(msg or "desempacotamento de Optional vazio", 2) end
        return self.value
    end
}

local optional_empty_methods = {
    unwrap_or = function(self, default) return default end,
    is_present = function(self) return false end,
    is_empty = function(self) return true end,
    unwrap = optional_methods.unwrap
}

setmetatable(zt.Optional.Empty, {
    __tostring = function() return "null" end,
    __index = optional_empty_methods
})

-- Redefinir Present para usar os métodos consolidados
zt.Optional.Present = function(value)
    local obj = { _tag = "Present", value = value, _1 = value }
    setmetatable(obj, {
        __tostring = function() return tostring(value) end,
        __index = optional_methods
    })
    return obj
end

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
                unwrap = function(self, msg) error(msg or ("desempacotamento de Failure: " .. tostring(self.error)), 2) end
            }
        })
        return obj
    end,
}

--- Operador Bang (!): Garante que um valor não é nulo/Empty/Failure — panic se for.
function zt.bang(val, msg)
    if val == nil then
        error(msg or "acesso a valor nulo (null!)", 2)
    end
    if type(val) == "table" then
        if val._tag == "Empty" then
            error(msg or "desempacotamento de Optional vazio (!)", 2)
        end
        if val._tag == "Failure" then
            error(msg or ("desempacotamento de Failure: " .. tostring(val.error or val._1)), 2)
        end
        if val._tag == "Present" or val._tag == "Success" then
            return val.value or val._1
        end
    end
    return val
end

--- Operador Try (?): Propaga Failure/Empty como retorno precoce.
--- Lança um erro estruturado especial {_zt_try = true, value = ...}
--- que deve ser capturado no contexto da função de chamada via zt.run_fallible.
function zt.try_unwrap(val)
    if val == nil then
        error({ _zt_try = true, _tag = "Failure", error = "null" }, 2)
    end
    if type(val) == "table" then
        if val._tag == "Empty" then
            error({ _zt_try = true, _tag = "Empty" }, 2)
        end
        if val._tag == "Failure" then
            error({ _zt_try = true, _tag = "Failure", error = val.error or val._1 }, 2)
        end
        if val._tag == "Present" or val._tag == "Success" then
            return val.value or val._1
        end
    end
    return val
end

--- Executa uma função que pode usar o operador ?.
--- Se um try_unwrap propagar, captura e retorna como Outcome.Failure.
--- Uso: zt.run_fallible(minha_func, args...)
function zt.run_fallible(fn, ...)
    local ok, result = pcall(fn, ...)
    if ok then
        return result
    end
    if type(result) == "table" and result._zt_try then
        if result._tag == "Empty" then return zt.Optional.Empty end
        return zt.Outcome.Failure(result.error)
    end
    -- Re-lança erros não relacionados ao operador ?
    error(result, 2)
end


--- Vinculação FFI (Foreign Function Interface) ou Globais Lua
function zt.ffi_bind(name)
    if name == "zt" then return zt end
    if name:sub(1, 3) == "zt." then
        local current = zt
        for part in name:sub(4):gmatch("[^%.]+") do
            if type(current) == "table" then
                current = current[part]
            else
                current = nil
                break
            end
        end
        if current then return current end
    end

    -- 1. Tenta buscar em tabela global Lua (suporta pontos como "io.write")
    local current = _G
    for part in name:gmatch("[^%.]+") do
        if type(current) == "table" then
            current = current[part]
        else
            current = nil
            break
        end
    end
    if current then return current end

    -- 2. Fallback para FFI C (LuaJIT pattern)
    local ok, ffi = pcall(require, "ffi")
    if ok then
        local status, res = pcall(function() return ffi.C[name] end)
        if status then return res end
    end

    return function() error("Símbolo externo não encontrado: " .. name) end
end

--- Verifica uma condição ou resultado.
function zt.check(val, msg)
    if val == nil or val == false then
        error(msg or "verificação de 'check' falhou", 2)
    end
    return val
end


function zt.assert(condition, msg)
    if condition == nil or condition == false then
        error(msg or "assertion failed", 2)
    end
    return condition
end
-- ----------------------------------------------------------------------------
-- REATIVIDADE MINIMA
-- ----------------------------------------------------------------------------

zt._watchers = zt._watchers or {}

function zt.watch(fn)
    table.insert(zt._watchers, fn)
    return fn()
end

function zt.state(initial)
    local obj = { _value = initial }
    return setmetatable(obj, {
        __index = function(self, key)
            if key == "get" then return rawget(self, "_value") end
            return nil
        end,
        __newindex = function(self, key, value)
            if key == "set" then
                rawset(self, "_value", value)
                for _, watcher in ipairs(zt._watchers) do watcher() end
            else
                rawset(self, key, value)
            end
        end,
    })
end

-- ----------------------------------------------------------------------------
-- LISTAS E COLEÇÕES
-- ----------------------------------------------------------------------------

local list_mt = {
    __index = {
        add = function(self, item) table.insert(self, item); return self end,
        push = function(self, item) table.insert(self, item); return self end,
        pop = function(self) return table.remove(self) end,
        remove = function(self, index) return table.remove(self, index) end,
        len = function(self) return #self end,
        length = function(self) return #self end,
        is_empty = function(self) return #self == 0 end,
        clear = function(self) for i=1,#self do self[i] = nil end end,
    }
}

--- Construtor de lista (usado pelo codegen)
function zt.list(t)
    return setmetatable(t or {}, list_mt)
end

-- ----------------------------------------------------------------------------
-- SEGURANÇA DE INDEXAÇÃO (FASE 11)
-- ----------------------------------------------------------------------------

function zt.index_seq(obj, i)
    if type(i) ~= "number" then error("ZT-R011: índice de sequência deve ser inteiro", 2) end
    if i < 1 or i > #obj then
        error(string.format("ZT-R011: índice de sequência fora dos limites (índice: %d, tamanho: %d)", i, #obj), 2)
    end
    return obj[i]
end

function zt.index_text(obj, i)
    if type(i) ~= "number" then error("ZT-R011: índice de texto deve ser inteiro", 2) end
    local len = zt.len(obj)
    if i < 1 or i > len then
        error(string.format("ZT-R011: índice de texto fora dos limites (índice: %d, tamanho: %d)", i, len), 2)
    end
    -- No Lua, indexação de string é por byte. Para UTF-8 no Zenith, podemos precisar de algo mais robusto futuramente.
    -- Por enquanto, seguimos a semântica de caracteres.
    return obj:sub(i, i)
end

function zt.index_any(obj, i)
    if type(obj) == "string" then return zt.index_text(obj, i) end
    if type(obj) == "table" then
        if obj._tag == "Grid" then return obj:get(i-1, 0) end -- Exemplo: simplificar para 1-based se for grid? (Não, grid é 0-based)
        if rawget(obj, 0) ~= nil then
            return rawget(obj, i)
        end
        -- Se for lista ou tabela com #, usamos index_seq
        if #obj > 0 or (getmetatable(obj) == list_mt) then
            return zt.index_seq(obj, i)
        end
        -- Fallback: Para mapas, acessamos direto sem validação de limite
        return obj[i]
    end
    return obj[i]
end

-- ----------------------------------------------------------------------------
-- OPERAÇÕES E UFCS STDLIB
-- ----------------------------------------------------------------------------

function zt.len(obj)
    if type(obj) == "string" then
        local _, count = obj:gsub("[%z\1-\127\194-\244][\128-\191]*", "")
        return count
    end
    -- Fallback safety para nulláveis
    if obj == nil or (type(obj) == "table" and obj._tag == "Empty") then return 0 end
    return #obj
end

function zt.split(obj, sep)
    local parts = {}
    local pattern = string.format("([^%s]+)", sep or ",")
    if type(obj) == "string" then
        obj:gsub(pattern, function(c) table.insert(parts, c) end)
    end
    return parts
end

function zt.keys(obj)
    local keys = {}
    if type(obj) == "table" then
        for k, _ in pairs(obj) do table.insert(keys, k) end
    end
    return keys
end

function zt.push(obj, value)
    if type(obj) == "table" then table.insert(obj, value) end
    return obj
end

function zt.pop(obj)
    if type(obj) == "table" then return table.remove(obj) end
    return nil
end

function zt.remove_at(obj, index)
    if type(obj) == "table" then return table.remove(obj, index) end
    return nil
end

function zt.is_present(obj)
    return type(obj) == "table" and (obj._tag == "Present" or obj._tag == "Success")
end

function zt.is_empty(obj)
    if obj == nil then return true end
    if type(obj) == "table" and obj._tag then
        return obj._tag == "Empty" or obj._tag == "Failure"
    end
    if type(obj) == "table" then return #obj == 0 end
    if type(obj) == "string" then return obj == "" end
    return false
end

function zt.is_success(obj)
    return type(obj) == "table" and obj._tag == "Success"
end

function zt.is_failure(obj)
    return type(obj) == "table" and obj._tag == "Failure"
end

function zt.unwrap(obj)
    if type(obj) == "table" and obj._tag then return obj.value end
    return obj
end

function zt.to_text(obj)
    return tostring(obj)
end

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

-- ----------------------------------------------------------------------------
-- MÉTODOS DE TIPOS PRIMITIVOS
-- ----------------------------------------------------------------------------

local function primitive_to_text(val)
    return tostring(val)
end

-- Configura metatables para tipos primitivos (se possível)
-- Isso permite usar 10.to_text() ou "abc".len() no Lua se o compilador emitir obj.method
-- Nota: No Lua padrão, números não aceitam metatable individual, mas aceitam via debug.setmetatable(0, ...)
local ok_num_mt, _ = pcall(function()
    debug.setmetatable(0, {
        __index = {
            to_text = primitive_to_text,
            to_float = function(self) return tonumber(self) + 0.0 end,
        }
    })
end)

-- Identificadores de tipo para `is` checks
zt.Outcome._name  = "Outcome"
zt.Optional._name = "Optional"

function zt.is(val, target_type)
    local t = type(val)

    -- Verificação por tipo primitivo (string)
    if type(target_type) == "string" then
        if t == target_type then return true end
        if target_type == "int"   and t == "number" and math.floor(val) == val then return true end
        if target_type == "float" and t == "number" then return true end
        if target_type == "text"  and t == "string" then return true end
        if target_type == "bool"  and t == "boolean" then return true end
        if t == "table" and val._metadata and val._metadata.name == target_type then return true end
        return false
    end

    -- Verificação por tipo estrutural (table com _name)
    if type(target_type) == "table" then
        if target_type._is_trait then return zt.implements(val, target_type) end
        if target_type._name == "Outcome" then
            return t == "table" and (val._tag == "Success" or val._tag == "Failure")
        end
        if target_type._name == "Optional" then
            return t == "table" and (val._tag == "Present" or val._tag == "Empty")
        end
        -- Verificação por struct _metadata
        if t == "table" and val._metadata then
            return getmetatable(val) == target_type
        end
        return false
    end

    return false
end


function zt.slice(obj, start_v, end_v)
    if type(obj) == "string" then
        local len = zt.len(obj)
        local s = start_v or 1
        local e = end_v or len
        if s < 1 or s > len or e < 1 or e > len then
             error(string.format("ZT-R011: slice de texto fora dos limites (range: %d..%d, tamanho: %d)", s, e, len), 2)
        end
        return obj:sub(s, e)
    elseif type(obj) == "table" then
        local len = #obj
        local s = start_v or 1
        local e = end_v or len
        if s < 1 or s > len or e < 1 or e > len then
             error(string.format("ZT-R011: slice de lista fora dos limites (range: %d..%d, tamanho: %d)", s, e, len), 2)
        end
        local res = {}
        for i = s, e do
            table.insert(res, obj[i])
        end
        return zt.list(res)
    end
    return obj
end

function zt.iter(obj)
    if type(obj) == "table" then
        if obj._iter then return obj:_iter() end
        local i = 0
        local n = #obj
        return function()
            i = i + 1
            if i <= n then return obj[i] end
        end
    end
    error("objeto não iterável")
end

-- ----------------------------------------------------------------------------
-- TRAITS E INTERFACES
-- ----------------------------------------------------------------------------

function zt.register_trait(struct_mt, trait_mt)
    if not struct_mt._implements then
        struct_mt._implements = {}
    end
    struct_mt._implements[trait_mt._name] = trait_mt
end

function zt.implements(obj, trait_mt)
    if type(obj) == "table" then
        local mt = getmetatable(obj)
        if mt and mt._implements and mt._implements[trait_mt._name] then
            return true
        end
    end
    return false
end


function zt.safe_get(obj, member)
    if obj == nil then return nil end
    local val = obj[member]
    -- Se for uma função Zenith encapsulada em Optional, lidamos com isso?
    -- No v0.4, apenas retornamos o valor ou nil.
    return val
end

function zt.async_run(fn, ...)
    local ok, res = pcall(fn, ...)
    if not ok then error(res, 2) end
    return res
end

zt.async = zt.async_run

function zt.await(task)
    if type(task) ~= "table" or not task.co then return task end
    while coroutine.status(task.co) ~= "dead" do
        local ok, res = coroutine.resume(task.co)
        if not ok then error(res, 2) end
        task.result = res
    end
    return task.result
end

function zt.drive(task)
    return zt.await(task)
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
