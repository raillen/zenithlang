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
