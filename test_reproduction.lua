package.path = package.path .. ";./src/?.lua;./tests/?.lua;./?/init.lua;./src/?/init.lua"

local Parser     = require("src.syntax.parser.parser")
local SK         = require("src.syntax.ast.syntax_kind")

local function parse(code)
    local unit, diags = Parser.parse_string(code)
    if #diags > 0 then
        for _, d in ipairs(diags) do
            print("Error: " .. d.message)
        end
    end
    return unit.declarations
end

local function test_struct_init(code, expected_name)
    print("Testing: " .. code)
    local decls = parse(code)
    if not decls or #decls == 0 then
        print("FAIL: no declarations")
        os.exit(1)
    end
    local expr = decls[1].expression
    if not expr then
        print("FAIL: no expression in statement")
        os.exit(1)
    end
    if expr.kind == SK.STRUCT_INIT_EXPR then
        print("Type Name: " .. (expr.type_name or "nil"))
        if expr.type_name == expected_name then
            print("PASS")
        else
            print("FAIL: expected " .. expected_name .. ", got " .. (expr.type_name or "nil"))
            os.exit(1)
        end
    else
        print("FAIL: not a STRUCT_INIT_EXPR, got " .. expr.kind)
        os.exit(1)
    end
end

test_struct_init("source.SourceText { field: 1 }", "source.SourceText")
test_struct_init("pkg.sub.Type { x: 1 }", "pkg.sub.Type")
test_struct_init("Type<int> { x: 1 }", "Type")
test_struct_init("pkg.Type<int> { x: 1 }", "pkg.Type")

print("All tests passed!")
