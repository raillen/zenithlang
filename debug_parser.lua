local SourceText = require("src.source.source_text")
local Parser     = require("src.syntax.parser.parser")
local TokenKind  = require("src.syntax.tokens.token_kind")

local code = "42"
local source = SourceText.new(code, "debug.zt")

print("--- INICIANDO PARSE ---")
local unit, diags = Parser.parse(source)
print("--- PARSE FINALIZADO ---")

if diags:has_errors() then
    print("DIAGS FORMATADOS:")
    print(diags:format(source))
end

print("Unit Kind: " .. tostring(unit.kind))
print("Unit Span: " .. tostring(unit.span))
print("Decls Count: " .. #unit.declarations)

if #unit.declarations > 0 then
    local d = unit.declarations[1]
    print("Decl 1 Kind: " .. tostring(d.kind))
    if d.expression then
        print("Expr Value: " .. tostring(d.expression.value))
    end
end
