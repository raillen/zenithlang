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

    t:test("nullable declaration keeps nullable node", function()
        local decls = parse("var maybe: int? = null")
        a.equal(decls[1].type_node.kind, SK.NULLABLE_TYPE)
    end)

end)

t:group("Structs Avancadas", function()

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

    t:test("declaration with hash attributes", function()
        local code = [[
#[windows, deprecated("use new_main")]
func main()
end
]]
        local decls, diags = parse(code)
        a.is_false(diags:has_errors())
        a.equal(#decls[1].attributes, 2)
        a.equal(decls[1].attributes[1].name, "windows")
        a.equal(decls[1].attributes[2].name, "deprecated")
    end)

    t:test("struct field with hash attributes", function()
        local code = [[
struct User
    #[min(18), max(120)]
    age: int
end
]]
        local decls = parse(code)
        local field = decls[1].fields[1]
        a.equal(#field.attributes, 2)
        a.equal(field.attributes[1].name, "min")
        a.equal(field.attributes[1].arguments[1].value, 18)
    end)

    t:test("struct field with validate clause", function()
        local code = [[
struct User
    age: int validate min_value(18), max_value(120)
end
]]
        local decls = parse(code)
        local field = decls[1].fields[1]
        a.is_not_nil(field.condition)
        a.equal(field.condition.kind, SK.BINARY_EXPR)
        a.equal(field.condition.operator.lexeme, "and")
    end)

    t:test("legacy @attribute emits migration warning", function()
        local code = [[
struct User
    @min(18)
    age: int
end
]]
        local _, diags = parse(code)
        local found = false
        for _, d in ipairs(diags.diagnostics) do
            if d.code == "ZT-W003" then found = true end
        end
        a.is_true(found)
    end)

    t:test("@field em metodo parseia como atribuicao para SELF_FIELD_EXPR", function()
        local code = [[
struct Player
    health: int

    func reset()
        @health = 100
    end
end
]]
        local decls, diags = parse(code)
        a.is_false(diags:has_errors())
        local stmt = decls[1].methods[1].body[1]
        a.equal(stmt.kind, SK.ASSIGN_STMT)
        a.equal(stmt.target.kind, SK.SELF_FIELD_EXPR)
        a.equal(stmt.target.field_name, "health")
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

    t:test("match type pattern", function()
        local code = [[
match value
    case text: print("text")
    else: print("other")
end
]]
        local decls = parse(code)
        a.equal(decls[1].kind, SK.MATCH_STMT)
        a.equal(decls[1].cases[1].patterns[1].kind, SK.IDENTIFIER_EXPR)
        a.equal(decls[1].cases[1].patterns[1].name, "text")
    end)

end)

t:group("Enums Genéricos", function()

    t:test("generic enum header", function()
        local code = [[
enum Optional<T>
    Present(value: T)
    Empty
end
]]
        local decls, diags = parse(code)
        a.is_false(diags:has_errors())
        a.equal(decls[1].kind, SK.ENUM_DECL)
        a.length(decls[1].generic_params, 1)
        a.equal(decls[1].generic_params[1].name, "T")
        a.equal(decls[1].members[1].params[1].type.name, "T")
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
