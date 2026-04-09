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
        -- Deve conter erro ZT-S201
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

end)

-- ============================================================================
-- Executar
-- ============================================================================

io.write("\n🧪 Zenith Semantic Tests\n")
local success = t:report()
os.exit(success and 0 or 1)
