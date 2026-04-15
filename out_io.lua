-- Transpilado por Zenith v0.2.5
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure

local lua_write, lua_print, lua_read, write, write_line, read_line

-- Namespace: std.io

function write(value)
    lua_write(tostring(value))
end

function write_line(value)
    lua_print(tostring(value))
end

function read_line()
    local line = lua_read("*l")
    if (line ~= nil) then
        return Present(line)
    end
    return Empty
end

-- Struct Methods
local lua_write = zt.ffi_bind("io.write")

local lua_print = zt.ffi_bind("print")

local lua_read = zt.ffi_bind("io.read")


return {
    write = write,
    write_line = write_line,
    read_line = read_line,
}