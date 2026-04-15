-- Transpilado por Zenith v0.2.5
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure

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
    local tasks = {}
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
        if zt.is_present(input_opt) then
            opt = zt.unwrap(input_opt)
        end
        if zt.is_empty(input_opt) then
            running = false
        end
        if not running then
            break
        end
        if (opt == "1") then
            io.write("Titulo: ")
            local title_opt = io.read_line()
            if zt.is_present(title_opt) then
                local title = zt.unwrap(title_opt)
                local new_task = Task.new({["id"] = next_id, ["title"] = title, ["done"] = false})
                zt.push(tasks, new_task)
                next_id = zt.add(next_id, 1)
                io.write_line("✅ Tarefa adicionada!")
            end
        elseif (opt == "2") then
            io.write_line("--- Suas Tarefas ---")
            if zt.is_empty(tasks) then
                io.write_line("📝 Nenhuma tarefa na lista.")
            else
                for t in zt.iter(tasks) do
                    local status = "[ ]"
                    if t.done then
                        status = "[X]"
                    end
                    io.write_line(zt.add(zt.add(zt.add(zt.add(zt.to_text(t.id), ". "), status), " "), t.title))
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

-- Auto-run main if not in a namespace
if not false then
    local status = main()
    if type(status) == 'table' and status.co then status = zt.drive(status) end
    if type(status) == 'number' then os.exit(status) end
end

return {
    main = main,
    main = main,
}