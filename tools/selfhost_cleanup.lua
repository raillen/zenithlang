local function is_windows()
    return package.config:sub(1, 1) == "\\"
end

local function quote(path)
    return '"' .. tostring(path or "") .. '"'
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
        error("command failed: " .. cmd)
    end
end

local function remove_dir(path)
    if is_windows() then
        os.execute("if exist " .. quote(path) .. " rmdir /s /q " .. quote(path))
    else
        os.execute("rm -rf " .. quote(path))
    end
end

local function delete_if_exists(path)
    os.remove(path)
end

local function parse_args(argv)
    local options = {
        legacy_scratch = false,
    }

    local i = 1
    while i <= #argv do
        local arg = argv[i]
        if arg == "--legacy-scratch" then
            options.legacy_scratch = true
        elseif arg == "--help" or arg == "-h" then
            options.help = true
        else
            error("unknown argument: " .. tostring(arg))
        end
        i = i + 1
    end

    return options
end

local function usage()
    print("Zenith self-host cleanup")
    print("Usage: lua tools/selfhost_cleanup.lua [--legacy-scratch]")
    print("")
    print("Default cleanup:")
    print("  - .ztc-tmp/")
    print("  - .selfhost-artifacts/")
    print("")
    print("Optional cleanup:")
    print("  --legacy-scratch   remove ignored scratch files left in .selfhost-bootstrap/")
end

local function cleanup_legacy_scratch()
    local files = {
        ".selfhost-bootstrap/alias_probe.zt",
        ".selfhost-bootstrap/selfhost-legacy-audit.txt",
        ".selfhost-bootstrap/syntax_stage1.lua.map",
        ".selfhost-bootstrap/test_optional_strict.lua",
        ".selfhost-bootstrap/test_optional_strict.lua.map",
        ".selfhost-bootstrap/zpm_strict.lua",
        ".selfhost-bootstrap/zpm_strict.lua.map",
        ".selfhost-bootstrap/manual-audit.log",
        ".selfhost-bootstrap/parse_progress.log",
        ".selfhost-bootstrap/profile_all.log",
        ".selfhost-bootstrap/profile_parse.log",
        ".selfhost-bootstrap/profile_tokenize.log",
        ".selfhost-bootstrap/stage2_phase.log",
        ".selfhost-bootstrap/token_progress.log",
        ".selfhost-bootstrap/token_progress2.log",
    }

    for _, file in ipairs(files) do
        delete_if_exists(file)
    end
end

local function main(argv)
    local options = parse_args(argv or {})
    if options.help then
        usage()
        return
    end

    remove_dir(".ztc-tmp")
    remove_dir(".selfhost-artifacts")
    delete_if_exists("site/public/test-health.json")
    delete_if_exists("site\\public\\test-health.json")

    if options.legacy_scratch then
        cleanup_legacy_scratch()
    end

    print("[OK] Cleanup completed")
end

local ok, err = pcall(main, arg or {})
if not ok then
    io.stderr:write("selfhost cleanup failed: " .. tostring(err) .. "\n")
    os.exit(1)
end
