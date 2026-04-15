-- ============================================================================
-- Zenith Compiler — Prelude (Globais Embutidos)
-- Injeta as funções/constantes globais de Lua e do runtime Zenith no escopo
-- semântico. Permite que `print`, `tostring`, `type`, etc. sejam usados em
-- qualquer arquivo .zt sem import explícito.
-- ============================================================================

local Symbol       = require("src.semantic.symbols.symbol")
local BuiltinTypes = require("src.semantic.types.builtin_types")
local ZenithType   = require("src.semantic.types.zenith_type")

local Prelude = {}

-- Tipo função genérica (qualquer → any)
local function fn(name, params_types, ret_type)
    local params = {}
    for i, t in ipairs(params_types or {}) do
        table.insert(params, Symbol.variable("p" .. i, t, true, nil))
    end
    local sym = Symbol.func(name, params, ret_type or BuiltinTypes.ANY, nil)
    sym.is_builtin = true
    return sym
end

-- Tipo variável global (any)
local function global(name, type_info)
    local sym = Symbol.variable(name, type_info or BuiltinTypes.ANY, true, nil)
    sym.is_builtin = true
    return sym
end

-- ── Funções base de Lua disponíveis em Zenith ──────────────────────────────

Prelude.symbols = {
    -- I/O
    fn("print",      {BuiltinTypes.ANY}, BuiltinTypes.VOID),
    fn("io_write",   {BuiltinTypes.TEXT}, BuiltinTypes.VOID),

    -- Conversão
    fn("tostring",   {BuiltinTypes.ANY},  BuiltinTypes.TEXT),
    fn("tonumber",   {BuiltinTypes.ANY},  BuiltinTypes.FLOAT),
    fn("tostring",   {BuiltinTypes.ANY},  BuiltinTypes.TEXT),

    -- Tipo dinâmico
    fn("type",       {BuiltinTypes.ANY},  BuiltinTypes.TEXT),

    -- Tabelas Lua
    fn("ipairs",     {BuiltinTypes.ANY},  BuiltinTypes.ANY),
    fn("pairs",      {BuiltinTypes.ANY},  BuiltinTypes.ANY),
    fn("next",       {BuiltinTypes.ANY},  BuiltinTypes.ANY),
    fn("select",     {BuiltinTypes.ANY},  BuiltinTypes.ANY),
    fn("unpack",     {BuiltinTypes.ANY},  BuiltinTypes.ANY),
    fn("setmetatable", {BuiltinTypes.ANY, BuiltinTypes.ANY}, BuiltinTypes.ANY),
    fn("getmetatable", {BuiltinTypes.ANY}, BuiltinTypes.ANY),
    fn("rawget",     {BuiltinTypes.ANY, BuiltinTypes.ANY}, BuiltinTypes.ANY),
    fn("rawset",     {BuiltinTypes.ANY, BuiltinTypes.ANY, BuiltinTypes.ANY}, BuiltinTypes.ANY),

    -- Erros
    fn("error",      {BuiltinTypes.ANY},  BuiltinTypes.VOID),
    fn("assert",     {BuiltinTypes.BOOL, BuiltinTypes.ANY}, BuiltinTypes.ANY),
    fn("pcall",      {BuiltinTypes.ANY},  BuiltinTypes.ANY),
    fn("xpcall",     {BuiltinTypes.ANY, BuiltinTypes.ANY}, BuiltinTypes.ANY),

    -- Módulos
    fn("require",    {BuiltinTypes.TEXT}, BuiltinTypes.ANY),

    -- Strings
    fn("rawlen",     {BuiltinTypes.ANY},  BuiltinTypes.INT),

    -- Matemática
    fn("math_floor", {BuiltinTypes.FLOAT}, BuiltinTypes.INT),
    fn("math_ceil",  {BuiltinTypes.FLOAT}, BuiltinTypes.INT),
    fn("math_abs",   {BuiltinTypes.FLOAT}, BuiltinTypes.FLOAT),

    -- Utilitários
    fn("collectgarbage", {BuiltinTypes.TEXT}, BuiltinTypes.ANY),
    fn("load",       {BuiltinTypes.ANY},  BuiltinTypes.ANY),
    fn("loadstring", {BuiltinTypes.TEXT}, BuiltinTypes.ANY),
    fn("dofile",     {BuiltinTypes.TEXT}, BuiltinTypes.ANY),

    -- Globais constantes
    global("true",  BuiltinTypes.BOOL),
    global("false", BuiltinTypes.BOOL),
    global("nil",   BuiltinTypes.NULL),
    global("arg",   BuiltinTypes.ANY),
    global("_G",    BuiltinTypes.ANY),
    global("_VERSION", BuiltinTypes.TEXT),

    -- Runtime Zenith (zt.*) — alias de conveniência
    fn("zt_add",    {BuiltinTypes.ANY, BuiltinTypes.ANY}, BuiltinTypes.ANY),

    -- ── Outcome<T> — construtores de resultado ─────────────────────────────
    fn("Success",       {BuiltinTypes.ANY}, BuiltinTypes.ANY),
    fn("Failure",       {BuiltinTypes.ANY}, BuiltinTypes.ANY),

    -- ── Optional<T> — construtores de valor opcional ────────────────────────
    fn("Present",       {BuiltinTypes.ANY}, BuiltinTypes.ANY),
    global("Empty",     BuiltinTypes.ANY),

    -- ── Funções de suporte ao operador ? ────────────────────────────────────
    fn("zt_run_fallible", {BuiltinTypes.ANY}, BuiltinTypes.ANY),
}

local function inject_primitive_methods(scope)
    local function _fn(name, type_params, ret)
        local params = {}
        for i, t in ipairs(type_params) do
            table.insert(params, Symbol.variable("p" .. i, t))
        end
        return Symbol.func(name, params, ret)
    end

    -- int
    local int_sym = Symbol.struct("int", {}, {}, nil)
    int_sym.methods = { _fn("to_text", {}, BuiltinTypes.TEXT) }
    BuiltinTypes.INT.symbol = int_sym
    
    -- float
    local float_sym = Symbol.struct("float", {}, {}, nil)
    float_sym.methods = { _fn("to_text", {}, BuiltinTypes.TEXT) }
    BuiltinTypes.FLOAT.symbol = float_sym
    
    -- text
    local text_sym = Symbol.struct("text", {}, {}, nil)
    text_sym.methods = {
        _fn("count",   {}, BuiltinTypes.INT),
        _fn("empty",   {}, BuiltinTypes.BOOL),
        _fn("to_text", {}, BuiltinTypes.TEXT),
    }
    BuiltinTypes.TEXT.symbol = text_sym

    -- any
    local any_sym = Symbol.struct("any", {}, {}, nil)
    any_sym.methods = { _fn("to_text", {}, BuiltinTypes.TEXT) }
    BuiltinTypes.ANY.symbol = any_sym

    -- list<T>
    local list_sym = Symbol.struct("list", {}, {}, nil)
    list_sym.generic_params = { { name = "T" } }
    list_sym.methods = {
        _fn("add",      { BuiltinTypes.ANY }, BuiltinTypes.VOID),
        _fn("push",     { BuiltinTypes.ANY }, BuiltinTypes.VOID),
        _fn("pop",      {},                   BuiltinTypes.ANY),
        _fn("len",      {},                   BuiltinTypes.INT),
        _fn("is_empty", {},                   BuiltinTypes.BOOL),
        _fn("to_text",  {},                   BuiltinTypes.TEXT),
    }
    BuiltinTypes.map["list"].symbol = list_sym

    -- Optional<T>
    local opt_sym = Symbol.struct("Optional", {}, {}, nil)
    opt_sym.generic_params = { { name = "T" } }
    opt_sym.methods = {
        _fn("unwrap",     {},                   BuiltinTypes.ANY),
        _fn("unwrap_or",  { BuiltinTypes.ANY }, BuiltinTypes.ANY),
        _fn("is_present", {},                   BuiltinTypes.BOOL),
        _fn("is_empty",   {},                   BuiltinTypes.BOOL),
    }
    BuiltinTypes.map["Optional"].symbol = opt_sym

    -- Outcome<T>
    local out_sym = Symbol.struct("Outcome", {}, {}, nil)
    out_sym.generic_params = { { name = "T" } }
    out_sym.methods = {
        _fn("is_success", {},                   BuiltinTypes.BOOL),
        _fn("is_failure", {},                   BuiltinTypes.BOOL),
        _fn("unwrap",     {},                   BuiltinTypes.ANY),
        _fn("unwrap_or",  { BuiltinTypes.ANY }, BuiltinTypes.ANY),
        _fn("is_present", {},                   BuiltinTypes.BOOL),
        _fn("is_empty",   {},                   BuiltinTypes.BOOL),
    }
    BuiltinTypes.map["Outcome"].symbol = out_sym

    -- Definir no escopo se fornecido
    if scope then
        scope:define(int_sym)
        scope:define(float_sym)
        scope:define(text_sym)
        scope:define(any_sym)
        scope:define(list_sym)
        scope:define(opt_sym)
        scope:define(out_sym)
    end
end

--- Injeta os símbolos do prelude num escopo fornecido.
--- @param scope Scope — escopo raiz onde os símbolos devem ser registrados
function Prelude.inject(scope)
    inject_primitive_methods(scope)
    
    for _, sym in ipairs(Prelude.symbols) do
        -- Não falha se já definido (globais podem ser redefinidos via import)
        scope:define(sym)
    end
end

return Prelude
