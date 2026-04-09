-- ============================================================================
-- Zenith Compiler — Syntax Node
-- Tipo-base de todos os nós da AST.
-- Cada nó tem: kind, span e children opcionais.
-- ============================================================================

local SyntaxNode = {}
SyntaxNode.__index = SyntaxNode

--- Cria um novo nó da AST.
--- @param kind string SyntaxKind
--- @param data table Dados específicos do nó
--- @param span table Span no código-fonte
--- @return table
function SyntaxNode.new(kind, data, span)
    local self = setmetatable({}, SyntaxNode)
    self.kind = kind
    self.span = span

    -- Copiar dados do nó
    if data then
        for k, v in pairs(data) do
            self[k] = v
        end
    end

    return self
end

--- Representação textual para debug (sem recursão).
--- @return string
function SyntaxNode:__tostring()
    return string.format("Node(%s)", self.kind)
end

return SyntaxNode
