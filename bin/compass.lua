-- Transpilado por Zenith v0.2.5
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure

local main

-- Namespace: lsp.compass

function main()
    -- native lua
    
 
 package . path = "./src/?.lua;./src/?/init.lua;" .. package . path 
 
 local Parser = require ( "src.syntax.parser.parser" ) 
 local Binder = require ( "src.semantic.binding.binder" ) 
 local ModuleManager = require ( "src.semantic.binding.module_manager" ) 
 local json_lib = require ( "src.common.json" ) 
 
 
 local function read_message ( ) 
 local header = io . read ( "*l" ) 
 if not header then return nil end 
 local length = header : match ( "Content%-Length: (%d+)" ) 
 if not length then return nil end 
 io . read ( "*l" ) 
 local body = io . read ( tonumber ( length ) ) 
 if not body then return nil end 
 local ok , data = pcall ( function ( ) return json_lib . decode ( body ) end ) 
 return ok and data or nil 
 end 
 
 local function send_message ( msg ) 
 local body = json_lib . encode ( msg ) 
 io . write ( "Content-Length: " .. # body .. "\r\n\r\n" ) 
 io . write ( body ) 
 io . flush ( ) 
 end 
 
 
 while true do 
 local msg = read_message ( ) 
 if not msg then break end 
 
 if msg . method == "initialize" then 
 send_message ( { 
 jsonrpc = "2.0" , 
 id = msg . id , 
 result = { 
 capabilities = { textDocumentSync = 1 } , 
 serverInfo = { name = "Compass" , version = "0.3.11-industrial" } 
 } 
 } ) 
 elseif msg . method == "textDocument/didOpen" or msg . method == "textDocument/didChange" then 
 local params = msg . params 
 local doc = params . textDocument 
 local uri = doc . uri 
 local content = doc . text 
 
 if msg . method == "textDocument/didChange" then 
 content = params . contentChanges [ 1 ] . text 
 end 
 
 local unit , diags = Parser . parse_string ( content , uri ) 
 local mm = ModuleManager . new ( "." ) 
 local binder = Binder . new ( diags , mm , "host" ) 
 binder : bind ( unit , "lsp" ) 
 
 local results = { } 
 if diags and diags . diagnostics then 
 for i = 0 , ( diags . count or 0 ) - 1 do 
 local d = diags . diagnostics [ i ] 
 if d then 
 table . insert ( results , { 
 range = { 
 start = { line = d . span . line - 1 , character = d . span . column - 1 } , 
 [ "end" ] = { line = d . span . line - 1 , character = d . span . column } 
 } , 
 severity = ( d . level or 0 ) + 1 , 
 message = d . message or "Erro de análise" , 
 code = d . id or "ZT-001" 
 } ) 
 end 
 end 
 end 
 
 send_message ( { 
 jsonrpc = "2.0" , 
 method = "textDocument/publishDiagnostics" , 
 params = { uri = uri , diagnostics = results } 
 } ) 
 
 elseif msg . method == "shutdown" then 
 send_message ( { jsonrpc = "2.0" , id = msg . id , result = nil } ) 
 os . exit ( 0 ) 
 end 
 end 
 end 
 
end

-- Struct Methods

-- Auto-run main if not in a namespace
if not true then
    local status = main()
    if type(status) == 'table' and status.co then status = zt.drive(status) end
    if type(status) == 'number' then os.exit(status) end
end

return {
    main = main,
    main = main,
}