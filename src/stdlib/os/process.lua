-- Implementação Nativa da std.os.process para Zenith
local zt = require("src.backend.lua.runtime.zenith_rt")

local PROCESS = {}

-- 📂 AUXILIARES
local is_windows = package.config:sub(1,1) == "\\"
local temp_dir = os.getenv("TEMP") or os.getenv("TMP") or "."

-- 🏗️ LÓGICA DE SPAWN

function PROCESS.spawn(cmd, args, capture, env_map)
    local arg_str = table.concat(args, " ")
    
    -- Gerar nomes únicos baseados no tempo e aleatoriedade
    local id = string.format("%d_%d", os.time(), math.random(1000, 9999))
    local exit_file = temp_dir .. "\\zt_exit_" .. id .. ".tmp"
    local output_file = temp_dir .. "\\zt_out_" .. id .. ".tmp"
    
    -- Limpa arquivos antigos se existirem
    os.remove(exit_file)
    os.remove(output_file)
    
    -- Preparar variáveis de ambiente
    local env_prefix = ""
    for k, v in pairs(env_map or {}) do
        if is_windows then
            env_prefix = env_prefix .. "set " .. k .. "=" .. v .. " && "
        else
            env_prefix = env_prefix .. k .. "=" .. v .. " "
        end
    end

    local shell_cmd
    if is_windows then
        -- No Windows, usamos cmd /c para capturar o erro e redirecionar
        local capture_redir = capture and (" > \"" .. output_file .. "\" 2>&1") or ""
        shell_cmd = string.format("start /b cmd /c \"(%s %s %s) & echo %%errorlevel%% > \"%s\"\"", 
            env_prefix, cmd, arg_str .. capture_redir, exit_file)
    else
        local capture_redir = capture and (" > " .. output_file .. " 2>&1") or ""
        shell_cmd = string.format("(%s %s %s; echo $? > %s) &", 
            env_prefix, cmd, arg_str .. capture_redir, exit_file)
    end

    local success = os.execute(shell_cmd)
    if not success then 
        return zt.Outcome.Failure("Falha ao iniciar processo no shell.") 
    end

    -- Criar o objeto Process
    local p = {
        pid = tonumber(id), -- Usamos nosso ID interno como PID simbólico
        executable = cmd,
        _exit_file = exit_file,
        _output_file = output_file,
        _captured = capture,
        _status = "Running"
    }

    -- 🛠️ MÉTODOS DA INSTÂNCIA

    function p:wait()
        return zt.async_run(function()
            local code = nil
            while true do
                local f = io.open(self._exit_file, "r")
                if f then
                    local content = f:read("*a")
                    f:close()
                    if content and content:match("%d+") then
                        code = tonumber(content:match("%d+"))
                        break
                    end
                end
                coroutine.yield() -- Espera o próximo ciclo do scheduler
            end
            self._status = "Finished"
            return code or 0
        end)
    end

    function p:kill()
        -- No Windows, não temos o PID real fácil, mas podemos tentar via taskkill se soubermos o nome
        -- Por enquanto, apenas removemos os arquivos de controle
        os.remove(self._exit_file)
        self._status = "Killed"
        return true
    end

    function p:read_output()
        if not self._captured then return zt.Optional.Empty end
        local f = io.open(self._output_file, "r")
        if not f then return zt.Optional.Empty end
        local content = f:read("*a")
        f:close()
        return zt.Optional.Present(content)
    end

    return zt.Outcome.Success(p)
end

function PROCESS.current_id()
    -- Em Lua puro 5.1 não há forma trivial de pegar o PID real
    return 0
end

return PROCESS
