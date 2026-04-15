-- Zenith self-host bootstrap verifier
-- Keeps bootstrap artifacts isolated and only promotes an output when explicitly requested.

local function is_windows()
    return package.config:sub(1, 1) == "\\"
end

local function quote(path)
    return '"' .. path .. '"'
end

local function command_succeeded(ok, why, code)
    if ok == true then
        return code == nil or code == 0
    end
    if type(ok) == "number" then
        return ok == 0
    end
    return false
end

local function run(cmd)
    print(">> " .. cmd)
    local ok, why, code = os.execute(cmd)
    if not command_succeeded(ok, why, code) then
        error(string.format("command failed: %s", cmd))
    end
end

local function ensure_dir(path)
    if is_windows() then
        run("if not exist " .. quote(path) .. " mkdir " .. quote(path))
    else
        run("mkdir -p " .. quote(path))
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

local function read_file(path)
    local handle = assert(io.open(path, "r"))
    local content = handle:read("*a")
    handle:close()
    return content
end

local function write_file(path, content)
    local handle = assert(io.open(path, "w"))
    handle:write(content)
    handle:close()
end

local function copy_file(source, target)
    write_file(target, read_file(source))
end

local function usage()
    print("Zenith self-host bootstrap verifier")
    print("Usage: lua tools/bootstrap.lua [--promote] [--target <path>] [--output-dir <dir>]")
    print("")
    print("Defaults:")
    print("  output-dir: .selfhost-bootstrap")
    print("  target:     ztc_selfhost.lua")
    print("")
    print("Behavior:")
    print("  - verifies the canonical self-hosted sources first")
    print("  - writes stage artifacts into an isolated directory")
    print("  - compares stage2 and stage3 for deterministic output")
    print("  - only copies the resulting compiler when --promote is passed")
end

local function parse_args(argv)
    local options = {
        promote = false,
        output_dir = ".selfhost-bootstrap",
        target = "ztc_selfhost.lua",
    }

    local i = 1
    while i <= #argv do
        local arg = argv[i]
        if arg == "--help" or arg == "-h" then
            options.help = true
        elseif arg == "--promote" then
            options.promote = true
        elseif arg == "--target" then
            i = i + 1
            options.target = argv[i]
        elseif arg == "--output-dir" then
            i = i + 1
            options.output_dir = argv[i]
        else
            error("unknown argument: " .. tostring(arg))
        end
        i = i + 1
    end

    return options
end

local function path_join(base, name)
    if is_windows() then
        return base .. "\\" .. name
    end
    return base .. "/" .. name
end

local function detect_host_os()
    return is_windows() and "windows" or "linux"
end

local function build_stage1(source_path, output_path)
    run("lua ztc.lua build " .. quote(source_path) .. " " .. quote(output_path))
end

local function load_stage_module(path)
    local previous_arg = rawget(_G, "arg")
    _G.arg = {}
    local ok, result = pcall(dofile, path)
    _G.arg = previous_arg
    if not ok then
        error(result)
    end
    if type(result) ~= "table" then
        error("stage did not return a module table: " .. path)
    end
    if type(result.compile_ext) ~= "function" then
        error("stage is missing compile_ext export: " .. path)
    end
    return result
end

local function validate_stage(path)
    local chunk, err = loadfile(path)
    if not chunk then
        error(err)
    end
end

local function self_compile(stage_path, source_path, output_path)
    local stage = load_stage_module(stage_path)
    local result = stage.compile_ext(read_file(source_path), source_path, detect_host_os())
    if type(result) ~= "string" then
        error("compile_ext did not return Lua text for " .. stage_path)
    end
    write_file(output_path, result)
end

local function verify_canonical_sources(source_path, legacy_bridge_path)
    print("--- Verifying canonical self-hosted sources ---")
    run("lua ztc.lua check " .. quote(source_path))
    if legacy_bridge_path and file_exists(legacy_bridge_path) then
        run("lua ztc.lua check " .. quote(legacy_bridge_path))
    end
end

local function main(argv)
    local options = parse_args(argv or {})
    if options.help then
        usage()
        return
    end

    if not file_exists("ztc.lua") then
        error("ztc.lua not found. Cannot bootstrap.")
    end

    local source_path = "src/compiler/syntax.zt"
    local legacy_bridge_path = "src/compiler/syntax_bridge.zt"

    if not file_exists(source_path) then
        error(source_path .. " not found. Cannot bootstrap.")
    end

    ensure_dir(options.output_dir)
    verify_canonical_sources(source_path, legacy_bridge_path)

    local stage1 = path_join(options.output_dir, "syntax_stage1.lua")
    local stage2 = path_join(options.output_dir, "syntax_stage2.lua")
    local stage3 = path_join(options.output_dir, "syntax_stage3.lua")
    local summary = path_join(options.output_dir, "summary.txt")
    local results = {}

    local function record(name, ok, detail)
        results[#results + 1] = {
            name = name,
            ok = ok,
            detail = detail or "",
        }
    end

    local function run_step(name, fn)
        local ok, err = pcall(fn)
        if ok then
            record(name, true, "ok")
            return true
        end
        record(name, false, tostring(err))
        return false
    end

    print("--- Stage 1: bootstrap from active compiler ---")
    local stage1_ok = run_step("stage1_build", function()
        build_stage1(source_path, stage1)
    end)

    local stage1_load_ok = false
    if stage1_ok then
        stage1_load_ok = run_step("stage1_load", function()
            validate_stage(stage1)
        end)
    end

    local stage2_ok = false
    local stage2_load_ok = false
    if stage1_load_ok then
        print("--- Stage 2: self-compilation ---")
        stage2_ok = run_step("stage2_build", function()
            self_compile(stage1, source_path, stage2)
        end)
        if stage2_ok then
            stage2_load_ok = run_step("stage2_load", function()
                validate_stage(stage2)
            end)
        end
    end

    local stage3_ok = false
    if stage2_load_ok then
        print("--- Stage 3: determinism check ---")
        stage3_ok = run_step("stage3_build", function()
            self_compile(stage2, source_path, stage3)
        end)
        if stage3_ok then
            run_step("stage3_load", function()
                validate_stage(stage3)
            end)
        end
    end

    local deterministic = false
    if stage2_ok and stage3_ok and file_exists(stage2) and file_exists(stage3) then
        deterministic = read_file(stage2) == read_file(stage3)
    end

    local lines = {
        "Zenith self-host bootstrap summary",
        "canonical=" .. source_path,
        "stage1=" .. stage1,
        "stage2=" .. stage2,
        "stage3=" .. stage3,
        "deterministic=" .. tostring(deterministic),
        "promoted=" .. tostring(options.promote and deterministic),
    }
    for _, result in ipairs(results) do
        lines[#lines + 1] = string.format("%s=%s", result.name, result.ok and "ok" or "failed")
        if result.detail ~= "" then
            lines[#lines + 1] = string.format("%s_detail=%s", result.name, result.detail)
        end
    end

    if deterministic then
        print("[OK] Stage 2 and Stage 3 are identical.")
        if options.promote then
            copy_file(stage2, options.target)
            print("[OK] Promoted self-hosted compiler to " .. options.target)
            lines[#lines + 1] = "target=" .. options.target
        else
            print("[OK] Promotion skipped. Use --promote to copy stage2 to a target path.")
        end
    elseif stage2_ok and stage3_ok then
        print("[WARN] Stage 2 and Stage 3 differ. Promotion was skipped.")
    else
        print("[WARN] Bootstrap did not reach deterministic comparison. See summary for the failing stage.")
    end

    write_file(summary, table.concat(lines, "\n") .. "\n")
    print("[OK] Summary written to " .. summary)

    for _, result in ipairs(results) do
        if not result.ok then
            error(string.format("%s failed: %s", result.name, result.detail))
        end
    end
    if stage2_ok and stage3_ok and not deterministic then
        error("stage2 and stage3 differ")
    end
end

local ok, err = pcall(main, arg or {})
if not ok then
    io.stderr:write("bootstrap failed: " .. tostring(err) .. "\n")
    os.exit(1)
end
