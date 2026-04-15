-- Transpilado por Zenith v0.2.5
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure

local TokenKind, NodeKind, Symbol, CompilationUnit, DeclNode, StmtNode, ExprNode, Token, Lexer, Parser, scope_lookup, Scope, ModuleManager, Binder, Emitter, compile

local TokenKind = {
    Identifier = "Identifier",
    IntegerLiteral = "IntegerLiteral",
    StringLiteral = "StringLiteral",
    Plus = "Plus",
    Minus = "Minus",
    Star = "Star",
    Slash = "Slash",
    Percent = "Percent",
    Caret = "Caret",
    PlusEqual = "PlusEqual",
    MinusEqual = "MinusEqual",
    StarEqual = "StarEqual",
    SlashEqual = "SlashEqual",
    Equal = "Equal",
    EqualEqual = "EqualEqual",
    BangEqual = "BangEqual",
    Less = "Less",
    LessEqual = "LessEqual",
    Greater = "Greater",
    GreaterEqual = "GreaterEqual",
    And = "And",
    Or = "Or",
    Bang = "Bang",
    LParen = "LParen",
    RParen = "RParen",
    LBrace = "LBrace",
    RBrace = "RBrace",
    LBracket = "LBracket",
    RBracket = "RBracket",
    Comma = "Comma",
    Dot = "Dot",
    Colon = "Colon",
    Semicolon = "Semicolon",
    Arrow = "Arrow",
    FatArrow = "FatArrow",
    DotDot = "DotDot",
    Pipe = "Pipe",
    At = "At",
    KwFunc = "KwFunc",
    KwVar = "KwVar",
    KwIf = "KwIf",
    KwElse = "KwElse",
    KwElif = "KwElif",
    KwWhile = "KwWhile",
    KwReturn = "KwReturn",
    KwPub = "KwPub",
    KwStruct = "KwStruct",
    KwEnum = "KwEnum",
    KwSelf = "KwSelf",
    KwAnd = "KwAnd",
    KwOr = "KwOr",
    KwNot = "KwNot",
    KwDo = "KwDo",
    KwEnd = "KwEnd",
    KwFor = "KwFor",
    KwIn = "KwIn",
    KwRepeat = "KwRepeat",
    KwMatch = "KwMatch",
    KwCase = "KwCase",
    KwBreak = "KwBreak",
    KwContinue = "KwContinue",
    KwNamespace = "KwNamespace",
    KwImport = "KwImport",
    KwExport = "KwExport",
    KwAs = "KwAs",
    EOF = "EOF",
    BAD = "BAD",
}

local NodeKind = {
    CompilationUnit = "CompilationUnit",
    FuncDecl = "FuncDecl",
    VarDecl = "VarDecl",
    StructDecl = "StructDecl",
    EnumDecl = "EnumDecl",
    Block = "Block",
    IfStmt = "IfStmt",
    WhileStmt = "WhileStmt",
    ReturnStmt = "ReturnStmt",
    AssignStmt = "AssignStmt",
    ExprStmt = "ExprStmt",
    MatchStmt = "MatchStmt",
    MatchCase = "MatchCase",
    ForInStmt = "ForInStmt",
    RepeatStmt = "RepeatStmt",
    BinaryExpr = "BinaryExpr",
    UnaryExpr = "UnaryExpr",
    IdentifierExpr = "IdentifierExpr",
    LiteralExpr = "LiteralExpr",
    CallExpr = "CallExpr",
    MemberExpr = "MemberExpr",
    IndexExpr = "IndexExpr",
    ListLiteral = "ListLiteral",
    MapLiteral = "MapLiteral",
    StructInitExpr = "StructInitExpr",
    SelfExpr = "SelfExpr",
    ImportDecl = "ImportDecl",
    NamespaceDecl = "NamespaceDecl",
}

local Symbol = {}
Symbol.__index = Symbol
Symbol._metadata = {
    name = "Symbol",
    fields = {
        { name = "kind", type = "any" },
        { name = "name", type = "any" },
        { name = "type_name", type = "any" },
        { name = "is_pub", type = "any" },
        { name = "declaration", type = "any" },
        { name = "module_scope", type = "any" },
    },
    methods = {
    }
}

function Symbol.new(fields)
    local self = setmetatable({}, Symbol)
    self.kind = fields.kind or nil
    self.name = fields.name or nil
    self.type_name = fields.type_name or nil
    self.is_pub = fields.is_pub or nil
    self.declaration = fields.declaration or nil
    self.module_scope = fields.module_scope or nil
    return self
end

local CompilationUnit = {}
CompilationUnit.__index = CompilationUnit
CompilationUnit._metadata = {
    name = "CompilationUnit",
    fields = {
        { name = "ns_name", type = "any" },
        { name = "imports", type = "any" },
        { name = "declarations", type = "any" },
        { name = "span", type = "any" },
    },
    methods = {
    }
}

function CompilationUnit.new(fields)
    local self = setmetatable({}, CompilationUnit)
    self.ns_name = fields.ns_name or nil
    self.imports = fields.imports or nil
    self.declarations = fields.declarations or nil
    self.span = fields.span or nil
    return self
end

local DeclNode = {}
DeclNode.__index = DeclNode
DeclNode._metadata = {
    name = "DeclNode",
    fields = {
        { name = "kind", type = "any" },
        { name = "name", type = "any" },
        { name = "params", type = "any" },
        { name = "body", type = "any" },
        { name = "fields", type = "any" },
        { name = "methods", type = "any" },
        { name = "members", type = "any" },
        { name = "is_pub", type = "any" },
        { name = "span", type = "any" },
        { name = "symbol", type = "any" },
    },
    methods = {
    }
}

function DeclNode.new(fields)
    local self = setmetatable({}, DeclNode)
    self.kind = fields.kind or nil
    self.name = fields.name or nil
    self.params = fields.params or nil
    self.body = fields.body or nil
    self.fields = fields.fields or nil
    self.methods = fields.methods or nil
    self.members = fields.members or nil
    self.is_pub = fields.is_pub or nil
    self.span = fields.span or nil
    self.symbol = fields.symbol or nil
    return self
end

local StmtNode = {}
StmtNode.__index = StmtNode
StmtNode._metadata = {
    name = "StmtNode",
    fields = {
        { name = "kind", type = "any" },
        { name = "condition", type = "any" },
        { name = "body", type = "any" },
        { name = "value", type = "any" },
        { name = "target", type = "any" },
        { name = "cases", type = "any" },
        { name = "patterns", type = "any" },
        { name = "variables", type = "any" },
        { name = "iterable", type = "any" },
        { name = "count_expr", type = "any" },
        { name = "span", type = "any" },
    },
    methods = {
    }
}

function StmtNode.new(fields)
    local self = setmetatable({}, StmtNode)
    self.kind = fields.kind or nil
    self.condition = fields.condition or nil
    self.body = fields.body or nil
    self.value = fields.value or nil
    self.target = fields.target or nil
    self.cases = fields.cases or nil
    self.patterns = fields.patterns or nil
    self.variables = fields.variables or nil
    self.iterable = fields.iterable or nil
    self.count_expr = fields.count_expr or nil
    self.span = fields.span or nil
    return self
end

local ExprNode = {}
ExprNode.__index = ExprNode
ExprNode._metadata = {
    name = "ExprNode",
    fields = {
        { name = "kind", type = "any" },
        { name = "left", type = "any" },
        { name = "right", type = "any" },
        { name = "operator", type = "any" },
        { name = "value", type = "any" },
        { name = "name", type = "any" },
        { name = "callee", type = "any" },
        { name = "args", type = "any" },
        { name = "object", type = "any" },
        { name = "index", type = "any" },
        { name = "items", type = "any" },
        { name = "keys", type = "any" },
        { name = "values", type = "any" },
        { name = "span", type = "any" },
        { name = "symbol", type = "any" },
    },
    methods = {
    }
}

function ExprNode.new(fields)
    local self = setmetatable({}, ExprNode)
    self.kind = fields.kind or nil
    self.left = fields.left or nil
    self.right = fields.right or nil
    self.operator = fields.operator or nil
    self.value = fields.value or nil
    self.name = fields.name or nil
    self.callee = fields.callee or nil
    self.args = fields.args or nil
    self.object = fields.object or nil
    self.index = fields.index or nil
    self.items = fields.items or nil
    self.keys = fields.keys or nil
    self.values = fields.values or nil
    self.span = fields.span or nil
    self.symbol = fields.symbol or nil
    return self
end

local Token = {}
Token.__index = Token
Token._metadata = {
    name = "Token",
    fields = {
        { name = "kind", type = "any" },
        { name = "text", type = "any" },
        { name = "span", type = "any" },
    },
    methods = {
    }
}

function Token.new(fields)
    local self = setmetatable({}, Token)
    self.kind = fields.kind or nil
    self.text = fields.text or nil
    self.span = fields.span or nil
    return self
end

local Lexer = {}
Lexer.__index = Lexer
Lexer._metadata = {
    name = "Lexer",
    fields = {
        { name = "source", type = "any" },
        { name = "pos", type = "any" },
        { name = "tokens", type = "any" },
    },
    methods = {
        "tokenize",
    }
}

function Lexer.new(fields)
    local self = setmetatable({}, Lexer)
    self.source = fields.source or nil
    self.pos = fields.pos or nil
    self.tokens = fields.tokens or nil
    return self
end

local Parser = {}
Parser.__index = Parser
Parser._metadata = {
    name = "Parser",
    fields = {
        { name = "tokens", type = "any" },
        { name = "pos", type = "any" },
    },
    methods = {
        "parse",
    }
}

function Parser.new(fields)
    local self = setmetatable({}, Parser)
    self.tokens = fields.tokens or nil
    self.pos = fields.pos or nil
    return self
end

function scope_lookup(sc, name)
    local cur = sc
    while (cur ~= nil) do
        local sym = cur.symbols[name]
        if (sym ~= nil) then
            return sym
        end
        cur = cur.parent
    end
    return nil
end

local Scope = {}
Scope.__index = Scope
Scope._metadata = {
    name = "Scope",
    fields = {
        { name = "parent", type = "any" },
        { name = "symbols", type = "any" },
    },
    methods = {
        "define",
    }
}

function Scope.new(fields)
    local self = setmetatable({}, Scope)
    self.parent = fields.parent or nil
    self.symbols = fields.symbols or nil
    return self
end

local ModuleManager = {}
ModuleManager.__index = ModuleManager
ModuleManager._metadata = {
    name = "ModuleManager",
    fields = {
        { name = "project_root", type = "any" },
        { name = "cache", type = "any" },
    },
    methods = {
        "resolve_path",
        "get_ast",
    }
}

function ModuleManager.new(fields)
    local self = setmetatable({}, ModuleManager)
    self.project_root = fields.project_root or nil
    self.cache = fields.cache or nil
    return self
end

local Binder = {}
Binder.__index = Binder
Binder._metadata = {
    name = "Binder",
    fields = {
        { name = "current_scope", type = "any" },
        { name = "module_manager", type = "any" },
        { name = "module_scopes", type = "any" },
    },
    methods = {
        "bind_unit",
    }
}

function Binder.new(fields)
    local self = setmetatable({}, Binder)
    self.current_scope = fields.current_scope or nil
    self.module_manager = fields.module_manager or nil
    self.module_scopes = fields.module_scopes or nil
    return self
end

local Emitter = {}
Emitter.__index = Emitter
Emitter._metadata = {
    name = "Emitter",
    fields = {
        { name = "output", type = "any" },
        { name = "indent_level", type = "any" },
        { name = "line_count", type = "any" },
    },
    methods = {
        "emit",
        "indent",
        "dedent",
        "generate",
    }
}

function Emitter.new(fields)
    local self = setmetatable({}, Emitter)
    self.output = fields.output or nil
    self.indent_level = fields.indent_level or nil
    self.line_count = fields.line_count or nil
    return self
end

function compile(src)
    local l = Lexer.new({["source"] = src, ["pos"] = 0, ["tokens"] = {}})
    local ts = l:tokenize()
    local p = Parser.new({["tokens"] = ts, ["pos"] = 0})
    local u = p:parse()
    local mm = ModuleManager.new({["project_root"] = ".", ["cache"] = {}})
    local b = Binder.new({["current_scope"] = Scope.new({["parent"] = nil, ["symbols"] = {}}), ["module_manager"] = mm, ["module_scopes"] = {}})
    b:bind_unit(u)
    local e = Emitter.new({["output"] = {}, ["indent_level"] = 0, ["line_count"] = 0})
    return e:generate(u)
end

-- Struct Methods
function Lexer:tokenize()
    local count = 0
    while true do
        local t = self:scan_token()
        self.tokens[count] = t
        count = zt.add(count, 1)
        if (t.kind == TokenKind.EOF) then
            break
        end
    end
    return self.tokens
end
function Lexer:scan_token()
    self:skip_whitespace()
    local start = self.pos
    if self:is_at_end() then
        return Token.new({["kind"] = TokenKind.EOF, ["text"] = "", ["span"] = nil})
    end
    local c = self:peek()
    self:advance()
    if self:is_alpha(c) then
        return self:scan_identifier(start)
    end
    if self:is_digit(c) then
        return self:scan_number(start)
    end
    if (c == "\"") then
        return self:scan_string(start)
    end
    local op2 = zt.add(c, self:peek())
    local k2 = self:lookup_operator_double(op2)
    if (k2 ~= nil) then
        self:advance()
        return Token.new({["kind"] = k2, ["text"] = op2, ["span"] = nil})
    end
    local k1 = self:lookup_operator_single(c)
    if (k1 ~= nil) then
        return Token.new({["kind"] = k1, ["text"] = c, ["span"] = nil})
    end
    return Token.new({["kind"] = TokenKind.BAD, ["text"] = c, ["span"] = nil})
end
function Lexer:skip_whitespace()
    while not self:is_at_end() do
        local c = self:peek()
        if zt.unwrap_or(zt.unwrap_or(zt.unwrap_or((c == " "), (c == "\r")), (c == "	")), (c == "\
")) then
            self:advance()
        elseif ((c == "-") and (self:peek_next() == "-")) then
            while (not self:is_at_end() and (self:peek() ~= "\
")) do
                self:advance()
            end
        else
            break
        end
    end
end
function Lexer:is_alpha(c)
    return zt.unwrap_or(zt.unwrap_or((((c >= "a") and (c <= "z"))), (((c >= "A") and (c <= "Z")))), (c == "_"))
end
function Lexer:is_digit(c)
    return ((c >= "0") and (c <= "9"))
end
function Lexer:scan_identifier(start)
    while zt.unwrap_or(self:is_alpha(self:peek()), self:is_digit(self:peek())) do
        self:advance()
    end
    local txt = zt.slice(self.source, start, (self.pos - 1))
    local kind = self:lookup_keyword(txt)
    return Token.new({["kind"] = kind, ["text"] = txt, ["span"] = nil})
end
function Lexer:scan_number(start)
    while self:is_digit(self:peek()) do
        self:advance()
    end
    return Token.new({["kind"] = TokenKind.IntegerLiteral, ["text"] = zt.slice(self.source, start, (self.pos - 1)), ["span"] = nil})
end
function Lexer:scan_string(start)
    while ((self:peek() ~= "\"") and not self:is_at_end()) do
        self:advance()
    end
    self:advance()
    return Token.new({["kind"] = TokenKind.StringLiteral, ["text"] = zt.slice(self.source, start, (self.pos - 1)), ["span"] = nil})
end
function Lexer:lookup_keyword(txt)
    if (txt == "func") then
        return TokenKind.KwFunc
    end
    if (txt == "var") then
        return TokenKind.KwVar
    end
    if (txt == "if") then
        return TokenKind.KwIf
    end
    if (txt == "else") then
        return TokenKind.KwElse
    end
    if (txt == "elif") then
        return TokenKind.KwElif
    end
    if (txt == "while") then
        return TokenKind.KwWhile
    end
    if (txt == "return") then
        return TokenKind.KwReturn
    end
    if (txt == "pub") then
        return TokenKind.KwPub
    end
    if (txt == "struct") then
        return TokenKind.KwStruct
    end
    if (txt == "enum") then
        return TokenKind.KwEnum
    end
    if (txt == "self") then
        return TokenKind.KwSelf
    end
    if (txt == "and") then
        return TokenKind.KwAnd
    end
    if (txt == "or") then
        return TokenKind.KwOr
    end
    if (txt == "not") then
        return TokenKind.KwNot
    end
    if (txt == "do") then
        return TokenKind.KwDo
    end
    if (txt == "end") then
        return TokenKind.KwEnd
    end
    if (txt == "for") then
        return TokenKind.KwFor
    end
    if (txt == "in") then
        return TokenKind.KwIn
    end
    if (txt == "repeat") then
        return TokenKind.KwRepeat
    end
    if (txt == "match") then
        return TokenKind.KwMatch
    end
    if (txt == "case") then
        return TokenKind.KwCase
    end
    if (txt == "break") then
        return TokenKind.KwBreak
    end
    if (txt == "continue") then
        return TokenKind.KwContinue
    end
    if (txt == "namespace") then
        return TokenKind.KwNamespace
    end
    if (txt == "import") then
        return TokenKind.KwImport
    end
    if (txt == "export") then
        return TokenKind.KwExport
    end
    if (txt == "as") then
        return TokenKind.KwAs
    end
    return TokenKind.Identifier
end
function Lexer:lookup_operator_double(op)
    if (op == "==") then
        return TokenKind.EqualEqual
    end
    if (op == "!=") then
        return TokenKind.BangEqual
    end
    if (op == "<=") then
        return TokenKind.LessEqual
    end
    if (op == ">=") then
        return TokenKind.GreaterEqual
    end
    if (op == "+=") then
        return TokenKind.PlusEqual
    end
    if (op == "-=") then
        return TokenKind.MinusEqual
    end
    if (op == "*=") then
        return TokenKind.StarEqual
    end
    if (op == "/=") then
        return TokenKind.SlashEqual
    end
    if (op == "->") then
        return TokenKind.Arrow
    end
    if (op == "=>") then
        return TokenKind.FatArrow
    end
    if (op == "..") then
        return TokenKind.DotDot
    end
    return nil
end
function Lexer:lookup_operator_single(op)
    if (op == "+") then
        return TokenKind.Plus
    end
    if (op == "-") then
        return TokenKind.Minus
    end
    if (op == "*") then
        return TokenKind.Star
    end
    if (op == "/") then
        return TokenKind.Slash
    end
    if (op == "^") then
        return TokenKind.Caret
    end
    if (op == "%") then
        return TokenKind.Percent
    end
    if (op == "=") then
        return TokenKind.Equal
    end
    if (op == "<") then
        return TokenKind.Less
    end
    if (op == ">") then
        return TokenKind.Greater
    end
    if (op == "(") then
        return TokenKind.LParen
    end
    if (op == ")") then
        return TokenKind.RParen
    end
    if (op == ((string . char ( 123 ) ))) then
        return TokenKind.LBrace
    end
    if (op == ((string . char ( 125 ) ))) then
        return TokenKind.RBrace
    end
    if (op == "[") then
        return TokenKind.LBracket
    end
    if (op == "]") then
        return TokenKind.RBracket
    end
    if (op == ",") then
        return TokenKind.Comma
    end
    if (op == ".") then
        return TokenKind.Dot
    end
    if (op == ":") then
        return TokenKind.Colon
    end
    if (op == ";") then
        return TokenKind.Semicolon
    end
    if (op == "|") then
        return TokenKind.Pipe
    end
    if (op == "@") then
        return TokenKind.At
    end
    return nil
end
function Lexer:peek()
    if self:is_at_end() then
        return ""
    end
    return zt.slice(self.source, self.pos, self.pos)
end
function Lexer:peek_next()
    if (zt.add(self.pos, 1) >= #(self.source)) then
        return ""
    end
    return zt.slice(self.source, zt.add(self.pos, 1), zt.add(self.pos, 1))
end
function Lexer:advance()
    self.pos = zt.add(self.pos, 1)
end
function Lexer:is_at_end()
    return (self.pos >= #(self.source))
end
function Parser:parse()
    local ns = ""
    if (self:peek().kind == TokenKind.KwNamespace) then
        self:advance()
        ns = self:expect(TokenKind.Identifier, "esperado namespace").text
    end
    local imps = {}
    local ic = 0
    while (self:peek().kind == TokenKind.KwImport) do
        imps[ic] = self:parse_import_decl()
        ic = zt.add(ic, 1)
    end
    local decls = {}
    local count = 0
    while not self:is_at_end() do
        local d = self:parse_declaration()
        if (d ~= nil) then
            decls[count] = d
            count = zt.add(count, 1)
        else
            self:advance()
        end
    end
    return CompilationUnit.new({["ns_name"] = ns, ["imports"] = imps, ["declarations"] = decls, ["span"] = nil})
end
function Parser:parse_import_decl()
    local t = self:peek()
    self:advance()
    local path = ""
    while true do
        path = zt.add(path, self:expect(TokenKind.Identifier, "esperado nome").text)
        if (self:peek().kind == TokenKind.Dot) then
            self:advance()
            path = zt.add(path, ".")
        else
            break
        end
    end
    local alias = path
    if (self:peek().kind == TokenKind.KwAs) then
        self:advance()
        alias = self:expect(TokenKind.Identifier, "esperado alias").text
    end
    return DeclNode.new({["kind"] = NodeKind.ImportDecl, ["name"] = alias, ["body"] = path, ["span"] = t.span, ["is_pub"] = false, ["params"] = nil, ["fields"] = nil, ["methods"] = nil, ["members"] = nil, ["symbol"] = nil})
end
function Parser:parse_declaration()
    local is_pub = false
    if zt.unwrap_or((self:peek().kind == TokenKind.KwPub), (self:peek().kind == TokenKind.KwExport)) then
        self:advance()
        is_pub = true
    end
    local t = self:peek()
    if (t.kind == TokenKind.KwFunc) then
        return self:parse_func_decl(is_pub)
    elseif (t.kind == TokenKind.KwVar) then
        return self:parse_var_decl(is_pub)
    elseif (t.kind == TokenKind.KwStruct) then
        return self:parse_struct_decl(is_pub)
    elseif (t.kind == TokenKind.KwEnum) then
        return self:parse_enum_decl(is_pub)
    end
    return nil
end
function Parser:parse_func_decl(is_pub)
    self:advance()
    local name_t = self:expect(TokenKind.Identifier, "esperado nome da função")
    local params = self:parse_params()
    if (self:peek().kind == TokenKind.Arrow) then
        self:advance()
        self:expect(TokenKind.Identifier, "esperado tipo")
    end
    local body = self:parse_block()
    return DeclNode.new({["kind"] = NodeKind.FuncDecl, ["name"] = name_t.text, ["params"] = params, ["body"] = body, ["is_pub"] = is_pub, ["span"] = name_t.span, ["symbol"] = nil})
end
function Parser:parse_var_decl(is_pub)
    self:advance()
    local name_t = self:expect(TokenKind.Identifier, "esperado nome")
    local init = nil
    if (self:peek().kind == TokenKind.Equal) then
        self:advance()
        init = self:parse_expression(0)
    end
    return DeclNode.new({["kind"] = NodeKind.VarDecl, ["name"] = name_t.text, ["is_pub"] = is_pub, ["body"] = init, ["span"] = name_t.span, ["params"] = nil, ["symbol"] = nil, ["fields"] = nil, ["methods"] = nil, ["members"] = nil})
end
function Parser:parse_struct_decl(is_pub)
    self:advance()
    local name_t = self:expect(TokenKind.Identifier, "esperado nome da struct")
    local fields = {}
    local methods = {}
    local fc = 0
    local mc = 0
    while (not self:is_at_end() and (self:peek().kind ~= TokenKind.KwEnd)) do
        local is_field_pub = false
        if (self:peek().kind == TokenKind.KwPub) then
            self:advance()
            is_field_pub = true
        end
        if (self:peek().kind == TokenKind.KwFunc) then
            methods[mc] = self:parse_func_decl(is_field_pub)
            mc = zt.add(mc, 1)
        else
            if (self:peek().kind == TokenKind.KwVar) then
                self:advance()
            end
            local f_name = self:expect(TokenKind.Identifier, "esperado nome do campo")
            if (self:peek().kind == TokenKind.Colon) then
                self:advance()
                self:expect(TokenKind.Identifier, "esperado tipo")
            end
            local init = nil
            if (self:peek().kind == TokenKind.Equal) then
                self:advance()
                init = self:parse_expression(0)
            end
            fields[fc] = DeclNode.new({["kind"] = NodeKind.VarDecl, ["name"] = f_name.text, ["is_pub"] = is_field_pub, ["body"] = init, ["span"] = f_name.span, ["params"] = nil, ["symbol"] = nil, ["fields"] = nil, ["methods"] = nil, ["members"] = nil})
            fc = zt.add(fc, 1)
        end
    end
    self:expect(TokenKind.KwEnd, "esperado 'end' na struct")
    return DeclNode.new({["kind"] = NodeKind.StructDecl, ["name"] = name_t.text, ["is_pub"] = is_pub, ["fields"] = fields, ["methods"] = methods, ["span"] = name_t.span, ["params"] = nil, ["body"] = nil, ["members"] = nil, ["symbol"] = nil})
end
function Parser:parse_enum_decl(is_pub)
    self:advance()
    local name_t = self:expect(TokenKind.Identifier, "esperado nome do enum")
    local members = {}
    local mc = 0
    while (not self:is_at_end() and (self:peek().kind ~= TokenKind.KwEnd)) do
        local m_name = self:expect(TokenKind.Identifier, "esperado membro")
        members[mc] = m_name.text
        mc = zt.add(mc, 1)
    end
    self:expect(TokenKind.KwEnd, "esperado 'end' no enum")
    return DeclNode.new({["kind"] = NodeKind.EnumDecl, ["name"] = name_t.text, ["is_pub"] = is_pub, ["members"] = members, ["span"] = name_t.span, ["params"] = nil, ["body"] = nil, ["fields"] = nil, ["methods"] = nil, ["symbol"] = nil})
end
function Parser:parse_block()
    local stmts = {}
    local count = 0
    while (((not self:is_at_end() and (self:peek().kind ~= TokenKind.KwEnd)) and (self:peek().kind ~= TokenKind.KwElse)) and (self:peek().kind ~= TokenKind.KwElif)) do
        local s = self:parse_statement()
        if (s ~= nil) then
            stmts[count] = s
            count = zt.add(count, 1)
        end
    end
    self:expect(TokenKind.KwEnd, "esperado 'end'")
    return stmts
end
function Parser:parse_statement()
    local t = self:peek()
    if (t.kind == TokenKind.KwVar) then
        return self:parse_var_decl(false)
    end
    if (t.kind == TokenKind.KwIf) then
        return self:parse_if_stmt()
    end
    if (t.kind == TokenKind.KwWhile) then
        return self:parse_while_stmt()
    end
    if (t.kind == TokenKind.KwFor) then
        return self:parse_for_stmt()
    end
    if (t.kind == TokenKind.KwRepeat) then
        return self:parse_repeat_stmt()
    end
    if (t.kind == TokenKind.KwMatch) then
        return self:parse_match_stmt()
    end
    if (t.kind == TokenKind.KwReturn) then
        self:advance()
        return StmtNode.new({["kind"] = NodeKind.ReturnStmt, ["value"] = self:parse_expression(0), ["span"] = t.span, ["condition"] = nil, ["body"] = nil, ["target"] = nil, ["cases"] = nil, ["patterns"] = nil, ["variables"] = nil, ["iterable"] = nil, ["count_expr"] = nil})
    end
    if self:is_assign_stmt() then
        return self:parse_assign_stmt()
    end
    return StmtNode.new({["kind"] = NodeKind.ExprStmt, ["value"] = self:parse_expression(0), ["span"] = t.span, ["condition"] = nil, ["body"] = nil, ["target"] = nil, ["cases"] = nil, ["patterns"] = nil, ["variables"] = nil, ["iterable"] = nil, ["count_expr"] = nil})
end
function Parser:is_assign_stmt()
    local t1 = self:peek().kind
    if (t1 == TokenKind.At) then
        return (self.tokens[zt.add(self.pos, 2)].kind == TokenKind.Equal)
    end
    if (t1 == TokenKind.Identifier) then
        return (self.tokens[zt.add(self.pos, 1)].kind == TokenKind.Equal)
    end
    return false
end
function Parser:parse_assign_stmt()
    local target_t = self:advance()
    if (target_t.kind == TokenKind.At) then
        local f = self:expect(TokenKind.Identifier, "esperado campo")
        self:expect(TokenKind.Equal, "esperado '='")
        local self_node = ExprNode.new({["kind"] = NodeKind.SelfExpr, ["span"] = target_t.span, ["left"] = nil, ["right"] = nil, ["operator"] = nil, ["value"] = nil, ["name"] = "", ["callee"] = nil, ["args"] = nil, ["object"] = nil, ["index"] = nil, ["items"] = nil, ["keys"] = nil, ["values"] = nil, ["symbol"] = nil})
        local target = ExprNode.new({["kind"] = NodeKind.MemberExpr, ["object"] = self_node, ["name"] = f.text, ["span"] = target_t.span, ["left"] = nil, ["right"] = nil, ["operator"] = nil, ["value"] = nil, ["callee"] = nil, ["args"] = nil, ["index"] = nil, ["items"] = nil, ["keys"] = nil, ["values"] = nil, ["symbol"] = nil})
        return StmtNode.new({["kind"] = NodeKind.AssignStmt, ["target"] = target, ["value"] = self:parse_expression(0), ["span"] = target_t.span, ["condition"] = nil, ["body"] = nil})
    end
    self:expect(TokenKind.Equal, "esperado '='")
    local target = ExprNode.new({["kind"] = NodeKind.IdentifierExpr, ["name"] = target_t.text, ["span"] = target_t.span, ["left"] = nil, ["right"] = nil, ["operator"] = nil, ["value"] = nil, ["callee"] = nil, ["args"] = nil, ["object"] = nil, ["index"] = nil, ["symbol"] = nil})
    return StmtNode.new({["kind"] = NodeKind.AssignStmt, ["target"] = target, ["value"] = self:parse_expression(0), ["span"] = target_t.span, ["condition"] = nil, ["body"] = nil})
end
function Parser:parse_if_stmt()
    local t = self:advance()
    local cond = self:parse_expression(0)
    local body = self:parse_block()
    return StmtNode.new({["kind"] = NodeKind.IfStmt, ["condition"] = cond, ["body"] = body, ["span"] = t.span, ["value"] = nil, ["target"] = nil})
end
function Parser:parse_while_stmt()
    local t = self:advance()
    local cond = self:parse_expression(0)
    local body = self:parse_block()
    return StmtNode.new({["kind"] = NodeKind.WhileStmt, ["condition"] = cond, ["body"] = body, ["span"] = t.span, ["value"] = nil, ["target"] = nil, ["cases"] = nil, ["patterns"] = nil, ["variables"] = nil, ["iterable"] = nil, ["count_expr"] = nil})
end
function Parser:parse_for_stmt()
    local t = self:advance()
    local vn_t = self:expect(TokenKind.Identifier, "esperado variável no for")
    local var_name = vn_t.text
    self:expect(TokenKind.KwIn, "esperado 'in'")
    local iter = self:parse_expression(0)
    local body = self:parse_block()
    local vars = {}
    vars[0] = var_name
    return StmtNode.new({["kind"] = NodeKind.ForInStmt, ["variables"] = vars, ["iterable"] = iter, ["body"] = body, ["span"] = t.span, ["target"] = nil, ["value"] = nil, ["condition"] = nil, ["cases"] = nil, ["patterns"] = nil, ["count_expr"] = nil, ["else_body"] = nil})
end
function Parser:parse_repeat_stmt()
    local t = self:advance()
    local n = self:parse_expression(0)
    self:expect(TokenKind.Identifier, "esperado 'times'")
    local body = self:parse_block()
    return StmtNode.new({["kind"] = NodeKind.RepeatStmt, ["count_expr"] = n, ["body"] = body, ["span"] = t.span, ["target"] = nil, ["value"] = nil, ["condition"] = nil, ["cases"] = nil, ["patterns"] = nil, ["variables"] = nil, ["iterable"] = nil})
end
function Parser:parse_match_stmt()
    local t = self:advance()
    local expr = self:parse_expression(0)
    local cases = {}
    local else_c = nil
    local cc = 0
    while (not self:is_at_end() and (self:peek().kind ~= TokenKind.KwEnd)) do
        if (self:peek().kind == TokenKind.KwElse) then
            self:advance()
            self:expect(TokenKind.FatArrow, "esperado '=>' no else")
            else_c = self:parse_block_simple()
        elseif (self:peek().kind == TokenKind.KwCase) then
            local ct = self:advance()
            local patterns = {}
            local pc = 0
            while true do
                patterns[pc] = self:parse_expression(0)
                pc = zt.add(pc, 1)
                if (self:peek().kind == TokenKind.Comma) then
                    self:advance()
                else
                    break
                end
            end
            self:expect(TokenKind.FatArrow, "esperado '=>'")
            local body = self:parse_block_simple()
            cases[cc] = StmtNode.new({["kind"] = NodeKind.MatchCase, ["patterns"] = patterns, ["body"] = body, ["span"] = ct.span, ["target"] = nil, ["value"] = nil, ["condition"] = nil, ["else_body"] = nil, ["cases"] = nil, ["variables"] = nil, ["iterable"] = nil, ["count_expr"] = nil})
            cc = zt.add(cc, 1)
        else
            break
        end
    end
    self:expect(TokenKind.KwEnd, "esperado 'end' no match")
    return StmtNode.new({["kind"] = NodeKind.MatchStmt, ["condition"] = expr, ["cases"] = cases, ["else_body"] = else_c, ["span"] = t.span, ["target"] = nil, ["value"] = nil, ["body"] = nil, ["patterns"] = nil, ["variables"] = nil, ["iterable"] = nil, ["count_expr"] = nil})
end
function Parser:parse_block_simple()
    local stmts = {}
    local sc = 0
    while (((not self:is_at_end() and (self:peek().kind ~= TokenKind.KwEnd)) and (self:peek().kind ~= TokenKind.KwCase)) and (self:peek().kind ~= TokenKind.KwElse)) do
        local s = self:parse_statement()
        if (s ~= nil) then
            stmts[sc] = s
            sc = zt.add(sc, 1)
        end
    end
    return stmts
end
function Parser:parse_expression(min_prec)
    local left = self:parse_primary()
    while true do
        local op = self:peek()
        local prec = self:get_precedence(op.kind)
        if (prec <= min_prec) then
            break
        end
        self:advance()
        left = ExprNode.new({["kind"] = NodeKind.BinaryExpr, ["left"] = left, ["right"] = self:parse_expression(prec), ["operator"] = op, ["span"] = op.span, ["value"] = nil, ["name"] = "", ["callee"] = nil, ["args"] = nil, ["object"] = nil, ["index"] = nil, ["symbol"] = nil})
    end
    return left
end
function Parser:parse_primary()
    local t = self:advance()
    local node = nil
    if zt.unwrap_or((t.kind == TokenKind.IntegerLiteral), (t.kind == TokenKind.StringLiteral)) then
        node = ExprNode.new({["kind"] = NodeKind.LiteralExpr, ["value"] = t.text, ["span"] = t.span, ["left"] = nil, ["right"] = nil, ["operator"] = nil, ["name"] = "", ["callee"] = nil, ["args"] = nil, ["object"] = nil, ["index"] = nil, ["symbol"] = nil})
    elseif (t.kind == TokenKind.Identifier) then
        if (self:peek().kind == TokenKind.LBrace) then
            self:advance()
            local keys = {}
            local vals = {}
            local sc = 0
            if (self:peek().kind ~= TokenKind.RBrace) then
                while true do
                    keys[sc] = self:expect(TokenKind.Identifier, "esperado campo").text
                    self:expect(TokenKind.Colon, "esperado ':'")
                    vals[sc] = self:parse_expression(0)
                    sc = zt.add(sc, 1)
                    if (self:peek().kind == TokenKind.Comma) then
                        self:advance()
                    else
                        break
                    end
                end
            end
            self:expect(TokenKind.RBrace, "esperado '}'")
            node = ExprNode.new({["kind"] = NodeKind.StructInitExpr, ["name"] = t.text, ["keys"] = keys, ["values"] = vals, ["span"] = t.span, ["left"] = nil, ["right"] = nil, ["operator"] = nil, ["value"] = nil, ["callee"] = nil, ["args"] = nil, ["object"] = nil, ["index"] = nil, ["items"] = nil, ["symbol"] = nil})
        else
            node = ExprNode.new({["kind"] = NodeKind.IdentifierExpr, ["name"] = t.text, ["span"] = t.span, ["left"] = nil, ["right"] = nil, ["operator"] = nil, ["value"] = nil, ["callee"] = nil, ["args"] = nil, ["object"] = nil, ["index"] = nil, ["items"] = nil, ["keys"] = nil, ["values"] = nil, ["symbol"] = nil})
        end
    elseif (t.kind == TokenKind.KwSelf) then
        node = ExprNode.new({["kind"] = NodeKind.SelfExpr, ["span"] = t.span, ["left"] = nil, ["right"] = nil, ["operator"] = nil, ["value"] = nil, ["name"] = "", ["callee"] = nil, ["args"] = nil, ["object"] = nil, ["index"] = nil, ["symbol"] = nil})
    elseif (t.kind == TokenKind.LParen) then
        node = self:parse_expression(0)
        self:expect(TokenKind.RParen, "esperado ')' após expressão")
    elseif (t.kind == TokenKind.At) then
        local f = self:expect(TokenKind.Identifier, "esperado nome do campo após '@'")
        local self_node = ExprNode.new({["kind"] = NodeKind.SelfExpr, ["span"] = t.span, ["left"] = nil, ["right"] = nil, ["operator"] = nil, ["value"] = nil, ["name"] = "", ["callee"] = nil, ["args"] = nil, ["object"] = nil, ["index"] = nil, ["items"] = nil, ["keys"] = nil, ["values"] = nil, ["symbol"] = nil})
        node = ExprNode.new({["kind"] = NodeKind.MemberExpr, ["object"] = self_node, ["name"] = f.text, ["span"] = t.span, ["left"] = nil, ["right"] = nil, ["operator"] = nil, ["value"] = nil, ["callee"] = nil, ["args"] = nil, ["index"] = nil, ["items"] = nil, ["keys"] = nil, ["values"] = nil, ["symbol"] = nil})
    elseif (t.kind == TokenKind.LBracket) then
        node = self:parse_list_literal(t)
    elseif (t.kind == TokenKind.LBrace) then
        node = self:parse_map_literal(t)
    elseif zt.unwrap_or((t.kind == TokenKind.KwNot), (t.kind == TokenKind.Minus)) then
        node = ExprNode.new({["kind"] = NodeKind.UnaryExpr, ["operator"] = t, ["right"] = self:parse_expression(9), ["span"] = t.span, ["left"] = nil, ["value"] = nil, ["name"] = "", ["callee"] = nil, ["args"] = nil, ["object"] = nil, ["index"] = nil, ["symbol"] = nil})
    end
    while true do
        local next = self:peek()
        if (next.kind == TokenKind.LParen) then
            self:advance()
            local args = {}
            local ac = 0
            if (self:peek().kind ~= TokenKind.RParen) then
                while true do
                    args[ac] = self:parse_expression(0)
                    ac = zt.add(ac, 1)
                    if (self:peek().kind == TokenKind.Comma) then
                        self:advance()
                    else
                        break
                    end
                end
            end
            self:expect(TokenKind.RParen, "esperado ')' após expressão")
            node = ExprNode.new({["kind"] = NodeKind.CallExpr, ["callee"] = node, ["args"] = args, ["span"] = t.span, ["left"] = nil, ["right"] = nil, ["operator"] = nil, ["value"] = nil, ["name"] = "", ["object"] = nil, ["index"] = nil, ["symbol"] = nil})
        elseif (next.kind == TokenKind.Dot) then
            self:advance()
            local m = self:expect(TokenKind.Identifier, "esperado membro")
            node = ExprNode.new({["kind"] = NodeKind.MemberExpr, ["object"] = node, ["name"] = m.text, ["span"] = t.span, ["left"] = nil, ["right"] = nil, ["operator"] = nil, ["value"] = nil, ["callee"] = nil, ["args"] = nil, ["index"] = nil, ["symbol"] = nil})
        elseif (next.kind == TokenKind.LBracket) then
            self:advance()
            local idx = self:parse_expression(0)
            self:expect(TokenKind.RBracket, "esperado ']'")
            node = ExprNode.new({["kind"] = NodeKind.IndexExpr, ["object"] = node, ["index"] = idx, ["span"] = t.span, ["left"] = nil, ["right"] = nil, ["operator"] = nil, ["value"] = nil, ["name"] = "", ["callee"] = nil, ["args"] = nil, ["symbol"] = nil})
        else
            break
        end
    end
    return node
end
function Parser:parse_list_literal(t)
    local items = {}
    local ic = 0
    if (self:peek().kind ~= TokenKind.RBracket) then
        while true do
            items[ic] = self:parse_expression(0)
            ic = zt.add(ic, 1)
            if (self:peek().kind == TokenKind.Comma) then
                self:advance()
            else
                break
            end
        end
    end
    self:expect(TokenKind.RBracket, "esperado ']' após lista")
    return ExprNode.new({["kind"] = NodeKind.ListLiteral, ["items"] = items, ["span"] = t.span, ["left"] = nil, ["right"] = nil, ["operator"] = nil, ["value"] = nil, ["name"] = "", ["callee"] = nil, ["args"] = nil, ["object"] = nil, ["index"] = nil, ["symbol"] = nil})
end
function Parser:parse_map_literal(t)
    local keys = {}
    local values = {}
    local mc = 0
    if (self:peek().kind ~= TokenKind.RBrace) then
        while true do
            keys[mc] = self:parse_expression(0)
            self:expect(TokenKind.Colon, "esperado ':' após chave")
            values[mc] = self:parse_expression(0)
            mc = zt.add(mc, 1)
            if (self:peek().kind == TokenKind.Comma) then
                self:advance()
            else
                break
            end
        end
    end
    self:expect(TokenKind.RBrace, "esperado '}' após mapa")
    return ExprNode.new({["kind"] = NodeKind.MapLiteral, ["keys"] = keys, ["values"] = values, ["span"] = t.span, ["left"] = nil, ["right"] = nil, ["operator"] = nil, ["value"] = nil, ["name"] = "", ["callee"] = nil, ["args"] = nil, ["object"] = nil, ["index"] = nil, ["symbol"] = nil})
end
function Parser:get_precedence(k)
    if zt.unwrap_or((k == TokenKind.Plus), (k == TokenKind.Minus)) then
        return 7
    end
    if zt.unwrap_or((k == TokenKind.Star), (k == TokenKind.Slash)) then
        return 8
    end
    if zt.unwrap_or((k == TokenKind.EqualEqual), (k == TokenKind.BangEqual)) then
        return 4
    end
    if zt.unwrap_or(zt.unwrap_or(zt.unwrap_or((k == TokenKind.Less), (k == TokenKind.LessEqual)), (k == TokenKind.Greater)), (k == TokenKind.GreaterEqual)) then
        return 5
    end
    if (k == TokenKind.KwAnd) then
        return 3
    end
    if (k == TokenKind.KwOr) then
        return 2
    end
    return 0
end
function Parser:parse_params()
    self:expect(TokenKind.LParen, "esperado '('")
    local params = {}
    local pc = 0
    if (self:peek().kind ~= TokenKind.RParen) then
        while true do
            local p = self:expect(TokenKind.Identifier, "esperado parâmetro")
            if (self:peek().kind == TokenKind.Colon) then
                self:advance()
                self:advance()
            end
            params[pc] = p.text
            pc = zt.add(pc, 1)
            if (self:peek().kind == TokenKind.Comma) then
                self:advance()
            else
                break
            end
        end
    end
    self:expect(TokenKind.RParen, "esperado ')'")
    return params
end
function Parser:peek()
    return self.tokens[self.pos]
end
function Parser:is_at_end()
    return (self.tokens[self.pos].kind == TokenKind.EOF)
end
function Parser:advance()
    local t = self.tokens[self.pos]
    self.pos = zt.add(self.pos, 1)
    return t
end
function Parser:expect(k, m)
    if (self:peek().kind == k) then
        return self:advance()
    end
    return Token.new({["kind"] = TokenKind.BAD, ["text"] = m, ["span"] = nil})
end
function Scope:define(sym)
    self.symbols[sym.name] = sym
end
function ModuleManager:resolve_path(module_name)
    local path = ""
    -- native lua
    
 if module_name : sub ( 1 , 4 ) == "std." then 
 path = "src/stdlib/" .. module_name : sub ( 5 ) : gsub ( "%." , "/" ) .. ".zt" 
 else 
 path = self . project_root .. "/" .. module_name : gsub ( "%." , "/" ) .. ".zt" 
 end 
 
    return path
end
function ModuleManager:get_ast(module_name)
    if (self.cache[module_name] ~= nil) then
        return self.cache[module_name]
    end
    local path = self:resolve_path(module_name)
    local src = ""
    -- native lua
    
 local f = io . open ( path , "r" ) 
 if f then 
 src = f : read ( "*a" ) 
 f : close ( ) 
 else 
 
 f = io . open ( module_name , "r" ) 
 if f then 
 src = f : read ( "*a" ) 
 f : close ( ) 
 end 
 end 
 
    if (src == "") then
        return nil
    end
    local l = Lexer.new({["source"] = src, ["pos"] = 0, ["tokens"] = {}})
    local ts = l:tokenize()
    local p = Parser.new({["tokens"] = ts, ["pos"] = 0})
    local unit = p:parse()
    self.cache[module_name] = unit
    return unit
end
function Binder:bind_unit(u)
    local imps = u.imports
    if (imps ~= nil) then
        local i = 0
        while (imps[i] ~= nil) do
            local imp = imps[i]
            local module_ast = self.module_manager:get_ast(imp.body)
            if (module_ast ~= nil) then
                local target_scope = self.module_scopes[imp.body]
                if (target_scope == nil) then
                    local mod_binder = Binder.new({["current_scope"] = Scope.new({["parent"] = nil, ["symbols"] = {}}), ["module_manager"] = self.module_manager, ["module_scopes"] = self.module_scopes})
                    mod_binder:bind_unit(module_ast)
                    target_scope = mod_binder.current_scope
                    self.module_scopes[imp.body] = target_scope
                end
                local sym = Symbol.new({["kind"] = 4, ["name"] = imp.name, ["type_name"] = "module", ["is_pub"] = false, ["declaration"] = imp, ["module_scope"] = target_scope})
                self.current_scope:define(sym)
            end
            i = zt.add(i, 1)
        end
    end
    local decls = u.declarations
    if (decls ~= nil) then
        local i = 0
        while (decls[i] ~= nil) do
            self:_declare(decls[i])
            i = zt.add(i, 1)
        end
        i = 0
        while (decls[i] ~= nil) do
            self:_bind_node(decls[i])
            i = zt.add(i, 1)
        end
    end
end
function Binder:_declare(n)
    if zt.unwrap_or(zt.unwrap_or(zt.unwrap_or((n.kind == NodeKind.FuncDecl), (n.kind == NodeKind.VarDecl)), (n.kind == NodeKind.StructDecl)), (n.kind == NodeKind.EnumDecl)) then
        local k = 0
        if (n.kind == NodeKind.VarDecl) then
            k = 1
        end
        if (n.kind == NodeKind.StructDecl) then
            k = 2
        end
        if (n.kind == NodeKind.EnumDecl) then
            k = 2
        end
        local sym = Symbol.new({["kind"] = k, ["name"] = n.name, ["type_name"] = "any", ["is_pub"] = n.is_pub, ["declaration"] = n})
        self.current_scope:define(sym)
    end
end
function Binder:_bind_node(n)
    if (n == nil) then
        return
    end
    if (n.kind == NodeKind.FuncDecl) then
        local prev = self.current_scope
        self.current_scope = Scope.new({["parent"] = prev, ["symbols"] = {}})
        local ps = n.params
        if (ps ~= nil) then
            local i = 0
            while (ps[i] ~= nil) do
                local sym = Symbol.new({["kind"] = 3, ["name"] = ps[i], ["type_name"] = "any", ["is_pub"] = false, ["declaration"] = n})
                self.current_scope:define(sym)
                i = zt.add(i, 1)
            end
        end
        local body = n.body
        if (body ~= nil) then
            local i = 0
            while (body[i] ~= nil) do
                self:_bind_node(body[i])
                i = zt.add(i, 1)
            end
        end
        self.current_scope = prev
    elseif (n.kind == NodeKind.VarDecl) then
        self:_bind_node(n.body)
    elseif (n.kind == NodeKind.IdentifierExpr) then
        n.symbol = scope_lookup(self.current_scope, n.name)
    elseif (n.kind == NodeKind.BinaryExpr) then
        self:_bind_node(n.left)
        self:_bind_node(n.right)
    elseif (n.kind == NodeKind.ReturnStmt) then
        self:_bind_node(n.value)
    elseif (n.kind == NodeKind.ExprStmt) then
        self:_bind_node(n.value)
    elseif (n.kind == NodeKind.AssignStmt) then
        self:_bind_node(n.target)
        self:_bind_node(n.value)
    elseif (n.kind == NodeKind.ForInStmt) then
        self:_bind_node(n.iterable)
        local fb = n.body
        if (fb ~= nil) then
            local i = 0
            while (fb[i] ~= nil) do
                self:_bind_node(fb[i])
                i = zt.add(i, 1)
            end
        end
    elseif (n.kind == NodeKind.RepeatStmt) then
        self:_bind_node(n.count_expr)
        local rb = n.body
        if (rb ~= nil) then
            local i = 0
            while (rb[i] ~= nil) do
                self:_bind_node(rb[i])
                i = zt.add(i, 1)
            end
        end
    elseif (n.kind == NodeKind.MatchStmt) then
        self:_bind_node(n.condition)
        local mcs = n.cases
        if (mcs ~= nil) then
            local i = 0
            while (mcs[i] ~= nil) do
                local c = mcs[i]
                local ps = c.patterns
                if (ps ~= nil) then
                    local k = 0
                    while (ps[k] ~= nil) do
                        self:_bind_node(ps[k])
                        k = zt.add(k, 1)
                    end
                end
                local cb = c.body
                if (cb ~= nil) then
                    local j = 0
                    while (cb[j] ~= nil) do
                        self:_bind_node(cb[j])
                        j = zt.add(j, 1)
                    end
                end
                i = zt.add(i, 1)
            end
        end
        if (n.else_body ~= nil) then
            local i = 0
            while (n.else_body[i] ~= nil) do
                self:_bind_node(n.else_body[i])
                i = zt.add(i, 1)
            end
        end
    elseif (n.kind == NodeKind.MemberExpr) then
        self:_bind_node(n.object)
        if ((n.object.kind == NodeKind.IdentifierExpr) and (n.object.symbol ~= nil)) then
            local sym = n.object.symbol
            if (sym.kind == 4) then
                local mod_scope = sym.module_scope
                n.symbol = scope_lookup(mod_scope, n.name)
            end
        end
    end
end
function Emitter:emit(txt)
    local indent = ""
    local i = 0
    while (self.indent_level > i) do
        indent = zt.add(indent, "    ")
        i = zt.add(i, 1)
    end
    self.output[self.line_count] = zt.add(indent, txt)
    self.line_count = zt.add(self.line_count, 1)
end
function Emitter:indent()
    self.indent_level = zt.add(self.indent_level, 1)
end
function Emitter:dedent()
    self.indent_level = (self.indent_level - 1)
end
function Emitter:generate(u)
    self:emit("-- Transpilado por Zenith Ascension (Native v0.4)")
    self:emit("local zt = require(\"src.backend.lua.runtime.zenith_rt\")")
    local imps = u.imports
    if (imps ~= nil) then
        local i = 0
        while (imps[i] ~= nil) do
            local imp = imps[i]
            local lua_path = imp.body
            -- native lua
            
 if lua_path : sub ( 1 , 4 ) == "std." then 
 lua_path = "src.stdlib." .. lua_path : sub ( 5 ) 
 end 
 
            self:emit(zt.add(zt.add(zt.add(zt.add("local ", imp.name), " = require(\""), lua_path), "\")"))
            i = zt.add(i, 1)
        end
    end
    self:emit("")
    local decls = u.declarations
    if (decls ~= nil) then
        local i = 0
        while (decls[i] ~= nil) do
            self:_bind_node_emit(decls[i])
            i = zt.add(i, 1)
        end
    end
    self:emit("")
    self:emit(zt.add("return ", ((string . char ( 123 ) ))))
    self:indent()
    if (decls ~= nil) then
        local i = 0
        while (decls[i] ~= nil) do
            local d = decls[i]
            if d.is_pub then
                self:emit(zt.add(zt.add(zt.add(d.name, " = "), d.name), ","))
            end
            i = zt.add(i, 1)
        end
    end
    self:dedent()
    self:emit(((string . char ( 125 ) )))
    local res = ""
    local j = 0
    while (self.line_count > j) do
        res = zt.add(zt.add(res, self.output[j]), "\
")
        j = zt.add(j, 1)
    end
    return res
end
function Emitter:_bind_node_emit(n)
    if (n == nil) then
        return
    end
    if (n.kind == NodeKind.FuncDecl) then
        local h = zt.add(zt.add("function ", n.name), "(")
        local ps = n.params
        if (ps ~= nil) then
            local i = 0
            while (ps[i] ~= nil) do
                h = zt.add(h, ps[i])
                if (ps[zt.add(i, 1)] ~= nil) then
                    h = zt.add(h, ", ")
                end
                i = zt.add(i, 1)
            end
        end
        self:emit(zt.add(h, ")"))
        self:indent()
        local body = n.body
        if (body ~= nil) then
            local i = 0
            while (body[i] ~= nil) do
                self:_bind_node_emit(body[i])
                i = zt.add(i, 1)
            end
        end
        self:dedent()
        self:emit("end")
    elseif (n.kind == NodeKind.VarDecl) then
        self:emit(zt.add(zt.add(zt.add("local ", n.name), " = "), self:expr_to_lua(n.body)))
    elseif (n.kind == NodeKind.ReturnStmt) then
        self:emit(zt.add("return ", self:expr_to_lua(n.value)))
    elseif (n.kind == NodeKind.AssignStmt) then
        self:emit(zt.add(zt.add(self:expr_to_lua(n.target), " = "), self:expr_to_lua(n.value)))
    elseif (n.kind == NodeKind.IfStmt) then
        self:emit(zt.add(zt.add("if (", self:expr_to_lua(n.condition)), ") then"))
        self:indent()
        local body = n.body
        if (body ~= nil) then
            local i = 0
            while (body[i] ~= nil) do
                self:_bind_node_emit(body[i])
                i = zt.add(i, 1)
            end
        end
        self:dedent()
        self:emit("end")
    elseif (n.kind == NodeKind.WhileStmt) then
        self:emit(zt.add(zt.add("while (", self:expr_to_lua(n.condition)), ") do"))
        self:indent()
        local body = n.body
        if (body ~= nil) then
            local i = 0
            while (body[i] ~= nil) do
                self:_bind_node_emit(body[i])
                i = zt.add(i, 1)
            end
        end
        self:dedent()
        self:emit("end")
    elseif (n.kind == NodeKind.ForInStmt) then
        local vars = n.variables
        local v_str = vars[0]
        self:emit(zt.add(zt.add(zt.add(zt.add("for _, ", v_str), " in zt.iter("), self:expr_to_lua(n.iterable)), ") do"))
        self:indent()
        local body = n.body
        if (body ~= nil) then
            local i = 0
            while (body[i] ~= nil) do
                self:_bind_node_emit(body[i])
                i = zt.add(i, 1)
            end
        end
        self:dedent()
        self:emit("end")
    elseif (n.kind == NodeKind.RepeatStmt) then
        self:emit(zt.add(zt.add("for _ = 1, ", self:expr_to_lua(n.count_expr)), " do"))
        self:indent()
        local body = n.body
        if (body ~= nil) then
            local i = 0
            while (body[i] ~= nil) do
                self:_bind_node_emit(body[i])
                i = zt.add(i, 1)
            end
        end
        self:dedent()
        self:emit("end")
    elseif (n.kind == NodeKind.MatchStmt) then
        self:emit("do")
        self:indent()
        self:emit(zt.add("local _m = ", self:expr_to_lua(n.condition)))
        local cases = n.cases
        local i = 0
        while (cases[i] ~= nil) do
            local c = cases[i]
            local cond = ""
            local ps = c.patterns
            local j = 0
            while (ps[j] ~= nil) do
                local p_str = self:expr_to_lua(ps[j])
                if (j > 0) then
                    cond = zt.add(cond, " or ")
                end
                cond = zt.add(zt.add(zt.add(cond, "(_m == "), p_str), ")")
                j = zt.add(j, 1)
            end
            if (i == 0) then
                self:emit(zt.add(zt.add("if ", cond), " then"))
            else
                self:emit(zt.add(zt.add("elseif ", cond), " then"))
            end
            self:indent()
            local cbody = c.body
            local k = 0
            while (cbody[k] ~= nil) do
                self:_bind_node_emit(cbody[k])
                k = zt.add(k, 1)
            end
            self:dedent()
            i = zt.add(i, 1)
        end
        if (n.else_body ~= nil) then
            self:emit("else")
            self:indent()
            local ebody = n.else_body
            local k = 0
            while (ebody[k] ~= nil) do
                self:_bind_node_emit(ebody[k])
                k = zt.add(k, 1)
            end
            self:dedent()
        end
        self:emit("end")
        self:dedent()
        self:emit("end")
    elseif (n.kind == NodeKind.ExprStmt) then
        self:emit(self:expr_to_lua(n.value))
    elseif (n.kind == NodeKind.StructDecl) then
        self:emit(zt.add(zt.add(zt.add(zt.add("local ", n.name), " = "), ((string . char ( 123 ) ))), ((string . char ( 125 ) ))))
        self:emit(zt.add(zt.add(n.name, ".__index = "), n.name))
        self:emit(zt.add(zt.add("function ", n.name), ".new(fields)"))
        self:indent()
        self:emit(zt.add(zt.add(zt.add(zt.add(zt.add("local self = setmetatable(", ((string . char ( 123 ) ))), ((string . char ( 125 ) ))), ", "), n.name), ")"))
        local fs = n.fields
        if (fs ~= nil) then
            local i = 0
            while (fs[i] ~= nil) do
                local f = fs[i]
                local init = "nil"
                if (f.body ~= nil) then
                    init = self:expr_to_lua(f.body)
                end
                self:emit(zt.add(zt.add(zt.add(zt.add(zt.add("self.", f.name), " = fields."), f.name), " or "), init))
                i = zt.add(i, 1)
            end
        end
        self:emit("return self")
        self:dedent()
        self:emit("end")
        local ms = n.methods
        if (ms ~= nil) then
            local i = 0
            while (ms[i] ~= nil) do
                self:_emit_method(ms[i], n.name)
                i = zt.add(i, 1)
            end
        end
    elseif (n.kind == NodeKind.EnumDecl) then
        self:emit(zt.add(zt.add(zt.add("local ", n.name), " = "), ((string . char ( 123 ) ))))
        self:indent()
        local mems = n.members
        if (mems ~= nil) then
            local i = 0
            while (mems[i] ~= nil) do
                self:emit(zt.add(zt.add(zt.add(zt.add(zt.add(mems[i], " = "), ((string . char ( 34 ) ))), mems[i]), ((string . char ( 34 ) ))), ","))
                i = zt.add(i, 1)
            end
        end
        self:dedent()
        self:emit(((string . char ( 125 ) )))
    end
end
function Emitter:_emit_method(n, struct_name)
    local h = zt.add(zt.add(zt.add(zt.add("function ", struct_name), ":"), n.name), "(")
    local ps = n.params
    if (ps ~= nil) then
        local i = 0
        while (ps[i] ~= nil) do
            h = zt.add(h, ps[i])
            if (ps[zt.add(i, 1)] ~= nil) then
                h = zt.add(h, ", ")
            end
            i = zt.add(i, 1)
        end
    end
    self:emit(zt.add(h, ")"))
    self:indent()
    local body = n.body
    if (body ~= nil) then
        local i = 0
        while (body[i] ~= nil) do
            self:_bind_node_emit(body[i])
            i = zt.add(i, 1)
        end
    end
    self:dedent()
    self:emit("end")
end
function Emitter:expr_to_lua(n)
    if (n == nil) then
        return "nil"
    end
    if (n.kind == NodeKind.LiteralExpr) then
        return n.value
    end
    if (n.kind == NodeKind.IdentifierExpr) then
        return n.name
    end
    if (n.kind == NodeKind.BinaryExpr) then
        return zt.add(zt.add(zt.add(zt.add(zt.add(zt.add("(", self:expr_to_lua(n.left)), " "), n.operator.text), " "), self:expr_to_lua(n.right)), ")")
    elseif (n.kind == NodeKind.CallExpr) then
        local c = self:expr_to_lua(n.callee)
        local a = "("
        local args = n.args
        if (args ~= nil) then
            local i = 0
            while (args[i] ~= nil) do
                a = zt.add(a, self:expr_to_lua(args[i]))
                if (args[zt.add(i, 1)] ~= nil) then
                    a = zt.add(a, ", ")
                end
                i = zt.add(i, 1)
            end
        end
        return zt.add(zt.add(c, a), ")")
    elseif (n.kind == NodeKind.MemberExpr) then
        return zt.add(zt.add(self:expr_to_lua(n.object), "."), n.name)
    elseif (n.kind == NodeKind.IndexExpr) then
        return zt.add(zt.add(zt.add(self:expr_to_lua(n.object), "["), self:expr_to_lua(n.index)), "]")
    elseif (n.kind == NodeKind.UnaryExpr) then
        local op = n.operator.text
        if (n.operator.kind == TokenKind.KwNot) then
            op = "not"
        end
        return zt.add(zt.add(zt.add(zt.add("(", op), " "), self:expr_to_lua(n.right)), ")")
    elseif (n.kind == NodeKind.ListLiteral) then
        local res = ((string . char ( 123 ) ))
        local items = n.items
        if (items ~= nil) then
            local i = 0
            while (items[i] ~= nil) do
                res = zt.add(res, self:expr_to_lua(items[i]))
                if (items[zt.add(i, 1)] ~= nil) then
                    res = zt.add(res, ", ")
                end
                i = zt.add(i, 1)
            end
        end
        return zt.add(res, ((string . char ( 125 ) )))
    elseif (n.kind == NodeKind.MapLiteral) then
        local res = ((string . char ( 123 ) ))
        local keys = n.keys
        local vals = n.values
        if (keys ~= nil) then
            local i = 0
            while (keys[i] ~= nil) do
                res = zt.add(zt.add(zt.add(zt.add(res, "["), self:expr_to_lua(keys[i])), "] = "), self:expr_to_lua(vals[i]))
                if (keys[zt.add(i, 1)] ~= nil) then
                    res = zt.add(res, ", ")
                end
                i = zt.add(i, 1)
            end
        end
        return zt.add(res, ((string . char ( 125 ) )))
    elseif (n.kind == NodeKind.StructInitExpr) then
        local res = zt.add(zt.add(n.name, ".new("), ((string . char ( 123 ) )))
        local keys = n.keys
        local vals = n.values
        if (keys ~= nil) then
            local i = 0
            while (keys[i] ~= nil) do
                res = zt.add(zt.add(zt.add(zt.add(zt.add(zt.add(res, "["), ((string . char ( 34 ) ))), keys[i]), ((string . char ( 34 ) ))), "] = "), self:expr_to_lua(vals[i]))
                if (keys[zt.add(i, 1)] ~= nil) then
                    res = zt.add(res, ", ")
                end
                i = zt.add(i, 1)
            end
        end
        return zt.add(zt.add(res, ((string . char ( 125 ) ))), ")")
    elseif (n.kind == NodeKind.SelfExpr) then
        return "self"
    end
    return "nil"
end
-- native lua

 if arg and # arg > 0 then 
 local src_file = arg [ 1 ] 
 local f = io . open ( src_file , "r" ) 
 if f then 
 local content = f : read ( "*a" ) 
 f : close ( ) 
 local success , result = pcall ( compile , content ) 
 if success then 
 io . write ( result ) 
 else 
 io . stderr : write ( "Erro na compilação: " .. tostring ( result ) .. "\n" ) 
 os . exit ( 1 ) 
 end 
 end 
 end 
 


return {
    TokenKind = TokenKind,
    NodeKind = NodeKind,
    Symbol = Symbol,
    CompilationUnit = CompilationUnit,
    DeclNode = DeclNode,
    StmtNode = StmtNode,
    ExprNode = ExprNode,
    Token = Token,
    Lexer = Lexer,
    Parser = Parser,
    scope_lookup = scope_lookup,
    Scope = Scope,
    ModuleManager = ModuleManager,
    Binder = Binder,
    Emitter = Emitter,
    compile = compile,
}