-- ============================================================================
-- Zenith Compiler — Declaration Syntax
-- Factory functions para nós de declaração da AST.
-- ============================================================================

local SyntaxNode = require("src.syntax.ast.syntax_node")
local SK = require("src.syntax.ast.syntax_kind")

local DeclSyntax = {}

--- var name: Type = value
function DeclSyntax.var_decl(name, type_node, initializer, is_pub, span)
    return SyntaxNode.new(SK.VAR_DECL, {
        name = name,
        type_node = type_node,
        initializer = initializer,
        is_pub = is_pub or false,
    }, span)
end

--- const NAME: Type = value
function DeclSyntax.const_decl(name, type_node, initializer, is_pub, span)
    return SyntaxNode.new(SK.CONST_DECL, {
        name = name,
        type_node = type_node,
        initializer = initializer,
        is_pub = is_pub or false,
    }, span)
end

--- global NAME: Type = value
function DeclSyntax.global_decl(name, type_node, initializer, is_pub, span)
    return SyntaxNode.new(SK.GLOBAL_DECL, {
        name = name,
        type_node = type_node,
        initializer = initializer,
        is_pub = is_pub or false,
    }, span)
end

--- state name: Type = value
function DeclSyntax.state_decl(name, type_node, initializer, is_pub, span)
    return SyntaxNode.new(SK.STATE_DECL, {
        name = name,
        type_node = type_node,
        initializer = initializer,
        is_pub = is_pub or false,
    }, span)
end

--- computed name: Type = expression
function DeclSyntax.computed_decl(name, type_node, expression, is_pub, span)
    return SyntaxNode.new(SK.COMPUTED_DECL, {
        name = name,
        type_node = type_node,
        expression = expression,
        is_pub = is_pub or false,
    }, span)
end

--- func name(params) -> ReturnType ... end
function DeclSyntax.func_decl(name, params, return_type, body, is_pub, span, generic_params)
    return SyntaxNode.new(SK.FUNC_DECL, {
        name = name,
        params = params,          -- lista de PARAM_NODE
        return_type = return_type, -- type_node ou nil (void)
        body = body,              -- lista de statements
        is_pub = is_pub or false,
        generic_params = generic_params,
    }, span)
end

--- async func name(params) -> ReturnType ... end
function DeclSyntax.async_func_decl(name, params, return_type, body, is_pub, span, generic_params)
    return SyntaxNode.new(SK.ASYNC_FUNC_DECL, {
        name = name,
        params = params,
        return_type = return_type,
        body = body,
        is_pub = is_pub or false,
        is_async = true,
        generic_params = generic_params,
    }, span)
end

--- Declaração de Type Alias: type Name = Type
function DeclSyntax.type_alias_decl(name, target_type, is_pub, span, generic_params)
    return SyntaxNode.new(SK.TYPE_ALIAS_DECL, {
        name = name,
        target = target_type,
        is_pub = is_pub or false,
        generic_params = generic_params or {},
    }, span)
end

--- Declaração de Union: union Name = A | B
function DeclSyntax.union_decl(name, union_type, is_pub, span, generic_params)
    return SyntaxNode.new(SK.UNION_DECL, {
        name = name,
        union_type = union_type,
        is_pub = is_pub or false,
        generic_params = generic_params or {},
    }, span)
end

--- Nó de parâmetro: name: Type = default
function DeclSyntax.param_node(name, type_node, default_value, span)
    return SyntaxNode.new(SK.PARAM_NODE, {
        name = name,
        type_node = type_node,
        default_value = default_value, -- expressão ou nil
    }, span)
end

--- struct Name ... end
function DeclSyntax.struct_decl(name, fields, methods, is_pub, span, generic_params)
    return SyntaxNode.new(SK.STRUCT_DECL, {
        name = name,
        fields = fields,    -- lista de FIELD_NODE
        methods = methods,  -- lista de FUNC_DECL
        is_pub = is_pub or false,
        generic_params = generic_params, -- lista de {name, constraint}
    }, span)
end

--- Nó de campo: @attr pub? name: Type = default where condition
function DeclSyntax.field_node(name, type_node, default_value, is_pub, attributes, condition, span)
    return SyntaxNode.new(SK.FIELD_NODE, {
        name = name,
        type_node = type_node,
        default_value = default_value,
        is_pub = is_pub or false,
        attributes = attributes or {}, -- lista de ATTRIBUTE_NODE
        condition = condition,         -- expressão do 'where'
    }, span)
end

--- Nó de atributo: @name(args)
function DeclSyntax.attribute_node(name, arguments, span)
    return SyntaxNode.new(SK.ATTRIBUTE_NODE, {
        name = name,
        arguments = arguments or {}, -- lista de expressões
    }, span)
end

--- enum Name ... end
function DeclSyntax.enum_decl(name, members, is_pub, span, generic_params)
    return SyntaxNode.new(SK.ENUM_DECL, {
        name = name,
        members = members,  -- lista de ENUM_MEMBER_NODE
        is_pub = is_pub or false,
        generic_params = generic_params or {},
    }, span)
end

--- Membro de enum
function DeclSyntax.enum_member(name, value, params, span)
    return SyntaxNode.new(SK.ENUM_MEMBER_NODE, {
        name = name,
        value = value,  -- expressão ou nil
        params = params, -- Parâmetros para Sum Types
    }, span)
end

--- trait Name ... end
function DeclSyntax.trait_decl(name, methods, is_pub, span)
    return SyntaxNode.new(SK.TRAIT_DECL, {
        name = name,
        methods = methods,
        is_pub = is_pub or false,
    }, span)
end

--- apply Trait to Struct ... end
function DeclSyntax.apply_decl(trait_name, struct_name, methods, span)
    return SyntaxNode.new(SK.APPLY_DECL, {
        trait_name = trait_name,
        struct_name = struct_name,
        methods = methods, -- lista de FUNC_DECL
    }, span)
end

--- import "path" / import std.module
function DeclSyntax.import_decl(path, alias, span)
    return SyntaxNode.new(SK.IMPORT_DECL, {
        path = path,
        alias = alias,  -- string ou nil
    }, span)
end

--- export func ...
function DeclSyntax.export_decl(declaration, span)
    return SyntaxNode.new(SK.EXPORT_DECL, {
        declaration = declaration,
    }, span)
end

--- redo func Type.method(params) ... end
function DeclSyntax.redo_decl(type_name, func_decl, span)
    return SyntaxNode.new(SK.REDO_DECL, {
        type_name = type_name,
        func_decl = func_decl,
    }, span)
end

--- group "name" ... end (testes)
function DeclSyntax.group_decl(name, body, span)
    return SyntaxNode.new(SK.GROUP_DECL, {
        name = name,
        body = body,
    }, span)
end

--- test "name" ... end (testes)
function DeclSyntax.test_decl(name, body, span)
    return SyntaxNode.new(SK.TEST_DECL, {
        name = name,
        body = body,
    }, span)
end

--- Cria um nó de declaração de namespace.
--- @param name string Nome do namespace (pode ser pontuado)
--- @param span table Span
function DeclSyntax.namespace_decl(name, span)
    return SyntaxNode.new(SK.NAMESPACE_DECL, {
        name = name,
    }, span)
end

--- Cria a unidade de compilação (arquivo).
--- @param declarations table Lista de declarações
--- @param span table Span
function DeclSyntax.compilation_unit(declarations, span)
    return SyntaxNode.new(SK.COMPILATION_UNIT, {
        declarations = declarations,
    }, span)
end

return DeclSyntax
