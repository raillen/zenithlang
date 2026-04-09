-- ============================================================================
-- Zenith Compiler — Project Config
-- Representação em memória da configuração do projeto (.ztproj).
-- ============================================================================

local ProjectConfig = {}
ProjectConfig.__index = ProjectConfig

function ProjectConfig.new()
    local self = setmetatable({}, ProjectConfig)
    self.info = {
        name = "unnamed",
        version = "0.1.0",
        author = "unknown",
        license = "MIT"
    }
    self.build = {
        main = "src/main.zt",
        out = "dist/out.lua",
        optimize = true,
        target = "lua_5_1"
    }
    self.dependencies = {} -- name -> version_constraint
    return self
end

return ProjectConfig
