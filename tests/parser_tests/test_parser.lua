-- ============================================================================
-- Zenith Compiler — Parser Tests
-- Testes completos para o parser.
-- ============================================================================

package.path = package.path .. ";./src/?.lua;./tests/?.lua;./?/init.lua;./src/?/init.lua"

local runner_mod = require("tests.test_runner")
local TestRunner = runner_mod.TestRunner
local a = runner_mod.assert

local Parser     = require("src.syntax.parser.parser")
local SK         = require("src.syntax.ast.syntax_kind")

--- Helper: parse uma string e retorna as declarações da compilation unit.
local function parse(code)
    local unit, diags = Parser.parse_string(code)
    return unit.declarations, diags
end

local t = TestRunner.new()

-- ============================================================================
-- Expressões
-- ============================================================================

t:group("Expressões Básicas", function()

    t:test("literal inteiro", function()
        local decls = parse("42")
        a.equal(#decls, 1)
        a.equal(decls[1].kind, SK.EXPR_STMT)
        a.equal(decls[1].expression.kind, SK.LITERAL_EXPR)
        a.equal(decls[1].expression.value, 42)
        a.equal(decls[1].expression.literal_type, "int")
    end)

    t:test("literal float", function()
        local decls = parse("3.14")
        a.equal(decls[1].expression.value, 3.14)
        a.equal(decls[1].expression.literal_type, "float")
    end)

    t:test("literal string", function()
        local decls = parse('"hello"')
        a.equal(decls[1].expression.value, "hello")
        a.equal(decls[1].expression.literal_type, "text")
    end)

    t:test("literal true", function()
        local decls = parse("true")
        a.equal(decls[1].expression.value, true)
        a.equal(decls[1].expression.literal_type, "bool")
    end)

    t:test("literal false", function()
        local decls = parse("false")
        a.equal(decls[1].expression.value, false)
    end)

    t:test("literal null", function()
        local decls = parse("null")
        a.equal(decls[1].expression.literal_type, "null")
    end)

    t:test("identificador", function()
        local decls = parse("foo")
        a.equal(decls[1].expression.kind, SK.IDENTIFIER_EXPR)
        a.equal(decls[1].expression.name, "foo")
    end)

    t:test("self", function()
        local decls = parse("self")
        a.equal(decls[1].expression.kind, SK.SELF_EXPR)
    end)

    t:test("it", function()
        local decls = parse("it")
        a.equal(decls[1].expression.kind, SK.IT_EXPR)
    end)

    t:test("@field (self sugar)", function()
        local decls = parse("@health")
        a.equal(decls[1].expression.kind, SK.SELF_FIELD_EXPR)
        a.equal(decls[1].expression.field_name, "health")
    end)


end)

t:group("Expressões Binárias", function()

    t:test("soma: a + b", function()
        local decls = parse("1 + 2")
        local expr = decls[1].expression
        a.equal(expr.kind, SK.BINARY_EXPR)
        a.equal(expr.left.value, 1)
        a.equal(expr.operator.lexeme, "+")
        a.equal(expr.right.value, 2)
    end)

    t:test("precedência: a + b * c", function()
        local decls = parse("1 + 2 * 3")
        local expr = decls[1].expression
        -- Deve gerar: (1 + (2 * 3))
        a.equal(expr.kind, SK.BINARY_EXPR)
        a.equal(expr.operator.lexeme, "+")
        a.equal(expr.left.value, 1)
        a.equal(expr.right.kind, SK.BINARY_EXPR)
        a.equal(expr.right.operator.lexeme, "*")
    end)

    t:test("associatividade esquerda: a - b - c", function()
        local decls = parse("1 - 2 - 3")
        local expr = decls[1].expression
        -- Deve gerar: ((1 - 2) - 3)
        a.equal(expr.kind, SK.BINARY_EXPR)
        a.equal(expr.right.value, 3)
        a.equal(expr.left.kind, SK.BINARY_EXPR)
        a.equal(expr.left.left.value, 1)
    end)

    t:test("potência right-assoc: a ^ b ^ c", function()
        local decls = parse("2 ^ 3 ^ 4")
        local expr = decls[1].expression
        -- Deve gerar: (2 ^ (3 ^ 4))
        a.equal(expr.kind, SK.BINARY_EXPR)
        a.equal(expr.left.value, 2)
        a.equal(expr.right.kind, SK.BINARY_EXPR)
    end)

    t:test("comparação: a == b", function()
        local decls = parse("x == 10")
        local expr = decls[1].expression
        a.equal(expr.kind, SK.BINARY_EXPR)
        a.equal(expr.operator.lexeme, "==")
    end)

    t:test("lógico: a and b or c", function()
        local decls = parse("a and b or c")
        local expr = decls[1].expression
        -- or tem menor precedência: (a and b) or c
        a.equal(expr.operator.lexeme, "or")
        a.equal(expr.left.kind, SK.BINARY_EXPR)
        a.equal(expr.left.operator.lexeme, "and")
    end)

    t:test("range: 0..5", function()
        local decls = parse("0..5")
        local expr = decls[1].expression
        a.equal(expr.kind, SK.RANGE_EXPR)
        a.equal(expr.start_expr.value, 0)
        a.equal(expr.end_expr.value, 5)
    end)

end)

t:group("Expressões Unárias", function()

    t:test("not x", function()
        local decls = parse("not x")
        local expr = decls[1].expression
        a.equal(expr.kind, SK.UNARY_EXPR)
        a.equal(expr.operator.lexeme, "not")
        a.equal(expr.operand.name, "x")
    end)

    t:test("-x", function()
        local decls = parse("-5")
        local expr = decls[1].expression
        a.equal(expr.kind, SK.UNARY_EXPR)
        a.equal(expr.operator.lexeme, "-")
    end)

    t:test("bang: expr!", function()
        local decls = parse("x!")
        local expr = decls[1].expression
        a.equal(expr.kind, SK.BANG_EXPR)
    end)

end)

t:group("Expressões de Acesso", function()

    t:test("chamada de função: foo()", function()
        local decls = parse("foo()")
        local expr = decls[1].expression
        a.equal(expr.kind, SK.CALL_EXPR)
        a.equal(expr.callee.name, "foo")
        a.equal(#expr.arguments, 0)
    end)

    t:test("chamada com args: add(1, 2)", function()
        local decls = parse("add(1, 2)")
        local expr = decls[1].expression
        a.equal(expr.kind, SK.CALL_EXPR)
        a.equal(#expr.arguments, 2)
    end)

    t:test("acesso a membro: obj.name", function()
        local decls = parse("obj.name")
        local expr = decls[1].expression
        a.equal(expr.kind, SK.MEMBER_EXPR)
        a.equal(expr.object.name, "obj")
        a.equal(expr.member_name, "name")
    end)

    t:test("acesso por índice: list[0]", function()
        local decls = parse("list[0]")
        local expr = decls[1].expression
        a.equal(expr.kind, SK.INDEX_EXPR)
        a.equal(expr.object.name, "list")
        a.equal(expr.index_expr.kind, SK.LITERAL_EXPR)
        a.equal(expr.index_expr.value, 0)
    end)

    t:test("chain: obj.method(x).field", function()
        local decls = parse("obj.method(x).field")
        local expr = decls[1].expression
        a.equal(expr.kind, SK.MEMBER_EXPR)
        a.equal(expr.member_name, "field")
        a.equal(expr.object.kind, SK.CALL_EXPR)
    end)

    t:test("keyword contextual test como objeto", function()
        local decls = parse("test.describe(\"suite\")")
        local expr = decls[1].expression
        a.equal(expr.kind, SK.CALL_EXPR)
        a.equal(expr.callee.kind, SK.MEMBER_EXPR)
        a.equal(expr.callee.object.name, "test")
        a.equal(expr.callee.member_name, "describe")
    end)

    t:test("agrupamento: (a + b)", function()
        local decls = parse("(1 + 2)")
        local expr = decls[1].expression
        a.equal(expr.kind, SK.GROUP_EXPR)
        a.equal(expr.expression.kind, SK.BINARY_EXPR)
    end)

    t:test("lista: [1, 2, 3]", function()
        local decls = parse("[1, 2, 3]")
        local expr = decls[1].expression
        a.equal(expr.kind, SK.LIST_EXPR)
        a.equal(#expr.elements, 3)
    end)

end)

-- ============================================================================
-- Declarações
-- ============================================================================

t:group("Declarações de Variáveis", function()

    t:test("var x: int = 10", function()
        local decls = parse("var x: int = 10")
        a.equal(#decls, 1)
        a.equal(decls[1].kind, SK.VAR_DECL)
        a.equal(decls[1].name, "x")
        a.equal(decls[1].type_node.name, "int")
        a.equal(decls[1].initializer.value, 10)
    end)

    t:test("const PI: float = 3.14", function()
        local decls = parse("const PI: float = 3.14")
        a.equal(decls[1].kind, SK.CONST_DECL)
        a.equal(decls[1].name, "PI")
        a.equal(decls[1].type_node.name, "float")
    end)

    t:test("global DEBUG: bool = false", function()
        local decls = parse("global DEBUG: bool = false")
        a.equal(decls[1].kind, SK.GLOBAL_DECL)
        a.equal(decls[1].name, "DEBUG")
    end)

    t:test("state counter: int = 0", function()
        local decls = parse("state counter: int = 0")
        a.equal(decls[1].kind, SK.STATE_DECL)
        a.equal(decls[1].name, "counter")
    end)

    t:test("computed doubled: int = counter * 2", function()
        local decls = parse("computed doubled: int = counter * 2")
        a.equal(decls[1].kind, SK.COMPUTED_DECL)
        a.equal(decls[1].name, "doubled")
    end)

    t:test("tipo genérico: list<int>", function()
        local decls = parse("var scores: list<int> = [1, 2]")
        a.equal(decls[1].type_node.kind, SK.GENERIC_TYPE)
        a.equal(decls[1].type_node.base_name, "list")
        a.equal(#decls[1].type_node.type_args, 1)
    end)

    t:test("tipo nullable: int?", function()
        local decls = parse("var x: int? = null")
        a.equal(decls[1].type_node.kind, SK.NULLABLE_TYPE)
    end)

end)

t:group("Funções", function()

    t:test("func simples sem retorno", function()
        local decls = parse("func hello()\n    print(\"hi\")\nend")
        a.equal(decls[1].kind, SK.FUNC_DECL)
        a.equal(decls[1].name, "hello")
        a.equal(#decls[1].params, 0)
        a.is_nil(decls[1].return_type)
    end)

    t:test("func com parâmetros e retorno", function()
        local decls = parse("func add(a: int, b: int) -> int\n    return a + b\nend")
        a.equal(decls[1].name, "add")
        a.equal(#decls[1].params, 2)
        a.equal(decls[1].params[1].name, "a")
        a.equal(decls[1].params[1].type_node.name, "int")
        a.equal(decls[1].return_type.name, "int")
    end)

    t:test("pub func", function()
        local decls = parse("pub func greet() -> text\n    return \"hi\"\nend")
        a.equal(decls[1].kind, SK.FUNC_DECL)
        a.is_true(decls[1].is_pub)
    end)

    t:test("func com parametros multilinha", function()
        local decls = parse("pub func spawn(\n    cmd: text,\n    args: list<text> = [],\n    capture: bool = false\n) -> Outcome<text, text>\nend")
        a.equal(decls[1].kind, SK.FUNC_DECL)
        a.equal(decls[1].name, "spawn")
        a.equal(#decls[1].params, 3)
        a.equal(decls[1].params[1].name, "cmd")
        a.equal(decls[1].params[2].name, "args")
        a.equal(decls[1].params[3].name, "capture")
    end)

end)

-- ============================================================================
-- Statements
-- ============================================================================

t:group("Statements de Controle", function()

    t:test("if simples", function()
        local decls = parse("if x > 0\n    print(x)\nend")
        a.equal(decls[1].kind, SK.IF_STMT)
        a.equal(decls[1].condition.kind, SK.BINARY_EXPR)
        a.equal(#decls[1].body, 1)
    end)

    t:test("if/elif/else", function()
        local decls = parse("if x > 0\n    print(\"pos\")\nelif x == 0\n    print(\"zero\")\nelse\n    print(\"neg\")\nend")
        a.equal(decls[1].kind, SK.IF_STMT)
        a.equal(#decls[1].elif_clauses, 1)
        a.is_not_nil(decls[1].else_clause)
    end)

    t:test("while", function()
        local decls = parse("while x > 0\n    x = x - 1\nend")
        a.equal(decls[1].kind, SK.WHILE_STMT)
    end)

    t:test("for in", function()
        local decls = parse("for item in list\n    print(item)\nend")
        a.equal(decls[1].kind, SK.FOR_IN_STMT)
        a.equal(#decls[1].variables, 1)
        a.equal(decls[1].variables[1].name, "item")
    end)

    t:test("for index, item in", function()
        local decls = parse("for i, item in list\n    print(i)\nend")
        a.equal(decls[1].kind, SK.FOR_IN_STMT)
        a.equal(#decls[1].variables, 2)
        a.equal(decls[1].variables[1].name, "i")
        a.equal(decls[1].variables[2].name, "item")
    end)

    t:test("repeat N times", function()
        local decls = parse("repeat 5 times\n    print(\"boom\")\nend")
        a.equal(decls[1].kind, SK.REPEAT_TIMES_STMT)
        a.equal(decls[1].count_expr.value, 5)
    end)

    t:test("return com expressão", function()
        local decls = parse("return 42")
        a.equal(decls[1].kind, SK.RETURN_STMT)
        a.equal(decls[1].expression.value, 42)
    end)

    t:test("return sem expressão", function()
        local decls = parse("return\n")
        a.equal(decls[1].kind, SK.RETURN_STMT)
        a.is_nil(decls[1].expression)
    end)

    t:test("break", function()
        local decls = parse("break")
        a.equal(decls[1].kind, SK.BREAK_STMT)
    end)

    t:test("continue", function()
        local decls = parse("continue")
        a.equal(decls[1].kind, SK.CONTINUE_STMT)
    end)

    t:test("assert com mensagem", function()
        local decls = parse("assert(x > 0, \"x deve ser positivo\")")
        a.equal(decls[1].kind, SK.ASSERT_STMT)
        a.is_not_nil(decls[1].message)
    end)

end)

t:group("Atribuição", function()

    t:test("atribuição simples", function()
        local decls = parse("x = 10")
        a.equal(decls[1].kind, SK.ASSIGN_STMT)
        a.equal(decls[1].target.name, "x")
        a.equal(decls[1].value.value, 10)
    end)

    t:test("atribuição composta +=", function()
        local decls = parse("x += 5")
        a.equal(decls[1].kind, SK.COMPOUND_ASSIGN_STMT)
        a.equal(decls[1].operator.lexeme, "+=")
    end)

end)

-- ============================================================================
-- Tipos compostos
-- ============================================================================

t:group("Struct", function()

    t:test("struct vazio", function()
        local decls = parse("struct Point\nend")
        a.equal(decls[1].kind, SK.STRUCT_DECL)
        a.equal(decls[1].name, "Point")
    end)

    t:test("struct com campos", function()
        local decls = parse("struct Player\n    pub name: text\n    health: int = 100\nend")
        a.equal(decls[1].kind, SK.STRUCT_DECL)
        a.equal(#decls[1].fields, 2)
        a.equal(decls[1].fields[1].name, "name")
        a.is_true(decls[1].fields[1].is_pub)
        a.equal(decls[1].fields[2].name, "health")
        a.is_not_nil(decls[1].fields[2].default_value)
    end)

    t:test("struct com metodo async publico", function()
        local decls = parse("struct Worker\n    pub async func run(task: text) -> Outcome<void>\n    end\nend")
        a.equal(decls[1].kind, SK.STRUCT_DECL)
        a.equal(#decls[1].methods, 1)
        a.equal(decls[1].methods[1].kind, SK.ASYNC_FUNC_DECL)
        a.equal(decls[1].methods[1].name, "run")
        a.is_true(decls[1].methods[1].is_pub)
    end)

end)

t:group("Enum", function()

    t:test("enum simples", function()
        local decls = parse("enum Direction\n    Up\n    Down\n    Left\n    Right\nend")
        a.equal(decls[1].kind, SK.ENUM_DECL)
        a.equal(decls[1].name, "Direction")
        a.equal(#decls[1].members, 4)
    end)

end)

t:group("Trait e Apply", function()

    t:test("trait com método", function()
        local decls = parse("trait Greetable\n    pub func greet() -> text\n        return \"hi\"\n    end\nend")
        a.equal(decls[1].kind, SK.TRAIT_DECL)
        a.equal(decls[1].name, "Greetable")
        a.equal(#decls[1].methods, 1)
    end)

    t:test("apply Trait to Struct", function()
        local decls = parse("apply Greetable to Player")
        a.equal(decls[1].kind, SK.APPLY_DECL)
        a.equal(decls[1].trait_name, "Greetable")
        a.equal(decls[1].struct_name, "Player")
    end)

end)

t:group("Import e Export", function()

    t:test("import string", function()
        local decls = parse('import "math_utils"')
        a.equal(decls[1].kind, SK.IMPORT_DECL)
        a.equal(decls[1].path, "math_utils")
    end)

    t:test("import dotted: std.time", function()
        local decls = parse("import std.time")
        a.equal(decls[1].kind, SK.IMPORT_DECL)
        a.equal(decls[1].path, "std.time")
    end)

    t:test("import dotted com keyword contextual: std.test", function()
        local decls = parse("import std.test")
        a.equal(decls[1].kind, SK.IMPORT_DECL)
        a.equal(decls[1].path, "std.test")
    end)

    t:test("namespace com keyword contextual: std.test", function()
        local decls = parse("namespace std.test")
        a.equal(decls[1].kind, SK.NAMESPACE_DECL)
        a.equal(decls[1].name, "std.test")
    end)

end)

t:group("Error e Attempt", function()

    t:test("attempt rescue", function()
        local decls = parse("attempt\n    foo()\nrescue e\n    print(e)\nend")
        a.equal(decls[1].kind, SK.ATTEMPT_STMT)
        a.is_not_nil(decls[1].rescue_clause)
        a.equal(decls[1].rescue_clause.error_name, "e")
    end)

end)

t:group("Funcionalidades Avançadas", function()

    t:test("construtor genérico: grid<int>(10, 10)", function()
        local decls = parse("grid<int>(10, 10, 0)")
        local expr = decls[1].expression
        a.equal(expr.kind, SK.CALL_EXPR)
        a.equal(expr.callee.name, "grid")
        a.is_not_nil(expr.generic_args)
        a.equal(#expr.generic_args, 1)
        a.equal(expr.generic_args[1].name, "int")
    end)

    t:test("struct com where: it >= 0", function()
        local code = [[
struct Item
    quantidade: int = 1 where it >= 0
end
]]
        local decls = parse(code)
        local field = decls[1].fields[1]
        a.equal(field.name, "quantidade")
        a.is_not_nil(field.condition)
        a.equal(field.condition.kind, SK.BINARY_EXPR)
        a.equal(field.condition.left.kind, SK.IT_EXPR)
    end)

    t:test("struct com atributos: #[min(100)]", function()
        local code = [[
struct Boss
    #[min(100)]
    vida: int = 500
end
]]
        local decls = parse(code)
        local field = decls[1].fields[1]
        a.equal(#field.attributes, 1)
        a.equal(field.attributes[1].name, "min")
        a.equal(field.attributes[1].arguments[1].value, 100)
    end)

    t:test("legacy @atributo gera warning de migracao", function()
        local code = [[
struct Boss
    @min(100)
    vida: int = 500
end
]]
        local _, diags = parse(code)
        local found = false
        for _, d in ipairs(diags.diagnostics) do
            if d.code == "ZT-W003" then found = true end
        end
        a.is_true(found)
    end)

end)

t:group("Programa Completo", function()

    t:test("mini programa", function()
        local code = [[
var x: int = 10
const PI: float = 3.14

func main()
    if x > 5
        print("maior")
    else
        print("menor")
    end
end
]]
        local decls, diags = parse(code)
        a.equal(#decls, 3)
        a.equal(decls[1].kind, SK.VAR_DECL)
        a.equal(decls[2].kind, SK.CONST_DECL)
        a.equal(decls[3].kind, SK.FUNC_DECL)
        a.is_false(diags:has_errors())
    end)

end)

-- ============================================================================
-- Executar
-- ============================================================================

io.write("\n🧪 Zenith Parser Tests\n")
local success = t:report()
os.exit(success and 0 or 1)
