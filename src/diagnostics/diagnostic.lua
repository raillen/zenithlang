-- ============================================================================
-- Zenith Compiler — Diagnostic
-- Estrutura de erro/aviso/dica com span e mensagem.
-- ============================================================================

local Diagnostic = {}
Diagnostic.__index = Diagnostic

--- Severidades possíveis.
Diagnostic.Severity = {
    ERROR   = "error",
    WARNING = "warning",
    HINT    = "hint",
    INFO    = "info",
}

--- Cria um novo Diagnostic.
--- @param severity string Uma das severidades
--- @param code string Código do erro (ex: "ZT-001")
--- @param message string Mensagem do erro
--- @param span table Span no código-fonte
--- @param hint string|nil Dica opcional
--- @return table
function Diagnostic.new(severity, code, message, span, hint)
    local self = setmetatable({}, Diagnostic)
    self.severity = severity
    self.code = code
    self.message = message
    self.span = span
    self.hint = hint
    return self
end

--- Cria um erro.
--- @param code string
--- @param message string
--- @param span table
--- @param hint string|nil
--- @return table
function Diagnostic.error(code, message, span, hint)
    return Diagnostic.new(Diagnostic.Severity.ERROR, code, message, span, hint)
end

--- Cria um aviso.
--- @param code string
--- @param message string
--- @param span table
--- @param hint string|nil
--- @return table
function Diagnostic.warning(code, message, span, hint)
    return Diagnostic.new(Diagnostic.Severity.WARNING, code, message, span, hint)
end

--- Cria uma dica.
--- @param code string
--- @param message string
--- @param span table
--- @param hint string|nil
--- @return table
function Diagnostic.hint(code, message, span, hint)
    return Diagnostic.new(Diagnostic.Severity.HINT, code, message, span, hint)
end

--- Retorna o ícone da severidade.
--- @return string
function Diagnostic:icon()
    local icons = {
        [Diagnostic.Severity.ERROR]   = "❌",
        [Diagnostic.Severity.WARNING] = "⚠️",
        [Diagnostic.Severity.HINT]    = "💡",
        [Diagnostic.Severity.INFO]    = "ℹ️",
    }
    return icons[self.severity] or "?"
end

--- Retorna o label da severidade.
--- @return string
function Diagnostic:label()
    local labels = {
        [Diagnostic.Severity.ERROR]   = "Erro",
        [Diagnostic.Severity.WARNING] = "Aviso",
        [Diagnostic.Severity.HINT]    = "Dica",
        [Diagnostic.Severity.INFO]    = "Info",
    }
    return labels[self.severity] or "?"
end

--- Verifica se é erro.
--- @return boolean
function Diagnostic:is_error()
    return self.severity == Diagnostic.Severity.ERROR
end

--- Representação textual simples.
--- @return string
function Diagnostic:__tostring()
    local s = string.format("%s %s [%s]: %s",
        self:icon(), self:label(), self.code, self.message)
    if self.hint then
        s = s .. string.format("\n💡 Dica: %s", tostring(self.hint))
    end
    return s
end

return Diagnostic
