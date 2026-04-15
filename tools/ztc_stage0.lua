local syntax = require("src.compiler.syntax")

local args = {...}
local input_path = args[1]
local output_path = args[2]

if not input_path then
    print("Usage: luajit tools/ztc_stage0.lua <input.zt> [output.lua]")
    os.exit(1)
end

local f = io.open(input_path, "r")
if not f then
    print("Could not open input file: " .. input_path)
    os.exit(1)
end
local src = f:read("*all")
f:close()

local result = syntax.compile_ext(src, input_path, "windows")

if type(result) == "string" then
    if output_path then
        local out = io.open(output_path, "w")
        out:write(result)
        out:close()
        print("Transpiled " .. input_path .. " to " .. output_path)
    else
        print(result)
    end
else
    -- Errors (DiagnosticBag)
    print("Compilation failed with " .. result.count .. " errors:")
    for i = 0, result.count - 1 do
        local d = result.diagnostics[i]
        print(string.format("[%s] %s:%d:%d: %s", d.code, d.span.file, d.span.line, d.span.column, d.message))
    end
    os.exit(1)
end
