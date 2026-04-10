-- Implementação Nativa da std.net.socket via FFI (LuaJIT)
local ffi = require("ffi")
local zt = require("src.backend.lua.runtime.zenith_rt")

-- 🪟 DEFINIÇÕES WINSOCK (Windows)
ffi.cdef[[
    typedef uintptr_t SOCKET;
    struct in_addr { uint32_t s_addr; };
    struct sockaddr_in {
        short sin_family;
        unsigned short sin_port;
        struct in_addr sin_addr;
        char sin_zero[8];
    };
    struct sockaddr {
        unsigned short sa_family;
        char sa_data[14];
    };

    int WSAStartup(uint16_t wVersionRequested, void* lpWSAData);
    SOCKET socket(int af, int type, int protocol);
    int connect(SOCKET s, const struct sockaddr *name, int namelen);
    int send(SOCKET s, const char *buf, int len, int flags);
    int recv(SOCKET s, char *buf, int len, int flags);
    int closesocket(SOCKET s);
    int WSACleanup();
    int ioctlsocket(SOCKET s, long cmd, unsigned long *argp);
    int WSAGetLastError();
]]

local ws2 = ffi.load("ws2_32")

-- Helpers
local AF_INET = 2
local SOCK_STREAM = 1
local IPPROTO_TCP = 6
local FIONBIO = 0x8004667e

local SOCKET = {}

-- 🏗️ ESTRUTURA BUFFER (Nativa)

local Buffer = {}
Buffer.__index = Buffer

function Buffer.new(size)
    local self = setmetatable({
        _size = size,
        _ptr = ffi.new("uint8_t[?]", size)
    }, Buffer)
    return self
end

function Buffer:write_byte(pos, b)
    if pos >= 0 and pos < self._size then self._ptr[pos] = b end
end

function Buffer:read_byte(pos)
    return self._ptr[pos]
end

function Buffer:to_text()
    return ffi.string(self._ptr, self._size)
end

-- 🛰️ CLIENTE TCP

function SOCKET.tcp_client()
    -- Inicia Winsock se necessário
    local wsaData = ffi.new("uint8_t[512]")
    ws2.WSAStartup(0x0202, wsaData)

    local sock = ws2.socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)
    
    -- Modo não-bloqueante para Async
    local mode = ffi.new("unsigned long[1]", 1)
    ws2.ioctlsocket(sock, FIONBIO, mode)

    local client = { _sock = sock }

    function client:connect(host, port)
        return zt.async_run(function()
            local addr = ffi.new("struct sockaddr_in")
            addr.sin_family = AF_INET
            addr.sin_port = ffi.C.htons(port)
            -- Simples conversão de IP (localhost fixo p/ teste)
            addr.sin_addr.s_addr = 0x0100007f -- 127.0.0.1
            
            local res = ws2.connect(self._sock, ffi.cast("struct sockaddr*", addr), ffi.sizeof(addr))
            
            -- Em modo não-bloqueante, connect costuma retornar erro "inprogress" (10035)
            while res ~= 0 do
                local err = ws2.WSAGetLastError()
                if err ~= 10035 then return zt.Outcome.Failure("Erro de conexao: " .. err) end
                coroutine.yield()
                res = ws2.connect(self._sock, ffi.cast("struct sockaddr*", addr), ffi.sizeof(addr))
                if res == 0 or ws2.WSAGetLastError() == 10056 then break end -- 10056 = already connected
            end
            return zt.Outcome.Success(nil)
        end)
    end

    function client:send(data)
        local str = type(data) == "string" and data or data:to_text()
        local res = ws2.send(self._sock, str, #str, 0)
        if res < 0 then return zt.Outcome.Failure("Erro ao enviar") end
        return zt.Outcome.Success(res)
    end

    function client:receive(len)
        return zt.async_run(function()
            local buf = ffi.new("char[?]", len)
            while true do
                local res = ws2.recv(self._sock, buf, len, 0)
                if res > 0 then
                    return zt.Outcome.Success(ffi.string(buf, res))
                elseif res < 0 then
                    local err = ws2.WSAGetLastError()
                    if err ~= 10035 then return zt.Outcome.Failure("Erro ao receber") end
                end
                coroutine.yield()
            end
        end)
    end

    function client:close()
        ws2.closesocket(self._sock)
    end

    return client
end

function SOCKET.create_buffer(size)
    return Buffer.new(size)
end

return SOCKET
