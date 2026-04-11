-- Transpilado por Zenith v0.2.5
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure

local Message, read_message

-- Namespace: lsp.jsonrpc

local Message = {}
Message.__index = Message
Message._metadata = {
    name = "Message",
    fields = {
        { name = "jsonrpc", type = "any" },
        { name = "id", type = "any" },
        { name = "method", type = "any" },
        { name = "params", type = "any" },
        { name = "result", type = "any" },
        { name = "error", type = "any" },
    },
    methods = {
    }
}

function Message.new(fields)
    local self = setmetatable({}, Message)
    self.jsonrpc = fields.jsonrpc or nil
    self.id = fields.id or nil
    self.method = fields.method or nil
    self.params = fields.params or nil
    self.result = fields.result or nil
    self.error = fields.error or nil
    return self
end

function read_message()
    -- native lua
    
 local header = io . read ( "*l" ) 
 if not header then return nil end 
 
 local length = header : match ( "Content%-Length: (%d+)" ) 
 if not length then return nil end 
 
 
 io . read ( "*l" ) 
 
 local body = io . read ( tonumber ( length ) ) 
 if not body then return nil end 
 
 local ok , data = pcall ( function ( ) 
 return require ( "src.common.json" ) . decode ( body ) 
 end ) 
 
 if ok then return data else return nil end 
 end 
 
    function send_message(msg)
        -- native lua
        
 local json_lib = require ( "src.common.json" ) 
 local body = json_lib . encode ( msg ) 
 local length = # body 
 
 io . write ( "Content-Length: " .. length .. "\r\n\r\n" ) 
 io . write ( body ) 
 io . flush ( ) 
 
    end
end

-- Struct Methods

return {
    Message = Message,
    read_message = read_message,
    send_message = send_message,
}