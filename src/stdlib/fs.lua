-- Implementação Nativa da std.fs para Zenith
local zt = require("src.backend.lua.runtime.zenith_rt")

local FS = {}

-- 📂 ENUMS
FS.FileMode = {
    Read = "Read",
    Write = "Write",
    Append = "Append"
}

-- 📂 AUXILIARES
local is_windows = package.config:sub(1,1) == "\\"

-- 🏗️ NÍVEL ATÔMICO

function FS.read_text_file(path)
    local f, err = io.open(path, "r")
    if not f then return zt.Outcome.Failure(err) end
    local content = f:read("*a")
    f:close()
    return zt.Outcome.Success(content)
end

function FS.write_text_file(path, content)
    local f, err = io.open(path, "w")
    if not f then return zt.Outcome.Failure(err) end
    f:write(content)
    f:close()
    return zt.Outcome.Success(nil)
end

function FS.append_text(path, content)
    local f, err = io.open(path, "a")
    if not f then return zt.Outcome.Failure(err) end
    f:write(content)
    f:close()
    return zt.Outcome.Success(nil)
end

function FS.prepend_text(path, content)
    -- Otimização via arquivo temporário
    local tmp_path = path .. ".tmp"
    local f_orig = io.open(path, "r")
    local f_tmp = io.open(tmp_path, "w")
    
    if not f_tmp then return zt.Outcome.Failure("Não foi possível criar arquivo temporário") end
    
    f_tmp:write(content)
    
    if f_orig then
        -- Copia em pedaços (stream) para economizar RAM
        while true do
            local block = f_orig:read(8192)
            if not block then break end
            f_tmp:write(block)
        end
        f_orig:close()
    end
    f_tmp:close()
    
    -- Substitui o original
    os.remove(path)
    os.rename(tmp_path, path)
    return zt.Outcome.Success(nil)
end

function FS.exists(path)
    local f = io.open(path, "r")
    if f then f:close(); return true end
    return false
end

function FS.remove_file(path)
    local ok, err = os.remove(path)
    if ok then return zt.Outcome.Success(nil) else return zt.Outcome.Failure(err) end
end

function FS.remove_folder(path, recursive)
    local cmd = is_windows 
        and (recursive and "rd /s /q " or "rd ")
        or (recursive and "rm -rf " or "rmdir ")
    
    local ok = os.execute(cmd .. "\"" .. path .. "\"")
    if ok == 0 or ok == true then 
        return zt.Outcome.Success(nil) 
    else 
        return zt.Outcome.Failure("Falha ao remover pasta") 
    end
end

-- 🚀 NÍVEL DE FLUXO

function FS.open_file(path, mode_enum)
    -- mode_enum vem como string "Read", "Write", "Append"
    local modes = { Read = "r", Write = "w", Append = "a" }
    local mode = modes[mode_enum] or "r"
    
    local f, err = io.open(path, mode)
    if not f then return zt.Outcome.Failure(err) end
    
    -- Retorna o FileHandle (tabela com métodos)
    local handle = {}
    function handle:read_line()
        local line = f:read("*l")
        if line then return zt.Optional.Present(line) else return zt.Optional.Empty end
    end
    function handle:write(content)
        local ok, err2 = f:write(content)
        if ok then return zt.Outcome.Success(nil) else return zt.Outcome.Failure(err2) end
    end
    function handle:close()
        f:close()
    end
    
    return zt.Outcome.Success(handle)
end

return FS
