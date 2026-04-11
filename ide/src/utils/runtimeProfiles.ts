export type RuntimeProfileId =
  | 'zenith'
  | 'node'
  | 'dotnet'
  | 'rust'
  | 'lua'
  | 'ruby'
  | 'generic';

export interface RuntimeProfile {
  id: RuntimeProfileId;
  label: string;
  settingsLabel: string;
  description: string;
  compilerPlaceholder: string;
  lspPlaceholder: string;
  buildPlaceholder: string;
  extensions: string[];
  fileNames: string[];
  workspaceMarkers: string[];
  recommendations: string[];
  badgeClass: string;
  surfaceClass: string;
  iconClass: string;
}

export interface RuntimeExtensionPreset {
  compilerPath: string;
  lspPath: string;
  buildCommand: string;
  isEnabled: boolean;
}

export const RUNTIME_PROFILE_ORDER: RuntimeProfileId[] = [
  'zenith',
  'node',
  'dotnet',
  'rust',
  'lua',
  'ruby',
  'generic',
];

export const DEFAULT_RUNTIME_EXTENSION_CONFIGS: Record<
  Exclude<RuntimeProfileId, 'generic'>,
  RuntimeExtensionPreset
> = {
  zenith: {
    compilerPath: 'ztc.lua',
    lspPath: '',
    buildCommand: 'lua ztc.lua build',
    isEnabled: true,
  },
  node: {
    compilerPath: 'node',
    lspPath: 'typescript-language-server',
    buildCommand: 'npm run build',
    isEnabled: false,
  },
  dotnet: {
    compilerPath: 'dotnet',
    lspPath: 'csharp-ls',
    buildCommand: 'dotnet build',
    isEnabled: false,
  },
  rust: {
    compilerPath: 'cargo',
    lspPath: 'rust-analyzer',
    buildCommand: 'cargo build',
    isEnabled: false,
  },
  lua: {
    compilerPath: 'lua',
    lspPath: 'lua-language-server',
    buildCommand: 'lua main.lua',
    isEnabled: false,
  },
  ruby: {
    compilerPath: 'ruby',
    lspPath: 'solargraph',
    buildCommand: 'bundle exec ruby main.rb',
    isEnabled: false,
  },
};

export const RUNTIME_PROFILES: Record<RuntimeProfileId, RuntimeProfile> = {
  zenith: {
    id: 'zenith',
    label: 'Zenith',
    settingsLabel: 'Zenith Runtime',
    description: 'Compiler and runtime settings for .zt source files.',
    compilerPlaceholder: 'ztc.lua',
    lspPlaceholder: 'zenith-language-server',
    buildPlaceholder: 'lua ztc.lua build',
    extensions: ['.zt'],
    fileNames: [],
    workspaceMarkers: ['ztc.lua', 'zenith.toml', 'zenith.json'],
    recommendations: [
      'Point the compiler path to your ztc.lua entrypoint.',
      'Keep a default build task for compile and run actions.',
    ],
    badgeClass: 'border-primary/25 bg-primary/10 text-primary',
    surfaceClass: 'border-primary/15 bg-primary/5',
    iconClass: 'text-primary',
  },
  node: {
    id: 'node',
    label: 'TypeScript / JavaScript Node',
    settingsLabel: 'TypeScript / Node Runtime',
    description: 'Node-based toolchains for TS, JS and package-driven workspaces.',
    compilerPlaceholder: 'node',
    lspPlaceholder: 'typescript-language-server',
    buildPlaceholder: 'npm run build',
    extensions: ['.ts', '.tsx', '.js', '.jsx', '.mjs', '.cjs', '.mts', '.cts'],
    fileNames: ['package.json', 'tsconfig.json', 'jsconfig.json'],
    workspaceMarkers: ['package.json', 'tsconfig.json', 'jsconfig.json', '.nvmrc', 'vite.config.ts', 'vite.config.js'],
    recommendations: [
      'Configure the Node binary and build command used by this workspace.',
      'Add a tsconfig or jsconfig near the active file for richer project context.',
    ],
    badgeClass: 'border-sky-500/25 bg-sky-500/10 text-sky-600',
    surfaceClass: 'border-sky-500/15 bg-sky-500/5',
    iconClass: 'text-sky-600',
  },
  dotnet: {
    id: 'dotnet',
    label: 'C# / .NET',
    settingsLabel: 'C# / .NET Runtime',
    description: 'Managed runtime settings for C#, solution files and SDK builds.',
    compilerPlaceholder: 'dotnet',
    lspPlaceholder: 'csharp-ls',
    buildPlaceholder: 'dotnet build',
    extensions: ['.cs', '.csx', '.csproj', '.sln', '.props', '.targets'],
    fileNames: ['global.json'],
    workspaceMarkers: ['.csproj', '.sln', 'global.json', 'Directory.Build.props', 'Directory.Build.targets'],
    recommendations: [
      'Enable the .NET toolchain once the SDK path and build task are known.',
      'Keep a .csproj or solution file close to the active source for better detection.',
    ],
    badgeClass: 'border-emerald-500/25 bg-emerald-500/10 text-emerald-600',
    surfaceClass: 'border-emerald-500/15 bg-emerald-500/5',
    iconClass: 'text-emerald-600',
  },
  rust: {
    id: 'rust',
    label: 'Rust',
    settingsLabel: 'Rust / Cargo Runtime',
    description: 'Cargo-based toolchains for Rust crates, workspaces and native binaries.',
    compilerPlaceholder: 'cargo',
    lspPlaceholder: 'rust-analyzer',
    buildPlaceholder: 'cargo build',
    extensions: ['.rs'],
    fileNames: ['cargo.toml'],
    workspaceMarkers: ['Cargo.toml', 'rust-toolchain.toml', 'rust-toolchain', 'rustfmt.toml', 'clippy.toml'],
    recommendations: [
      'Configure Cargo or rustc, plus a default build task for compile and test flows.',
      'Keep Cargo.toml or rust-toolchain files near the active source for accurate workspace detection.',
    ],
    badgeClass: 'border-orange-500/25 bg-orange-500/10 text-orange-600',
    surfaceClass: 'border-orange-500/15 bg-orange-500/5',
    iconClass: 'text-orange-600',
  },
  lua: {
    id: 'lua',
    label: 'Lua',
    settingsLabel: 'Lua Runtime',
    description: 'Interpreter, formatting and language-server context for Lua files.',
    compilerPlaceholder: 'lua',
    lspPlaceholder: 'lua-language-server',
    buildPlaceholder: 'lua main.lua',
    extensions: ['.lua'],
    fileNames: [],
    workspaceMarkers: ['.luarc.json', '.luarc.jsonc', 'stylua.toml', 'init.lua'],
    recommendations: [
      'Set the Lua binary that should run scripts and tasks.',
      'Add a .luarc or stylua config if this workspace needs shared editor behavior.',
    ],
    badgeClass: 'border-amber-500/25 bg-amber-500/10 text-amber-600',
    surfaceClass: 'border-amber-500/15 bg-amber-500/5',
    iconClass: 'text-amber-600',
  },
  ruby: {
    id: 'ruby',
    label: 'Ruby',
    settingsLabel: 'Ruby Runtime',
    description: 'Ruby runtime and Bundler-aware settings for app and script files.',
    compilerPlaceholder: 'ruby',
    lspPlaceholder: 'solargraph',
    buildPlaceholder: 'bundle exec ruby main.rb',
    extensions: ['.rb', '.rake', '.ru', '.gemspec'],
    fileNames: ['gemfile', 'rakefile', 'config.ru', '.ruby-version'],
    workspaceMarkers: ['Gemfile', '.ruby-version', 'Rakefile', 'config.ru', '.gemspec'],
    recommendations: [
      'Configure Ruby or Bundler commands that match this project.',
      'Keep Gemfile and .ruby-version close to the active file for clearer runtime hints.',
    ],
    badgeClass: 'border-rose-500/25 bg-rose-500/10 text-rose-600',
    surfaceClass: 'border-rose-500/15 bg-rose-500/5',
    iconClass: 'text-rose-600',
  },
  generic: {
    id: 'generic',
    label: 'Generic File',
    settingsLabel: 'Generic Toolchain',
    description: 'No language-specific runtime profile was matched for this file yet.',
    compilerPlaceholder: 'binary',
    lspPlaceholder: 'language-server',
    buildPlaceholder: 'build command',
    extensions: [],
    fileNames: [],
    workspaceMarkers: [],
    recommendations: [
      'Add a runtime profile if you want language-aware controls for this file type.',
    ],
    badgeClass: 'border-ide-border bg-ide-bg/60 text-ide-text-dim',
    surfaceClass: 'border-ide-border bg-ide-bg/30',
    iconClass: 'text-ide-text-dim',
  },
};

function basename(input: string) {
  return input.split(/[\\/]/).pop()?.toLowerCase() || '';
}

function matchesPattern(fileName: string, pattern: string) {
  const normalizedName = fileName.toLowerCase();
  const normalizedPattern = pattern.toLowerCase();

  if (normalizedPattern.startsWith('.')) {
    return normalizedName.endsWith(normalizedPattern);
  }

  return normalizedName === normalizedPattern;
}

export function getRuntimeProfileForSettingsKey(id: string) {
  return RUNTIME_PROFILES[id as RuntimeProfileId] || RUNTIME_PROFILES.generic;
}

export function matchesRuntimeMarker(profile: RuntimeProfile, fileName: string) {
  return profile.workspaceMarkers.some((pattern) => matchesPattern(fileName, pattern));
}

export function detectRuntimeProfileFromFile(file: { name: string; path: string }): RuntimeProfileId {
  const fileName = basename(file.name || file.path);

  for (const profileId of RUNTIME_PROFILE_ORDER) {
    if (profileId === 'generic') continue;

    const profile = RUNTIME_PROFILES[profileId];
    if (profile.fileNames.includes(fileName)) {
      return profile.id;
    }

    if (profile.extensions.some((extension) => fileName.endsWith(extension))) {
      return profile.id;
    }
  }

  return 'generic';
}

export function getLanguageLabelForFile(fileName: string, runtimeId: RuntimeProfileId) {
  const normalized = basename(fileName);

  if (normalized === 'package.json') return 'Node Package Manifest';
  if (normalized === 'tsconfig.json') return 'TypeScript Project Config';
  if (normalized === 'jsconfig.json') return 'JavaScript Project Config';
  if (normalized.endsWith('.tsx')) return 'TypeScript React';
  if (normalized.endsWith('.ts')) return 'TypeScript';
  if (normalized.endsWith('.jsx')) return 'JavaScript React';
  if (normalized.endsWith('.js') || normalized.endsWith('.mjs') || normalized.endsWith('.cjs')) return 'JavaScript';
  if (normalized.endsWith('.cts') || normalized.endsWith('.mts')) return 'TypeScript Module';
  if (normalized.endsWith('.csproj')) return 'C# Project';
  if (normalized.endsWith('.csx')) return 'C# Script';
  if (normalized.endsWith('.cs')) return 'C#';
  if (normalized.endsWith('.sln')) return '.NET Solution';
  if (normalized === 'cargo.toml') return 'Rust Cargo Manifest';
  if (normalized === 'rust-toolchain' || normalized === 'rust-toolchain.toml') return 'Rust Toolchain Config';
  if (normalized === 'rustfmt.toml') return 'Rust Formatter Config';
  if (normalized === 'clippy.toml') return 'Rust Lint Config';
  if (normalized.endsWith('.rs')) return 'Rust';
  if (normalized.endsWith('.lua')) return 'Lua';
  if (normalized === '.luarc.json' || normalized === '.luarc.jsonc') return 'Lua Runtime Config';
  if (normalized.endsWith('.rb')) return 'Ruby';
  if (normalized.endsWith('.rake') || normalized === 'rakefile') return 'Ruby Task File';
  if (normalized === 'config.ru') return 'Ruby Rack Config';
  if (normalized === 'gemfile') return 'Ruby Gemfile';
  if (normalized === '.ruby-version') return 'Ruby Version File';
  if (normalized.endsWith('.zt')) return 'Zenith';

  return RUNTIME_PROFILES[runtimeId].label;
}
