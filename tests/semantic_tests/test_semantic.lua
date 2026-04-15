-- ============================================================================
-- Zenith Compiler — Semantic Tests
-- Testes para Binder, Scopes e Type Checking.
-- ============================================================================

package.path = package.path .. ";./src/?.lua;./tests/?.lua;./?/init.lua;./src/?/init.lua"

local runner_mod = require("tests.test_runner")
local TestRunner = runner_mod.TestRunner
local a = runner_mod.assert

local Parser     = require("src.syntax.parser.parser")
local Binder     = require("src.semantic.binding.binder")
local ModuleManager = require("src.semantic.binding.module_manager")
local BuiltinTypes = require("src.semantic.types.builtin_types")

--- Helper: Analisa uma string e retorna o escopo e os diagnósticos.
local function analyze(code)
    local unit, diags = Parser.parse_string(code)
    -- Se o parser falhar, nem tenta o binder
    if diags:has_errors() then return nil, diags, BuiltinTypes.ERROR end
    
    local binder = Binder.new(diags)
    local scope, final_diags, last_type = binder:bind(unit)
    return scope, final_diags, last_type
end

local function analyze_with_modules(code, module_name)
    local unit, diags = Parser.parse_string(code, module_name or "semantic_test.zt")
    if diags:has_errors() then return nil, diags, BuiltinTypes.ERROR end

    local binder = Binder.new(diags, ModuleManager.new("."))
    local scope, final_diags, last_type = binder:bind(unit, module_name or "semantic_test")
    return scope, final_diags, last_type
end

local t = TestRunner.new()

-- ============================================================================
-- Testes de Variáveis e Tipagem
-- ============================================================================

t:group("Semântica: Variáveis", function()

    t:test("declaração válida: var x: int = 10", function()
        local scope, diags, last_type = analyze("var x: int = 10")
        a.is_false(diags:has_errors())
        a.equal(last_type, BuiltinTypes.INT)
        local sym = scope:lookup("x")
        a.is_not_nil(sym)
        a.equal(sym.type_info, BuiltinTypes.INT)
    end)

    t:test("erro: tipo incompatível (int = text)", function()
        local _, diags = analyze('var x: int = "texto"')
        a.is_true(diags:has_errors())
        -- Deve conter erro ZT-S100
        local found = false
        for _, d in ipairs(diags.diagnostics) do
            if d.code == "ZT-S100" then found = true end
        end
        a.is_true(found)
    end)

    t:test("promoção: float = int (permitido)", function()
        local _, diags = analyze('var x: float = 10')
        a.is_false(diags:has_errors())
    end)

    t:test("lista literal homogênea respeita o tipo declarado", function()
        local _, diags = analyze('var xs: list<int> = [1, 2, 3]')
        a.is_false(diags:has_errors())
    end)

    t:test("lista literal promove int para float quando necessário", function()
        local _, diags = analyze('var xs: list<float> = [1, 2.5]')
        a.is_false(diags:has_errors())
    end)

    t:test("lista literal mista falha contra list<int>", function()
        local _, diags = analyze('var xs: list<int> = [1, "dois"]')
        a.is_true(diags:has_errors())
        local found = false
        for _, d in ipairs(diags.diagnostics) do
            if d.code == "ZT-S100" then found = true end
        end
        a.is_true(found)
    end)

    t:test("map literal homogêneo respeita o tipo declarado", function()
        local _, diags = analyze('var m: map<text, int> = { "a": 1, "b": 2 }')
        a.is_false(diags:has_errors())
    end)

    t:test("map literal com valor incompatível falha contra map<text, int>", function()
        local _, diags = analyze('var m: map<text, int> = { "a": 1, "b": "dois" }')
        a.is_true(diags:has_errors())
        local found = false
        for _, d in ipairs(diags.diagnostics) do
            if d.code == "ZT-S100" then found = true end
        end
        a.is_true(found)
    end)

    t:test("redefinição no mesmo escopo (erro)", function()
        local _, diags = analyze('var x: int = 1\nvar x: int = 2')
        a.is_true(diags:has_errors())
    end)

    t:test("símbolo não encontrado", function()
        local _, diags = analyze('var x: int = y')
        a.is_true(diags:has_errors())
    end)

end)

-- ============================================================================
-- Testes de Operações
-- ============================================================================

t:group("Semântica: Operadores", function()

    t:test("soma de inteiros", function()
        local _, diags = analyze('var x: int = 10 + 20')
        a.is_false(diags:has_errors())
    end)

    t:test("concatenação com texto", function()
        local _, diags = analyze('var s: text = "oi " + 10')
        a.is_false(diags:has_errors())
    end)

    t:test("erro: operador inválido (bool + int)", function()
        local _, diags = analyze('var x: int = true + 10')
        a.is_true(diags:has_errors())
    end)

end)

-- ============================================================================
-- Testes de Funções
-- ============================================================================

t:group("Semântica: Funções", function()

    t:test("chamada de função simples", function()
        local code = [[
func greet(name: text)
    var msg: text = "Olá " + name
end
]]
        local _, diags = analyze(code)
        a.is_false(diags:has_errors())
    end)

    t:test("escopo local de função", function()
        local code = [[
func test()
    var local_v: int = 10
end
var global_v: int = local_v
]]
        local _, diags = analyze(code)
        a.is_true(diags:has_errors()) -- local_v não deve vazar
    end)

end)

-- ============================================================================
-- Testes de Structs e Açúcar Sintático
-- ============================================================================

t:group("Semântica: Structs e Sugar", function()

    t:test("declaração de struct e campo", function()
        local code = [[
struct Point
    pub x: int
    pub y: int
end
]]
        local _, diags = analyze(code)
        a.is_false(diags:has_errors())
    end)

    t:test("autodesreferência: @field (sugar)", function()
        local code = [[
    struct Player
    pub health: int
    pub func reset()
        @health = 100
    end
    end
    ]]
        local _, diags = analyze(code)
        a.is_false(diags:has_errors())
    end)

    t:test("erro: @field fora de struct", function()
        local code = [[
    func fail()
    @health = 100
    end
    ]]
        local _, diags = analyze(code)
        a.is_true(diags:has_errors())
        local found = false
        for _, d in ipairs(diags.diagnostics) do
            if d.code == "ZT-S201" then found = true end
        end
        a.is_true(found)
    end)


    t:test("uso do 'it'", function()
        local code = [[
var mylist: any = [1, 2, 3]
mylist.filter(it > 1)
]]
        local _, diags = analyze(code)
        a.is_false(diags:has_errors())
    end)

    t:test("inicialização de struct", function()
        local code = [[
struct Vec2
    pub x: int
    pub y: int
end
var v: Vec2 = Vec2 { x: 10, y: 20 }
]]
        local _, diags = analyze(code)
        a.is_false(diags:has_errors())
    end)

    t:test("valor padrao de campo respeita o tipo", function()
        local code = [[
struct Counter
    value: int = 1
end
]]
        local _, diags = analyze(code)
        a.is_false(diags:has_errors())
    end)

    t:test("valor padrao de campo incompatível gera erro", function()
        local code = [[
struct Counter
    value: int = "um"
end
]]
        local _, diags = analyze(code)
        a.is_true(diags:has_errors())
        local found = false
        for _, d in ipairs(diags.diagnostics) do
            if d.code == "ZT-S100" then found = true end
        end
        a.is_true(found)
    end)

    t:test("validate local exige predicado booleano", function()
        local code = [[
func invalid_contract(it: int) -> int
    return it
end

struct Item
    quantidade: int validate invalid_contract
end
]]
        local _, diags = analyze(code)
        a.is_true(diags:has_errors())
        local found = false
        for _, d in ipairs(diags.diagnostics) do
            if d.code == "ZT-S102" then found = true end
        end
        a.is_true(found)
    end)

    t:test("validate com std.validation funciona via import", function()
        local code = [[
import std.validation as validation

struct User
    age: int validate validation.min_value(18), validation.max_value(120)
end

var user: User = User { age: 30 }
]]
        local _, diags = analyze_with_modules(code, "semantic_validation_test")
        a.is_false(diags:has_errors())
    end)

end)

-- ============================================================================
-- Executar
-- ============================================================================

io.write("\n🧪 Zenith Semantic Tests\n")
local success = t:report()
os.exit(success and 0 or 1)
