-- ============================================================================
-- Zenith Tool — Check Docs Sync
-- Verifica se as palavras-chave e tipos da linguagem estão documentados.
-- ============================================================================

-- Mocking do ambiente para carregar módulos do projeto
package.path = package.path .. ";./?.lua;./src/?.lua"

local KeywordTable = require("src.syntax.tokens.keyword_table")
local colors = {
    reset = "\27[0m",
    red = "\27[31m",
    green = "\27[32m",
    yellow = "\27[33m",
    blue = "\27[34m",
    magenta = "\27[35m",
    cyan = "\27[36m"
}

local function read_file(path)
    local f = io.open(path, "r")
    if not f then return nil end
    local content = f:read("*all")
    f:close()
    return content
end

local function check_sync()
    print(colors.cyan .. "🔍 Iniciando auditoria de sincronização Zenith..." .. colors.reset)
    
    local docs_path = "docs/handbook/syntax.md"
    local docs_content = read_file(docs_path)
    
    if not docs_content then
        print(colors.red .. "❌ Erro: Não foi possível ler " .. docs_path .. colors.reset)
        os.exit(1)
    end

    local missing = {}
    local found_count = 0
    local total_count = 0

    -- 1. Verificar Keywords do Lexer
    print("\n" .. colors.blue .. "--- Verificando Palavras-chave ---" .. colors.reset)
    for kw, _ in pairs(KeywordTable.keywords) do
        total_count = total_count + 1
        -- Procura a keyword cercada por backticks ou em blocos de código
        -- Usamos uma regex simples: `kw` ou keyword no texto
        if docs_content:find("`" .. kw .. "`") or docs_content:find(" " .. kw .. " ") then
            found_count = found_count + 1
        else
            table.insert(missing, { term = kw, type = "Keyword" })
            print(colors.yellow .. "⚠️  Ausente: " .. colors.reset .. kw)
        end
    end

    -- 2. Verificar Tipos Primitivos (Hardcoded para Zenith)
    local primitives = { "int", "float", "text", "bool", "list", "map", "grid", "void", "any" }
    print("\n" .. colors.blue .. "--- Verificando Tipos Primitivos ---" .. colors.reset)
    for _, t in ipairs(primitives) do
        total_count = total_count + 1
        if docs_content:find("`" .. t .. "`") then
            found_count = found_count + 1
        else
            table.insert(missing, { term = t, type = "Tipo" })
            print(colors.yellow .. "⚠️  Ausente: " .. colors.reset .. t)
        end
    end

    -- Relatório Final
    print("\n" .. string.rep("=", 40))
    if #missing == 0 then
        print(colors.green .. "✨ SUCESSO: Tudo parece estar em sincronia!" .. colors.reset)
        print(string.format("📊 %d/%d termos documentados.", found_count, total_count))
    else
        print(colors.magenta .. "📝 Resumo de Pendências:" .. colors.reset)
        print(string.format("❌ %d termos não encontrados na documentação.", #missing))
        print(string.format("✅ %d termos documentados.", found_count))
        print("\n" .. colors.cyan .. "DICA: Use a Skill 'zenith-sincronizacao-docs' para resolver." .. colors.reset)
    end
    print(string.rep("=", 40))

    if #missing > 0 then
        os.exit(1)
    end
end

check_sync()
