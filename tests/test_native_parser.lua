-- ============================================================================
-- Zenith Test — Native Parser Validation
-- Valida se o parser transpilado do syntax.zt funciona corretamente.
-- ============================================================================

local zt = require("src.backend.lua.runtime.zenith_rt")
local syntax = require("out")

local source = [[
func somar(a: int, b: int) -> int do
    var x = a + b * 2
    return x
end

func main() do
    var result = somar(10, 5)
    if result == 20
        print("sucesso")
    end
end
]]

print("--- Iniciando Lexer Nativo ---")
local lexer = syntax.Lexer.new({ source = source, pos = 0, line = 1, col = 1 })
local tokens = lexer:tokenize()
print("Tokens gerados: " .. #tokens)

print("\n--- Iniciando Parser Nativo ---")
local parser = syntax.Parser.new({ tokens = tokens, pos = 0 })
local unit = parser:parse()

print("Unidade de Compilação gerada.")
print("Total de declarações: " .. #unit.declarations)

function dump_node(node, indent)
    indent = indent or ""
    if not node then return end
    
    local k = node.kind
    if k == syntax.NodeKind.FuncDecl then
        print(indent .. "FuncDecl: " .. node.name)
        if node.body then
            for i = 0, #node.body do
                dump_node(node.body[i], indent .. "  ")
            end
        end
    elseif k == syntax.NodeKind.VarDecl then
        print(indent .. "VarDecl: " .. node.name)
        if node.initializer then
            dump_node(node.initializer, indent .. "  Init: ")
        end
    elseif k == syntax.NodeKind.BinaryExpr then
        print(indent .. "BinaryExpr: (op " .. node.operator.text .. ")")
        dump_node(node.left, indent .. "  L: ")
        dump_node(node.right, indent .. "  R: ")
    elseif k == syntax.NodeKind.LiteralExpr then
        print(indent .. "Literal: " .. tostring(node.value))
    elseif k == syntax.NodeKind.IdentifierExpr then
        print(indent .. "Identifier: " .. node.name)
    elseif k == syntax.NodeKind.ReturnStmt then
        print(indent .. "ReturnStmt")
        dump_node(node.value, indent .. "  ")
    elseif k == syntax.NodeKind.IfStmt then
        print(indent .. "IfStmt")
        if node.body then
            for i = 0, #node.body do
                dump_node(node.body[i], indent .. "  ")
            end
        end
    elseif k == syntax.NodeKind.AssignStmt then
        print(indent .. "AssignStmt")
        dump_node(node.target, indent .. "  Target: ")
        dump_node(node.value, indent .. "  Value: ")
    elseif k == syntax.NodeKind.CallExpr then
        print(indent .. "CallExpr")
        dump_node(node.callee, indent .. "  Callee: ")
        if node.arguments then
            for i = 0, #node.arguments do
                dump_node(node.arguments[i], indent .. "  Arg["..i.."]: ")
            end
        end
    elseif k == syntax.NodeKind.ExprStmt then
        dump_node(node.value, indent)
    else
        print(indent .. "Node: " .. tostring(k))
    end
end

for i = 0, #unit.declarations do
    local d = unit.declarations[i]
    if d then
        print("Encontrada decl: " .. d.name)
        dump_node(d)
    end
end
