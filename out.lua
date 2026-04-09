-- Transpilado por Zenith v0.2.0
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure

local main

local fs = require("src/stdlib/fs")

local path = require("src/stdlib/fs/path")

function main()
    print("--- Testando Módulo std.fs ---")
    local arq = "teste_zenith.txt"
    print("Criando arquivo...")
    fs.write_text_file(arq, "Linha 2\
Linha 3")
    print("Testando Prepend (Linha 1)...")
    fs.prepend_text(arq, "Linha 1\
")
    print("Testando Append (Linha 4)...")
    fs.append_text(arq, "\
Linha 4")
    local res_read = fs.read_text_file(arq)
    local _m = res_read
    if ((_m._tag == "Success") and true) then
        local txt = _m._1
        print(("Conteúdo final:\
" .. txt))
    elseif ((_m._tag == "Failure") and true) then
        local e = _m._1
        print("Erro ao ler")
    end
    print("\
Lendo linha por linha (Stream):")
    local res_open = fs.open_file(arq, fs.FileMode.Read)
    local _m = res_open
    if ((_m._tag == "Success") and true) then
        local h = _m._1
        local handle = h
        local l1_opt = handle.read_line()
        local _m = l1_opt
        if ((_m._tag == "Present") and true) then
            local t = _m._1
            local txt = t
            print(("Linha 1: " .. txt))
        elseif (_m._tag == "Empty") then
            print("Vazio")
        end
        handle.close()
    elseif ((_m._tag == "Failure") and true) then
        local err = _m._1
        print("Falha ao abrir")
    end
    print("\
--- Testando std.fs.path ---")
    local full = path.join({"C:", "Projetos", "Zenith", "main.zt"})
    print(("Path join: " .. full))
    print(("Extensão: " .. path.extension(full)))
    print(("Basename: " .. path.basename(full)))
    print(("Dirname:  " .. path.dirname(full)))
    fs.remove_file(arq)
    print("\
--- Fim dos testes de FS ---")
    return 0
end

-- Struct Methods

-- Auto-run main if not in a namespace
if not false then
    local status = main()
    if type(status) == 'number' then os.exit(status) end
end

return {
    main = main,
}