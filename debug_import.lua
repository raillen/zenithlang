local SourceText = require("src.source.source_text")
local Parser     = require("src.syntax.parser.parser")

local code = 'import "math"'
local source = SourceText.new(code, "debug.zt")
local unit, diags = Parser.parse(source)

print("Count: " .. #unit.declarations)
if #unit.declarations > 0 then
    print("Kind: " .. tostring(unit.declarations[1].kind))
end
