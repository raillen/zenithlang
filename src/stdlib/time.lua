-- Transpilado por Zenith Ascension (Native v0.4)
local zt = require("src.backend.lua.runtime.zenith_rt")

function get_timestamp()
    -- native lua
    return os . time ( )
end
function get_cpu_time()
    -- native lua
    return os . clock ( )
end
function format_date(timestamp, fmt)
    -- native lua
    return os . date ( fmt , timestamp )
end

return {
    get_timestamp = get_timestamp,
    get_cpu_time = get_cpu_time,
    format_date = format_date,
}
