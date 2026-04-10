-- Transpilado por Zenith v0.2.5
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure

local main

local text = require("src/stdlib/text")

function main()
    print("--- Testando Módulo std.text ---")
    local frase = "  Zenith é Sensacional! 🍎  "
    print((tostring((tostring("Original: [") .. tostring(frase))) .. tostring("]")))
    print((tostring((tostring("Trim:     [") .. tostring(text.trim(frase)))) .. tostring("]")))
    local limpa = text.trim(frase)
    print((tostring("Lower:    ") .. tostring(text.to_lower(limpa))))
    print((tostring("Reverse:  ") .. tostring(text.reverse(limpa))))
    print("\
--- Testando UTF-8 e Emojis ---")
    local br = "Ação e Reação"
    print((tostring("Texto: ") .. tostring(br)))
    print((tostring("Caracteres (real): ") .. tostring(text.count_chars(br))))
    print((tostring("Sem acentos: ") .. tostring(text.clean_accents(br))))
    local emo = "Olá 🌍!"
    print((tostring("\
Texto: ") .. tostring(emo)))
    print((tostring("Tem emoji? ") .. tostring(text.is_emoji(emo))))
    print((tostring("Invertido: ") .. tostring(text.reverse(emo))))
    print("\
--- Testando Cases ---")
    local nome = "usuario_logado_hoje"
    print((tostring("Snake: ") .. tostring(nome)))
    print((tostring("Camel: ") .. tostring(text.to_camel_case(nome))))
    local titulo = "Guia de Programação Zenith v0.3"
    print((tostring("Slugify: ") .. tostring(text.slugify(titulo))))
    print("\
--- Testando Segurança e UI ---")
    local card = "1234567890123456"
    print((tostring("Mascara: ") .. tostring(text.mask(card, 4, 4))))
    local longo = "Este é um texto muito longo que deve ser cortado."
    print((tostring("Truncate: ") .. tostring(text.truncate(longo, 20))))
    print("\
--- Fim dos testes de TEXT ---")
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