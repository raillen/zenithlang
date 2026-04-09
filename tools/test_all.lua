-- ============================================================================
-- Zenith Tool — Test All
-- Executa todos os arquivos de teste (*_test.lua ou test_*.lua) em /tests.
-- ============================================================================

local colors = {
    reset   = "\27[0m",
    red     = "\27[31m",
    green   = "\27[32m",
    yellow  = "\27[33m",
    blue    = "\27[34m",
    magenta = "\27[35m",
    cyan    = "\27[36m"
}

local function run_test_file(path)
    print(colors.blue .. "🚀 Rodando: " .. path .. colors.reset)
    -- Executa o arquivo via comando lua
    local handle = io.popen("lua " .. path)
    local result = handle:read("*a")
    local success = handle:close()
    
    print(result)
    return success
end

local function main()
    print(colors.cyan .. "🧪 Iniciando suite de testes completa Zenith..." .. colors.reset)
    
    -- No Windows, usamos 'dir /s /b' para listar arquivos recursivamente
    local cmd = "dir tests\\*test*.lua /s /b"
    local handle = io.popen(cmd)
    local output = handle:read("*a")
    handle:close()

    local files = {}
    for file in output:gmatch("[^\r\n]+") do
        -- Filtrar para garantir que pegamos apenas arquivos .lua e ignoramos o test_runner
        if file:match("%.lua$") and not file:match("test_runner%.lua") then
            table.insert(files, file)
        end
    end

    if #files == 0 then
        print(colors.red .. "❌ Nenhum arquivo de teste encontrado em /tests" .. colors.reset)
        os.exit(1)
    end

    print(string.format(colors.yellow .. "Found %d test files." .. colors.reset, #files))

    local all_success = true
    for _, file in ipairs(files) do
        if not run_test_file(file) then
            all_success = false
        end
    end

    print("\n" .. string.rep("=", 40))
    if all_success then
        print(colors.green .. "✨ SUCESSO: Todos os testes passaram!" .. colors.reset)
        os.exit(0)
    else
        print(colors.red .. "💥 FALHA: Alguns testes falharam." .. colors.reset)
        os.exit(1)
    end
end

main()
