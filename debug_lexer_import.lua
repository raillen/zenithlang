local SourceText = require("src.source.source_text")
local Lexer      = require("src.syntax.lexer.lexer")
local TokenKind  = require("src.syntax.tokens.token_kind")

local source = SourceText.new('import "math"', "debug.zt")
local lexer = Lexer.new(source)
local tokens = lexer:tokenize()

for i, t in ipairs(tokens) do
    print(string.format("Token %d: kind=%s, lexeme='%s'", i, t.kind, t.lexeme))
end
