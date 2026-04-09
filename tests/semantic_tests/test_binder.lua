-- ============================================================================
-- Zenith Compiler — Binder Tests
-- ============================================================================

local Parser      = require("src.syntax.parser.parser")
local Binder      = require("src.semantic.binding.binder")
local DiagnosticBag = require("src.diagnostics.diagnostic_bag")
local ZenithType    = require("src.semantic.types.zenith_type")

local test = {
    passed = 0,
    failed = 0,
    total = 0
}

local function assert_no_errors(diagnostics)
    test.total = test.total + 1
    if diagnostics:has_errors() then
        print("FAIL: Erros inesperados encontrados:")
        for _, err in ipairs(diagnostics.diagnostics) do
            print("  - " .. err.message)
        end
        test.failed = test.failed + 1
    else
        test.passed = test.passed + 1
    end
end

local function assert_has_error(diagnostics, code)
    test.total = test.total + 1
    local found = false
    for _, err in ipairs(diagnostics.diagnostics) do
        if err.code == code then found = true break end -- Corrigido: err.code em vez de err.id
    end
    
    if found then
        test.passed = test.passed + 1
    else
        print("FAIL: Esperado erro " .. code .. " mas não foi encontrado.")
        test.failed = test.failed + 1
    end
end

local function run_bind(code)
    local unit, diag_parser = Parser.parse_string(code)
    local binder = Binder.new()
    local _, diag_binder = binder:bind(unit)
    
    -- Merge diagnostics
    diag_parser:merge(diag_binder)
    
    return binder.scope, diag_parser
end

print("--- Rodando Testes Semânticos (Binder) ---")

-- 1. Atribuição Simples e Tipos Primitivos
local _, d1 = run_bind("var x: int = 10")
assert_no_errors(d1)

local _, d2 = run_bind("var x: int = \"texto\"")
assert_has_error(d2, "ZT-S100") -- Incompatível

-- 2. Genéricos: list<T>
local _, d3 = run_bind("var l: list<int> = [1, 2, 3]")
assert_no_errors(d3)

-- 3. Nuláveis: T?
local _, d4 = run_bind("var n: int? = null")
assert_no_errors(d4)

local _, d5 = run_bind("var n: int? = 10")
assert_no_errors(d5)

-- 4. If / Elif / Else
local _, d6 = run_bind([[
if true
    var a: int = 1
elif false
    var a: int = 2
else
    var a: int = 3
end
]])
assert_no_errors(d6) -- Devem ser escopos diferentes, então 'a' redefinido nâo gera erro se em blocos separados

-- 5. Attempt / Rescue
local _, d7 = run_bind([[
attempt
    var x: int = 10
rescue e
    print(e)
end
]])
assert_no_errors(d7)

-- 6. Operador Bang (!)
local _, d8 = run_bind([[
var n: int? = 10
var x: int = n!
]])
assert_no_errors(d8)

-- 7. Redefinição
local _, d9 = run_bind([[
var x: int = 1
var x: int = 2
]])
assert_has_error(d9, "ZT-S001")

print(string.format("\nResultado Binder: %d/%d passaram", test.passed, test.total))
if test.failed > 0 then os.exit(1) end
