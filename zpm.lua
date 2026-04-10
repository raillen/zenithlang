-- ============================================================================
-- 🔭 ZENITH PACKAGE MANAGER (ZPM) — v0.4.0
-- Estilo Visual: Zenith Aura
-- ============================================================================

local VERSION = "0.4.0"
local CORE_VERSION = "0.2.5"

-- 🎨 AURA ENGINE
local aura = {
    reset   = "\27[0m", bold = "\27[1m", gray = "\27[38;5;244m",
    purple  = "\27[38;5;93m", indigo = "\27[38;5;63m", blue = "\27[38;5;39m",
    cyan    = "\27[38;5;51m", green = "\27[38;5;46m", red = "\27[38;5;196m",
    yellow  = "\27[38;5;226m"
}

-- Detecção básica de suporte a cores
local is_windows = package.config:sub(1,1) == "\\"
if is_windows and not os.getenv("WT_SESSION") then
    for k, v in pairs(aura) do aura[k] = "" end
end

-- 📁 CAMINHOS
local global_dir = is_windows and (os.getenv("APPDATA") .. "\\Zenith\\global") or (os.getenv("HOME") .. "/.zenith/global")

-- 🛠️ UTILITÁRIOS VISUAIS

local function print_header()
    print("\n " .. aura.purple .. "🔭 " .. aura.bold .. "ZENITH MISSION CONTROL" .. aura.reset .. aura.gray .. " — v" .. VERSION .. aura.reset)
    print(aura.indigo .. "====================================" .. aura.reset .. "\n")
end

local function print_info(msg) print(aura.blue .. " ➜ " .. aura.reset .. msg) end
local function print_success(msg) print(aura.green .. " 🚀 " .. msg .. aura.reset) end
local function print_error(msg) print(aura.red .. " ✘ " .. aura.bold .. "ERRO: " .. aura.reset .. msg) end
local function print_warning(msg) print(aura.yellow .. " [!] " .. msg .. aura.reset) end

-- 🌀 ANIMAÇÃO (Spinner)
local function run_with_spinner(msg, cmd)
    local frames = { "⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏" }
    io.write(aura.cyan .. " " .. frames[1] .. " " .. aura.reset .. msg)
    io.flush()
    
    local success = os.execute(cmd .. " > nul 2>&1") == 0
    
    io.write("\r")
    if success then print_success(msg .. aura.gray .. " [OK]" .. aura.reset)
    else print_error(msg .. aura.gray .. " [FALHA]" .. aura.reset) end
    return success
end

local function check_git()
    return os.execute("git --version > nul 2>&1") == 0
end

local function print_git_guide()
    print_warning("Git não detectado neste sistema operacional.")
    print(aura.gray .. "--------------------------------------------------" .. aura.reset)
    print(aura.bold .. " Para acoplar pacotes, você precisa instalar o Git:" .. aura.reset)
    print(" 🌐 Site Oficial: " .. aura.cyan .. "https://git-scm.com/downloads" .. aura.reset)
    print("\n " .. aura.bold .. "Comandos Rápidos:" .. aura.reset)
    print("  " .. aura.purple .. "Windows: " .. aura.reset .. "winget install --id Git.Git -e --source winget")
    print("  " .. aura.purple .. "Linux:   " .. aura.reset .. "sudo apt install git")
    print(aura.gray .. "--------------------------------------------------" .. aura.reset .. "\n")
end

-- 🔒 LOCKFILE (Segurança)
local function update_lockfile()
    -- Carregamento dinâmico da Lib JSON nativa
    local json = require("src/stdlib/json")
    local f_proj = io.open("zenith.ztproj", "r")
    if not f_proj then return end
    
    local deps = {}
    local in_deps = false
    for line in f_proj:lines() do
        if line:match("dependencies") then in_deps = true
        elseif line:match("end") and in_deps then in_deps = false
        elseif in_deps then
            local name, url = line:match("%s*(.-):%s*\"?(.-)\"?$")
            if name and url ~= "official" then deps[name] = url end
        end
    end
    f_proj:close()

    local lock_data = {
        zpm_version = VERSION,
        packages = {}
    }

    print(aura.gray .. " 🔒 Travando órbita (zenith.lock)..." .. aura.reset)
    
    for name, url in pairs(deps) do
        local path = ".zt/vendor/" .. name
        local cmd = "cd " .. path .. " && git rev-parse HEAD"
        if is_windows then cmd = "cd /d " .. path:gsub("/", "\\") .. " && git rev-parse HEAD" end
        
        local handle = io.popen(cmd .. " 2>nul")
        if handle then
            local hash = handle:read("*l")
            handle:close()
            if hash then
                lock_data.packages[name] = { url = url, hash = hash }
            end
        end
    end

    local fw = io.open("zenith.lock", "w")
    if fw then
        fw:write(json.stringify(lock_data, 4))
        fw:close()
        print(aura.gray .. "    Check-in estelar concluído." .. aura.reset)
    end
end

-- 🛰️ COMANDOS

local commands = {}

function commands.version()
    print_info("ZPM Engine: " .. aura.cyan .. VERSION .. aura.reset)
    print_info("Zenith Core: " .. aura.cyan .. CORE_VERSION .. aura.reset)
end

function commands.help()
    print_header()
    print(aura.bold .. " [ Órbita (Gestão) ]" .. aura.reset)
    print("  " .. aura.cyan .. "add <url>" .. aura.reset .. " ........ Acopla uma nova dependência.")
    print("  " .. aura.cyan .. "install" .. aura.reset .. " .......... Sincroniza todos os satélites.")
    print("  " .. aura.cyan .. "remove <nome>" .. aura.reset .. " .... Desorbita um pacote.")
    print("  " .. aura.cyan .. "update" .. aura.reset .. " .......... Busca novas versões nas estrelas.\n")

    print(aura.bold .. " [ Propulsão (Scripts) ]" .. aura.reset)
    print("  " .. aura.cyan .. "run <nome>" .. aura.reset .. " ....... Executa um script do .ztproj.")
    print("  " .. aura.cyan .. "list" .. aura.reset .. " ............. Mapeia todos os pacotes ativos.\n")

    print(aura.bold .. " [ Manutenção (Higiene) ]" .. aura.reset)
    print("  " .. aura.cyan .. "publish" .. aura.reset .. " .......... Publica seu módulo no Git.")
    print("  " .. aura.cyan .. "doctor" .. aura.reset .. " ........... Diagnostica seu ambiente.")
    print("  " .. aura.cyan .. "clean" .. aura.reset .. " ............ Remove destroços temporários.\n")

    print(aura.gray .. " Use 'zpm <comando> --help' para detalhes de um setor." .. aura.reset)
    print("")
end

function commands.init()
    print_header()
    local folder_name = "novo_projeto"
    if io.open("zenith.ztproj", "r") then
        print_error("Já existe um manifesto " .. aura.bold .. "zenith.ztproj" .. aura.reset .. " neste setor.")
        return
    end
    local template = [[
project ]] .. folder_name .. [[

    version: 0.1.0
    author:  "Explorer"
end

dependencies
    std: official
end

scripts
    test: "zt tests/main.zt"
end
]]
    local f = io.open("zenith.ztproj", "w")
    f:write(template)
    f:close()
    print_success("Projeto inicializado com sucesso!")
end

function commands.run()
    local script_name = arg[2]
    if not script_name then print_error("Informe o nome do script.") return end
    print_header()
    local f = io.open("zenith.ztproj", "r")
    if not f then print_error("Manifesto não encontrado.") return end
    local command = nil
    local in_scripts = false
    for line in f:lines() do
        if line:match("scripts") then in_scripts = true
        elseif line:match("end") and in_scripts then in_scripts = false
        elseif in_scripts then
            local name, cmd = line:match("%s*(.-):%s*\"(.*)\"")
            if name == script_name then command = cmd; break end
        end
    end
    f:close()
    if not command then print_error("Script não mapeado."); return end
    if command:sub(1, 3) == "zt " then command = "lua ztc.lua " .. command:sub(4) end
    print(aura.purple .. " ⚡ PROPULSÃO: " .. aura.reset .. aura.bold .. script_name .. aura.reset)
    print(aura.gray .. " $ " .. command .. aura.reset .. "\n")
    os.execute(command)
    print("\n" .. aura.gray .. "------------------------------------" .. aura.reset)
end

function commands.list()
    local mode = arg[2] or "--local"
    print_header()
    local function list_local()
        local f = io.open("zenith.ztproj", "r")
        if not f then return end
        print(aura.bold .. " [ Órbita Local (Projeto) ]" .. aura.reset)
        local in_deps = false
        for line in f:lines() do
            if line:match("dependencies") then in_deps = true
            elseif line:match("end") and in_deps then in_deps = false
            elseif in_deps then
                local name, url = line:match("%s*(.-):%s*\"?(.-)\"?$")
                if name then print("  " .. aura.cyan .. "• " .. aura.reset .. string.format("%-15s %s", name, aura.gray .. url .. aura.reset)) end
            end
        end
        f:close()
        print("")
    end
    local function list_global()
        print(aura.bold .. " [ Órbita Global (Sistema) ]" .. aura.reset)
        local cmd = is_windows and ("dir /b " .. global_dir .. " 2>nul") or ("ls " .. global_dir .. " 2>/dev/null")
        local handle = io.popen(cmd)
        local found = false
        for line in handle:lines() do print("  " .. aura.purple .. "★ " .. aura.reset .. line); found = true end
        handle:close()
        if not found then print(aura.gray .. "  (Nenhum pacote detectado)") end
        print("")
    end
    if mode == "--all" then list_local(); list_global()
    elseif mode == "--global" or mode == "-g" then list_global()
    else list_local() end
end

function commands.add()
    local url = arg[2]
    if not url then print_error("Informe a URL.") return end
    if not check_git() then print_git_guide(); return end
    print_header()
    local repo_name = url:match("([^/]+)$") or "lib"
    local full_url = url:match("^http") and url or ("https://" .. url)
    local dest_path = ".zt/vendor/" .. repo_name
    local mkdir_cmd = is_windows and "if not exist .zt\\vendor mkdir .zt\\vendor" or "mkdir -p .zt/vendor"
    os.execute(mkdir_cmd .. " > nul 2>&1")
    print_info("Acoplando: " .. aura.bold .. repo_name .. aura.reset)
    if run_with_spinner("Baixando do hiperespaço...", "git clone --depth 1 " .. full_url .. " " .. dest_path) then
        local f = io.open("zenith.ztproj", "a")
        f:write("\n    " .. repo_name .. ": \"" .. url .. "\"\n")
        f:close()
        print_success("Pacote integrado!")
        update_lockfile()
    end
end

function commands.install()
    print_header()
    print_info("Sincronizando órbita do projeto...")
    local f = io.open("zenith.ztproj", "r")
    if not f then print_error("Manifesto não encontrado."); return end
    local deps = {}
    local in_deps = false
    for line in f:lines() do
        if line:match("dependencies") then in_deps = true
        elseif line:match("end") and in_deps then in_deps = false
        elseif in_deps then
            local name, url = line:match("%s*(.-):%s*\"?(.-)\"?$")
            if name and url ~= "official" then table.insert(deps, {name=name, url=url}) end
        end
    end
    f:close()
    if #deps == 0 then print_success("Nenhuma dependência externa."); return end
    local mkdir_cmd = is_windows and "if not exist .zt\\vendor mkdir .zt\\vendor" or "mkdir -p .zt/vendor"
    os.execute(mkdir_cmd .. " > nul 2>&1")
    if not check_git() then print_git_guide(); return end
    for _, dep in ipairs(deps) do
        local dest_path = ".zt/vendor/" .. dep.name
        local exists = os.execute((is_windows and ("if exist " .. dest_path:gsub("/", "\\") .. " exit 0") or ("test -d " .. dest_path)) .. " > nul 2>&1") == 0
        if not exists then
            print_info("Sincronizando: " .. aura.bold .. dep.name .. aura.reset)
            run_with_spinner("Baixando...", "git clone --depth 1 " .. (dep.url:match("^http") and dep.url or ("https://" .. dep.url)) .. " " .. dest_path)
        end
    end
    print_success("Sincronia concluída.")
    update_lockfile()
end

function commands.remove()
    local name = arg[2]
    if not name then print_error("Informe o nome."); return end
    print_header()
    local path = ".zt/vendor/" .. name
    local rm_cmd = is_windows and ("rd /s /q " .. path:gsub("/", "\\")) or ("rm -rf " .. path)
    if run_with_spinner("Removendo...", rm_cmd) then
        local f = io.open("zenith.ztproj", "r")
        local lines = {}
        if f then
            for line in f:lines() do if not line:match("^%s*" .. name .. ":") then table.insert(lines, line) end end
            f:close()
            local fw = io.open("zenith.ztproj", "w")
            fw:write(table.concat(lines, "\n"))
            fw:close()
        end
        print_success("Pacote desorbitado.")
        update_lockfile()
    end
end

function commands.publish()
    print_header()
    print_info("Lançamento estelar...")
    local f = io.open("zenith.ztproj", "r")
    if not f then print_error("Manifesto não encontrado."); return end
    local content = f:read("*a"); f:close()
    local p_version = content:match("version:%s*\"?([%d%.%w%-]+)\"?") or "0.1.0"
    if not check_git() then print_git_guide(); return end
    local tag = "v" .. p_version
    os.execute("git tag " .. tag .. " > nul 2>&1")
    if run_with_spinner("Lançando para as estrelas...", "git push origin " .. tag) then
        print_success("Publicado com sucesso!")
    end
end

function commands.clean()
    print_header()
    print_info("Faxina de órbita...")
    local paths = { ".zt/vendor", "dist", "out.lua", "zenith.lock" }
    for _, p in ipairs(paths) do
        local cmd = is_windows and ("rd /s /q " .. p:gsub("/", "\\") .. " 2>nul || del /q " .. p:gsub("/", "\\") .. " 2>nul") or ("rm -rf " .. p)
        os.execute(cmd)
    end
    print_success("Espaço limpo.")
end

function commands.update()
    print_header()
    print_info("Buscando atualizações...")
    commands.install() -- Reutiliza install para garantir presença e atualizar lock
end

function commands.doctor()
    print_header()
    local function check(label, cmd)
        local success = os.execute(cmd .. " > nul 2>&1") == 0
        print(string.format(" %s %-18s .... %s", success and aura.green.."[✔]" or aura.red.."[✘]", label, success and aura.green.."OK" or aura.red.."Falha"))
    end
    check("Compilador ZTC", "lua ztc.lua --version")
    check("Motor Lua", "lua -v")
    check("Conexão Git", "git --version")
    print("\n " .. aura.blue .. "🚀 Órbita estável." .. aura.reset .. "\n")
end

-- 🚀 DISPATCHER
local cmd_name = arg[1]
if not cmd_name or cmd_name == "help" or cmd_name == "--help" then commands.help()
elseif commands[cmd_name] then commands[cmd_name]()
else
    print_error("Comando estelar desconhecido: " .. aura.bold .. tostring(cmd_name) .. aura.reset)
end
