-- Transpilado por Zenith Ascension (Native v0.4)
local zt = require("src.backend.lua.runtime.zenith_rt")

function write(value)
    -- native lua
    io . write ( tostring ( value ) )
end
function write_line(value)
    -- native lua
    print ( tostring ( value ) )
end
function read_line()
    -- native lua
    local line = io . read ( "*l" ) if line then return zt.Optional . Present ( line ) else return zt.Optional . Empty end
end
function read_file(path)
    local f = io.open(path, "r")
    if not f then return "" end
    local content = f:read("*a")
    f:close()
    return content
end
function write_file(path, content)
    local f = io.open(path, "w")
    if f then
        f:write(content)
        f:close()
    end
end

return {
    write = write,
    write_line = write_line,
    print = write,
    println = write_line,
    read_line = read_line,
    read_file = read_file,
    write_file = write_file,
}
