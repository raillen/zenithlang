local function mark(path, ...)
    local f = assert(io.open(path, "a"))
    local parts = {}
    for i = 1, select("#", ...) do
        parts[#parts + 1] = tostring(select(i, ...))
    end
    f:write(table.concat(parts, " "), "\n")
    f:close()
end

local log_path = ".selfhost-bootstrap/parse_progress.log"
os.remove(log_path)

local src_path = "src/compiler/syntax.zt"
local f = assert(io.open(src_path, "r"))
local src = f:read("*a")
f:close()

local mod = dofile(".selfhost-bootstrap/syntax_stage1.lua")
local bag = mod.DiagnosticBag.new({ diagnostics = {}, count = 0 })

mark(log_path, "loaded", os.clock())

local lexer = mod.Lexer.new({
    source = src,
    pos = 0,
    line = 1,
    col = 1,
    file = src_path,
    tokens = {},
})

local tokens = lexer:tokenize()
mark(log_path, "tokenized", os.clock(), "lexer_pos", lexer.pos)

local orig_advance = mod.Parser.advance
mod.Parser.advance = function(self)
    local t = orig_advance(self)
    self._dbg_adv = (self._dbg_adv or 0) + 1
    if self._dbg_adv <= 20 or self._dbg_adv % 1000 == 0 then
        local next_t = self:peek()
        mark(
            log_path,
            "advance",
            self._dbg_adv,
            "pos",
            self.pos,
            "last",
            t and t.kind or "nil",
            "next",
            next_t and next_t.kind or "nil",
            "clock",
            os.clock()
        )
    end
    return t
end

local orig_expect = mod.Parser.expect
mod.Parser.expect = function(self, k, m)
    local t = self:peek()
    if t.kind ~= k then
        self._dbg_expect_fail = (self._dbg_expect_fail or 0) + 1
        if self._dbg_expect_fail <= 20 then
            local info1 = debug.getinfo(2, "nSl")
            local info2 = debug.getinfo(3, "nSl")
            mark(
                log_path,
                "expect_fail",
                self._dbg_expect_fail,
                "pos",
                self.pos,
                "wanted",
                k,
                "got",
                t.kind,
                "text",
                t.text,
                "msg",
                m,
                "f1",
                info1 and info1.name or "?",
                info1 and info1.currentline or -1,
                "f2",
                info2 and info2.name or "?",
                info2 and info2.currentline or -1,
                "clock",
                os.clock()
            )
        end
    end
    return orig_expect(self, k, m)
end

local parser = mod.Parser.new({ tokens = tokens, pos = 0, diagnostics = bag })
mark(log_path, "before_parse", os.clock())
local samples = 0
debug.sethook(function()
    samples = samples + 1
    local info1 = debug.getinfo(2, "nSl")
    local info2 = debug.getinfo(3, "nSl")
    mark(
        log_path,
        "sample",
        samples,
        "clock",
        os.clock(),
        "f1",
        (info1 and info1.name or "?"),
        (info1 and info1.currentline or -1),
        "src1",
        (info1 and info1.short_src or "?"),
        "f2",
        (info2 and info2.name or "?"),
        (info2 and info2.currentline or -1),
        "src2",
        (info2 and info2.short_src or "?")
    )
end, "", 500000)
local ok, err = pcall(function()
    parser:parse()
end)
debug.sethook()
mark(log_path, "after_parse", os.clock(), ok, err or "nil", "pos", parser.pos, "bag", bag.count)
