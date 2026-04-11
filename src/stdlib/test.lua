local zt = require("src.backend.lua.runtime.zenith_rt")

local Test = {
    _groups = {},
    _current_group = nil,
    _stats = { passed = 0, failed = 0, total = 0 }
}

function Test.describe(name, body)
    local group = {
        name = name,
        tests = {},
        before_each = nil,
        after_each = nil
    }
    table.insert(Test._groups, group)
    
    local prev_group = Test._current_group
    Test._current_group = group
    
    local ok, err = pcall(body)
    if not ok then
        error("Erro ao definir grupo '" .. name .. "': " .. tostring(err))
    end
    
    Test._current_group = prev_group
end

function Test.test(name, body)
    if not Test._current_group then
        error("Função test() deve ser chamada dentro de describe()")
    end
    
    table.insert(Test._current_group.tests, {
        name = name,
        body = body
    })
end

function Test.before_each(body)
    if not Test._current_group then error("before_each() deve estar dentro de describe()") end
    Test._current_group.before_each = body
end

function Test.after_each(body)
    if not Test._current_group then error("after_each() deve estar dentro de describe()") end
    Test._current_group.after_each = body
end

-- Assertions
function Test.assert_eq(actual, expected)
    if actual ~= expected then
        local esc = string.char(27)
        local msg = string.format("\n      %sEsperado: %s[%s]%s\n      %sRecebido: %s[%s]%s", 
            esc .. "[32m", esc .. "[1m", tostring(expected), esc .. "[0m" .. esc .. "[32m",
            esc .. "[31m", esc .. "[1m", tostring(actual), esc .. "[0m")
        error("Assertion Failed:" .. msg, 0)
    end
end

function Test.assert_true(cond)
    if not cond then
        local esc = string.char(27)
        error("Assertion Failed: esperado " .. esc .. "[1mtrue" .. esc .. "[0m, recebido " .. esc .. "[1mfalse" .. esc .. "[0m", 0)
    end
end

function Test.assert_false(cond)
    if cond then
        local esc = string.char(27)
        error("Assertion Failed: esperado " .. esc .. "[1mfalse" .. esc .. "[0m, recebido " .. esc .. "[1mtrue" .. esc .. "[0m", 0)
    end
end

function Test.assert_snapshot(name, actual)
    local json = require("src/stdlib/json")
    local fs = require("src/stdlib/fs")
    local snapshot_path = "tests/snapshots/" .. name .. ".snap"
    
    local actual_str = type(actual) == "table" and json.stringify(actual, 4) or tostring(actual)
    
    local existing = fs.read_text_file(snapshot_path)
    if existing._tag == "Failure" then
        -- Primeiro run: cria o snapshot
        os.execute("mkdir tests\\snapshots 2>nul")
        fs.write_text_file(snapshot_path, actual_str)
        return
    end
    
    local expected = existing.value
    if actual_str ~= expected then
        local esc = string.char(27)
        local msg = string.format("\n      %sSnapshot [%s] divergiu!%s\n      Use 'zt test --update-snapshots' se a mudança for intencional.", 
            esc .. "[31m", name, esc .. "[0m")
        error("Assertion Failed: " .. msg, 0)
    end
end

function Test.assert_outcome_success(res)
    if res._tag ~= "Success" then
        local esc = string.char(27)
        error("Assertion Failed: esperado " .. esc .. "[32mSuccess" .. esc .. "[0m, recebido " .. esc .. "[31m" .. tostring(res._tag) .. esc .. "[0m (Erro: " .. tostring(res.error or "n/a") .. ")", 0)
    end
end

function Test.assert_outcome_failure(res)
    if res._tag ~= "Failure" then
        local esc = string.char(27)
        error("Assertion Failed: esperado " .. esc .. "[31mFailure" .. esc .. "[0m, recebido " .. esc .. "[32m" .. tostring(res._tag) .. esc .. "[0m", 0)
    end
end

-- Runner Engine (Called by ztest orchestrator)
function Test.run_all()
    local esc = string.char(27)
    local aura = {
        reset   = esc .. "[0m", bold = esc .. "[1m", green = esc .. "[32m",
        red     = esc .. "[31m", cyan  = esc .. "[36m", gray  = esc .. "[90m",
        yellow  = esc .. "[33m"
    }

    local results = {
        suites = {},
        stats = { passed = 0, failed = 0, total = 0 }
    }

    for _, group in ipairs(Test._groups) do
        local group_res = { name = group.name, tests = {} }
        
        for _, t in ipairs(group.tests) do
            results.stats.total = results.stats.total + 1
            local test_entry = { name = t.name, status = "passed" }
            
            if group.before_each then pcall(group.before_each) end
            
            local ok, err = pcall(t.body)
            
            if group.after_each then pcall(group.after_each) end
            
            if ok then
                results.stats.passed = results.stats.passed + 1
            else
                results.stats.failed = results.stats.failed + 1
                test_entry.status = "failed"
                test_entry.error = tostring(err)
            end
            table.insert(group_res.tests, test_entry)
        end
        table.insert(results.suites, group_res)
    end

    -- Exportação JSON se solicitado via flag de ambiente interna
    if os.getenv("ZTEST_EXPORT_JSON") then
        local json = require("src/stdlib/json")
        local fs = require("src/stdlib/fs")
        fs.write_text_file("test_report_tmp.json", json.stringify(results, 4))
    end

    -- Saída visual (se não estiver em modo silencioso)
    if not os.getenv("ZTEST_SILENT") then
        print("\n" .. aura.bold .. aura.cyan .. "[TEST] ZTEST UNIT ENGINE" .. aura.reset)
        for _, g in ipairs(results.suites) do
            print("\n " .. aura.bold .. "GROUP: " .. g.name .. aura.reset)
            for _, t in ipairs(g.tests) do
                if t.status == "passed" then
                    print(aura.green .. "   PASS: " .. aura.reset .. t.name)
                else
                    print(aura.red .. "   FAIL: " .. aura.reset .. t.name)
                    print(aura.red .. "     └─ " .. t.error .. aura.reset)
                end
            end
        end
    end
    
    os.exit(results.stats.failed == 0 and 0 or 1)
end

return Test
