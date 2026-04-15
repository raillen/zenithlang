-- Transpilado por Zenith v0.2.5
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty   = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure
local Outcome  = zt.Outcome
local Optional = zt.Optional

local double_value, get_task_name, main

local core = require("src/stdlib/core")

local io = require("src/stdlib/io")

function double_value(val)
    return (val * 2)
end

function get_task_name(id)
    if (id == 1) then
        return Outcome.Success("Corrigir o Binder")
    elseif (id == 2) then
        return Outcome.Success("Escrever testes UFCS")
    end
    return Outcome.Failure("Tarefa Inexistente")
end

function main(args)
    io.write_line("--- MEGA AUDITORIA: FASES 1-12 ---")
    local num = 10
    local doubled = double_value(num)
    io.write_line(zt.add("UFCS Test (10 * 2): ", doubled:to_text()))
    local l = zt.list({1, 2, 3, 4, 5})
    io.write_line(zt.add("Tamanho da lista (len): ", l:len():to_text()))
    io.write("Elementos da lista: ")
    for item in zt.iter(l) do
        io.write(zt.add(item:to_text(), " "))
    end
    io.write_line("")
    local opt_val = Optional.Present(42)
    if opt_val:is_present() then
        io.write_line(zt.add("Optional Present: ", opt_val:unwrap():to_text()))
    end
    local name = zt.bang(get_task_name(1))
    io.write_line(zt.add("Outcome '!' test: ", name))
    local error_outcome = get_task_name(99)
    if error_outcome.is_failure() then
        io.write_line("Outcome falhou como esperado.")
    end
    local fatia = zt.slice(l, 2, 4)
    io.write("Fatia (1-indexed, elem 2..4): ")
    for f in zt.iter(fatia) do
        io.write(zt.add(f:to_text(), " "))
    end
    io.write_line("")
    io.write_line("Auditoria completa!")
end

-- Struct Methods

-- Auto-run main
if true then
    local status = main(arg)
    if type(status) == 'table' and status.co then status = zt.drive(status) end
    if type(status) == 'number' then os.exit(status) end
end

return {
    double_value = double_value,
    get_task_name = get_task_name,
    main = main,
}