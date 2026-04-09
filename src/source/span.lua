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
