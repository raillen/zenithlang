local SourceText = require("src.source.source_text")
local Lexer      = require("src.syntax.lexer.lexer")

local code = 'var s = "\\{ \\"nome\\": \\"Zenith\\" \\}"'
local source = SourceText.new(code, "test.zt")
local lexer = Lexer.new(source)
local tokens = lexer:tokenize()

for i, t in ipairs(tokens) do
    print(i .. ": " .. t.kind .. " [" .. t.lexeme .. "]")
end
