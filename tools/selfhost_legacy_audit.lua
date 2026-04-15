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

local function normalize_exit_code(ok, why, code)
    if type(ok) == "number" then
        return ok
    end
    if ok == true then
        return type(code) == "number" and code or 0
    end
    return type(code) == "number" and code or 1
end

local function ensure_dir(path)
    if is_windows() then
        os.execute("if not exist " .. quote(path) .. " mkdir " .. quote(path))
    else
        os.execute("mkdir -p " .. quote(path))
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

local function delete_if_exists(path)
    if file_exists(path) then
        os.remove(path)
    end
end

local function parse_args(argv)
    local options = {
        output_dir = ".selfhost-artifacts/audit",
    }

    local i = 1
    while i <= #argv do
        local arg = argv[i]
        if arg == "--output-dir" then
            i = i + 1
            options.output_dir = argv[i]
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
    print("Zenith self-host legacy audit")
    print("Usage: lua tools/selfhost_legacy_audit.lua [--output-dir <dir>]")
    print("Default output-dir: .selfhost-artifacts/audit")
end

local function path_join(base, name)
    if is_windows() then
        return base .. "\\" .. name
    end
    return base .. "/" .. name
end

local function run_probe(name, cmd, log_path)
    delete_if_exists(log_path)
    print("--- Probe: " .. name .. " ---")
    print(">> " .. cmd)
    local ok, why, code = os.execute(cmd)
    return {
        name = name,
        command = cmd,
        log_path = log_path,
        ok = command_succeeded(ok, why, code),
        exit_code = normalize_exit_code(ok, why, code),
    }
end

local function parse_log(path)
    local events = {}
    if not file_exists(path) then
        return events
    end

    for line in read_file(path):gmatch("[^\r\n]+") do
        local event = {}
        for field in line:gmatch("[^\t]+") do
            local key, value = field:match("^([^=]+)=(.*)$")
            if key then
                event[key] = value
            end
        end
        events[#events + 1] = event
    end

    return events
end

local function append_probe_report(lines, probe, events)
    lines[#lines + 1] = "## " .. probe.name
    lines[#lines + 1] = ""
    lines[#lines + 1] = "- command: " .. probe.command
    lines[#lines + 1] = "- status: " .. (probe.ok and "ok" or ("failed (" .. tostring(probe.exit_code) .. ")"))
    lines[#lines + 1] = "- audit log: " .. probe.log_path
    lines[#lines + 1] = "- legacy events: " .. tostring(#events)
    if #events == 0 then
        lines[#lines + 1] = "- details: none"
    else
        for _, event in ipairs(events) do
            local parts = {
                "event=" .. tostring(event.event or ""),
                "reason=" .. tostring(event.reason or ""),
            }
            if event.mode and event.mode ~= "" then
                parts[#parts + 1] = "mode=" .. event.mode
            end
            if event.input and event.input ~= "" then
                parts[#parts + 1] = "input=" .. event.input
            end
            if event.command and event.command ~= "" then
                parts[#parts + 1] = "command=" .. event.command
            end
            if event.detail and event.detail ~= "" then
                parts[#parts + 1] = "detail=" .. event.detail
            end
            lines[#lines + 1] = "- " .. table.concat(parts, " | ")
        end
    end
    lines[#lines + 1] = ""
end

local function build_probes(output_dir)
    local zpm_log = path_join(output_dir, "audit-zpm.log")
    local zman_log = path_join(output_dir, "audit-zman.log")
    local ztest_log = path_join(output_dir, "audit-ztest.log")
    local bootstrap_log = path_join(output_dir, "audit-bootstrap.log")
    local release_log = path_join(output_dir, "audit-release.log")

    return {
        {
            name = "bootstrap",
            command = "lua tools/bootstrap.lua --audit-legacy " .. quote(bootstrap_log),
            log_path = bootstrap_log,
        },
        {
            name = "release",
            command = "lua tools/selfhost_release.lua --audit-legacy " .. quote(release_log),
            log_path = release_log,
        },
        {
            name = "cli-zpm",
            command = "lua ztc.lua --strict-selfhost --audit-legacy " .. quote(zpm_log) .. " zpm help",
            log_path = zpm_log,
        },
        {
            name = "cli-zman",
            command = "lua ztc.lua --strict-selfhost --audit-legacy " .. quote(zman_log) .. " zman show std.core",
            log_path = zman_log,
        },
        {
            name = "cli-ztest",
            command = "lua ztc.lua --strict-selfhost --audit-legacy " .. quote(ztest_log) .. " ztest --grep parser_tests/test_parser.lua",
            log_path = ztest_log,
        },
    }
end

local function main(argv)
    local options = parse_args(argv or {})
    if options.help then
        usage()
        return
    end

    ensure_dir(".selfhost-artifacts")
    ensure_dir(options.output_dir)

    local probes = build_probes(options.output_dir)
    local report_lines = {
        "Zenith self-host legacy audit",
        "generated_at=" .. os.date("!%Y-%m-%dT%H:%M:%SZ"),
        "",
    }

    local had_failures = false
    local has_legacy_events = false
    local total_legacy_events = 0
    local failed_probe_names = {}

    for _, probe in ipairs(probes) do
        local result = run_probe(probe.name, probe.command, probe.log_path)
        local events = parse_log(probe.log_path)
        total_legacy_events = total_legacy_events + #events
        if #events > 0 then
            has_legacy_events = true
        end
        append_probe_report(report_lines, result, events)
        if not result.ok then
            had_failures = true
            failed_probe_names[#failed_probe_names + 1] = probe.name
        end
    end

    table.insert(report_lines, 4, "summary.failed_probes=" .. tostring(#failed_probe_names))
    table.insert(report_lines, 5, "summary.legacy_events=" .. tostring(total_legacy_events))
    if #failed_probe_names > 0 then
        table.insert(report_lines, 6, "summary.failed_names=" .. table.concat(failed_probe_names, ","))
        table.insert(report_lines, 7, "")
    end

    local report_path = path_join(options.output_dir, "selfhost-legacy-audit.txt")
    write_file(report_path, table.concat(report_lines, "\n"))
    print("[OK] Audit report written to " .. report_path)

    if had_failures or has_legacy_events then
        io.stderr:write("[FAIL] Official surfaces still reference legacy execution. See " .. report_path .. "\n")
        os.exit(1)
    end

    print("[OK] No legacy execution detected in the audited official surfaces")
end

local ok, err = pcall(main, arg or {})
if not ok then
    io.stderr:write("selfhost legacy audit failed: " .. tostring(err) .. "\n")
    os.exit(1)
end
