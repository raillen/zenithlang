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
