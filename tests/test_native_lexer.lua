local LexerModule = require("out")
local zt = require("src.backend.lua.runtime.zenith_rt")

-- Injetar os tipos no ambiente global (como o ztc faria)
local Lexer = LexerModule.Lexer or _G.Lexer
local TokenKind = LexerModule.TokenKind or _G.TokenKind

local source = [[
var x = 10
func main()
    if x > 5
        print("ok")
    end
end
]]

local lexer = Lexer.new({
    source = source,
    pos = 0,
    line = 1,
    col = 1
})

print("--- Iniciando Tokenização Nativa ---")
local tokens = lexer:tokenize()

-- Zenith v0.2 listas são 0-indexed no Zenith, mas o codegen mapeou para 0 no Lua tbm?
-- Vamos ver como o codegen lidou com 'tokens[count] = t'
-- No Lua, tokens[0] é válido, mas #tokens não contará o item 0.

local i = 0
while tokens[i] do
    local t = tokens[i]
    print(string.format("Token[%d]: %-15s | Text: %-10s | Line: %d", i, t.kind, t.text:gsub("\n", "\\n"), t.span.line))
    if t.kind == "EOF" then break end
    i = i + 1
end

print("--- Tokenização concluída com sucesso! ---")
