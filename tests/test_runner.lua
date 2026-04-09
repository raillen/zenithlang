-- ============================================================================
-- Zenith Test Runner
-- Framework de testes mínimo para o compilador Zenith.
-- Suporta group/test/assert — consistente com a filosofia da linguagem.
-- ============================================================================

local TestRunner = {}
TestRunner.__index = TestRunner

--- Cria uma nova instância do test runner.
--- @return table
function TestRunner.new()
    local self = setmetatable({}, TestRunner)
    self.groups = {}
    self.current_group = nil
    self.total_passed = 0
    self.total_failed = 0
    self.total_errors = 0
    self.failures = {}
    return self
end

--- Inicia um grupo de testes.
--- @param name string Nome do grupo
--- @param fn function Função contendo os testes
function TestRunner:group(name, fn)
    self.current_group = {
        name = name,
        tests = {},
        passed = 0,
        failed = 0,
    }
    table.insert(self.groups, self.current_group)

    local ok, err = pcall(fn)
    if not ok then
        self.total_errors = self.total_errors + 1
        io.write(string.format("  💥 Erro no grupo '%s': %s\n", name, tostring(err)))
    end

    self.current_group = nil
end

--- Define um teste individual.
--- @param name string Nome do teste
--- @param fn function Função do teste
function TestRunner:test(name, fn)
    local group = self.current_group
    if not group then
        error("test() deve ser chamado dentro de group()")
    end

    local ok, err = pcall(fn)
    if ok then
        group.passed = group.passed + 1
        self.total_passed = self.total_passed + 1
        table.insert(group.tests, { name = name, passed = true })
    else
        group.failed = group.failed + 1
        self.total_failed = self.total_failed + 1
        table.insert(group.tests, { name = name, passed = false, error = tostring(err) })
        table.insert(self.failures, {
            group = group.name,
            test = name,
            error = tostring(err),
        })
    end
end

--- Imprime os resultados dos testes.
function TestRunner:report()
    io.write("\n")

    for _, group in ipairs(self.groups) do
        io.write(string.format("  📦 %s\n", group.name))
        for _, t in ipairs(group.tests) do
            if t.passed then
                io.write(string.format("    ✅ %s\n", t.name))
            else
                io.write(string.format("    ❌ %s\n", t.name))
                io.write(string.format("       %s\n", t.error))
            end
        end
        io.write("\n")
    end

    local total = self.total_passed + self.total_failed
    io.write(string.format("  Resultado: %d/%d passaram", self.total_passed, total))

    if self.total_failed > 0 then
        io.write(string.format(" (%d falhou)", self.total_failed))
    end
    if self.total_errors > 0 then
        io.write(string.format(" (%d erro de grupo)", self.total_errors))
    end
    io.write("\n\n")

    return self.total_failed == 0 and self.total_errors == 0
end

-- ============================================================================
-- Funções de asserção
-- ============================================================================

local assert_lib = {}

--- Asserção de igualdade.
--- @param actual any Valor obtido
--- @param expected any Valor esperado
--- @param msg string|nil Mensagem opcional
function assert_lib.equal(actual, expected, msg)
    if actual ~= expected then
        local message = msg or ""
        error(string.format(
            "esperado: %s, recebido: %s %s",
            tostring(expected),
            tostring(actual),
            message
        ), 2)
    end
end

--- Asserção de desigualdade.
--- @param actual any
--- @param unexpected any
--- @param msg string|nil
function assert_lib.not_equal(actual, unexpected, msg)
    if actual == unexpected then
        local message = msg or ""
        error(string.format(
            "não esperado: %s, mas recebido igual %s",
            tostring(unexpected),
            message
        ), 2)
    end
end

--- Asserção de verdade.
--- @param value any
--- @param msg string|nil
function assert_lib.is_true(value, msg)
    if not value then
        error(msg or "esperado verdadeiro, recebido falso", 2)
    end
end

--- Asserção de falsidade.
--- @param value any
--- @param msg string|nil
function assert_lib.is_false(value, msg)
    if value then
        error(msg or "esperado falso, recebido verdadeiro", 2)
    end
end

--- Asserção de nulidade.
--- @param value any
--- @param msg string|nil
function assert_lib.is_nil(value, msg)
    if value ~= nil then
        error(msg or string.format("esperado nil, recebido: %s", tostring(value)), 2)
    end
end

--- Asserção de não-nulidade.
--- @param value any
--- @param msg string|nil
function assert_lib.is_not_nil(value, msg)
    if value == nil then
        error(msg or "esperado não-nil, recebido nil", 2)
    end
end

--- Asserção de que uma função lança erro.
--- @param fn function
--- @param msg string|nil
function assert_lib.throws(fn, msg)
    local ok, _ = pcall(fn)
    if ok then
        error(msg or "esperado que lançasse erro, mas não lançou", 2)
    end
end

--- Asserção de tipo.
--- @param value any
--- @param expected_type string
--- @param msg string|nil
function assert_lib.is_type(value, expected_type, msg)
    local actual_type = type(value)
    if actual_type ~= expected_type then
        error(msg or string.format(
            "esperado tipo '%s', recebido tipo '%s'",
            expected_type,
            actual_type
        ), 2)
    end
end

--- Asserção de tamanho de tabela/string.
--- @param value table|string
--- @param expected_length number
--- @param msg string|nil
function assert_lib.length(value, expected_length, msg)
    local len
    if type(value) == "string" then
        len = #value
    elseif type(value) == "table" then
        len = #value
    else
        error("length() espera string ou table", 2)
    end
    if len ~= expected_length then
        error(msg or string.format(
            "esperado tamanho %d, recebido %d",
            expected_length,
            len
        ), 2)
    end
end

return {
    TestRunner = TestRunner,
    assert = assert_lib,
}
