-- ============================================================================
-- Zenith Compiler — Parser
-- Ponto central: coordena parsing do arquivo inteiro.
-- ============================================================================

local SourceText        = require("src.source.source_text")
local Lexer             = require("src.syntax.lexer.lexer")
local ParserContext     = require("src.syntax.parser.parser_context")
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
    ctx:skip_newlines()

    while not ctx:is_at_end() do
        local node = ParseDeclarations.parse_declaration_or_statement(ctx)
        if node then
            table.insert(declarations, node)
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
