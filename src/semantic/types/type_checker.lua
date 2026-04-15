-- ============================================================================
-- Zenith Compiler — Type Checker
-- Lógica central de validação de tipos e operações.
-- ============================================================================

local BuiltinTypes = require("src.semantic.types.builtin_types")
local ZenithType    = require("src.semantic.types.zenith_type")

local TypeChecker = {}

--- Verifica se um tipo é atribuível a outro.
--- @param source table ZenithType (o valor fornecido)
--- @param target table ZenithType (o tipo esperado)
--- @return boolean
function TypeChecker.is_assignable(source, target)
    -- Caso base: os mesmos tipos ou sentinelas de erro
    if source == target or source.kind == ZenithType.Kind.ERROR or target.kind == ZenithType.Kind.ERROR then
        return true
    end
    
    -- "any" no alvo ou na origem aceita tudo (pragmatismo para interoperabilidade e blocos native)
    if target.name == "any" or target == BuiltinTypes.ANY or source.name == "any" or source == BuiltinTypes.ANY then
        return true
    end
    
    -- Delega para a lógica estruturada do ZenithType
    return source:is_assignable_to(target)
end

local function is_text(t)
    return t and t.kind == ZenithType.Kind.PRIMITIVE and t.name == "text"
end

local function is_int(t)
    return t and t.kind == ZenithType.Kind.PRIMITIVE and (t.name == "int" or t.name == "float")
end

function TypeChecker.is_text(t)
    return t and t.kind == ZenithType.Kind.PRIMITIVE and t.name == "text"
end

function TypeChecker.is_int(t)
    return t and t.kind == ZenithType.Kind.PRIMITIVE and (t.name == "int" or t.name == "float")
end

--- Retorna o tipo resultante de uma operação binária.
--- @param left table ZenithType
--- @param operator string Lexema do operador (+, -, etc)
--- @param right table ZenithType
--- @return table ZenithType
function TypeChecker.get_binary_result(left, operator, right)
    -- Heurística ANY
    if left == BuiltinTypes.ANY or right == BuiltinTypes.ANY then
        return BuiltinTypes.ANY
    end

    -- Multi-uso do '+' : Matemática e Concatenação
    if operator == "+" then
        -- Concatenação de texto: qualquer coisa que contenha text
        if is_text(left) or is_text(right) then
            return BuiltinTypes.TEXT
        end
        -- Matemática
        if left == BuiltinTypes.INT and right == BuiltinTypes.INT then
            return BuiltinTypes.INT
        end
        if is_int(left) and is_int(right) then
            return BuiltinTypes.FLOAT
        end
    end
    
    -- Comparação e Lógica: Sempre retorna bool
    local bool_ops = { 
        ["=="]=1, ["!="]=1, ["<"]=1, ["<="]=1, [">"]=1, [">="]=1,
        ["and"]=1, ["or"]=1
    }
    if bool_ops[operator] then
        return BuiltinTypes.BOOL
    end
    
    -- Aritmética básica
    local arithmetic = { ["-"]=1, ["*"]=1, ["/"]=1, ["%"]=1, ["^"]=1 }
    if arithmetic[operator] then
        if left == BuiltinTypes.FLOAT or right == BuiltinTypes.FLOAT then
            return BuiltinTypes.FLOAT
        end
        return BuiltinTypes.INT
    end

    -- Concatenação ou Range
    if operator == ".." then
        if is_text(left) or is_text(right) then
            return BuiltinTypes.TEXT
        end
        -- Pode ser um range, o Binder decidirá
        return BuiltinTypes.ANY
    end

    return BuiltinTypes.ERROR
end

--- Retorna o tipo resultante de uma operação unária.
function TypeChecker.get_unary_result(operator, operand)
    if operand == BuiltinTypes.ANY then return BuiltinTypes.ANY end
    
    if operator == "not" then
        return BuiltinTypes.BOOL
    end
    if operator == "-" then
        if operand == BuiltinTypes.INT or operand == BuiltinTypes.FLOAT then
            return operand
        end
    end
    if operator == "!" then
        -- O operador bang (!) remove a nulabilidade ou indica erro provável (v1.0-alpha)
        if operand.kind == ZenithType.Kind.NULLABLE then
            return operand.base_type
        end
        return operand
    end
    return BuiltinTypes.ERROR
end

return TypeChecker
