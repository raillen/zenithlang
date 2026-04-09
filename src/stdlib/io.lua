-- Standard Library Zenith — IO (Native)
local i = {}

function i.write(value)
    io.write(tostring(value))
end

function i.read_line()
    return io.read("*l")
end

function i.write_line(value)
    print(value)
end

return i
