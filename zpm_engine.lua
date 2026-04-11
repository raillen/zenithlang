-- Transpilado por Zenith v0.2.5
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure

local c_reset, c_bold, c_purple, c_blue, c_green, c_red, c_gray, c_cyan, VERSION, CORE_VERSION, print_header, print_info, print_success, print_error, show_help, run_doctor, main

local fs = require("src/stdlib/fs")

local io = require("src/stdlib/io")

local os = require("src/stdlib/os")

local text = require("src/stdlib/text")

local json = require("src/stdlib/json")

function c_reset()
    return "[0m"
end

function c_bold()
    return "[1m"
end

function c_purple()
    return "[38;5;93m"
end

function c_blue()
    return "[38;5;39m"
end

function c_green()
    return "[38;5;46m"
end

function c_red()
    return "[38;5;196m"
end

function c_gray()
    return "[38;5;244m"
end

function c_cyan()
    return "[38;5;51m"
end

function print_header()
    io.write_line(zt.add(zt.add(zt.add(zt.add(zt.add(zt.add(zt.add(zt.add(zt.add("\
 ", c_purple()), "🔭 "), c_bold()), "ZENITH MISSION CONTROL"), c_reset()), c_gray()), " — v"), VERSION), c_reset()))
    io.write_line(zt.add(zt.add(zt.add(c_blue(), "===================================="), c_reset()), "\
"))
end

function print_info(msg)
    io.write_line(zt.add(zt.add(zt.add(c_blue(), " ➜ "), c_reset()), msg))
end

function print_success(msg)
    io.write_line(zt.add(zt.add(zt.add(c_green(), " 🚀 "), msg), c_reset()))
end

function print_error(msg)
    io.write_line(zt.add(zt.add(zt.add(zt.add(zt.add(c_red(), " ✘ "), c_bold()), "ERRO: "), c_reset()), msg))
end

function show_help()
    print_header()
    io.write_line(zt.add(zt.add(c_bold(), " [ Orbita (Gestao) ]"), c_reset()))
    io.write_line(zt.add(zt.add(zt.add(zt.add("  ", c_cyan()), "add <url>"), c_reset()), " ........ Acopla uma nova dependencia."))
    io.write_line(zt.add(zt.add(zt.add(zt.add("  ", c_cyan()), "install"), c_reset()), " .......... Sincroniza todos os satelites."))
    io.write_line(zt.add(zt.add(zt.add(zt.add("  ", c_cyan()), "remove <nome>"), c_reset()), " .... Desorbita um pacote."))
    io.write_line(zt.add(zt.add(zt.add(zt.add("  ", c_cyan()), "update"), c_reset()), " .......... Busca novas versoes nas estrelas.\
"))
    io.write_line(zt.add(zt.add(c_bold(), " [ Propulsao (Scripts) ]"), c_reset()))
    io.write_line(zt.add(zt.add(zt.add(zt.add("  ", c_cyan()), "run <nome>"), c_reset()), " ....... Executa um script do .ztproj."))
    io.write_line(zt.add(zt.add(zt.add(zt.add("  ", c_cyan()), "list"), c_reset()), " ............. Mapeia todos os pacotes ativos.\
"))
    io.write_line(zt.add(zt.add(c_bold(), " [ Manutencao (Higiene) ]"), c_reset()))
    io.write_line(zt.add(zt.add(zt.add(zt.add("  ", c_cyan()), "publish"), c_reset()), " .......... Publica seu modulo no Git."))
    io.write_line(zt.add(zt.add(zt.add(zt.add("  ", c_cyan()), "doctor"), c_reset()), " ........... Diagnostica seu ambiente."))
    io.write_line(zt.add(zt.add(zt.add(zt.add("  ", c_cyan()), "clean"), c_reset()), " ............ Remove destricos temporarios.\
"))
    io.write_line(zt.add(zt.add(c_gray(), " Use 'zpm <comando> --help' para detalhes de um setor."), c_reset()))
    io.write_line("")
end

function run_doctor()
    print_header()
    local c_ztc = os.run_command("lua ztc.lua --version")
    local ztc_ok = ""
    if c_ztc:is_success() then
        ztc_ok = zt.add(c_green(), "[✔] OK")
    else
        ztc_ok = zt.add(c_red(), "[✘] Falha")
    end
    io.write_line(zt.add(zt.add(" ", ztc_ok), " Compilador ZTC"))
    local c_lua = os.run_command("lua -v")
    local lua_ok = ""
    if c_lua:is_success() then
        lua_ok = zt.add(c_green(), "[✔] OK")
    else
        lua_ok = zt.add(c_red(), "[✘] Falha")
    end
    io.write_line(zt.add(zt.add(" ", lua_ok), " Motor Lua"))
    local c_git = os.run_command("git --version")
    local git_ok = ""
    if c_git:is_success() then
        git_ok = zt.add(c_green(), "[✔] OK")
    else
        git_ok = zt.add(c_red(), "[✘] Falha")
    end
    io.write_line(zt.add(zt.add(" ", git_ok), " Conexao Git"))
    io.write_line(zt.add(zt.add(zt.add(zt.add("\
 ", c_blue()), "🚀 Orbita estavel."), c_reset()), "\
"))
end

function main()
    local args = os.get_args()
    if (#(args) == 0) then
        show_help()
        return
    end
    local cmd = args[1]
    local _m = cmd
    if (_m == "help") then
        show_help()
    elseif (_m == "--help") then
        show_help()
    elseif (_m == "version") then
        io.write_line(zt.add("ZPM Engine: ", VERSION))
    elseif (_m == "doctor") then
        run_doctor()
    elseif (_m == "init") then
        if fs.exists("zenith.ztproj") then
            print_error("Ja existe um manifesto zenith.ztproj neste setor.")
        else
            local template = "project novo_projeto\
    version: 0.1.0\
end\
\
dependencies\
    std: official\
end\
"
            fs.write_text_file("zenith.ztproj", template)
            print_success("Projeto inicializado!")
        end
    elseif (_m == "clean") then
        print_header()
        io.write_line("Limpando destrocos...")
        fs.remove_file("out.lua")
        fs.remove_file("zenith.lock")
        print_success("Limpeza concluida.")
    elseif (_m == "run") then
        local script_name = ""
        if (#(args) >= 2) then
            script_name = args[2]
        end
        if (script_name == "") then
            print_error("Informe o nome do script.")
        else
            if not fs.exists("zenith.ztproj") then
                print_error("Manifesto zenith.ztproj nao encontrado.")
            else
                local content_res = fs.read_text_file("zenith.ztproj")
                if content_res:is_failure() then
                    print_error("Nao foi possivel ler o manifesto.")
                else
                    local content = content_res:unwrap_or("")
                    local lines = text.lines(content)
                    local command = ""
                    local in_scripts = false
                    for i, line in zt.iter(lines) do
                        if text.contains(line, "scripts") then
                            in_scripts = true
                        elseif (text.contains(line, "end") and in_scripts) then
                            in_scripts = false
                        elseif in_scripts then
                            if text.contains(line, zt.add(script_name, ":")) then
                                local parts = text.split(line, "\"")
                                if (#(parts) >= 2) then
                                    command = parts[2]
                                end
                            end
                        end
                    end
                    if (command == "") then
                        print_error(zt.add(zt.add("Script '", script_name), "' nao mapeado."))
                    else
                        if text.starts_with(command, "zt ") then
                            command = zt.add("lua ztc.lua ", text.slice(command, 3))
                        end
                        print_header()
                        io.write_line(zt.add(zt.add(zt.add(zt.add(zt.add(c_purple(), " ⚡ PROPULSAO: "), c_reset()), c_bold()), script_name), c_reset()))
                        io.write_line(zt.add(zt.add(zt.add(zt.add(c_gray(), " $ "), command), c_reset()), "\
"))
                        os.run_command(command)
                        io.write_line(zt.add(zt.add(zt.add("\
", c_gray()), "------------------------------------"), c_reset()))
                    end
                end
            end
        end
    elseif true then
        print_error(zt.add("Comando estelar desconhecido: ", cmd))
    end
end

-- Struct Methods
VERSION = "0.5.0"

CORE_VERSION = "0.3.1"


-- Auto-run main if not in a namespace
if not false then
    local status = main()
    if type(status) == 'table' and status.co then status = zt.drive(status) end
    if type(status) == 'number' then os.exit(status) end
end

return {
    main = main,
}