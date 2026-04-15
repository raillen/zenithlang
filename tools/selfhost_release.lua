local manifest = dofile("tools/selfhost_abi.lua")
local runtime = require("src.backend.lua.runtime.zenith_rt")

local function is_windows()
    return package.config:sub(1, 1) == "\\"
end

local function quote(path)
    return '"' .. tostring(path or "") .. '"'
end

local function path_join(base, name)
    if is_windows() then
        return base .. "\\" .. name
    end
    return base .. "/" .. name
end

local function ensure_dir(path)
    if is_windows() then
        os.execute("if not exist " .. quote(path) .. " mkdir " .. quote(path))
    else
        os.execute("mkdir -p " .. quote(path))
    end
end

local function parse_args(argv)
    local options = {
        audit_legacy = nil,
        output_dir = ".selfhost-artifacts/release",
    }

    local i = 1
    while i <= #argv do
        local arg = argv[i]
        if arg == "--audit-legacy" then
            i = i + 1
            options.audit_legacy = argv[i]
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

local function inject_audit_flag(cmd, audit_path)
    if not audit_path or audit_path == "" then
        return cmd
    end

    local audit_flag = " --audit-legacy " .. quote(audit_path)
    if cmd:match("^lua ztc%.lua") then
        return (cmd:gsub("^lua ztc%.lua", "lua ztc.lua" .. audit_flag, 1))
    end
    if cmd:match("^lua tools/bootstrap%.lua") then
        return (cmd:gsub("^lua tools/bootstrap%.lua", "lua tools/bootstrap.lua" .. audit_flag, 1))
    end
    return cmd
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

local function read_file(path)
    local handle = assert(io.open(path, "r"))
    local content = handle:read("*a")
    handle:close()
    return content
end

local function file_exists(path)
    local handle = io.open(path, "r")
    if not handle then
        return false
    end
    handle:close()
    return true
end

local function delete_if_exists(path)
    if file_exists(path) then
        os.remove(path)
    end
end

local function collect_extern_names(source_text)
    local names = {}
    for name in source_text:gmatch('extern func "([^"]+)"') do
        names[name] = true
    end
    return names
end

local function collect_manifest_names()
    local names = {}
    for _, name in ipairs(manifest.names or {}) do
        names[name] = true
    end
    return names
end

local function assert_same_name_set(expected, actual, label_expected, label_actual)
    for name in pairs(expected) do
        if not actual[name] then
            error(string.format("%s missing in %s: %s", label_expected, label_actual, name))
        end
    end
    for name in pairs(actual) do
        if not expected[name] then
            error(string.format("%s missing in %s: %s", label_actual, label_expected, name))
        end
    end
end

local function verify_runtime_bindings()
    local table_ref = runtime.selfhost or {}
    for _, name in ipairs(manifest.names or {}) do
        local short = name:gsub("^zt%.selfhost%.", "")
        if type(table_ref[short]) ~= "function" then
            error("runtime binding missing: " .. name)
        end
    end
end

local function cleanup_release_outputs(output_dir)
    delete_if_exists(path_join(output_dir, "selfhost-release-demo.lua"))
    delete_if_exists(path_join(output_dir, "selfhost-release-demo.lua.map"))
end

local function main(argv)
    local options = parse_args(argv or {})
    local syntax_source = read_file("src/compiler/syntax.zt")
    local extern_names = collect_extern_names(syntax_source)
    local manifest_names = collect_manifest_names()

    ensure_dir(".selfhost-artifacts")
    ensure_dir(options.output_dir)

    assert_same_name_set(manifest_names, extern_names, "manifest", "syntax.zt externs")
    verify_runtime_bindings()

    run(inject_audit_flag("lua tools/bootstrap.lua --promote --target ztc_selfhost.lua", options.audit_legacy))

    for _, cmd in ipairs(manifest.corpus or {}) do
        run(inject_audit_flag(cmd, options.audit_legacy))
    end

    cleanup_release_outputs(options.output_dir)

    print("[OK] selfhost release gate passed")
end

local ok, err = pcall(main, arg or {})
if not ok then
    io.stderr:write("selfhost release failed: " .. tostring(err) .. "\n")
    os.exit(1)
end
