-- ============================================================================
-- Zenith Compiler — Lexer
-- Tokeniza o texto-fonte em uma lista de tokens.
-- ============================================================================

local TokenKind    = require("src.syntax.tokens.token_kind")
local Token        = require("src.syntax.tokens.token")
local KeywordTable = require("src.syntax.tokens.keyword_table")
local Span         = require("src.source.span")
local DiagnosticBag = require("src.diagnostics.diagnostic_bag")

local Lexer = {}
Lexer.__index = Lexer

function Lexer.new(source_text)
    local self = setmetatable({}, Lexer)
    self.source = source_text
    self.text = source_text.text
    self.pos = 1
    self.diagnostics = DiagnosticBag.new()
    self.buffer = {} 

    -- Tabelas de operadores locais à instância usando string.char para evitar bugs de encoding
    self.TWO_CHAR = {
        ["=="] = TokenKind.EQUAL_EQUAL, ["!="] = TokenKind.BANG_EQUAL,
        ["<="] = TokenKind.LESS_EQUAL, [">="] = TokenKind.GREATER_EQUAL,
        ["+="] = TokenKind.PLUS_EQUAL, ["-="] = TokenKind.MINUS_EQUAL,
        ["*="] = TokenKind.STAR_EQUAL, ["/="] = TokenKind.SLASH_EQUAL,
        ["->"] = TokenKind.ARROW, ["=>"] = TokenKind.FAT_ARROW, [".."] = TokenKind.DOT_DOT
    }

    self.SINGLE_CHAR = {
        ["+"] = TokenKind.PLUS, ["-"] = TokenKind.MINUS, ["*"] = TokenKind.STAR,
        ["/"] = TokenKind.SLASH, ["%"] = TokenKind.PERCENT, ["^"] = TokenKind.CARET,
        ["<"] = TokenKind.LESS, [">"] = TokenKind.GREATER, ["="] = TokenKind.EQUAL,
        ["("] = TokenKind.LPAREN, [")"] = TokenKind.RPAREN, ["["] = TokenKind.LBRACKET,
        ["]"] = TokenKind.RBRACKET, ["{"] = TokenKind.LBRACE, ["}"] = TokenKind.RBRACE,
        ["."] = TokenKind.DOT, [","] = TokenKind.COMMA, [":"] = TokenKind.COLON,
        [";"] = TokenKind.SEMICOLON, [string.char(124)] = TokenKind.PIPE, ["!"] = TokenKind.BANG,
        ["?"] = TokenKind.QUESTION, ["#"] = TokenKind.HASH, ["@"] = TokenKind.AT
    }

    return self
end

function Lexer:current()
    if self.pos > #self.text then return "\0" end
    return self.text:sub(self.pos, self.pos)
end

function Lexer:peek(offset)
    local p = self.pos + (offset or 1)
    if p > #self.text then return "\0" end
    return self.text:sub(p, p)
end

function Lexer:advance(n)
    self.pos = self.pos + (n or 1)
end

function Lexer:is_at_end()
    return self.pos > #self.text
end

local function is_digit(c) return c >= "0" and c <= "9" end
local function is_alpha(c) return (c >= "a" and c <= "z") or (c >= "A" and c <= "Z") or c == "_" end
local function is_alnum(c) return is_alpha(c) or is_digit(c) end
local function is_whitespace(c) return c == " " or c == "\t" or c == "\r" end

function Lexer:tokenize()
    local tokens = {}
    while not self:is_at_end() or #self.buffer > 0 do
        local token = self:_next_token()
        if token then table.insert(tokens, token) end
    end
    table.insert(tokens, Token.new(TokenKind.EOF, "", nil, Span.new(self.pos, 0)))
    return tokens
end

function Lexer:_next_token()
    if #self.buffer > 0 then
        return table.remove(self.buffer, 1)
    end

    local c = self:current()
    if is_whitespace(c) then self:_skip_whitespace() return nil end
    if c == "\n" then return self:_read_newline() end
    if c == "-" and self:peek() == "-" then self:_skip_comment() return nil end
    if is_digit(c) then return self:_read_number() end
    if c == '"' then return self:_read_string() end
    if is_alpha(c) then return self:_read_identifier() end
    return self:_read_operator()
end

function Lexer:_skip_whitespace()
    while not self:is_at_end() and is_whitespace(self:current()) do self:advance() end
end

function Lexer:_skip_comment()
    self:advance(2)
    while not self:is_at_end() and self:current() ~= "\n" do self:advance() end
end

function Lexer:_read_newline()
    local start = self.pos
    self:advance()
    return Token.new(TokenKind.NEWLINE, "\n", nil, Span.new(start, 1))
end

function Lexer:_read_number()
    local start = self.pos
    local has_dot = false
    while not self:is_at_end() and (is_digit(self:current()) or self:current() == ".") do
        if self:current() == "." then
            if self:peek() == "." or has_dot then break end
            has_dot = true
        end
        self:advance()
    end
    local lexeme = self.text:sub(start, self.pos - 1)
    local value = tonumber(lexeme)
    local span = Span.new(start, #lexeme)
    local kind = has_dot and TokenKind.FLOAT_LITERAL or TokenKind.INTEGER_LITERAL
    
    if is_alpha(self:current()) then
        while not self:is_at_end() and is_alnum(self:current()) do self:advance() end
        lexeme = self.text:sub(start, self.pos - 1)
        self.diagnostics:report_error("ZT-L002", "número seguido de identificador inválido", Span.new(start, #lexeme))
        return Token.new(TokenKind.BAD_TOKEN, lexeme, nil, Span.new(start, #lexeme))
    end

    return Token.new(kind, lexeme, value, span)
end

function Lexer:_read_string()
    local start = self.pos
    self:advance() -- "
    
    local fragments = {}
    local current_fragment = ""
    local is_interpolated = false

    while not self:is_at_end() and self:current() ~= '"' do
        local c = self:current()
        if c == "{" then
            is_interpolated = true
            table.insert(fragments, { type = "LITERAL", val = current_fragment, pos = self.pos - #current_fragment })
            current_fragment = ""
            
            self:advance() -- {
            local expr_start = self.pos
            local depth = 1
            while not self:is_at_end() and depth > 0 do
                if self:current() == "{" then depth = depth + 1
                elseif self:current() == "}" then depth = depth - 1 end
                if depth > 0 then self:advance() end
            end
            
            local expr_text = self.text:sub(expr_start, self.pos - 1)
            table.insert(fragments, { type = "EXPR", val = expr_text, pos = expr_start })
            
            if self:current() == "}" then self:advance() end
        elseif c == "\\" then
            self:advance()
            local esc = self:current()
            local map = { n="\n", t="\t", r="\r", ['"']='"', ["\\"]="\\" }
            current_fragment = current_fragment .. (map[esc] or esc)
            self:advance()
        else
            current_fragment = current_fragment .. c
            self:advance()
        end
    end
    
    table.insert(fragments, { type = "LITERAL", val = current_fragment, pos = self.pos - #current_fragment })
    
    if self:is_at_end() then
        self.diagnostics:report_error("ZT-L001", "string não terminada", Span.new(start, self.pos - start))
        return Token.new(TokenKind.BAD_TOKEN, self.text:sub(start, self.pos - 1), nil, Span.new(start, self.pos - start))
    end
    
    self:advance() -- "

    local full_lexeme = self.text:sub(start, self.pos - 1)
    local full_span = Span.new(start, #full_lexeme)

    if not is_interpolated then
        return Token.new(TokenKind.STRING_LITERAL, full_lexeme, fragments[1].val, full_span)
    end

    for i, frag in ipairs(fragments) do
        if frag.type == "LITERAL" then
            if #frag.val > 0 or (i == 1 or i == #fragments) then
                table.insert(self.buffer, Token.new(TokenKind.STRING_LITERAL, '"'..frag.val..'"', frag.val, Span.new(frag.pos, #frag.val)))
            end
        else
            table.insert(self.buffer, Token.new(TokenKind.PLUS, "+", nil, Span.new(frag.pos-1, 1)))
            table.insert(self.buffer, Token.new(TokenKind.LPAREN, "(", nil, Span.new(frag.pos, 0)))
            
            local sub_lexer = Lexer.new({ text = frag.val, source = self.source })
            local sub_tokens = sub_lexer:tokenize()
            for _, st in ipairs(sub_tokens) do
                if st.kind ~= TokenKind.EOF then table.insert(self.buffer, st) end
            end
            
            table.insert(self.buffer, Token.new(TokenKind.RPAREN, ")", nil, Span.new(frag.pos + #frag.val, 0)))
            table.insert(self.buffer, Token.new(TokenKind.PLUS, "+", nil, Span.new(self.pos, 1)))
        end
    end
    
    if self.buffer[#self.buffer] and self.buffer[#self.buffer].kind == TokenKind.PLUS then table.remove(self.buffer) end
    return table.remove(self.buffer, 1)
end

function Lexer:_read_identifier()
    local start = self.pos
    while not self:is_at_end() and is_alnum(self:current()) do self:advance() end
    local lexeme = self.text:sub(start, self.pos - 1)
    local kind = KeywordTable.lookup(lexeme) or TokenKind.IDENTIFIER
    local value = nil
    if kind == TokenKind.KW_TRUE then value = true
    elseif kind == TokenKind.KW_FALSE then value = false end

    if kind ~= TokenKind.IDENTIFIER then
        return Token.new(kind, lexeme, value, Span.new(start, #lexeme))
    end

    return Token.new(kind, lexeme, lexeme, Span.new(start, #lexeme))
end

function Lexer:_read_operator()
    local start = self.pos
    local c = self:current()
    local next_c = self:peek()
    
    local combined = c .. next_c
    if self.TWO_CHAR[combined] then
        self:advance(2)
        return Token.new(self.TWO_CHAR[combined], combined, nil, Span.new(start, 2))
    end

    if self.SINGLE_CHAR[c] then
        self:advance()
        return Token.new(self.SINGLE_CHAR[c], c, nil, Span.new(start, 1))
    end

    self.diagnostics:report_error("ZT-L003", "caractere desconhecido: " .. c, Span.new(start, 1))
    self:advance()
    return Token.new(TokenKind.BAD_TOKEN, c, nil, Span.new(start, 1))
end

return Lexer
