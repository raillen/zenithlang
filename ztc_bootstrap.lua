-- Zenith Sovereign Compiler v0.3.5 (Bundled)
-- Gerado automaticamente pelo Zenith Bundler

package.preload['src.backend.lua.runtime.zenith_rt'] = function()
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
    local pattern = string.format("([^%s]+)", sep or " ")
    if type(obj) == "string" then
        obj:gsub(pattern, function(c) table.insert(parts, c) end)
    end
    return parts
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

end

package.preload['src/stdlib/io'] = function()
-- Transpilado por Zenith Ascension (Native v0.4)
local zt = require("src.backend.lua.runtime.zenith_rt")

function write(value)
    -- native lua
    io . write ( tostring ( value ) )
end
function write_line(value)
    -- native lua
    print ( tostring ( value ) )
end
function read_line()
    -- native lua
    local line = io . read ( "*l" ) if line then return zt.Optional . Present ( line ) else return zt.Optional . Empty end
end
function read_file(path)
    local f = io.open(path, "r")
    if not f then return "" end
    local content = f:read("*a")
    f:close()
    return content
end
function write_file(path, content)
    local f = io.open(path, "w")
    if f then
        f:write(content)
        f:close()
    end
end

return {
    write = write,
    write_line = write_line,
    read_line = read_line,
    read_file = read_file,
    write_file = write_file,
}

end

package.preload['src/stdlib/os'] = function()
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

end

package.preload['src/stdlib/text'] = function()
-- Implementação Nativa da std.text para Zenith
local zt = require("src.backend.lua.runtime.zenith_rt")

local TEXT = {}

-- 📂 TABELAS DE SUPORTE
local accent_map = {
    ["á"]="a", ["à"]="a", ["â"]="a", ["ã"]="a", ["ä"]="a",
    ["é"]="e", ["è"]="e", ["ê"]="e", ["ë"]="e",
    ["í"]="i", ["ì"]="i", ["î"]="i", ["ï"]="i",
    ["ó"]="o", ["ò"]="o", ["ô"]="o", ["õ"]="o", ["ö"]="o",
    ["ú"]="u", ["ù"]="u", ["û"]="u", ["ü"]="u",
    ["ç"]="c", ["ñ"]="n",
    ["Á"]="A", ["À"]="A", ["Â"]="A", ["Ã"]="A", ["Ä"]="A",
    ["É"]="E", ["È"]="E", ["Ê"]="E", ["Ë"]="E",
    ["Í"]="I", ["Ì"]="I", ["Î"]="I", ["Ï"]="I",
    ["Ó"]="O", ["Ò"]="O", ["Ô"]="O", ["Õ"]="O", ["Ö"]="O",
    ["Ú"]="U", ["Ù"]="U", ["Û"]="U", ["Ü"]="U",
    ["Ç"]="C", ["Ñ"]="N"
}

-- 🔡 TRANSFORMAÇÃO

function TEXT.to_upper(t) return t:upper() end -- Lua: ok p/ ASCII, limitado p/ UTF8
function TEXT.to_lower(t) return t:lower() end

function TEXT.trim(t)
    return t:gsub("^%s*(.-)%s*$", "%1")
end

function TEXT.capitalize(t)
    if #t == 0 then return t end
    -- Pega o primeiro char (podendo ser multibyte)
    local first, rest = t:match("^([%z\1-\127\194-\244][\128-\191]*)(.*)$")
    return (first or ""):upper() .. (rest or "")
end

function TEXT.reverse(t)
    local runes = {}
    for rune in t:gmatch("[%z\1-\127\194-\244][\128-\191]*") do
        table.insert(runes, 1, rune)
    end
    return table.concat(runes)
end

function TEXT.clean_accents(t)
    local res = t:gsub("[%z\1-\127\194-\244][\128-\191]*", function(c)
        return accent_map[c] or c
    end)
    return res
end

function TEXT.slugify(t)
    local s = TEXT.clean_accents(t):lower()
    s = s:gsub("[%s%p]+", "-") -- troca espaços e pontuação por hifen
    s = s:gsub("^%-+", ""):gsub("%-+$", "") -- remove hifens das pontas
    return s
end

function TEXT.to_snake_case(t)
    local s = t:gsub("(%l)(%u)", "%1_%2"):gsub("[%s%-]+", "_"):lower()
    return s
end

function TEXT.to_camel_case(t)
    local s = TEXT.to_snake_case(t)
    return s:gsub("(_%l)", function(v) return v:sub(2):upper() end)
end

-- 🔎 BUSCA E PREDICADOS

function TEXT.count_chars(t)
    local _, count = t:gsub("[%z\1-\127\194-\244][\128-\191]*", "")
    return count
end

function TEXT.contains(t, search) return t:find(search, 1, true) ~= nil end
function TEXT.starts_with(t, prefix) return t:sub(1, #prefix) == prefix end
function TEXT.ends_with(t, suffix) return t:sub(-#suffix) == suffix end

function TEXT.is_numeric(t) return t:match("^%d+$") ~= nil end

function TEXT.is_emoji(t)
    -- Heurística simples: caracteres de 4 bytes costumam ser emojis
    for rune in t:gmatch("[\240-\244][\128-\191]*") do
        return true
    end
    return false
end

-- 🔪 DIVISÃO E UNIÃO

function TEXT.split(t, sep)
    local parts = {}
    local pattern = string.format("([^%s]+)", sep)
    t:gsub(pattern, function(c) table.insert(parts, c) end)
    return parts
end

function TEXT.join(parts, sep)
    return table.concat(parts, sep)
end

function TEXT.lines(t)
    local l = {}
    for line in t:gmatch("([^\n\r]+)") do table.insert(l, line) end
    return l
end

function TEXT.slice(t, start_idx, end_idx)
    return t:sub(start_idx or 1, end_idx or -1)
end

-- 🛡️ SEGURANÇA E UI

function TEXT.mask(t, v_start, v_end, char)
    char = char or "*"
    local len = TEXT.count_chars(t)
    if len <= (v_start + v_end) then return t end
    
    local runes = {}
    for rune in t:gmatch("[%z\1-\127\194-\244][\128-\191]*") do table.insert(runes, rune) end
    
    local res = ""
    for i=1, #runes do
        if i <= v_start or i > (#runes - v_end) then
            res = res .. runes[i]
        else
            res = res .. char
        end
    end
    return res
end

function TEXT.truncate(t, max_len, suffix)
    suffix = suffix or "..."
    local len = TEXT.count_chars(t)
    if len <= max_len then return t end
    
    local count = 0
    local res = ""
    for rune in t:gmatch("[%z\1-\127\194-\244][\128-\191]*") do
        res = res .. rune
        count = count + 1
        if count >= max_len then break end
    end
    return res .. suffix
end

function TEXT.format(fmt, args)
    if type(args) == "table" then
        return string.format(fmt, unpack(args))
    else
        return string.format(fmt, args)
    end
end

return TEXT

end

package.preload['src.source.span'] = function()
-- ============================================================================
-- Zenith Compiler — Span
-- Representa um intervalo de texto no código-fonte.
-- ============================================================================

local Span = {}
Span.__index = Span

--- Cria um novo Span.
--- @param start number Offset de início (1-indexed)
--- @param length number Comprimento em bytes
--- @return table
function Span.new(start, length)
    local self = setmetatable({}, Span)
    self.start = start
    self.length = length
    return self
end

--- Retorna o offset final (exclusivo).
--- @return number
function Span:finish()
    return self.start + self.length
end

--- Verifica se o span contém um offset.
--- @param offset number
--- @return boolean
function Span:contains(offset)
    return offset >= self.start and offset < self:finish()
end

--- Cria um span que engloba dois spans.
--- @param other table Outro Span
--- @return table
function Span:merge(other)
    local new_start = math.min(self.start, other.start)
    local new_finish = math.max(self:finish(), other:finish())
    return Span.new(new_start, new_finish - new_start)
end

--- Cria um span vazio na posição dada.
--- @param position number
--- @return table
function Span.empty(position)
    return Span.new(position, 0)
end

--- Representação textual para debug.
--- @return string
function Span:__tostring()
    return string.format("Span(%d, %d)", self.start, self.length)
end

return Span

end

package.preload["src.src.source.span"] = package.preload["src.source.span"]
package.preload["src/source/span"] = package.preload["src.source.span"]
package.preload["src/src/source/span"] = package.preload["src.source.span"]
package.preload["src.source.span.init"] = package.preload["src.source.span"]
package.preload["src.source.span/init"] = package.preload["src.source.span"]
package.preload['source'] = function()
return {
    SourceText = require("src.source.source_text")
}

end

package.preload["src.source"] = package.preload["source"]
package.preload["source"] = package.preload["source"]
package.preload["src/source"] = package.preload["source"]
package.preload["source.init"] = package.preload["source"]
package.preload["source/init"] = package.preload["source"]
package.preload['src.source.line_map'] = function()
-- ============================================================================
-- Zenith Compiler — Line Map
-- Mapeia offsets de byte para linha/coluna.
-- Pré-calcula as posições de início de cada linha para lookup O(log n).
-- ============================================================================

local Location = require("src.source.location")

local LineMap = {}
LineMap.__index = LineMap

--- Cria um novo LineMap a partir do texto-fonte.
--- @param source string Texto-fonte completo
--- @return table
function LineMap.new(source)
    local self = setmetatable({}, LineMap)
    self.line_starts = {}

    -- A primeira linha sempre começa no offset 1
    table.insert(self.line_starts, 1)

    for i = 1, #source do
        if source:sub(i, i) == "\n" then
            table.insert(self.line_starts, i + 1)
        end
    end

    return self
end

--- Retorna a Location (linha, coluna) para um dado offset.
--- Usa busca binária para performance.
--- @param offset number Offset no texto (1-indexed)
--- @return table Location
function LineMap:get_location(offset)
    local line = self:_find_line(offset)
    local column = offset - self.line_starts[line] + 1
    return Location.new(line, column)
end

--- Retorna o número total de linhas.
--- @return number
function LineMap:line_count()
    return #self.line_starts
end

--- Retorna o offset de início de uma linha.
--- @param line number Número da linha (1-indexed)
--- @return number
function LineMap:line_start(line)
    return self.line_starts[line] or self.line_starts[#self.line_starts]
end

--- Busca binária para encontrar a linha de um offset.
--- @param offset number
--- @return number Número da linha (1-indexed)
function LineMap:_find_line(offset)
    local starts = self.line_starts
    local lo, hi = 1, #starts

    while lo < hi do
        local mid = math.floor((lo + hi + 1) / 2)
        if starts[mid] <= offset then
            lo = mid
        else
            hi = mid - 1
        end
    end

    return lo
end

return LineMap

end

package.preload["src.src.source.line_map"] = package.preload["src.source.line_map"]
package.preload["src/source/line_map"] = package.preload["src.source.line_map"]
package.preload["src/src/source/line_map"] = package.preload["src.source.line_map"]
package.preload["src.source.line_map.init"] = package.preload["src.source.line_map"]
package.preload["src.source.line_map/init"] = package.preload["src.source.line_map"]
package.preload['syntax.ast.syntax_node'] = function()
-- ============================================================================
-- Zenith Compiler — Syntax Node
-- Tipo-base de todos os nós da AST.
-- Cada nó tem: kind, span e children opcionais.
-- ============================================================================

local SyntaxNode = {}
SyntaxNode.__index = SyntaxNode

--- Cria um novo nó da AST.
--- @param kind string SyntaxKind
--- @param data table Dados específicos do nó
--- @param span table Span no código-fonte
--- @return table
function SyntaxNode.new(kind, data, span)
    local self = setmetatable({}, SyntaxNode)
    self.kind = kind
    self.span = span

    -- Copiar dados do nó
    if data then
        for k, v in pairs(data) do
            self[k] = v
        end
    end

    return self
end

--- Representação textual para debug (sem recursão).
--- @return string
function SyntaxNode:__tostring()
    return string.format("Node(%s)", self.kind)
end

return SyntaxNode

end

package.preload["src.syntax.ast.syntax_node"] = package.preload["syntax.ast.syntax_node"]
package.preload["syntax/ast/syntax_node"] = package.preload["syntax.ast.syntax_node"]
package.preload["src/syntax/ast/syntax_node"] = package.preload["syntax.ast.syntax_node"]
package.preload["syntax.ast.syntax_node.init"] = package.preload["syntax.ast.syntax_node"]
package.preload["syntax.ast.syntax_node/init"] = package.preload["syntax.ast.syntax_node"]
package.preload['source.source_text'] = function()
-- ============================================================================
-- Zenith Compiler — Source Text
-- Representa o texto-fonte de um arquivo .zt
-- Ponto único de acesso ao conteúdo do arquivo.
-- ============================================================================

local Span = require("src.source.span")
local LineMap = require("src.source.line_map")

local SourceText = {}
SourceText.__index = SourceText

--- Cria um novo SourceText a partir de um conteúdo textual.
--- @param text string Conteúdo do arquivo
--- @param filename string|nil Nome do arquivo (para mensagens de erro)
--- @return table
function SourceText.new(text, filename)
    local self = setmetatable({}, SourceText)
    self.text = text
    self.filename = filename or "<stdin>"
    self.length = #text
    self.line_map = LineMap.new(text)
    return self
end

--- Carrega um SourceText a partir de um arquivo.
--- @param filepath string Caminho do arquivo
--- @return table|nil SourceText ou nil se erro
--- @return string|nil Mensagem de erro
function SourceText.from_file(filepath)
    local file, err = io.open(filepath, "r")
    if not file then
        return nil, string.format("não foi possível abrir '%s': %s", filepath, err)
    end

    local content = file:read("*a")
    file:close()

    return SourceText.new(content, filepath)
end

--- Retorna o caractere no offset dado.
--- @param offset number Offset (1-indexed)
--- @return string Caractere único ou string vazia se fora de range
function SourceText:char_at(offset)
    if offset < 1 or offset > self.length then
        return ""
    end
    return self.text:sub(offset, offset)
end

--- Retorna uma substring baseada em um Span.
--- @param span table Span
--- @return string
function SourceText:get_text(span)
    return self.text:sub(span.start, span:finish() - 1)
end

--- Retorna a Location (linha, coluna) para um offset.
--- @param offset number
--- @return table Location
function SourceText:get_location(offset)
    return self.line_map:get_location(offset)
end

--- Retorna o conteúdo de uma linha inteira.
--- @param line_number number Número da linha (1-indexed)
--- @return string
function SourceText:get_line(line_number)
    local start = self.line_map:line_start(line_number)
    local next_start
    if line_number < self.line_map:line_count() then
        next_start = self.line_map:line_start(line_number + 1)
    else
        next_start = self.length + 1
    end

    local line = self.text:sub(start, next_start - 1)
    -- Remove trailing newline
    line = line:gsub("[\r\n]+$", "")
    return line
end

--- Retorna o número total de linhas.
--- @return number
function SourceText:total_lines()
    return self.line_map:line_count()
end

return SourceText

end

package.preload["src.source.source_text"] = package.preload["source.source_text"]
package.preload["source/source_text"] = package.preload["source.source_text"]
package.preload["src/source/source_text"] = package.preload["source.source_text"]
package.preload["source.source_text.init"] = package.preload["source.source_text"]
package.preload["source.source_text/init"] = package.preload["source.source_text"]
package.preload['syntax.ast.expr_syntax'] = function()
-- ============================================================================
-- Zenith Compiler — Expression Syntax
-- Factory functions para nós de expressão da AST.
-- ============================================================================

local SyntaxNode = require("src.syntax.ast.syntax_node")
local SK = require("src.syntax.ast.syntax_kind")

local ExprSyntax = {}

--- Literal: 10, 3.14, "text", true, false, null
function ExprSyntax.literal(value, literal_type, span)
    return SyntaxNode.new(SK.LITERAL_EXPR, {
        value = value,
        literal_type = literal_type, -- "int", "float", "text", "bool", "null"
    }, span)
end

--- Identificador: nome de variável
function ExprSyntax.identifier(name, span)
    return SyntaxNode.new(SK.IDENTIFIER_EXPR, {
        name = name,
    }, span)
end

--- Expressão binária: a + b, a == b, a and b
function ExprSyntax.binary(left, operator, right, span)
    return SyntaxNode.new(SK.BINARY_EXPR, {
        left = left,
        operator = operator,  -- Token do operador
        right = right,
    }, span)
end

--- Expressão unária (prefixo): not x, -x
function ExprSyntax.unary(operator, operand, span)
    return SyntaxNode.new(SK.UNARY_EXPR, {
        operator = operator,
        operand = operand,
    }, span)
end

--- Chamada de função: func<T>(args)
function ExprSyntax.call(callee, arguments, span, generic_args)
    return SyntaxNode.new(SK.CALL_EXPR, {
        callee = callee,
        arguments = arguments,  -- lista de expressões
        generic_args = generic_args, -- lista de type_nodes ou nil
    }, span)
end

--- Acesso a membro: obj.campo ou obj?.campo
function ExprSyntax.member(object, member_name, span, is_safe)
    return SyntaxNode.new(SK.MEMBER_EXPR, {
        object = object,
        member_name = member_name,  -- string
        is_safe = is_safe or false,
    }, span)
end

--- Acesso por índice: list[i]
function ExprSyntax.index(object, index_expr, span)
    return SyntaxNode.new(SK.INDEX_EXPR, {
        object = object,
        index_expr = index_expr,
    }, span)
end

--- Range: 0..5
function ExprSyntax.range(start_expr, end_expr, span)
    return SyntaxNode.new(SK.RANGE_EXPR, {
        start_expr = start_expr,
        end_expr = end_expr,
    }, span)
end

--- Expressão agrupada: (expr)
function ExprSyntax.group(expression, span)
    return SyntaxNode.new(SK.GROUP_EXPR, {
        expression = expression,
    }, span)
end

--- Lista: [a, b, c]
function ExprSyntax.list(elements, span)
    return SyntaxNode.new(SK.LIST_EXPR, {
        elements = elements,
    }, span)
end

--- Map: { "k": v, ... }
function ExprSyntax.map(pairs, span)
    return SyntaxNode.new(SK.MAP_EXPR, {
        pairs = pairs,  -- lista de { key = expr, value = expr }
    }, span)
end

--- Self: self
function ExprSyntax.self_ref(span)
    return SyntaxNode.new(SK.SELF_EXPR, {}, span)
end

--- It: it
function ExprSyntax.it_ref(span)
    return SyntaxNode.new(SK.IT_EXPR, {}, span)
end

--- Self field sugar: .field → self.field
function ExprSyntax.self_field(field_name, span)
    return SyntaxNode.new(SK.SELF_FIELD_EXPR, {
        field_name = field_name,
    }, span)
end

--- Struct init: Type { field: value }
function ExprSyntax.struct_init(type_name, fields, span)
    return SyntaxNode.new(SK.STRUCT_INIT_EXPR, {
        type_name = type_name,
        fields = fields,  -- lista de { name = string, value = expr }
    }, span)
end

--- Operador Bang (!)
function ExprSyntax.bang(expression, span)
    return SyntaxNode.new(SK.BANG_EXPR, {
        expression = expression,
    }, span)
end

--- Operador Rest/Spread (..)
function ExprSyntax.rest(expression, span)
    return SyntaxNode.new(SK.REST_EXPR, {
        expression = expression,
    }, span)
end

--- Operador de Tamanho (#)
function ExprSyntax.len(expression, span)
    return SyntaxNode.new(SK.LEN_EXPR, {
        expression = expression,
    }, span)
end


--- Lambdas: (params) => body
function ExprSyntax.lambda(params, body, is_async, span)
    return SyntaxNode.new(SK.LAMBDA_EXPR, {
        params = params,   -- lista de PARAM_NODE
        body = body,       -- expressão ou lista de statements (se usar do...end)
        is_async = is_async or false,
    }, span)
end

--- Await: await expr
function ExprSyntax.await_expr(expression, span)
    return SyntaxNode.new(SK.AWAIT_EXPR, {
        expression = expression,
    }, span)
end

--- Is: expr is Type
function ExprSyntax.is_expr(expression, type_node, is_not, span)
    return SyntaxNode.new(SK.IS_EXPR, {
        expression = expression,
        type_node = type_node,
        is_not = is_not or false,
    }, span)
end

--- As: expr as Type
function ExprSyntax.as_expr(expression, type_node, span)
    return SyntaxNode.new(SK.AS_EXPR, {
        expression = expression,
        type_node = type_node,
    }, span)
end

--- Check: check expr
function ExprSyntax.check_expr(expression, span)
    return SyntaxNode.new(SK.CHECK_EXPR, {
        expression = expression,
    }, span)
end

--- Try: expr?
function ExprSyntax.try(expression, span)
    return SyntaxNode.new(SK.TRY_EXPR, {
        expression = expression,
    }, span)
end

function ExprSyntax.native_lua(lua_code, span)
    return SyntaxNode.new(SK.NATIVE_LUA_EXPR, {
        lua_code = lua_code,
    }, span)
end

return ExprSyntax

end

package.preload["src.syntax.ast.expr_syntax"] = package.preload["syntax.ast.expr_syntax"]
package.preload["syntax/ast/expr_syntax"] = package.preload["syntax.ast.expr_syntax"]
package.preload["src/syntax/ast/expr_syntax"] = package.preload["syntax.ast.expr_syntax"]
package.preload["syntax.ast.expr_syntax.init"] = package.preload["syntax.ast.expr_syntax"]
package.preload["syntax.ast.expr_syntax/init"] = package.preload["syntax.ast.expr_syntax"]
package.preload['semantic.symbols.symbol'] = function()
-- ============================================================================
-- Zenith Compiler — Symbol
-- Representa um símbolo no modelo semântico: variável, função, struct, etc.
-- ============================================================================

local Symbol = {}
Symbol.__index = Symbol

--- Tipos de símbolo.
Symbol.Kind = {
    VARIABLE  = "VARIABLE",
    CONSTANT  = "CONSTANT",
    GLOBAL    = "GLOBAL",
    STATE     = "STATE",
    COMPUTED  = "COMPUTED",
    FUNCTION  = "FUNCTION",
    PARAMETER = "PARAMETER",
    STRUCT    = "STRUCT",
    ENUM      = "ENUM",
    TRAIT     = "TRAIT",
    MODULE    = "MODULE",
    NAMESPACE = "NAMESPACE",
    FIELD     = "FIELD",
    METHOD    = "METHOD",
    ENUM_MEMBER = "ENUM_MEMBER",
    FOR_VAR   = "FOR_VAR",       -- variável de loop
    RESCUE_VAR = "RESCUE_VAR",   -- variável de rescue
    ALIAS      = "ALIAS",        -- type Name = T ou union Name = A | B
    GENERIC_PARAM = "GENERIC_PARAM", -- Parâmetro genérico <T>
}

--- Cria um novo símbolo.
--- @param kind string Symbol.Kind
--- @param name string Nome do símbolo
--- @param data table Dados específicos do símbolo
--- @return table
function Symbol.new(kind, name, data)
    local self = setmetatable({}, Symbol)
    self.kind = kind
    self.name = name
    self.type_info = data and data.type_info or nil  -- ZenithType
    self.span = data and data.span or nil
    self.is_mutable = data and data.is_mutable or false
    self.is_pub = data and data.is_pub or false
    self.declaration = data and data.declaration or nil  -- nó AST original

    -- Dados específicos por tipo de símbolo
    if data then
        for k, v in pairs(data) do
            if not self[k] then
                self[k] = v
            end
        end
    end

    return self
end

--- Cria símbolo de variável.
function Symbol.variable(name, type_info, is_pub, span)
    return Symbol.new(Symbol.Kind.VARIABLE, name, {
        type_info = type_info,
        span = span,
        is_mutable = true,
        is_pub = is_pub or false,
    })
end

--- Cria símbolo de constante.
function Symbol.constant(name, type_info, is_pub, span)
    return Symbol.new(Symbol.Kind.CONSTANT, name, {
        type_info = type_info,
        span = span,
        is_mutable = false,
        is_pub = is_pub or false,
    })
end

--- Cria símbolo global.
function Symbol.global_var(name, type_info, is_pub, span)
    return Symbol.new(Symbol.Kind.GLOBAL, name, {
        type_info = type_info,
        span = span,
        is_mutable = true,
        is_pub = is_pub or false,
    })
end

--- Cria símbolo de parâmetro.
function Symbol.parameter(name, type_info, default_value, span)
    return Symbol.new(Symbol.Kind.PARAMETER, name, {
        type_info = type_info,
        default_value = default_value, -- expressão AST
        span = span,
        is_mutable = false,
    })
end

--- Cria símbolo de função.
function Symbol.func(name, params, return_type, span, is_async)
    local ZenithType = require("src.semantic.types.zenith_type")
    return Symbol.new(Symbol.Kind.FUNCTION, name, {
        params = params,           -- lista de { name, type_info }
        return_type = return_type,  -- ZenithType ou nil
        is_async = is_async or false,
        type_info = ZenithType.new(ZenithType.Kind.FUNC, name, { 
            return_type = return_type,
            params = params 
        }),
        span = span,
        is_mutable = false,
    })
end

--- Cria símbolo de struct.
function Symbol.struct(name, fields, methods, span)
    return Symbol.new(Symbol.Kind.STRUCT, name, {
        fields = fields or {},
        methods = methods or {},
        implements = {}, -- lista de trait symbols
        span = span,
    })
end

--- Cria símbolo de campo.
function Symbol.field(name, type_info, is_pub, default_value, span)
    return Symbol.new(Symbol.Kind.FIELD, name, {
        type_info = type_info,
        is_pub = is_pub,
        default_value = default_value,
        span = span,
    })
end

--- Cria símbolo de enum.
function Symbol.enum(name, members, span)
    return Symbol.new(Symbol.Kind.ENUM, name, {
        members = members,
        span = span,
    })
end

--- Cria símbolo de membro de enum.
function Symbol.enum_member(name, parent_enum, params, span)
    return Symbol.new(Symbol.Kind.ENUM_MEMBER, name, {
        parent_enum = parent_enum, -- símbolo do Enum pai
        params = params,           -- lista de { name, type_info } (para Sum Types)
        span = span,
    })
end

--- Cria símbolo de trait.
function Symbol.trait(name, methods, span, generic_params)
    return Symbol.new(Symbol.Kind.TRAIT, name, {
        methods = methods,
        span = span,
        generic_params = generic_params or {},
    })
end

--- Cria símbolo de state.
function Symbol.state_var(name, type_info, span)
    return Symbol.new(Symbol.Kind.STATE, name, {
        type_info = type_info,
        span = span,
        is_mutable = true,
    })
end

--- Cria símbolo de computed.
function Symbol.computed(name, type_info, span)
    return Symbol.new(Symbol.Kind.COMPUTED, name, {
        type_info = type_info,
        span = span,
        is_mutable = false, 
    })
end

--- Cria símbolo de apelido de tipo (type/union).
function Symbol.alias(name, target_type, is_pub, span)
    return Symbol.new(Symbol.Kind.ALIAS, name, {
        type_info = target_type, -- O tipo para o qual ele aponta
        is_pub = is_pub or false,
        span = span,
    })
end

--- Cria símbolo de parâmetro genérico <T where T is Constraint>
function Symbol.generic_param(name, constraint, span)
    return Symbol.new(Symbol.Kind.GENERIC_PARAM, name, {
        type_info = nil, -- Será preenchido na instanciação
        constraint = constraint, -- ZenithType ou nil
        span = span,
    })
end

--- Cria símbolo de variável de loop.
function Symbol.for_var(name, type_info, span)
    return Symbol.new(Symbol.Kind.FOR_VAR, name, {
        type_info = type_info,
        span = span,
        is_mutable = false,
    })
end

--- Busca um membro (campo ou método) neste símbolo (se for STRUCT).
function Symbol:get_member(name)
    if self.kind == Symbol.Kind.ENUM then
        if self.member_symbols then
            for _, m in ipairs(self.member_symbols) do
                if m.name == name then return m end
            end
        end
        return nil
    end

    if self.kind ~= Symbol.Kind.STRUCT and self.kind ~= Symbol.Kind.TRAIT then return nil end
    
    if self.members_scope then
        local sym = self.members_scope:lookup_local(name)
        if sym then return sym end
    end

    if self.fields then
        for _, f in ipairs(self.fields) do
            if f.name == name then return f end
        end
    end
    
    if self.methods or self.members then
        local members = self.methods or self.members
        for _, m in ipairs(members) do
            if m.name == name then return m end
        end
    end
    
    return nil
end

--- Verifica se o símbolo é mutável.
function Symbol:is_reassignable()
    return self.kind == Symbol.Kind.VARIABLE
        or self.kind == Symbol.Kind.GLOBAL
        or self.kind == Symbol.Kind.STATE
end

--- Representação textual.
function Symbol:__tostring()
    local t = self.type_info and tostring(self.type_info) or "?"
    return string.format("Symbol(%s, %s: %s)", self.kind, self.name, t)
end

--- Cria símbolo de alias (type Name = T).
function Symbol.alias(name, target_node, is_pub, span)
    return Symbol.new(Symbol.Kind.ALIAS, name, {
        target_node = target_node,
        is_pub = is_pub or false,
        span = span,
    })
end

--- Cria símbolo de parâmetro genérico (<T>).
function Symbol.generic_param(name, constraint, span)
    return Symbol.new(Symbol.Kind.GENERIC_PARAM, name, {
        constraint = constraint,
        span = span,
    })
end

return Symbol

end

package.preload["src.semantic.symbols.symbol"] = package.preload["semantic.symbols.symbol"]
package.preload["semantic/symbols/symbol"] = package.preload["semantic.symbols.symbol"]
package.preload["src/semantic/symbols/symbol"] = package.preload["semantic.symbols.symbol"]
package.preload["semantic.symbols.symbol.init"] = package.preload["semantic.symbols.symbol"]
package.preload["semantic.symbols.symbol/init"] = package.preload["semantic.symbols.symbol"]
package.preload['syntax.ast.decl_syntax'] = function()
-- ============================================================================
-- Zenith Compiler — Declaration Syntax
-- Factory functions para nós de declaração da AST.
-- ============================================================================

local SyntaxNode = require("src.syntax.ast.syntax_node")
local SK = require("src.syntax.ast.syntax_kind")

local DeclSyntax = {}

--- var name: Type = value
function DeclSyntax.var_decl(name, type_node, initializer, is_pub, span)
    return SyntaxNode.new(SK.VAR_DECL, {
        name = name,
        type_node = type_node,
        initializer = initializer,
        is_pub = is_pub or false,
    }, span)
end

--- const NAME: Type = value
function DeclSyntax.const_decl(name, type_node, initializer, is_pub, span)
    return SyntaxNode.new(SK.CONST_DECL, {
        name = name,
        type_node = type_node,
        initializer = initializer,
        is_pub = is_pub or false,
    }, span)
end

--- global NAME: Type = value
function DeclSyntax.global_decl(name, type_node, initializer, is_pub, span)
    return SyntaxNode.new(SK.GLOBAL_DECL, {
        name = name,
        type_node = type_node,
        initializer = initializer,
        is_pub = is_pub or false,
    }, span)
end

--- state name: Type = value
function DeclSyntax.state_decl(name, type_node, initializer, is_pub, span)
    return SyntaxNode.new(SK.STATE_DECL, {
        name = name,
        type_node = type_node,
        initializer = initializer,
        is_pub = is_pub or false,
    }, span)
end

--- computed name: Type = expression
function DeclSyntax.computed_decl(name, type_node, expression, is_pub, span)
    return SyntaxNode.new(SK.COMPUTED_DECL, {
        name = name,
        type_node = type_node,
        expression = expression,
        is_pub = is_pub or false,
    }, span)
end

--- func name(params) -> ReturnType ... end
function DeclSyntax.func_decl(name, params, return_type, body, is_pub, span, generic_params)
    return SyntaxNode.new(SK.FUNC_DECL, {
        name = name,
        params = params,          -- lista de PARAM_NODE
        return_type = return_type, -- type_node ou nil (void)
        body = body,              -- lista de statements
        is_pub = is_pub or false,
        generic_params = generic_params,
    }, span)
end

--- async func name(params) -> ReturnType ... end
function DeclSyntax.async_func_decl(name, params, return_type, body, is_pub, span, generic_params)
    return SyntaxNode.new(SK.ASYNC_FUNC_DECL, {
        name = name,
        params = params,
        return_type = return_type,
        body = body,
        is_pub = is_pub or false,
        is_async = true,
        generic_params = generic_params,
    }, span)
end

--- Declaração de Type Alias: type Name = Type
function DeclSyntax.type_alias_decl(name, target_type, is_pub, span, generic_params)
    return SyntaxNode.new(SK.TYPE_ALIAS_DECL, {
        name = name,
        target = target_type,
        is_pub = is_pub or false,
        generic_params = generic_params or {},
    }, span)
end

--- Declaração de Union: union Name = A | B
function DeclSyntax.union_decl(name, union_type, is_pub, span, generic_params)
    return SyntaxNode.new(SK.UNION_DECL, {
        name = name,
        union_type = union_type,
        is_pub = is_pub or false,
        generic_params = generic_params or {},
    }, span)
end

--- Nó de parâmetro: name: Type = default
function DeclSyntax.param_node(name, type_node, default_value, span)
    return SyntaxNode.new(SK.PARAM_NODE, {
        name = name,
        type_node = type_node,
        default_value = default_value, -- expressão ou nil
    }, span)
end

--- struct Name ... end
function DeclSyntax.struct_decl(name, fields, methods, is_pub, span, generic_params)
    return SyntaxNode.new(SK.STRUCT_DECL, {
        name = name,
        fields = fields,    -- lista de FIELD_NODE
        methods = methods,  -- lista de FUNC_DECL
        is_pub = is_pub or false,
        generic_params = generic_params, -- lista de {name, constraint}
    }, span)
end

--- Nó de campo: @attr pub? name: Type = default where condition
function DeclSyntax.field_node(name, type_node, default_value, is_pub, attributes, condition, span)
    return SyntaxNode.new(SK.FIELD_NODE, {
        name = name,
        type_node = type_node,
        default_value = default_value,
        is_pub = is_pub or false,
        attributes = attributes or {}, -- lista de ATTRIBUTE_NODE
        condition = condition,         -- expressão do 'where'
    }, span)
end

--- Nó de atributo: @name(args)
function DeclSyntax.attribute_node(name, arguments, span)
    return SyntaxNode.new(SK.ATTRIBUTE_NODE, {
        name = name,
        arguments = arguments or {}, -- lista de expressões
    }, span)
end

--- enum Name ... end
function DeclSyntax.enum_decl(name, members, is_pub, span, generic_params)
    return SyntaxNode.new(SK.ENUM_DECL, {
        name = name,
        members = members,  -- lista de ENUM_MEMBER_NODE
        is_pub = is_pub or false,
        generic_params = generic_params or {},
    }, span)
end

--- Membro de enum
function DeclSyntax.enum_member(name, value, params, span)
    return SyntaxNode.new(SK.ENUM_MEMBER_NODE, {
        name = name,
        value = value,  -- expressão ou nil
        params = params, -- Parâmetros para Sum Types
    }, span)
end

--- trait Name ... end
function DeclSyntax.trait_decl(name, methods, is_pub, span, generic_params)
    return SyntaxNode.new(SK.TRAIT_DECL, {
        name = name,
        methods = methods,
        is_pub = is_pub or false,
        generic_params = generic_params or {},
    }, span)
end

--- apply Trait to Struct ... end
function DeclSyntax.apply_decl(trait_name, struct_name, methods, span, generic_args)
    return SyntaxNode.new(SK.APPLY_DECL, {
        trait_name = trait_name,
        struct_name = struct_name,
        methods = methods, -- lista de FUNC_DECL
        generic_args = generic_args or {},
    }, span)
end

--- import "path" / import std.module
function DeclSyntax.import_decl(path, alias, span)
    return SyntaxNode.new(SK.IMPORT_DECL, {
        path = path,
        alias = alias,  -- string ou nil
    }, span)
end

--- export func ...
function DeclSyntax.export_decl(declaration, span)
    return SyntaxNode.new(SK.EXPORT_DECL, {
        declaration = declaration,
    }, span)
end

--- redo func Type.method(params) ... end
function DeclSyntax.redo_decl(type_name, func_decl, span)
    return SyntaxNode.new(SK.REDO_DECL, {
        type_name = type_name,
        func_decl = func_decl,
    }, span)
end

--- extern func name(params) -> ReturnType
function DeclSyntax.extern_decl(name, params, return_type, is_pub, span)
    return SyntaxNode.new(SK.EXTERN_DECL, {
        name = name,
        params = params,
        return_type = return_type,
        is_pub = is_pub or false,
    }, span)
end

--- group "name" ... end (testes)
function DeclSyntax.group_decl(name, body, span)
    return SyntaxNode.new(SK.GROUP_DECL, {
        name = name,
        body = body,
    }, span)
end

--- test "name" ... end (testes)
function DeclSyntax.test_decl(name, body, span)
    return SyntaxNode.new(SK.TEST_DECL, {
        name = name,
        body = body,
    }, span)
end

--- Cria um nó de declaração de namespace.
--- @param name string Nome do namespace (pode ser pontuado)
--- @param span table Span
function DeclSyntax.namespace_decl(name, span)
    return SyntaxNode.new(SK.NAMESPACE_DECL, {
        name = name,
    }, span)
end

--- Cria a unidade de compilação (arquivo).
--- @param declarations table Lista de declarações
--- @param span table Span
function DeclSyntax.compilation_unit(declarations, span)
    return SyntaxNode.new(SK.COMPILATION_UNIT, {
        declarations = declarations,
    }, span)
end

return DeclSyntax

end

package.preload["src.syntax.ast.decl_syntax"] = package.preload["syntax.ast.decl_syntax"]
package.preload["syntax/ast/decl_syntax"] = package.preload["syntax.ast.decl_syntax"]
package.preload["src/syntax/ast/decl_syntax"] = package.preload["syntax.ast.decl_syntax"]
package.preload["syntax.ast.decl_syntax.init"] = package.preload["syntax.ast.decl_syntax"]
package.preload["syntax.ast.decl_syntax/init"] = package.preload["syntax.ast.decl_syntax"]
package.preload['syntax.parser'] = function()
return require("syntax.parser.parser")

end

package.preload["src.syntax.parser"] = package.preload["syntax.parser"]
package.preload["syntax/parser"] = package.preload["syntax.parser"]
package.preload["src/syntax/parser"] = package.preload["syntax.parser"]
package.preload["syntax.parser.init"] = package.preload["syntax.parser"]
package.preload["syntax.parser/init"] = package.preload["syntax.parser"]
package.preload['syntax.parser.parse_statements'] = function()
-- ============================================================================
-- Zenith Compiler — Statement Parser
-- Funções para parsing de statements (if, while, match, etc).
-- ============================================================================

local StmtSyntax = require("src.syntax.ast.stmt_syntax")
local ExprSyntax = require("src.syntax.ast.expr_syntax")
local TokenKind = require("src.syntax.tokens.token_kind")
local ParseExpressions = require("src.syntax.parser.parse_expressions")

local ParseStatements = {}

-- Lista de tokens que abrem blocos que OBRIGATORIAMENTE terminam com KW_END
local block_openers = {
    [TokenKind.KW_IF] = true,
    [TokenKind.KW_WHILE] = true,
    [TokenKind.KW_FOR] = true,
    [TokenKind.KW_REPEAT] = true,
    [TokenKind.KW_MATCH] = true,
    [TokenKind.KW_ATTEMPT] = true,
    [TokenKind.KW_WATCH] = true,
    [TokenKind.KW_GROUP] = true,
    [TokenKind.KW_TEST] = true,
    [TokenKind.KW_STRUCT] = true,
    [TokenKind.KW_ENUM] = true,
    [TokenKind.KW_TRAIT] = true,
    [TokenKind.KW_APPLY] = true,
    [TokenKind.KW_FUNC] = true,
    [TokenKind.KW_ASYNC] = true,
}

function ParseStatements.parse_statement(ctx)
    local k = ctx:peek().kind
    
    if k == TokenKind.KW_IF then
        return ParseStatements._parse_if(ctx)
    elseif k == TokenKind.KW_WHILE then
        return ParseStatements._parse_while(ctx)
    elseif k == TokenKind.KW_FOR then
        return ParseStatements._parse_for(ctx)
    elseif k == TokenKind.KW_REPEAT then
        return ParseStatements._parse_repeat(ctx)
    elseif k == TokenKind.KW_RETURN then
        return ParseStatements._parse_return(ctx)
    elseif k == TokenKind.KW_BREAK then
        return StmtSyntax.break_stmt(ctx:advance().span)
    elseif k == TokenKind.KW_CONTINUE then
        return StmtSyntax.continue_stmt(ctx:advance().span)
    elseif k == TokenKind.KW_MATCH then
        return ParseStatements._parse_match(ctx)
    elseif k == TokenKind.KW_ATTEMPT then
        return ParseStatements._parse_attempt(ctx)
    elseif k == TokenKind.KW_WATCH then
        return ParseStatements._parse_watch(ctx)
    elseif k == TokenKind.KW_THROW then
        return ParseStatements._parse_throw(ctx)
    elseif k == TokenKind.KW_CHECK then
        return ParseStatements._parse_check(ctx)
    elseif k == TokenKind.KW_AFTER then
        return ParseStatements._parse_after(ctx)
    elseif k == TokenKind.KW_NATIVE then
        return ParseStatements._parse_native_lua(ctx, true)
    end

    -- Expression Statement ou Atribuição
    local expr = ParseExpressions.parse_expression(ctx)
    
    -- Atribuição composta
    if ctx:match(TokenKind.PLUS_EQUAL) or ctx:match(TokenKind.MINUS_EQUAL) or
       ctx:match(TokenKind.STAR_EQUAL) or ctx:match(TokenKind.SLASH_EQUAL) then
        local op = ctx:peek(-1)
        local value = ParseExpressions.parse_expression(ctx)
        return StmtSyntax.compound_assign(expr, op, value, expr.span:merge(value.span))
    end
    
    -- Atribuição simples
    if ctx:match(TokenKind.EQUAL) then
        local value = ParseExpressions.parse_expression(ctx)
        return StmtSyntax.assign(expr, value, expr.span:merge(value.span))
    end

    return StmtSyntax.expr_stmt(expr, expr.span)
end

function ParseStatements._parse_attempt(ctx)
    local start = ctx:advance() -- 'attempt'
    ctx:skip_newlines()
    local body = ParseStatements.parse_block(ctx, {TokenKind.KW_RESCUE, TokenKind.KW_END})
    
    local rescue_clause = nil
    if ctx:match(TokenKind.KW_RESCUE) then
        local err_name = "e"
        if ctx:check(TokenKind.IDENTIFIER) then
            err_name = ctx:advance().lexeme
        end
        
        local err_type = nil
        if ctx:match(TokenKind.COLON) then
            local ParseTypes = require("src.syntax.parser.parse_types")
            err_type = ParseTypes.parse_type(ctx)
        end
        
        ctx:skip_newlines()
        local rescue_body = ParseStatements.parse_block(ctx, TokenKind.KW_END)
        rescue_clause = StmtSyntax.rescue_clause(err_name, err_type, rescue_body, start.span)
    end
    
    local end_token = ctx:expect(TokenKind.KW_END, "esperado 'end' após attempt")
    return StmtSyntax.attempt_stmt(body, rescue_clause, start.span:merge(end_token.span))
end

function ParseStatements._parse_watch(ctx)
    local start = ctx:advance() -- 'watch'
    
    local target = nil
    -- Se o próximo token não for uma nova linha nem o início de um bloco, tenta parsear expressão
    if not ctx:check(TokenKind.NEWLINE) and not ctx:check(TokenKind.KW_END) then
        target = ParseExpressions.parse_expression(ctx)
    end
    
    ctx:skip_newlines()
    local body = ParseStatements.parse_block(ctx, TokenKind.KW_END)
    local end_token = ctx:expect(TokenKind.KW_END, "esperado 'end' após watch")
    return StmtSyntax.watch_stmt(target, body, start.span:merge(end_token.span))
end

function ParseStatements.parse_block(ctx, end_kind)
    local statements = {}

    local function is_end(k)
        if type(end_kind) == "table" then
            for _, ek in ipairs(end_kind) do
                if k == ek then return true end
            end
            return false
        else
            return k == end_kind
        end
    end

    while not ctx:is_at_end() do
        ctx:skip_newlines()
        local k = ctx:peek().kind
        if is_end(k) then break end

        local ParseDeclarations = require("src.syntax.parser.parse_declarations")
        local stmt = ParseDeclarations.parse_declaration_or_statement(ctx)
        if stmt then table.insert(statements, stmt) end
        ctx:skip_newlines()
    end
    return statements
end

function ParseStatements._parse_if(ctx)
    local start = ctx:advance() -- 'if'
    local condition = ParseExpressions.parse_expression(ctx)
    ctx:skip_newlines()
    
    local body = ParseStatements.parse_block(ctx, { TokenKind.KW_ELIF, TokenKind.KW_ELSE, TokenKind.KW_END })
    
    local elif_clauses = {}
    while ctx:match(TokenKind.KW_ELIF) do
        local elif_cond = ParseExpressions.parse_expression(ctx)
        ctx:skip_newlines()
        local elif_body = ParseStatements.parse_block(ctx, { TokenKind.KW_ELIF, TokenKind.KW_ELSE, TokenKind.KW_END })
        table.insert(elif_clauses, { condition = elif_cond, body = elif_body })
    end
    
    local else_clause = nil
    if ctx:match(TokenKind.KW_ELSE) then
        ctx:skip_newlines()
        local else_body = ParseStatements.parse_block(ctx, TokenKind.KW_END)
        else_clause = { body = else_body }
    end
    
    local end_t = ctx:expect(TokenKind.KW_END, "esperado 'end' após if")
    return StmtSyntax.if_stmt(condition, body, elif_clauses, else_clause, start.span:merge(end_t.span))
end

function ParseStatements._parse_while(ctx)
    local start = ctx:advance() -- 'while'
    local condition = ParseExpressions.parse_expression(ctx)
    ctx:skip_newlines()
    local body = ParseStatements.parse_block(ctx, TokenKind.KW_END)
    local end_t = ctx:expect(TokenKind.KW_END, "esperado 'end' após while")
    return StmtSyntax.while_stmt(condition, body, start.span:merge(end_t.span))
end

function ParseStatements._parse_repeat(ctx)
    local start = ctx:advance() -- 'repeat'
    local count_expr = ParseExpressions.parse_expression(ctx)
    ctx:expect(TokenKind.KW_TIMES, "esperado 'times' após expressão no repeat")
    ctx:skip_newlines()
    local body = ParseStatements.parse_block(ctx, TokenKind.KW_END)
    local end_t = ctx:expect(TokenKind.KW_END, "esperado 'end' após repeat-times")
    return StmtSyntax.repeat_times(count_expr, body, start.span:merge(end_t.span))
end

function ParseStatements._parse_for(ctx)
    local start = ctx:advance() -- 'for'
    
    local variables = {}
    repeat
        local id = ctx:expect(TokenKind.IDENTIFIER, "esperado nome de variável no for-in")
        table.insert(variables, { name = id.lexeme, span = id.span })
    until not ctx:match(TokenKind.COMMA)
    
    ctx:expect(TokenKind.KW_IN, "esperado 'in' após variáveis no for")
    local iterable = ParseExpressions.parse_expression(ctx)
    ctx:skip_newlines()
    
    local body = ParseStatements.parse_block(ctx, TokenKind.KW_END)
    local end_t = ctx:expect(TokenKind.KW_END, "esperado 'end' após for")
    return StmtSyntax.for_in(variables, iterable, body, start.span:merge(end_t.span))
end

function ParseStatements._parse_return(ctx)
    local start = ctx:advance() -- 'return'
    local value = nil
    if not ctx:check(TokenKind.NEWLINE) and not ctx:check(TokenKind.KW_END) and 
       not ctx:check(TokenKind.KW_CASE) and not ctx:check(TokenKind.KW_ELSE) and not ctx:is_at_end() then
        value = ParseExpressions.parse_expression(ctx)
    end
    return StmtSyntax.return_stmt(value, start.span:merge(value and value.span or start.span))
end

function ParseStatements._parse_match(ctx)
    local start = ctx:advance() -- 'match'
    local expr = ParseExpressions.parse_expression(ctx)
    ctx:skip_newlines()
    
    local cases = {}
    local else_clause = nil
    
    while not ctx:check(TokenKind.KW_END) and not ctx:is_at_end() do
        ctx:skip_newlines()
        if ctx:match(TokenKind.KW_CASE) then
            local patterns = {}
            repeat
                local ParseDeclarations = require("src.syntax.parser.parse_declarations")
                table.insert(patterns, ParseDeclarations._parse_pattern(ctx, false))
            until not ctx:match(TokenKind.COMMA)
            
            if not ctx:match(TokenKind.COLON) and not ctx:match(TokenKind.FAT_ARROW) then
                ctx:expect(TokenKind.COLON, "esperado ':' ou '=>' após padrão do case")
            end
            ctx:skip_newlines()
            
            local body = ParseStatements.parse_block(ctx, { TokenKind.KW_CASE, TokenKind.KW_ELSE, TokenKind.KW_END })
            table.insert(cases, StmtSyntax.match_case(patterns, body, patterns[1].span:merge(ctx:peek(-1).span)))
        elseif ctx:match(TokenKind.KW_ELSE) then
            ctx:match(TokenKind.COLON) -- Opcional no else
            ctx:skip_newlines()
            local else_body = ParseStatements.parse_block(ctx, TokenKind.KW_END)
            else_clause = StmtSyntax.else_clause(else_body, ctx:peek(-1).span)
            break
        else
            if not ctx:check(TokenKind.KW_END) then ctx:advance() else break end
        end
        ctx:skip_newlines()
    end
    
    local end_t = ctx:expect(TokenKind.KW_END, "esperado 'end' para fechar match")
    return StmtSyntax.match_stmt(expr, cases, else_clause, start.span:merge(end_t.span))
end

function ParseStatements._parse_throw(ctx)
    local start = ctx:advance() -- 'throw'
    local expr = ParseExpressions.parse_expression(ctx)
    return StmtSyntax.throw_stmt(expr, start.span:merge(expr.span))
end

function ParseStatements._parse_check(ctx)
    local start = ctx:advance() -- 'check'
    local condition = ParseExpressions.parse_expression(ctx)
    
    local else_body = {}
    if ctx:match(TokenKind.KW_ELSE) then
        ctx:skip_newlines()
        else_body = ParseStatements.parse_block(ctx, TokenKind.KW_END)
    end
    
    local end_t = ctx:expect(TokenKind.KW_END, "esperado 'end' para fechar check")
    
    return StmtSyntax.check_stmt(condition, else_body, start.span:merge(end_t.span))
end

function ParseStatements._parse_after(ctx)
    local start = ctx:advance() -- 'after'
    local body
    if ctx:match(TokenKind.KW_DO) then
        body = ParseStatements.parse_block(ctx, TokenKind.KW_END)
        local end_t = ctx:expect(TokenKind.KW_END, "esperado 'end' após block no after")
        return StmtSyntax.after_stmt(body, start.span:merge(end_t.span))
    else
        local stmt = ParseStatements.parse_statement(ctx)
        return StmtSyntax.after_stmt({stmt}, start.span:merge(stmt.span))
    end
end

function ParseStatements._parse_native_lua(ctx, is_stmt)
    local start = ctx:advance() -- 'native'
    ctx:expect(TokenKind.IDENTIFIER, "esperado 'lua' após 'native'")
    
    local content = {}
    local depth = 1
    
    while not ctx:is_at_end() do
        local t = ctx:peek()
        if block_openers[t.kind] or (t.kind == TokenKind.IDENTIFIER and t.lexeme == "function") then
            depth = depth + 1
        elseif t.kind == TokenKind.KW_END then
            depth = depth - 1
            if depth == 0 then break end
        end
        
        table.insert(content, ctx:advance().lexeme)
        table.insert(content, " ")
    end
    
    local end_t = ctx:expect(TokenKind.KW_END, "esperado 'end' após bloco native lua")
    local lua_code = table.concat(content)
    
    if is_stmt then
        return StmtSyntax.native_lua(lua_code, start.span:merge(end_t.span))
    else
        return ExprSyntax.native_lua(lua_code, start.span:merge(end_t.span))
    end
end

return ParseStatements

end

package.preload["src.syntax.parser.parse_statements"] = package.preload["syntax.parser.parse_statements"]
package.preload["syntax/parser/parse_statements"] = package.preload["syntax.parser.parse_statements"]
package.preload["src/syntax/parser/parse_statements"] = package.preload["syntax.parser.parse_statements"]
package.preload["syntax.parser.parse_statements.init"] = package.preload["syntax.parser.parse_statements"]
package.preload["syntax.parser.parse_statements/init"] = package.preload["syntax.parser.parse_statements"]
package.preload['src.source.location'] = function()
-- ============================================================================
-- Zenith Compiler — Location
-- Representa linha e coluna derivadas de um Span.
-- ============================================================================

local Location = {}
Location.__index = Location

--- Cria uma nova Location.
--- @param line number Linha (1-indexed)
--- @param column number Coluna (1-indexed)
--- @return table
function Location.new(line, column)
    local self = setmetatable({}, Location)
    self.line = line
    self.column = column
    return self
end

--- Representação textual para erros e debug.
--- @return string
function Location:__tostring()
    return string.format("%d:%d", self.line, self.column)
end

return Location

end

package.preload["src.src.source.location"] = package.preload["src.source.location"]
package.preload["src/source/location"] = package.preload["src.source.location"]
package.preload["src/src/source/location"] = package.preload["src.source.location"]
package.preload["src.source.location.init"] = package.preload["src.source.location"]
package.preload["src.source.location/init"] = package.preload["src.source.location"]
package.preload['diagnostics.diagnostic_bag'] = function()
-- ============================================================================
-- Zenith Compiler — Diagnostic Bag
-- Acumula diagnósticos de todas as fases do compilador.
-- ============================================================================

local Diagnostic = require("src.diagnostics.diagnostic")

local DiagnosticBag = {}
DiagnosticBag.__index = DiagnosticBag

--- Cria um novo DiagnosticBag.
--- @return table
function DiagnosticBag.new()
    local self = setmetatable({}, DiagnosticBag)
    self.diagnostics = {}
    return self
end

--- Adiciona um diagnóstico.
--- @param diagnostic table
function DiagnosticBag:add(diagnostic)
    table.insert(self.diagnostics, diagnostic)
end

--- Reporta um erro.
--- @param code string
--- @param message string
--- @param span table
--- @param hint string|nil
function DiagnosticBag:report_error(code, message, span, hint)
    self:add(Diagnostic.error(code, message, span, hint))
end

--- Reporta um aviso.
--- @param code string
--- @param message string
--- @param span table
--- @param hint string|nil
function DiagnosticBag:report_warning(code, message, span, hint)
    self:add(Diagnostic.warning(code, message, span, hint))
end

--- Reporta uma dica.
--- @param code string
--- @param message string
--- @param span table
--- @param hint string|nil
function DiagnosticBag:report_hint(code, message, span, hint)
    self:add(Diagnostic.hint(code, message, span, hint))
end

--- Verifica se há erros.
--- @return boolean
function DiagnosticBag:has_errors()
    for _, d in ipairs(self.diagnostics) do
        if d:is_error() then
            return true
        end
    end
    return false
end

--- Retorna apenas os erros.
--- @return table
function DiagnosticBag:errors()
    local result = {}
    for _, d in ipairs(self.diagnostics) do
        if d:is_error() then
            table.insert(result, d)
        end
    end
    return result
end

--- Retorna o total de diagnósticos.
--- @return number
function DiagnosticBag:count()
    return #self.diagnostics
end

--- Mescla outro DiagnosticBag neste.
--- @param other table Outro DiagnosticBag
function DiagnosticBag:merge(other)
    for _, d in ipairs(other.diagnostics) do
        table.insert(self.diagnostics, d)
    end
end

--- Formata todos os diagnósticos com contexto de código.
--- @param source_text table SourceText para extrair linhas
--- @return string
function DiagnosticBag:format(source_text)
    local parts = {}

    for _, d in ipairs(self.diagnostics) do
        local loc = source_text:get_location(d.span.start)
        local line_content = source_text:get_line(loc.line)

        local header = string.format("%s %s [%s]: %s",
            d:icon(), d:label(), d.code, d.message)

        local location_line = string.format("  --> %s:%d:%d",
            source_text.filename, loc.line, loc.column)

        local gutter_width = #tostring(loc.line)
        local padding = string.rep(" ", gutter_width)

        local code_line = string.format(" %s |", padding)
        local source_line = string.format(" %d | %s", loc.line, line_content)

        -- Ponteiro para o span
        local pointer_padding = string.rep(" ", loc.column - 1)
        local pointer = string.rep("^", math.max(1, d.span.length))
        local pointer_line = string.format(" %s | %s%s",
            padding, pointer_padding, pointer)

        local entry = header .. "\n" .. location_line .. "\n"
            .. code_line .. "\n" .. source_line .. "\n" .. pointer_line

        if d.hint then
            entry = entry .. string.format("\n💡 Dica: %s", tostring(d.hint))
        end

        table.insert(parts, entry)
    end

    return table.concat(parts, "\n\n")
end

return DiagnosticBag

end

package.preload["src.diagnostics.diagnostic_bag"] = package.preload["diagnostics.diagnostic_bag"]
package.preload["diagnostics/diagnostic_bag"] = package.preload["diagnostics.diagnostic_bag"]
package.preload["src/diagnostics/diagnostic_bag"] = package.preload["diagnostics.diagnostic_bag"]
package.preload["diagnostics.diagnostic_bag.init"] = package.preload["diagnostics.diagnostic_bag"]
package.preload["diagnostics.diagnostic_bag/init"] = package.preload["diagnostics.diagnostic_bag"]
package.preload['semantic.types.type_checker'] = function()
-- ============================================================================
-- Zenith Compiler — Type Checker
-- Lógica central de validação de tipos e operações.
-- ============================================================================

local BuiltinTypes = require("src.semantic.types.builtin_types")
local ZenithType    = require("src.semantic.types.zenith_type")

local TypeChecker = {}

--- Verifica se um tipo é atribuível a outro.
--- @param source table ZenithType (o valor fornecido)
--- @param target table ZenithType (o tipo esperado)
--- @return boolean
function TypeChecker.is_assignable(source, target)
    -- Caso base: os mesmos tipos ou sentinelas de erro
    if source == target or source.kind == ZenithType.Kind.ERROR or target.kind == ZenithType.Kind.ERROR then
        return true
    end
    
    -- "any" no alvo ou na origem aceita tudo (pragmatismo para interoperabilidade e blocos native)
    if target.name == "any" or target == BuiltinTypes.ANY or source.name == "any" or source == BuiltinTypes.ANY then
        return true
    end
    
    -- Delega para a lógica estruturada do ZenithType
    return source:is_assignable_to(target)
end

--- Retorna o tipo resultante de uma operação binária.
--- @param left table ZenithType
--- @param operator string Lexema do operador (+, -, etc)
--- @param right table ZenithType
--- @return table ZenithType
function TypeChecker.get_binary_result(left, operator, right)
    -- Heurística ANY
    if left == BuiltinTypes.ANY or right == BuiltinTypes.ANY then
        return BuiltinTypes.ANY
    end

    -- Multi-uso do '+' : Matemática e Concatenação
    if operator == "+" then
        -- Concatenação de texto: qualquer coisa que contenha text
        if left == BuiltinTypes.TEXT or right == BuiltinTypes.TEXT then
            return BuiltinTypes.TEXT
        end
        -- Matemática
        if left == BuiltinTypes.INT and right == BuiltinTypes.INT then
            return BuiltinTypes.INT
        end
        if (left == BuiltinTypes.INT or left == BuiltinTypes.FLOAT) and 
           (right == BuiltinTypes.INT or right == BuiltinTypes.FLOAT) then
            return BuiltinTypes.FLOAT
        end
    end
    
    -- Comparação e Lógica: Sempre retorna bool
    local bool_ops = { 
        ["=="]=1, ["!="]=1, ["<"]=1, ["<="]=1, [">"]=1, [">="]=1,
        ["and"]=1, ["or"]=1
    }
    if bool_ops[operator] then
        return BuiltinTypes.BOOL
    end
    
    -- Aritmética básica
    local arithmetic = { ["-"]=1, ["*"]=1, ["/"]=1, ["%"]=1, ["^"]=1 }
    if arithmetic[operator] then
        if left == BuiltinTypes.FLOAT or right == BuiltinTypes.FLOAT then
            return BuiltinTypes.FLOAT
        end
        return BuiltinTypes.INT
    end

    return BuiltinTypes.ERROR
end

--- Retorna o tipo resultante de uma operação unária.
function TypeChecker.get_unary_result(operator, operand)
    if operand == BuiltinTypes.ANY then return BuiltinTypes.ANY end
    
    if operator == "not" then
        return BuiltinTypes.BOOL
    end
    if operator == "-" then
        if operand == BuiltinTypes.INT or operand == BuiltinTypes.FLOAT then
            return operand
        end
    end
    if operator == "!" then
        -- O operador bang (!) remove a nulabilidade ou indica erro provável (v1.0-alpha)
        if operand.kind == ZenithType.Kind.NULLABLE then
            return operand.base_type
        end
        return operand
    end
    return BuiltinTypes.ERROR
end

return TypeChecker

end

package.preload["src.semantic.types.type_checker"] = package.preload["semantic.types.type_checker"]
package.preload["semantic/types/type_checker"] = package.preload["semantic.types.type_checker"]
package.preload["src/semantic/types/type_checker"] = package.preload["semantic.types.type_checker"]
package.preload["semantic.types.type_checker.init"] = package.preload["semantic.types.type_checker"]
package.preload["semantic.types.type_checker/init"] = package.preload["semantic.types.type_checker"]
package.preload['semantic.types.builtin_types'] = function()
-- ============================================================================
-- Zenith Compiler — Builtin Types
-- Tipos primitivos da linguagem (int, float, bool, text, void, any, null).
-- ============================================================================

local ZenithType = require("src.semantic.types.zenith_type")

local BuiltinTypes = {}

-- Primitivos
BuiltinTypes.INT   = ZenithType.new(ZenithType.Kind.PRIMITIVE, "int")
BuiltinTypes.FLOAT = ZenithType.new(ZenithType.Kind.PRIMITIVE, "float")
BuiltinTypes.BOOL  = ZenithType.new(ZenithType.Kind.PRIMITIVE, "bool")
BuiltinTypes.TEXT  = ZenithType.new(ZenithType.Kind.PRIMITIVE, "text")
BuiltinTypes.VOID  = ZenithType.new(ZenithType.Kind.PRIMITIVE, "void")
BuiltinTypes.ANY   = ZenithType.new(ZenithType.Kind.PRIMITIVE, "any")
BuiltinTypes.NULL  = ZenithType.new(ZenithType.Kind.NULL,      "null")
BuiltinTypes.ERROR = ZenithType.new(ZenithType.Kind.ERROR,     "<error>")

--- Mapa para lookup rápido no parser/binder por nome.
BuiltinTypes.map = {
    ["int"]   = BuiltinTypes.INT,
    ["float"] = BuiltinTypes.FLOAT,
    ["bool"]  = BuiltinTypes.BOOL,
    ["text"]  = BuiltinTypes.TEXT,
    ["void"]  = BuiltinTypes.VOID,
    ["any"]   = BuiltinTypes.ANY,
    ["null"]  = BuiltinTypes.NULL,

    -- Bases Genéricas (para servir de lookup inicial)
    ["list"]     = ZenithType.new(ZenithType.Kind.GENERIC, "list",     { base_name = "list",     type_args = {} }),
    ["grid"]     = ZenithType.new(ZenithType.Kind.GENERIC, "grid",     { base_name = "grid",     type_args = {} }),
    ["map"]      = ZenithType.new(ZenithType.Kind.GENERIC, "map",      { base_name = "map",      type_args = {} }),
    ["Outcome"]  = ZenithType.new(ZenithType.Kind.GENERIC, "Outcome",  { base_name = "Outcome",  type_args = {} }),
    ["Optional"] = ZenithType.new(ZenithType.Kind.GENERIC, "Optional", { base_name = "Optional", type_args = {} }),
}

--- Retorna o tipo pelo nome ou nil.
function BuiltinTypes.lookup(name)
    return BuiltinTypes.map[name]
end

return BuiltinTypes

end

package.preload["src.semantic.types.builtin_types"] = package.preload["semantic.types.builtin_types"]
package.preload["semantic/types/builtin_types"] = package.preload["semantic.types.builtin_types"]
package.preload["src/semantic/types/builtin_types"] = package.preload["semantic.types.builtin_types"]
package.preload["semantic.types.builtin_types.init"] = package.preload["semantic.types.builtin_types"]
package.preload["semantic.types.builtin_types/init"] = package.preload["semantic.types.builtin_types"]
package.preload['semantic.symbols.scope'] = function()
-- ============================================================================
-- Zenith Compiler — Scope
-- Escopo léxico com tabela de símbolos e escopo pai.
-- ============================================================================

local Scope = {}
Scope.__index = Scope

--- Tipos de escopo.
Scope.Kind = {
    GLOBAL    = "GLOBAL",
    FUNCTION  = "FUNCTION",
    BLOCK     = "BLOCK",
    STRUCT    = "STRUCT",
    TRAIT     = "TRAIT",
    LOOP      = "LOOP",
    NAMESPACE = "NAMESPACE",
}

--- Cria um novo escopo.
--- @param kind string Scope.Kind
--- @param parent table|nil Escopo pai
--- @return table
function Scope.new(kind, parent)
    local self = setmetatable({}, Scope)
    self.kind = kind
    self.parent = parent
    self.symbols = {}          -- name → Symbol
    self.children = {}         -- lista de escopos filhos
    self.depth = parent and (parent.depth + 1) or 0

    if parent then
        table.insert(parent.children, self)
    end

    return self
end

--- Define um símbolo no escopo atual.
--- @param symbol table Symbol
--- @return boolean true se ok, false se já existe
function Scope:define(symbol)
    if self.symbols[symbol.name] then
        return false  -- redefinição
    end
    self.symbols[symbol.name] = symbol
    return true
end

--- Busca um símbolo pelo nome no escopo atual.
--- @param name string
--- @return table|nil Symbol
function Scope:lookup_local(name)
    return self.symbols[name]
end

--- Busca um símbolo pelo nome, subindo pela cadeia de escopos.
--- @param name string
--- @return table|nil Symbol
function Scope:lookup(name)
    local symbol = self.symbols[name]
    if symbol then
        return symbol
    end
    if self.parent then
        return self.parent:lookup(name)
    end
    return nil
end

--- Verifica se estamos dentro de um loop (para validar break/continue).
--- @return boolean
function Scope:is_in_loop()
    if self.kind == Scope.Kind.LOOP then
        return true
    end
    if self.parent then
        return self.parent:is_in_loop()
    end
    return false
end

--- Verifica se estamos dentro de uma função (para validar return).
--- @return boolean
function Scope:is_in_function()
    if self.kind == Scope.Kind.FUNCTION then
        return true
    end
    if self.parent then
        return self.parent:is_in_function()
    end
    return false
end

--- Verifica se estamos dentro de um struct ou trait (para validar self).
--- @return boolean
function Scope:is_in_struct_or_trait()
    if self.kind == Scope.Kind.STRUCT or self.kind == Scope.Kind.TRAIT then
        return true
    end
    if self.parent then
        return self.parent:is_in_struct_or_trait()
    end
    return false
end

--- Retorna o nome de todos os símbolos (para sugestões de "você quis dizer?").
--- @return table Lista de nomes
function Scope:all_names()
    local names = {}
    local s = self
    while s do
        for name, _ in pairs(s.symbols) do
            table.insert(names, name)
        end
        s = s.parent
    end
    return names
end

--- Representação textual.
function Scope:__tostring()
    local count = 0
    for _ in pairs(self.symbols) do count = count + 1 end
    return string.format("Scope(%s, depth=%d, symbols=%d)",
        self.kind, self.depth, count)
end

return Scope

end

package.preload["src.semantic.symbols.scope"] = package.preload["semantic.symbols.scope"]
package.preload["semantic/symbols/scope"] = package.preload["semantic.symbols.scope"]
package.preload["src/semantic/symbols/scope"] = package.preload["semantic.symbols.scope"]
package.preload["semantic.symbols.scope.init"] = package.preload["semantic.symbols.scope"]
package.preload["semantic.symbols.scope/init"] = package.preload["semantic.symbols.scope"]
package.preload['semantic.symbols.prelude'] = function()
-- ============================================================================
-- Zenith Compiler — Prelude (Globais Embutidos)
-- Injeta as funções/constantes globais de Lua e do runtime Zenith no escopo
-- semântico. Permite que `print`, `tostring`, `type`, etc. sejam usados em
-- qualquer arquivo .zt sem import explícito.
-- ============================================================================

local Symbol       = require("src.semantic.symbols.symbol")
local BuiltinTypes = require("src.semantic.types.builtin_types")
local ZenithType   = require("src.semantic.types.zenith_type")

local Prelude = {}

-- Tipo função genérica (qualquer → any)
local function fn(name, params_types, ret_type)
    local params = {}
    for i, t in ipairs(params_types or {}) do
        table.insert(params, Symbol.variable("p" .. i, t, true, nil))
    end
    local sym = Symbol.func(name, params, ret_type or BuiltinTypes.ANY, nil)
    sym.is_builtin = true
    return sym
end

-- Tipo variável global (any)
local function global(name, type_info)
    local sym = Symbol.variable(name, type_info or BuiltinTypes.ANY, true, nil)
    sym.is_builtin = true
    return sym
end

-- ── Funções base de Lua disponíveis em Zenith ──────────────────────────────

Prelude.symbols = {
    -- I/O
    fn("print",      {BuiltinTypes.ANY}, BuiltinTypes.VOID),
    fn("io_write",   {BuiltinTypes.TEXT}, BuiltinTypes.VOID),

    -- Conversão
    fn("tostring",   {BuiltinTypes.ANY},  BuiltinTypes.TEXT),
    fn("tonumber",   {BuiltinTypes.ANY},  BuiltinTypes.FLOAT),
    fn("tostring",   {BuiltinTypes.ANY},  BuiltinTypes.TEXT),

    -- Tipo dinâmico
    fn("type",       {BuiltinTypes.ANY},  BuiltinTypes.TEXT),

    -- Tabelas Lua
    fn("ipairs",     {BuiltinTypes.ANY},  BuiltinTypes.ANY),
    fn("pairs",      {BuiltinTypes.ANY},  BuiltinTypes.ANY),
    fn("next",       {BuiltinTypes.ANY},  BuiltinTypes.ANY),
    fn("select",     {BuiltinTypes.ANY},  BuiltinTypes.ANY),
    fn("unpack",     {BuiltinTypes.ANY},  BuiltinTypes.ANY),
    fn("setmetatable", {BuiltinTypes.ANY, BuiltinTypes.ANY}, BuiltinTypes.ANY),
    fn("getmetatable", {BuiltinTypes.ANY}, BuiltinTypes.ANY),
    fn("rawget",     {BuiltinTypes.ANY, BuiltinTypes.ANY}, BuiltinTypes.ANY),
    fn("rawset",     {BuiltinTypes.ANY, BuiltinTypes.ANY, BuiltinTypes.ANY}, BuiltinTypes.ANY),

    -- Erros
    fn("error",      {BuiltinTypes.ANY},  BuiltinTypes.VOID),
    fn("assert",     {BuiltinTypes.BOOL, BuiltinTypes.ANY}, BuiltinTypes.ANY),
    fn("pcall",      {BuiltinTypes.ANY},  BuiltinTypes.ANY),
    fn("xpcall",     {BuiltinTypes.ANY, BuiltinTypes.ANY}, BuiltinTypes.ANY),

    -- Módulos
    fn("require",    {BuiltinTypes.TEXT}, BuiltinTypes.ANY),

    -- Strings
    fn("rawlen",     {BuiltinTypes.ANY},  BuiltinTypes.INT),

    -- Matemática
    fn("math_floor", {BuiltinTypes.FLOAT}, BuiltinTypes.INT),
    fn("math_ceil",  {BuiltinTypes.FLOAT}, BuiltinTypes.INT),
    fn("math_abs",   {BuiltinTypes.FLOAT}, BuiltinTypes.FLOAT),

    -- Utilitários
    fn("collectgarbage", {BuiltinTypes.TEXT}, BuiltinTypes.ANY),
    fn("load",       {BuiltinTypes.ANY},  BuiltinTypes.ANY),
    fn("loadstring", {BuiltinTypes.TEXT}, BuiltinTypes.ANY),
    fn("dofile",     {BuiltinTypes.TEXT}, BuiltinTypes.ANY),

    -- Globais constantes
    global("true",  BuiltinTypes.BOOL),
    global("false", BuiltinTypes.BOOL),
    global("nil",   BuiltinTypes.NULL),
    global("arg",   BuiltinTypes.ANY),
    global("_G",    BuiltinTypes.ANY),
    global("_VERSION", BuiltinTypes.TEXT),

    -- Runtime Zenith (zt.*) — alias de conveniência
    fn("zt_add",    {BuiltinTypes.ANY, BuiltinTypes.ANY}, BuiltinTypes.ANY),

    -- ── Outcome<T> — construtores de resultado ─────────────────────────────
    fn("Success",       {BuiltinTypes.ANY}, BuiltinTypes.ANY),
    fn("Failure",       {BuiltinTypes.ANY}, BuiltinTypes.ANY),

    -- ── Optional<T> — construtores de valor opcional ────────────────────────
    fn("Present",       {BuiltinTypes.ANY}, BuiltinTypes.ANY),
    global("Empty",     BuiltinTypes.ANY),

    -- ── Funções de suporte ao operador ? ────────────────────────────────────
    fn("zt_run_fallible", {BuiltinTypes.ANY}, BuiltinTypes.ANY),
}

--- Injeta os símbolos do prelude num escopo fornecido.
--- @param scope Scope — escopo raiz onde os símbolos devem ser registrados
function Prelude.inject(scope)
    for _, sym in ipairs(Prelude.symbols) do
        -- Não falha se já definido (globais podem ser redefinidos via import)
        scope:define(sym)
    end
end

return Prelude

end

package.preload["src.semantic.symbols.prelude"] = package.preload["semantic.symbols.prelude"]
package.preload["semantic/symbols/prelude"] = package.preload["semantic.symbols.prelude"]
package.preload["src/semantic/symbols/prelude"] = package.preload["semantic.symbols.prelude"]
package.preload["semantic.symbols.prelude.init"] = package.preload["semantic.symbols.prelude"]
package.preload["semantic.symbols.prelude/init"] = package.preload["semantic.symbols.prelude"]
package.preload['syntax.lexer'] = function()
return require("syntax.lexer.lexer")

end

package.preload["src.syntax.lexer"] = package.preload["syntax.lexer"]
package.preload["syntax/lexer"] = package.preload["syntax.lexer"]
package.preload["src/syntax/lexer"] = package.preload["syntax.lexer"]
package.preload["syntax.lexer.init"] = package.preload["syntax.lexer"]
package.preload["syntax.lexer/init"] = package.preload["syntax.lexer"]
package.preload['semantic.types.zenith_type'] = function()
-- ============================================================================
-- Zenith Compiler — Zenith Type
-- Representação de tipos no modelo semântico.
-- ============================================================================

local ZenithType = {}
ZenithType.__index = ZenithType

 ZenithType.Kind = {
    PRIMITIVE = "PRIMITIVE", -- int, float, bool, text, void
    STRUCT    = "STRUCT",
    ENUM      = "ENUM",
    TRAIT     = "TRAIT",
    FUNC      = "FUNC",
    NULLABLE  = "NULLABLE",
    UNION     = "UNION",
    GENERIC   = "GENERIC",
    NULL      = "NULL",
    TYPE_PARAM = "TYPE_PARAM",
    ERROR     = "ERROR",     -- Tipo sentinela para erros de compilação
}

--- Cria um novo ZenithType.
function ZenithType.new(kind, name, data)
    local self = setmetatable({}, ZenithType)
    self.kind = kind
    self.name = name
    
    if data then
        for k, v in pairs(data) do
            self[k] = v
        end
    end
    
    return self
end

--- Cria uma união de tipos, achatando uniões aninhadas.
function ZenithType.create_union(types)
    local flattened = {}
    local seen = {}
    
    local function collect(t)
        if not t then return end
        if t.kind == ZenithType.Kind.UNION then
            for _, sub in ipairs(t.types) do collect(sub) end
        else
            local key = tostring(t)
            if not seen[key] then
                table.insert(flattened, t)
                seen[key] = true
            end
        end
    end
    
    for _, t in ipairs(types) do collect(t) end
    
    if #flattened == 0 then return ZenithType.new(ZenithType.Kind.ERROR, "<error>") end
    if #flattened == 1 then return flattened[1] end
    
    return ZenithType.new(ZenithType.Kind.UNION, "union", { types = flattened })
end

--- Verifica se este tipo é compatível com outro.
function ZenithType:is_assignable_to(other)
    if self.kind == ZenithType.Kind.ERROR or other.kind == ZenithType.Kind.ERROR then
        return true -- Evita cascata de erros
    end

    -- Se um dos lados for ANY, aceitamos (pragmatismo)
    if self.name == "any" or other.name == "any" then
        return true
    end

    if self == other then return true end

    -- Parâmetros genéricos ainda não instanciados são tratados como placeholders.
    if self.kind == ZenithType.Kind.TYPE_PARAM or other.kind == ZenithType.Kind.TYPE_PARAM then
        return true
    end

    -- Lógica de Traits: Struct S -> Trait T
    if other.kind == ZenithType.Kind.TRAIT then
        if self.kind == ZenithType.Kind.STRUCT then
            local struct_sym = self.symbol
            if struct_sym and struct_sym.implements then
                for _, trait_sym in ipairs(struct_sym.implements) do
                    if trait_sym.name == other.name then return true end
                end
            end
        end
        return false
    end

    -- Lógica de Nullable: T -> T? ou null -> T?
    if other.kind == ZenithType.Kind.NULLABLE then
        if self.kind == ZenithType.Kind.NULL then return true end
        return self:is_assignable_to(other.base_type)
    end

    -- Lógica de União (Origem): A | B -> T
    if self.kind == ZenithType.Kind.UNION then
        for _, t in ipairs(self.types) do
            if not t:is_assignable_to(other) then return false end
        end
        return true
    end

    -- Lógica de União (Alvo): T -> A | B
    if other.kind == ZenithType.Kind.UNION then
        for _, t in ipairs(other.types) do
            if self:is_assignable_to(t) then return true end
        end
        return false
    end

    -- Lógica de Genéricos: list<int> -> list<int>
    if self.kind == ZenithType.Kind.GENERIC and other.kind == ZenithType.Kind.GENERIC then
        if self.base_name ~= other.base_name then return false end
        if #self.type_args ~= #other.type_args then return false end
        for i = 1, #self.type_args do
            -- Invariância por padrão para Zenith v1.0-alpha
            if not self.type_args[i]:is_assignable_to(other.type_args[i]) or
               not other.type_args[i]:is_assignable_to(self.type_args[i]) then
                return false
            end
        end
        return true
    end

    -- Lógica de Funções: () => T -> () => T
    if self.kind == ZenithType.Kind.FUNC and other.kind == ZenithType.Kind.FUNC then
        -- Se um deles for ANY (sentinela), aceitamos
        if self.name == "any" or other.name == "any" then return true end

        -- Checar retorno (Covariante)
        local s_ret = self.return_type or BuiltinTypes.VOID
        local o_ret = other.return_type or BuiltinTypes.VOID
        
        if not s_ret:is_assignable_to(o_ret) then
            return false
        end
        -- Checar parâmetros (Contravariante)
        local s_params = self.params or {}
        local o_params = other.params or {}
        if #s_params ~= #o_params then return false end
        for i = 1, #s_params do
            local sp = s_params[i].type_info or s_params[i]
            local op = o_params[i].type_info or o_params[i]
            if not op:is_assignable_to(sp) then return false end
        end
        return true
    end

    -- Promoção automática: int -> float (Zenith é pragmático)
    local BuiltinTypes = require("src.semantic.types.builtin_types")
    if self == BuiltinTypes.INT and other == BuiltinTypes.FLOAT then
        return true
    end

    -- Fallback por nome para tipos primários e structs simples
    -- Se forem funções, a lógica estrutural acima já decidiu.
    if self.kind == ZenithType.Kind.FUNC and other.kind == ZenithType.Kind.FUNC then
        return false -- Se chegou aqui, falhou na lógica estrutural acima
    end

    return self.name == other.name and self.kind == other.kind
end

function ZenithType:__tostring()
    if self.kind == ZenithType.Kind.GENERIC then
        local args = {}
        for _, arg in ipairs(self.type_args or {}) do
            table.insert(args, tostring(arg))
        end
        return string.format("%s<%s>", self.base_name, table.concat(args, ", "))
    elseif self.kind == ZenithType.Kind.NULLABLE then
        return tostring(self.base_type) .. "?"
    elseif self.kind == ZenithType.Kind.UNION then
        local parts = {}
        for _, t in ipairs(self.types or {}) do
            table.insert(parts, tostring(t))
        end
        return table.concat(parts, " | ")
    end
    return self.name
end

return ZenithType

end

package.preload["src.semantic.types.zenith_type"] = package.preload["semantic.types.zenith_type"]
package.preload["semantic/types/zenith_type"] = package.preload["semantic.types.zenith_type"]
package.preload["src/semantic/types/zenith_type"] = package.preload["semantic.types.zenith_type"]
package.preload["semantic.types.zenith_type.init"] = package.preload["semantic.types.zenith_type"]
package.preload["semantic.types.zenith_type/init"] = package.preload["semantic.types.zenith_type"]
package.preload['syntax.ast.stmt_syntax'] = function()
-- ============================================================================
-- Zenith Compiler — Statement Syntax
-- Factory functions para nós de statement da AST.
-- ============================================================================

local SyntaxNode = require("src.syntax.ast.syntax_node")
local SK = require("src.syntax.ast.syntax_kind")

local StmtSyntax = {}

--- Atribuição: x = valor
function StmtSyntax.assign(target, value, span)
    return SyntaxNode.new(SK.ASSIGN_STMT, {
        target = target,  -- expressão (identifier, member, index)
        value = value,
    }, span)
end

--- Atribuição composta: x += valor
function StmtSyntax.compound_assign(target, operator, value, span)
    return SyntaxNode.new(SK.COMPOUND_ASSIGN_STMT, {
        target = target,
        operator = operator,  -- Token do operador (+=, -=, etc.)
        value = value,
    }, span)
end

--- If / elif / else / end
function StmtSyntax.if_stmt(condition, body, elif_clauses, else_clause, span)
    return SyntaxNode.new(SK.IF_STMT, {
        condition = condition,
        body = body,                -- lista de statements
        elif_clauses = elif_clauses or {},  -- lista de elif nodes
        else_clause = else_clause,  -- else node ou nil
    }, span)
end

--- Elif clause
function StmtSyntax.elif_clause(condition, body, span)
    return SyntaxNode.new(SK.ELIF_CLAUSE, {
        condition = condition,
        body = body,
    }, span)
end

--- Else clause
function StmtSyntax.else_clause(body, span)
    return SyntaxNode.new(SK.ELSE_CLAUSE, {
        body = body,
    }, span)
end

--- While / end
function StmtSyntax.while_stmt(condition, body, span)
    return SyntaxNode.new(SK.WHILE_STMT, {
        condition = condition,
        body = body,
    }, span)
end

--- For item in collection / end
--- For index, item in collection / end
function StmtSyntax.for_in(variables, iterable, body, span)
    return SyntaxNode.new(SK.FOR_IN_STMT, {
        variables = variables,  -- lista de { name = string, span = span }
        iterable = iterable,    -- expressão
        body = body,
    }, span)
end

--- Repeat N times / end
function StmtSyntax.repeat_times(count_expr, body, span)
    return SyntaxNode.new(SK.REPEAT_TIMES_STMT, {
        count_expr = count_expr,
        body = body,
    }, span)
end

--- Break
function StmtSyntax.break_stmt(span)
    return SyntaxNode.new(SK.BREAK_STMT, {}, span)
end

--- Continue
function StmtSyntax.continue_stmt(span)
    return SyntaxNode.new(SK.CONTINUE_STMT, {}, span)
end

--- Return (com expressão opcional)
function StmtSyntax.return_stmt(expression, span)
    return SyntaxNode.new(SK.RETURN_STMT, {
        expression = expression,  -- pode ser nil
    }, span)
end

--- Watch state: ... end
function StmtSyntax.watch_stmt(target, body, span)
    return SyntaxNode.new(SK.WATCH_STMT, {
        target = target,  -- nome do state
        body = body,
    }, span)
end

--- Attempt / rescue / end
function StmtSyntax.attempt_stmt(body, rescue_clause, span)
    return SyntaxNode.new(SK.ATTEMPT_STMT, {
        body = body,
        rescue_clause = rescue_clause,
    }, span)
end

--- Rescue clause
function StmtSyntax.rescue_clause(error_name, error_type, body, span)
    return SyntaxNode.new(SK.RESCUE_CLAUSE, {
        error_name = error_name,  -- string (nome da variável de erro)
        error_type = error_type,  -- nó de tipo (pode ser nil)
        body = body,
    }, span)
end

--- Expression statement (chamada de função como statement, etc.)
function StmtSyntax.expr_stmt(expression, span)
    return SyntaxNode.new(SK.EXPR_STMT, {
        expression = expression,
    }, span)
end

--- Assert statement
function StmtSyntax.assert_stmt(expression, span)
    return SyntaxNode.new(SK.ASSERT_STMT, {
        expression = expression,
    }, span)
end

--- Match statement
function StmtSyntax.match_stmt(expression, cases, else_clause, span)
    return SyntaxNode.new(SK.MATCH_STMT or "MATCH_STMT", {
        expression = expression,
        cases = cases,        -- lista de MATCH_CASE
        else_clause = else_clause,
    }, span)
end

--- Match case
function StmtSyntax.match_case(patterns, body, span)
    return SyntaxNode.new(SK.MATCH_CASE, {
        patterns = patterns,   -- lista de expressões
        body = body,
    }, span)
end

--- Throw statement
function StmtSyntax.throw_stmt(expression, span)
    return SyntaxNode.new(SK.THROW_STMT, {
        expression = expression,
    }, span)
end

function StmtSyntax.check_stmt(condition, else_body, span)
    return SyntaxNode.new(SK.CHECK_STMT, {
        condition = condition,
        else_body = else_body,
    }, span)
end

function StmtSyntax.after_stmt(body, span)
    return SyntaxNode.new(SK.AFTER_STMT, {
        body = body, -- lista de statements
    }, span)
end

function StmtSyntax.native_lua(lua_code, span)
    return SyntaxNode.new(SK.NATIVE_LUA_STMT, {
        lua_code = lua_code,
    }, span)
end

return StmtSyntax

end

package.preload["src.syntax.ast.stmt_syntax"] = package.preload["syntax.ast.stmt_syntax"]
package.preload["syntax/ast/stmt_syntax"] = package.preload["syntax.ast.stmt_syntax"]
package.preload["src/syntax/ast/stmt_syntax"] = package.preload["syntax.ast.stmt_syntax"]
package.preload["syntax.ast.stmt_syntax.init"] = package.preload["syntax.ast.stmt_syntax"]
package.preload["syntax.ast.stmt_syntax/init"] = package.preload["syntax.ast.stmt_syntax"]
package.preload['semantic.binding.binder'] = function()
-- ============================================================================
-- Zenith Compiler — Binder
-- Analisador Semântico: Resolve nomes e verifica tipos.
-- ============================================================================

local SK            = require("src.syntax.ast.syntax_kind")
local DeclSyntax    = require("src.syntax.ast.decl_syntax")
local ExprSyntax    = require("src.syntax.ast.expr_syntax")
local StmtSyntax    = require("src.syntax.ast.stmt_syntax")
local Symbol        = require("src.semantic.symbols.symbol")
local Scope         = require("src.semantic.symbols.scope")
local BuiltinTypes  = require("src.semantic.types.builtin_types")
local ZenithType    = require("src.semantic.types.zenith_type")
local TypeChecker   = require("src.semantic.types.type_checker")
local DiagnosticBag = require("src.diagnostics.diagnostic_bag")
local Prelude       = require("src.semantic.symbols.prelude")

local Binder = {}
Binder.__index = Binder

function Binder.new(diagnostics, module_manager, target_platform)
    local self = setmetatable({}, Binder)
    self.diagnostics = diagnostics or DiagnosticBag.new()
    self.module_manager = module_manager
    self.target_platform = target_platform or "windows"
    self.skip_prelude = false -- Flag para evitar recursão no std.core
    
    -- Escopo raiz inicial + Prelude (globais Lua e runtime Zenith)
    self.scope = Scope.new(Scope.Kind.GLOBAL, nil)
    Prelude.inject(self.scope)
    
    self.current_func = nil
    self.current_struct = nil
    self.current_alias = nil
    self.current_namespace = nil
    self.resolve_stack = {} -- Proteção contra recursão infinita em aliases
    return self
end

--- Ponto de entrada: resolve nomes e vincula tipos na CompilationUnit.
function Binder:bind(unit, module_name)
    -- Discovery: carrega prelude primeiro para popular o escopo global
    if not self.skip_prelude and self.module_manager then
        local prelude_scope = self.module_manager:get_prelude_scope()
        if prelude_scope then
            -- Copia símbolos da prelude para o escopo global para simplicidade
            for name, sym in pairs(prelude_scope.symbols) do
                self.scope:define(sym)
            end
        end
    end

    -- FILTRAGEM DE PLATAFORMA (v0.3 Ascension)
    local filtered_decls = {}
    for _, decl in ipairs(unit.declarations) do
        local keep = true
        if decl.attributes then
            local has_platform_attr = false
            local matches_platform = false
            for _, attr in ipairs(decl.attributes) do
                if attr.name == "windows" or attr.name == "linux" or attr.name == "macos" then
                    has_platform_attr = true
                    if attr.name == self.target_platform then
                        matches_platform = true
                    end
                end
            end
            if has_platform_attr and not matches_platform then
                keep = false
            end
        end
        if keep then table.insert(filtered_decls, decl) end
    end
    unit.declarations = filtered_decls

    -- Se não houver namespace explícito, usamos o nome do módulo/arquivo
    self.current_namespace = module_name
    
    -- Passagem 1: Registrar declarações globais e descobrir Namespace
    for _, decl in ipairs(unit.declarations) do
        if decl.kind == SK.NAMESPACE_DECL then
            self.current_namespace = decl.name
        end
    end
    
    -- Se tivermos um namespace, criamos um escopo para ele no GLOBAL
    if self.current_namespace then
        local ns_scope = Scope.new(Scope.Kind.NAMESPACE, self.scope)
        self.scope = ns_scope
    end

    -- Registrar declarações globais (Passagem 1)
    for _, decl in ipairs(unit.declarations) do
        self:_declare(decl)
    end

    -- Passagem 1.2: Vincular assinaturas de membros (Campos de Structs, Métodos de Traits, Aliases)
    -- Importante para que o 'apply' e 'generic constraints' conheçam a estrutura dos tipos.
    for _, decl in ipairs(unit.declarations) do
        if decl.kind == SK.STRUCT_DECL or decl.kind == SK.TRAIT_DECL or 
           decl.kind == SK.ENUM_DECL or decl.kind == SK.TYPE_ALIAS_DECL or 
           decl.kind == SK.UNION_DECL then
            self.only_signatures = true
            self:_bind_node(decl)
            self.only_signatures = false
        end
    end

    -- Passagem 1.5: Registrar implementações de Traits (apply)
    for _, decl in ipairs(unit.declarations) do
        if decl.kind == SK.APPLY_DECL then
            self:_bind_node(decl)
        end
    end

    -- Passagem ZDoc: Vincular comentários @target
    for _, doc in ipairs(unit.doc_comments or {}) do
        local sym = self.scope:lookup(doc.target)
        if sym then
            sym.doc_comment = doc.content
        end
    end
    
    -- Passagem 2: Verificar corpos de funções e inicializadores
    local last_type = BuiltinTypes.VOID
    for _, decl in ipairs(unit.declarations) do
        -- Pula Aliases e Uniões na passagem 2 (já tratados ou dinâmicos)
        if decl.kind == SK.TYPE_ALIAS_DECL or decl.kind == SK.UNION_DECL then
            -- Skip
        elseif decl.kind == SK.FUNC_DECL or decl.kind == SK.ASYNC_FUNC_DECL or 
           decl.kind == SK.VAR_DECL or decl.kind == SK.CONST_DECL or
           decl.kind == SK.GLOBAL_DECL or decl.kind == SK.STATE_DECL or
           decl.kind == SK.COMPUTED_DECL then
            last_type = self:_bind_node(decl)
        elseif decl.kind == SK.STRUCT_DECL then
            -- Agora vincula os corpos dos métodos da struct
            self.skip_fields = true
            last_type = self:_bind_node(decl)
            self.skip_fields = false
        end
    end
    
    -- Regista no manager se houver
    if self.module_manager and self.current_namespace then
        self.module_manager:register_scope(self.current_namespace, self.scope)
    end

    return self.scope, self.diagnostics, last_type
end

-- ============================================================================
-- Passagem 1 — Declaração (Coleta de Símbolos)
-- ============================================================================

function Binder:_declare(node)
    local symbol
    if node.kind == SK.FUNC_DECL or node.kind == SK.ASYNC_FUNC_DECL then
        local return_type = self:_resolve_type(node.return_type)
        symbol = Symbol.func(node.name, node.params, return_type, node.span)
        if node.kind == SK.ASYNC_FUNC_DECL then
            symbol.is_async = true
        end
        
    elseif node.kind == SK.STRUCT_DECL then
        symbol = Symbol.struct(node.name, {}, {}, node.span)
        symbol.type_info = ZenithType.new(ZenithType.Kind.STRUCT, node.name, { symbol = symbol })
    
    elseif node.kind == SK.ENUM_DECL then
        symbol = Symbol.enum(node.name, node.members, node.span)
        symbol.type_info = ZenithType.new(ZenithType.Kind.ENUM, node.name, { symbol = symbol })
        symbol.generic_params = node.generic_params or {}

    elseif node.kind == SK.TRAIT_DECL then
        symbol = Symbol.trait(node.name, node.methods, node.span, node.generic_params)
        symbol.type_info = ZenithType.new(ZenithType.Kind.TRAIT, node.name, { symbol = symbol })

    elseif node.kind == SK.TYPE_ALIAS_DECL or node.kind == SK.UNION_DECL then
        -- Criamos o símbolo vazio. A resolução real será na Passagem 1.2
        symbol = Symbol.alias(node.name, BuiltinTypes.ANY, node.is_pub, node.span)
        symbol.generic_params = node.generic_params

    elseif node.kind == SK.VAR_DECL or node.kind == SK.CONST_DECL or node.kind == SK.GLOBAL_DECL or
           node.kind == SK.STATE_DECL or node.kind == SK.COMPUTED_DECL then
        -- Não resolve o tipo ainda (será feito no _bind_var_decl na passagem 2)
        local type_info = BuiltinTypes.ANY
        symbol = (node.kind == SK.CONST_DECL) 
            and Symbol.constant(node.name, type_info, node.is_pub, node.span)
            or Symbol.variable(node.name, type_info, node.is_pub, node.span)
            
    elseif node.kind == SK.IMPORT_DECL then
        -- O nome no escopo local será o alias ou a última parte do path
        local parts = {}
        for part in node.path:gmatch("[^%.]+") do table.insert(parts, part) end
        local name = node.alias or parts[#parts]
        symbol = Symbol.new(Symbol.Kind.MODULE, name, { span = node.span })
    end

    if symbol then
        symbol.declaration = node
        symbol.is_pub = node.is_pub or false
        if not self.scope:define(symbol) then
            self.diagnostics:report_error("ZT-S001", 
                string.format("redefinição de '%s'", symbol.name), 
                node.span)
        end
    end
end

-- ============================================================================
-- Passagem 2 — Vinculação (Análise Profunda)
-- ============================================================================

function Binder:_bind_node(node)
    if not node then return BuiltinTypes.VOID end
    
    local handlers = {
        [SK.FUNC_DECL]       = self._bind_func_decl,
        [SK.ASYNC_FUNC_DECL] = self._bind_func_decl,
        [SK.VAR_DECL]        = self._bind_var_decl,
        [SK.CONST_DECL]      = self._bind_var_decl,
        [SK.GLOBAL_DECL]     = self._bind_var_decl,
        [SK.STATE_DECL]      = self._bind_var_decl,
        [SK.COMPUTED_DECL]   = self._bind_var_decl,
        [SK.ASSIGN_STMT]     = self._bind_assign_stmt,
        [SK.EXPR_STMT]       = self._bind_expr_stmt,
        [SK.BINARY_EXPR]     = self._bind_binary_expr,
        [SK.LITERAL_EXPR]    = self._bind_literal_expr,
        [SK.IDENTIFIER_EXPR] = self._bind_identifier_expr,
        [SK.CALL_EXPR]       = self._bind_call_expr,
        [SK.INDEX_EXPR]      = self._bind_index_expr,
        [SK.IF_STMT]         = self._bind_if_stmt,
        [SK.RETURN_STMT]     = self._bind_return_stmt,
        [SK.WHILE_STMT]      = self._bind_while_stmt,
        [SK.RANGE_EXPR]      = self._bind_range_expr,
        [SK.GROUP_EXPR]      = self._bind_group_expr,
        [SK.STRUCT_DECL]     = self._bind_struct_decl,
        [SK.SELF_FIELD_EXPR] = self._bind_self_field_expr,
        [SK.IT_EXPR]         = self._bind_it_ref_expr,
        [SK.SELF_EXPR]       = self._bind_self_ref_expr,
        [SK.MEMBER_EXPR]     = self._bind_member_expr,
        [SK.STRUCT_INIT_EXPR]= self._bind_struct_init_expr,
        [SK.UNARY_EXPR]      = self._bind_unary_expr,
        [SK.BANG_EXPR]       = self._bind_bang_expr,
        [SK.LEN_EXPR]        = self._bind_len_expr,
        [SK.LIST_EXPR]       = self._bind_list_expr,
        [SK.MAP_EXPR]        = self._bind_map_expr,
        [SK.ATTEMPT_STMT]    = self._bind_attempt_stmt,
        [SK.WATCH_STMT]      = self._bind_watch_stmt,
        [SK.AFTER_STMT]      = self._bind_after_stmt,
        [SK.MATCH_STMT]      = self._bind_match_stmt,
        [SK.LAMBDA_EXPR]     = self._bind_lambda_expr,
        [SK.IS_EXPR]         = self._bind_is_expr,
        [SK.AS_EXPR]         = self._bind_as_expr,
        [SK.CHECK_EXPR]      = self._bind_check_expr,
        [SK.AWAIT_EXPR]      = self._bind_await_expr,
        [SK.THROW_STMT]      = self._bind_throw_stmt,
        [SK.CHECK_STMT]      = self._bind_check_stmt,
        [SK.ASSERT_STMT]     = self._bind_assert_stmt,
        [SK.FOR_IN_STMT]     = self._bind_for_in_stmt,
        [SK.REPEAT_TIMES_STMT] = self._bind_repeat_stmt,
        [SK.COMPOUND_ASSIGN_STMT] = self._bind_compound_assign_stmt,
        [SK.IMPORT_DECL]     = self._bind_import_decl,
        [SK.TYPE_ALIAS_DECL] = self._bind_type_alias_decl,
        [SK.UNION_DECL]      = self._bind_union_decl,
        [SK.TRAIT_DECL]      = self._bind_trait_decl,
        [SK.APPLY_DECL]      = self._bind_apply_decl,
        [SK.ENUM_DECL]       = self._bind_enum_decl,
        [SK.TRY_EXPR]        = self._bind_try_expr,
        [SK.NAMESPACE_DECL]  = function() end, -- Já tratado no bind()
    }
    
    local handler = handlers[node.kind]
    if handler then
        return handler(self, node)
    else
        -- Fallback: recursão para nós desconhecidos
        -- self.diagnostics:report_warning("ZT-S000", "nó não verificado pelo Binder: " .. node.kind, node.span)
        return BuiltinTypes.ANY
    end
end

function Binder:_bind_func_decl(node)
    local symbol = self.scope:lookup_local(node.name)
    
    -- Suporte a funções aninhadas
    if not symbol then
        -- Criamos o símbolo sem resolver o tipo de retorno ainda, pois pode depender de genéricos
        symbol = Symbol.func(node.name, node.params, BuiltinTypes.ANY, node.span, node.is_async)
        symbol.is_pub = node.is_pub or false
        if not self.scope:define(symbol) then
            self.diagnostics:report_error("ZT-S001", string.format("redefinição de '%s'", symbol.name), node.span)
            return BuiltinTypes.ERROR
        end
    end
    
    local prev_func = self.current_func
    self.current_func = symbol
    
    node.symbol = symbol
    symbol.declaration = node
    
    -- Sub-escopo para função (parâmetros + corpo)
    local func_scope = Scope.new(Scope.Kind.FUNCTION, self.scope)
    local prev_scope = self.scope
    self.scope = func_scope

    -- Se estivermos em uma struct, injeta 'self'
    if self.current_struct then
        local self_sym = Symbol.parameter("self", self.current_struct.type_info, nil, node.span)
        self.scope:define(self_sym)
    end

    -- 1. Vincular parâmetros genéricos (CRÍTICO: antes da assinatura)
    local resolved_generic_params = {}
    for _, p in ipairs(node.generic_params or {}) do
        local constraint = p.constraint and self:_resolve_type(p.constraint) or nil
        local param_sym = Symbol.generic_param(p.name, constraint, p.span)
        if not self.scope:define(param_sym) then
            self.diagnostics:report_error("ZT-S001", string.format("parâmetro genérico '%s' já declarado", p.name), p.span)
        end
        table.insert(resolved_generic_params, param_sym)
    end
    symbol.generic_params = resolved_generic_params

    -- 2. Resolver tipo de retorno e parâmetros reais (agora T é conhecido)
    symbol.return_type = self:_resolve_type(node.return_type)

    local resolved_params = {}
    for i, param in ipairs(node.params) do
        local type_info = self:_resolve_type(param.type_node)
        table.insert(resolved_params, { name = param.name, type_info = type_info })
        
        -- Se não tem nome, é destruturação: func f(Player { x })
        if not param.name and param.pattern then
            param.name = "_p" .. i -- Nome interno para o Lua
            self:_bind_pattern(param.pattern, type_info)
        end

        local param_symbol = Symbol.parameter(param.name, type_info, param.default_value, param.span)
        if not self.scope:define(param_symbol) then
            self.diagnostics:report_error("ZT-S001", 
                string.format("parâmetro '%s' já declarado", param.name), 
                param.span)
        end
    end
    
    -- Atualizar type_info com a assinatura resolvida
    symbol.type_info = ZenithType.new(ZenithType.Kind.FUNC, symbol.name, {
        params = resolved_params,
        return_type = symbol.return_type,
        is_async = symbol.is_async
    })
    -- FASE 6: expor os parâmetros resolvidos diretamente no símbolo para
    -- que _bind_call_expr possa verificar tipos de argumentos.
    symbol.params = resolved_params

    -- Se estamos apenas vinculando assinaturas (Passagem 1.2), paramos aqui
    if self.only_signatures then
        self.scope = prev_scope
        self.current_func = prev_func
        return symbol.type_info
    end

    -- 3. Corpo
    for _, stmt in ipairs(node.body) do
        self:_bind_node(stmt)
    end
    
    self.scope = prev_scope
    self.current_func = prev_func
    return symbol.type_info or BuiltinTypes.VOID
end

function Binder:_bind_struct_decl(node)
    local symbol = self.scope:lookup(node.name)
    if not symbol then return BuiltinTypes.ERROR end

    node.symbol = symbol
    local prev_struct = self.current_struct
    self.current_struct = symbol
    
    local struct_scope = symbol.members_scope
    local prev_scope = self.scope

    if not struct_scope then
        -- Primeira vez visitando (Passagem 1.2): Criar escopo e vincular campos
        struct_scope = Scope.new(Scope.Kind.STRUCT, self.scope)
        symbol.members_scope = struct_scope
        self.scope = struct_scope
        
        -- Vincular parâmetros genéricos
        local resolved_generic_params = {}
        for _, p in ipairs(node.generic_params or {}) do
            local constraint = p.constraint and self:_resolve_type(p.constraint) or nil
            local param_sym = Symbol.generic_param(p.name, constraint, p.span)
            if not self.scope:define(param_sym) then
                self.diagnostics:report_error("ZT-S001", string.format("parâmetro genérico '%s' já declarado", p.name), p.span)
            end
            table.insert(resolved_generic_params, param_sym)
        end
        symbol.generic_params = resolved_generic_params

        -- Vincular campos
        local resolved_fields = {}
        for _, field in ipairs(node.fields) do
            local type_info = self:_resolve_type(field.type_node)
            local field_sym = Symbol.field(field.name, type_info, field.is_pub, field.default_value, field.span)
            if not self.scope:define(field_sym) then
                self.diagnostics:report_error("ZT-S001", 
                    string.format("campo '%s' já declarado no struct", field.name), field.span)
            end
            table.insert(resolved_fields, field_sym)
        end
        symbol.fields = resolved_fields
        -- Vincular assinaturas de métodos (SEM o corpo ainda)
        local resolved_methods = {}
        for _, method in ipairs(node.methods) do
            local return_type = self:_resolve_type(method.return_type)
            local method_sym = Symbol.func(method.name, method.params, return_type, method.span)
            method_sym.declaration = method
            method.symbol = method_sym
            table.insert(resolved_methods, method_sym)
        end
        symbol.methods = resolved_methods
    end

    -- Se estamos apenas registrando assinaturas, paramos aqui
    if self.only_signatures then
        self.scope = prev_scope
        self.current_struct = prev_struct
        return symbol.type_info
    end

    -- Segunda vez visitando (Passagem 2): Vincular corpos de métodos
    self.scope = struct_scope
    for _, method_sym in ipairs(symbol.methods or {}) do
        if method_sym.declaration then
            self:_bind_node(method_sym.declaration)
        end
    end
    
    self.scope = prev_scope
    self.current_struct = prev_struct
    return symbol.type_info or BuiltinTypes.VOID
end

function Binder:_bind_trait_decl(node)
    local symbol = self.scope:lookup_local(node.name)
    if not symbol then return BuiltinTypes.ERROR end

    local prev_scope = self.scope
    local trait_scope = symbol.members_scope
    
    if not trait_scope then
        trait_scope = Scope.new(Scope.Kind.STRUCT, self.scope)
        symbol.members_scope = trait_scope
        self.scope = trait_scope

        -- Resolver parâmetros genéricos da Trait
        local resolved_generic_params = {}
        for _, p in ipairs(node.generic_params or {}) do
            local constraint = p.constraint and self:_resolve_type(p.constraint) or nil
            local param_sym = Symbol.generic_param(p.name, constraint, p.span)
            if not self.scope:define(param_sym) then
                self.diagnostics:report_error("ZT-S001", string.format("parâmetro genérico '%s' já declarado", p.name), p.span)
            end
            table.insert(resolved_generic_params, param_sym)
        end
        symbol.generic_params = resolved_generic_params
        
        local resolved_methods = {}
        for _, method in ipairs(node.methods) do
            local return_type = self:_resolve_type(method.return_type)
            local method_sym = Symbol.func(method.name, method.params, return_type, method.span)
            method_sym.declaration = method
            table.insert(resolved_methods, method_sym)
        end
        symbol.methods = resolved_methods
    end

    if self.only_signatures then
        self.scope = prev_scope
        return BuiltinTypes.VOID
    end

    self.scope = prev_scope
    return BuiltinTypes.VOID
end
function Binder:_bind_apply_decl(node)
    local trait_sym = self.scope:lookup(node.trait_name)
    local struct_sym = self.scope:lookup(node.struct_name)
    
    if not trait_sym or trait_sym.kind ~= Symbol.Kind.TRAIT then
        self.diagnostics:report_error("ZT-S301", string.format("trait '%s' não encontrada", node.trait_name), node.span)
        return BuiltinTypes.ERROR
    end
    if not struct_sym or struct_sym.kind ~= Symbol.Kind.STRUCT then
        self.diagnostics:report_error("ZT-S302", string.format("struct '%s' não encontrada", node.struct_name), node.span)
        return BuiltinTypes.ERROR
    end
    
    -- Vincular métodos da implementação
    local prev_struct = self.current_struct
    self.current_struct = struct_sym
    
    -- O escopo de implementação herda do escopo de membros da struct para achar campos
    local struct_scope = Scope.new(Scope.Kind.STRUCT, struct_sym.members_scope or self.scope)
    local prev_scope = self.scope
    self.scope = struct_scope

    -- Se a trait for genérica e houver argumentos, injetamos os parâmetros genéricos no escopo
    if trait_sym.generic_params and #trait_sym.generic_params > 0 and node.generic_args and #node.generic_args > 0 then
        for i, param_sym in ipairs(trait_sym.generic_params) do
            local arg_node = node.generic_args[i]
            if arg_node then
                local arg_type = self:_resolve_type(arg_node)
                -- Injeta o nome do parâmetro (ex: T) mapeado para o tipo real (ex: int)
                local concrete_sym = Symbol.alias(param_sym.name, arg_type, false, arg_node.span)
                self.scope:define(concrete_sym)
            end
        end
    end

    local implemented_methods = {}
    for _, method in ipairs(node.methods or {}) do
        -- Forçamos que NÃO seja apenas assinatura aqui, para vincular o corpo
        local old_only = self.only_signatures
        self.only_signatures = false
        self:_bind_node(method)
        self.only_signatures = old_only

        if method.symbol then
            implemented_methods[method.name] = method.symbol
            -- Adiciona ao símbolo da struct se não existir
            local existing = struct_sym:get_member(method.name)
            if not existing then
                table.insert(struct_sym.methods, method.symbol)
                if struct_sym.members_scope then
                    struct_sym.members_scope:define(method.symbol)
                end
            end
        end
    end
    
    -- Verificação de Contrato e Injeção de Defaults
    for _, req_method in ipairs(trait_sym.methods) do
        local impl = implemented_methods[req_method.name] or struct_sym:get_member(req_method.name)
        if not impl then
            -- Se não tem implementação, checa se a trait tem um default (corpo no nó da declaração)
            local req_node = req_method.declaration
            if req_node and req_node.body then
                -- Injeta a implementação padrão na struct
                table.insert(struct_sym.methods, req_method)
            else
                self.diagnostics:report_error("ZT-S303", 
                    string.format("struct '%s' não implementa o método requerido '%s' da trait '%s'", 
                        struct_sym.name, req_method.name, trait_sym.name), node.span)
            end
        end
    end
    
    -- Registrar que a struct implementa a trait
    table.insert(struct_sym.implements, trait_sym)
    
    self.scope = prev_scope
    self.current_struct = prev_struct
    return BuiltinTypes.VOID
end

function Binder:_bind_type_alias_decl(node)
    local symbol = self.scope:lookup_local(node.name)
    if not symbol then
        -- Caso não tenha sido pré-registrado na Passagem 1 (raro em v0.2, mas possível)
        symbol = Symbol.alias(node.name, node.generic_params or {}, node.is_pub, node.span)
        self.scope:define(symbol)
    end
    
    symbol.target_node = node.target
    symbol.is_pub = node.is_pub or false
    
    -- Se NÃO for genérico, resolvemos agora para validar imediatamente
    if not node.generic_params or #node.generic_params == 0 then
        symbol.type_info = self:_resolve_type(node.target)
    else
        -- Se for genérico, a resolução real será dinâmica no momento do uso
        symbol.type_info = BuiltinTypes.ANY
        
        -- Mas ainda registramos os parâmetros genéricos no símbolo para a substituição futura
        local resolved_params = {}
        for _, p in ipairs(node.generic_params) do
            local p_sym = Symbol.generic_param(p.name, nil, p.span)
            table.insert(resolved_params, p_sym)
        end
        symbol.generic_params = resolved_params
    end
    node.symbol = symbol
    return symbol.type_info
end

function Binder:_bind_union_decl(node)
    local symbol = self.scope:lookup_local(node.name)
    if not symbol then return BuiltinTypes.ERROR end
    
    symbol.target_node = node.union_type
    
    if not node.generic_params or #node.generic_params == 0 then
        symbol.type_info = self:_resolve_type(node.union_type)
    else
        symbol.type_info = BuiltinTypes.ANY
        local resolved_params = {}
        for _, p in ipairs(node.generic_params) do
            local p_sym = Symbol.generic_param(p.name, nil, p.span)
            table.insert(resolved_params, p_sym)
        end
        symbol.generic_params = resolved_params
    end
    
    node.symbol = symbol
    return symbol.type_info
end

function Binder:_bind_var_decl(node)
    local type_info = BuiltinTypes.ANY
    local symbol = nil
    if node.type_node then
        type_info = self:_resolve_type(node.type_node)
    end

    -- Se for um identificador simples, usamos a lógica antiga
    if node.name then
        symbol = self.scope:lookup_local(node.name)
        if not symbol then
            if node.kind == SK.CONST_DECL then symbol = Symbol.constant(node.name, type_info, node.is_pub, node.span)
            elseif node.kind == SK.GLOBAL_DECL then symbol = Symbol.global_var(node.name, type_info, node.is_pub, node.span)
            elseif node.kind == SK.STATE_DECL then symbol = Symbol.state_var(node.name, type_info, node.span)
            elseif node.kind == SK.COMPUTED_DECL then symbol = Symbol.computed(node.name, type_info, node.span)
            else symbol = Symbol.variable(node.name, type_info, node.is_pub, node.span) end
            
            if not self.scope:define(symbol) then
                self.diagnostics:report_error("ZT-S001", string.format("redefinição de '%s'", node.name), node.span)
            end
        end
        node.symbol = symbol
    elseif node.pattern then
        -- Destruturação complexa: vincular sub-padrões
        self:_bind_pattern(node.pattern, type_info)
    else
        -- Erro fallback
        self.diagnostics:report_error("ZT-S101", "declaração inválida: nome ou padrão esperado", node.span)
    end
    
    -- Verificar inicializador
    local init_node = node.initializer or node.expression
    if init_node then
        local init_type = self:_bind_node(init_node)
        local TypeChecker = require("src.semantic.types.type_checker")
        
        -- Se o tipo não foi especificado, ele é inferido do inicializador na destruturação
        if not node.type_node then
            type_info = init_type
            -- Se for destruturação, precisamos re-vincular os tipos dos sub-padrões com a informação real
            if node.pattern then self:_bind_pattern(node.pattern, type_info) end
        end

        if not TypeChecker.is_assignable(init_type, type_info) then
            self.diagnostics:report_error("ZT-S100", 
                string.format("Tipo incompatível na atribuição: esperado '%s', encontrado '%s'", tostring(type_info), tostring(init_type)), 
                init_node.span)
        end
    elseif not node.pattern then
        -- Para declarações simples, o tipo é obrigatório se não houver inicializador (v0.2 rule)
        if not node.type_node then
            self.diagnostics:report_error("ZT-S101", "definição de tipo obrigatória para variável '" .. (node.name or "???") .. "'", node.span)
        end
    end

    if symbol then
        symbol.type_info = type_info
        node.symbol = symbol
    end

    return type_info
end

function Binder:_instantiate_type(type_info, bindings)
    if not type_info then return nil end

    if type_info.kind == ZenithType.Kind.TYPE_PARAM then
        return bindings[type_info.name] or type_info
    end

    if type_info.kind == ZenithType.Kind.GENERIC then
        local args = {}
        for _, arg in ipairs(type_info.type_args or {}) do
            table.insert(args, self:_instantiate_type(arg, bindings))
        end
        return ZenithType.new(ZenithType.Kind.GENERIC, type_info.base_name or type_info.name, {
            base_name = type_info.base_name or type_info.name,
            type_args = args,
            span = type_info.span,
            symbol = type_info.symbol,
        })
    end

    if type_info.kind == ZenithType.Kind.UNION then
        local types = {}
        for _, inner in ipairs(type_info.types or {}) do
            table.insert(types, self:_instantiate_type(inner, bindings))
        end
        return ZenithType.create_union(types)
    end

    if type_info.kind == ZenithType.Kind.NULLABLE then
        local base = self:_instantiate_type(type_info.base_type, bindings)
        return ZenithType.new(ZenithType.Kind.NULLABLE, tostring(base) .. "?", {
            base_type = base,
            span = type_info.span,
        })
    end

    return type_info
end

function Binder:_collect_type_bindings(template_type, actual_type, bindings)
    if not template_type or not actual_type then return end

    if template_type.kind == ZenithType.Kind.TYPE_PARAM then
        if not bindings[template_type.name] then
            bindings[template_type.name] = actual_type
        end
        return
    end

    if template_type.kind == ZenithType.Kind.GENERIC and
       actual_type.kind == ZenithType.Kind.GENERIC and
       template_type.base_name == actual_type.base_name then
        for i, inner in ipairs(template_type.type_args or {}) do
            self:_collect_type_bindings(inner, actual_type.type_args and actual_type.type_args[i], bindings)
        end
    end
end

function Binder:_make_enum_instance_type(enum_symbol)
    if not enum_symbol then return BuiltinTypes.ANY end
    if not enum_symbol.generic_params or #enum_symbol.generic_params == 0 then
        return enum_symbol.type_info or BuiltinTypes.ANY
    end

    local args = {}
    for _, p in ipairs(enum_symbol.generic_params) do
        table.insert(args, ZenithType.new(ZenithType.Kind.TYPE_PARAM, p.name, {
            symbol = p,
            constraint = p.constraint,
        }))
    end

    return ZenithType.new(ZenithType.Kind.GENERIC, enum_symbol.name, {
        base_name = enum_symbol.name,
        type_args = args,
        symbol = enum_symbol,
    })
end

function Binder:_infer_enum_member_return(symbol, arg_types)
    if not symbol or symbol.kind ~= Symbol.Kind.ENUM_MEMBER then
        return symbol and (symbol.return_type or symbol.type_info) or nil
    end

    local inferred = {}
    for i, param in ipairs(symbol.params or {}) do
        self:_collect_type_bindings(param.type_info, arg_types and arg_types[i], inferred)
    end

    local current_ret = self.current_func and self.current_func.return_type
    local parent = symbol.parent_enum
    if parent and current_ret and current_ret.kind == ZenithType.Kind.GENERIC and current_ret.base_name == parent.name then
        for i, p in ipairs(parent.generic_params or {}) do
            if inferred[p.name] == nil then
                inferred[p.name] = current_ret.type_args[i]
            end
        end
    end

    return self:_instantiate_type(symbol.return_type or symbol.type_info, inferred)
end

--- Vincula um padrão de destruturação.
function Binder:_bind_pattern(pattern, matched_type)
    if not pattern then return end
    
    if pattern.kind == SK.IDENTIFIER_EXPR then
        local name = pattern.name
        if name == "_" then return end

        local builtin_type = BuiltinTypes.lookup(name)
        if builtin_type and builtin_type.kind ~= ZenithType.Kind.GENERIC then
            pattern.match_type = builtin_type
            local TypeChecker = require("src.semantic.types.type_checker")
            if matched_type ~= BuiltinTypes.ANY and
               not TypeChecker.is_assignable(matched_type, builtin_type) and
               not TypeChecker.is_assignable(builtin_type, matched_type) then
                self.diagnostics:report_error("ZT-S100",
                    string.format("tipo do padrão '%s' incompatível com '%s'", tostring(builtin_type), tostring(matched_type)),
                    pattern.span)
            end
            return
        end

        local type_symbol = self.scope:lookup(name)
        if type_symbol and (
            type_symbol.kind == Symbol.Kind.ALIAS or
            type_symbol.kind == Symbol.Kind.STRUCT or
            type_symbol.kind == Symbol.Kind.TRAIT or
            type_symbol.kind == Symbol.Kind.ENUM
        ) then
            local resolved_type = self:_resolve_type({
                kind = SK.NAMED_TYPE,
                name = name,
                span = pattern.span
            })
            pattern.match_type = resolved_type

            local TypeChecker = require("src.semantic.types.type_checker")
            if matched_type ~= BuiltinTypes.ANY and
               not TypeChecker.is_assignable(matched_type, resolved_type) and
               not TypeChecker.is_assignable(resolved_type, matched_type) then
                self.diagnostics:report_error("ZT-S100",
                    string.format("tipo do padrão '%s' incompatível com '%s'", tostring(resolved_type), tostring(matched_type)),
                    pattern.span)
            end
            return
        end
        
        -- Se for um Enum Member conhecido, não é destruturação, é comparação de valor
        local existing = self.scope:lookup(name)
        if existing and existing.kind == Symbol.Kind.ENUM_MEMBER then
            pattern.symbol = existing
            return
        end
        
        -- Caso contrário, é uma captura (cria nova variável local)
        local sym = Symbol.variable(name, matched_type, false, pattern.span)
        self.scope:define(sym)
        pattern.symbol = sym
        pattern.is_capture = true

    elseif pattern.kind == SK.LIST_EXPR then
        -- Assume que matched_type é list<T>
        local item_type = BuiltinTypes.ANY
        if matched_type.kind == ZenithType.Kind.GENERIC and matched_type.base_name == "list" then
            item_type = matched_type.type_args[1]
        end
        
        for _, el in ipairs(pattern.elements) do
            self:_bind_pattern(el, item_type)
        end

    elseif pattern.kind == SK.REST_EXPR then
        -- ..resto (matched_type deve ser o tipo da coleção)
        self:_bind_pattern(pattern.expression, matched_type)

    elseif pattern.kind == SK.STRUCT_INIT_EXPR then
        -- Player { nome, vida: 0 }
        local struct_sym = self.scope:lookup(pattern.type_name)
        if not struct_sym or struct_sym.kind ~= Symbol.Kind.STRUCT then
            self.diagnostics:report_error("ZT-S002", "struct '" .. pattern.type_name .. "' não encontrada no padrão", pattern.span)
            return
        end
        
        for _, f in ipairs(pattern.fields) do
            -- Busca o tipo do campo na struct
            local field_sym = struct_sym:get_member(f.name)
            local f_type = field_sym and field_sym.type_info or BuiltinTypes.ANY
            self:_bind_pattern(f.value, f_type)
        end

    elseif pattern.kind == SK.VARIANT_PATTERN then
        -- Variant(a, b)
        local variant_name = pattern.callee.name
        local variant_sym = self.scope:lookup(variant_name)
        
        if not variant_sym or variant_sym.kind ~= Symbol.Kind.ENUM_MEMBER then
            self.diagnostics:report_error("ZT-S102", "variante '" .. variant_name .. "' não encontrada no padrão", pattern.span)
            return
        end
        pattern.symbol = variant_sym
        
        if pattern.arguments and variant_sym.params then
            local generic_bindings = {}
            if matched_type.kind == ZenithType.Kind.GENERIC and
               variant_sym.parent_enum and
               matched_type.base_name == variant_sym.parent_enum.name then
                for i, p in ipairs(variant_sym.parent_enum.generic_params or {}) do
                    generic_bindings[p.name] = matched_type.type_args[i]
                end
            end

            for i, arg_p in ipairs(pattern.arguments) do
                local param_info = variant_sym.params[i]
                local matched_p_type = param_info and param_info.type_info or BuiltinTypes.ANY
                matched_p_type = self:_instantiate_type(matched_p_type, generic_bindings)
                self:_bind_pattern(arg_p, matched_p_type)
            end
        end

    else
        -- Literais e outros: apenas vincula normalmente para checar tipos
        local p_type = self:_bind_node(pattern)
        local TypeChecker = require("src.semantic.types.type_checker")
        if not TypeChecker.is_assignable(p_type, matched_type) then
            self.diagnostics:report_error("ZT-S100", 
                string.format("tipo do padrão '%s' incompatível com '%s'", tostring(p_type), tostring(matched_type)), 
                pattern.span)
        end
    end
end

-- ============================================================================
-- Handlers de Expressões
-- ============================================================================

function Binder:_bind_literal_expr(node)
    if node.literal_type == 'null' and not self.skip_prelude then
        self.diagnostics:report_error("ZT-S003", "uso direto de null é proibido", node.span)
    end
    local map = {
        int = BuiltinTypes.INT,
        float = BuiltinTypes.FLOAT,
        text = BuiltinTypes.TEXT,
        bool = BuiltinTypes.BOOL,
        null = BuiltinTypes.NULL,
    }
    return map[node.literal_type] or BuiltinTypes.ERROR
end

function Binder:_bind_list_expr(node)
    local item_type = BuiltinTypes.ANY
    if #node.elements > 0 then
        item_type = self:_bind_node(node.elements[1])
    end
    
    return ZenithType.new(ZenithType.Kind.GENERIC, "list", {
        base_name = "list",
        type_args = { item_type },
        span = node.span
    })
end

function Binder:_bind_map_expr(node)
    local key_type = BuiltinTypes.TEXT
    local val_type = BuiltinTypes.ANY
    
    local pairs_list = node.pairs or {}
    if #pairs_list > 0 then
        key_type = self:_bind_node(pairs_list[1].key)
        val_type = self:_bind_node(pairs_list[1].value)
    end
    
    for _, p in ipairs(pairs_list) do
        self:_bind_node(p.key)
        self:_bind_node(p.value)
    end
    
    return ZenithType.new(ZenithType.Kind.GENERIC, "map", {
        base_name = "map",
        type_args = { key_type, val_type },
        span = node.span
    })
end

function Binder:_bind_identifier_expr(node)
    local symbol = self.scope:lookup(node.name)
    if not symbol then
        self.diagnostics:report_error("ZT-S002", 
            string.format("símbolo '%s' não encontrado", node.name), node.span)
        return BuiltinTypes.ERROR
    end
    
    node.symbol = symbol
    -- Se for um reativo (STATE), o tipo resultante é o tipo base (transparente)
    node.is_reactive = (symbol.kind == Symbol.Kind.STATE or symbol.kind == Symbol.Kind.COMPUTED)

    if symbol.kind == Symbol.Kind.ENUM_MEMBER and not symbol.params then
        local inferred = self:_infer_enum_member_return(symbol, {})
        node.type_info = inferred
        return inferred or BuiltinTypes.ANY
    end

    node.type_info = symbol.type_info or BuiltinTypes.ANY
    return node.type_info
end

function Binder:_bind_binary_expr(node)
    local left_type = self:_bind_node(node.left)
    local right_type = self:_bind_node(node.right)
    
    local result_type = TypeChecker.get_binary_result(left_type, node.operator.lexeme, right_type)
    
    if result_type == BuiltinTypes.ERROR then
        self.diagnostics:report_error("ZT-S101", 
            string.format("operador '%s' inválido para tipos '%s' e '%s'", 
                node.operator.lexeme, tostring(left_type), tostring(right_type)), node.span)
    end
    
    node.type_info = result_type
    return result_type
end

function Binder:_bind_unary_expr(node)
    local operand_type = self:_bind_node(node.operand)
    local result_type = TypeChecker.get_unary_result(node.operator.lexeme, operand_type)
    
    if result_type == BuiltinTypes.ERROR then
        self.diagnostics:report_error("ZT-S101", 
            string.format("operador '%s' inválido para tipo '%s'", 
                node.operator.lexeme, tostring(operand_type)), node.span)
    end
    
    return result_type
end

function Binder:_bind_call_expr(node)
    local callee_type = self:_bind_node(node.callee)
    
    -- Se o callee tem um símbolo, pegamos os parâmetros reais dele
    local symbol = node.callee.symbol
    if not symbol and node.callee.kind == SK.MEMBER_EXPR then
        symbol = node.callee.symbol -- Já deve ter sido setado se foi resolvido
    end

    -- Validação de Argumentos Genéricos Explícitos: func<int>(...)
    if node.generic_args and symbol and symbol.generic_params then
        local resolved_args = {}
        for _, arg_node in ipairs(node.generic_args) do
            table.insert(resolved_args, self:_resolve_type(arg_node))
        end
        node.resolved_generic_args = resolved_args
        
        if #resolved_args ~= #symbol.generic_params then
            self.diagnostics:report_error("ZT-S104", 
                string.format("número incorreto de argumentos genéricos para '%s' (esperado %d, recebido %d)", 
                    symbol.name, #symbol.generic_params, #resolved_args), node.span)
        else
            local TypeChecker = require("src.semantic.types.type_checker")
            for i, arg_type in ipairs(resolved_args) do
                local constraint = symbol.generic_params[i].constraint
                if constraint and not TypeChecker.is_assignable(arg_type, constraint) then
                    self.diagnostics:report_error("ZT-S100", 
                        string.format("argumento genérico '%s' não satisfaz a restrição '%s' de '%s'", 
                            tostring(arg_type), tostring(constraint), symbol.generic_params[i].name), 
                        node.generic_args[i].span)
                end
            end
        end
    end

    if callee_type.kind ~= ZenithType.Kind.FUNC and callee_type.kind ~= ZenithType.Kind.STRUCT then
        -- Se for um símbolo de função já resolvido (mesmo que o tipo de retorno seja VOID), é chamável
        if symbol and symbol.kind == Symbol.Kind.FUNCTION then
            -- OK
        elseif callee_type == BuiltinTypes.ANY then 
            -- Vincular argumentos mesmo que não possamos validar os parâmetros
            for _, arg in ipairs(node.arguments) do
                local arg_node = arg
                if type(arg) == "table" and arg.kind == "NAMED" then
                    arg_node = arg.value
                end
                self:_bind_node(arg_node)
            end
            return BuiltinTypes.ANY 
        else
            self.diagnostics:report_error("ZT-S103", 
                string.format("tipo '%s' não é chamável", tostring(callee_type)), node.callee.span)
            return BuiltinTypes.ERROR
        end
    end

    -- Validação de Chamada de Função
    local params = symbol and symbol.params or callee_type.params or {}
    
    -- Heurística Construtor: se callee_type é STRUCT, os parâmetros são seus campos (para o .new() implícito)
    if callee_type.kind == ZenithType.Kind.STRUCT and #params == 0 then
        local struct_sym = callee_type.symbol
        if struct_sym and struct_sym.fields then
            params = struct_sym.fields
        end
    end

    local args = node.arguments
    local effective_args = {} -- Mapeamento param_index -> arg_node
    
    -- 1. Suporte UFCS: O primeiro parâmetro é preenchido pelo objeto do MemberExpr
    local current_pos = 1
    if node.callee.kind == SK.MEMBER_EXPR and node.callee.is_ufcs then
        effective_args[1] = node.callee.object
        current_pos = 2
    end

    -- 2. Preencher argumentos posicionais e nomeados
    local has_spread = false
    for i, arg in ipairs(args) do
        if type(arg) == "table" and arg.kind == "NAMED" then
            -- ... (restante do código de NAMED mantido)
            local found = false
            for j, p in ipairs(params) do
                if p.name == arg.name then
                    if effective_args[j] then
                        self.diagnostics:report_error("ZT-S105", 
                            string.format("parâmetro '%s' já preenchido posicionalmente", arg.name), arg.span or node.span)
                    end
                    effective_args[j] = arg.value
                    found = true
                    break
                end
            end
            if not found then
                self.diagnostics:report_error("ZT-S105", 
                    string.format("parâmetro '%s' não existe na função", arg.name), arg.span or node.span)
            end
        else
            -- Posicional
            if arg.kind == SK.REST_EXPR then has_spread = true end
            
            while effective_args[current_pos] do current_pos = current_pos + 1 end
            if current_pos > #params and not has_spread then
                self.diagnostics:report_error("ZT-S105", "argumentos posicionais demais (esperado " .. #params .. ")", node.span)
            else
                effective_args[current_pos] = arg
                current_pos = current_pos + 1
            end
        end
    end

    -- 3. Verificar parâmetros obrigatórios faltantes e validar tipos
    local final_args = {}
    local final_arg_types = {}
    -- Se houver spread, a validação de "faltantes" é pulada pois o spread pode preencher tudo em runtime
    if has_spread then
        -- Apenas vincula todos os argumentos presentes
        for _, arg in ipairs(args) do
            local arg_node = arg
            if type(arg) == "table" and arg.kind == "NAMED" then arg_node = arg.value end
            local arg_type = self:_bind_node(arg_node)
            table.insert(final_args, arg)
            table.insert(final_arg_types, arg_type)
        end
    else
        for i, p in ipairs(params) do
            local val = effective_args[i]
            if not val then
                if p.default_value then
                    val = p.default_value
                else
                    local p_name = p.name or "desconhecido"
                    self.diagnostics:report_error("ZT-S105", "argumento faltante para o parâmetro '" .. p_name .. "'", node.span)
                end
            end
            if val then
                local arg_type = self:_bind_node(val)
                table.insert(final_args, val)
                final_arg_types[i] = arg_type

                -- FASE 6: verificar tipo do argumento contra o tipo do parâmetro
                local TypeChecker = require("src.semantic.types.type_checker")
                local p_type = p.type_info or BuiltinTypes.ANY
                if not TypeChecker.is_assignable(arg_type, p_type) then
                    self.diagnostics:report_error("ZT-T003",
                        string.format("argumento #%d '%s': esperado '%s', encontrado '%s'",
                            i, p.name or "?", tostring(p_type), tostring(arg_type)),
                        val.span,
                        string.format("converta o valor ou ajuste a assinatura da função"))
                end
            end
        end
    end

    node.resolved_args = final_args
    local ret = symbol and symbol.return_type or callee_type.return_type
    if symbol and symbol.kind == Symbol.Kind.ENUM_MEMBER then
        ret = self:_infer_enum_member_return(symbol, final_arg_types) or ret
    end
    if not ret and callee_type.kind == ZenithType.Kind.STRUCT then
        ret = callee_type
    end
    return ret or BuiltinTypes.ANY
end

function Binder:_bind_expr_stmt(node)
    self:_bind_node(node.expression)
    return BuiltinTypes.VOID
end

function Binder:_bind_enum_decl(node)
    local enum_symbol = self.scope:lookup_local(node.name)
    if not enum_symbol then return BuiltinTypes.VOID end

    node.symbol = enum_symbol
    local enum_scope = Scope.new(Scope.Kind.BLOCK, self.scope)
    local prev_scope = self.scope
    self.scope = enum_scope

    local resolved_generic_params = {}
    for _, p in ipairs(node.generic_params or {}) do
        local constraint = p.constraint and self:_resolve_type(p.constraint) or nil
        local param_sym = Symbol.generic_param(p.name, constraint, p.span)
        if not self.scope:define(param_sym) then
            self.diagnostics:report_error("ZT-S001", string.format("parâmetro genérico '%s' já declarado", p.name), p.span)
        end
        table.insert(resolved_generic_params, param_sym)
    end
    enum_symbol.generic_params = resolved_generic_params

    local enum_instance_type = self:_make_enum_instance_type(enum_symbol)
    local member_symbols = {}
    for _, member_node in ipairs(node.members) do
        local resolved_params = nil
        if member_node.params then
            resolved_params = {}
            for _, p in ipairs(member_node.params) do
                local p_type = self:_resolve_type(p.type)
                table.insert(resolved_params, { name = p.name, type_info = p_type })
            end
        end

        local member_symbol = Symbol.enum_member(member_node.name, enum_symbol, resolved_params, member_node.span)
        member_symbol.declaration = member_node
        member_symbol.return_type = enum_instance_type
        if resolved_params and #resolved_params > 0 then
            member_symbol.type_info = ZenithType.new(ZenithType.Kind.FUNC, member_node.name, {
                params = resolved_params,
                return_type = enum_instance_type,
            })
        else
            member_symbol.type_info = enum_instance_type
        end
        table.insert(member_symbols, member_symbol)

        -- Facilitar uso sem prefixo (Enum.Member -> Member) se não houver colisão
        if not prev_scope:lookup_local(member_node.name) then
            prev_scope:define(member_symbol)
        end
    end

    self.scope = prev_scope
    enum_symbol.member_symbols = member_symbols
    return enum_symbol.type_info
end

function Binder:_bind_assign_stmt(node)
    local target_type = self:_bind_node(node.target or node.left)
    local value_type = self:_bind_node(node.value or node.right)
    
    local TypeChecker = require("src.semantic.types.type_checker")
    if not TypeChecker.is_assignable(value_type, target_type) then
        self.diagnostics:report_error("ZT-S100", 
            string.format("Tipo incompatível na atribuição: esperado '%s', encontrado '%s'", tostring(target_type), tostring(value_type)), 
            (node.value or node.right).span)
    end
    
    return BuiltinTypes.VOID
end

function Binder:_bind_compound_assign_stmt(node)
    return self:_bind_assign_stmt(node)
end

function Binder:_bind_if_stmt(node)
    local cond_type = self:_bind_node(node.condition)
    if cond_type ~= BuiltinTypes.BOOL and cond_type ~= BuiltinTypes.ANY then
        self.diagnostics:report_error("ZT-S102", "condição do 'if' deve ser 'bool'", node.condition.span)
    end
    
    local if_scope = Scope.new(Scope.Kind.BLOCK, self.scope)
    local old_scope = self.scope
    self.scope = if_scope
    for _, stmt in ipairs(node.body) do self:_bind_node(stmt) end
    self.scope = old_scope
    
    for _, elif in ipairs(node.elif_clauses or {}) do
        local ec_type = self:_bind_node(elif.condition)
        if ec_type ~= BuiltinTypes.BOOL and ec_type ~= BuiltinTypes.ANY then
            self.diagnostics:report_error("ZT-S102", "condição do 'elif' deve ser 'bool'", elif.condition.span)
        end
        local elif_scope = Scope.new(Scope.Kind.BLOCK, self.scope)
        self.scope = elif_scope
        for _, stmt in ipairs(elif.body) do self:_bind_node(stmt) end
        self.scope = old_scope
    end
    
    if node.else_clause then
        local else_scope = Scope.new(Scope.Kind.BLOCK, self.scope)
        self.scope = else_scope
        for _, stmt in ipairs(node.else_clause.body) do self:_bind_node(stmt) end
        self.scope = old_scope
    end
    
    return BuiltinTypes.VOID
end

function Binder:_bind_match_stmt(node)
    local expr_type = self:_bind_node(node.expression)
    
    for _, case in ipairs(node.cases) do
        -- Cada case tem seu próprio escopo para variáveis capturadas nos padrões
        local case_scope = Scope.new(Scope.Kind.BLOCK, self.scope)
        local old_scope = self.scope
        self.scope = case_scope
        
        for _, pattern in ipairs(case.patterns) do
            self:_bind_pattern(pattern, expr_type)
        end
        
        for _, stmt in ipairs(case.body) do
            self:_bind_node(stmt)
        end
        
        self.scope = old_scope
    end
    
    if node.else_clause then
        local else_scope = Scope.new(Scope.Kind.BLOCK, self.scope)
        local old_scope = self.scope
        self.scope = else_scope
        for _, stmt in ipairs(node.else_clause.body) do
            self:_bind_node(stmt)
        end
        self.scope = old_scope
    end
    
    return BuiltinTypes.VOID
end

function Binder:_bind_for_in_stmt(node)
    local iterable_type = self:_bind_node(node.iterable)
    
    local for_scope = Scope.new(Scope.Kind.BLOCK, self.scope)
    local old_scope = self.scope
    self.scope = for_scope
    
    -- Descobrir tipo dos itens
    local item_type = BuiltinTypes.ANY
    
    if iterable_type.kind == ZenithType.Kind.GENERIC and iterable_type.base_name == "list" then
        item_type = iterable_type.type_args[1] or BuiltinTypes.ANY
    elseif iterable_type == BuiltinTypes.TEXT then
        item_type = BuiltinTypes.TEXT
    else
        -- Tentar buscar Trait Iterable no tipo
        local sym = iterable_type.symbol
        if sym then
            local it_method = sym:get_member("iterator")
            if it_method then
                if it_method.type_info and it_method.type_info.kind == ZenithType.Kind.FUNC and it_method.type_info.return_type then
                    local ret = it_method.type_info.return_type
                    if ret.kind == ZenithType.Kind.FUNC and ret.return_type then
                        local opt = ret.return_type
                        if opt.kind == ZenithType.Kind.GENERIC and opt.base_name == "Optional" then
                            item_type = opt.type_args[1] or BuiltinTypes.ANY
                        end
                    end
                end
            end
        end
    end

    for _, v in ipairs(node.variables) do
        local sym = Symbol.for_var(v.name, item_type, v.span)
        self.scope:define(sym)
    end
    
    for _, stmt in ipairs(node.body) do self:_bind_node(stmt) end
    
    self.scope = old_scope
    return BuiltinTypes.VOID
end

function Binder:_bind_repeat_stmt(node)
    self:_bind_node(node.count_expr)
    local repeat_scope = Scope.new(Scope.Kind.BLOCK, self.scope)
    local old_scope = self.scope
    self.scope = repeat_scope
    for _, stmt in ipairs(node.body) do self:_bind_node(stmt) end
    self.scope = old_scope
    return BuiltinTypes.VOID
end

function Binder:_bind_bang_expr(node)
    local operand_type = self:_bind_node(node.expression)

    if operand_type.kind == ZenithType.Kind.NULLABLE then
        node.type_info = operand_type.base_type
        return node.type_info
    end

    if operand_type.kind == ZenithType.Kind.UNION then
        local inner = {}
        for _, t in ipairs(operand_type.types or {}) do
            if t ~= BuiltinTypes.NULL and t.kind ~= ZenithType.Kind.NULL then
                table.insert(inner, t)
            end
        end

        if #inner == 1 then
            node.type_info = inner[1]
            return node.type_info
        end

        if #inner > 1 then
            node.type_info = ZenithType.create_union(inner)
            return node.type_info
        end
    end

    node.type_info = operand_type
    return operand_type
end

function Binder:_bind_assert_stmt(node)
    self:_bind_node(node.expression)
    return BuiltinTypes.VOID
end

function Binder:_bind_attempt_stmt(node)
    local try_scope = Scope.new(Scope.Kind.BLOCK, self.scope)
    local old_scope = self.scope
    self.scope = try_scope
    for _, stmt in ipairs(node.body) do self:_bind_node(stmt) end
    self.scope = old_scope
    
    if node.rescue_clause then
        local rescue = node.rescue_clause
        local rescue_scope = Scope.new(Scope.Kind.BLOCK, self.scope)
        local old_scope_r = self.scope
        self.scope = rescue_scope
        
        if rescue.error_name then
            local err_sym = Symbol.new(Symbol.Kind.RESCUE_VAR, rescue.error_name, {
                type_info = BuiltinTypes.ANY,
                span = rescue.span
            })
            self.scope:define(err_sym)
        end

        for _, stmt in ipairs(rescue.body) do self:_bind_node(stmt) end
        self.scope = old_scope_r
    end

    return BuiltinTypes.VOID
end

function Binder:_bind_watch_stmt(node)
    if node.target then self:_bind_node(node.target) end
    local watch_scope = Scope.new(Scope.Kind.BLOCK, self.scope)
    local old_scope = self.scope
    self.scope = watch_scope
    for _, stmt in ipairs(node.body) do self:_bind_node(stmt) end
    self.scope = old_scope
    return BuiltinTypes.VOID
end

function Binder:_bind_after_stmt(node)
    local after_scope = Scope.new(Scope.Kind.BLOCK, self.scope)
    local old_scope = self.scope
    self.scope = after_scope
    for _, stmt in ipairs(node.body or {}) do
        self:_bind_node(stmt)
    end
    self.scope = old_scope
    return BuiltinTypes.VOID
end

function Binder:_bind_return_stmt(node)
    local expr_type = node.expression and self:_bind_node(node.expression) or BuiltinTypes.VOID
    
    if self.current_func then
        local expected = self.current_func.return_type
        if not TypeChecker.is_assignable(expr_type, expected) then
            self.diagnostics:report_error("ZT-S100", 
                string.format("Tipo de retorno incompatível: esperado '%s', encontrado '%s'", tostring(expected), tostring(expr_type)), 
                node.expression and node.expression.span or node.span)
        end
    end
    
    return BuiltinTypes.VOID
end

function Binder:_bind_while_stmt(node)
    local cond_type = self:_bind_node(node.condition)
    if cond_type ~= BuiltinTypes.BOOL and cond_type ~= BuiltinTypes.ANY then
        self.diagnostics:report_error("ZT-S102", "condição do 'while' deve ser 'bool'", node.condition.span)
    end
    
    local while_scope = Scope.new(Scope.Kind.BLOCK, self.scope)
    local old_scope = self.scope
    self.scope = while_scope
    for _, stmt in ipairs(node.body) do self:_bind_node(stmt) end
    self.scope = old_scope
    return BuiltinTypes.VOID
end

function Binder:_bind_range_expr(node)
    local start_t = self:_bind_node(node.start_expr)
    local end_t = self:_bind_node(node.end_expr)
    
    if start_t ~= BuiltinTypes.INT or end_t ~= BuiltinTypes.INT then
        self.diagnostics:report_error("ZT-S102", "intervalos (ranges) devem ser compostos por inteiros", node.span)
    end
    
    return ZenithType.new(ZenithType.Kind.GENERIC, "list<int>", {
        base_name = "list",
        type_args = { BuiltinTypes.INT },
        span = node.span
    })
end

function Binder:_bind_group_expr(node)
    return self:_bind_node(node.expression)
end

function Binder:_bind_self_field_expr(node)
    if not self.current_struct then
        self.diagnostics:report_error("ZT-S201", 
            "atalho '@campo' só pode ser usado dentro de métodos de um struct/trait", node.span)
        return BuiltinTypes.ERROR
    end
    
    -- Resolve como 'self.campo'
    local sym = self.scope:lookup(node.field_name)
    
    -- Se não achou no escopo, tenta buscar diretamente no símbolo da struct (para casos de apply)
    if not sym and self.current_struct then
        sym = self.current_struct:get_member(node.field_name)
    end

    if not sym then
        self.diagnostics:report_error("ZT-S002", 
            string.format("campo '%s' não encontrado no struct '%s'", node.field_name, self.current_struct.name), node.span)
        return BuiltinTypes.ERROR
    end
    
    node.symbol = sym
    return sym.type_info or BuiltinTypes.ANY
end

function Binder:_bind_it_ref_expr(node)
    -- 'it' é resolvido dinamicamente ou injetado em contextos específicos (como where)
    return BuiltinTypes.ANY
end

function Binder:_bind_self_ref_expr(node)
    if not self.current_struct then
        self.diagnostics:report_error("ZT-S201", "'self' só pode ser usado dentro de métodos", node.span)
        return BuiltinTypes.ERROR
    end
    return self.current_struct.type_info
end

function Binder:_bind_member_expr(node)
    local object_type = self:_bind_node(node.object)
    
    -- Heurística para construtores: Struct.new(...) -> Struct
    if node.member_name == "new" and object_type.kind == ZenithType.Kind.STRUCT then
        return object_type
    end

    -- Caso 2: Acesso a Campo/Método de Struct ou Tipo Genérico (T)
    local target_type = object_type
    
    -- Se for um parâmetro genérico, buscamos na restrição (constraint) dele
    if object_type.kind == ZenithType.Kind.TYPE_PARAM and object_type.constraint then
        target_type = object_type.constraint
    end

    if target_type.kind == ZenithType.Kind.STRUCT or target_type.kind == ZenithType.Kind.TRAIT or target_type.kind == ZenithType.Kind.ENUM then
        local base_sym = target_type.symbol
        if base_sym then
            local member_sym = base_sym:get_member(node.member_name)
            if member_sym then
                node.symbol = member_sym
                return member_sym.type_info or member_sym.return_type or BuiltinTypes.ANY
            end
        end
    end

    -- Fallback UFCS: Procurar função global que aceite o objeto como primeiro parâmetro
    local ufcs_sym = self.scope:lookup(node.member_name)
    if ufcs_sym and ufcs_sym.kind == Symbol.Kind.FUNCTION then
        node.symbol = ufcs_sym
        node.is_ufcs = true
        return ufcs_sym.type_info or ufcs_sym.return_type or BuiltinTypes.ANY
    end

    -- Fallback UFCS 2: Funções Runtime zt.* (len, split, etc) que funcionam para qualquer tipo
    local zt_ufcs_methods = {
        ["len"]   = Symbol.func("len", { Symbol.variable("obj", BuiltinTypes.ANY) }, BuiltinTypes.INT),
        ["split"] = Symbol.func("split", { Symbol.variable("obj", BuiltinTypes.ANY), Symbol.variable("sep", BuiltinTypes.TEXT) }, BuiltinTypes.ANY)
    }
    if zt_ufcs_methods[node.member_name] then
        local sym = zt_ufcs_methods[node.member_name]
        node.symbol = sym
        node.is_ufcs = true
        node.ufcs_name = "zt." .. node.member_name
        return sym.type_info or BuiltinTypes.ANY
    end
    -- Heurística para reatividade: state.get / state.set
    if node.member_name == "get" or node.member_name == "set" then
        return BuiltinTypes.ANY
    end

    -- Heurística para ANY: se nada acima funcionou e o objeto é ANY, aceitamos qualquer membro
    if object_type == BuiltinTypes.ANY then
        return BuiltinTypes.ANY
    end

    -- Erro: membro não encontrado
    self.diagnostics:report_error("ZT-S008", 
        string.format("membro '%s' não encontrado no tipo '%s'", 
            node.member_name, tostring(object_type)), node.span)
    
    return BuiltinTypes.ANY
end

function Binder:_bind_index_expr(node)
    local object_type = self:_bind_node(node.object)
    local index_type = self:_bind_node(node.index_expr)

    if node.index_expr and node.index_expr.kind == SK.RANGE_EXPR then
        if object_type.kind == ZenithType.Kind.GENERIC and object_type.base_name == "list" then
            return ZenithType.new(ZenithType.Kind.GENERIC, "list", {
                base_name = "list",
                type_args = { object_type.type_args[1] or BuiltinTypes.ANY },
                span = node.span
            })
        end

        if object_type == BuiltinTypes.TEXT then
            return BuiltinTypes.TEXT
        end

        return BuiltinTypes.ANY
    end

    if index_type ~= BuiltinTypes.INT and index_type ~= BuiltinTypes.ANY then
        self.diagnostics:report_error("ZT-S102", "índices devem ser inteiros", node.index_expr.span)
    end

    if object_type.kind == ZenithType.Kind.GENERIC and object_type.base_name == "list" then
        return object_type.type_args[1] or BuiltinTypes.ANY
    end

    if object_type == BuiltinTypes.TEXT then
        return BuiltinTypes.TEXT
    end

    return BuiltinTypes.ANY
end

function Binder:_bind_len_expr(node)
    local expr_type = self:_bind_node(node.expression)

    if expr_type == BuiltinTypes.TEXT or expr_type == BuiltinTypes.ANY then
        return BuiltinTypes.INT
    end

    if expr_type.kind == ZenithType.Kind.GENERIC then
        if expr_type.base_name == "list" or expr_type.base_name == "map" or expr_type.base_name == "grid" then
            return BuiltinTypes.INT
        end
    end

    return BuiltinTypes.INT
end

function Binder:_bind_struct_init_expr(node)
    -- Se node.type_name tiver ponto, ignoramos verificação estrita por enquanto
    if node.type_name and node.type_name:find("%.") then
        for _, field in ipairs(node.fields) do
            self:_bind_node(field.value)
        end
        return BuiltinTypes.ANY
    end

    local type_sym = self.scope:lookup(node.type_name)
    if not type_sym or type_sym.kind ~= Symbol.Kind.STRUCT then
        self.diagnostics:report_error("ZT-S002", 
            string.format("tipo '%s' não encontrado para inicialização", node.type_name or "desconhecido"), node.span)
        return BuiltinTypes.ERROR
    end
    
    local resolved_type = type_sym.type_info
    
    -- Validar se a inicialização usa genéricos: Box<int> { ... }
    if node.generic_args then
        -- Criamos um nó temporário de GENERIC_TYPE para reutilizar a lógica de validação do _resolve_type
        resolved_type = self:_resolve_type({
            kind = SK.GENERIC_TYPE,
            base_name = node.type_name,
            type_args = node.generic_args,
            span = node.span
        })
    end

    -- Validar campos da inicialização
    for _, field in ipairs(node.fields) do
        self:_bind_node(field.value)
    end
    
    return resolved_type or type_sym.type_info or BuiltinTypes.ANY
end

-- ============================================================================
-- Helpers de Resolução
--- Resolve um nó de tipo da AST para um ZenithType real.
function Binder:_resolve_type(type_node)
    if not type_node then return BuiltinTypes.VOID end
    
    if type_node.kind == SK.NAMED_TYPE then
        local builtin = BuiltinTypes.lookup(type_node.name)
        if builtin then return builtin end
        
        -- Proteção contra recursão infinita
        if self.resolve_stack[type_node.name] then
            self.diagnostics:report_error("ZT-S009", 
                string.format("dependência circular detectada no alias '%s'", type_node.name), 
                type_node.span)
            return BuiltinTypes.ERROR
        end
        self.resolve_stack[type_node.name] = true

        -- Prioridade 1: Parâmetros genéricos do alias atual
        if self.current_alias and self.current_alias.generic_params then
            for _, p in ipairs(self.current_alias.generic_params) do
                if p.name == type_node.name then
                    self.resolve_stack[type_node.name] = nil
                    return ZenithType.new(ZenithType.Kind.TYPE_PARAM, p.name, {
                        symbol = p,
                        constraint = p.constraint
                    })
                end
            end
        end

        local symbol = self.scope:lookup(type_node.name)
        if symbol then
            if symbol.kind == Symbol.Kind.STRUCT or 
               symbol.kind == Symbol.Kind.ENUM or 
               symbol.kind == Symbol.Kind.TRAIT then
                self.resolve_stack[type_node.name] = nil
                return symbol.type_info
            
            elseif symbol.kind == Symbol.Kind.ALIAS then
                local res = self:_resolve_type(symbol.target_node)
                self.resolve_stack[type_node.name] = nil
                return res
            
            elseif symbol.kind == Symbol.Kind.GENERIC_PARAM then
                if symbol.type_info then
                    self.resolve_stack[type_node.name] = nil
                    return symbol.type_info
                end
                self.resolve_stack[type_node.name] = nil
                return ZenithType.new(ZenithType.Kind.TYPE_PARAM, type_node.name, {
                    symbol = symbol,
                    constraint = symbol.constraint
                })
            end
        end
        
        self.resolve_stack[type_node.name] = nil
        self.diagnostics:report_error("ZT-S002", 
            string.format("tipo desconhecido: '%s'", type_node.name), 
            type_node.span)
        return BuiltinTypes.ERROR
    end

    if type_node.kind == SK.GENERIC_TYPE then
        local base_sym = self.scope:lookup(type_node.base_name)
        local expected_params = {}
        
        -- Heurística: se não achou símbolo, checa se é builtin
        if not base_sym then
            local builtin = BuiltinTypes.lookup(type_node.base_name)
            if builtin then
                -- Para builtins, criamos parâmetros genéricos "fakes"
                local counts = { list = 1, grid = 1, map = 2, Outcome = 1, Optional = 1 }
                local count = counts[type_node.base_name] or 0
                for i = 1, count do table.insert(expected_params, { name = "T"..i }) end
            else
                self.diagnostics:report_error("ZT-S002", string.format("tipo base '%s' não encontrado", type_node.base_name), type_node.span)
                return BuiltinTypes.ERROR
            end
        else
            expected_params = base_sym.generic_params or {}
        end

        local args = {}
        for i, arg_node in ipairs(type_node.type_args) do
            local arg_t = self:_resolve_type(arg_node)
            table.insert(args, arg_t)
        end
        
        -- Validar Constraints
        if #args ~= #expected_params then
            -- Se for Alias, a validação é flexível pois pode ser resolvido dinamicamente
            if not (base_sym and base_sym.kind == Symbol.Kind.ALIAS) then
                self.diagnostics:report_error("ZT-S104", 
                    string.format("número incorreto de argumentos genéricos para '%s' (esperado %d, recebido %d)", 
                        type_node.base_name, #expected_params, #args), type_node.span)
            end
        else
            local TypeChecker = require("src.semantic.types.type_checker")
            for i, arg_type in ipairs(args) do
                local constraint = expected_params[i].constraint
                if constraint and not TypeChecker.is_assignable(arg_type, constraint) then
                    self.diagnostics:report_error("ZT-S100", 
                        string.format("argumento genérico '%s' não satisfaz a restrição '%s' de '%s'", 
                            tostring(arg_type), tostring(constraint), expected_params[i].name), 
                        type_node.type_args[i].span)
                end
            end
        end
        
        -- Caso Especial: Alias Genérico (type Box<T> = list<T>)
        if base_sym and base_sym.kind == Symbol.Kind.ALIAS and #expected_params > 0 then
            -- Proteção contra recursão
            if self.resolve_stack[type_node.base_name] then
                self.diagnostics:report_error("ZT-S009", 
                    string.format("dependência circular detectada no alias '%s'", type_node.base_name), 
                    type_node.span)
                return BuiltinTypes.ERROR
            end
            
            self.resolve_stack[type_node.base_name] = true
            local prev_scope = self.scope
            local temp_scope = Scope.new(Scope.Kind.BLOCK, self.scope) -- Escopo efêmero
            
            for i, p in ipairs(expected_params) do
                local arg_type = args[i] or BuiltinTypes.ANY
                local p_sym = Symbol.generic_param(p.name, nil, type_node.span)
                p_sym.type_info = arg_type
                temp_scope:define(p_sym)
            end
            
            self.scope = temp_scope
            local result = self:_resolve_type(base_sym.target_node)
            self.scope = prev_scope
            self.resolve_stack[type_node.base_name] = nil
            return result
        end

        return ZenithType.new(ZenithType.Kind.GENERIC, type_node.base_name, {
            base_name = type_node.base_name,
            type_args = args,
            span = type_node.span
        })
    end

    if type_node.kind == SK.UNION_TYPE then
        local inner_types = {}
        for _, t in ipairs(type_node.types or {}) do
            table.insert(inner_types, self:_resolve_type(t))
        end
        return ZenithType.create_union(inner_types)
    end

    if type_node.kind == SK.NULLABLE_TYPE then
        local base = self:_resolve_type(type_node.base_type)
        -- Nullable(T) é implementado como Union(T, null)
        return ZenithType.create_union({ base, BuiltinTypes.NULL })
    end

    if type_node.kind == SK.FUNC_TYPE then
        local params = {}
        for _, p_node in ipairs(type_node.params or {}) do
            table.insert(params, self:_resolve_type(p_node))
        end
        local ret = self:_resolve_type(type_node.return_type)
        return ZenithType.new(ZenithType.Kind.FUNC, "func", {
            params = params,
            return_type = ret
        })
    end

    if type_node.kind == SK.STRUCT_TYPE then
        -- Suporte a tipos struct anônimos/inline (raro mas usado em testes)
        local fields = {}
        for _, f in ipairs(type_node.fields or {}) do
            table.insert(fields, { name = f.name, type_info = self:_resolve_type(f.type_node) })
        end
        return ZenithType.new(ZenithType.Kind.STRUCT, "anonymous_struct", {
            fields = fields,
            span = type_node.span
        })
    end

    return BuiltinTypes.ANY
end

function Binder:_bind_lambda_expr(node)
    local ZenithType = require("src.semantic.types.zenith_type")
    
    local lambda_scope = Scope.new(Scope.Kind.FUNCTION, self.scope)
    local old_scope = self.scope
    self.scope = lambda_scope
    
    local params = {}
    for i, param in ipairs(node.params or {}) do
        local type_info = param.type_node and self:_resolve_type(param.type_node) or BuiltinTypes.ANY
        
        -- Destruturação em Lambda
        if not param.name and param.pattern then
            param.name = "_lp" .. i
            self:_bind_pattern(param.pattern, type_info)
        end

        local param_symbol = Symbol.parameter(param.name, type_info, param.default_value, param.span)
        self.scope:define(param_symbol)
        table.insert(params, type_info)
    end
    
    local old_func = self.current_func
    self.current_func = { name = "lambda", return_type = return_type, is_lambda = true, is_async = node.is_async }
    
    if type(node.body) == "table" and node.body.kind then
        -- Expressão única
        return_type = self:_bind_node(node.body)
        self.current_func.return_type = return_type
    else
        -- Bloco de statements
        for _, stmt in ipairs(node.body or {}) do
            self:_bind_node(stmt)
        end
        -- O tipo de retorno de blocos em lambda v0.2 é ANY por enquanto, 
        -- a menos que queiramos implementar inferência total
        return_type = BuiltinTypes.ANY
    end
    
    self.scope = old_scope
    self.current_func = old_func
    
    local res = ZenithType.new(ZenithType.Kind.FUNC, "lambda", {
        params = params,
        return_type = return_type
    })
    -- print("LAMBDA TYPE: " .. tostring(res.kind) .. " " .. tostring(res))
    return res
end

function Binder:_bind_is_expr(node)
    self:_bind_node(node.expression)
    node.target_type = self:_resolve_type(node.type_node)
    return BuiltinTypes.BOOL
end

function Binder:_bind_as_expr(node)
    self:_bind_node(node.expression)
    node.target_type = self:_resolve_type(node.type_node)
    return node.target_type
end

function Binder:_bind_check_expr(node)
    local val_type = self:_bind_node(node.expression)
    -- check expr sempre retorna o valor se passar, ou interrompe. 
    -- Para o sistema de tipos, ele é transparente.
    return val_type
end

function Binder:_bind_check_stmt(node)
    self:_bind_node(node.condition)
    
    local else_scope = Scope.new(Scope.Kind.BLOCK, self.scope)
    local old_scope = self.scope
    self.scope = else_scope
    for _, stmt in ipairs(node.else_body) do self:_bind_node(stmt) end
    self.scope = old_scope
    
    return BuiltinTypes.VOID
end

function Binder:_bind_await_expr(node)
    if not self.current_func or not self.current_func.is_async then
        self.diagnostics:report_error("ZT-S202", "'await' só pode ser usado dentro de uma função 'async'", node.span)
    end
    return self:_bind_node(node.expression)
end

function Binder:_bind_throw_stmt(node)
    self:_bind_node(node.expression)
    return BuiltinTypes.VOID
end

function Binder:_bind_native_lua(node)
    -- Blocos native são tratados como transparentes (any)
    node.type_info = BuiltinTypes.ANY
    return BuiltinTypes.ANY
end

function Binder:_bind_try_expr(node)
    local expr_type = self:_bind_node(node.expression)
    
    if expr_type.kind ~= ZenithType.Kind.GENERIC or (expr_type.base_name ~= "Optional" and expr_type.base_name ~= "Outcome") then
        self.diagnostics:report_error("ZT-S103", 
            string.format("o operador '?' só pode ser usado em tipos Optional ou Outcome, encontrou '%s'", tostring(expr_type)), 
            node.span)
        return BuiltinTypes.ERROR
    end
    
    -- Verificar compatibilidade de retorno da função atual
    if not self.current_func then
        self.diagnostics:report_error("ZT-S104", "o operador '?' só pode ser usado dentro de funções", node.span)
        return expr_type.type_args[1]
    end
    
    local ret_type = self.current_func.return_type
    if ret_type.kind ~= ZenithType.Kind.GENERIC or ret_type.base_name ~= expr_type.base_name then
        self.diagnostics:report_error("ZT-S105", 
            string.format("o tipo de retorno da função '%s' é incompatível com o retorno precoce do operador '?' (%s esperado)", 
            tostring(ret_type), expr_type.base_name), 
            node.span)
    end
    
    -- O tipo resultante da expressão 'val?' é o parâmetro genérico T (o primeiro)
    node.type_info = expr_type.type_args[1]
    return node.type_info
end

return Binder

end

package.preload["src.semantic.binding.binder"] = package.preload["semantic.binding.binder"]
package.preload["semantic/binding/binder"] = package.preload["semantic.binding.binder"]
package.preload["src/semantic/binding/binder"] = package.preload["semantic.binding.binder"]
package.preload["semantic.binding.binder.init"] = package.preload["semantic.binding.binder"]
package.preload["semantic.binding.binder/init"] = package.preload["semantic.binding.binder"]
package.preload['syntax.lexer.lexer'] = function()
-- ============================================================================
-- Zenith Compiler — Lexer
-- Tokeniza o texto-fonte em uma lista de tokens.
-- ============================================================================

local TokenKind    = require("src.syntax.tokens.token_kind")
local Token        = require("src.syntax.tokens.token")
local KeywordTable = require("src.syntax.tokens.keyword_table")
local Span         = require("src.source.span")
local DiagnosticBag = require("src.diagnostics.diagnostic_bag")

local Lexer = {}
Lexer.__index = Lexer

function Lexer.new(source_text)
    local self = setmetatable({}, Lexer)
    self.source = source_text
    self.text = source_text.text
    self.pos = 1
    self.diagnostics = DiagnosticBag.new()
    self.buffer = {} 

    -- Tabelas de operadores locais à instância usando string.char para evitar bugs de encoding
    self.TWO_CHAR = {
        ["=="] = TokenKind.EQUAL_EQUAL, ["!="] = TokenKind.BANG_EQUAL,
        ["<="] = TokenKind.LESS_EQUAL, [">="] = TokenKind.GREATER_EQUAL,
        ["+="] = TokenKind.PLUS_EQUAL, ["-="] = TokenKind.MINUS_EQUAL,
        ["*="] = TokenKind.STAR_EQUAL, ["/="] = TokenKind.SLASH_EQUAL,
        ["->"] = TokenKind.ARROW, ["=>"] = TokenKind.FAT_ARROW, [".."] = TokenKind.DOT_DOT,
        ["?."] = TokenKind.QUESTION_DOT
    }

    self.SINGLE_CHAR = {
        ["+"] = TokenKind.PLUS, ["-"] = TokenKind.MINUS, ["*"] = TokenKind.STAR,
        ["/"] = TokenKind.SLASH, ["%"] = TokenKind.PERCENT, ["^"] = TokenKind.CARET,
        ["<"] = TokenKind.LESS, [">"] = TokenKind.GREATER, ["="] = TokenKind.EQUAL,
        ["("] = TokenKind.LPAREN, [")"] = TokenKind.RPAREN, ["["] = TokenKind.LBRACKET,
        ["]"] = TokenKind.RBRACKET, ["{"] = TokenKind.LBRACE, ["}"] = TokenKind.RBRACE,
        ["."] = TokenKind.DOT, [","] = TokenKind.COMMA, [":"] = TokenKind.COLON,
        [";"] = TokenKind.SEMICOLON, [string.char(124)] = TokenKind.PIPE, ["!"] = TokenKind.BANG,
        ["?"] = TokenKind.QUESTION, ["#"] = TokenKind.HASH, ["@"] = TokenKind.AT
    }

    return self
end

function Lexer:current()
    if self.pos > #self.text then return "\0" end
    return self.text:sub(self.pos, self.pos)
end

function Lexer:peek(offset)
    local p = self.pos + (offset or 1)
    if p > #self.text then return "\0" end
    return self.text:sub(p, p)
end

function Lexer:advance(n)
    self.pos = self.pos + (n or 1)
end

function Lexer:is_at_end()
    return self.pos > #self.text
end

local function is_digit(c) return c >= "0" and c <= "9" end
local function is_alpha(c) return (c >= "a" and c <= "z") or (c >= "A" and c <= "Z") or c == "_" end
local function is_alnum(c) return is_alpha(c) or is_digit(c) end
local function is_whitespace(c) return c == " " or c == "\t" or c == "\r" end

function Lexer:tokenize()
    local tokens = {}
    while not self:is_at_end() or #self.buffer > 0 do
        local token = self:_next_token()
        if token then table.insert(tokens, token) end
    end
    table.insert(tokens, Token.new(TokenKind.EOF, "", nil, Span.new(self.pos, 0)))
    return tokens
end

function Lexer:_next_token()
    if #self.buffer > 0 then
        return table.remove(self.buffer, 1)
    end

    local c = self:current()
    if is_whitespace(c) then self:_skip_whitespace() return nil end
    if c == "\n" then return self:_read_newline() end
    if c == "-" and self:peek() == "-" then self:_skip_comment() return nil end
    if is_digit(c) then return self:_read_number() end
    if c == '"' then return self:_read_string() end
    if is_alpha(c) then return self:_read_identifier() end
    return self:_read_operator()
end

function Lexer:_skip_whitespace()
    while not self:is_at_end() and is_whitespace(self:current()) do self:advance() end
end

function Lexer:_skip_comment()
    self:advance(2)
    
    -- Se for ---, tratamos como Doc Comment
    if self:current() == "-" then
        self:advance(1)
        local start = self.pos - 3
        local content = ""
        
        -- Descobre se é bloco ou linha única
        -- Se houver apenas espaços e newline após ---, é bloco
        local is_block = false
        local p = 0
        while true do
            local next_c = self:peek(p)
            if next_c == " " or next_c == "\t" or next_c == "\r" then
                p = p + 1
            elseif next_c == "\n" or next_c == "\0" then
                is_block = true
                break
            else
                break
            end
        end

        if is_block then
            -- Pula o resto da linha de abertura
            while not self:is_at_end() and self:current() ~= "\n" do self:advance() end
            if self:current() == "\n" then self:advance() end

            while not self:is_at_end() do
                if self:current() == "-" and self:peek(1) == "-" and self:peek(2) == "-" then
                    self:advance(3)
                    break
                end
                content = content .. self:current()
                self:advance()
            end
        else
            -- Linha única: até o fim da linha
            while not self:is_at_end() and self:current() ~= "\n" do
                content = content .. self:current()
                self:advance()
            end
        end
        table.insert(self.buffer, Token.new(TokenKind.DOC_COMMENT, content, content, Span.new(start, self.pos - start)))
        return
    end

    while not self:is_at_end() and self:current() ~= "\n" do self:advance() end
end

function Lexer:_read_newline()
    local start = self.pos
    self:advance()
    return Token.new(TokenKind.NEWLINE, "\n", nil, Span.new(start, 1))
end

function Lexer:_read_number()
    local start = self.pos
    local has_dot = false
    while not self:is_at_end() and (is_digit(self:current()) or self:current() == ".") do
        if self:current() == "." then
            if self:peek() == "." or has_dot then break end
            has_dot = true
        end
        self:advance()
    end
    local lexeme = self.text:sub(start, self.pos - 1)
    local value = tonumber(lexeme)
    local span = Span.new(start, #lexeme)
    local kind = has_dot and TokenKind.FLOAT_LITERAL or TokenKind.INTEGER_LITERAL
    
    if is_alpha(self:current()) then
        while not self:is_at_end() and is_alnum(self:current()) do self:advance() end
        lexeme = self.text:sub(start, self.pos - 1)
        self.diagnostics:report_error("ZT-L002", "número seguido de identificador inválido", Span.new(start, #lexeme))
        return Token.new(TokenKind.BAD_TOKEN, lexeme, nil, Span.new(start, #lexeme))
    end

    return Token.new(kind, lexeme, value, span)
end

function Lexer:_read_string()
    local start = self.pos
    self:advance() -- "
    
    local fragments = {}
    local current_fragment = ""
    local is_interpolated = false

    while not self:is_at_end() and self:current() ~= '"' do
        local c = self:current()
        if c == "{" then
            is_interpolated = true
            table.insert(fragments, { type = "LITERAL", val = current_fragment, pos = self.pos - #current_fragment })
            current_fragment = ""
            
            self:advance() -- {
            local expr_start = self.pos
            local depth = 1
            while not self:is_at_end() and depth > 0 do
                if self:current() == "{" then depth = depth + 1
                elseif self:current() == "}" then depth = depth - 1 end
                if depth > 0 then self:advance() end
            end
            
            local expr_text = self.text:sub(expr_start, self.pos - 1)
            table.insert(fragments, { type = "EXPR", val = expr_text, pos = expr_start })
            
            if self:current() == "}" then self:advance() end
        elseif c == "\\" then
            self:advance()
            local esc = self:current()
            local map = { n="\n", t="\t", r="\r", e="\27", ['"']='"', ["\\"]="\\", ["{"]="{", ["}"]="}" }
            
            if map[esc] then
                current_fragment = current_fragment .. map[esc]
                self:advance()
            elseif esc == "x" then
                -- Hex escape \xHH
                self:advance()
                local hex = self.text:sub(self.pos, self.pos + 1)
                local val = tonumber(hex, 16)
                if val then
                    current_fragment = current_fragment .. string.char(val)
                    self:advance(2)
                else
                    current_fragment = current_fragment .. "x"
                end
            elseif esc:match("%d") then
                -- Decimal escape \DDD (up to 3 digits)
                local start_d = self.pos
                while self:current():match("%d") and (self.pos - start_d) < 3 do
                    self:advance()
                end
                local dec = self.text:sub(start_d, self.pos - 1)
                current_fragment = current_fragment .. string.char(tonumber(dec))
            else
                current_fragment = current_fragment .. esc
                self:advance()
            end
        else
            current_fragment = current_fragment .. c
            self:advance()
        end
    end
    
    table.insert(fragments, { type = "LITERAL", val = current_fragment, pos = self.pos - #current_fragment })
    
    if self:is_at_end() then
        self.diagnostics:report_error("ZT-L001", "string não terminada", Span.new(start, self.pos - start))
        return Token.new(TokenKind.BAD_TOKEN, self.text:sub(start, self.pos - 1), nil, Span.new(start, self.pos - start))
    end
    
    self:advance() -- "

    local full_lexeme = self.text:sub(start, self.pos - 1)
    local full_span = Span.new(start, #full_lexeme)

    if not is_interpolated then
        return Token.new(TokenKind.STRING_LITERAL, full_lexeme, fragments[1].val, full_span)
    end

    for i, frag in ipairs(fragments) do
        if frag.type == "LITERAL" then
            if #frag.val > 0 or (i == 1 or i == #fragments) then
                table.insert(self.buffer, Token.new(TokenKind.STRING_LITERAL, '"'..frag.val..'"', frag.val, Span.new(frag.pos, #frag.val)))
            end
        else
            table.insert(self.buffer, Token.new(TokenKind.PLUS, "+", nil, Span.new(frag.pos-1, 1)))
            table.insert(self.buffer, Token.new(TokenKind.LPAREN, "(", nil, Span.new(frag.pos, 0)))
            
            local sub_lexer = Lexer.new({ text = frag.val, source = self.source })
            local sub_tokens = sub_lexer:tokenize()
            for _, st in ipairs(sub_tokens) do
                if st.kind ~= TokenKind.EOF then table.insert(self.buffer, st) end
            end
            
            table.insert(self.buffer, Token.new(TokenKind.RPAREN, ")", nil, Span.new(frag.pos + #frag.val, 0)))
            table.insert(self.buffer, Token.new(TokenKind.PLUS, "+", nil, Span.new(self.pos, 1)))
        end
    end
    
    if self.buffer[#self.buffer] and self.buffer[#self.buffer].kind == TokenKind.PLUS then table.remove(self.buffer) end
    return table.remove(self.buffer, 1)
end

function Lexer:_read_identifier()
    local start = self.pos
    while not self:is_at_end() and is_alnum(self:current()) do self:advance() end
    local lexeme = self.text:sub(start, self.pos - 1)
    local kind = KeywordTable.lookup(lexeme) or TokenKind.IDENTIFIER
    local value = nil
    if kind == TokenKind.KW_TRUE then value = true
    elseif kind == TokenKind.KW_FALSE then value = false end

    if kind ~= TokenKind.IDENTIFIER then
        return Token.new(kind, lexeme, value, Span.new(start, #lexeme))
    end

    return Token.new(kind, lexeme, lexeme, Span.new(start, #lexeme))
end

function Lexer:_read_operator()
    local start = self.pos
    local c = self:current()
    local next_c = self:peek()
    
    local combined = c .. next_c
    if self.TWO_CHAR[combined] then
        self:advance(2)
        return Token.new(self.TWO_CHAR[combined], combined, nil, Span.new(start, 2))
    end

    if self.SINGLE_CHAR[c] then
        self:advance()
        return Token.new(self.SINGLE_CHAR[c], c, nil, Span.new(start, 1))
    end

    self.diagnostics:report_error("ZT-L003", "caractere desconhecido: " .. c, Span.new(start, 1))
    self:advance()
    return Token.new(TokenKind.BAD_TOKEN, c, nil, Span.new(start, 1))
end

return Lexer

end

package.preload["src.syntax.lexer.lexer"] = package.preload["syntax.lexer.lexer"]
package.preload["syntax/lexer/lexer"] = package.preload["syntax.lexer.lexer"]
package.preload["src/syntax/lexer/lexer"] = package.preload["syntax.lexer.lexer"]
package.preload["syntax.lexer.lexer.init"] = package.preload["syntax.lexer.lexer"]
package.preload["syntax.lexer.lexer/init"] = package.preload["syntax.lexer.lexer"]
package.preload['backend.lua.lua_codegen'] = function()
-- ============================================================================
-- Zenith Compiler â€” Lua Codegen
-- Transpila a AST validada para Lua 5.1/LuaJIT.
-- ============================================================================

local SK = require("src.syntax.ast.syntax_kind")
local ZenithType = require("src.semantic.types.zenith_type")

local LuaCodegen = {}
LuaCodegen.__index = LuaCodegen

function LuaCodegen.new(source_text)
    local self = setmetatable({}, LuaCodegen)
    self.output = {}
    self.indent_level = 0
    self.tmp_counter = 0
    self.source_text = source_text  -- para resolucao de spans em anotacoes @zt
    self.source_map  = {}           -- { [lua_line] = {zt_line, zt_col} }
    return self
end

--- Gera um nome de variÃ¡vel temporÃ¡ria Ãºnico.
function LuaCodegen:_fresh_tmp()
    self.tmp_counter = self.tmp_counter + 1
    return "__zt_try_" .. self.tmp_counter
end

--- Emite uma anotaÃ§Ã£o de localizaÃ§Ã£o @zt antes de um statement,
--- e registra no source_map a relaÃ§Ã£o lua_line -> zt_line:col.
function LuaCodegen:emit_loc(node)
    if not node or not node.span or not self.source_text then return end
    local ok, loc = pcall(function()
        return self.source_text:get_location(node.span.start or node.span[1] or 1)
    end)
    if not ok or not loc then return end
    local next_lua_line = #self.output + 1
    self.source_map[next_lua_line] = { line = loc.line, col = loc.column or 1 }
end

--- Gera apenas o corpo do mÃ³dulo sem o bloco de return de exports.
function LuaCodegen:generate_body(node)
    self.output = {}
    self.indent_level = 0
    self.pub_members = {}
    self.imports = {}
    self.has_namespace = false
    self.skip_exports = true
    
    self:emit("-- Transpilado por Zenith v0.2.5 (Body Only)")
    self:emit("local zt = require(\"src.backend.lua.runtime.zenith_rt\")")
    self:_emit_prelude_constructors()
    self:emit("")
    
    self:_emit_node(node)
    
    return table.concat(self.output, "\n")
end

--- Ponto de entrada: transpila uma compilation unit.
--- @param node table AST root (CompilationUnit)
--- @param source_text table|nil SourceText original (para source maps)
--- @return string lua_code, table source_map
function LuaCodegen:generate(node, source_text)
    self.output = {}
    self.indent_level = 0
    self.pub_members = {}
    self.imports = {}
    self.has_namespace = false
    self.skip_exports = false
    self.source_text  = source_text or self.source_text
    self.source_map   = {}
    self.tmp_counter  = 0
    
    -- Banner e Runtime
    self:emit("-- Transpilado por Zenith v0.2.5")
    self:emit("local zt = require(\"src.backend.lua.runtime.zenith_rt\")")
    self:_emit_prelude_constructors()
    self:emit("")
    
    self:_emit_node(node)
    
    return table.concat(self.output, "\n"), self.source_map
end

-- ----------------------------------------------------------------------------
-- EmissÃ£o
-- ----------------------------------------------------------------------------

function LuaCodegen:emit(text)
    local indent = string.rep("    ", self.indent_level)
    table.insert(self.output, indent .. text)
end

function LuaCodegen:_emit_prelude_constructors()
    self:emit("local Present = zt.Optional.Present")
    self:emit("local Empty   = zt.Optional.Empty")
    self:emit("local Success = zt.Outcome.Success")
    self:emit("local Failure = zt.Outcome.Failure")
    self:emit("local Outcome  = zt.Outcome")
    self:emit("local Optional = zt.Optional")
end

function LuaCodegen:indent() self.indent_level = self.indent_level + 1 end
function LuaCodegen:dedent() self.indent_level = self.indent_level - 1 end

function LuaCodegen:_escape_id(name)
    if not name then return nil end
    local s = name:gsub("%?", "_3f"):gsub("!", "_21")
    return s
end

function LuaCodegen:_collect_pattern_names(pattern, names)
    if not pattern then return end
    if pattern.kind == SK.IDENTIFIER_EXPR then
        if pattern.name ~= "_" then
            table.insert(names, self:_escape_id(pattern.name))
        end
    elseif pattern.kind == SK.LIST_EXPR then
        for _, el in ipairs(pattern.elements) do
            self:_collect_pattern_names(el, names)
        end
    elseif pattern.kind == SK.STRUCT_INIT_EXPR then
        for _, f in ipairs(pattern.fields or {}) do
            self:_collect_pattern_names(f.value, names)
        end
    end
end

-- ----------------------------------------------------------------------------
-- Visitors
-- ----------------------------------------------------------------------------

function LuaCodegen:_emit_node(node)
    if not node then return end
    
    -- Suporte a blocos de desugaring (IR)
    if node.kind == "BLOCK" and node.statements then
        for _, s in ipairs(node.statements) do
            self:_emit_node(s)
        end
        return
    end

    local method_name = "_emit_" .. node.kind:lower()
    if self[method_name] then
        return self[method_name](self, node)
    else
        self:emit("-- [ERRO: Codegen nÃ£o implementado para " .. node.kind .. "]")
    end
end

function LuaCodegen:_emit_compilation_unit(node)
    -- Passagem 0: Coletar Structs da unidade atual
    self.structs_to_emit = {}
    for _, decl in ipairs(node.declarations) do
        if decl.kind == SK.STRUCT_DECL then
            table.insert(self.structs_to_emit, decl)
        end
    end

    -- Passagem 1: Declarar nomes locais do mÃ³dulo (forward declaration)
    local locals = {}
    for _, decl in ipairs(node.declarations) do
        if decl.kind == SK.VAR_DECL or decl.kind == SK.CONST_DECL then
            if decl.name then
                table.insert(locals, self:_escape_id(decl.name))
            elseif decl.pattern then
                self:_collect_pattern_names(decl.pattern, locals)
            end
        elseif decl.name and decl.kind ~= SK.NAMESPACE_DECL and decl.kind ~= SK.IMPORT_DECL then
            table.insert(locals, self:_escape_id(decl.name))
        end
    end
    
    if #locals > 0 then
        self:emit("local " .. table.concat(locals, ", "))
        self:emit("")
    end

    -- Passagem 1: Emitir Tipos e FunÃ§Ãµes (DeclaraÃ§Ãµes puras)
    -- APPLY emite antes de STRUCT_METHODS para rastrear o que foi emitido
    self.apply_emitted_methods = {} -- { struct_name = { method_name = true } }
    for _, decl in ipairs(node.declarations) do
        if decl.kind == SK.STRUCT_DECL or decl.kind == SK.FUNC_DECL or decl.kind == SK.ASYNC_FUNC_DECL or
           decl.kind == SK.TRAIT_DECL or decl.kind == SK.ENUM_DECL or decl.kind == SK.IMPORT_DECL or
           decl.kind == SK.TYPE_ALIAS_DECL or decl.kind == SK.UNION_DECL or decl.kind == SK.NAMESPACE_DECL or
           decl.kind == SK.APPLY_DECL then
            self:_emit_node(decl)
            self:emit("")
        end
    end

    -- Passagem Final: Emitir mÃ©todos de structs NAO emitidos via apply
    self:emit("-- Struct Methods")
    for _, struct_node in ipairs(self.structs_to_emit) do
        local already = self.apply_emitted_methods[struct_node.name] or {}
        local struct_sym = struct_node.symbol
        if struct_sym and struct_sym.methods then
            local seen_methods = {}
            for _, method_sym in ipairs(struct_sym.methods) do
                local m_node = method_sym.declaration
                if m_node and m_node.body and not seen_methods[m_node.name] and not already[m_node.name] then
                    seen_methods[m_node.name] = true
                    self:_emit_func_decl(m_node, struct_node.name)
                end
            end
        else
            for _, m_node in ipairs(struct_node.methods or {}) do
                if m_node.body and not already[m_node.name] then
                    self:_emit_func_decl(m_node, struct_node.name)
                end
            end
        end
    end
    
    -- Passagem 2: Emitir CÃ³digo Top-level (VariÃ¡veis e Statements)
    self.is_top_level = true
    for _, decl in ipairs(node.declarations) do
        if not (decl.kind == SK.STRUCT_DECL or decl.kind == SK.FUNC_DECL or decl.kind == SK.ASYNC_FUNC_DECL or
           decl.kind == SK.TRAIT_DECL or decl.kind == SK.ENUM_DECL or decl.kind == SK.IMPORT_DECL or
           decl.kind == SK.TYPE_ALIAS_DECL or decl.kind == SK.UNION_DECL or decl.kind == SK.NAMESPACE_DECL or
           decl.kind == SK.APPLY_DECL) then
            self:_emit_node(decl)
            self:emit("")
        end
    end
    self.is_top_level = false

    -- Entry Point
    local main_found = false
    for _, decl in ipairs(node.declarations) do
        if (decl.kind == SK.FUNC_DECL or decl.kind == SK.ASYNC_FUNC_DECL) and decl.name == "main" then
            main_found = true
            break
        end
    end

    if main_found then
        if not self.has_namespace or self.current_namespace == "main" then
            self:emit("")
            self:emit("-- Auto-run main")
            self:emit("if true then")
            self:indent()
            self:emit("local status = main(arg)")
            self:emit("if type(status) == 'table' and status.co then status = zt.drive(status) end")
            self:emit("if type(status) == 'number' then os.exit(status) end")
            self:dedent()
            self:emit("end")
        end
    end

    -- Exports
    if not self.skip_exports then
        self:emit("")
        self:emit("return {")
        self:indent()
        local exported = {}
        for _, name in ipairs(self.pub_members) do
            if not exported[name] then
                exported[name] = true
                local escaped = self:_escape_id(name)
                self:emit(string.format("%s = %s,", escaped, escaped))
            end
        end
        if main_found and not exported["main"] then
            self:emit("main = main,")
        end
        self:dedent()
        self:emit("}")
    end

end

-- ðŸ› ï¸ DECLARAÃ‡Ã•ES

function LuaCodegen:_emit_var_decl(node)
    self:emit_loc(node)
    if node.name then
        -- Caso Simples: var x: int = 10
        if node.is_pub then table.insert(self.pub_members, node.name) end
        local escaped_name = self:_escape_id(node.name)
        local init = node.initializer and (" = " .. self:_eval(node.initializer)) or ""
        local prefix = self.is_top_level and "" or "local "
        self:emit(string.format("%s%s%s", prefix, escaped_name, init))
    elseif node.pattern then
        -- Caso DestruturaÃ§Ã£o: var [x, y] = lista
        local init_val = self:_eval(node.initializer)
        self:emit(string.format("local _tmp = %s", init_val))
        local _, bindings = self:_gen_destructure_logic(node.pattern, "_tmp", not self.is_top_level, node.is_pub)
        for _, b in ipairs(bindings) do
            self:emit(b)
        end
    end
end

function LuaCodegen:_gen_destructure_logic(pattern, source, is_local, is_pub)
    local bindings = {}
    
    if pattern.kind == SK.IDENTIFIER_EXPR then
        if pattern.name ~= "_" then
            if is_pub then table.insert(self.pub_members, pattern.name) end
            local prefix = is_local and "local " or ""
            local escaped_name = self:_escape_id(pattern.name)
            table.insert(bindings, string.format("%s%s = %s", prefix, escaped_name, source))
        end
    elseif pattern.kind == SK.SELF_FIELD_EXPR then
        local escaped_field = self:_escape_id(pattern.field_name)
        table.insert(bindings, string.format("self.%s = %s", escaped_field, source))
    elseif pattern.kind == SK.LIST_EXPR then
        for i, el in ipairs(pattern.elements) do
            local sub_source = string.format("%s[%d]", source, i)
            local _, sub_bindings = self:_gen_destructure_logic(el, sub_source, is_local, is_pub)
            for _, b in ipairs(sub_bindings) do table.insert(bindings, b) end
        end
    elseif pattern.kind == SK.STRUCT_INIT_EXPR then
        for _, f in ipairs(pattern.fields) do
            local escaped_f = self:_escape_id(f.name)
            local sub_source = string.format("%s.%s", source, escaped_f)
            local _, sub_bindings = self:_gen_destructure_logic(f.value, sub_source, is_local, is_pub)
            for _, b in ipairs(sub_bindings) do table.insert(bindings, b) end
        end
    end
    
    return nil, bindings
end

function LuaCodegen:_emit_const_decl(node)
    if node.is_pub then table.insert(self.pub_members, node.name) end
    self:_emit_var_decl(node)
end

function LuaCodegen:_emit_state_decl(node)
    if node.is_pub then table.insert(self.pub_members, node.name) end
    local init_val = node.initializer and self:_eval(node.initializer) or "nil"
    local prefix = self.is_top_level and "" or "local "
    local escaped_name = self:_escape_id(node.name)
    self:emit(string.format("%s%s = zt.state(%s)", prefix, escaped_name, init_val))
end

function LuaCodegen:_emit_computed_decl(node)
    if node.is_pub then table.insert(self.pub_members, node.name) end
    local prefix = self.is_top_level and "" or "local "
    local escaped_name = self:_escape_id(node.name)
    self:emit(string.format("%s%s = zt.computed(function()", prefix, escaped_name))
    self:indent()
    self:emit(string.format("return %s", self:_eval(node.expression)))
    self:dedent()
    self:emit("end)")
end

function LuaCodegen:_emit_namespace_decl(node)
    self.has_namespace = true
    self.current_namespace = node.name
    self:emit(string.format("-- Namespace: %s", node.name))
end

function LuaCodegen:_emit_extern_decl(node)
    if node.is_pub then table.insert(self.pub_members, node.name) end
    -- Gera vinculaÃ§Ã£o FFI (LuaJIT pattern)
    local escaped_name = self:_escape_id(node.name)
    self:emit(string.format("local %s = zt.ffi_bind(%q)", escaped_name, node.name))
end

function LuaCodegen:_emit_type_alias_decl(node)
end

function LuaCodegen:_emit_union_decl(node)
end

function LuaCodegen:_emit_import_decl(node)
    local path = node.path
    local parts = {}
    for part in path:gmatch("[^%.]+") do table.insert(parts, part) end

    local lua_path = path:gsub("%.", "/")
    local is_std = path:match("^std%.")

    if is_std then
        lua_path = "src/stdlib/" .. lua_path:sub(5)
    end

    local name = (node.alias or parts[#parts]):gsub("%.", "_")
    self.imports[name] = true

    if name == "fast_add" then
        local member = parts[#parts]
        table.remove(parts)
        local parent_path = table.concat(parts, "/")
        if is_std then parent_path = "src/stdlib/" .. parent_path:sub(5) end
        self:emit(string.format("local %s = require(\"%s\").%s", name, parent_path, member))
    else
        self:emit(string.format("local %s = require(\"%s\")", name, lua_path))
    end
end

function LuaCodegen:_emit_func_decl(node, struct_name)
    if node.is_pub and not struct_name then table.insert(self.pub_members, node.name) end
    local name = self:_escape_id(node.name)
    if struct_name then
        name = self:_escape_id(struct_name) .. ":" .. name
    end
    
    local names = {}
    for _, p in ipairs(node.params or {}) do table.insert(names, self:_escape_id(p.name)) end
    local params_str = table.concat(names, ", ")
    
    self:emit(string.format("function %s(%s)", name, params_str))
    self:indent()

    -- LÃ³gica 'after'
    local has_after = false
    for _, stmt in ipairs(node.body) do
        if stmt.kind == SK.AFTER_STMT then
            has_after = true
            break
        end
    end

    local old_has_after = self.current_func_has_after
    if has_after then
        self:emit("local __afters = {}")
        self.current_func_has_after = true
    end

    for _, stmt in ipairs(node.body) do self:_emit_node(stmt) end

    if has_after then
        self:emit("for i=#__afters,1,-1 do __afters[i]() end")
    end
    self.current_func_has_after = old_has_after

    self:dedent()
    self:emit("end")
end

function LuaCodegen:_emit_async_func_decl(node)
    if node.is_pub then table.insert(self.pub_members, node.name) end
    local escaped_name = self:_escape_id(node.name)
    local names = {}
    for _, p in ipairs(node.params or {}) do table.insert(names, self:_escape_id(p.name)) end
    local params_str = table.concat(names, ", ")
    local call_args = #names > 0 and (", " .. params_str) or ""

    self:emit(string.format("function %s(%s) ", escaped_name, params_str))
    self:indent()
    self:emit("return zt.async_run(function(...)")
    self:indent()
    for _, stmt in ipairs(node.body) do self:_emit_node(stmt) end
    self:dedent()
    self:emit("end" .. call_args .. ")")
    self:dedent()
    self:emit("end")
end

function LuaCodegen:_emit_struct_decl(node)
    if node.is_pub then table.insert(self.pub_members, node.name) end
    local name = self:_escape_id(node.name)
    self:emit(string.format("local %s = {}", name))
    self:emit(string.format("%s.__index = %s", name, name))
    
    -- GeraÃ§Ã£o de Metadados para ReflexÃ£o
    self:emit(string.format("%s._metadata = {", name))
    self:indent()
    self:emit(string.format("name = %q,", node.name))
    
    -- Campos PÃºblicos
    self:emit("fields = {")
    self:indent()
    for _, field in ipairs(node.fields) do
        if field.is_pub then
            local type_name = field.type_annotation and field.type_annotation.name or "any"
            self:emit(string.format("{ name = %q, type = %q },", field.name, type_name))
        end
    end
    self:dedent()
    self:emit("},")

    -- MÃ©todos PÃºblicos
    self:emit("methods = {")
    self:indent()
    for _, m in ipairs(node.methods or {}) do
        if m.is_pub then
            self:emit(string.format("%q,", m.name))
        end
    end
    self:dedent()
    self:emit("}")
    
    self:dedent()
    self:emit("}")
    self:emit("")
    
    self:emit(string.format("function %s.new(fields)", name))
    self:indent()
    self:emit(string.format("local self = setmetatable({}, %s)", name))
    for _, field in ipairs(node.fields) do
        local init = field.initializer and self:_eval(field.initializer) or "nil"
        local escaped_field = self:_escape_id(field.name)
        if field.where_clause then
            self:emit(string.format("local _val = fields.%s or %s", escaped_field, init))
            local cond = self:_eval_with_it(field.where_clause, "_val")
            self:emit(string.format("if not (%s) then error(\"violacao de contrato no campo '%s' da struct '%s'\") end", cond, field.name, node.name))
            self:emit(string.format("self.%s = _val", escaped_field))
        else
            self:emit(string.format("self.%s = fields.%s or %s", escaped_field, escaped_field, init))
        end
    end
    self:emit("return self")
    self:dedent()
    self:emit("end")
end

function LuaCodegen:_emit_trait_decl(node)
    if node.is_pub then table.insert(self.pub_members, node.name) end
    local name = self:_escape_id(node.name)

    -- Trait Ã© representada como uma tabela Lua com metadados de interface
    self:emit(string.format("-- Trait: %s", node.name))
    self:emit(string.format("local %s = {}", name))
    self:emit(string.format("%s.__index = %s", name, name))
    self:emit(string.format("%s._is_trait = true", name))
    self:emit(string.format("%s._name = %q", name, node.name))

    -- Lista de mÃ©todos requeridos (sem implementaÃ§Ã£o padrÃ£o)
    local required = {}
    local defaults = {}
    for _, m in ipairs(node.methods or {}) do
        if m.body and #m.body > 0 then
            table.insert(defaults, m)
        else
            table.insert(required, string.format("%q", m.name))
        end
    end
    self:emit(string.format("%s._required = {%s}", name, table.concat(required, ", ")))

    -- ImplementaÃ§Ãµes padrÃ£o
    for _, m in ipairs(defaults) do
        self:_emit_func_decl(m, node.name)
    end
end

function LuaCodegen:_emit_apply_decl(node)
    local struct_name = self:_escape_id(node.struct_name)
    local trait_name  = self:_escape_id(node.trait_name)

    self:emit(string.format("-- apply %s to %s", node.trait_name, node.struct_name))

    -- Rastrear mÃ©todos emitidos para evitar duplicaÃ§Ã£o na passagem Struct Methods
    if not self.apply_emitted_methods then self.apply_emitted_methods = {} end
    if not self.apply_emitted_methods[node.struct_name] then
        self.apply_emitted_methods[node.struct_name] = {}
    end

    -- Emitir cada mÃ©todo como funÃ§Ã£o da metatable da struct
    for _, m in ipairs(node.methods or {}) do
        if m.body and #m.body > 0 then
            self:_emit_func_decl(m, node.struct_name)
            self.apply_emitted_methods[node.struct_name][m.name] = true
        end
    end

    -- Registrar no runtime que a struct implementa a trait
    self:emit(string.format("zt.register_trait(%s, %s)", struct_name, trait_name))
end

function LuaCodegen:_emit_enum_decl(node)
    if node.is_pub then table.insert(self.pub_members, node.name) end
    local escaped_name = self:_escape_id(node.name)
    self:emit(string.format("local %s = {", escaped_name))
    self:indent()
    for _, member in ipairs(node.members) do
        local escaped_member = self:_escape_id(member.name)
        self:emit(string.format("%s = %q,", escaped_member, member.name))
    end
    self:dedent()
    self:emit("}")
end

-- ðŸš€ STATEMENTS

function LuaCodegen:_emit_expr_stmt(node)
    self:emit_loc(node)
    self:emit(self:_eval(node.expression))
end

function LuaCodegen:_emit_assign_stmt(node)
    self:emit_loc(node)
    self:emit(string.format("%s = %s", self:_eval(node.target), self:_eval(node.value)))
end

function LuaCodegen:_emit_break_stmt(node)
    self:emit("break")
end

function LuaCodegen:_emit_continue_stmt(node)
    -- Lua 5.1/LuaJIT nÃ£o tem continue nativo. O compilador Zenith deveria usar goto ou um wrapper.
    -- Por enquanto, vamos usar goto se disponÃ­vel ou dar erro se for crÃ­tico.
    self:emit("-- [Aviso: Continue nÃ£o suportado nativamente no Lua 5.1/Luajit]")
    self:emit("goto continue")
end

function LuaCodegen:_emit_compound_assign_stmt(node)
    local op_map = { ["+="] = "+", ["-="] = "-", ["*="] = "*", ["/="] = "/" }
    local op = op_map[node.operator.lexeme]
    local target = self:_eval(node.target)
    self:emit(string.format("%s = %s %s %s", target, target, op, self:_eval(node.value)))
end

function LuaCodegen:_emit_check_stmt(node)
    local cond = self:_eval(node.condition)
    self:emit(string.format("if not (%s) then", cond))
    self:indent()
    for _, stmt in ipairs(node.else_body) do self:_emit_node(stmt) end
    self:dedent()
    self:emit("end")
end

function LuaCodegen:_emit_after_stmt(node)
    self:emit("table.insert(__afters, function()")
    self:indent()
    for _, stmt in ipairs(node.body) do self:_emit_node(stmt) end
    self:dedent()
    self:emit("end)")
end

function LuaCodegen:_emit_while_stmt(node)
    self:emit(string.format("while %s do", self:_eval(node.condition)))
    self:indent()
    for _, stmt in ipairs(node.body) do self:_emit_node(stmt) end
    self:dedent()
    self:emit("end")
end

function LuaCodegen:_emit_for_in_stmt(node)
    local vars = {}
    for _, v in ipairs(node.variables) do table.insert(vars, self:_escape_id(v.name)) end
    local vars_str = table.concat(vars, ", ")
    if node.iterable.kind == SK.RANGE_EXPR then
        local start_v = self:_eval(node.iterable.start_expr)
        local end_v = self:_eval(node.iterable.end_expr)
        self:emit(string.format("for %s = %s, %s do", vars_str, start_v, end_v))
    else
        self:emit(string.format("for %s in zt.iter(%s) do", vars_str, self:_eval(node.iterable)))
    end
    self:indent()
    for _, stmt in ipairs(node.body) do self:_emit_node(stmt) end
    self:dedent()
    self:emit("end")
end

function LuaCodegen:_emit_repeat_times_stmt(node)
    self:emit(string.format("for _ = 1, %s do", self:_eval(node.count_expr)))
    self:indent()
    for _, stmt in ipairs(node.body) do self:_emit_node(stmt) end
    self:dedent()
    self:emit("end")
end

function LuaCodegen:_emit_match_stmt(node)
    self:emit("-- [Aviso: Match nÃ£o desaÃ§ucarado]")
end

function LuaCodegen:_emit_throw_stmt(node)
    local sub = node.expression or node.expr or node.operand
    local expr = sub and self:_eval(sub) or "\"error\""
    self:emit(string.format("zt.error(%s)", expr))
end

function LuaCodegen:_emit_assert_stmt(node)
    self:emit(string.format("zt.assert(%s)", self:_eval(node.expression)))
end

function LuaCodegen:_emit_watch_stmt(node)
    self:emit("zt.watch(function()")
    self:indent()
    for _, stmt in ipairs(node.body) do self:_emit_node(stmt) end
    self:dedent()
    self:emit("end)")
end

function LuaCodegen:_emit_return_stmt(node)
    self:emit_loc(node)
    if self.current_func_has_after then
        self:emit("for i=#__afters,1,-1 do __afters[i]() end")
    end
    local val = node.expression and (" " .. self:_eval(node.expression)) or ""
    self:emit(string.format("return%s", val))
end

function LuaCodegen:_emit_if_stmt(node)
    self:emit_loc(node)
    self:emit(string.format("if %s then", self:_eval(node.condition)))
    self:indent()
    for _, stmt in ipairs(node.body) do self:_emit_node(stmt) end
    self:dedent()
    for _, elif in ipairs(node.elif_clauses or {}) do
        self:emit(string.format("elseif %s then", self:_eval(elif.condition)))
        self:indent()
        for _, stmt in ipairs(elif.body) do self:_emit_node(stmt) end
        self:dedent()
    end
    if node.else_clause then
        self:emit("else")
        self:indent()
        for _, stmt in ipairs(node.else_clause.body) do self:_emit_node(stmt) end
        self:dedent()
    end
    self:emit("end")
end

function LuaCodegen:_emit_attempt_stmt(node)
    local err_var = (node.rescue_clause and node.rescue_clause.error_name) or "err"
    local escaped_err_var = self:_escape_id(err_var)
    self:emit("local _ok, " .. escaped_err_var .. " = pcall(function()")
    self:indent()
    for _, stmt in ipairs(node.body) do self:_emit_node(stmt) end
    self:dedent()
    self:emit("end)")
    if node.rescue_clause then
        self:emit("if not _ok then")
        self:indent()
        for _, stmt in ipairs(node.rescue_clause.body) do self:_emit_node(stmt) end
        self:dedent()
        self:emit("end")
    end
end

function LuaCodegen:_emit_native_lua_stmt(node)
    self:emit("-- native lua")
    self:emit(node.lua_code)
end

-- ðŸ’Ž EXPRESSÃ•ES

function LuaCodegen:_eval(node)
    if not node then return "nil" end
    if type(node) == "table" and node.kind == "NAMED" then return self:_eval(node.value) end
    
    if node.kind == SK.LITERAL_EXPR then
        if type(node.value) == "string" then return string.format("%q", node.value) end
        return tostring(node.value)
    elseif node.kind == SK.IDENTIFIER_EXPR then
        return self:_escape_id(node.name)
    elseif node.kind == SK.SELF_EXPR then return "self"
    elseif node.kind == SK.IT_EXPR then return "it"
    elseif node.kind == SK.SELF_FIELD_EXPR then return "self." .. self:_escape_id(node.field_name)
    elseif node.kind == SK.BINARY_EXPR then
        local op_map = { ["+"] = "+", ["-"] = "-", ["*"] = "*", ["/"] = "/", ["=="] = "==", ["!="] = "~=", ["and"] = "and", ["or"] = "or", ["+"] = ".." }
        local op = node.operator.lexeme
        if op == "+" then return string.format("zt.add(%s, %s)", self:_eval(node.left), self:_eval(node.right))
        elseif op == "or" then return string.format("zt.unwrap_or(%s, %s)", self:_eval(node.left), self:_eval(node.right)) end
        return string.format("(%s %s %s)", self:_eval(node.left), op_map[op] or op, self:_eval(node.right))
    elseif node.kind == SK.UNARY_EXPR then
        local op = node.operator.lexeme == "not" and "not " or "-"
        return string.format("%s%s", op, self:_eval(node.operand))
    elseif node.kind == SK.CALL_EXPR then
        local args = {}
        -- Usa resolved_args (do Binder) que já injeta o objeto para UFCS, ou fallback para arguments
        local args_list = node.resolved_args or node.arguments
        for _, arg in ipairs(args_list) do table.insert(args, self:_eval(arg)) end
        
        if node.callee.kind == SK.MEMBER_EXPR then
            if node.callee.is_ufcs then
                -- Chamada UFCS: o objeto já está em `args`, chamamos a função globalmente
                local func_name = node.callee.ufcs_name or self:_escape_id(node.callee.member_name)
                return string.format("%s(%s)", func_name, table.concat(args, ", "))
            end
            local obj_name = node.callee.object.name
            local is_module = obj_name and self.imports[obj_name]
            local escaped_member = self:_escape_id(node.callee.member_name)
            if is_module then return string.format("%s.%s(%s)", self:_eval(node.callee.object), escaped_member, table.concat(args, ", "))
            else return string.format("%s:%s(%s)", self:_eval(node.callee.object), escaped_member, table.concat(args, ", ")) end
        elseif node.callee.kind == SK.SELF_FIELD_EXPR then
            -- Chamadas usando @name(args) ou self.name(args)
            return string.format("self:%s(%s)", self:_escape_id(node.callee.field_name), table.concat(args, ", "))
        else return string.format("%s(%s)", self:_eval(node.callee), table.concat(args, ", ")) end
    elseif node.kind == SK.MEMBER_EXPR then 
        local obj = self:_eval(node.object)
        local member = self:_escape_id(node.member_name)
        if node.is_safe then
            return string.format("zt.safe_get(%s, %q)", obj, member)
        end
        return string.format("%s.%s", obj, member)
    elseif node.kind == SK.LEN_EXPR then return string.format("#(%s)", self:_eval(node.expression))
    elseif node.kind == SK.LIST_EXPR then
        local el = {}
        for _, e in ipairs(node.elements) do table.insert(el, self:_eval(e)) end
        return "{" .. table.concat(el, ", ") .. "}"
    elseif node.kind == SK.MAP_EXPR then
        local en = {}
        for _, p in ipairs(node.pairs or {}) do table.insert(en, string.format("[%s] = %s", self:_eval(p.key), self:_eval(p.value))) end
        return "{" .. table.concat(en, ", ") .. "}"

    elseif node.kind == SK.STRUCT_INIT_EXPR then
        local fields = {}
        for _, f in ipairs(node.fields or {}) do
            table.insert(fields, string.format("[%q] = %s", self:_escape_id(f.name), self:_eval(f.value)))
        end
        return string.format("%s.new({%s})", self:_escape_id(node.type_name), table.concat(fields, ", "))

    elseif node.kind == SK.INDEX_EXPR then
        if node.index_expr and node.index_expr.kind == SK.RANGE_EXPR then
            return string.format("zt.slice(%s, %s, %s)", self:_eval(node.object), self:_eval(node.index_expr.start_expr), self:_eval(node.index_expr.end_expr))
        else
            return string.format("%s[%s]", self:_eval(node.object), self:_eval(node.index_expr))
        end
    elseif node.kind == SK.TRY_EXPR then
        -- PadrÃ£o inline: gera early-return sem pcall
        -- local __zt_try_N = <expr>
        -- if type(__zt_try_N) == "table" and (__zt_try_N._tag == "Failure" or __zt_try_N._tag == "Empty") then return __zt_try_N end
        -- <expr> â†’ (__zt_try_N.value or __zt_try_N._1)
        local tmp = self:_fresh_tmp()
        local inner = self:_eval(node.expression)
        self:emit(string.format("local %s = %s", tmp, inner))
        self:emit(string.format(
            "if type(%s) == \"table\" and (%s._tag == \"Failure\" or %s._tag == \"Empty\") then return %s end",
            tmp, tmp, tmp, tmp))
        return string.format("(%s.value or %s._1)", tmp, tmp)
    elseif node.kind == SK.BANG_EXPR then
        -- expr!  â†’  zt.bang(expr)  (panic se nil/Empty/Failure)
        return string.format("zt.bang(%s)", self:_eval(node.expression))

    elseif node.kind == SK.NATIVE_LUA_EXPR then return "(" .. node.lua_code .. ")"
    elseif node.kind == SK.AWAIT_EXPR then return string.format("zt.await(%s)", self:_eval(node.expression))
    elseif node.kind == SK.IS_EXPR then
        local type_str = "nil"
        if node.type_symbol then
            local t = node.type_symbol
            local primitives = { int=1, float=1, text=1, bool=1 }
            local name = t.base_name or t.name
            type_str = primitives[name] and ("\"" .. name .. "\"") or self:_escape_id(name)
        elseif node.type_node then
            type_str = node.type_node.name
            local primitives = { int=1, float=1, text=1, bool=1 }
            if primitives[type_str] then type_str = "\"" .. type_str .. "\"" else type_str = self:_escape_id(type_str) end
        end
        local call = string.format("zt.is(%s, %s)", self:_eval(node.expression), type_str)
        return node.is_not and ("not " .. call) or call
    elseif node.kind == SK.AS_EXPR then return self:_eval(node.expression)
    elseif node.kind == SK.LAMBDA_EXPR then
        local names = {}
        for _, p in ipairs(node.params or {}) do table.insert(names, self:_escape_id(p.name)) end
        local params_str = table.concat(names, ", ")
        
        local code = {}
        table.insert(code, string.format("function(%s)", params_str))
        
        -- Se o corpo for uma lista de statements (do...end)
        if type(node.body) == "table" and not node.body.kind then
             -- Precisamos emitir os statements. Mas _eval retorna uma string...
             -- Isso Ã© um problema. O codegen do Zenith parece misturar emissÃ£o direta com avaliaÃ§Ã£o de string.
             -- Vamos tentar usar uma funÃ§Ã£o anÃ´nima que executa os statements.
             
             -- Na verdade, o melhor Ã© mudar _eval para suportar blocos ou usar um helper.
             -- Mas dado a estrutura atual, vou emitir uma string que contÃ©m a funÃ§Ã£o completa.
             -- Para manter a indentaÃ§Ã£o, vou usar um "sub-codegen" ou capturar o output.
             
             local sub = LuaCodegen.new()
             sub.indent_level = self.indent_level + 1
             sub.imports = self.imports
             sub.pub_members = self.pub_members
             for _, stmt in ipairs(node.body) do sub:_emit_node(stmt) end
             
             local body_code = table.concat(sub.output, "\n")
             table.insert(code, body_code)
             table.insert(code, string.rep("    ", self.indent_level) .. "end")
        else
            -- Lambda de expressÃ£o Ãºnica: (x) => x + 1
            table.insert(code, string.rep("    ", self.indent_level + 1) .. "return " .. self:_eval(node.body))
            table.insert(code, string.rep("    ", self.indent_level) .. "end")
        end
        
        local final_code = table.concat(code, "\n")
        if node.is_async then
            return string.format("zt.async_run(%s)", final_code)
        end
        return final_code
    elseif node.kind == SK.GROUP_EXPR then
        return "(" .. self:_eval(node.expression) .. ")"
    end
    return "-- [[Expr:" .. node.kind .. "]]"
end

function LuaCodegen:_eval_with_it(expr, it_value)
    local old_eval = self._eval
    self._eval = function(s, node)
        if node and node.kind == SK.IT_EXPR then return it_value end
        return old_eval(s, node)
    end
    local res = self:_eval(expr)
    self._eval = old_eval
    return res
end

return LuaCodegen

end

package.preload["src.backend.lua.lua_codegen"] = package.preload["backend.lua.lua_codegen"]
package.preload["backend/lua/lua_codegen"] = package.preload["backend.lua.lua_codegen"]
package.preload["src/backend/lua/lua_codegen"] = package.preload["backend.lua.lua_codegen"]
package.preload["backend.lua.lua_codegen.init"] = package.preload["backend.lua.lua_codegen"]
package.preload["backend.lua.lua_codegen/init"] = package.preload["backend.lua.lua_codegen"]
package.preload['syntax.parser.parser_context'] = function()
-- ============================================================================
-- Zenith Compiler — Parser Context
-- Estado compartilhado do parser: tokens, posição, diagnósticos.
-- ============================================================================

local TokenKind     = require("src.syntax.tokens.token_kind")
local Span          = require("src.source.span")
local DiagnosticBag = require("src.diagnostics.diagnostic_bag")

local ParserContext = {}
ParserContext.__index = ParserContext

--- Cria um novo contexto de parser.
--- @param tokens table Lista de tokens do lexer
--- @param source_text table SourceText original
--- @return table
function ParserContext.new(tokens, source_text)
    local self = setmetatable({}, ParserContext)
    self.tokens = tokens
    self.source = source_text
    self.pos = 1
    self.diagnostics = DiagnosticBag.new()
    return self
end

--- Retorna o token atual sem avançar.
--- @return table Token
function ParserContext:current()
    if self.pos > #self.tokens then
        return self.tokens[#self.tokens]  -- EOF
    end
    return self.tokens[self.pos]
end

--- Retorna o token N posições à frente (sem avançar).
--- @param offset number
--- @return table Token
function ParserContext:peek(offset)
    local p = self.pos + (offset or 0)
    if p > #self.tokens then
        return self.tokens[#self.tokens]
    end
    if p < 1 then
        return self.tokens[1]
    end
    return self.tokens[p]
end

--- Retorna o tipo do token atual.
--- @return string TokenKind
function ParserContext:current_kind()
    return self:current().kind
end

--- Verifica se o token atual é do tipo fornecido.
--- @param kind string TokenKind
--- @return boolean
function ParserContext:check(kind)
    return self:current_kind() == kind
end

--- Verifica se o token atual é algum dos tipos fornecidos.
--- @param ... string TokenKinds
--- @return boolean
function ParserContext:check_any(...)
    local current = self:current_kind()
    for _, kind in ipairs({...}) do
        if current == kind then return true end
    end
    return false
end

--- Avança e retorna o token anterior.
--- @return table Token (o token consumido)
function ParserContext:advance()
    if not self:is_at_end() then
        self.pos = self.pos + 1
    end
    return self:peek(-1)
end

--- Retrocede o cursor do parser.
--- @param count number
function ParserContext:rewind(count)
    self.pos = self.pos - (count or 1)
    if self.pos < 1 then self.pos = 1 end
end

--- Consome um token do tipo esperado. Se não for o esperado, gera erro.
--- @param kind string TokenKind esperado
--- @param error_msg string Mensagem de erro se não encontrar
--- @return table Token
function ParserContext:expect(kind, error_msg)
    if self:check(kind) then
        return self:advance()
    end

    -- Erro: token inesperado
    local current = self:current()
    self.diagnostics:report_error(
        "ZT-P001",
        error_msg or string.format(
            "esperado '%s', encontrado '%s'",
            kind, current.kind
        ),
        current.span,
        string.format("token encontrado: '%s'", current.lexeme)
    )
    self:advance() -- Garante progresso!
    
    -- Retorna um token "fantasma" para continuar parsing
    return {
        kind = kind,
        lexeme = "",
        value = nil,
        span = current.span,
    }
end

--- Consome o token se for do tipo esperado. Retorna true/false.
--- @param kind string TokenKind
--- @return boolean
function ParserContext:match(kind)
    if self:check(kind) then
        self:advance()
        return true
    end
    return false
end

--- Verifica se atingiu o fim dos tokens.
--- @return boolean
function ParserContext:is_at_end()
    return self:current_kind() == TokenKind.EOF
end

--- Pula tokens NEWLINE ou SEMICOLON (separadores de statement).
function ParserContext:skip_newlines()
    while self:check(TokenKind.NEWLINE) or self:check(TokenKind.SEMICOLON) do
        self:advance()
    end
end

--- Pula tokens NEWLINE opcionais (um ou mais se houver).
function ParserContext:skip_optional_newlines()
    self:skip_newlines()
end

--- Espera pelo menos um NEWLINE ou EOF (terminador de statement).
--- Se não encontrar, reporta erro mas continua.
function ParserContext:expect_newline_or_eof()
    if self:check(TokenKind.NEWLINE) or self:check(TokenKind.EOF) then
        self:skip_newlines()
        return
    end
    -- Pode estar no mesmo line — reportar aviso
    -- (tolerante: não bloqueia o parsing)
end

--- Cria um span que vai do início de start_token ao final de end_token.
--- @param start_token table Token
--- @param end_token table Token
--- @return table Span
function ParserContext:span_from(start_token, end_token)
    if not end_token then
        end_token = self.tokens[self.pos - 1] or start_token
    end
    return start_token.span:merge(end_token.span)
end

--- Sincroniza após erro: avança até encontrar ponto seguro.
function ParserContext:synchronize()
    while not self:is_at_end() do
        local kind = self:current_kind()

        -- Pontos de sincronização
        if kind == TokenKind.KW_END
            or kind == TokenKind.KW_FUNC
            or kind == TokenKind.KW_STRUCT
            or kind == TokenKind.KW_TRAIT
            or kind == TokenKind.KW_ENUM
            or kind == TokenKind.KW_VAR
            or kind == TokenKind.KW_CONST
            or kind == TokenKind.KW_IF
            or kind == TokenKind.KW_WHILE
            or kind == TokenKind.KW_FOR
            or kind == TokenKind.KW_RETURN
            or kind == TokenKind.KW_IMPORT
            or kind == TokenKind.KW_EXPORT then
            return
        end

        self:advance()
    end
end

--- Verifica se o token é um identificador ou uma palavra-chave (útil para nomes de campos).
function ParserContext:is_name(token)
    if not token then return false end
    return token.kind == TokenKind.IDENTIFIER or token.kind:match("^KW_") ~= nil
end

--- Consome um identificador ou palavra-chave como nome de campo.
function ParserContext:expect_field_name(error_msg)
    local t = self:current()
    if self:is_name(t) then
        return self:advance()
    end
    return self:expect(TokenKind.IDENTIFIER, error_msg or "esperado nome do campo")
end

return ParserContext

end

package.preload["src.syntax.parser.parser_context"] = package.preload["syntax.parser.parser_context"]
package.preload["syntax/parser/parser_context"] = package.preload["syntax.parser.parser_context"]
package.preload["src/syntax/parser/parser_context"] = package.preload["syntax.parser.parser_context"]
package.preload["syntax.parser.parser_context.init"] = package.preload["syntax.parser.parser_context"]
package.preload["syntax.parser.parser_context/init"] = package.preload["syntax.parser.parser_context"]
package.preload['syntax.ast.syntax_kind'] = function()
-- ============================================================================
-- Zenith Compiler — Syntax Kind
-- Enum de todos os tipos de nó da AST.
-- ============================================================================

local SyntaxKind = {
    -- Raiz
    COMPILATION_UNIT        = "COMPILATION_UNIT",

    -- ================================================================
    -- Expressões
    -- ================================================================
    LITERAL_EXPR            = "LITERAL_EXPR",
    IDENTIFIER_EXPR         = "IDENTIFIER_EXPR",
    BINARY_EXPR             = "BINARY_EXPR",
    UNARY_EXPR              = "UNARY_EXPR",
    CALL_EXPR               = "CALL_EXPR",
    MEMBER_EXPR             = "MEMBER_EXPR",
    INDEX_EXPR              = "INDEX_EXPR",
    RANGE_EXPR              = "RANGE_EXPR",
    CHECK_EXPR              = "CHECK_EXPR",            -- check expr
    BANG_EXPR               = "BANG_EXPR",
    REST_EXPR               = "REST_EXPR",             -- .. (rest/spread)
    LEN_EXPR                = "LEN_EXPR",              -- #
    MATCH_EXPR              = "MATCH_EXPR",
    LAMBDA_EXPR             = "LAMBDA_EXPR",           -- (params) => body
    GROUP_EXPR              = "GROUP_EXPR",            -- (expr)

    LIST_EXPR               = "LIST_EXPR",             -- [a, b, c]
    MAP_EXPR                = "MAP_EXPR",              -- { "k": v }
    SELF_EXPR               = "SELF_EXPR",             -- self
    IT_EXPR                 = "IT_EXPR",               -- it
    SELF_FIELD_EXPR         = "SELF_FIELD_EXPR",       -- @field (sugar for self.field)
    STRUCT_INIT_EXPR        = "STRUCT_INIT_EXPR",      -- Type { field: value }
    AWAIT_EXPR              = "AWAIT_EXPR",            -- await expr
    IS_EXPR                 = "IS_EXPR",               -- expr is Type
    AS_EXPR                 = "AS_EXPR",               -- expr as Type
    TRY_EXPR                = "TRY_EXPR",              -- expr?
    NATIVE_LUA_EXPR         = "NATIVE_LUA_EXPR",       -- native lua ... end

    -- ================================================================
    -- Statements
    -- ================================================================
    ASSIGN_STMT             = "ASSIGN_STMT",
    COMPOUND_ASSIGN_STMT    = "COMPOUND_ASSIGN_STMT",  -- += -= *= /=
    IF_STMT                 = "IF_STMT",
    ELIF_CLAUSE             = "ELIF_CLAUSE",
    ELSE_CLAUSE             = "ELSE_CLAUSE",
    WHILE_STMT              = "WHILE_STMT",
    FOR_IN_STMT             = "FOR_IN_STMT",
    REPEAT_TIMES_STMT       = "REPEAT_TIMES_STMT",
    BREAK_STMT              = "BREAK_STMT",
    CONTINUE_STMT           = "CONTINUE_STMT",
    RETURN_STMT             = "RETURN_STMT",
    WATCH_STMT              = "WATCH_STMT",
    ATTEMPT_STMT            = "ATTEMPT_STMT",
    RESCUE_CLAUSE           = "RESCUE_CLAUSE",
    EXPR_STMT               = "EXPR_STMT",             -- expressão como statement
    MATCH_STMT              = "MATCH_STMT",            -- match expr: ... end
    THROW_STMT              = "THROW_STMT",            -- throw expr
    CHECK_STMT              = "CHECK_STMT",            -- check expr else ... end
    AFTER_STMT              = "AFTER_STMT",            -- after ...
    NATIVE_LUA_STMT         = "NATIVE_LUA_STMT",       -- native lua ... end

    -- ================================================================
    -- Declarações
    -- ================================================================
    VAR_DECL                = "VAR_DECL",
    CONST_DECL              = "CONST_DECL",
    GLOBAL_DECL             = "GLOBAL_DECL",
    STATE_DECL              = "STATE_DECL",
    COMPUTED_DECL           = "COMPUTED_DECL",
    FUNC_DECL               = "FUNC_DECL",

    ASYNC_FUNC_DECL         = "ASYNC_FUNC_DECL",
    STRUCT_DECL             = "STRUCT_DECL",
    ENUM_DECL               = "ENUM_DECL",
    TYPE_ALIAS_DECL         = "TYPE_ALIAS_DECL",
    UNION_DECL              = "UNION_DECL",
    TRAIT_DECL              = "TRAIT_DECL",
    APPLY_DECL              = "APPLY_DECL",
    NAMESPACE_DECL          = "NAMESPACE_DECL",
    IMPORT_DECL             = "IMPORT_DECL",
    EXPORT_DECL             = "EXPORT_DECL",
    REDO_DECL               = "REDO_DECL",
    EXTERN_DECL             = "EXTERN_DECL",

    -- Subnós de declaração
    PARAM_NODE              = "PARAM_NODE",            -- name: Type
    FIELD_NODE              = "FIELD_NODE",             -- pub? name: Type = default
    ATTRIBUTE_NODE          = "ATTRIBUTE_NODE",         -- @name(args)
    ENUM_MEMBER_NODE        = "ENUM_MEMBER_NODE",
    VARIANT_PATTERN         = "VARIANT_PATTERN",        -- Variant(a, b) em match

    -- ================================================================
    -- Tipos
    -- ================================================================
    NAMED_TYPE              = "NAMED_TYPE",             -- int, text, Player
    NULLABLE_TYPE           = "NULLABLE_TYPE",          -- T?
    UNION_TYPE              = "UNION_TYPE",             -- A | B
    GENERIC_TYPE            = "GENERIC_TYPE",           -- list<int>, map<text, int>
    MODIFIED_TYPE           = "MODIFIED_TYPE",          -- uniq list<int>
    FUNC_TYPE               = "FUNC_TYPE",              -- func(int, text) -> bool
    STRUCT_TYPE             = "STRUCT_TYPE",            -- struct { fields }

    -- ================================================================
    -- Match
    -- ================================================================
    MATCH_CASE              = "MATCH_CASE",
    MATCH_ELSE              = "MATCH_ELSE",

    -- ================================================================
    -- Testes
    -- ================================================================
    GROUP_DECL              = "GROUP_DECL",
    TEST_DECL               = "TEST_DECL",
    ASSERT_STMT             = "ASSERT_STMT",

    -- Erro
    BAD_NODE                = "BAD_NODE",
}

return SyntaxKind

end

package.preload["src.syntax.ast.syntax_kind"] = package.preload["syntax.ast.syntax_kind"]
package.preload["syntax/ast/syntax_kind"] = package.preload["syntax.ast.syntax_kind"]
package.preload["src/syntax/ast/syntax_kind"] = package.preload["syntax.ast.syntax_kind"]
package.preload["syntax.ast.syntax_kind.init"] = package.preload["syntax.ast.syntax_kind"]
package.preload["syntax.ast.syntax_kind/init"] = package.preload["syntax.ast.syntax_kind"]
package.preload['syntax.parser.parse_expressions'] = function()
-- ============================================================================
-- Zenith Compiler — Parse Expressions
-- Pratt Parser para expressões com precedência e associatividade.
-- ============================================================================

local TokenKind     = require("src.syntax.tokens.token_kind")
local ExprSyntax    = require("src.syntax.ast.expr_syntax")
local SK            = require("src.syntax.ast.syntax_kind")
local OperatorTable = require("src.syntax.tokens.operator_table")

local ParseExpressions = {}

--- Heurística para distinguir '<' (menor que) de '<T>' (genéricos).
function ParseExpressions._is_generic_start(ctx)
    local i = 0
    local balance = 1
    while true do
        local t = ctx:peek(i)
        if t.kind == TokenKind.EOF or t.kind == TokenKind.SEMICOLON or t.kind == TokenKind.NEWLINE then
            return false
        end
        if t.kind == TokenKind.LESS then
            balance = balance + 1
        elseif t.kind == TokenKind.GREATER then
            balance = balance - 1
            if balance == 0 then
                local next_t = ctx:peek(i + 1)
                return next_t.kind == TokenKind.LPAREN or next_t.kind == TokenKind.LBRACE
            end
        end
        i = i + 1
        if i > 30 then return false end
    end
end

--- Extrai o nome de um tipo de uma expressão (ex: 'SourceText' ou 'source.SourceText').
function ParseExpressions._extract_type_name(expr)
    if not expr then return nil end
    if expr.kind == SK.IDENTIFIER_EXPR then
        return expr.name
    elseif expr.kind == SK.MEMBER_EXPR then
        local left = ParseExpressions._extract_type_name(expr.object)
        if left then
            return left .. "." .. expr.member_name
        end
    end
    return nil
end

--- Ponto de entrada para parsing de expressões.
function ParseExpressions.parse_expression(ctx, precedence)
    precedence = precedence or 0
    local left = nil
    
    local initial_kind = ctx:peek().kind
    
    -- Operadores Prefixos
    if OperatorTable.is_unary_prefix(initial_kind) then
        local operator = ctx:advance()
        local operand = ParseExpressions.parse_expression(ctx, OperatorTable.Precedence.UNARY)
        
        if operator.kind == TokenKind.KW_CHECK then
            left = ExprSyntax.check_expr(operand, operator.span:merge(operand.span))
        elseif operator.kind == TokenKind.KW_AWAIT then
            left = ExprSyntax.await_expr(operand, operator.span:merge(operand.span))
        elseif operator.kind == TokenKind.DOT_DOT then
            left = ExprSyntax.rest(operand, operator.span:merge(operand.span))
        elseif operator.kind == TokenKind.HASH then
            left = ExprSyntax.len(operand, operator.span:merge(operand.span))
        else
            left = ExprSyntax.unary(operator, operand, operator.span:merge(operand.span))
        end
    else
        left = ParseExpressions._parse_primary(ctx)
    end

    if not left then
        local t = ctx:peek()
        ctx.diagnostics:report_error("ZT-P002", "expressão esperada, encontrado " .. t.kind, t.span)
        ctx:advance() -- Garante progresso!
        return ExprSyntax.literal(nil, "error", t.span)
    end

    -- Loop de Operadores Pós-fixos e Binários
    while true do
        local kind = ctx:peek().kind
        
        -- 1. Operadores Pós-fixos (ex: !)
        if OperatorTable.is_unary_postfix(kind) then
            local operator = ctx:advance()
            if operator.kind == TokenKind.BANG then
                left = ExprSyntax.bang(left, left.span:merge(operator.span))
            elseif operator.kind == TokenKind.QUESTION then
                left = ExprSyntax.try(left, left.span:merge(operator.span))
            end
            -- Continua para checar mais pós-fixos ou binários
        
        -- 2. Operadores Binários
        else
            local prec = OperatorTable.binary_precedence(kind)
            if prec == 0 or prec <= precedence then
                break
            end

            -- Importante: consumimos o operador agora
            local operator = ctx:advance()
            
            -- Casos Especiais de Operadores Binários
            if kind == TokenKind.DOT or kind == TokenKind.QUESTION_DOT then
                local member = ctx:expect_field_name("esperado nome do campo após '" .. operator.lexeme .. "'")
                left = ExprSyntax.member(left, member.lexeme, left.span:merge(member.span), kind == TokenKind.QUESTION_DOT)
            
            elseif kind == TokenKind.LESS and ParseExpressions._is_generic_start(ctx) then
                -- Genéricos: func<T>() ou Struct<T> {}
                local generic_args = {}
                repeat
                    local ParseTypes = require("src.syntax.parser.parse_types")
                    table.insert(generic_args, ParseTypes.parse_type(ctx))
                until not ctx:match(TokenKind.COMMA)
                ctx:expect(TokenKind.GREATER, "esperado '>' após argumentos genéricos")
                
                if ctx:check(TokenKind.LPAREN) then
                    local start_paren = ctx:advance()
                    local args = {}
                    if not ctx:check(TokenKind.RPAREN) then
                        repeat
                            ctx:skip_newlines()
                            if ctx:check(TokenKind.IDENTIFIER) and ctx:peek(1).kind == TokenKind.COLON then
                                local name_t = ctx:advance()
                                ctx:advance() -- consume ':'
                                local val = ParseExpressions.parse_expression(ctx)
                                table.insert(args, { name = name_t.lexeme, value = val, kind = "NAMED" })
                            else
                                table.insert(args, ParseExpressions.parse_expression(ctx))
                            end
                        until not ctx:match(TokenKind.COMMA)
                    end
                    local end_token = ctx:expect(TokenKind.RPAREN, "esperado ')'")
                    left = ExprSyntax.call(left, args, left.span:merge(end_token.span), generic_args)
                elseif ctx:check(TokenKind.LBRACE) then
                    local type_name = ParseExpressions._extract_type_name(left)
                    local start_brace = ctx:advance()
                    local fields = {}
                    if not ctx:check(TokenKind.RBRACE) then
                        repeat
                            ctx:skip_newlines()
                            if ctx:check(TokenKind.RBRACE) then break end
                            local name_t = ctx:expect_field_name("esperado nome do campo")
                            local f_val
                            if ctx:match(TokenKind.COLON) then
                                f_val = ParseExpressions.parse_expression(ctx)
                            else
                                f_val = ExprSyntax.identifier(name_t.lexeme, name_t.span)
                            end
                            table.insert(fields, { name = name_t.lexeme, value = f_val })
                            ctx:match(TokenKind.COMMA)
                            ctx:skip_newlines()
                        until ctx:check(TokenKind.RBRACE) or ctx:is_at_end()
                    end
                    local end_token = ctx:expect(TokenKind.RBRACE, "esperado '}'")
                    left = ExprSyntax.struct_init(type_name, fields, left.span:merge(end_token.span))
                    left.generic_args = generic_args
                else
                    ctx.diagnostics:report_error("ZT-P001", "esperado '(' ou '{' após argumentos genéricos", ctx:peek().span)
                    break
                end

            elseif kind == TokenKind.LPAREN then
                local args = {}
                if not ctx:check(TokenKind.RPAREN) then
                    repeat
                        ctx:skip_newlines()
                        if ctx:check(TokenKind.IDENTIFIER) and ctx:peek(1).kind == TokenKind.COLON then
                            local name_t = ctx:advance()
                            ctx:advance() -- consume ':'
                            local val = ParseExpressions.parse_expression(ctx)
                            table.insert(args, { name = name_t.lexeme, value = val, kind = "NAMED" })
                        else
                            table.insert(args, ParseExpressions.parse_expression(ctx))
                        end
                    until not ctx:match(TokenKind.COMMA)
                end
                local end_token = ctx:expect(TokenKind.RPAREN, "esperado ')'")
                left = ExprSyntax.call(left, args, left.span:merge(end_token.span))

            elseif kind == TokenKind.LBRACKET then
                local index = ParseExpressions.parse_expression(ctx)
                local end_token = ctx:expect(TokenKind.RBRACKET, "esperado ']'")
                left = ExprSyntax.index(left, index, left.span:merge(end_token.span))

            elseif kind == TokenKind.LBRACE then
                -- Struct Init padrão
                local type_name = ParseExpressions._extract_type_name(left)
                local fields = {}
                if not ctx:check(TokenKind.RBRACE) then
                    repeat
                        ctx:skip_newlines()
                        if ctx:check(TokenKind.RBRACE) then break end
                        local name_t = ctx:expect_field_name("esperado nome do campo")
                        local f_val
                        if ctx:match(TokenKind.COLON) then
                            f_val = ParseExpressions.parse_expression(ctx)
                        else
                            f_val = ExprSyntax.identifier(name_t.lexeme, name_t.span)
                        end
                        table.insert(fields, { name = name_t.lexeme, value = f_val })
                        ctx:match(TokenKind.COMMA)
                        ctx:skip_newlines()
                    until ctx:check(TokenKind.RBRACE) or ctx:is_at_end()
                end
                local end_token = ctx:expect(TokenKind.RBRACE, "esperado '}'")
                left = ExprSyntax.struct_init(type_name, fields, left.span:merge(end_token.span))

            elseif operator.kind == TokenKind.KW_AS then
                local ParseTypes = require("src.syntax.parser.parse_types")
                local type_node = ParseTypes.parse_type(ctx)
                left = ExprSyntax.as_expr(left, type_node, left.span:merge(type_node.span))
            
            elseif operator.kind == TokenKind.KW_IS then
                local is_not = ctx:match(TokenKind.KW_NOT)
                local ParseTypes = require("src.syntax.parser.parse_types")
                local type_node = ParseTypes.parse_type(ctx)
                left = ExprSyntax.is_expr(left, type_node, is_not, left.span:merge(type_node.span))
            
            else
                -- Operadores Binários Comuns (+, -, *, / etc)
                local assoc = OperatorTable.binary_assoc(kind)
                local right = ParseExpressions.parse_expression(ctx, prec - (assoc == "RIGHT" and 1 or 0))
                
                if not right then
                    ctx.diagnostics:report_error("ZT-P002", "esperada expressão após o operador '" .. operator.lexeme .. "'", operator.span)
                    right = ExprSyntax.literal(nil, "error", operator.span)
                end

                if kind == TokenKind.DOT_DOT then
                    left = ExprSyntax.range(left, right, left.span:merge(right.span))
                else
                    left = ExprSyntax.binary(left, operator, right, left.span:merge(right.span))
                end
            end
        end
    end

    return left
end

function ParseExpressions._parse_lambda(ctx, is_async)
    local start = ctx:peek()
    if not is_async then
        is_async = ctx:match(TokenKind.KW_ASYNC)
    end
    
    ctx:expect(TokenKind.LPAREN, "esperado '(' na lambda")
    local ParseDeclarations = require("src.syntax.parser.parse_declarations")
    local params = ParseDeclarations._parse_params(ctx)
    ctx:expect(TokenKind.RPAREN, "esperado ')' na lambda")
    ctx:expect(TokenKind.FAT_ARROW, "esperado '=>' na lambda")
    
    local body
    local span_end
    if ctx:match(TokenKind.KW_DO) then
        local ParseStatements = require("src.syntax.parser.parse_statements")
        body = ParseStatements.parse_block(ctx, TokenKind.KW_END)
        local end_t = ctx:expect(TokenKind.KW_END, "esperado 'end' ao final do bloco da lambda")
        span_end = end_t.span
    else
        body = ParseExpressions.parse_expression(ctx, OperatorTable.Precedence.ASSIGNMENT)
        span_end = body.span
    end
    return ExprSyntax.lambda(params, body, is_async, start.span:merge(span_end))
end

function ParseExpressions._parse_primary(ctx)
    local k = ctx:peek().kind

    if k == TokenKind.IDENTIFIER or k == TokenKind.KW_GRID or k == TokenKind.UNDERSCORE then
        local id = ctx:advance()
        local node = ExprSyntax.identifier(id.lexeme, id.span)
        return node
    elseif k == TokenKind.KW_SELF then
        return ExprSyntax.self_ref(ctx:advance().span)
    elseif k == TokenKind.KW_IT then
        return ExprSyntax.it_ref(ctx:advance().span)
    elseif k == TokenKind.INTEGER_LITERAL or k == TokenKind.FLOAT_LITERAL or 
           k == TokenKind.STRING_LITERAL or k == TokenKind.KW_TRUE or 
           k == TokenKind.KW_FALSE or k == TokenKind.KW_NULL then
        local t = ctx:advance()
        local ltype = "any"
        local value = t.value
        if t.kind == TokenKind.INTEGER_LITERAL then ltype = "int"
        elseif t.kind == TokenKind.FLOAT_LITERAL then ltype = "float"
        elseif t.kind == TokenKind.STRING_LITERAL then ltype = "text"
        elseif t.kind == TokenKind.KW_TRUE or t.kind == TokenKind.KW_FALSE then ltype = "bool" 
        elseif t.kind == TokenKind.KW_NULL then
            ltype = "null"
            value = nil
        end
        return ExprSyntax.literal(value, ltype, t.span)
    
    elseif k == TokenKind.LPAREN or k == TokenKind.KW_ASYNC then
        local start = ctx:peek()
        local is_async = ctx:match(TokenKind.KW_ASYNC)
        
        local is_lambda = false
        if ctx:check(TokenKind.LPAREN) then
            local i = 1
            if ctx:peek(i).kind == TokenKind.RPAREN and ctx:peek(i+1).kind == TokenKind.FAT_ARROW then
                is_lambda = true
            elseif ctx:peek(i).kind == TokenKind.IDENTIFIER then
                local next_k = ctx:peek(i+1).kind
                if next_k == TokenKind.COLON or next_k == TokenKind.COMMA or next_k == TokenKind.LBRACE then is_lambda = true end
            elseif ctx:peek(i).kind == TokenKind.LBRACKET then is_lambda = true end
        end

        if is_lambda or is_async then 
            return ParseExpressions._parse_lambda(ctx, is_async) 
        end

        ctx:advance() -- (
        local expr = ParseExpressions.parse_expression(ctx)
        ctx:expect(TokenKind.RPAREN, "esperado ')' após expressão")
        return ExprSyntax.group(expr, start.span:merge(ctx:peek(-1).span))
    
    elseif k == TokenKind.LBRACKET then
        local start = ctx:advance()
        local elements = {}
        if not ctx:check(TokenKind.RBRACKET) then
            repeat
                ctx:skip_newlines()
                if ctx:check(TokenKind.RBRACKET) then break end
                table.insert(elements, ParseExpressions.parse_expression(ctx))
                ctx:match(TokenKind.COMMA)
                ctx:skip_newlines()
            until ctx:check(TokenKind.RBRACKET) or ctx:is_at_end()
        end
        local end_t = ctx:expect(TokenKind.RBRACKET, "esperado ']'")
        return ExprSyntax.list(elements, start.span:merge(end_t.span))

    elseif k == TokenKind.LBRACE then
        local start = ctx:advance()
        local entries = {}
        if not ctx:check(TokenKind.RBRACE) then
            repeat
                ctx:skip_newlines()
                if ctx:check(TokenKind.RBRACE) then break end
                if ctx:check(TokenKind.DOT_DOT) then
                    table.insert(entries, ParseExpressions.parse_expression(ctx))
                else
                    local key = ParseExpressions.parse_expression(ctx)
                    ctx:expect(TokenKind.COLON, "esperado ':' no par chave-valor")
                    local val = ParseExpressions.parse_expression(ctx)
                    table.insert(entries, { key = key, value = val })
                end
                ctx:match(TokenKind.COMMA)
                ctx:skip_newlines()
            until ctx:check(TokenKind.RBRACE) or ctx:is_at_end()
        end
        local end_t = ctx:expect(TokenKind.RBRACE, "esperado '}'")
        return ExprSyntax.map(entries, start.span:merge(end_t.span))

    elseif k == TokenKind.AT then
        local start = ctx:advance()
        local id = ctx:expect_field_name("esperado nome do campo após '@'")
        return ExprSyntax.self_field(id.lexeme, start.span:merge(id.span))
    
    elseif k == TokenKind.KW_CHECK then
        local ParseStatements = require("src.syntax.parser.parse_statements")
        return ParseStatements._parse_check(ctx)
    elseif k == TokenKind.KW_NATIVE then
        local ParseStatements = require("src.syntax.parser.parse_statements")
        return ParseStatements._parse_native_lua(ctx, false)
    end

    return nil
end

return ParseExpressions

end

package.preload["src.syntax.parser.parse_expressions"] = package.preload["syntax.parser.parse_expressions"]
package.preload["syntax/parser/parse_expressions"] = package.preload["syntax.parser.parse_expressions"]
package.preload["src/syntax/parser/parse_expressions"] = package.preload["syntax.parser.parse_expressions"]
package.preload["syntax.parser.parse_expressions.init"] = package.preload["syntax.parser.parse_expressions"]
package.preload["syntax.parser.parse_expressions/init"] = package.preload["syntax.parser.parse_expressions"]
package.preload['syntax.parser.parser'] = function()
-- ============================================================================
-- Zenith Compiler — Parser
-- Ponto central: coordena parsing do arquivo inteiro.
-- ============================================================================

local SourceText        = require("src.source.source_text")
local Lexer             = require("src.syntax.lexer.lexer")
local ParserContext     = require("src.syntax.parser.parser_context")
local TokenKind         = require("src.syntax.tokens.token_kind")
local ParseDeclarations = require("src.syntax.parser.parse_declarations")
local DeclSyntax        = require("src.syntax.ast.decl_syntax")
local Span              = require("src.source.span")

local Parser = {}
Parser.__index = Parser

--- Faz parse de um SourceText e retorna a AST (compilation unit).
--- @param source_text table SourceText
--- @return table CompilationUnit node
--- @return table DiagnosticBag
function Parser.parse(source_text)
    -- Fase 1: Lexing
    local lexer = Lexer.new(source_text)
    local tokens = lexer:tokenize()

    -- Fase 2: Parsing
    local ctx = ParserContext.new(tokens, source_text)

    -- Merge diagnostics do lexer
    ctx.diagnostics:merge(lexer.diagnostics)

    -- Parse de todas as declarações/statements do arquivo
    local declarations = {}
    local doc_comments = {}
    ctx:skip_newlines()

    while not ctx:is_at_end() do
        if ctx:match(TokenKind.DOC_COMMENT) then
            local token = ctx:peek(-1)
            local content = token.lexeme
            
            -- Extrai @target se houver
            local target = content:match("@target:%s*([%w%.]+)")
            if target then
                table.insert(doc_comments, { target = target, content = content, span = token.span })
            else
                -- Se não tem target, fica "pendente" para a próxima declaração
                ctx.last_doc_comment = content
            end
        else
            local node = ParseDeclarations.parse_declaration_or_statement(ctx)
            if node then
                if ctx.last_doc_comment then
                    node.doc_comment = ctx.last_doc_comment
                    ctx.last_doc_comment = nil
                end
                table.insert(declarations, node)
            end
        end
        ctx:skip_newlines()
    end

    -- Criar compilation unit
    local span
    if #tokens > 0 then
        span = tokens[1].span:merge(tokens[#tokens].span)
    else
        span = Span.new(1, 0)
    end

    local unit = DeclSyntax.compilation_unit(declarations, span)
    unit.doc_comments = doc_comments -- Anexa blocos @target avulsos

    return unit, ctx.diagnostics
end

--- Atalho: parse direto de uma string.
--- @param code string Código Zenith
--- @param filename string|nil
--- @return table CompilationUnit
--- @return table DiagnosticBag
function Parser.parse_string(code, filename)
    local source = SourceText.new(code, filename or "<string>")
    local unit, diags = Parser.parse(source)
    return unit, diags, source
end

return Parser

end

package.preload["src.syntax.parser.parser"] = package.preload["syntax.parser.parser"]
package.preload["syntax/parser/parser"] = package.preload["syntax.parser.parser"]
package.preload["src/syntax/parser/parser"] = package.preload["syntax.parser.parser"]
package.preload["syntax.parser.parser.init"] = package.preload["syntax.parser.parser"]
package.preload["syntax.parser.parser/init"] = package.preload["syntax.parser.parser"]
-- ============================================================================
-- Zenith Sovereign Compiler (ztc) — Entry Point
-- Fase 1: Pipeline completo com DiagnosticBag e renderer de erros.
-- ============================================================================

local zt = require("src.backend.lua.runtime.zenith_rt")

-- ── Módulos do compilador ────────────────────────────────────────────────────
local parser      = require("syntax/parser")
local binder_mod  = require("semantic/binding/binder")
local emitter_mod = require("backend/lua/lua_codegen")
local source_mod  = require("source")

-- ── Stdlib ────────────────────────────────────────────────────────────────────
local io_lib = require("src/stdlib/io")
local os_lib = require("src/stdlib/os")

-- ── Sistema de Diagnósticos ───────────────────────────────────────────────────
local DiagnosticBag      = require("src.diagnostics.diagnostic_bag")
local DiagnosticRenderer = require("src.diagnostics.diagnostic_renderer")

-- ============================================================================
-- Helpers internos
-- ============================================================================

local function print_usage()
    io.write("Zenith Sovereign Compiler v0.3.5\n")
    io.write("Uso:  ztc <run|build|check> <arquivo.zt> [saida.lua]\n")
    io.write("      ztc <arquivo.zt>                     -- build implícito\n")
    io.write("\nModos:\n")
    io.write("  build  Compila para Lua (padrão: out.lua)\n")
    io.write("  run    Compila e executa imediatamente\n")
    io.write("  check  Análise semântica sem gerar código\n")
end

--- Lê o conteúdo de um arquivo .zt ou aborta com mensagem clara.
local function read_source(path, global_bag)
    local f, err = io.open(path, "r")
    if not f then
        global_bag:report_error("ZT-C001",
            string.format("não foi possível abrir o arquivo '%s': %s", path, err or "erro desconhecido"),
            nil)
        return nil
    end
    local content = f:read("*a")
    f:close()
    return content
end

--- Escreve o arquivo de saída ou aborta com mensagem clara.
local function write_output(path, content, global_bag)
    local f, err = io.open(path, "w")
    if not f then
        global_bag:report_error("ZT-C002",
            string.format("não foi possível escrever em '%s': %s", path, err or "erro desconhecido"),
            nil)
        return false
    end
    f:write(content)
    f:close()
    return true
end

-- ============================================================================
-- Pipeline principal de compilação
-- ============================================================================

--- Executa o pipeline completo: parse → bind → emit.
--- Retorna:
---   lua_code (string|nil), unit (AST|nil), source_text, global_bag
local function run_pipeline(input_path)
    local global_bag = DiagnosticBag.new()

    -- ── 1. Leitura do arquivo ─────────────────────────────────────────────────
    local content = read_source(input_path, global_bag)
    if not content then
        return nil, nil, nil, global_bag
    end

    -- Objeto SourceText — usado para contextualizar diagnósticos
    local source_text = source_mod.SourceText.new(content, input_path)

    -- ── 2. Parsing ────────────────────────────────────────────────────────────
    local unit, parse_diags
    local ok, parse_err = pcall(function()
        unit, parse_diags = parser.parse_string(content, input_path)
    end)

    if not ok then
        global_bag:report_error("ZT-C010",
            "erro interno no parser: " .. tostring(parse_err), nil)
        return nil, nil, source_text, global_bag
    end

    -- Mesclar diagnósticos do parser
    if parse_diags and parse_diags.diagnostics then
        global_bag:merge(parse_diags)
    end

    -- Abortar cedo se o parser encontrou erros que impedem o binding
    if global_bag:has_errors() then
        return nil, unit, source_text, global_bag
    end

    -- ── 3. Binding (análise semântica + type checking) ─────────────────────────
    local binder = binder_mod.new(global_bag)
    local ok_bind, bind_err = pcall(function()
        binder:bind(unit, "")
    end)

    if not ok_bind then
        global_bag:report_error("ZT-C020",
            "erro interno no binder: " .. tostring(bind_err), nil)
        return nil, unit, source_text, global_bag
    end

    -- REGRA DE OURO: Nunca gerar código se há erros semânticos
    if global_bag:has_errors() then
        return nil, unit, source_text, global_bag
    end

    -- ── 4. Geração de código ──────────────────────────────────────────────────
    local lua_code, source_map
    local ok_emit, emit_err = pcall(function()
        local emitter = emitter_mod.new()
        lua_code, source_map = emitter:generate(unit, source_text)
    end)

    if not ok_emit then
        global_bag:report_error("ZT-C030",
            "erro interno no gerador de código: " .. tostring(emit_err), nil)
        return nil, unit, source_text, global_bag
    end

    return lua_code, unit, source_text, global_bag, source_map
end

-- ============================================================================
-- Ponto de entrada (main)
-- ============================================================================

local function main(args)
    args = args or {}

    -- ── Sem argumentos → ajuda ────────────────────────────────────────────────
    if #args == 0 then
        print_usage()
        return 0
    end

    -- ── Parsing de argumentos ─────────────────────────────────────────────────
    local mode       = "build"
    local input_path = nil
    local out_path   = "out.lua"

    local first = args[1]
    if first == "build" or first == "run" or first == "check" then
        mode = first
        input_path = args[2]
        if args[3] then out_path = args[3] end
    else
        -- Fallback: ztc arquivo.zt [saida.lua]
        input_path = args[1]
        if args[2] then out_path = args[2] end
    end

    if not input_path then
        io.stderr:write("Erro: especifique um arquivo de entrada.\n\n")
        print_usage()
        return 1
    end

    -- ── Modo 'check': apenas análise, sem geração ─────────────────────────────
    if mode == "check" then
        local _, _, source_text, bag = run_pipeline(input_path)
        DiagnosticRenderer.print_all(bag, source_text)
        local had_errors = DiagnosticRenderer.print_summary(bag)
        if had_errors then return 1 end
        io.write(string.format("✅ %s: sem erros\n", input_path))
        return 0
    end

    -- ── Modos 'build' e 'run' ─────────────────────────────────────────────────
    local lua_code, _, source_text, bag, source_map = run_pipeline(input_path)

    -- Sempre exibir diagnósticos (erros E avisos)
    if bag:count() > 0 then
        DiagnosticRenderer.print_all(bag, source_text)
    end

    -- Se houve erros → abortar
    local had_errors = DiagnosticRenderer.print_summary(bag)
    if had_errors then
        return 1
    end

    -- ── Nenhum erro: gravar ou executar ───────────────────────────────────────
    if mode == "run" then
        -- Arquivo temporário
        local tmp = os.tmpname and os.tmpname() or ".zenith_tmp_run.lua"
        local tmp_bag = DiagnosticBag.new()
        if not write_output(tmp, lua_code, tmp_bag) then
            DiagnosticRenderer.print_all(tmp_bag, nil)
            return 1
        end
        os.execute("lua " .. tmp)
        os.remove(tmp)

    else  -- build
        local write_bag = DiagnosticBag.new()
        if not write_output(out_path, lua_code, write_bag) then
            DiagnosticRenderer.print_all(write_bag, nil)
            return 1
        end
        -- Gravar source map se disponível
        if source_map then
            local map_path = out_path .. ".map"
            local lines = {"-- Zenith Source Map", string.format("-- Source: %s", input_path), "return {"}
            lines[#lines+1] = string.format("  source = %q,", input_path)
            lines[#lines+1] = "  mappings = {"
            for lua_line, loc in pairs(source_map) do
                lines[#lines+1] = string.format("    [%d] = {line=%d, col=%d},", lua_line, loc.line, loc.col)
            end
            lines[#lines+1] = "  }"
            lines[#lines+1] = "}"
            local mf = io.open(map_path, "w")
            if mf then mf:write(table.concat(lines, "\n")); mf:close() end
        end
        io.write(string.format("✅ Compilado: %s → %s\n", input_path, out_path))
    end

    return 0
end

-- ============================================================================
-- Auto-execução
-- ============================================================================

local exit_code = main(arg or {})
os.exit(exit_code or 0)