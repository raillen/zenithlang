local SourceText = require("src.source.source_text")
local Lexer      = require("src.syntax.lexer.lexer")
local TokenKind  = require("src.syntax.tokens.token_kind")

local path = arg[1] or "tests/core/test_v02_refinement.zt"
local f = io.open(path, "r")
local content = f:read("*a")
f:close()

local source = SourceText.new(content, path)
local lexer = Lexer.new(source)
local tokens = lexer:tokenize()

for i, t in ipairs(tokens) do
    print(string.format("Token %d: kind=%s, lexeme='%s', value=%s", i, t.kind, t.lexeme, tostring(t.value)))
end
