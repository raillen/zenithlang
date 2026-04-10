-- Implementação Nativa da std.http para Zenith
local zt = require("src.backend.lua.runtime.zenith_rt")
local json = require("src/stdlib/json")
local process = require("src/stdlib/os/process")

local HTTP = {}

-- 📂 AUXILIARES
local is_windows = package.config:sub(1,1) == "\\"
local curl_cmd = is_windows and "curl.exe" or "curl"

local function create_response(status, body, headers)
    local res = {
        status = status,
        body = body,
        headers = headers or {},
        _tag = "Response"
    }
    
    function res:json()
        return json.parse(self.body)
    end
    
    return res
end

-- 🚀 CLIENTE ASYNC

function HTTP.get(url, timeout)
    return zt.async_run(function()
        -- Usamos curl com -i para pegar os headers e -s para silenciar progresso
        local args = { "-i", "-s", "--max-time", tostring((timeout or 30000) / 1000), url }
        
        local p_res = process.spawn(curl_cmd, args, true)
        if p_res._tag == "Failure" then return p_res end
        
        local p = p_res.value
        local exit_code = zt.await(p:wait())
        
        if exit_code ~= 0 then
            return zt.Outcome.Failure("Falha na requisição HTTP (Código curl: " .. tostring(exit_code) .. ")")
        end
        
        local raw_output = zt.unwrap_or(p:read_output(), "")
        
        -- Separar Headers do Corpo (Busca robusta por \n\n ou \r\n\r\n)
        local header_end = raw_output:find("\n\r?\n")
        local body_part = header_end and raw_output:sub(header_end + 1) or raw_output
        local header_part = header_end and raw_output:sub(1, header_end) or ""
        
        -- Limpa espaços iniciais do corpo
        body_part = body_part:gsub("^%s+", "")
        
        -- Extrair Status Code
        local status = tonumber(header_part:match("HTTP/%d%.%d%s+(%d+)")) or 200
        
        return zt.Outcome.Success(create_response(status, body_part))
    end)
end

function HTTP.post(url, body, headers, timeout)
    return zt.async_run(function()
        local body_str = type(body) == "table" and json.stringify(body) or tostring(body)
        
        local args = { "-i", "-s", "-X", "POST", "--data", body_str, "--max-time", tostring((timeout or 30000) / 1000) }
        
        for k, v in pairs(headers or {}) do
            table.insert(args, "-H")
            table.insert(args, k .. ": " .. v)
        end
        
        if type(body) == "table" and not (headers and headers["Content-Type"]) then
            table.insert(args, "-H")
            table.insert(args, "Content-Type: application/json")
        end
        
        table.insert(args, url)
        
        local p_res = process.spawn(curl_cmd, args, true)
        if p_res._tag == "Failure" then return p_res end
        
        local p = p_res.value
        local exit_code = zt.await(p:wait())
        
        local raw_output = zt.unwrap_or(p:read_output(), "")
        local header_end = raw_output:find("\r?\n\r?\n")
        local body_part = header_end and raw_output:sub(header_end + 4) or raw_output
        local status = tonumber(raw_output:match("HTTP/%d%.%d%s+(%d+)")) or 200
        
        return zt.Outcome.Success(create_response(status, body_part))
    end)
end

return HTTP
