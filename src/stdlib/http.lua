-- Implementação Nativa do Servidor HTTP Zenith via Sockets FFI
local zt = require("src.backend.lua.runtime.zenith_rt")
local json = require("src/stdlib/json")
local socket = require("src/stdlib/net/socket")

local HTTP = {}

-- 🛠️ HELPERS DE PARSING

local function parse_request(raw)
    if not raw or #raw == 0 then return nil end
    local req = { headers = {}, body = "" }
    
    local header_end = raw:find("\n\r?\n")
    local header_part = header_end and raw:sub(1, header_end) or raw
    local body_part = header_end and raw:sub(header_end + 1) or ""
    
    local lines = {}
    for l in header_part:gmatch("[^\r\n]+") do table.insert(lines, l) end
    if #lines == 0 then return nil end
    
    -- Primeira Linha: GET /rota HTTP/1.1
    local method, url = lines[1]:match("^(%w+)%s+(%S+)")
    if not method then return nil end
    req.method = method
    req.url = url
    
    -- Headers
    for j = 2, #lines do
        local k, v = lines[j]:match("^(.-):%s*(.*)$")
        if k then req.headers[k:lower()] = v end
    end
    
    req.body = body_part:gsub("^%s+", "")
    return req
end

-- 📡 ESTRUTURA DO SERVIDOR

function HTTP.server(port)
    local self = {
        port = port or 8080,
        max_payload_size = 10 * 1024 * 1024,
        routes = { GET = {}, POST = {}, PUT = {}, DELETE = {}, PATCH = {} },
        middlewares = {}
    }

    function self:on_get(path, handler) self.routes.GET[path] = handler end
    function self:on_post(path, handler) self.routes.POST[path] = handler end
    function self:on_put(path, handler) self.routes.PUT[path] = handler end
    function self:on_delete(path, handler) self.routes.DELETE[path] = handler end
    function self:on_patch(path, handler) self.routes.PATCH[path] = handler end
    
    function self:use(handler) table.insert(self.middlewares, handler) end

    function self:start()
        return zt.async_run(function()
            local srv = socket.tcp_server()
            local res = srv:listen(self.port)
            
            if res._tag == "Failure" then 
                print("✘ Falha estelar: " .. res.error)
                return
            end

            while true do
                local client_res = zt.await(srv:accept())
                if client_res._tag == "Success" then
                    local client = client_res.value
                    
                    zt.async_run(function()
                        local raw_res = zt.await(client:receive(4096))
                        if raw_res._tag == "Success" then
                            local req_data = parse_request(raw_res.value)
                            if req_data then
                                local response = { status_code = 200, _client = client }
                                
                                function response:status(c) self.status_code = c; return self end
                                
                                function response:send_text(txt, type)
                                    local head = "HTTP/1.1 " .. self.status_code .. " OK\r\n" ..
                                                 "Content-Type: " .. (type or "text/plain") .. "; charset=utf-8\r\n" ..
                                                 "Content-Length: " .. #txt .. "\r\n" ..
                                                 "Connection: close\r\n\r\n"
                                    zt.await(self._client:send(head .. txt))
                                    self._client:close()
                                end
                                
                                function response:send_html(html) self:send_text(html, "text/html") end
                                function response:send_json(obj) self:send_text(json.stringify(obj), "application/json") end

                                -- Middlewares e Rotas
                                for _, m in ipairs(self.middlewares) do m(req_data, response) end
                                local handler = self.routes[req_data.method] and self.routes[req_data.method][req_data.url]
                                if handler then
                                    local ok, err = pcall(handler, req_data, response)
                                    if not ok then
                                        response:status(500):send_html("<h1>🚀 Erro de Ignição (500)</h1><pre>" .. tostring(err) .. "</pre>")
                                    end
                                else
                                    response:status(404):send_text("404 - Rota não mapeada.")
                                end
                            end
                        else
                            client:close()
                        end
                    end)
                end
                coroutine.yield()
            end
        end)
    end

    return self
end

-- 🛰️ CLIENTE (Legado via Curl)
function HTTP.get(url, timeout)
    local process = require("src/stdlib/os/process")
    local curl_cmd = (package.config:sub(1,1) == "\\") and "curl.exe" or "curl"
    return zt.async_run(function()
        local args = { "-i", "-s", "--max-time", tostring((timeout or 30000)/1000), url }
        local p_res = process.spawn(curl_cmd, args, true)
        if p_res._tag == "Failure" then return p_res end
        local p = p_res.value
        zt.await(p:wait())
        local raw = zt.unwrap_or(p:read_output(), "")
        local header_end = raw:find("\n\r?\n")
        local body = header_end and raw:sub(header_end + 1):gsub("^%s+", "") or raw
        return zt.Outcome.Success({ status = 200, body = body, _tag = "Response", json = function(s) return json.parse(s.body) end })
    end)
end

return HTTP
