local syntax = require("out")

print("--- Iniciando Lexer Nativo ---")
local source = [[
func main(a: any) do
    var b = a
end
]]
local lexer = syntax.Lexer.new({ 
    source = source,
    pos = 0,
    line = 1,
    col = 1
})
local tokens = lexer:tokenize()
print("Tokens gerados: " .. #tokens)

print("\n--- Iniciando Parser Nativo ---")
local parser = syntax.Parser.new({ 
    tokens = tokens,
    pos = 0
})
local unit = parser:parse()
print("Unidade de Compilação gerada.")

print("\n--- Iniciando Binder Nativo ---")
local global_scope = syntax.Scope.new({ parent = nil, symbols = {} })
local binder = syntax.Binder.new({ 
    global_scope = global_scope, 
    current_scope = global_scope 
})
binder:bind_unit(unit)
print("Vinculação concluída.")

-- Verificação
print("\n--- Verificando Símbolos ---")
local main_decl = unit.declarations[0]
if main_decl and main_decl.kind == syntax.NodeKind.FuncDecl then
    print("Encontrada FuncDecl: " .. main_decl.name)
    
    local body = main_decl.body
    if body then
        local stmts_count = 0
        while body[stmts_count] ~= nil do stmts_count = stmts_count + 1 end
        print("Statements no corpo: " .. stmts_count)

        local var_decl = body[0]
        if var_decl then
            print("Encontrada primeira stmt, Kind: " .. (var_decl.kind or "nil"))
            if var_decl.kind == syntax.NodeKind.VarDecl then
                print("Encontrada VarDecl: " .. var_decl.name)
                
                local init = var_decl.body
                if init and init.kind == syntax.NodeKind.IdentifierExpr then
                    print("Encontrado IdentifierExpr: " .. init.name)
                    if init.symbol then
                        print("SÍMBOLO VINCULADO: " .. init.symbol.name .. " (Kind: " .. init.symbol.kind .. ")")
                        if init.symbol.name == "a" and init.symbol.kind == 3 then
                            print("SUCESSO: 'a' vinculado corretamente ao parâmetro!")
                        else
                            print("ERRO: Vínculo incorreto.")
                        end
                    else
                        print("ERRO: Símbolo não vinculado.")
                    end
                end
            else
                print("ERRO: Esperado VarDecl, encontrado " .. var_decl.kind)
            end
        else
            print("ERRO: Corpo está vazio.")
        end
    else
        print("ERRO: Corpo é nulo.")
    end
else
    print("ERRO: Declaração 'main' não encontrada.")
end
