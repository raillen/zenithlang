-- ============================================================================
-- Zenith Front Door (ztc)
-- Self-hosted by default, legacy Lua pipeline as explicit fallback.
-- ============================================================================

local function frontdoor_dir()
    local raw = (arg and arg[0]) or "ztc.lua"
    local normalized = tostring(raw):gsub("\\", "/")
    return normalized:match("^(.*)/[^/]+$") or "."
end

local REPO_ROOT = frontdoor_dir()

local function repo_path(relative_path)
    if REPO_ROOT == "." then
        return relative_path
    end
    return REPO_ROOT .. "/" .. relative_path
end

package.path = package.path
    .. ";" .. repo_path("?.lua")
    .. ";" .. repo_path("?/init.lua")
    .. ";" .. repo_path("src/?.lua")
    .. ";" .. repo_path("src/?/init.lua")

local parser = require("src.syntax.parser")
local binder_mod = require("src.semantic.binding.binder")
local emitter_mod = require("src.backend.lua.lua_codegen")
local source_mod = require("src.source.source_text")
local DiagnosticBag = require("src.diagnostics.diagnostic_bag")
local DiagnosticRenderer = require("src.diagnostics.diagnostic_renderer")

local VERSION = "0.3.6"
local DEFAULT_OUTPUT = "out.lua"
local DEFAULT_TEMP_DIR = ".ztc-tmp"

math.randomseed(os.time())

local function is_windows()
    return package.config:sub(1, 1) == "\\"
end

local function detect_host_os()
    return is_windows() and "windows" or "linux"
end

local function legacy_frontdoor_enabled()
    return rawget(_G, "ZTC_ALLOW_LEGACY_FRONTDOOR") == true or os.getenv("ZTC_ALLOW_LEGACY_FRONTDOOR") == "1"
end

local function quote_arg(value)
    local s = tostring(value or "")
    s = s:gsub('"', '\"')
    return '"' .. s .. '"'
end

local function path_join(base, name)
    if is_windows() then
        return tostring(base or "") .. "\\" .. tostring(name or "")
    end
    return tostring(base or "") .. "/" .. tostring(name or "")
end

local function ensure_dir(path)
    if is_windows() then
        os.execute("if not exist " .. quote_arg(path) .. " mkdir " .. quote_arg(path))
    else
        os.execute("mkdir -p " .. quote_arg(path))
    end
end

local function file_exists(path)
    local handle = io.open(path, "r")
    if not handle then
        return false
    end
    handle:close()
    return true
end

local function read_text(path)
    local handle, err = io.open(path, "r")
    if not handle then
        return nil, err or "erro desconhecido"
    end
    local content = handle:read("*a")
    handle:close()
    return content
end

local function write_text(path, content)
    local handle, err = io.open(path, "w")
    if not handle then
        return false, err or "erro desconhecido"
    end
    handle:write(content)
    handle:close()
    return true
end

local function print_usage()
    io.write("Zenith Compiler v" .. VERSION .. "\n")
    io.write("Uso:  ztc [--selfhost|--strict-selfhost] <run|build|check> <arquivo.zt> [saida.lua]\n")
    io.write("      ztc [--selfhost|--strict-selfhost] <arquivo.zt>\n")
    io.write("      ztc [--selfhost|--strict-selfhost] <zpm|zman|ztest> <subcomando>\n")
    io.write("\n")
    io.write("Flags de engine:\n")
    io.write("  --selfhost         Usa o compilador self-hosted (padrao)\n")
    io.write("  --strict-selfhost  Alias explicito do caminho oficial estrito\n")
    io.write("\n")
    io.write("Recuperacao:\n")
    io.write("  legado isolado     lua tools/ztc_legacy.lua <comando>\n")
    io.write("\n")
    io.write("Comandos:\n")
    io.write("  build              Compila para Lua (padrao: out.lua)\n")
    io.write("  run                Compila e executa imediatamente\n")
    io.write("  check              Analisa sem gerar codigo\n")
    io.write("  version            Mostra a versao do front door\n")
    io.write("  help               Mostra esta ajuda\n")
    io.write("  zpm                Executa o CLI Zenith Package Manager\n")
    io.write("  zman               Executa o manual textual da linguagem\n")
    io.write("  ztest              Executa o runner de testes da linguagem\n")
end

local function print_version()
    io.write("Zenith Front Door v" .. VERSION .. " (default engine: selfhost)\n")
end

local function runtime_lua_path()
    return table.concat({
        repo_path("?.lua"),
        repo_path("?/init.lua"),
        repo_path("src/?.lua"),
        repo_path("src/?/init.lua"),
        ";;",
    }, ";")
end

local function normalize_exit_code(ok, why, code)
    if type(ok) == "number" then
        return ok
    end
    if ok == true then
        if type(code) == "number" then
            return code
        end
        return 0
    end
    if type(code) == "number" then
        return code
    end
    return 1
end

local function run_lua_script(path, extra_args)
    local cmd = "lua " .. quote_arg(path)
    for _, arg_value in ipairs(extra_args or {}) do
        cmd = cmd .. " " .. quote_arg(arg_value)
    end

    local lua_path = runtime_lua_path()
    local frontdoor_path = repo_path("ztc.lua")
    local repo_root = REPO_ROOT

    if is_windows() then
        cmd = 'set "LUA_PATH=' .. lua_path .. '" && '
            .. 'set "ZTC_FRONTDOOR_PATH=' .. frontdoor_path .. '" && '
            .. 'set "ZTC_REPO_ROOT=' .. repo_root .. '" && '
            .. cmd
    else
        cmd = "LUA_PATH=" .. quote_arg(lua_path)
            .. " ZTC_FRONTDOOR_PATH=" .. quote_arg(frontdoor_path)
            .. " ZTC_REPO_ROOT=" .. quote_arg(repo_root)
            .. " " .. cmd
    end

    local ok, why, code = os.execute(cmd)
    return normalize_exit_code(ok, why, code)
end

local function unique_temp_path(area, prefix)
    local unique = tostring({}):gsub("[^%w]", "")
    local dir = DEFAULT_TEMP_DIR
    ensure_dir(dir)
    if area and area ~= "" then
        dir = path_join(dir, area)
        ensure_dir(dir)
    end
    return path_join(dir, string.format("%s-%d-%s.lua", prefix, os.time(), unique))
end

local function delete_file_if_exists(path)
    os.remove(path)
end

local current_legacy_audit_file = nil

local function audit_escape(value)
    local s = tostring(value or "")
    s = s:gsub("[\r\n\t]", " ")
    return s
end

local function append_legacy_audit(event_name, fields)
    if not current_legacy_audit_file or current_legacy_audit_file == "" then
        return
    end

    local parts = {
        "event=" .. audit_escape(event_name),
    }

    local ordered_keys = { "reason", "mode", "input", "command", "detail" }
    for _, key in ipairs(ordered_keys) do
        local value = fields and fields[key] or nil
        if value ~= nil and value ~= "" then
            parts[#parts + 1] = key .. "=" .. audit_escape(value)
        end
    end

    local handle = io.open(current_legacy_audit_file, "a")
    if not handle then
        return
    end
    handle:write(table.concat(parts, "\t") .. "\n")
    handle:close()
end

local function split_lines(text)
    local lines = {}
    local index = 1
    text = text or ""
    for line in (text .. "\n"):gmatch("(.-)\n") do
        lines[index] = line
        index = index + 1
    end
    return lines
end

local function print_selfhost_diagnostics(bag, source)
    local lines = split_lines(source)
    local count = tonumber(bag and bag.count or 0) or 0
    for i = 0, count - 1 do
        local d = bag.diagnostics and bag.diagnostics[i] or nil
        if d then
            io.stderr:write(string.format("[%s] %s\n", tostring(d.id or "ZT-SH"), tostring(d.message or "diagnostico sem mensagem")))
            local sp = d.span
            if sp then
                local file = sp.file or "<input>"
                local line = tonumber(sp.line or 0) or 0
                local column = tonumber(sp.column or 0) or 0
                io.stderr:write(string.format("  at %s:%d:%d\n", tostring(file), line, column))
                local line_text = lines[line]
                if line_text then
                    io.stderr:write(string.format("   |\n %d | %s\n", line, line_text))
                    if column > 0 then
                        io.stderr:write(string.format("   | %s^\n", string.rep(" ", column - 1)))
                    end
                end
            end
            io.stderr:write("\n")
        end
    end
end

local function write_source_map(path, input_path, source_map)
    local lines = {
        "-- Zenith Source Map",
        string.format("-- Source: %s", input_path),
        "return {",
        string.format("  source = %q,", input_path),
        "  mappings = {",
    }
    if source_map then
        for lua_line, loc in pairs(source_map) do
            lines[#lines + 1] = string.format("    [%d] = {line=%d, col=%d},", lua_line, loc.line, loc.col)
        end
    end
    lines[#lines + 1] = "  }"
    lines[#lines + 1] = "}"
    local ok, err = write_text(path, table.concat(lines, "\n"))
    return ok, err
end

local function parse_engine_args(args)
    local env_legacy = os.getenv("ZTC_ENGINE") == "legacy"
    local options = {
        engine = env_legacy and "legacy" or "selfhost",
        engine_source = env_legacy and "env-legacy" or "default-selfhost",
        strict_selfhost = false,
        audit_legacy_file = nil,
        legacy_blocked_message = nil,
    }
    local filtered = {}
    local i = 1
    while i <= #args do
        local value = args[i]
        if value == "--legacy" then
            if legacy_frontdoor_enabled() then
                options.engine = "legacy"
                options.engine_source = "flag-legacy"
            else
                options.legacy_blocked_message = "--legacy saiu da superficie oficial; use 'lua tools/ztc_legacy.lua ...' apenas para recuperacao"
            end
        elseif value == "--selfhost" then
            options.engine = "selfhost"
            options.engine_source = "flag-selfhost"
        elseif value == "--strict-selfhost" then
            options.engine = "selfhost"
            options.engine_source = "flag-strict-selfhost"
            options.strict_selfhost = true
        elseif value == "--audit-legacy" then
            i = i + 1
            local audit_path = args[i]
            if not audit_path then
                error("missing path for --audit-legacy")
            end
            options.audit_legacy_file = audit_path
        else
            filtered[#filtered + 1] = value
        end
        i = i + 1
    end
    return options, filtered
end

local function parse_compile_args(args)
    local mode = "build"
    local input_path = nil
    local out_path = DEFAULT_OUTPUT
    local first = args[1]

    if first == "build" or first == "run" or first == "check" then
        mode = first
        input_path = args[2]
        if args[3] then
            out_path = args[3]
        end
    else
        input_path = args[1]
        if args[2] then
            out_path = args[2]
        end
    end

    return mode, input_path, out_path
end

local function read_source_for_compile(path)
    local source, err = read_text(path)
    if not source then
        io.stderr:write(string.format("Erro: nao foi possivel abrir '%s': %s\n", path, tostring(err)))
        return nil
    end
    return source:gsub("\r", "")
end

local selfhost_cache = nil
local selfhost_cache_path = nil

local function load_selfhost_module(path)
    local previous_arg = rawget(_G, "arg")
    _G.arg = {}
    local ok, result = pcall(dofile, path)
    _G.arg = previous_arg
    if not ok then
        return nil, tostring(result)
    end
    if type(result) ~= "table" or type(result.compile_ext) ~= "function" then
        return nil, "modulo self-hosted sem export compile_ext"
    end
    return result
end

local function resolve_selfhost_module()
    if selfhost_cache then
        return selfhost_cache, selfhost_cache_path
    end

    local candidates = {
        repo_path("ztc_selfhost.lua"),
        repo_path(".selfhost-artifacts/bootstrap/syntax_stage2.lua"),
        repo_path("src/compiler/syntax.lua"),
    }

    local errors = {}
    for _, candidate in ipairs(candidates) do
        if file_exists(candidate) then
            local module, err = load_selfhost_module(candidate)
            if module then
                selfhost_cache = module
                selfhost_cache_path = candidate
                return module, candidate
            end
            errors[#errors + 1] = string.format("%s: %s", candidate, tostring(err))
        end
    end

    return nil, nil, table.concat(errors, " | ")
end

local function warn_selfhost_fallback(reason, audit_fields)
    append_legacy_audit("legacy-fallback", {
        reason = audit_fields and audit_fields.reason or "fallback",
        mode = audit_fields and audit_fields.mode or "",
        input = audit_fields and audit_fields.input or "",
        command = audit_fields and audit_fields.command or "",
        detail = tostring(reason),
    })
    io.stderr:write("[warn] selfhost indisponivel; fallback para pipeline legado: " .. tostring(reason) .. "\n")
end

local function selfhost_compile(module, input_path, options)
    options = options or {}
    local source = read_source_for_compile(input_path)
    if not source then
        return false, 1
    end

    local ok, result = pcall(module.compile_ext, source, input_path, detect_host_os())
    if not ok then
        return nil, tostring(result)
    end

    if type(result) == "table" and result.diagnostics then
        if options.print_diagnostics ~= false then
            print_selfhost_diagnostics(result, source)
        end
        return false, 1
    end

    if type(result) ~= "string" then
        return nil, "resultado inesperado do compile_ext self-hosted"
    end

    return true, result, source
end

local function selfhost_command(args, options)
    options = options or {}
    local module, compiler_path, load_err = resolve_selfhost_module()
    if not module then
        return nil, load_err or "nenhum artefato self-hosted encontrado"
    end

    local mode, input_path, out_path = parse_compile_args(args)
    if not input_path then
        print_usage()
        return false, 1
    end

    local ok, result_or_exit, source = selfhost_compile(module, input_path, options)
    if ok == nil then
        return nil, string.format("falha estrutural ao usar %s: %s", compiler_path, tostring(result_or_exit))
    end
    if ok == false then
        return false, result_or_exit
    end

    local lua_code = result_or_exit

    if mode == "check" then
        io.write(string.format("[ok][selfhost] %s: sem erros\n", input_path))
        return true, 0
    end

    if mode == "run" then
        local tmp_path = unique_temp_path("run", "selfhost-run")
        local write_ok, write_err = write_text(tmp_path, lua_code)
        if not write_ok then
            io.stderr:write(string.format("Erro: nao foi possivel escrever '%s': %s\n", tmp_path, tostring(write_err)))
            return true, 1
        end
        local exit_code = run_lua_script(tmp_path, {})
        os.remove(tmp_path)
        return true, exit_code
    end

    local write_ok, write_err = write_text(out_path, lua_code)
    if not write_ok then
        io.stderr:write(string.format("Erro: nao foi possivel escrever '%s': %s\n", out_path, tostring(write_err)))
        return true, 1
    end

    local map_ok, map_err = write_source_map(out_path .. ".map", input_path, nil)
    if not map_ok then
        io.stderr:write(string.format("[warn] falha ao escrever source map stub '%s.map': %s\n", out_path, tostring(map_err)))
    end

    io.write(string.format("[ok][selfhost] %s -> %s\n", input_path, out_path))
    return true, 0
end

local function legacy_run_pipeline(input_path)
    local global_bag = DiagnosticBag.new()

    local content = read_text(input_path)
    if not content then
        global_bag:report_error("ZT-C001", string.format("nao foi possivel abrir o arquivo '%s'", input_path), nil)
        return nil, nil, nil, global_bag
    end

    local source_text = source_mod.new(content, input_path)

    local unit, parse_diags
    local ok, parse_err = pcall(function()
        unit, parse_diags = parser.parse_string(content, input_path)
    end)

    if not ok then
        global_bag:report_error("ZT-C010", "erro interno no parser: " .. tostring(parse_err), nil)
        return nil, nil, source_text, global_bag
    end

    if parse_diags and parse_diags.diagnostics then
        global_bag:merge(parse_diags)
    end

    if global_bag:has_errors() then
        return nil, unit, source_text, global_bag
    end

    local ModuleManager = require("src.semantic.binding.module_manager")
    local manager = ModuleManager.new(".")
    local binder = binder_mod.new(global_bag, manager)
    local ok_bind, bind_err = pcall(function()
        binder:bind(unit, "")
    end)

    if not ok_bind then
        global_bag:report_error("ZT-C020", "erro interno no binder: " .. tostring(bind_err), nil)
        return nil, unit, source_text, global_bag
    end

    if global_bag:has_errors() then
        return nil, unit, source_text, global_bag
    end

    local lua_code, source_map
    local ok_emit, emit_err = pcall(function()
        local emitter = emitter_mod.new()
        lua_code, source_map = emitter:generate(unit, source_text)
    end)

    if not ok_emit then
        global_bag:report_error("ZT-C030", "erro interno no gerador de codigo: " .. tostring(emit_err), nil)
        return nil, unit, source_text, global_bag
    end

    return lua_code, unit, source_text, global_bag, source_map
end

local function legacy_command(args)
    local mode, input_path, out_path = parse_compile_args(args)
    if not input_path then
        print_usage()
        return 1
    end

    if mode == "check" then
        local _, _, source_text, bag = legacy_run_pipeline(input_path)
        DiagnosticRenderer.print_all(bag, source_text)
        local had_errors = DiagnosticRenderer.print_summary(bag)
        if had_errors then
            return 1
        end
        io.write(string.format("[ok][legacy] %s: sem erros\n", input_path))
        return 0
    end

    local lua_code, _, source_text, bag, source_map = legacy_run_pipeline(input_path)

    if bag:count() > 0 then
        DiagnosticRenderer.print_all(bag, source_text)
    end

    if DiagnosticRenderer.print_summary(bag) then
        return 1
    end

    if mode == "run" then
        local tmp_path = unique_temp_path("run", "legacy-run")
        local write_ok, write_err = write_text(tmp_path, lua_code)
        if not write_ok then
            io.stderr:write(string.format("Erro: nao foi possivel escrever '%s': %s\n", tmp_path, tostring(write_err)))
            return 1
        end
        local exit_code = run_lua_script(tmp_path, {})
        os.remove(tmp_path)
        return exit_code
    end

    local write_ok, write_err = write_text(out_path, lua_code)
    if not write_ok then
        io.stderr:write(string.format("Erro: nao foi possivel escrever '%s': %s\n", out_path, tostring(write_err)))
        return 1
    end

    local map_ok, map_err = write_source_map(out_path .. ".map", input_path, source_map)
    if not map_ok then
        io.stderr:write(string.format("[warn] falha ao escrever source map '%s.map': %s\n", out_path, tostring(map_err)))
    end

    io.write(string.format("[ok][legacy] %s -> %s\n", input_path, out_path))
    return 0
end

local invoke_legacy_command

local function compile_builtin_temp(engine_options, builtin_path, tmp_path)
    local compile_args = { "build", builtin_path, tmp_path }

    if engine_options.engine == "legacy" then
        return invoke_legacy_command(compile_args, {
            reason = "builtin-explicit-legacy",
            command = builtin_path,
            detail = engine_options.engine_source,
        })
    end

    local handled, result = selfhost_command(compile_args, {
        print_diagnostics = engine_options.strict_selfhost,
    })
    if handled == nil then
        io.stderr:write("[fatal] builtin selfhost falhou: " .. tostring(result) .. "\n")
        return 1
    end

    if result ~= 0 then
        io.stderr:write("[fatal] builtin oficial falhou no caminho self-hosted: " .. builtin_path .. "\n")
        return result
    end

    return 0
end

invoke_legacy_command = function(args, audit_fields)
    local mode, input_path = parse_compile_args(args)
    append_legacy_audit("legacy-command", {
        reason = audit_fields and audit_fields.reason or "explicit-legacy",
        mode = mode or "",
        input = input_path or "",
        command = audit_fields and audit_fields.command or "",
        detail = audit_fields and audit_fields.detail or "",
    })
    return legacy_command(args)
end

local function run_builtin_cli(engine_options, command, forwarded_args)
    local builtin_targets = {
        zpm = repo_path("src/cli/zpm.zt"),
        zman = repo_path("src/cli/zman.zt"),
        ztest = repo_path("src/cli/ztest.zt"),
    }

    local builtin_path = builtin_targets[command]
    if not builtin_path or not file_exists(builtin_path) then
        io.stderr:write(string.format("Erro: builtin '%s' nao encontrado.\n", tostring(command)))
        return 1
    end

    local tmp_path = unique_temp_path("builtin", "builtin-" .. command)
    local build_exit = compile_builtin_temp(engine_options, builtin_path, tmp_path)
    if build_exit ~= 0 then
        delete_file_if_exists(tmp_path)
        delete_file_if_exists(tmp_path .. ".map")
        return build_exit
    end

    local exit_code = run_lua_script(tmp_path, forwarded_args)
    delete_file_if_exists(tmp_path)
    delete_file_if_exists(tmp_path .. ".map")
    return exit_code
end

local function main(raw_args)
    local engine_options, args = parse_engine_args(raw_args or {})
    current_legacy_audit_file = engine_options.audit_legacy_file

    if engine_options.legacy_blocked_message then
        io.stderr:write("[fatal] " .. engine_options.legacy_blocked_message .. "\n")
        return 1
    end

    if #args == 0 then
        print_usage()
        return 0
    end

    local first = args[1]

    if first == "version" or first == "--version" or first == "-v" then
        print_version()
        return 0
    end

    if first == "help" or first == "--help" or first == "-h" then
        print_usage()
        return 0
    end

    if first == "zpm" or first == "zman" or first == "ztest" then
        local forwarded = {}
        for i = 2, #args do
            forwarded[#forwarded + 1] = args[i]
        end
        return run_builtin_cli(engine_options, first, forwarded)
    end

    if engine_options.engine == "legacy" then
        return invoke_legacy_command(args, {
            reason = "explicit-legacy",
            detail = engine_options.engine_source,
        })
    end

    local handled, result = selfhost_command(args)
    if handled == nil then
        io.stderr:write("[fatal] selfhost oficial falhou: " .. tostring(result) .. "\n")
        return 1
    end

    return result
end

local exit_code = main(arg or {})
os.exit(exit_code or 0)
