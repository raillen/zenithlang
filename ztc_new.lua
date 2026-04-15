-- Transpilado por Zenith v0.2.5
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure

local main

-- Namespace: main

local lexer = require("syntax/lexer")

local parser = require("syntax/parser")

local binder = require("semantic/binding/binder")

local emitter = require("backend/lua/lua_codegen")

local source = require("source")

local text = require("src/stdlib/text")

local io = require("src/stdlib/io")

local os = require("src/stdlib/os")

function main(args)
    if (#(args) == 0) then
        print("Zenith Sovereign Compiler v0.3.5")
        print("Uso: ztc <run|build> <arquivo.zt> [saida]")
        return
    end
    local mode = args[1]
    local input_path = ""
    if zt.unwrap_or((mode == "run"), (mode == "build")) then
        if (#(args) < 2) then
            print("Especifique um arquivo de entrada.")
            return
        end
        input_path = args[2]
    else
        input_path = args[1]
        mode = "build"
    end
    local content = io.read_file(input_path)
    if (content == "") then
        print(zt.add("Erro: Nao foi possivel ler o arquivo ", input_path))
        return
    end
    local src_text = source.SourceText.new({["text"] = content, ["filename"] = input_path})
    local unit
    local p_diags
    local src
    -- native lua
    
 unit , p_diags , src = parser . parse_string ( content , input_path ) 
 
    local b = binder.new()
    b:bind(unit, "")
    local emit = emitter.new()
    local lua_code = emit:generate(unit)
    if (mode == "run") then
        local out_path = ".zenith_tmp_run.lua"
        io.write_file(out_path, lua_code)
        -- native lua
        
 local cmd = "lua " .. out_path 
 _G . os . execute ( cmd ) 
 _G . os . remove ( out_path ) 
 
    else
        local out_path = "out.lua"
        if (#(args) >= 3) then
            out_path = args[3]
        elseif ((mode == "build") and (#(args) >= 2)) then
            out_path = args[2]
        elseif (#(args) >= 2) then
            out_path = args[2]
        end
        io.write_file(out_path, lua_code)
    end
end

-- Struct Methods

-- Auto-run main
if true then
    local status = main(arg)
    if type(status) == 'table' and status.co then status = zt.drive(status) end
    if type(status) == 'number' then os.exit(status) end
end

return {
    main = main,
    main = main,
}