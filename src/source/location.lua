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
