-- ============================================================================
-- Zenith Compiler — Builtin Types
-- Tipos primitivos da linguagem (int, float, bool, text, void, any, null).
-- ============================================================================

local ZenithType = require("src.semantic.types.zenith_type")

local BuiltinTypes = {}

-- Primitivos
BuiltinTypes.INT   = ZenithType.new(ZenithType.Kind.PRIMITIVE, "int")
BuiltinTypes.FLOAT = ZenithType.new(ZenithType.Kind.PRIMITIVE, "float")
BuiltinTypes.BOOL  = ZenithType.new(ZenithType.Kind.PRIMITIVE, "bool")
BuiltinTypes.TEXT  = ZenithType.new(ZenithType.Kind.PRIMITIVE, "text")
BuiltinTypes.VOID  = ZenithType.new(ZenithType.Kind.PRIMITIVE, "void")
BuiltinTypes.ANY   = ZenithType.new(ZenithType.Kind.PRIMITIVE, "any")
BuiltinTypes.NULL  = ZenithType.new(ZenithType.Kind.NULL,      "null")
BuiltinTypes.ERROR = ZenithType.new(ZenithType.Kind.ERROR,     "<error>")

--- Mapa para lookup rápido no parser/binder por nome.
BuiltinTypes.map = {
    ["int"]   = BuiltinTypes.INT,
    ["float"] = BuiltinTypes.FLOAT,
    ["bool"]  = BuiltinTypes.BOOL,
    ["text"]  = BuiltinTypes.TEXT,
    ["void"]  = BuiltinTypes.VOID,
    ["any"]   = BuiltinTypes.ANY,
    ["null"]  = BuiltinTypes.NULL,

    -- Bases Genéricas (para servir de lookup inicial)
    ["list"]  = ZenithType.new(ZenithType.Kind.GENERIC, "list", { base_name = "list", type_args = {} }),
    ["grid"]  = ZenithType.new(ZenithType.Kind.GENERIC, "grid", { base_name = "grid", type_args = {} }),
    ["map"]   = ZenithType.new(ZenithType.Kind.GENERIC, "map",  { base_name = "map",  type_args = {} }),
}

--- Retorna o tipo pelo nome ou nil.
function BuiltinTypes.lookup(name)
    return BuiltinTypes.map[name]
end

return BuiltinTypes
