import runtimeSource from '../assets/lua/zenith_rt.lua?raw';
import syntaxSource from '../assets/lua/syntax.lua?raw';

let fengariPromise = null;

const loadFengari = async () => {
  if (typeof globalThis.process === 'undefined') {
    globalThis.process = { env: {} };
  }

  if (!fengariPromise) {
    fengariPromise = import('fengari');
  }

  return fengariPromise;
};

const readLuaString = (luaState, index, luaApi, toJsString) => {
  const value = luaApi.lua_tostring(luaState, index);
  return value ? toJsString(value) : '';
};

const getNow = () => {
  if (typeof globalThis.performance?.now === 'function') {
    return globalThis.performance.now();
  }

  return Date.now();
};

const formatDuration = (startedAt) => Number((getNow() - startedAt).toFixed(1));

const toLuaLongString = (value) => {
  let level = 0;

  while (value.includes(`]${'='.repeat(level)}]`)) {
    level += 1;
  }

  const delimiters = '='.repeat(level);
  return `[${delimiters}[${value}]${delimiters}]`;
};

export const runZenithPlayground = async (source) => {
  const startedAt = getNow();

  try {
    const { lua, lauxlib, lualib, to_jsstring, to_luastring } = await loadFengari();
    const luaState = lauxlib.luaL_newstate();
    lualib.luaL_openlibs(luaState);

    const bootstrap = `
package.preload["src.backend.lua.runtime.zenith_rt"] = function()
${runtimeSource}
end

package.preload["zenith.syntax"] = function()
${syntaxSource}
end

local compiler = require("zenith.syntax")
local out = {}
local original_print = print

print = function(...)
    local parts = {}
    for i = 1, select('#', ...) do
        parts[#parts + 1] = tostring(select(i, ...))
    end
    out[#out + 1] = table.concat(parts, "\\t")
end

local ok, transpiled_or_err = pcall(compiler.compile, ${toLuaLongString(source)})
if not ok then
    print = original_print
    return false, "", table.concat(out, "\\n"), tostring(transpiled_or_err)
end

local transpiled = transpiled_or_err
local chunk, load_err = load(transpiled)
if not chunk then
    print = original_print
    return false, transpiled, table.concat(out, "\\n"), tostring(load_err)
end

local chunk_ok, exports_or_err = pcall(chunk)
if not chunk_ok then
    print = original_print
    return false, transpiled, table.concat(out, "\\n"), tostring(exports_or_err)
end

local exports = exports_or_err
local entry = nil

if type(exports) == "table" then
    entry = exports.main or exports.run
end

if entry == nil then
    entry = _G.main or _G.run
end

if type(entry) == "function" then
    local exec_ok, exec_result = pcall(entry)
    if exec_ok and exec_result ~= nil then
        out[#out + 1] = "return: " .. tostring(exec_result)
    end

    print = original_print
    return exec_ok, transpiled, table.concat(out, "\\n"), exec_ok and "" or tostring(exec_result)
end

print = original_print
return true, transpiled, table.concat(out, "\\n"), ""
`;

    const status = lauxlib.luaL_dostring(luaState, to_luastring(bootstrap));

    if (status !== lua.LUA_OK) {
      return {
        ok: false,
        transpiled: '',
        output: '',
        error: readLuaString(luaState, -1, lua, to_jsstring) || 'Falha ao iniciar o runtime Lua.',
        durationMs: formatDuration(startedAt),
      };
    }

    return {
      ok: Boolean(lua.lua_toboolean(luaState, -4)),
      transpiled: readLuaString(luaState, -3, lua, to_jsstring),
      output: readLuaString(luaState, -2, lua, to_jsstring),
      error: readLuaString(luaState, -1, lua, to_jsstring),
      durationMs: formatDuration(startedAt),
    };
  } catch (error) {
    return {
      ok: false,
      transpiled: '',
      output: '',
      error: error instanceof Error ? error.message : String(error),
      durationMs: formatDuration(startedAt),
    };
  }
};
