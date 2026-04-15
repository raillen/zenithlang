-- Transpilado por Zenith v0.2.5
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty   = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure
local Outcome  = zt.Outcome
local Optional = zt.Optional

local Task, main

local core = require("src/stdlib/core")

local io = require("src/stdlib/io")

local Task = {}
Task.__index = Task
Task._metadata = {
    name = "Task",
    fields = {
    },
    methods = {
    }
}

function Task.new(fields)
    local self = setmetatable({}, Task)
    self.id = fields.id or nil
    self.title = fields.title or nil
    self.done = fields.done or nil
    return self
end

function main(args)
    io.write_line("=================================")
    io.write_line("🌟 Zenith Task Manager 0.3.5 🌟")
    io.write_line("=================================")
    local tasks = zt.list({})
    local running = true
    local next_id = 1
    while running do
        io.write_line("\
--- Menu ---")
        io.write_line("1. Adicionar Tarefa")
        io.write_line("2. Listar Tarefas")
        io.write_line("3. Sair")
        io.write("-> ")
        local input_opt = io.read_line()
        local opt = "0"
        if input_opt:is_present() then
            opt = input_opt:unwrap()
        end
        if input_opt:is_empty() then
            running = false
        end
        if not running then
            break
        end
        if (opt == "1") then
            io.write("Titulo: ")
            local title_opt = io.read_line()
            if title_opt:is_present() then
                local title = title_opt:unwrap()
                local new_task = Task.new({["id"] = next_id, ["title"] = title, ["done"] = false})
                tasks:push(new_task)
                next_id = zt.add(next_id, 1)
                io.write_line("✅ Tarefa adicionada!")
            end
        elseif (opt == "2") then
            io.write_line("--- Suas Tarefas ---")
            if tasks:is_empty() then
                io.write_line("📝 Nenhuma tarefa na lista.")
            else
                for t in zt.iter(tasks) do
                    local status = "[ ]"
                    if t.done then
                        status = "[X]"
                    end
                    io.write_line(zt.add(zt.add(zt.add(zt.add(t.id:to_text(), ". "), status), " "), t.title))
                end
            end
        elseif (opt == "3") then
            io.write_line("Tchau!")
            running = false
        else
            io.write_line("❌ Opcao invalida.")
        end
    end
end

-- Struct Methods

-- Auto-run main
if true then
    local status = main(arg)
    if type(status) == 'table' and status.co then status = zt.drive(status) end
    if type(status) == 'number' then os.exit(status) end
end

return {
    main = main,
}