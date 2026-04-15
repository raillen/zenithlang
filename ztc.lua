-- ============================================================================
-- Zenith Sovereign Compiler (ztc) — Entry Point
-- Fase 1: Pipeline completo com DiagnosticBag e renderer de erros.
-- ============================================================================

package.path = package.path .. ";./src/?.lua;./src/?/init.lua"

local zt = require("src.backend.lua.runtime.zenith_rt")

-- ── Módulos do compilador ────────────────────────────────────────────────────
local parser      = require("src.syntax.parser")
local binder_mod  = require("src.semantic.binding.binder")
local emitter_mod = require("src.backend.lua.lua_codegen")
local source_mod  = require("src.source.source_text")

-- ── Stdlib ────────────────────────────────────────────────────────────────────
local io_lib = require("src/stdlib/io")
local os_lib = require("src/stdlib/os")

-- ── Sistema de Diagnósticos ───────────────────────────────────────────────────
local DiagnosticBag      = require("src.diagnostics.diagnostic_bag")
local DiagnosticRenderer = require("src.diagnostics.diagnostic_renderer")

-- ============================================================================
-- Helpers internos
-- ============================================================================

local function print_usage()
    io.write("Zenith Sovereign Compiler v0.3.5\n")
    io.write("Uso:  ztc <run|build|check> <arquivo.zt> [saida.lua]\n")
    io.write("      ztc <arquivo.zt>                     -- build implícito\n")
    io.write("\nModos:\n")
    io.write("  build  Compila para Lua (padrão: out.lua)\n")
    io.write("  run    Compila e executa imediatamente\n")
    io.write("  check  Análise semântica sem gerar código\n")
end

--- Lê o conteúdo de um arquivo .zt ou aborta com mensagem clara.
local function read_source(path, global_bag)
    local f, err = io.open(path, "r")
    if not f then
        global_bag:report_error("ZT-C001",
            string.format("não foi possível abrir o arquivo '%s': %s", path, err or "erro desconhecido"),
            nil)
        return nil
    end
    local content = f:read("*a")
    f:close()
    return content
end

--- Escreve o arquivo de saída ou aborta com mensagem clara.
local function write_output(path, content, global_bag)
    local f, err = io.open(path, "w")
    if not f then
        global_bag:report_error("ZT-C002",
            string.format("não foi possível escrever em '%s': %s", path, err or "erro desconhecido"),
            nil)
        return false
    end
    f:write(content)
    f:close()
    return true
end

-- ============================================================================
-- Pipeline principal de compilação
-- ============================================================================

--- Executa o pipeline completo: parse → bind → emit.
--- Retorna:
---   lua_code (string|nil), unit (AST|nil), source_text, global_bag
local function run_pipeline(input_path)
    local global_bag = DiagnosticBag.new()

    -- ── 1. Leitura do arquivo ─────────────────────────────────────────────────
    local content = read_source(input_path, global_bag)
    if not content then
        return nil, nil, nil, global_bag
    end

    -- Objeto SourceText — usado para contextualizar diagnósticos
    local source_text = source_mod.new(content, input_path)

    -- ── 2. Parsing ────────────────────────────────────────────────────────────
    local unit, parse_diags
    local ok, parse_err = pcall(function()
        unit, parse_diags = parser.parse_string(content, input_path)
    end)

    if not ok then
        global_bag:report_error("ZT-C010",
            "erro interno no parser: " .. tostring(parse_err), nil)
        return nil, nil, source_text, global_bag
    end

    -- Mesclar diagnósticos do parser
    if parse_diags and parse_diags.diagnostics then
        global_bag:merge(parse_diags)
    end

    -- Abortar cedo se o parser encontrou erros que impedem o binding
    if global_bag:has_errors() then
        return nil, unit, source_text, global_bag
    end

    -- ── 3. Binding (análise semântica + type checking) ─────────────────────────
    local ModuleManager = require("src.semantic.binding.module_manager")
    local manager = ModuleManager.new(".")
    
    local binder = binder_mod.new(global_bag, manager)
    local ok_bind, bind_err = pcall(function()
        binder:bind(unit, "")
    end)

    if not ok_bind then
        global_bag:report_error("ZT-C020",
            "erro interno no binder: " .. tostring(bind_err), nil)
        return nil, unit, source_text, global_bag
    end

    -- REGRA DE OURO: Nunca gerar código se há erros semânticos
    if global_bag:has_errors() then
        return nil, unit, source_text, global_bag
    end

    -- ── 4. Geração de código ──────────────────────────────────────────────────
    local lua_code, source_map
    local ok_emit, emit_err = pcall(function()
        local emitter = emitter_mod.new()
        lua_code, source_map = emitter:generate(unit, source_text)
    end)

    if not ok_emit then
        global_bag:report_error("ZT-C030",
            "erro interno no gerador de código: " .. tostring(emit_err), nil)
        return nil, unit, source_text, global_bag
    end

    return lua_code, unit, source_text, global_bag, source_map
end

-- ============================================================================
-- Ponto de entrada (main)
-- ============================================================================

local function main(args)
    args = args or {}

    -- ── Sem argumentos → ajuda ────────────────────────────────────────────────
    if #args == 0 then
        print_usage()
        return 0
    end

    -- ── Parsing de argumentos ─────────────────────────────────────────────────
    local mode       = "build"
    local input_path = nil
    local out_path   = "out.lua"

    local first = args[1]
    if first == "build" or first == "run" or first == "check" then
        mode = first
        input_path = args[2]
        if args[3] then out_path = args[3] end
    else
        -- Fallback: ztc arquivo.zt [saida.lua]
        input_path = args[1]
        if args[2] then out_path = args[2] end
    end

    if not input_path then
        io.stderr:write("Erro: especifique um arquivo de entrada.\n\n")
        print_usage()
        return 1
    end

    -- ── Modo 'check': apenas análise, sem geração ─────────────────────────────
    if mode == "check" then
        local _, _, source_text, bag = run_pipeline(input_path)
        DiagnosticRenderer.print_all(bag, source_text)
        local had_errors = DiagnosticRenderer.print_summary(bag)
        if had_errors then return 1 end
        io.write(string.format("✅ %s: sem erros\n", input_path))
        return 0
    end

    -- ── Modos 'build' e 'run' ─────────────────────────────────────────────────
    local lua_code, _, source_text, bag, source_map = run_pipeline(input_path)

    -- Sempre exibir diagnósticos (erros E avisos)
    if bag:count() > 0 then
        DiagnosticRenderer.print_all(bag, source_text)
    end

    -- Se houve erros → abortar
    local had_errors = DiagnosticRenderer.print_summary(bag)
    if had_errors then
        return 1
    end

    -- ── Nenhum erro: gravar ou executar ───────────────────────────────────────
    if mode == "run" then
        -- Arquivo temporário
        local tmp = string.format(".ztc-run-%d-%d.lua", os.time(), math.random(1000000))
        local tmp_bag = DiagnosticBag.new()
        if not write_output(tmp, lua_code, tmp_bag) then
            DiagnosticRenderer.print_all(tmp_bag, nil)
            return 1
        end
        os.execute("lua " .. tmp)
        os.remove(tmp)

    else  -- build
        local write_bag = DiagnosticBag.new()
        if not write_output(out_path, lua_code, write_bag) then
            DiagnosticRenderer.print_all(write_bag, nil)
            return 1
        end
        -- Gravar source map se disponível
        if source_map then
            local map_path = out_path .. ".map"
            local lines = {"-- Zenith Source Map", string.format("-- Source: %s", input_path), "return {"}
            lines[#lines+1] = string.format("  source = %q,", input_path)
            lines[#lines+1] = "  mappings = {"
            for lua_line, loc in pairs(source_map) do
                lines[#lines+1] = string.format("    [%d] = {line=%d, col=%d},", lua_line, loc.line, loc.col)
            end
            lines[#lines+1] = "  }"
            lines[#lines+1] = "}"
            local mf = io.open(map_path, "w")
            if mf then mf:write(table.concat(lines, "\n")); mf:close() end
        end
        io.write(string.format("✅ Compilado: %s → %s\n", input_path, out_path))
    end

    return 0
end

-- ============================================================================
-- Auto-execução
-- ============================================================================

local exit_code = main(arg or {})
os.exit(exit_code or 0)