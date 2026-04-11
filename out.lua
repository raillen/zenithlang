-- Transpilado por Zenith v0.2.5
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure

local main

local os = require("src/stdlib/os")

function main()
    print("--- Testando Módulo std.os ---")
    print(zt.add("Versão do Zenith: ", os.version))
    print(zt.add("Plataforma: ", os.platform))
    print(zt.add("Arquitetura: ", os.arch))
    print("--- Testando Variáveis de Ambiente ---")
    local user = zt.unwrap_or(os.get_env_variable("USERNAME"), "desconhecido")
    print(zt.add("Usuário atual: ", user))
    local envs = os.get_all_env_variables()
    print(zt.add("Total de variáveis de ambiente: ", #(envs)))
    print("--- Testando Informações de Hardware ---")
    local hw = os.get_hardware_info()
    print(zt.add("Processador: ", hw.cpu))
    print(zt.add(zt.add("RAM: ", hw.ram_gb), " GB"))
    print("--- Testando Caminhos ---")
    print(zt.add("Trabalhando em: ", os.get_working_dir()))
    print(zt.add("Área de Trabalho: ", os.get_special_path(os.SpecialPath.Desktop)))
    print("--- Testando Comandos ---")
    local code = os.run_command("echo Hello from Zenith Command!")
    print(zt.add("Código de saída do echo: ", code))
    print("--- Testando Privilégios ---")
    if os.is_admin() then
        print("Rodando como Administrador")
    else
        print("Rodando como Usuário Normal")
    end
    print("--- Fim dos testes de OS ---")
    return 0
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