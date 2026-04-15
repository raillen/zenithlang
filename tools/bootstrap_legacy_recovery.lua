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

local function parse_args(argv)
    local options = {
        target = "ztc_selfhost.lua",
    }

    local i = 1
    while i <= #argv do
        local value = argv[i]
        if value == "--target" then
            i = i + 1
            options.target = argv[i]
        elseif value == "--help" or value == "-h" then
            options.help = true
        else
            error("unknown argument: " .. tostring(value))
        end
        i = i + 1
    end

    return options
end

local function usage()
    print("Zenith bootstrap legacy recovery")
    print("Usage: lua tools/bootstrap_legacy_recovery.lua [--target <path>]")
    print("")
    print("This tool is for extraordinary recovery only.")
    print("It rebuilds the promoted self-hosted compiler from the isolated legacy entrypoint.")
end

local function main(argv)
    local options = parse_args(argv or {})
    if options.help then
        usage()
        return
    end

    run("lua tools/ztc_legacy.lua check " .. quote("src/compiler/syntax.zt"))
    run("lua tools/ztc_legacy.lua build " .. quote("src/compiler/syntax.zt") .. " " .. quote(options.target))
    print("[OK] Recovery compiler written to " .. options.target)
end

local ok, err = pcall(main, arg or {})
if not ok then
    io.stderr:write("bootstrap legacy recovery failed: " .. tostring(err) .. "\n")
    os.exit(1)
end
