local f = io.open("src/main.zt", "r")
local c = f:read("*a")
f:close()
print("Bytes of first 50 chars:")
for i=1,50 do
    local b = string.byte(c, i)
    if not b then break end
    print(string.format("%d: %d (%s)", i, b, string.char(b):match("%s") and "WS" or string.char(b)))
end
