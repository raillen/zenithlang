import { ExtensionConfig, FileEntry } from '../store/useWorkspaceStore';
import {
  RuntimeProfileId,
  detectRuntimeProfileFromFile,
  getRuntimeProfileForSettingsKey,
} from './runtimeProfiles';

export interface RunTargetOption {
  id: string;
  label: string;
  description: string;
}

export interface PipelineCommandPlan {
  runtimeId: RuntimeProfileId;
  targetId: string;
  label: string;
  command: string;
  workingDir: string;
}

interface PipelineContext {
  file: FileEntry;
  fileTree: FileEntry[];
  currentProjectRoot: string;
  extensionConfig: ExtensionConfig;
  runtimeId?: RuntimeProfileId;
}

const RUN_TARGETS: Record<RuntimeProfileId, RunTargetOption[]> = {
  zenith: [
    { id: 'script', label: 'Lua Script', description: 'Compile and run the active .zt file.' },
    { id: 'web', label: 'Web Build', description: 'Use the workspace build command for a web target.' },
  ],
  node: [
    { id: 'file', label: 'Current File', description: 'Run the active file with the configured runtime.' },
    { id: 'workspace', label: 'Workspace Task', description: 'Run the configured workspace task.' },
  ],
  dotnet: [
    { id: 'project', label: 'Project Run', description: 'Run the nearest .NET project.' },
  ],
  rust: [
    { id: 'cargo-run', label: 'Cargo Run', description: 'Run the nearest Cargo package.' },
    { id: 'cargo-test', label: 'Cargo Test', description: 'Run tests for the nearest Cargo package.' },
  ],
  lua: [
    { id: 'file', label: 'Current File', description: 'Run the active file with Lua.' },
  ],
  ruby: [
    { id: 'file', label: 'Current File', description: 'Run the active file with Ruby.' },
  ],
  generic: [
    { id: 'file', label: 'Current File', description: 'Run the active file with the configured binary.' },
  ],
};

export function getRunTargetOptions(runtimeId: RuntimeProfileId) {
  return RUN_TARGETS[runtimeId] || RUN_TARGETS.generic;
}

export function getDefaultRunTarget(runtimeId: RuntimeProfileId) {
  return getRunTargetOptions(runtimeId)[0]?.id || 'file';
}

export function getRuntimeForFile(file: FileEntry) {
  return detectRuntimeProfileFromFile(file);
}

export function buildRunPlan({
  file,
  fileTree,
  currentProjectRoot,
  extensionConfig,
  runtimeId = detectRuntimeProfileFromFile(file),
  targetId,
}: PipelineContext & { runtimeId?: RuntimeProfileId; targetId: string }): PipelineCommandPlan | null {
  const profile = getRuntimeProfileForSettingsKey(runtimeId);
  const runtimeBinary = extensionConfig.compilerPath || profile.compilerPlaceholder;
  const defaultWorkingDir = findWorkingDirectory(runtimeId, file, fileTree, currentProjectRoot);
  const activeFileCommand = `${runtimeBinary} ${quotePath(file.path)}`;

  switch (runtimeId) {
    case 'zenith': {
      const compilerDir = findMarkerDirectory(file, fileTree, ['ztc.lua']) || currentProjectRoot;
      if (targetId === 'web') {
        return {
          runtimeId,
          targetId,
          label: 'Zenith Web Build',
          command: extensionConfig.buildCommand || profile.buildPlaceholder,
          workingDir: compilerDir,
        };
      }

      return {
        runtimeId,
        targetId,
        label: 'Zenith Lua Script',
        command: `lua ztc.lua ${quotePath(file.path)}`,
        workingDir: compilerDir,
      };
    }

    case 'node': {
      if (targetId === 'workspace') {
        return {
          runtimeId,
          targetId,
          label: 'Workspace Task',
          command: extensionConfig.buildCommand || profile.buildPlaceholder,
          workingDir: findMarkerDirectory(file, fileTree, ['package.json', 'tsconfig.json', 'jsconfig.json']) || currentProjectRoot,
        };
      }

      return {
        runtimeId,
        targetId,
        label: 'Node Current File',
        command: activeFileCommand,
        workingDir: defaultWorkingDir,
      };
    }

    case 'dotnet': {
      const projectFile = findNearestMarker(file, fileTree, ['.csproj', '.sln']);
      const command = projectFile && projectFile.name.toLowerCase().endsWith('.csproj')
        ? `${runtimeBinary} run --project ${quotePath(projectFile.path)}`
        : `${runtimeBinary} run`;

      return {
        runtimeId,
        targetId,
        label: 'Dotnet Run',
        command,
        workingDir: findWorkingDirectory(runtimeId, file, fileTree, currentProjectRoot),
      };
    }

    case 'rust': {
      const manifest = findNearestMarker(file, fileTree, ['Cargo.toml']);
      const manifestArg = manifest ? ` --manifest-path ${quotePath(manifest.path)}` : '';
      const command = targetId === 'cargo-test'
        ? `${runtimeBinary} test${manifestArg}`
        : `${runtimeBinary} run${manifestArg}`;

      return {
        runtimeId,
        targetId,
        label: targetId === 'cargo-test' ? 'Cargo Test' : 'Cargo Run',
        command,
        workingDir: findWorkingDirectory(runtimeId, file, fileTree, currentProjectRoot),
      };
    }

    case 'lua':
      return {
        runtimeId,
        targetId,
        label: 'Lua Current File',
        command: activeFileCommand,
        workingDir: defaultWorkingDir,
      };

    case 'ruby': {
      const gemfile = findNearestMarker(file, fileTree, ['Gemfile']);
      return {
        runtimeId,
        targetId,
        label: 'Ruby Current File',
        command: gemfile
          ? `bundle exec ${runtimeBinary} ${quotePath(file.path)}`
          : activeFileCommand,
        workingDir: findWorkingDirectory(runtimeId, file, fileTree, currentProjectRoot),
      };
    }

    default:
      return {
        runtimeId: 'generic',
        targetId,
        label: 'Current File',
        command: activeFileCommand,
        workingDir: defaultWorkingDir,
      };
  }
}

export function buildBuildPlan({
  file,
  fileTree,
  currentProjectRoot,
  extensionConfig,
  runtimeId = detectRuntimeProfileFromFile(file),
}: PipelineContext): PipelineCommandPlan | null {
  const profile = getRuntimeProfileForSettingsKey(runtimeId);
  const runtimeBinary = extensionConfig.compilerPath || profile.compilerPlaceholder;
  const buildCommand = extensionConfig.buildCommand || profile.buildPlaceholder;

  switch (runtimeId) {
    case 'dotnet':
      return {
        runtimeId,
        targetId: 'build',
        label: 'Dotnet Build',
        command: buildCommand,
        workingDir: findWorkingDirectory(runtimeId, file, fileTree, currentProjectRoot),
      };

    case 'rust': {
      const manifest = findNearestMarker(file, fileTree, ['Cargo.toml']);
      const command =
        extensionConfig.buildCommand && extensionConfig.buildCommand !== profile.buildPlaceholder
          ? extensionConfig.buildCommand
          : `${runtimeBinary} build${manifest ? ` --manifest-path ${quotePath(manifest.path)}` : ''}`;

      return {
        runtimeId,
        targetId: 'build',
        label: 'Cargo Build',
        command,
        workingDir: findWorkingDirectory(runtimeId, file, fileTree, currentProjectRoot),
      };
    }

    case 'lua':
    case 'ruby':
    case 'generic':
      return {
        runtimeId,
        targetId: 'build',
        label: 'Build Task',
        command: buildCommand,
        workingDir: findWorkingDirectory(runtimeId, file, fileTree, currentProjectRoot),
      };

    default:
      return {
        runtimeId,
        targetId: 'build',
        label: 'Build Task',
        command: buildCommand,
        workingDir: findWorkingDirectory(runtimeId, file, fileTree, currentProjectRoot),
      };
  }
}

export function wrapPipelineCommandForShell(params: {
  shell: string | null;
  taskId: string;
  command: string;
  workingDir: string;
}) {
  const { shell, taskId, command, workingDir } = params;
  const shellLower = (shell || '').toLowerCase();

  if (shellLower.endsWith('powershell.exe') || shellLower.endsWith('pwsh.exe')) {
    return [
      `Write-Output "__ZENITH_TASK_START__:${taskId}"`,
      `Set-Location -LiteralPath ${quotePowerShell(workingDir)}`,
      command,
      `$__zenithExit = if ($LASTEXITCODE -ne $null) { [int]$LASTEXITCODE } elseif ($?) { 0 } else { 1 }`,
      `Write-Output "__ZENITH_TASK_END__:${taskId}:$__zenithExit"`,
      '',
    ].join('\r\n');
  }

  if (shellLower.endsWith('cmd.exe')) {
    return [
      `echo __ZENITH_TASK_START__:${taskId}`,
      `cd /d ${quoteCmd(workingDir)}`,
      command,
      `echo __ZENITH_TASK_END__:${taskId}:%errorlevel%`,
      '',
    ].join('\r\n');
  }

  return [
    `printf '__ZENITH_TASK_START__:${taskId}\\n'`,
    `cd ${quoteBash(workingDir)}`,
    command,
    `__zenith_exit=$?`,
    `printf '__ZENITH_TASK_END__:${taskId}:%s\\n' "$__zenith_exit"`,
    '',
  ].join('\n');
}

function flattenFiles(entries: FileEntry[]): FileEntry[] {
  const files: FileEntry[] = [];

  for (const entry of entries) {
    if (entry.is_directory) {
      files.push(...flattenFiles(entry.children || []));
    } else {
      files.push(entry);
    }
  }

  return files;
}

function normalizePath(input: string) {
  return input.replace(/\\/g, '/').toLowerCase();
}

function getPathSegments(input: string) {
  return normalizePath(input).split('/').filter(Boolean);
}

function sharedPathScore(left: string, right: string) {
  const leftSegments = getPathSegments(left);
  const rightSegments = getPathSegments(right);
  let score = 0;

  while (
    score < leftSegments.length &&
    score < rightSegments.length &&
    leftSegments[score] === rightSegments[score]
  ) {
    score += 1;
  }

  return score;
}

function findNearestMarker(file: FileEntry, fileTree: FileEntry[], patterns: string[]) {
  return flattenFiles(fileTree)
    .filter((entry) =>
      patterns.some((pattern) =>
        pattern.startsWith('.')
          ? entry.name.toLowerCase().endsWith(pattern.toLowerCase())
          : entry.name.toLowerCase() === pattern.toLowerCase()
      )
    )
    .sort((left, right) => {
      const rightScore = sharedPathScore(file.path, right.path);
      const leftScore = sharedPathScore(file.path, left.path);

      if (leftScore !== rightScore) {
        return rightScore - leftScore;
      }

      return left.path.length - right.path.length;
    })[0] || null;
}

function directoryOf(input: string) {
  return input.replace(/[\\/][^\\/]+$/, '') || input;
}

function findMarkerDirectory(file: FileEntry, fileTree: FileEntry[], patterns: string[]) {
  const match = findNearestMarker(file, fileTree, patterns);
  return match ? directoryOf(match.path) : null;
}

function findWorkingDirectory(
  runtimeId: RuntimeProfileId,
  file: FileEntry,
  fileTree: FileEntry[],
  currentProjectRoot: string
) {
  switch (runtimeId) {
    case 'node':
      return findMarkerDirectory(file, fileTree, ['package.json', 'tsconfig.json', 'jsconfig.json']) || directoryOf(file.path) || currentProjectRoot;
    case 'dotnet':
      return findMarkerDirectory(file, fileTree, ['.csproj', '.sln', 'global.json']) || currentProjectRoot;
    case 'rust':
      return findMarkerDirectory(file, fileTree, ['Cargo.toml', 'rust-toolchain.toml', 'rust-toolchain']) || currentProjectRoot;
    case 'ruby':
      return findMarkerDirectory(file, fileTree, ['Gemfile', '.ruby-version', 'Rakefile', 'config.ru']) || directoryOf(file.path) || currentProjectRoot;
    case 'lua':
      return findMarkerDirectory(file, fileTree, ['.luarc.json', '.luarc.jsonc', 'stylua.toml']) || directoryOf(file.path) || currentProjectRoot;
    case 'zenith':
      return findMarkerDirectory(file, fileTree, ['ztc.lua']) || currentProjectRoot;
    default:
      return directoryOf(file.path) || currentProjectRoot;
  }
}

function quotePath(input: string) {
  return `"${input.replace(/"/g, '\"')}"`;
}

function quotePowerShell(input: string) {
  return `'${input.replace(/'/g, "''")}'`;
}

function quoteCmd(input: string) {
  return `"${input.replace(/"/g, '""')}"`;
}

function quoteBash(input: string) {
  return `'${input.replace(/'/g, "'\\''")}'`;
}
