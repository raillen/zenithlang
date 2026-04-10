#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

// Linkar com ws2_32.lib no Windows
#pragma comment(lib, "ws2_32.lib")

// Estrutura para o Socket Zenith
typedef struct {
    SOCKET sock;
} zt_socket;

// Helper: Configurar Socket como Não-Bloqueante
static void set_nonblocking(SOCKET s) {
    u_long mode = 1;
    ioctlsocket(s, FIONBIO, &mode);
}

// 🏗️ FUNÇÕES EXPOSTAS AO LUA/ZENITH

static int l_net_create(lua_State *L) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        lua_pushnil(L);
        lua_pushstring(L, "Falha ao iniciar Winsock");
        return 2;
    }

    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) {
        lua_pushnil(L);
        lua_pushstring(L, "Nao foi possivel criar socket");
        return 2;
    }

    set_nonblocking(s);
    
    zt_socket *zs = (zt_socket *)lua_newuserdata(L, sizeof(zt_socket));
    zs->sock = s;
    return 1;
}

static int l_net_connect(lua_State *L) {
    zt_socket *zs = (zt_socket *)lua_touserdata(L, 1);
    const char *host = luaL_checkstring(L, 2);
    int port = luaL_checkinteger(L, 3);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &addr.sin_addr);

    int res = connect(zs->sock, (struct sockaddr *)&addr, sizeof(addr));
    
    if (res == 0) {
        lua_pushboolean(L, 1);
        return 1;
    } else {
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK || err == WSAEINPROGRESS) {
            lua_pushboolean(L, 0);
            lua_pushstring(L, "inprogress");
            return 2;
        }
        lua_pushboolean(L, 0);
        lua_pushstring(L, "erro de conexao");
        return 2;
    }
}

static int l_net_send(lua_State *L) {
    zt_socket *zs = (zt_socket *)lua_touserdata(L, 1);
    const char *data = luaL_checkstring(L, 2);
    int len = (int)lua_objlen(L, 2);

    int sent = send(zs->sock, data, len, 0);
    if (sent == SOCKET_ERROR) {
        lua_pushnil(L);
        lua_pushstring(L, "erro ao enviar");
        return 2;
    }
    lua_pushinteger(L, sent);
    return 1;
}

static int l_net_recv(lua_State *L) {
    zt_socket *zs = (zt_socket *)lua_touserdata(L, 1);
    int buffer_size = luaL_checkinteger(L, 2);
    char *buffer = malloc(buffer_size);

    int received = recv(zs->sock, buffer, buffer_size, 0);
    if (received > 0) {
        lua_pushlstring(L, buffer, received);
        free(buffer);
        return 1;
    } else {
        free(buffer);
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK) {
            lua_pushnil(L);
            lua_pushstring(L, "again");
            return 2;
        }
        lua_pushnil(L);
        lua_pushstring(L, "closed");
        return 2;
    }
}

static int l_net_close(lua_State *L) {
    zt_socket *zs = (zt_socket *)lua_touserdata(L, 1);
    closesocket(zs->sock);
    WSACleanup();
    return 0;
}

// Registro do Módulo
static const struct luaL_Reg net_lib[] = {
    {"create",  l_net_create},
    {"connect", l_net_connect},
    {"send",    l_net_send},
    {"recv",    l_net_recv},
    {"close",   l_net_close},
    {NULL, NULL}
};

int __declspec(dllexport) luaopen_zenith_net(lua_State *L) {
    luaL_register(L, "zenith_net", net_lib);
    return 1;
}
