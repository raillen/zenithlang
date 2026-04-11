import { invoke as tauriInvoke } from "@tauri-apps/api/core";

interface MockFileEntry {
  name: string;
  path: string;
  is_directory: boolean;
  children?: MockFileEntry[];
}

const DEFAULT_BROWSER_SOURCE = `// Zenith Language Prototype
// Welcome to the Zenith Keter Preview Mode

func main() {
    print("Zenith is running in Browser Mode!");

    let compiler = check Zenith.Compiler.init();
    match compiler.status {
        .Ready => print("Stable Architecture"),
        .Error(e) => print("Error: {e}"),
    }
}
`;

const INITIAL_MOCK_FILE_TREE: MockFileEntry[] = [
  {
    name: "src",
    path: "../src",
    is_directory: true,
    children: [
      { name: "main.zt", path: "../src/main.zt", is_directory: false },
      { name: "compiler.zt", path: "../src/compiler.zt", is_directory: false },
    ],
  },
  {
    name: "tests",
    path: "../tests",
    is_directory: true,
    children: [{ name: "test_syntax.zt", path: "../tests/test_syntax.zt", is_directory: false }],
  },
  { name: "ztc.lua", path: "../ztc.lua", is_directory: false },
  { name: "README.md", path: "../README.md", is_directory: false },
];

function normalizeMockPath(input: string) {
  const raw = String(input || ".").replace(/\\/g, "/").replace(/\/+/g, "/");
  if (raw === "." || raw === "./") return ".";
  return raw.replace(/\/+$/g, "") || ".";
}

function joinMockPath(parentPath: string, name: string) {
  const normalizedParent = normalizeMockPath(parentPath);
  return normalizedParent === "." ? `./${name}` : `${normalizedParent}/${name}`;
}

function getParentMockPath(path: string) {
  const normalized = normalizeMockPath(path);
  const separatorIndex = normalized.lastIndexOf("/");
  if (separatorIndex <= 0) return ".";
  return normalized.slice(0, separatorIndex);
}

function cloneMockEntry(entry: MockFileEntry): MockFileEntry {
  return {
    ...entry,
    children: entry.children?.map(cloneMockEntry),
  };
}

function cloneMockTree(entries: MockFileEntry[]) {
  return entries.map(cloneMockEntry);
}

function getUniqueMockName(existingNames: Set<string>, preferredName: string, duplicate = false) {
  const lower = preferredName.toLowerCase();
  if (!existingNames.has(lower) && !duplicate) return preferredName;

  const dotIndex = preferredName.lastIndexOf(".");
  const hasExtension = dotIndex > 0;
  const base = hasExtension ? preferredName.slice(0, dotIndex) : preferredName;
  const extension = hasExtension ? preferredName.slice(dotIndex) : "";
  const duplicateBase = duplicate ? `${base} copy` : base;

  for (let index = 1; index < 1000; index += 1) {
    const suffix = index === 1 ? "" : ` ${index}`;
    const candidate = `${duplicateBase}${suffix}${extension}`;
    if (!existingNames.has(candidate.toLowerCase())) {
      return candidate;
    }
  }

  return `${duplicateBase}${extension}`;
}

function findEntryLocation(
  entries: MockFileEntry[],
  targetPath: string
): { entry: MockFileEntry; siblings: MockFileEntry[]; index: number } | null {
  const normalizedTarget = normalizeMockPath(targetPath);

  for (let index = 0; index < entries.length; index += 1) {
    const entry = entries[index];
    if (normalizeMockPath(entry.path) === normalizedTarget) {
      return { entry, siblings: entries, index };
    }

    if (entry.children?.length) {
      const found = findEntryLocation(entry.children, targetPath);
      if (found) return found;
    }
  }

  return null;
}

function getChildrenForParent(parentPath: string) {
  const normalizedParent = normalizeMockPath(parentPath);
  if (normalizedParent === ".") return mockFileTree;

  const location = findEntryLocation(mockFileTree, normalizedParent);
  if (!location?.entry.is_directory) {
    throw new Error(`Mock folder not found: ${parentPath}`);
  }

  if (!location.entry.children) {
    location.entry.children = [];
  }

  return location.entry.children;
}

function cloneEntryToPath(entry: MockFileEntry, destinationPath: string, name = entry.name): MockFileEntry {
  const cloned: MockFileEntry = {
    name,
    path: destinationPath,
    is_directory: entry.is_directory,
  };

  if (entry.children?.length) {
    cloned.children = entry.children.map((child) =>
      cloneEntryToPath(child, joinMockPath(destinationPath, child.name), child.name)
    );
  }

  return cloned;
}

function copyMockContentsRecursive(source: MockFileEntry, cloned: MockFileEntry) {
  if (!source.is_directory) {
    mockFileContents[normalizeMockPath(cloned.path)] =
      mockFileContents[normalizeMockPath(source.path)] ?? "";
    return;
  }

  const sourceChildren = source.children || [];
  const clonedChildren = cloned.children || [];
  for (let index = 0; index < sourceChildren.length; index += 1) {
    const sourceChild = sourceChildren[index];
    const clonedChild = clonedChildren[index];
    if (sourceChild && clonedChild) {
      copyMockContentsRecursive(sourceChild, clonedChild);
    }
  }
}

function removeMockContentsUnderPath(path: string) {
  const normalizedTarget = normalizeMockPath(path);
  for (const key of Object.keys(mockFileContents)) {
    if (key === normalizedTarget || key.startsWith(`${normalizedTarget}/`)) {
      delete mockFileContents[key];
    }
  }
}

function cloneEntryForRead(entry: MockFileEntry) {
  return cloneMockEntry(entry);
}

const INITIAL_MOCK_FILE_CONTENTS: Record<string, string> = {
  [normalizeMockPath("../src/main.zt")]: DEFAULT_BROWSER_SOURCE,
  [normalizeMockPath("../src/compiler.zt")]: `module Zenith.Compiler\n\nfunc init() -> Result<Compiler, String> {\n    return .Ready\n}\n`,
  [normalizeMockPath("../tests/test_syntax.zt")]: `test \"parser accepts prototype syntax\" {\n    expect(parse(\"func main() {}\")).toBeOk()\n}\n`,
  [normalizeMockPath("../ztc.lua")]: `-- Browser preview runtime shim\nprint(\"Zenith browser preview\")\n`,
  [normalizeMockPath("../README.md")]: `# Zenith Keter\n\nBrowser preview workspace used for UI smoke tests.\n`,
};

const STATIC_MOCK_DATA: Record<string, any> = {
  get_git_status: {},
  get_git_branch: "browser-preview",
  get_sys_info: {
    cpu: 18.4,
    memory: 8 * 1024 * 1024 * 1024,
    memory_used: 3.2 * 1024 * 1024 * 1024,
  },
};

let mockTerminalSessionId = 1000;
let mockFileTree = cloneMockTree(INITIAL_MOCK_FILE_TREE);
let mockFileContents = { ...INITIAL_MOCK_FILE_CONTENTS };

function createMockFile(parentPath: string, name: string, content = "") {
  const siblings = getChildrenForParent(parentPath);
  const nextName = getUniqueMockName(new Set(siblings.map((entry) => entry.name.toLowerCase())), name);
  const entry: MockFileEntry = {
    name: nextName,
    path: joinMockPath(parentPath, nextName),
    is_directory: false,
  };

  siblings.push(entry);
  mockFileContents[normalizeMockPath(entry.path)] = content;
  return cloneEntryForRead(entry);
}

function createMockFolder(parentPath: string, name: string) {
  const siblings = getChildrenForParent(parentPath);
  const nextName = getUniqueMockName(new Set(siblings.map((entry) => entry.name.toLowerCase())), name);
  const entry: MockFileEntry = {
    name: nextName,
    path: joinMockPath(parentPath, nextName),
    is_directory: true,
    children: [],
  };

  siblings.push(entry);
  return cloneEntryForRead(entry);
}

function renameMockPath(currentPath: string, newName: string) {
  const location = findEntryLocation(mockFileTree, currentPath);
  if (!location) {
    throw new Error(`Mock path not found: ${currentPath}`);
  }

  const parentPath = getParentMockPath(location.entry.path);
  const siblings = location.siblings.filter((_, index) => index !== location.index);
  const nextName = getUniqueMockName(new Set(siblings.map((entry) => entry.name.toLowerCase())), newName);
  const renamedEntry = cloneEntryToPath(location.entry, joinMockPath(parentPath, nextName), nextName);

  location.siblings[location.index] = renamedEntry;
  copyMockContentsRecursive(location.entry, renamedEntry);
  removeMockContentsUnderPath(location.entry.path);

  return cloneEntryForRead(renamedEntry);
}

function copyMockPath(sourcePath: string, destinationParentPath: string) {
  const location = findEntryLocation(mockFileTree, sourcePath);
  if (!location) {
    throw new Error(`Mock source path not found: ${sourcePath}`);
  }

  const destinationChildren = getChildrenForParent(destinationParentPath);
  const sourceParentPath = getParentMockPath(location.entry.path);
  const nextName = getUniqueMockName(
    new Set(destinationChildren.map((entry) => entry.name.toLowerCase())),
    location.entry.name,
    normalizeMockPath(sourceParentPath) === normalizeMockPath(destinationParentPath)
  );

  const clonedEntry = cloneEntryToPath(location.entry, joinMockPath(destinationParentPath, nextName), nextName);
  destinationChildren.push(clonedEntry);
  copyMockContentsRecursive(location.entry, clonedEntry);

  return cloneEntryForRead(clonedEntry);
}

function moveMockPath(sourcePath: string, destinationParentPath: string) {
  const location = findEntryLocation(mockFileTree, sourcePath);
  if (!location) {
    throw new Error(`Mock source path not found: ${sourcePath}`);
  }

  const destinationChildren = getChildrenForParent(destinationParentPath);
  const sourceParentPath = getParentMockPath(location.entry.path);
  const siblingEntries =
    normalizeMockPath(sourceParentPath) === normalizeMockPath(destinationParentPath)
      ? destinationChildren.filter((_, index) => index !== location.index)
      : destinationChildren;
  const nextName = getUniqueMockName(
    new Set(siblingEntries.map((entry) => entry.name.toLowerCase())),
    location.entry.name
  );

  const movedEntry = cloneEntryToPath(location.entry, joinMockPath(destinationParentPath, nextName), nextName);
  location.siblings.splice(location.index, 1);
  destinationChildren.push(movedEntry);
  copyMockContentsRecursive(location.entry, movedEntry);
  removeMockContentsUnderPath(location.entry.path);

  return cloneEntryForRead(movedEntry);
}

function deleteMockPath(targetPath: string) {
  const location = findEntryLocation(mockFileTree, targetPath);
  if (!location) {
    return;
  }

  location.siblings.splice(location.index, 1);
  removeMockContentsUnderPath(location.entry.path);
}

function ensureMockFile(path: string, content = "") {
  const normalizedPath = normalizeMockPath(path);
  const location = findEntryLocation(mockFileTree, normalizedPath);
  if (location) {
    mockFileContents[normalizedPath] = content;
    return cloneEntryForRead(location.entry);
  }

  const parentPath = getParentMockPath(normalizedPath);
  const name = normalizedPath.split("/").pop() || "untitled";
  return createMockFile(parentPath, name, content);
}

export function isTauriEnvironment() {
  return typeof window !== "undefined" && !!(window as any).__TAURI_INTERNALS__;
}

export async function invoke<T>(command: string, args?: any): Promise<T> {
  if (!isTauriEnvironment()) {
    console.warn(`[Tauri Mock] Command "${command}" invoked in browser mode. Returning mock data.`);

    await new Promise((resolve) => setTimeout(resolve, 150));

    if (command === "get_file_tree") {
      return cloneMockTree(mockFileTree) as T;
    }

    if (command === "read_file") {
      const path = normalizeMockPath(String(args?.path || "../src/main.zt"));
      return (mockFileContents[path] ?? DEFAULT_BROWSER_SOURCE) as T;
    }

    if (command === "write_file") {
      const path = String(args?.path || "./untitled.zt");
      const content = String(args?.content || "");
      ensureMockFile(path, content);
      return undefined as T;
    }

    if (command in STATIC_MOCK_DATA) {
      return STATIC_MOCK_DATA[command] as T;
    }

    if (
      command === "terminal_write" ||
      command === "terminal_resize" ||
      command === "terminal_kill" ||
      command === "reveal_in_system_explorer" ||
      command === "open_in_system_explorer"
    ) {
      return undefined as T;
    }

    if (command === "terminal_create") {
      const cwd = String(args?.cwd || ".");
      return {
        sessionId: mockTerminalSessionId++,
        cwd,
        shell: "browser-shell",
      } as T;
    }

    if (command === "create_file") {
      const parentPath = String(args?.parentPath || ".");
      const name = String(args?.name || "Untitled.zt");
      const content = String(args?.content || "");
      return createMockFile(parentPath, name, content) as T;
    }

    if (command === "create_folder") {
      const parentPath = String(args?.parentPath || ".");
      const name = String(args?.name || "New Folder");
      return createMockFolder(parentPath, name) as T;
    }

    if (command === "copy_path") {
      return copyMockPath(String(args?.sourcePath || "."), String(args?.destinationParentPath || ".")) as T;
    }

    if (command === "move_path") {
      return moveMockPath(String(args?.sourcePath || "."), String(args?.destinationParentPath || ".")) as T;
    }

    if (command === "rename_path") {
      return renameMockPath(String(args?.path || "."), String(args?.newName || "Renamed")) as T;
    }

    if (command === "delete_path") {
      deleteMockPath(String(args?.path || "."));
      return undefined as T;
    }

    return [] as unknown as T;
  }

  try {
    return await tauriInvoke<T>(command, args);
  } catch (error) {
    console.error(`[Tauri Invoke Error] ${command}:`, error);
    throw error;
  }
}
