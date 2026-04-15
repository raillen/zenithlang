-- ============================================================================
-- Zenith Compiler — Diagnostic Renderer
-- Formata erros e avisos no terminal com contexto de código.
-- Inspirado no estilo de saída do Rust Compiler (rustc).
--
-- Formato de saída:
--   error[ZT-S100]: tipo incompatível na atribuição
--     --> src/main.zt:5:14
--      |
--    5 |     var age: int = "vinte"
--      |                    ^^^^^^^ esperado `int`, encontrado `text`
--      |
--      = dica: use um valor inteiro ou converta explicitamente
-- ============================================================================

local DiagnosticRenderer = {}

-- ============================================================================
-- ANSI Color Codes (desabilitados automaticamente em sistemas sem suporte)
-- ============================================================================

local function supports_color()
    -- No Windows, checar se é terminal moderno (WT, VS Code) ou redirecionar
    -- Em caso de dúvida, desabilitar para evitar lixo visual
    local term = os.getenv("TERM") or ""
    local wt = os.getenv("WT_SESSION")
    local no_color = os.getenv("NO_COLOR")
    if no_color then return false end
    if wt then return true end  -- Windows Terminal
    if term == "xterm-256color" or term == "screen-256color" or term == "xterm" then return true end
    return false
end

local USE_COLOR = supports_color()

local C = {
    reset   = USE_COLOR and "\27[0m"  or "",
    bold    = USE_COLOR and "\27[1m"  or "",
    red     = USE_COLOR and "\27[31m" or "",
    yellow  = USE_COLOR and "\27[33m" or "",
    blue    = USE_COLOR and "\27[34m" or "",
    cyan    = USE_COLOR and "\27[36m" or "",
    white   = USE_COLOR and "\27[37m" or "",
    -- Combinados
    bold_red    = USE_COLOR and "\27[1;31m" or "",
    bold_yellow = USE_COLOR and "\27[1;33m" or "",
    bold_blue   = USE_COLOR and "\27[1;34m" or "",
    bold_cyan   = USE_COLOR and "\27[1;36m" or "",
    bold_white  = USE_COLOR and "\27[1;37m" or "",
}

-- ============================================================================
-- Utilitários internos
-- ============================================================================

local function severity_color(severity)
    if severity == "error"   then return C.bold_red    end
    if severity == "warning" then return C.bold_yellow end
    if severity == "hint"    then return C.bold_cyan   end
    return C.bold_white
end

local function severity_label(severity)
    if severity == "error"   then return "error"   end
    if severity == "warning" then return "warning" end
    if severity == "hint"    then return "dica"    end
    if severity == "info"    then return "info"    end
    return severity
end

--- Extrai a linha e coluna de um span usando o SourceText.
--- Retorna linha (number), coluna (number), conteúdo da linha (string)
local function get_location_info(span, source_text)
    if not span or not source_text then
        return 1, 1, ""
    end

    local ok_loc, loc = pcall(function()
        return source_text:get_location(span.start or span[1] or 1)
    end)
    if not ok_loc then
        loc = { line = 1, column = 1 }
    end

    local line_content = ""
    local ok_line, line_result = pcall(function()
        return source_text:get_line(loc.line)
    end)
    if ok_line then
        line_content = line_result or ""
    end

    return loc.line, loc.column, line_content
end

--- Calcula o comprimento do span em caracteres.
local function span_length(span)
    if not span then return 1 end
    local s = span.start_pos or span.start or span[1] or 0
    local e = span.end_pos or span.finish or span[2]
    if type(e) == "function" then e = nil end
    if type(span["end"]) == "number" then e = span["end"] end
    e = e or s
    return math.max(1, e - s)
end

--- Gera a linha de ponteiro (^^^^^^) com o comprimento correto.
local function make_pointer(col, length, severity)
    local pad = string.rep(" ", math.max(0, col - 1))
    local ptr = string.rep("^", math.min(length, 80))  -- limitado para não quebrar terminal
    return pad .. C[severity == "warning" and "bold_yellow" or "bold_red"] .. ptr .. C.reset
end

-- ============================================================================
-- Renderização de um Diagnostic individual
-- ============================================================================

--- Renderiza um único diagnóstico em texto formatado.
--- @param diag table Diagnostic
--- @param source_text table|nil SourceText (opcional — sem ele, sem contexto de código)
--- @return string
function DiagnosticRenderer.render_one(diag, source_text)
    local parts = {}

    -- ── Linha 1: Cabeçalho  error[ZT-S100]: mensagem
    local sev_color = severity_color(diag.severity)
    local sev_label = severity_label(diag.severity)
    local header = string.format("%s%s%s[%s]%s: %s%s%s",
        sev_color, sev_label, C.reset,
        diag.code or "ZT-???",
        "",
        C.bold_white, diag.message or "(sem mensagem)", C.reset)
    table.insert(parts, header)

    -- ── Linha 2: Localização  --> arquivo:linha:coluna
    if diag.span and source_text then
        local line, col, line_content = get_location_info(diag.span, source_text)
        local filename = source_text.filename or "<entrada>"

        local location = string.format("  %s-->%s %s:%d:%d",
            C.bold_blue, C.reset, filename, line, col)
        table.insert(parts, location)

        -- ── Contexto de código (3 linhas: blank, code, pointer)
        local gutter = tostring(line)
        local gutter_w = #gutter
        local blank_gutter = string.rep(" ", gutter_w)

        -- linha em branco antes
        table.insert(parts, string.format(" %s%s |%s", C.bold_blue, blank_gutter, C.reset))

        -- linha de código
        table.insert(parts, string.format(" %s%s |%s %s",
            C.bold_blue, gutter, C.reset, line_content))

        -- linha de ponteiro
        local ptr = make_pointer(col, span_length(diag.span), diag.severity)
        table.insert(parts, string.format(" %s%s |%s %s",
            C.bold_blue, blank_gutter, C.reset, ptr))

        -- linha em branco depois
        table.insert(parts, string.format(" %s%s |%s", C.bold_blue, blank_gutter, C.reset))
    end

    -- ── Dica (se houver)
    local hint_text = rawget(diag, "hint")
    if type(hint_text) == "string" then
        table.insert(parts, string.format("  %s=%s %sdica%s: %s",
            C.bold_blue, C.reset, C.bold_white, C.reset, hint_text))
    end

    return table.concat(parts, "\n")
end

-- ============================================================================
-- Renderização de um DiagnosticBag completo
-- ============================================================================

--- Renderiza todos os diagnósticos no bag.
--- @param bag table DiagnosticBag
--- @param source_text table|nil SourceText
--- @return string Texto completo formatado
function DiagnosticRenderer.render(bag, source_text)
    if not bag or #bag.diagnostics == 0 then return "" end

    local rendered = {}
    for _, diag in ipairs(bag.diagnostics) do
        table.insert(rendered, DiagnosticRenderer.render_one(diag, source_text))
    end

    return table.concat(rendered, "\n\n")
end

--- Imprime todos os diagnósticos diretamente no stderr.
--- @param bag table DiagnosticBag
--- @param source_text table|nil SourceText
function DiagnosticRenderer.print_all(bag, source_text)
    if not bag or #bag.diagnostics == 0 then return end

    local output = DiagnosticRenderer.render(bag, source_text)
    io.stderr:write(output .. "\n")
end

--- Imprime o resumo final: "aborting due to N error(s)"
--- @param bag table DiagnosticBag
--- @return boolean true se houve erros
function DiagnosticRenderer.print_summary(bag)
    if not bag then return false end

    local error_count = 0
    local warning_count = 0
    for _, d in ipairs(bag.diagnostics) do
        if d.severity == "error" then error_count = error_count + 1
        elseif d.severity == "warning" then warning_count = warning_count + 1
        end
    end

    if warning_count > 0 then
        io.stderr:write(string.format("\n%s%d aviso(s) gerado(s)%s\n",
            C.bold_yellow, warning_count, C.reset))
    end

    if error_count > 0 then
        io.stderr:write(string.format(
            "\n%scompilação abortada: %d erro(s) encontrado(s)%s\n",
            C.bold_red, error_count, C.reset))
        return true
    end

    return false
end

return DiagnosticRenderer
