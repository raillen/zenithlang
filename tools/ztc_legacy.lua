-- Isolated maintenance entrypoint for the legacy Lua pipeline.
-- This tool is outside the official product surface.

_G.ZTC_ALLOW_LEGACY_FRONTDOOR = true

local forwarded = { "--legacy" }
for i = 1, #(arg or {}) do
    forwarded[#forwarded + 1] = arg[i]
end

arg = forwarded
dofile("ztc.lua")
