local function compile_all()
    local cmd = "luajit tools/ztc_stage0.lua "
    os.execute(cmd .. "src/syntax/ast/nodes.zt src/syntax/ast/nodes.lua")
    os.execute(cmd .. "src/syntax/lexer/lexer.zt src/syntax/lexer/lexer.lua")
    os.execute(cmd .. "src/syntax/parser/parser.zt src/syntax/parser/parser.lua")
    os.execute(cmd .. "src/semantic/binding/binder.zt src/semantic/binding/binder.lua")
    os.execute(cmd .. "src/semantic/symbols/scope.zt src/semantic/symbols/scope.lua")
    os.execute(cmd .. "src/semantic/symbols/symbol.zt src/semantic/symbols/symbol.lua")
    
    os.execute(cmd .. "src/stdlib/io.zt src/stdlib/io.lua")
    os.execute(cmd .. "src/stdlib/text.zt src/stdlib/text.lua")
    os.execute(cmd .. "src/stdlib/os.zt src/stdlib/os.lua")
    os.execute(cmd .. "src/stdlib/fs.zt src/stdlib/fs.lua")
    
    os.execute(cmd .. "src/main.zt ztc_stage1.lua")
    
    print("Compilation done.")
end
compile_all()
