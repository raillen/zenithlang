-- Zenith Static Bundler (tools/bundler.lua)
-- Transforma o compilador modular em um único arquivo independente.

local function read_file(path)
    local f = io.open(path, "r")
    if not f then return nil end
    local content = f:read("*a")
    f:close()
    return content
end

local function bundle()
    print("--- 📦 Iniciando Empacotamento Soberano ---")
    
    local output = {}
    table.insert(output, "-- Zenith Sovereign Compiler v0.3.5 (Bundled)")
    table.insert(output, "-- Gerado automaticamente pelo Zenith Bundler\n")

    -- 1. Injetar o Runtime
    print("1. Injetando Runtime...")
    local rt = read_file("src/backend/lua/runtime/zenith_rt.lua")
    if not rt then error("Runtime nao encontrado!") end
    
    -- Ajuste para que o require("src.backend.lua.runtime.zenith_rt") funcione internamente
    table.insert(output, "package.preload['src.backend.lua.runtime.zenith_rt'] = function()")
    table.insert(output, rt)
    table.insert(output, "end\n")

    -- 2. Injetar Bibliotecas Padrão (necessárias para o compilador)
    local std_libs = { "io", "os", "text" }
    for _, lib in ipairs(std_libs) do
        print("2. Injetando std." .. lib .. "...")
        local content = read_file("src/stdlib/" .. lib .. ".lua")
        table.insert(output, "package.preload['src/stdlib/" .. lib .. "'] = function()")
        table.insert(output, content)
        table.insert(output, "end\n")
    end

    -- 3. Injetar Módulos do Compilador
    local modules = {
        ["source"] = "src/source/init.lua",
        ["source.source_text"] = "src/source/source_text.lua",
        ["src.source.line_map"]  = "src/source/line_map.lua",
        ["src.source.location"]  = "src/source/location.lua",
        ["src.source.span"]      = "src/source/span.lua",
        ["syntax.lexer"] = "src/syntax/lexer/init.lua",
        ["syntax.lexer.lexer"] = "src/syntax/lexer/lexer.lua",
        ["syntax.parser"] = "src/syntax/parser/init.lua",
        ["syntax.parser.parser"] = "src/syntax/parser/parser.lua",
        ["syntax.parser.parser_context"] = "src/syntax/parser/parser_context.lua",
        ["syntax.parser.parse_expressions"] = "src/syntax/parser/parse_expressions.lua",
        ["syntax.parser.parse_statements"] = "src/syntax/parser/parse_statements.lua",
        ["syntax.ast.syntax_kind"] = "src/syntax/ast/syntax_kind.lua",
        ["syntax.ast.syntax_node"] = "src/syntax/ast/syntax_node.lua",
        ["syntax.ast.expr_syntax"] = "src/syntax/ast/expr_syntax.lua",
        ["syntax.ast.stmt_syntax"] = "src/syntax/ast/stmt_syntax.lua",
        ["syntax.ast.decl_syntax"] = "src/syntax/ast/decl_syntax.lua",
        ["semantic.binding.binder"] = "src/semantic/binding/binder.lua",
        ["semantic.symbols.symbol"] = "src/semantic/symbols/symbol.lua",
        ["semantic.symbols.scope"] = "src/semantic/symbols/scope.lua",
        ["semantic.symbols.prelude"] = "src/semantic/symbols/prelude.lua",
        ["semantic.types.zenith_type"] = "src/semantic/types/zenith_type.lua",
        ["semantic.types.builtin_types"] = "src/semantic/types/builtin_types.lua",
        ["semantic.types.type_checker"] = "src/semantic/types/type_checker.lua",
        ["diagnostics.diagnostic_bag"] = "src/diagnostics/diagnostic_bag.lua",
        ["backend.lua.lua_codegen"] = "src/backend/lua/lua_codegen.lua",
    }

    for name, path in pairs(modules) do
        print("3. Injetando modulo: " .. name)
        local content = read_file(path)
        if not content then 
            print("   [AVISO] Modulo nao encontrado em " .. path)
        else
            table.insert(output, "package.preload['" .. name .. "'] = function()")
            table.insert(output, content)
            table.insert(output, "end\n")
            
            -- Variacoes para garantir que require funcione com pontos ou barras, e com/sem prefixo src
            local alt_names = {
                "src." .. name,
                name:gsub("%.", "/"),
                "src/" .. name:gsub("%.", "/"),
                name .. ".init",
                name .. "/init"
            }
            for _, alt in ipairs(alt_names) do
                table.insert(output, string.format("package.preload[%q] = package.preload[%q]", alt, name))
            end
        end
    end

    -- 4. Injetar o Entry Point (ztc.lua atual)
    print("4. Injetando Entry Point...")
    local entry = read_file("ztc.lua")
    table.insert(output, entry)

    -- 5. Gravar Bundle
    print("5. Gravando dist/ztc.lua...")
    os.execute("mkdir dist 2>nul")
    local f = io.open("dist/ztc.lua", "w")
    f:write(table.concat(output, "\n"))
    f:close()
    
    print("\n--- ✅ Bundle concluido com sucesso! ---")
    print("Arquivo gerado: dist/ztc.lua")
end

bundle()
