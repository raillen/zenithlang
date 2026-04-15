-- Transpilado por Zenith v0.2.5
local zt = require("src.backend.lua.runtime.zenith_rt")
local Present = zt.Optional.Present
local Empty = zt.Optional.Empty
local Success = zt.Outcome.Success
local Failure = zt.Outcome.Failure

local TokenKind, NodeKind, Span, Diagnostic, DiagnosticBag, Symbol, CompilationUnit, DeclNode, StmtNode, ExprNode, Token, lbrace_text, rbrace_text, any_key, has_slot, text_len, Lexer, Parser, scope_lookup, symbol_is_present, symbol_is_missing, scope_has_local_symbol, Scope, ModuleManager, Binder, Emitter, compile_ext, compile

-- Namespace: compiler.syntax

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
    KwNative = "KwNative",
    KwExtern = "KwExtern",
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
    NativeLuaStmt = "NativeLuaStmt",
    NativeLuaExpr = "NativeLuaExpr",
    ExternDecl = "ExternDecl",
}

local Span = {}
Span.__index = Span
Span._metadata = {
    name = "Span",
    fields = {
        { name = "file", type = "any" },
        { name = "start", type = "any" },
        { name = "finish", type = "any" },
        { name = "line", type = "any" },
        { name = "column", type = "any" },
    },
    methods = {
        "merge",
    }
}

function Span.new(fields)
    local self = setmetatable({}, Span)
    self.file = fields.file or nil
    self.start = fields.start or nil
    self.finish = fields.finish or nil
    self.line = fields.line or nil
    self.column = fields.column or nil
    return self
end

local Diagnostic = {}
Diagnostic.__index = Diagnostic
Diagnostic._metadata = {
    name = "Diagnostic",
    fields = {
        { name = "id", type = "any" },
        { name = "message", type = "any" },
        { name = "span", type = "any" },
        { name = "level", type = "any" },
    },
    methods = {
    }
}

function Diagnostic.new(fields)
    local self = setmetatable({}, Diagnostic)
    self.id = fields.id or nil
    self.message = fields.message or nil
    self.span = fields.span or nil
    self.level = fields.level or nil
    return self
end

local DiagnosticBag = {}
DiagnosticBag.__index = DiagnosticBag
DiagnosticBag._metadata = {
    name = "DiagnosticBag",
    fields = {
        { name = "diagnostics", type = "any" },
        { name = "count", type = "any" },
    },
    methods = {
        "report",
        "report_error",
    }
}

function DiagnosticBag.new(fields)
    local self = setmetatable({}, DiagnosticBag)
    self.diagnostics = fields.diagnostics or nil
    self.count = fields.count or nil
    return self
end

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
        { name = "namespace", type = "any" },
        { name = "imports", type = "any" },
        { name = "declarations", type = "any" },
        { name = "span", type = "any" },
        { name = "diagnostics", type = "any" },
    },
    methods = {
    }
}

function CompilationUnit.new(fields)
    local self = setmetatable({}, CompilationUnit)
    self.namespace = fields.namespace or nil
    self.imports = fields.imports or nil
    self.declarations = fields.declarations or nil
    self.span = fields.span or nil
    self.diagnostics = fields.diagnostics or nil
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
        { name = "attributes", type = "any" },
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
    self.attributes = fields.attributes or nil
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
        { name = "else_body", type = "any" },
        { name = "value", type = "any" },
        { name = "target", type = "any" },
        { name = "cases", type = "any" },
        { name = "patterns", type = "any" },
        { name = "variables", type = "any" },
        { name = "iterable", type = "any" },
        { name = "count_expr", type = "any" },
        { name = "span", type = "any" },
        { name = "lua_code", type = "any" },
    },
    methods = {
    }
}

function StmtNode.new(fields)
    local self = setmetatable({}, StmtNode)
    self.kind = fields.kind or nil
    self.condition = fields.condition or nil
    self.body = fields.body or nil
    self.else_body = fields.else_body or nil
    self.value = fields.value or nil
    self.target = fields.target or nil
    self.cases = fields.cases or nil
    self.patterns = fields.patterns or nil
    self.variables = fields.variables or nil
    self.iterable = fields.iterable or nil
    self.count_expr = fields.count_expr or nil
    self.span = fields.span or nil
    self.lua_code = fields.lua_code or nil
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
        { name = "lua_code", type = "any" },
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
    self.lua_code = fields.lua_code or nil
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

function lbrace_text()
    return "{"
end

function rbrace_text()
    return "}"
end

function any_key(v)
    return v
end

function has_slot(items, index)
    if (items == nil) then
        return false
    end
    return (zt.index_any(items, index) ~= nil)
end

function text_len(v)
    -- native lua
    return # v 
end

local Lexer = {}
Lexer.__index = Lexer
Lexer._metadata = {
    name = "Lexer",
    fields = {
        { name = "source", type = "any" },
        { name = "pos", type = "any" },
        { name = "line", type = "any" },
        { name = "col", type = "any" },
        { name = "file", type = "any" },
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
    self.line = fields.line or nil
    self.col = fields.col or nil
    self.file = fields.file or nil
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
        { name = "diagnostics", type = "any" },
    },
    methods = {
        "parse",
    }
}

function Parser.new(fields)
    local self = setmetatable({}, Parser)
    self.tokens = fields.tokens or nil
    self.pos = fields.pos or nil
    self.diagnostics = fields.diagnostics or nil
    return self
end

function scope_lookup(sc, name)
    local cur = sc
    while (cur ~= nil) do
        local sym = zt.index_any(cur.symbols, name)
        if symbol_is_present(sym) then
            return sym
        end
        cur = cur.parent
    end
    return nil
end

function symbol_is_present(sym)
    return (sym ~= nil)
end

function symbol_is_missing(sym)
    return not symbol_is_present(sym)
end

function scope_has_local_symbol(sc, name)
    return symbol_is_present(zt.index_any(sc.symbols, name))
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
        { name = "diagnostics", type = "any" },
        { name = "target_os", type = "any" },
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
    self.diagnostics = fields.diagnostics or nil
    self.target_os = fields.target_os or nil
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
        { name = "has_namespace", type = "any" },
        { name = "target_os", type = "any" },
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
    self.has_namespace = fields.has_namespace or nil
    self.target_os = fields.target_os or nil
    return self
end

function compile_ext(src, file, target)
    local bag = DiagnosticBag.new({["diagnostics"] = {}, ["count"] = 0})
    local l = Lexer.new({["source"] = src, ["pos"] = 0, ["line"] = 1, ["col"] = 1, ["file"] = file, ["tokens"] = {}})
    local ts = l:tokenize()
    local p = Parser.new({["tokens"] = ts, ["pos"] = 0, ["diagnostics"] = bag})
    local u = p:parse()
    if (bag.count > 0) then
        return bag
    end
    local mm = ModuleManager.new({["project_root"] = ".", ["cache"] = {}})
    local b = Binder.new({["current_scope"] = Scope.new({["parent"] = nil, ["symbols"] = {}}), ["module_manager"] = mm, ["module_scopes"] = {}, ["diagnostics"] = bag, ["target_os"] = target})
    b:bind_unit(u)
    if (bag.count > 0) then
        return bag
    end
    local e = Emitter.new({["output"] = {}, ["indent_level"] = 0, ["line_count"] = 0, ["has_namespace"] = false, ["target_os"] = target})
    local lua = e:generate(u)
    return lua
end

function compile(src)
    local host_os = "windows"
    -- native lua
    
 local sep = package . config : sub ( 1 , 1 ) 
 if sep == "/" then host_os = "linux" end 
 
    local res = compile_ext(src, "main.zt", host_os)
    -- native lua
    
 if type ( res ) == "string" then return res end 
 return "Erro: Falha na compilação (" .. ( res . count or 0 ) .. " erros)" 
 
end

-- Struct Methods
function Span:merge(other)
    return Span.new({["file"] = self.file, ["start"] = self.start, ["finish"] = other.finish, ["line"] = self.line, ["column"] = self.column})
end
function DiagnosticBag:report(id, msg, sp, lv)
    local d = Diagnostic.new({["id"] = id, ["message"] = msg, ["span"] = sp, ["level"] = lv})
    if (self.diagnostics == nil) then
        self.diagnostics = {}
    end
    self.diagnostics[self.count] = d
    self.count = zt.add(self.count, 1)
end
function DiagnosticBag:report_error(id, msg, sp)
    self:report(id, msg, sp, 0)
end
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
    local start_pos = self.pos
    local start_line = self.line
    local start_col = self.col
    if self:is_at_end() then
        return Token.new({["kind"] = TokenKind.EOF, ["text"] = "", ["span"] = Span.new({["file"] = self.file, ["start"] = start_pos, ["finish"] = start_pos, ["line"] = start_line, ["column"] = start_col})})
    end
    local c = self:peek()
    self:advance()
    if self:is_alpha(c) then
        return self:scan_identifier(start_pos, start_line, start_col)
    end
    if self:is_digit(c) then
        return self:scan_number(start_pos, start_line, start_col)
    end
    if (c == "\"") then
        return self:scan_string(start_pos, start_line, start_col)
    end
    local op2 = zt.add(c, self:peek())
    local k2 = self:lookup_operator_double(op2)
    if (k2 ~= TokenKind.BAD) then
        self:advance()
        return Token.new({["kind"] = k2, ["text"] = op2, ["span"] = Span.new({["file"] = self.file, ["start"] = start_pos, ["finish"] = self.pos, ["line"] = start_line, ["column"] = start_col})})
    end
    local k1 = self:lookup_operator_single(c)
    if (k1 ~= TokenKind.BAD) then
        return Token.new({["kind"] = k1, ["text"] = c, ["span"] = Span.new({["file"] = self.file, ["start"] = start_pos, ["finish"] = self.pos, ["line"] = start_line, ["column"] = start_col})})
    end
    return Token.new({["kind"] = TokenKind.BAD, ["text"] = c, ["span"] = Span.new({["file"] = self.file, ["start"] = start_pos, ["finish"] = self.pos, ["line"] = start_line, ["column"] = start_col})})
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
function Lexer:slice_text(start_index, finish_index)
    if (finish_index < start_index) then
        return ""
    end
    local txt = ""
    -- native lua
    
 txt = self . source : sub ( start_index , finish_index ) 
 
    return txt
end
function Lexer:scan_identifier(start, line, col)
    while zt.unwrap_or(self:is_alpha(self:peek()), self:is_digit(self:peek())) do
        self:advance()
    end
    local txt = self:slice_text(zt.add(start, 1), self.pos)
    local kind = self:lookup_keyword(txt)
    return Token.new({["kind"] = kind, ["text"] = txt, ["span"] = Span.new({["file"] = self.file, ["start"] = start, ["finish"] = self.pos, ["line"] = line, ["column"] = col})})
end
function Lexer:scan_number(start, line, col)
    while self:is_digit(self:peek()) do
        self:advance()
    end
    return Token.new({["kind"] = TokenKind.IntegerLiteral, ["text"] = self:slice_text(zt.add(start, 1), self.pos), ["span"] = Span.new({["file"] = self.file, ["start"] = start, ["finish"] = self.pos, ["line"] = line, ["column"] = col})})
end
function Lexer:scan_string(start, line, col)
    while not self:is_at_end() do
        local c = self:peek()
        if (c == "\"") then
            break
        elseif (c == "\\") then
            self:advance()
            if not self:is_at_end() then
                self:advance()
            end
        else
            self:advance()
        end
    end
    self:advance()
    return Token.new({["kind"] = TokenKind.StringLiteral, ["text"] = self:slice_text(zt.add(start, 1), self.pos), ["span"] = Span.new({["file"] = self.file, ["start"] = start, ["finish"] = self.pos, ["line"] = line, ["column"] = col})})
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
    if (txt == "native") then
        return TokenKind.KwNative
    end
    if (txt == "extern") then
        return TokenKind.KwExtern
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
    return TokenKind.BAD
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
    if (op == lbrace_text()) then
        return TokenKind.LBrace
    end
    if (op == rbrace_text()) then
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
    return TokenKind.BAD
end
function Lexer:peek()
    if self:is_at_end() then
        return ""
    end
    return self:slice_text(zt.add(self.pos, 1), zt.add(self.pos, 1))
end
function Lexer:peek_next()
    if (zt.add(self.pos, 1) >= text_len(self.source)) then
        return ""
    end
    return self:slice_text(zt.add(self.pos, 2), zt.add(self.pos, 2))
end
function Lexer:advance()
    local c = self:peek()
    self.pos = zt.add(self.pos, 1)
    if (c == "\
") then
        self.line = zt.add(self.line, 1)
        self.col = 1
    else
        self.col = zt.add(self.col, 1)
    end
end
function Lexer:is_at_end()
    return (self.pos >= text_len(self.source))
end
function Parser:parse()
    local ns = ""
    if (self:peek().kind == TokenKind.KwNamespace) then
        self:advance()
        ns = self:expect_name("esperado namespace").text
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
        local start_pos = self.pos
        local d = self:parse_declaration()
        if (d ~= nil) then
            decls[count] = d
            count = zt.add(count, 1)
        else
            if (self.pos == start_pos) then
                self:advance()
            end
        end
    end
    return CompilationUnit.new({["namespace"] = ns, ["imports"] = imps, ["declarations"] = decls, ["span"] = nil, ["diagnostics"] = self.diagnostics})
end
function Parser:is_name_token(k)
    if (k == TokenKind.Identifier) then
        return true
    end
    if zt.unwrap_or(zt.unwrap_or(zt.unwrap_or(zt.unwrap_or((k == TokenKind.KwFunc), (k == TokenKind.KwVar)), (k == TokenKind.KwIf)), (k == TokenKind.KwElse)), (k == TokenKind.KwElif)) then
        return true
    end
    if zt.unwrap_or(zt.unwrap_or(zt.unwrap_or(zt.unwrap_or((k == TokenKind.KwWhile), (k == TokenKind.KwReturn)), (k == TokenKind.KwPub)), (k == TokenKind.KwStruct)), (k == TokenKind.KwEnum)) then
        return true
    end
    if zt.unwrap_or(zt.unwrap_or(zt.unwrap_or((k == TokenKind.KwSelf), (k == TokenKind.KwAnd)), (k == TokenKind.KwOr)), (k == TokenKind.KwDo)) then
        return true
    end
    if zt.unwrap_or(zt.unwrap_or(zt.unwrap_or(zt.unwrap_or((k == TokenKind.KwEnd), (k == TokenKind.KwFor)), (k == TokenKind.KwIn)), (k == TokenKind.KwRepeat)), (k == TokenKind.KwMatch)) then
        return true
    end
    if zt.unwrap_or(zt.unwrap_or(zt.unwrap_or(zt.unwrap_or((k == TokenKind.KwCase), (k == TokenKind.KwBreak)), (k == TokenKind.KwContinue)), (k == TokenKind.KwNamespace)), (k == TokenKind.KwImport)) then
        return true
    end
    if zt.unwrap_or(zt.unwrap_or(zt.unwrap_or((k == TokenKind.KwExport), (k == TokenKind.KwAs)), (k == TokenKind.KwNative)), (k == TokenKind.KwExtern)) then
        return true
    end
    return false
end
function Parser:expect_name(m)
    local t = self:peek()
    if self:is_name_token(t.kind) then
        return self:advance()
    end
    if (self.diagnostics ~= nil) then
        self.diagnostics:report_error("ZT-1001", m, t.span)
    end
    self:advance()
    return t
end
function Parser:parse_import_decl()
    local t = self:peek()
    self:advance()
    local path = ""
    while true do
        path = zt.add(path, self:expect_name("esperado nome").text)
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
        alias = self:expect_name("esperado alias").text
    end
    return DeclNode.new({["kind"] = NodeKind.ImportDecl, ["name"] = alias, ["body"] = path, ["span"] = t.span, ["is_pub"] = false, ["attributes"] = nil, ["params"] = nil, ["fields"] = nil, ["methods"] = nil, ["members"] = nil, ["symbol"] = nil})
end
function Parser:parse_type()
    self:expect_name("esperado nome do tipo")
    if (self:peek().kind == TokenKind.Less) then
        self:advance()
        while (not self:is_at_end() and (self:peek().kind ~= TokenKind.Greater)) do
            self:parse_type()
            if (self:peek().kind == TokenKind.Comma) then
                self:advance()
            end
        end
        self:expect(TokenKind.Greater, "esperado '>' após os genéricos")
    end
end
function Parser:parse_declaration()
    local attrs = {}
    local ac = 0
    while (self:peek().kind == TokenKind.At) do
        self:advance()
        local attr_name = self:expect_name("esperado nome do atributo").text
        attrs[ac] = attr_name
        ac = zt.add(ac, 1)
    end
    local is_pub = false
    if zt.unwrap_or((self:peek().kind == TokenKind.KwPub), (self:peek().kind == TokenKind.KwExport)) then
        self:advance()
        is_pub = true
    end
    local t = self:peek()
    local node = nil
    if (t.kind == TokenKind.KwFunc) then
        node = self:parse_func_decl(is_pub)
    elseif (t.kind == TokenKind.KwVar) then
        node = self:parse_var_decl(is_pub)
    elseif (t.kind == TokenKind.KwStruct) then
        node = self:parse_struct_decl(is_pub)
    elseif (t.kind == TokenKind.KwEnum) then
        node = self:parse_enum_decl(is_pub)
    elseif (t.kind == TokenKind.KwExtern) then
        node = self:parse_extern_decl()
    elseif (t.kind == TokenKind.KwNative) then
        self:parse_native_lua_stmt()
    end
    if (node ~= nil) then
        node.attributes = attrs
    end
    return node
end
function Parser:parse_func_decl(is_pub)
    self:advance()
    local name_t = self:expect_name("esperado nome da função")
    local params = self:parse_params()
    if (self:peek().kind == TokenKind.Arrow) then
        self:advance()
        self:parse_type()
    end
    local body = self:parse_block()
    self:expect(TokenKind.KwEnd, "esperado 'end' na função")
    return DeclNode.new({["kind"] = NodeKind.FuncDecl, ["name"] = name_t.text, ["params"] = params, ["body"] = body, ["is_pub"] = is_pub, ["attributes"] = nil, ["span"] = name_t.span, ["symbol"] = nil})
end
function Parser:parse_var_decl(is_pub)
    self:advance()
    local name_t = self:expect_name("esperado nome")
    if (self:peek().kind == TokenKind.Colon) then
        self:advance()
        self:parse_type()
    end
    local init = nil
    if (self:peek().kind == TokenKind.Equal) then
        self:advance()
        init = self:parse_expression(0)
    end
    return DeclNode.new({["kind"] = NodeKind.VarDecl, ["name"] = name_t.text, ["is_pub"] = is_pub, ["attributes"] = nil, ["body"] = init, ["span"] = name_t.span, ["params"] = nil, ["symbol"] = nil, ["fields"] = nil, ["methods"] = nil, ["members"] = nil})
end
function Parser:parse_struct_decl(is_pub)
    self:advance()
    local name_t = self:expect_name("esperado nome da struct")
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
            local f_name = self:expect_name("esperado nome do campo")
            if (self:peek().kind == TokenKind.Colon) then
                self:advance()
                self:parse_type()
            end
            local init = nil
            if (self:peek().kind == TokenKind.Equal) then
                self:advance()
                init = self:parse_expression(0)
            end
            fields[fc] = DeclNode.new({["kind"] = NodeKind.VarDecl, ["name"] = f_name.text, ["is_pub"] = is_field_pub, ["attributes"] = nil, ["body"] = init, ["span"] = f_name.span, ["params"] = nil, ["symbol"] = nil, ["fields"] = nil, ["methods"] = nil, ["members"] = nil})
            fc = zt.add(fc, 1)
        end
    end
    self:expect(TokenKind.KwEnd, "esperado 'end' na struct")
    return DeclNode.new({["kind"] = NodeKind.StructDecl, ["name"] = name_t.text, ["is_pub"] = is_pub, ["attributes"] = nil, ["fields"] = fields, ["methods"] = methods, ["span"] = name_t.span, ["params"] = nil, ["body"] = nil, ["members"] = nil, ["symbol"] = nil})
end
function Parser:parse_enum_decl(is_pub)
    self:advance()
    local name_t = self:expect_name("esperado nome do enum")
    local members = {}
    local mc = 0
    while (not self:is_at_end() and (self:peek().kind ~= TokenKind.KwEnd)) do
        local m_name = self:expect_name("esperado membro")
        members[mc] = m_name.text
        mc = zt.add(mc, 1)
    end
    self:expect(TokenKind.KwEnd, "esperado 'end' no enum")
    return DeclNode.new({["kind"] = NodeKind.EnumDecl, ["name"] = name_t.text, ["is_pub"] = is_pub, ["attributes"] = nil, ["members"] = members, ["span"] = name_t.span, ["params"] = nil, ["body"] = nil, ["fields"] = nil, ["methods"] = nil, ["symbol"] = nil})
end
function Parser:parse_extern_decl()
    self:advance()
    local lib_t = self:expect(TokenKind.StringLiteral, "esperado nome da biblioteca (string)")
    local lib_name = lib_t.text
    -- native lua
    
 lib_name = lib_name : gsub ( "^\"" , "" ) : gsub ( "\"$" , "" ) 
 
    local methods = {}
    local mc = 0
    while (not self:is_at_end() and (self:peek().kind ~= TokenKind.KwEnd)) do
        if (self:peek().kind == TokenKind.KwFunc) then
            methods[mc] = self:parse_func_decl(true)
            mc = zt.add(mc, 1)
        else
            self:advance()
        end
    end
    self:expect(TokenKind.KwEnd, "esperado 'end' no bloco extern")
    return DeclNode.new({["kind"] = NodeKind.ExternDecl, ["name"] = lib_name, ["methods"] = methods, ["is_pub"] = true, ["attributes"] = nil, ["span"] = lib_t.span, ["params"] = nil, ["body"] = nil, ["fields"] = nil, ["members"] = nil, ["symbol"] = nil})
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
    return stmts
end
function Parser:parse_statement()
    local t = self:peek()
    if (t.kind == TokenKind.Semicolon) then
        self:advance()
        return nil
    end
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
    if (t.kind == TokenKind.KwNative) then
        return self:parse_native_lua_stmt()
    end
    if (t.kind == TokenKind.KwReturn) then
        self:advance()
        local ret_value = nil
        if ((((self:peek().kind ~= TokenKind.KwEnd) and (self:peek().kind ~= TokenKind.KwElse)) and (self:peek().kind ~= TokenKind.KwElif)) and (self:peek().kind ~= TokenKind.EOF)) then
            ret_value = self:parse_expression(0)
        end
        return StmtNode.new({["kind"] = NodeKind.ReturnStmt, ["value"] = ret_value, ["span"] = t.span, ["condition"] = nil, ["body"] = nil, ["target"] = nil, ["cases"] = nil, ["patterns"] = nil, ["variables"] = nil, ["iterable"] = nil, ["count_expr"] = nil})
    end
    local assign_stmt = self:try_parse_assign_stmt()
    if (assign_stmt ~= nil) then
        return assign_stmt
    end
    return StmtNode.new({["kind"] = NodeKind.ExprStmt, ["value"] = self:parse_expression(0), ["span"] = t.span, ["condition"] = nil, ["body"] = nil, ["target"] = nil, ["cases"] = nil, ["patterns"] = nil, ["variables"] = nil, ["iterable"] = nil, ["count_expr"] = nil})
end
function Parser:parse_assign_target()
    local start_t = self:peek()
    local target = nil
    if (start_t.kind == TokenKind.At) then
        self:advance()
        local f = self:expect_name("esperado campo")
        local self_node = ExprNode.new({["kind"] = NodeKind.SelfExpr, ["span"] = start_t.span, ["left"] = nil, ["right"] = nil, ["operator"] = nil, ["value"] = nil, ["name"] = "", ["callee"] = nil, ["args"] = nil, ["object"] = nil, ["index"] = nil, ["items"] = nil, ["keys"] = nil, ["values"] = nil, ["symbol"] = nil})
        target = ExprNode.new({["kind"] = NodeKind.MemberExpr, ["object"] = self_node, ["name"] = f.text, ["span"] = start_t.span, ["left"] = nil, ["right"] = nil, ["operator"] = nil, ["value"] = nil, ["callee"] = nil, ["args"] = nil, ["index"] = nil, ["items"] = nil, ["keys"] = nil, ["values"] = nil, ["symbol"] = nil})
    elseif (start_t.kind == TokenKind.Identifier) then
        local ident_t = self:advance()
        target = ExprNode.new({["kind"] = NodeKind.IdentifierExpr, ["name"] = ident_t.text, ["span"] = ident_t.span, ["left"] = nil, ["right"] = nil, ["operator"] = nil, ["value"] = nil, ["callee"] = nil, ["args"] = nil, ["object"] = nil, ["index"] = nil, ["items"] = nil, ["keys"] = nil, ["values"] = nil, ["symbol"] = nil})
    elseif (start_t.kind == TokenKind.KwSelf) then
        local self_t = self:advance()
        target = ExprNode.new({["kind"] = NodeKind.SelfExpr, ["span"] = self_t.span, ["left"] = nil, ["right"] = nil, ["operator"] = nil, ["value"] = nil, ["name"] = "", ["callee"] = nil, ["args"] = nil, ["object"] = nil, ["index"] = nil, ["items"] = nil, ["keys"] = nil, ["values"] = nil, ["symbol"] = nil})
    else
        return nil
    end
    while true do
        if (self:peek().kind == TokenKind.Dot) then
            self:advance()
            local member_t = self:expect_name("esperado membro")
            target = ExprNode.new({["kind"] = NodeKind.MemberExpr, ["object"] = target, ["name"] = member_t.text, ["span"] = member_t.span, ["left"] = nil, ["right"] = nil, ["operator"] = nil, ["value"] = nil, ["callee"] = nil, ["args"] = nil, ["index"] = nil, ["items"] = nil, ["keys"] = nil, ["values"] = nil, ["symbol"] = nil})
        elseif (self:peek().kind == TokenKind.LBracket) then
            local bracket_t = self:advance()
            local idx = self:parse_expression(0)
            self:expect(TokenKind.RBracket, "esperado ']'")
            target = ExprNode.new({["kind"] = NodeKind.IndexExpr, ["object"] = target, ["index"] = idx, ["span"] = bracket_t.span, ["left"] = nil, ["right"] = nil, ["operator"] = nil, ["value"] = nil, ["name"] = "", ["callee"] = nil, ["args"] = nil, ["items"] = nil, ["keys"] = nil, ["values"] = nil, ["symbol"] = nil})
        else
            break
        end
    end
    return target
end
function Parser:try_parse_assign_stmt()
    local start = self.pos
    local target = self:parse_assign_target()
    if (target == nil) then
        self.pos = start
        return nil
    end
    if (self:peek().kind ~= TokenKind.Equal) then
        self.pos = start
        return nil
    end
    self:advance()
    return StmtNode.new({["kind"] = NodeKind.AssignStmt, ["target"] = target, ["value"] = self:parse_expression(0), ["span"] = target.span, ["condition"] = nil, ["body"] = nil})
end
function Parser:parse_native_lua_stmt()
    local start_t = self:advance()
    local t_lua = self:expect(TokenKind.Identifier, "esperado 'lua' após 'native'")
    local code = ""
    local depth = 1
    local inline_body = (not self:is_at_end() and (self:peek().span.line == start_t.span.line))
    while not self:is_at_end() do
        local t = self:advance()
        local k = t.kind
        if inline_body then
            if zt.unwrap_or(zt.unwrap_or(zt.unwrap_or(zt.unwrap_or(zt.unwrap_or((k == TokenKind.KwIf), (k == TokenKind.KwDo)), (k == TokenKind.KwFunc)), (k == TokenKind.KwWhile)), (k == TokenKind.KwFor)), (k == TokenKind.KwRepeat)) then
                depth = zt.add(depth, 1)
            elseif (k == TokenKind.KwEnd) then
                depth = (depth - 1)
                if (depth == 0) then
                    break
                end
            end
        elseif ((k == TokenKind.KwEnd) and (t.span.column == start_t.span.column)) then
            break
        end
        if (code ~= "") then
            code = zt.add(code, " ")
        end
        code = zt.add(code, t.text)
    end
    return StmtNode.new({["kind"] = NodeKind.NativeLuaStmt, ["lua_code"] = code, ["span"] = start_t.span})
end
function Parser:parse_elif_tail()
    local t = self:advance()
    local cond = self:parse_expression(0)
    local inline_body = (not self:is_at_end() and (self:peek().span.line == t.span.line))
    local body = {}
    if inline_body then
        body = self:parse_inline_statement_block(t.span.line)
    else
        body = self:parse_block()
    end
    local else_body = nil
    if (self:peek().kind == TokenKind.KwElif) then
        else_body = {}
        else_body[0] = self:parse_elif_tail()
    elseif (self:peek().kind == TokenKind.KwElse) then
        local else_t = self:advance()
        local else_inline = (not self:is_at_end() and (self:peek().span.line == else_t.span.line))
        if else_inline then
            else_body = self:parse_inline_statement_block(else_t.span.line)
        else
            else_body = self:parse_block()
        end
    end
    return StmtNode.new({["kind"] = NodeKind.IfStmt, ["condition"] = cond, ["body"] = body, ["else_body"] = else_body, ["span"] = t.span})
end
function Parser:parse_if_stmt()
    local t = self:advance()
    local cond = self:parse_expression(0)
    local inline_body = (not self:is_at_end() and (self:peek().span.line == t.span.line))
    local body = {}
    if inline_body then
        body = self:parse_inline_statement_block(t.span.line)
    else
        body = self:parse_block()
    end
    local else_body = nil
    if (self:peek().kind == TokenKind.KwElif) then
        else_body = {}
        else_body[0] = self:parse_elif_tail()
        self:expect(TokenKind.KwEnd, "esperado end apos if")
    elseif (self:peek().kind == TokenKind.KwElse) then
        local else_t = self:advance()
        local else_inline = (not self:is_at_end() and (self:peek().span.line == else_t.span.line))
        if else_inline then
            else_body = self:parse_inline_statement_block(else_t.span.line)
        else
            else_body = self:parse_block()
        end
        self:expect(TokenKind.KwEnd, "esperado end apos else")
    else
        self:expect(TokenKind.KwEnd, "esperado end apos if")
    end
    return StmtNode.new({["kind"] = NodeKind.IfStmt, ["condition"] = cond, ["body"] = body, ["else_body"] = else_body, ["span"] = t.span})
end
function Parser:parse_while_stmt()
    local t = self:advance()
    local cond = self:parse_expression(0)
    local body = self:parse_block()
    self:expect(TokenKind.KwEnd, "esperado 'end' após while")
    return StmtNode.new({["kind"] = NodeKind.WhileStmt, ["condition"] = cond, ["body"] = body, ["span"] = t.span})
end
function Parser:parse_for_stmt()
    local t = self:advance()
    local vn_t = self:expect_name("esperado variável no for")
    local var_name = vn_t.text
    self:expect(TokenKind.KwIn, "esperado 'in'")
    local iter = self:parse_expression(0)
    local body = self:parse_block()
    self:expect(TokenKind.KwEnd, "esperado 'end' após for")
    local vars = {}
    vars[0] = var_name
    return StmtNode.new({["kind"] = NodeKind.ForInStmt, ["variables"] = vars, ["iterable"] = iter, ["body"] = body, ["span"] = t.span})
end
function Parser:parse_repeat_stmt()
    local t = self:advance()
    local n = self:parse_expression(0)
    self:expect(TokenKind.Identifier, "esperado 'times'")
    local body = self:parse_block()
    self:expect(TokenKind.KwEnd, "esperado 'end' após repeat")
    return StmtNode.new({["kind"] = NodeKind.RepeatStmt, ["count_expr"] = n, ["body"] = body, ["span"] = t.span})
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
function Parser:parse_inline_statement_block(line_no)
    local stmts = {}
    if self:is_at_end() then
        return stmts
    end
    if (self:peek().span.line ~= line_no) then
        return stmts
    end
    if zt.unwrap_or(zt.unwrap_or((self:peek().kind == TokenKind.KwEnd), (self:peek().kind == TokenKind.KwElse)), (self:peek().kind == TokenKind.KwElif)) then
        return stmts
    end
    local s = self:parse_statement()
    if (s ~= nil) then
        stmts[0] = s
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
        if (t.text == "null") then
            node = ExprNode.new({["kind"] = NodeKind.LiteralExpr, ["value"] = "nil", ["span"] = t.span, ["left"] = nil, ["right"] = nil, ["operator"] = nil, ["name"] = "", ["callee"] = nil, ["args"] = nil, ["object"] = nil, ["index"] = nil, ["symbol"] = nil})
        elseif zt.unwrap_or((t.text == "true"), (t.text == "false")) then
            node = ExprNode.new({["kind"] = NodeKind.LiteralExpr, ["value"] = t.text, ["span"] = t.span, ["left"] = nil, ["right"] = nil, ["operator"] = nil, ["name"] = "", ["callee"] = nil, ["args"] = nil, ["object"] = nil, ["index"] = nil, ["symbol"] = nil})
        elseif (self:peek().kind == TokenKind.LBrace) then
            self:advance()
            local keys = {}
            local vals = {}
            local sc = 0
            if (self:peek().kind ~= TokenKind.RBrace) then
                while true do
                    keys[sc] = self:expect_name("esperado campo").text
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
    elseif self:is_name_token(t.kind) then
        node = ExprNode.new({["kind"] = NodeKind.IdentifierExpr, ["name"] = t.text, ["span"] = t.span, ["left"] = nil, ["right"] = nil, ["operator"] = nil, ["value"] = nil, ["callee"] = nil, ["args"] = nil, ["object"] = nil, ["index"] = nil, ["items"] = nil, ["keys"] = nil, ["values"] = nil, ["symbol"] = nil})
    elseif (t.kind == TokenKind.LParen) then
        node = self:parse_expression(0)
        self:expect(TokenKind.RParen, "esperado ')' após expressão")
    elseif (t.kind == TokenKind.At) then
        local f = self:expect_name("esperado nome do campo após '@'")
        local self_node = ExprNode.new({["kind"] = NodeKind.SelfExpr, ["span"] = t.span, ["left"] = nil, ["right"] = nil, ["operator"] = nil, ["value"] = nil, ["name"] = "", ["callee"] = nil, ["args"] = nil, ["object"] = nil, ["index"] = nil, ["items"] = nil, ["keys"] = nil, ["values"] = nil, ["symbol"] = nil})
        node = ExprNode.new({["kind"] = NodeKind.MemberExpr, ["object"] = self_node, ["name"] = f.text, ["span"] = t.span, ["left"] = nil, ["right"] = nil, ["operator"] = nil, ["value"] = nil, ["callee"] = nil, ["args"] = nil, ["index"] = nil, ["items"] = nil, ["keys"] = nil, ["values"] = nil, ["symbol"] = nil})
    elseif (t.kind == TokenKind.LBracket) then
        node = self:parse_list_literal(t)
    elseif (t.kind == TokenKind.LBrace) then
        node = self:parse_map_literal(t)
    elseif zt.unwrap_or((t.kind == TokenKind.KwNot), (t.kind == TokenKind.Minus)) then
        node = ExprNode.new({["kind"] = NodeKind.UnaryExpr, ["operator"] = t, ["right"] = self:parse_expression(9), ["span"] = t.span, ["left"] = nil, ["value"] = nil, ["name"] = "", ["callee"] = nil, ["args"] = nil, ["object"] = nil, ["index"] = nil, ["symbol"] = nil, ["lua_code"] = ""})
    elseif (t.kind == TokenKind.KwNative) then
        local t_lua = self:expect(TokenKind.Identifier, "esperado 'lua' após 'native'")
        local code = ""
        local depth = 1
        while not self:is_at_end() do
            local nt = self:advance()
            local nk = nt.kind
            if zt.unwrap_or(zt.unwrap_or(zt.unwrap_or(zt.unwrap_or(zt.unwrap_or((nk == TokenKind.KwIf), (nk == TokenKind.KwDo)), (nk == TokenKind.KwFunc)), (nk == TokenKind.KwWhile)), (nk == TokenKind.KwFor)), (nk == TokenKind.KwRepeat)) then
                depth = zt.add(depth, 1)
            elseif (nk == TokenKind.KwEnd) then
                depth = (depth - 1)
                if (depth == 0) then
                    break
                end
            end
            if (code ~= "") then
                code = zt.add(code, " ")
            end
            code = zt.add(code, nt.text)
        end
        node = ExprNode.new({["kind"] = NodeKind.NativeLuaExpr, ["lua_code"] = code, ["span"] = t.span, ["left"] = nil, ["right"] = nil, ["operator"] = nil, ["value"] = nil, ["name"] = "", ["callee"] = nil, ["args"] = nil, ["object"] = nil, ["index"] = nil, ["symbol"] = nil})
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
            local m = self:expect_name("esperado membro")
            node = ExprNode.new({["kind"] = NodeKind.MemberExpr, ["object"] = node, ["name"] = m.text, ["span"] = t.span, ["left"] = nil, ["right"] = nil, ["operator"] = nil, ["value"] = nil, ["callee"] = nil, ["args"] = nil, ["index"] = nil, ["symbol"] = nil})
        elseif (next.kind == TokenKind.LBracket) then
            self:advance()
            local idx = self:parse_expression(0)
            self:expect(TokenKind.RBracket, "esperado ']'")
            node = ExprNode.new({["kind"] = NodeKind.IndexExpr, ["object"] = node, ["index"] = idx, ["span"] = t.span, ["left"] = nil, ["right"] = nil, ["operator"] = nil, ["value"] = nil, ["name"] = "", ["callee"] = nil, ["args"] = nil, ["symbol"] = nil})
        elseif (next.kind == TokenKind.KwAs) then
            self:advance()
            self:parse_type()
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
            local p = self:expect_name("esperado parâmetro")
            if (self:peek().kind == TokenKind.Colon) then
                self:advance()
                self:parse_type()
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
    return zt.index_any(self.tokens, self.pos)
end
function Parser:is_at_end()
    return (zt.index_any(self.tokens, self.pos).kind == TokenKind.EOF)
end
function Parser:advance()
    local t = zt.index_any(self.tokens, self.pos)
    self.pos = zt.add(self.pos, 1)
    return t
end
function Parser:expect(k, m)
    local t = self:peek()
    if (t.kind == k) then
        return self:advance()
    end
    if (self.diagnostics ~= nil) then
        self.diagnostics:report_error("ZT-1001", m, t.span)
    end
    self:advance()
    return t
end
function Scope:define(sym)
    if scope_has_local_symbol(self, sym.name) then
        return false
    end
    self.symbols[sym.name] = sym
    return true
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
    if (zt.index_any(self.cache, module_name) ~= nil) then
        return zt.index_any(self.cache, module_name)
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
    if has_slot(imps, 0) then
        local i = 0
        while has_slot(imps, i) do
            local imp = zt.index_any(imps, i)
            local module_ast = self.module_manager:get_ast(imp.body)
            if (module_ast ~= nil) then
                local target_scope = zt.index_any(self.module_scopes, imp.body)
                if (target_scope == nil) then
                    local mod_binder = Binder.new({["current_scope"] = Scope.new({["parent"] = nil, ["symbols"] = {}}), ["module_manager"] = self.module_manager, ["module_scopes"] = self.module_scopes})
                    mod_binder:bind_unit(module_ast)
                    target_scope = mod_binder.current_scope
                    self.module_scopes[imp.body] = target_scope
                end
                local sym = Symbol.new({["kind"] = 4, ["name"] = imp.name, ["type_name"] = "module", ["is_pub"] = false, ["declaration"] = imp, ["module_scope"] = target_scope})
                local ok = self.current_scope:define(sym)
                if (not ok and (self.diagnostics ~= nil)) then
                    self.diagnostics:report_error("ZT-2002", zt.add(zt.add("Símbolo '", imp.name), "' já definido neste escopo"), imp.span)
                end
            end
            i = zt.add(i, 1)
        end
    end
    local decls = u.declarations
    if has_slot(decls, 0) then
        local i = 0
        while has_slot(decls, i) do
            if self:_is_platform_allowed(zt.index_any(decls, i)) then
                self:_declare(zt.index_any(decls, i))
            end
            i = zt.add(i, 1)
        end
        i = 0
        while has_slot(decls, i) do
            if self:_is_platform_allowed(zt.index_any(decls, i)) then
                self:_bind_node(zt.index_any(decls, i))
            end
            i = zt.add(i, 1)
        end
    end
end
function Binder:_is_platform_allowed(n)
    if zt.unwrap_or((n == nil), (n.attributes == nil)) then
        return true
    end
    local has_platform_attr = false
    local allowed = false
    local i = 0
    while has_slot(n.attributes, i) do
        local a = zt.index_any(n.attributes, i)
        if zt.unwrap_or(zt.unwrap_or((a == "windows"), (a == "linux")), (a == "macos")) then
            has_platform_attr = true
            if (a == self.target_os) then
                allowed = true
            end
        end
        i = zt.add(i, 1)
    end
    if not has_platform_attr then
        return true
    end
    return allowed
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
        local ok = self.current_scope:define(sym)
        if (not ok and (self.diagnostics ~= nil)) then
            self.diagnostics:report_error("ZT-2002", zt.add(zt.add("Símbolo '", n.name), "' já definido neste escopo"), n.span)
        end
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
        if has_slot(ps, 0) then
            local i = 0
            while has_slot(ps, i) do
                local sym = Symbol.new({["kind"] = 3, ["name"] = zt.index_any(ps, i), ["type_name"] = "any", ["is_pub"] = false, ["declaration"] = n})
                self.current_scope:define(sym)
                i = zt.add(i, 1)
            end
        end
        local body = n.body
        if has_slot(body, 0) then
            local i = 0
            while has_slot(body, i) do
                self:_bind_node(zt.index_any(body, i))
                i = zt.add(i, 1)
            end
        end
        self.current_scope = prev
    elseif (n.kind == NodeKind.VarDecl) then
        self:_bind_node(n.body)
        if not scope_has_local_symbol(self.current_scope, n.name) then
            local sym = Symbol.new({["kind"] = 1, ["name"] = n.name, ["type_name"] = "any", ["is_pub"] = n.is_pub, ["declaration"] = n})
            self.current_scope:define(sym)
        end
    elseif (n.kind == NodeKind.IdentifierExpr) then
        n.symbol = scope_lookup(self.current_scope, n.name)
        if (symbol_is_missing(n.symbol) and (self.diagnostics ~= nil)) then
            self.diagnostics:report_error("ZT-2001", zt.add(zt.add("Símbolo não encontrado: '", n.name), "'"), n.span)
        end
    elseif (n.kind == NodeKind.BinaryExpr) then
        self:_bind_node(n.left)
        self:_bind_node(n.right)
    elseif (n.kind == NodeKind.ReturnStmt) then
        self:_bind_node(n.value)
    elseif (n.kind == NodeKind.ExternDecl) then
        local ms = n.methods
        if has_slot(ms, 0) then
            local i = 0
            while has_slot(ms, i) do
                local m = zt.index_any(ms, i)
                local sym = Symbol.new({["kind"] = 0, ["name"] = m.name, ["type_name"] = "any", ["is_pub"] = true, ["declaration"] = m})
                self.current_scope:define(sym)
                i = zt.add(i, 1)
            end
        end
    elseif (n.kind == NodeKind.ExprStmt) then
        self:_bind_node(n.value)
    elseif (n.kind == NodeKind.AssignStmt) then
        self:_bind_node(n.target)
        self:_bind_node(n.value)
    elseif (n.kind == NodeKind.ForInStmt) then
        self:_bind_node(n.iterable)
        local fb = n.body
        if has_slot(fb, 0) then
            local i = 0
            while has_slot(fb, i) do
                self:_bind_node(zt.index_any(fb, i))
                i = zt.add(i, 1)
            end
        end
    elseif (n.kind == NodeKind.RepeatStmt) then
        self:_bind_node(n.count_expr)
        local rb = n.body
        if has_slot(rb, 0) then
            local i = 0
            while has_slot(rb, i) do
                self:_bind_node(zt.index_any(rb, i))
                i = zt.add(i, 1)
            end
        end
    elseif (n.kind == NodeKind.MatchStmt) then
        self:_bind_node(n.condition)
        local mcs = n.cases
        if has_slot(mcs, 0) then
            local i = 0
            while has_slot(mcs, i) do
                local c = zt.index_any(mcs, i)
                local ps = c.patterns
                if has_slot(ps, 0) then
                    local k = 0
                    while has_slot(ps, k) do
                        self:_bind_node(zt.index_any(ps, k))
                        k = zt.add(k, 1)
                    end
                end
                local cb = c.body
                if has_slot(cb, 0) then
                    local j = 0
                    while has_slot(cb, j) do
                        self:_bind_node(zt.index_any(cb, j))
                        j = zt.add(j, 1)
                    end
                end
                i = zt.add(i, 1)
            end
        end
        if has_slot(n.else_body, 0) then
            local i = 0
            while has_slot(n.else_body, i) do
                self:_bind_node(zt.index_any(n.else_body, i))
                i = zt.add(i, 1)
            end
        end
    elseif (n.kind == NodeKind.MemberExpr) then
        self:_bind_node(n.object)
        if ((n.object.kind == NodeKind.IdentifierExpr) and symbol_is_present(n.object.symbol)) then
            local sym = n.object.symbol
            if (sym.kind == 4) then
                local mod_scope = sym.module_scope
                n.symbol = scope_lookup(mod_scope, n.name)
                if (symbol_is_missing(n.symbol) and (self.diagnostics ~= nil)) then
                    self.diagnostics:report_error("ZT-2001", zt.add(zt.add(zt.add(zt.add("Símbolo '", n.name), "' não encontrado no módulo '"), n.object.name), "'"), n.span)
                end
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
function Emitter:_zenith_to_c_type(zt_type)
    if (zt_type == "int") then
        return "int"
    end
    if (zt_type == "text") then
        return "const char*"
    end
    if (zt_type == "bool") then
        return "bool"
    end
    if (zt_type == "any") then
        return "void*"
    end
    return zt_type
end
function Emitter:indent()
    self.indent_level = zt.add(self.indent_level, 1)
end
function Emitter:dedent()
    self.indent_level = (self.indent_level - 1)
end
function Emitter:generate(u)
    self.output = {}
    self.line_count = 0
    self.indent_level = 0
    self.has_namespace = ((u.namespace ~= nil) and (u.namespace ~= ""))
    self:emit("-- Transpilado por Zenith Ascension (Transcendencia v0.10)")
    self:emit("local zt = require(\"src.backend.lua.runtime.zenith_rt\")")
    local imps = u.imports
    if has_slot(imps, 0) then
        local i = 0
        while has_slot(imps, i) do
            local imp = zt.index_any(imps, i)
            local lua_path = imp.body
            -- native lua
            
 if lua_path : sub ( 1 , 4 ) == "std." then 
 lua_path = "src.stdlib." .. lua_path : sub ( 5 ) 
 end 
 
            local clean_name = imp.name
            -- native lua
            
 clean_name = clean_name : gsub ( "%." , "_" ) 
 
            self:emit(zt.add(zt.add(zt.add(zt.add("local ", clean_name), " = require(\""), lua_path), "\")"))
            i = zt.add(i, 1)
        end
    end
    self:emit("")
    local decls = u.declarations
    if has_slot(decls, 0) then
        local i = 0
        while has_slot(decls, i) do
            if self:_is_platform_allowed(zt.index_any(decls, i)) then
                self:_bind_node_emit(zt.index_any(decls, i))
            end
            i = zt.add(i, 1)
        end
    end
    if not self.has_namespace then
        local found_main = false
        local i = 0
        if has_slot(decls, 0) then
            while has_slot(decls, i) do
                local d = zt.index_any(decls, i)
                if (((d.kind == NodeKind.FuncDecl) and (d.name == "main")) and self:_is_platform_allowed(d)) then
                    found_main = true
                end
                i = zt.add(i, 1)
            end
        end
        if found_main then
            self:emit("")
            self:emit("-- Auto-run main")
            self:emit("local _s = main()")
            self:emit("if type(_s) == 'table' and _s.co then zt.drive(_s) end")
        end
    end
    self:emit("")
    self:emit(zt.add("return ", lbrace_text()))
    self:indent()
    if has_slot(decls, 0) then
        local i = 0
        while has_slot(decls, i) do
            local d = zt.index_any(decls, i)
            if (d.is_pub and self:_is_platform_allowed(d)) then
                self:emit(zt.add(zt.add(zt.add(d.name, " = "), d.name), ","))
            end
            i = zt.add(i, 1)
        end
    end
    self:dedent()
    self:emit(rbrace_text())
    local res = ""
    local j = 0
    while (self.line_count > j) do
        res = zt.add(zt.add(res, zt.index_any(self.output, j)), "\
")
        j = zt.add(j, 1)
    end
    return res
end
function Emitter:_is_platform_allowed(n)
    if zt.unwrap_or((n == nil), (n.attributes == nil)) then
        return true
    end
    local has_platform_attr = false
    local allowed = false
    local j = 0
    while has_slot(n.attributes, j) do
        local a = zt.index_any(n.attributes, j)
        if zt.unwrap_or(zt.unwrap_or((a == "windows"), (a == "linux")), (a == "macos")) then
            has_platform_attr = true
            if (a == self.target_os) then
                allowed = true
            end
        end
        j = zt.add(j, 1)
    end
    if not has_platform_attr then
        return true
    end
    return allowed
end
function Emitter:_bind_node_emit(n)
    if (n == nil) then
        return
    end
    if (n.kind == NodeKind.FuncDecl) then
        local h = zt.add(zt.add("function ", n.name), "(")
        local ps = n.params
        if has_slot(ps, 0) then
            local i = 0
            while has_slot(ps, i) do
                h = zt.add(h, zt.index_any(ps, i))
                if has_slot(ps, zt.add(i, 1)) then
                    h = zt.add(h, ", ")
                end
                i = zt.add(i, 1)
            end
        end
        self:emit(zt.add(h, ")"))
        self:indent()
        local body = n.body
        if has_slot(body, 0) then
            local i = 0
            while has_slot(body, i) do
                self:_bind_node_emit(zt.index_any(body, i))
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
        if has_slot(body, 0) then
            local i = 0
            while has_slot(body, i) do
                self:_bind_node_emit(zt.index_any(body, i))
                i = zt.add(i, 1)
            end
        end
        self:dedent()
        if has_slot(n.else_body, 0) then
            self:emit("else")
            self:indent()
            local eb = n.else_body
            local i = 0
            while has_slot(eb, i) do
                self:_bind_node_emit(zt.index_any(eb, i))
                i = zt.add(i, 1)
            end
            self:dedent()
        end
        self:emit("end")
    elseif (n.kind == NodeKind.WhileStmt) then
        self:emit(zt.add(zt.add("while (", self:expr_to_lua(n.condition)), ") do"))
        self:indent()
        local body = n.body
        if has_slot(body, 0) then
            local i = 0
            while has_slot(body, i) do
                self:_bind_node_emit(zt.index_any(body, i))
                i = zt.add(i, 1)
            end
        end
        self:dedent()
        self:emit("end")
    elseif (n.kind == NodeKind.ForInStmt) then
        local vars = n.variables
        local v_str = zt.index_any(vars, 0)
        self:emit(zt.add(zt.add(zt.add(zt.add("for _, ", v_str), " in zt.iter("), self:expr_to_lua(n.iterable)), ") do"))
        self:indent()
        local body = n.body
        if has_slot(body, 0) then
            local i = 0
            while has_slot(body, i) do
                self:_bind_node_emit(zt.index_any(body, i))
                i = zt.add(i, 1)
            end
        end
        self:dedent()
        self:emit("end")
    elseif (n.kind == NodeKind.RepeatStmt) then
        self:emit(zt.add(zt.add("for _ = 1, ", self:expr_to_lua(n.count_expr)), " do"))
        self:indent()
        local body = n.body
        if has_slot(body, 0) then
            local i = 0
            while has_slot(body, i) do
                self:_bind_node_emit(zt.index_any(body, i))
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
        while has_slot(cases, i) do
            local c = zt.index_any(cases, i)
            local cond = ""
            local ps = c.patterns
            local j = 0
            while has_slot(ps, j) do
                local p_str = self:expr_to_lua(zt.index_any(ps, j))
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
            while has_slot(cbody, k) do
                self:_bind_node_emit(zt.index_any(cbody, k))
                k = zt.add(k, 1)
            end
            self:dedent()
            i = zt.add(i, 1)
        end
        if has_slot(n.else_body, 0) then
            self:emit("else")
            self:indent()
            local ebody = n.else_body
            local k = 0
            while has_slot(ebody, k) do
                self:_bind_node_emit(zt.index_any(ebody, k))
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
        self:emit(zt.add(zt.add(zt.add(zt.add("local ", n.name), " = "), lbrace_text()), rbrace_text()))
        self:emit(zt.add(zt.add(n.name, ".__index = "), n.name))
        self:emit(zt.add(zt.add("function ", n.name), ".new(fields)"))
        self:indent()
        self:emit(zt.add(zt.add(zt.add(zt.add(zt.add("local self = setmetatable(", lbrace_text()), rbrace_text()), ", "), n.name), ")"))
        local fs = n.fields
        if has_slot(fs, 0) then
            local i = 0
            while has_slot(fs, i) do
                local f = zt.index_any(fs, i)
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
        if has_slot(ms, 0) then
            local i = 0
            while has_slot(ms, i) do
                self:_emit_method(zt.index_any(ms, i), n.name)
                i = zt.add(i, 1)
            end
        end
    elseif (n.kind == NodeKind.EnumDecl) then
        self:emit(zt.add(zt.add(zt.add("local ", n.name), " = "), lbrace_text()))
        self:indent()
        local mems = n.members
        if has_slot(mems, 0) then
            local i = 0
            while has_slot(mems, i) do
                self:emit(zt.add(zt.add(zt.add(zt.index_any(mems, i), " = \""), zt.index_any(mems, i)), "\","))
                i = zt.add(i, 1)
            end
        end
        self:dedent()
        self:emit(rbrace_text())
    elseif (n.kind == NodeKind.ExternDecl) then
        self:emit("local ffi = require(\"ffi\")")
        local cdef = "ffi.cdef[["
        local methods = n.methods
        if has_slot(methods, 0) then
            local i = 0
            while has_slot(methods, i) do
                local m = zt.index_any(methods, i)
                local ret = "void"
                cdef = zt.add(zt.add(zt.add(cdef, "\
    int "), m.name), "(")
                local params = m.params
                if has_slot(params, 0) then
                    local j = 0
                    while has_slot(params, j) do
                        cdef = zt.add(zt.add(cdef, "void* "), zt.index_any(params, j))
                        if has_slot(params, zt.add(j, 1)) then
                            cdef = zt.add(cdef, ", ")
                        end
                        j = zt.add(j, 1)
                    end
                end
                cdef = zt.add(cdef, ");")
                i = zt.add(i, 1)
            end
        end
        cdef = zt.add(cdef, "\
]]")
        self:emit(cdef)
        local clean_lib = n.name
        -- native lua
        
 clean_lib = clean_lib : gsub ( "[^%a]" , "_" ) 
 
        self:emit(zt.add(zt.add(zt.add(zt.add("local _lib_", clean_lib), " = ffi.load(\""), n.name), "\")"))
        local i = 0
        while has_slot(methods, i) do
            local m = zt.index_any(methods, i)
            self:emit(zt.add(zt.add(zt.add(zt.add(zt.add("local ", m.name), " = _lib_"), clean_lib), "."), m.name))
            i = zt.add(i, 1)
        end
    elseif (n.kind == NodeKind.NativeLuaStmt) then
        self:emit("-- native lua")
        self:emit(n.lua_code)
    end
end
function Emitter:_emit_method(n, struct_name)
    local h = zt.add(zt.add(zt.add(zt.add("function ", struct_name), ":"), n.name), "(")
    local ps = n.params
    if has_slot(ps, 0) then
        local i = 0
        local wrote = false
        while has_slot(ps, i) do
            if not (((i == 0) and (zt.index_any(ps, i) == "self"))) then
                if wrote then
                    h = zt.add(h, ", ")
                end
                h = zt.add(h, zt.index_any(ps, i))
                wrote = true
            end
            i = zt.add(i, 1)
        end
    end
    self:emit(zt.add(h, ")"))
    self:indent()
    local body = n.body
    if has_slot(body, 0) then
        local i = 0
        while has_slot(body, i) do
            self:_bind_node_emit(zt.index_any(body, i))
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
        local name = n.name
        -- native lua
        name = name : gsub ( "%." , "_" ) 
        return name
    end
    if (n.kind == NodeKind.BinaryExpr) then
        local l = self:expr_to_lua(n.left)
        local r = self:expr_to_lua(n.right)
        local folded = nil
        -- native lua
        
 local nl = tonumber ( l ) 
 local nr = tonumber ( r ) 
 if nl and nr then 
 local k = n . operator . kind 
 if k == 7 then folded = tostring ( nl + nr ) end 
 if k == 8 then folded = tostring ( nl - nr ) end 
 if k == 9 then folded = tostring ( nl * nr ) end 
 if k == 10 then folded = tostring ( nl / nr ) end 
 end 
 
        if (folded ~= nil) then
            return folded
        end
        local op = n.operator.text
        if (n.operator.kind == TokenKind.Plus) then
            return zt.add(zt.add(zt.add(zt.add("zt.add(", l), ", "), r), ")")
        end
        if (n.operator.kind == TokenKind.BangEqual) then
            op = "~="
        end
        if (n.operator.kind == TokenKind.KwAnd) then
            op = "and"
        end
        if (n.operator.kind == TokenKind.KwOr) then
            op = "or"
        end
        return zt.add(zt.add(zt.add(zt.add(zt.add(zt.add("(", l), " "), op), " "), r), ")")
    elseif (n.kind == NodeKind.CallExpr) then
        local c = self:expr_to_lua(n.callee)
        if (n.callee.kind == NodeKind.MemberExpr) then
            c = zt.add(zt.add(self:expr_to_lua(n.callee.object), ":"), n.callee.name)
        end
        local a = "("
        local args = n.args
        if has_slot(args, 0) then
            local i = 0
            while has_slot(args, i) do
                a = zt.add(a, self:expr_to_lua(zt.index_any(args, i)))
                if has_slot(args, zt.add(i, 1)) then
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
        local res = lbrace_text()
        local items = n.items
        if has_slot(items, 0) then
            local i = 0
            while has_slot(items, i) do
                res = zt.add(res, self:expr_to_lua(zt.index_any(items, i)))
                if has_slot(items, zt.add(i, 1)) then
                    res = zt.add(res, ", ")
                end
                i = zt.add(i, 1)
            end
        end
        return zt.add(res, rbrace_text())
    elseif (n.kind == NodeKind.MapLiteral) then
        local res = lbrace_text()
        local keys = n.keys
        local vals = n.values
        if has_slot(keys, 0) then
            local i = 0
            while has_slot(keys, i) do
                res = zt.add(zt.add(zt.add(zt.add(res, "["), self:expr_to_lua(zt.index_any(keys, i))), "] = "), self:expr_to_lua(zt.index_any(vals, i)))
                if has_slot(keys, zt.add(i, 1)) then
                    res = zt.add(res, ", ")
                end
                i = zt.add(i, 1)
            end
        end
        return zt.add(res, rbrace_text())
    elseif (n.kind == NodeKind.StructInitExpr) then
        local res = zt.add(zt.add(n.name, ".new("), lbrace_text())
        local keys = n.keys
        local vals = n.values
        if has_slot(keys, 0) then
            local i = 0
            while has_slot(keys, i) do
                res = zt.add(zt.add(zt.add(zt.add(res, "[\""), zt.index_any(keys, i)), "\"] = "), self:expr_to_lua(zt.index_any(vals, i)))
                if has_slot(keys, zt.add(i, 1)) then
                    res = zt.add(res, ", ")
                end
                i = zt.add(i, 1)
            end
        end
        return zt.add(zt.add(res, rbrace_text()), ")")
    elseif (n.kind == NodeKind.SelfExpr) then
        return "self"
    elseif (n.kind == NodeKind.NativeLuaExpr) then
        return zt.add(zt.add("(", n.lua_code), ")")
    end
    return "nil"
end
-- native lua

 local function split_lines ( text ) 
 local lines = { } 
 for line in text : gmatch ( "([^\n]*)\n?" ) do 
 table . insert ( lines , line ) 
 end 
 return lines 
 end 
 
 local function print_diag ( bag , source ) 
 local lines = source and split_lines ( source ) or { } 
 for i = 0 , ( bag . count or 0 ) - 1 do 
 local d = bag . diagnostics [ i ] 
 local sp = d . span 
 local color = d . level == 0 and "\27[31m" or "\27[33m" 
 local reset = "\27[0m" 
 
 io . stderr : write ( string . format ( "%s[%s] %s%s\n" , color , d . id , d . message , reset ) ) 
 
 if sp then 
 io . stderr : write ( string . format ( "  at %s:%d:%d\n" , sp . file or "main.zt" , sp . line , sp . column ) ) 
 local line_text = lines [ sp . line ] 
 if line_text then 
 io . stderr : write ( string . format ( "   |\n %d | %s\n   | %s^\n" , sp . line , line_text , string . rep ( " " , sp . column - 1 ) ) ) 
 end 
 end 
 io . stderr : write ( "\n" ) 
 end 
 end 
 
 if arg and # arg > 0 then 
 local first = arg [ 1 ] 
 local is_build = ( first == "build" ) 
 local is_bundle = false 
 local src_file = nil 
 
 if is_build then 
 if arg [ 2 ] == "--bundle" then 
 is_bundle = true 
 src_file = arg [ 3 ] 
 else 
 src_file = arg [ 2 ] 
 end 
 else 
 src_file = first 
 end 
 
 if not src_file then 
 io . stderr : write ( "Uso: zt <arquivo.zt> ou zt build [--bundle] <arquivo.zt>\n" ) 
 os . exit ( 1 ) 
 end 
 
 local out_name = src_file : gsub ( "%.zt$" , "" ) .. ".lua" 
 if is_bundle then out_name = "dist/main.lua" end 
 
 
 local host_os = "windows" 
 local sep = package . config : sub ( 1 , 1 ) 
 if sep == "/" then host_os = "linux" end 
 
 local f = io . open ( src_file , "r" ) 
 if f then 
 local cnt = f : read ( "*a" ) : gsub ( "\r" , "" ) 
 f : close ( ) 
 local res = compile_ext ( cnt , src_file , host_os ) 
 if type ( res ) == "string" then 
 
 if is_bundle then 
 if host_os == "windows" then os . execute ( "mkdir dist >nul 2>nul" ) 
 else os . execute ( "mkdir -p dist" ) end 
 end 
 
 local out_f = io . open ( out_name , "w" ) 
 if out_f then out_f : write ( res ) out_f : close ( ) end 
 
 local msg = "✅ Sucesso (Target: " .. host_os .. "): " .. src_file 
 if is_bundle then msg = msg .. " -> " .. out_name end 
 io . stderr : write ( msg .. "\n" ) 
 else 
 if type ( res ) == "table" and res . diagnostics then 
 print_diag ( res , cnt ) 
 else 
 io . stderr : write ( "Erro fatal: " .. tostring ( res ) .. "\n" ) 
 end 
 os . exit ( 1 ) 
 end 
 else 
 io . stderr : write ( "Erro: Não foi possível abrir o arquivo " .. src_file .. "\n" ) 
 os . exit ( 1 ) 
 end 
 end 
 


return {
    TokenKind = TokenKind,
    NodeKind = NodeKind,
    Span = Span,
    Diagnostic = Diagnostic,
    DiagnosticBag = DiagnosticBag,
    Symbol = Symbol,
    CompilationUnit = CompilationUnit,
    DeclNode = DeclNode,
    StmtNode = StmtNode,
    ExprNode = ExprNode,
    Token = Token,
    Lexer = Lexer,
    Parser = Parser,
    scope_lookup = scope_lookup,
    symbol_is_present = symbol_is_present,
    symbol_is_missing = symbol_is_missing,
    scope_has_local_symbol = scope_has_local_symbol,
    Scope = Scope,
    ModuleManager = ModuleManager,
    Binder = Binder,
    Emitter = Emitter,
    compile_ext = compile_ext,
    compile = compile,
}