-- ============================================================================
-- Zenith Compiler — Lexer Tests
-- Testes completos para o lexer.
-- ============================================================================

-- Configura o package.path para encontrar os módulos src/
package.path = package.path .. ";./src/?.lua;./tests/?.lua;./?/init.lua;./src/?/init.lua"

local runner_mod = require("tests.test_runner")
local TestRunner = runner_mod.TestRunner
local assert_eq  = runner_mod.assert

local SourceText = require("src.source.source_text")
local Lexer      = require("src.syntax.lexer.lexer")
local TokenKind  = require("src.syntax.tokens.token_kind")

--- Helper: tokeniza uma string e retorna tokens (sem NEWLINE e EOF).
--- @param input string
--- @return table Lista de tokens significativos
local function lex(input)
    local source = SourceText.new(input, "<test>")
    local lexer = Lexer.new(source)
    local tokens = lexer:tokenize()
    -- Filtra NEWLINE e EOF para facilitar comparação
    local result = {}
    for _, t in ipairs(tokens) do
        if t.kind ~= TokenKind.NEWLINE and t.kind ~= TokenKind.EOF then
            table.insert(result, t)
        end
    end
    return result, lexer.diagnostics
end

--- Helper: tokeniza e retorna todos os tokens (incluindo NEWLINE e EOF).
local function lex_all(input)
    local source = SourceText.new(input, "<test>")
    local lexer = Lexer.new(source)
    return lexer:tokenize(), lexer.diagnostics
end

local t = TestRunner.new()

-- ============================================================================
-- Testes de literais numéricos
-- ============================================================================

t:group("Literais Numéricos", function()

    t:test("integer simples", function()
        local tokens = lex("42")
        assert_eq.equal(#tokens, 1)
        assert_eq.equal(tokens[1].kind, TokenKind.INTEGER_LITERAL)
        assert_eq.equal(tokens[1].value, 42)
        assert_eq.equal(tokens[1].lexeme, "42")
    end)

    t:test("integer zero", function()
        local tokens = lex("0")
        assert_eq.equal(#tokens, 1)
        assert_eq.equal(tokens[1].kind, TokenKind.INTEGER_LITERAL)
        assert_eq.equal(tokens[1].value, 0)
    end)

    t:test("float simples", function()
        local tokens = lex("3.14")
        assert_eq.equal(#tokens, 1)
        assert_eq.equal(tokens[1].kind, TokenKind.FLOAT_LITERAL)
        assert_eq.equal(tokens[1].value, 3.14)
    end)

    t:test("float começando com zero", function()
        local tokens = lex("0.5")
        assert_eq.equal(#tokens, 1)
        assert_eq.equal(tokens[1].kind, TokenKind.FLOAT_LITERAL)
        assert_eq.equal(tokens[1].value, 0.5)
    end)

    t:test("múltiplos números", function()
        local tokens = lex("10 20 30")
        assert_eq.equal(#tokens, 3)
        assert_eq.equal(tokens[1].value, 10)
        assert_eq.equal(tokens[2].value, 20)
        assert_eq.equal(tokens[3].value, 30)
    end)

    t:test("número seguido de letras gera erro", function()
        local tokens, diags = lex("10abc")
        assert_eq.equal(#tokens, 1)
        assert_eq.equal(tokens[1].kind, TokenKind.BAD_TOKEN)
        assert_eq.is_true(diags:has_errors())
    end)

end)

-- ============================================================================
-- Testes de strings
-- ============================================================================

t:group("Strings", function()

    t:test("string simples", function()
        local tokens = lex('"hello"')
        assert_eq.equal(#tokens, 1)
        assert_eq.equal(tokens[1].kind, TokenKind.STRING_LITERAL)
        assert_eq.equal(tokens[1].value, "hello")
    end)

    t:test("string vazia", function()
        local tokens = lex('""')
        assert_eq.equal(#tokens, 1)
        assert_eq.equal(tokens[1].kind, TokenKind.STRING_LITERAL)
        assert_eq.equal(tokens[1].value, "")
    end)

    t:test("string com escape \\n", function()
        local tokens = lex('"line1\\nline2"')
        assert_eq.equal(#tokens, 1)
        assert_eq.equal(tokens[1].value, "line1\nline2")
    end)

    t:test("string com escape \\t", function()
        local tokens = lex('"col1\\tcol2"')
        assert_eq.equal(#tokens, 1)
        assert_eq.equal(tokens[1].value, "col1\tcol2")
    end)

    t:test("string com aspas escapada", function()
        local tokens = lex('"she said \\"hi\\""')
        assert_eq.equal(#tokens, 1)
        assert_eq.equal(tokens[1].value, 'she said "hi"')
    end)

    t:test("string com interpolação decompõe em tokens", function()
        local tokens = lex('"Olá, {name}!"')
        -- "Olá, " + (name) + "!"
        assert_eq.equal(#tokens, 7)
        assert_eq.equal(tokens[1].kind, TokenKind.STRING_LITERAL)
        assert_eq.equal(tokens[2].kind, TokenKind.PLUS)
        assert_eq.equal(tokens[3].kind, TokenKind.LPAREN)
        assert_eq.equal(tokens[4].kind, TokenKind.IDENTIFIER)
        assert_eq.equal(tokens[5].kind, TokenKind.RPAREN)
        assert_eq.equal(tokens[6].kind, TokenKind.PLUS)
        assert_eq.equal(tokens[7].kind, TokenKind.STRING_LITERAL)
    end)

    t:test("string não terminada gera erro", function()
        local tokens, diags = lex('"unterminated')
        assert_eq.equal(tokens[1].kind, TokenKind.BAD_TOKEN)
        assert_eq.is_true(diags:has_errors())
    end)

end)

-- ============================================================================
-- Testes de keywords
-- ============================================================================

t:group("Keywords", function()

    t:test("var", function()
        local tokens = lex("var")
        assert_eq.equal(#tokens, 1)
        assert_eq.equal(tokens[1].kind, TokenKind.KW_VAR)
    end)

    t:test("const", function()
        local tokens = lex("const")
        assert_eq.equal(tokens[1].kind, TokenKind.KW_CONST)
    end)

    t:test("func", function()
        local tokens = lex("func")
        assert_eq.equal(tokens[1].kind, TokenKind.KW_FUNC)
    end)

    t:test("struct", function()
        local tokens = lex("struct")
        assert_eq.equal(tokens[1].kind, TokenKind.KW_STRUCT)
    end)

    t:test("trait", function()
        local tokens = lex("trait")
        assert_eq.equal(tokens[1].kind, TokenKind.KW_TRAIT)
    end)

    t:test("apply", function()
        local tokens = lex("apply")
        assert_eq.equal(tokens[1].kind, TokenKind.KW_APPLY)
    end)

    t:test("redo", function()
        local tokens = lex("redo")
        assert_eq.equal(tokens[1].kind, TokenKind.KW_REDO)
    end)

    t:test("if/elif/else/end", function()
        local tokens = lex("if elif else end")
        assert_eq.equal(tokens[1].kind, TokenKind.KW_IF)
        assert_eq.equal(tokens[2].kind, TokenKind.KW_ELIF)
        assert_eq.equal(tokens[3].kind, TokenKind.KW_ELSE)
        assert_eq.equal(tokens[4].kind, TokenKind.KW_END)
    end)

    t:test("true/false/null", function()
        local tokens = lex("true false null")
        assert_eq.equal(tokens[1].kind, TokenKind.KW_TRUE)
        assert_eq.equal(tokens[1].value, true)
        assert_eq.equal(tokens[2].kind, TokenKind.KW_FALSE)
        assert_eq.equal(tokens[2].value, false)
        assert_eq.equal(tokens[3].kind, TokenKind.KW_NULL)
    end)

    t:test("state computed watch", function()
        local tokens = lex("state computed watch")
        assert_eq.equal(tokens[1].kind, TokenKind.KW_STATE)
        assert_eq.equal(tokens[2].kind, TokenKind.KW_COMPUTED)
        assert_eq.equal(tokens[3].kind, TokenKind.KW_WATCH)
    end)

    t:test("group test assert (testes)", function()
        local tokens = lex("group test assert")
        assert_eq.equal(tokens[1].kind, TokenKind.KW_GROUP)
        assert_eq.equal(tokens[2].kind, TokenKind.KW_TEST)
        assert_eq.equal(tokens[3].kind, TokenKind.KW_ASSERT)
    end)

    t:test("it e self", function()
        local tokens = lex("it self")
        assert_eq.equal(tokens[1].kind, TokenKind.KW_IT)
        assert_eq.equal(tokens[2].kind, TokenKind.KW_SELF)
    end)

    t:test("identificador não é keyword", function()
        local tokens = lex("player")
        assert_eq.equal(tokens[1].kind, TokenKind.IDENTIFIER)
        assert_eq.equal(tokens[1].value, "player")
    end)

    t:test("_ é UNDERSCORE", function()
        local tokens = lex("_")
        assert_eq.equal(tokens[1].kind, TokenKind.UNDERSCORE)
    end)

    t:test("_name é identificador", function()
        local tokens = lex("_name")
        assert_eq.equal(tokens[1].kind, TokenKind.IDENTIFIER)
    end)

end)

-- ============================================================================
-- Testes de operadores
-- ============================================================================

t:group("Operadores", function()

    t:test("aritméticos simples", function()
        local tokens = lex("+ - * / % ^")
        assert_eq.equal(tokens[1].kind, TokenKind.PLUS)
        assert_eq.equal(tokens[2].kind, TokenKind.MINUS)
        assert_eq.equal(tokens[3].kind, TokenKind.STAR)
        assert_eq.equal(tokens[4].kind, TokenKind.SLASH)
        assert_eq.equal(tokens[5].kind, TokenKind.PERCENT)
        assert_eq.equal(tokens[6].kind, TokenKind.CARET)
    end)

    t:test("comparação", function()
        local tokens = lex("== != < <= > >=")
        assert_eq.equal(tokens[1].kind, TokenKind.EQUAL_EQUAL)
        assert_eq.equal(tokens[2].kind, TokenKind.BANG_EQUAL)
        assert_eq.equal(tokens[3].kind, TokenKind.LESS)
        assert_eq.equal(tokens[4].kind, TokenKind.LESS_EQUAL)
        assert_eq.equal(tokens[5].kind, TokenKind.GREATER)
        assert_eq.equal(tokens[6].kind, TokenKind.GREATER_EQUAL)
    end)

    t:test("atribuição composta", function()
        local tokens = lex("+= -= *= /=")
        assert_eq.equal(tokens[1].kind, TokenKind.PLUS_EQUAL)
        assert_eq.equal(tokens[2].kind, TokenKind.MINUS_EQUAL)
        assert_eq.equal(tokens[3].kind, TokenKind.STAR_EQUAL)
        assert_eq.equal(tokens[4].kind, TokenKind.SLASH_EQUAL)
    end)

    t:test("arrow ->", function()
        local tokens = lex("->")
        assert_eq.equal(tokens[1].kind, TokenKind.ARROW)
    end)

    t:test("range ..", function()
        local tokens = lex("..")
        assert_eq.equal(tokens[1].kind, TokenKind.DOT_DOT)
    end)

    t:test("dot simples .", function()
        local tokens = lex(".")
        assert_eq.equal(tokens[1].kind, TokenKind.DOT)
    end)

    t:test("bang !", function()
        local tokens = lex("!")
        assert_eq.equal(tokens[1].kind, TokenKind.BANG)
    end)

    t:test("question ?", function()
        local tokens = lex("?")
        assert_eq.equal(tokens[1].kind, TokenKind.QUESTION)
    end)

    t:test("pipe |", function()
        local tokens = lex("|")
        assert_eq.equal(tokens[1].kind, TokenKind.PIPE)
    end)

    t:test("delimitadores", function()
        local tokens = lex("( ) [ ] { }")
        assert_eq.equal(tokens[1].kind, TokenKind.LPAREN)
        assert_eq.equal(tokens[2].kind, TokenKind.RPAREN)
        assert_eq.equal(tokens[3].kind, TokenKind.LBRACKET)
        assert_eq.equal(tokens[4].kind, TokenKind.RBRACKET)
        assert_eq.equal(tokens[5].kind, TokenKind.LBRACE)
        assert_eq.equal(tokens[6].kind, TokenKind.RBRACE)
    end)

    t:test("pontuação", function()
        local tokens = lex(", : ;")
        assert_eq.equal(tokens[1].kind, TokenKind.COMMA)
        assert_eq.equal(tokens[2].kind, TokenKind.COLON)
        assert_eq.equal(tokens[3].kind, TokenKind.SEMICOLON)
    end)

end)

-- ============================================================================
-- Testes de comentários
-- ============================================================================

t:group("Comentários", function()

    t:test("comentário de linha é ignorado", function()
        local tokens = lex("-- isso é comentário")
        assert_eq.equal(#tokens, 0)
    end)

    t:test("código antes de comentário", function()
        local tokens = lex("42 -- número")
        assert_eq.equal(#tokens, 1)
        assert_eq.equal(tokens[1].kind, TokenKind.INTEGER_LITERAL)
        assert_eq.equal(tokens[1].value, 42)
    end)

end)

-- ============================================================================
-- Testes de edge cases
-- ============================================================================

t:group("Edge Cases", function()

    t:test("input vazio", function()
        local tokens = lex("")
        assert_eq.equal(#tokens, 0)
    end)

    t:test("apenas whitespace", function()
        local tokens = lex("   \t   ")
        assert_eq.equal(#tokens, 0)
    end)

    t:test("range entre números: 0..5", function()
        local tokens = lex("0..5")
        assert_eq.equal(#tokens, 3)
        assert_eq.equal(tokens[1].kind, TokenKind.INTEGER_LITERAL)
        assert_eq.equal(tokens[1].value, 0)
        assert_eq.equal(tokens[2].kind, TokenKind.DOT_DOT)
        assert_eq.equal(tokens[3].kind, TokenKind.INTEGER_LITERAL)
        assert_eq.equal(tokens[3].value, 5)
    end)

    t:test("declaração completa: var x: int = 10", function()
        local tokens = lex("var x: int = 10")
        assert_eq.equal(#tokens, 6)
        assert_eq.equal(tokens[1].kind, TokenKind.KW_VAR)
        assert_eq.equal(tokens[2].kind, TokenKind.IDENTIFIER)
        assert_eq.equal(tokens[2].value, "x")
        assert_eq.equal(tokens[3].kind, TokenKind.COLON)
        assert_eq.equal(tokens[4].kind, TokenKind.IDENTIFIER) -- 'int' é identifier no lexer
        assert_eq.equal(tokens[5].kind, TokenKind.EQUAL)
        assert_eq.equal(tokens[6].kind, TokenKind.INTEGER_LITERAL)
        assert_eq.equal(tokens[6].value, 10)
    end)

    t:test("declaração completa com valor", function()
        local tokens = lex("var x: int = 10")
        -- Vamos pegar todos os tokens
        assert_eq.equal(tokens[1].kind, TokenKind.KW_VAR)
        assert_eq.equal(tokens[5].kind, TokenKind.EQUAL)
    end)

    t:test("função completa", function()
        local tokens = lex("func greet(name: text) -> text")
        assert_eq.equal(tokens[1].kind, TokenKind.KW_FUNC)
        assert_eq.equal(tokens[2].kind, TokenKind.IDENTIFIER) -- greet
        assert_eq.equal(tokens[3].kind, TokenKind.LPAREN)
        assert_eq.equal(tokens[4].kind, TokenKind.IDENTIFIER) -- name
        assert_eq.equal(tokens[5].kind, TokenKind.COLON)
        assert_eq.equal(tokens[6].kind, TokenKind.IDENTIFIER) -- text
        assert_eq.equal(tokens[7].kind, TokenKind.RPAREN)
        assert_eq.equal(tokens[8].kind, TokenKind.ARROW)
        assert_eq.equal(tokens[9].kind, TokenKind.IDENTIFIER) -- text
    end)

    t:test("caractere desconhecido gera BAD_TOKEN", function()
        local tokens, diags = lex("§")
        -- § é multibyte (2 bytes), gera 2 BAD_TOKENs
        assert_eq.is_true(#tokens >= 1)
        assert_eq.equal(tokens[1].kind, TokenKind.BAD_TOKEN)
        assert_eq.is_true(diags:has_errors())
    end)

    t:test("newlines são tokenizados", function()
        local tokens = lex_all("a\nb")
        -- a, NEWLINE, b, EOF
        assert_eq.equal(tokens[1].kind, TokenKind.IDENTIFIER)
        assert_eq.equal(tokens[2].kind, TokenKind.NEWLINE)
        assert_eq.equal(tokens[3].kind, TokenKind.IDENTIFIER)
        assert_eq.equal(tokens[4].kind, TokenKind.EOF)
    end)

    t:test("span correto para primeiro token", function()
        local tokens = lex("hello")
        assert_eq.equal(tokens[1].span.start, 1)
        assert_eq.equal(tokens[1].span.length, 5)
    end)

    t:test("span correto para segundo token", function()
        local tokens = lex("ab cd")
        assert_eq.equal(tokens[2].span.start, 4)
        assert_eq.equal(tokens[2].span.length, 2)
    end)

end)

-- ============================================================================
-- Testes de integração léxica
-- ============================================================================

t:group("Integração Léxica", function()

    t:test("struct completo", function()
        local input = 'struct Player\n    pub name: text\n    health: int = 100\nend'
        local tokens = lex(input)
        assert_eq.equal(tokens[1].kind, TokenKind.KW_STRUCT)
        assert_eq.equal(tokens[2].kind, TokenKind.IDENTIFIER) -- Player
        assert_eq.equal(tokens[3].kind, TokenKind.KW_PUB)
        -- ... continua
    end)

    t:test("trait com método", function()
        local input = 'trait Flyable\n    pub func fly() -> text\n        return "voando"\n    end\nend'
        local tokens = lex(input)
        assert_eq.equal(tokens[1].kind, TokenKind.KW_TRAIT)
        assert_eq.equal(tokens[2].kind, TokenKind.IDENTIFIER) -- Flyable
    end)

    t:test("match com cases", function()
        local input = 'match dir:\n    case Up: print("up")\n    else: print("other")\nend'
        local tokens = lex(input)
        assert_eq.equal(tokens[1].kind, TokenKind.KW_MATCH)
    end)

end)

-- ============================================================================
-- Executar
-- ============================================================================

io.write("\n🧪 Zenith Lexer Tests\n")
local success = t:report()
os.exit(success and 0 or 1)
