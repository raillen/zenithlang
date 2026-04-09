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
