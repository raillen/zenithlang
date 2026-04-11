local zt = require("src.backend.lua.runtime.zenith_rt")

local REGEX = {}

local RegexMethods = {}
RegexMethods.__index = RegexMethods

function RegexMethods:test(input)
    return input:find(self.pattern) ~= nil
end

function RegexMethods:find(input)
    local s, e, cap = input:find(self.pattern)
    if not s then return nil end
    
    return {
        value = input:sub(s, e),
        start_pos = s - 1, -- Zenith 0-indexed
        end_pos = e - 1,
        groups = { [0] = cap or "" } -- Captura simples do Lua
    }
end

function RegexMethods:replace(input, replacement)
    -- Lua uses %1, %2 for captures, we can adapt this later if needed
    return (input:gsub(self.pattern, replacement))
end

function REGEX.create_native(pattern_str)
    local obj = {
        pattern = pattern_str,
        _tag = "Regex"
    }
    setmetatable(obj, RegexMethods)
    return obj
end

function REGEX.new_builder()
    local builder = {
        internal_parts = {},
        _tag = "RegexBuilder"
    }
    
    local BuilderMethods = {}
    BuilderMethods.__index = BuilderMethods
    
    function BuilderMethods:starts_with() 
        table.insert(self.internal_parts, "^")
        return self
    end
    
    function BuilderMethods:literal(t)
        local escaped = t:gsub("[%(%)%.%%%+%-%*%?%[%^%$%]]", "%%%1")
        table.insert(self.internal_parts, escaped)
        return self
    end
    
    function BuilderMethods:digit()
        table.insert(self.internal_parts, "%d")
        return self
    end
    
    function BuilderMethods:alpha()
        table.insert(self.internal_parts, "%a")
        return self
    end
    
    function BuilderMethods:whitespace()
        table.insert(self.internal_parts, "%s")
        return self
    end
    
    function BuilderMethods:many()
        table.insert(self.internal_parts, "+")
        return self
    end
    
    function BuilderMethods:optional()
        table.insert(self.internal_parts, "?")
        return self
    end
    
    function BuilderMethods:ends_with()
        table.insert(self.internal_parts, "$")
        return self
    end
    
    function BuilderMethods:build()
        local final_p = table.concat(self.internal_parts, "")
        return REGEX.create_native(final_p)
    end
    
    return setmetatable(builder, BuilderMethods)
end

return REGEX
