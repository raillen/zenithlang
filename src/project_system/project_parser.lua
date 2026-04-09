-- ============================================================================
-- Zenith Compiler — Project Parser
-- Parser para o formato .ztproj focado em redução de carga cognitiva.
-- ============================================================================

local Lexer         = require("src.syntax.lexer.lexer")
local TokenKind     = require("src.syntax.tokens.token_kind")
local ProjectConfig = require("src.project_system.project_config")
local SourceText    = require("src.source.source_text")
local ParserContext = require("src.syntax.parser.parser_context")
local DiagnosticBag = require("src.diagnostics.diagnostic_bag")

local ProjectParser = {}
ProjectParser.__index = ProjectParser

function ProjectParser.parse(path)
    local file = io.open(path, "r")
    if not file then return nil, "Não foi possível abrir o arquivo de projeto: " .. path end
    local content = file:read("*all")
    file:close()

    local source = SourceText.new(content, path)
    local diags = DiagnosticBag.new()
    local lexer = Lexer.new(source, diags)
    local tokens = lexer:tokenize()
    local ctx = ParserContext.new(tokens, diags)
    
    local config = ProjectConfig.new()

    while not ctx:is_at_end() do
        ctx:skip_newlines()
        if ctx:is_at_end() then break end

        local t = ctx:peek()
        if t.lexeme == "project" then
            ProjectParser._parse_info_block(ctx, config)
        elseif t.lexeme == "build" then
            ProjectParser._parse_build_block(ctx, config)
        elseif t.lexeme == "dependencies" then
            ProjectParser._parse_deps_block(ctx, config)
        else
            ctx:advance() -- Ignorar outros tokens no topo
        end
    end

    if diags:has_errors() then
        return nil, diags:format(source)
    end

    return config
end

function ProjectParser._parse_info_block(ctx, config)
    ctx:advance() -- 'project'
    local name_t = ctx:expect(TokenKind.IDENTIFIER, "esperado nome do projeto")
    config.info.name = name_t.lexeme
    ctx:skip_newlines()

    while not ctx:check(TokenKind.KW_END) and not ctx:is_at_end() do
        local key = ctx:expect(TokenKind.IDENTIFIER, "esperada propriedade do projeto")
        ctx:expect(TokenKind.COLON, "esperado ':'")
        local val = ProjectParser._parse_value(ctx)
        config.info[key.lexeme] = val
        ctx:skip_newlines()
    end
    ctx:expect(TokenKind.KW_END, "esperado 'end' ao final do bloco project")
end

function ProjectParser._parse_build_block(ctx, config)
    ctx:advance() -- 'build'
    ctx:skip_newlines()

    while not ctx:check(TokenKind.KW_END) and not ctx:is_at_end() do
        if ctx:match(TokenKind.AT) then
            -- Atributo de build: @optimize(true)
            local attr = ctx:expect(TokenKind.IDENTIFIER, "esperado nome da diretiva").lexeme
            if ctx:match(TokenKind.LPAREN) then
                local val = ProjectParser._parse_value(ctx)
                config.build[attr] = val
                ctx:expect(TokenKind.RPAREN, "esperado ')'")
            else
                config.build[attr] = true
            end
        else
            local key = ctx:expect(TokenKind.IDENTIFIER, "esperada propriedade de build")
            ctx:expect(TokenKind.COLON, "esperado ':'")
            local val = ProjectParser._parse_value(ctx)
            config.build[key.lexeme] = val
        end
        ctx:skip_newlines()
    end
    ctx:expect(TokenKind.KW_END, "esperado 'end' ao final do bloco build")
end

function ProjectParser._parse_deps_block(ctx, config)
    ctx:advance() -- 'dependencies'
    ctx:skip_newlines()

    while not ctx:check(TokenKind.KW_END) and not ctx:is_at_end() do
        local name = ctx:expect(TokenKind.IDENTIFIER, "esperado nome da dependência").lexeme
        ctx:expect(TokenKind.COLON, "esperado ':'")
        
        -- Parsing de versão (simplificado para v1.0-alpha)
        local version = ""
        while not ctx:check(TokenKind.NEWLINE) and not ctx:check(TokenKind.KW_END) and not ctx:is_at_end() do
            version = version .. ctx:advance().lexeme
        end
        config.dependencies[name] = version
        ctx:skip_newlines()
    end
    ctx:expect(TokenKind.KW_END, "esperado 'end' ao final do bloco dependencies")
end

function ProjectParser._parse_value(ctx)
    local t = ctx:peek()
    if t.kind == TokenKind.STRING_LITERAL or t.kind == TokenKind.INTEGER_LITERAL or t.kind == TokenKind.FLOAT_LITERAL then
        return ctx:advance().value
    elseif t.kind == TokenKind.KW_TRUE then ctx:advance(); return true
    elseif t.kind == TokenKind.KW_FALSE then ctx:advance(); return false
    else
        -- Heurística para caminhos ou versões: consome até o fim da linha ou delimitador
        local val = ""
        while not ctx:check(TokenKind.NEWLINE) and not ctx:check(TokenKind.COMMA) 
              and not ctx:check(TokenKind.RPAREN) and not ctx:check(TokenKind.KW_END) 
              and not ctx:is_at_end() do
            val = val .. ctx:advance().lexeme
        end
        -- Tenta converter para número se parecer um
        return tonumber(val) or val
    end
end

return ProjectParser
