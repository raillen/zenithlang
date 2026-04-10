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
    print((tostring("Versão do Zenith: ") .. tostring(os.version)))
    print((tostring("Plataforma: ") .. tostring(os.platform)))
    print((tostring("Arquitetura: ") .. tostring(os.arch)))
    print("--- Testando Variáveis de Ambiente ---")
    local user = zt.unwrap_or(os.get_env_variable("USERNAME"), "desconhecido")
    print((tostring("Usuário atual: ") .. tostring(user)))
    local envs = os.get_all_env_variables()
    print((tostring("Total de variáveis de ambiente: ") .. tostring(#(envs))))
    print("--- Testando Informações de Hardware ---")
    local hw = os.get_hardware_info()
    print((tostring("Processador: ") .. tostring(hw.cpu)))
    print((tostring((tostring("RAM: ") .. tostring(hw.ram_gb))) .. tostring(" GB")))
    print("--- Testando Caminhos ---")
    print((tostring("Trabalhando em: ") .. tostring(os.get_working_dir())))
    print((tostring("Área de Trabalho: ") .. tostring(os.get_special_path(os.SpecialPath.Desktop))))
    print("--- Testando Comandos ---")
    local code = os.run_command("echo Hello from Zenith Command!")
    print((tostring("Código de saída do echo: ") .. tostring(code)))
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
    if type(status) == 'number' then os.exit(status) end
end

return {
    main = main,
}