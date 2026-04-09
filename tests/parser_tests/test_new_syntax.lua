-- ============================================================================
-- Zenith Compiler — New Syntax Parser Tests
-- Testes para grid, uniq, where, attributes e interpolation.
-- ============================================================================

package.path = package.path .. ";./src/?.lua;./tests/?.lua;./?/init.lua;./src/?/init.lua"

local runner_mod = require("tests.test_runner")
local TestRunner = runner_mod.TestRunner
local a = runner_mod.assert

local Parser     = require("src.syntax.parser.parser")
local SK         = require("src.syntax.ast.syntax_kind")

local function parse(code)
    local unit, diags = Parser.parse_string(code)
    if diags:has_errors() then
        for _, d in ipairs(diags.diagnostics) do
            print("ERRO: " .. d.message)
        end
    end
    return unit.declarations, diags
end

local t = TestRunner.new()

t:group("Coleções e Modificadores", function()

    t:test("grid<int> declaration", function()
        local decls = parse("var m: grid<int> = grid<int>(10, 10)")
        a.equal(decls[1].type_node.kind, SK.GENERIC_TYPE)
        a.equal(decls[1].type_node.base_name, "grid")
    end)

    t:test("uniq list<text> declaration", function()
        local decls = parse("var names: uniq list<text> = []")
        a.equal(decls[1].type_node.kind, SK.MODIFIED_TYPE)
        a.equal(decls[1].type_node.modifier, "uniq")
        a.equal(decls[1].type_node.base_type.base_name, "list")
    end)

end)

t:group("Structs Avançadas", function()

    t:test("struct with where clause", function()
        local code = [[
struct Bank
    balance: int = 0 where it >= 0
end
]]
        local decls = parse(code)
        local field = decls[1].fields[1]
        a.equal(field.name, "balance")
        a.is_not_nil(field.condition)
        a.equal(field.condition.kind, SK.BINARY_EXPR)
    end)

    t:test("struct with attributes", function()
        local code = [[
struct User
    @min(18) @max(120)
    age: int
end
]]
        local decls = parse(code)
        local field = decls[1].fields[1]
        a.equal(#field.attributes, 2)
        a.equal(field.attributes[1].name, "min")
        a.equal(field.attributes[1].arguments[1].value, 18)
    end)

end)

t:group("Match Statement", function()

    t:test("match statement basic", function()
        local code = [[
match x
    case 1: print("one")
    case 2: print("two")
    else: print("other")
end
]]
        local decls = parse(code)
        a.equal(decls[1].kind, SK.MATCH_STMT)
        a.equal(#decls[1].cases, 2)
        a.is_not_nil(decls[1].else_clause)
    end)

end)

t:group("String Interpolation", function()

    t:test("interpolation with expression", function()
        -- O lexer desubuga para "A " + (expr) + " B"
        local decls = parse('"val: {1 + 1}"')
        local expr = decls[1].expression
        -- Como é desubugado para binary expressions:
        -- ("val: " + (1 + 1))
        local expr = decls[1].expression
        -- Como é desubugado para binary expressions:
        -- (("val: " + (1)) + (1)) + ""
        a.equal(expr.kind, SK.BINARY_EXPR)
        a.equal(expr.operator.lexeme, "+")
        -- O nó mais à esquerda deve ser o string literal inicial
        local leftmost = expr
        while leftmost.kind == SK.BINARY_EXPR do leftmost = leftmost.left end
        a.equal(leftmost.value, "val: ")
    end)

end)

io.write("\n🧪 Zenith New Syntax Parser Tests\n")
local success = t:report()
os.exit(success and 0 or 1)
