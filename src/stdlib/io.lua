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
    local line = io . read ( "*l" ) if line then return Optional . Present ( line ) else return Optional . Empty end
end

return {
    write = write,
    write_line = write_line,
    read_line = read_line,
}
