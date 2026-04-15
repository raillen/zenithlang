import { StateCreator } from 'zustand';
import { invoke } from '../utils/tauri';
import { WorkspaceState, FileSystemSlice, FileEntry } from '../types/ide';
import {
  decorateExplorerEntries,
  loadDirectoryEntries,
  mergeExplorerEntries,
  normalizeWorkspacePath,
} from '../utils/workspaceExplorer';

let lastBootstrappedRoot: string | null = null;
const bootstrapRequests = new Map<string, Promise<void>>();
let gitRefreshTimer: number | null = null;

export function bootstrapWorkspaceRoot(rootPath: string) {
  const normalizedRoot = normalizeWorkspacePath(rootPath).toLowerCase();

  if (lastBootstrappedRoot === normalizedRoot) {
    return Promise.resolve();
  }

  const pendingRequest = bootstrapRequests.get(normalizedRoot);
  if (pendingRequest) {
    return pendingRequest;
  }

  const request = invoke('workspace_index_bootstrap', { rootPath })
    .then(() => {
      lastBootstrappedRoot = normalizedRoot;
    })
    .finally(() => {
      bootstrapRequests.delete(normalizedRoot);
    });

  bootstrapRequests.set(normalizedRoot, request);
  return request;
}

export const createFileSystemSlice: StateCreator<
  WorkspaceState,
  [],
  [],
  FileSystemSlice
> = (set, get) => ({
  currentProjectRoot: '.',
  fileTree: [],
  fileBuffers: {},
  dirtyFiles: new Set(),
  gitStatus: {},
  gitCurrentBranch: null,
  availableWorkspaces: [],
  systemStats: null,
  searchFilterPath: null,

  setProjectRoot: async (path) => {
    set({
      currentProjectRoot: path,
      fileTree: [],
      gitStatus: {},
      gitCurrentBranch: null,
      searchFilterPath: null,
    });
    void get().refreshExplorerRoot();
    void bootstrapWorkspaceRoot(path).catch((error) => {
      console.error('Failed to bootstrap workspace index', error);
    });
    get().scheduleGitRefresh(700);
  },
  setFileTree: (tree) => set({ fileTree: decorateExplorerEntries(tree) }),
  updateFileTreeNode: (path, children) => {
    const updateNode = (nodes: FileEntry[]): FileEntry[] => {
      return nodes.map((node) => {
        if (node.path === path) {
          return {
            ...node,
            isLoaded: true,
            children: mergeExplorerEntries(node.children || [], children),
          };
        }
        if (node.children) return { ...node, children: updateNode(node.children) };
        return node;
      });
    };
    set((state) => ({ fileTree: updateNode(state.fileTree) }));
  },
  refreshExplorerRoot: async () => {
    const requestRoot = get().currentProjectRoot;

    try {
      const entries = await loadDirectoryEntries(requestRoot);
      if (get().currentProjectRoot !== requestRoot) return;
      const currentTree = get().fileTree;

      set({
        fileTree: mergeExplorerEntries(currentTree, entries),
      });
    } catch (error) {
      console.error('Failed to load workspace root', error);
      throw error;
    }
  },
  loadDirectoryChildren: async (path) => {
    const requestRoot = get().currentProjectRoot;
    const children = await loadDirectoryEntries(path);

    if (get().currentProjectRoot !== requestRoot) {
      return children;
    }

    get().updateFileTreeNode(path, children);
    return children;
  },
  setFileBuffer: (path, content) =>
    set((state) => ({
      fileBuffers: { ...state.fileBuffers, [path]: content },
    })),
  removeFileBuffer: (path) =>
    set((state) => {
      const { [path]: _, ...rest } = state.fileBuffers;
      return { fileBuffers: rest };
    }),
  setFileDirty: (path, dirty) =>
    set((state) => {
      const nextDirty = new Set(state.dirtyFiles);
      if (dirty) nextDirty.add(path);
      else nextDirty.delete(path);
      return { dirtyFiles: nextDirty };
    }),

  scheduleGitRefresh: (delayMs = 250) => {
    if (typeof window === 'undefined') {
      void get().refreshGitStatus();
      return;
    }

    if (gitRefreshTimer !== null) {
      window.clearTimeout(gitRefreshTimer);
    }

    gitRefreshTimer = window.setTimeout(() => {
      gitRefreshTimer = null;
      void get().refreshGitStatus();
    }, delayMs);
  },

  refreshGitStatus: async () => {
    const requestRoot = get().currentProjectRoot;

    try {
      const status = await invoke<any>('get_git_status');
      if (get().currentProjectRoot !== requestRoot) return;
      // Backend returns { branch: string, files: Array<{path, status, staged}> }
      // We'll convert files array to the Record<string, string> format expected by components
      const statusMap: Record<string, string> = {};
      if (status && Array.isArray(status.files)) {
        status.files.forEach((f: any) => {
          const filePath = typeof f?.path === 'string' ? f.path.trim() : '';
          const fileStatus = typeof f?.status === 'string' ? f.status : '';
          if (!filePath || !fileStatus) return;
          statusMap[filePath] = fileStatus;
        });
      }
      set({
        gitStatus: statusMap,
        gitCurrentBranch: status?.branch || '',
      });
    } catch (e) {
      console.error(e);
    }
  },
  refreshGitBranch: async () => {
    const requestRoot = get().currentProjectRoot;

    try {
      const branch = await invoke<string>('get_git_branch');
      if (get().currentProjectRoot !== requestRoot) return;
      set({ gitCurrentBranch: branch });
    } catch (e) {
      console.error(e);
    }
  },
  stageFile: async (path) => {
    await invoke('git_stage', { path });
    await get().refreshGitStatus();
  },
  unstageFile: async (path) => {
    await invoke('git_unstage', { path });
    await get().refreshGitStatus();
  },
  stageAll: async () => {
    await invoke('git_stage_all');
    await get().refreshGitStatus();
  },
  unstageAll: async () => {
    await invoke('git_unstage_all');
    await get().refreshGitStatus();
  },
  discardChanges: async (path) => {
    await invoke('git_discard_changes', { path });
    await get().refreshGitStatus();
  },
  commitChanges: async (message) => {
    await invoke('git_commit', { message });
    await get().refreshGitStatus();
    await get().refreshGitBranch();
  },
  openDiff: (path) => {
    get().setDiffFile(path);
  },

  loadAvailableWorkspaces: async () => {
    try {
      const workspaces = await invoke<string[]>('workspace_list_contexts');
      set({ availableWorkspaces: workspaces });
    } catch (e) {
      console.error(e);
    }
  },
  loadWorkspaceContext: async (name) => {
    try {
      const context = await invoke<any>('workspace_load_context', { name });
      if (context.root_path) {
        await get().setProjectRoot(context.root_path);
      }
      // Re-hydrate settings if present
      if (context.settings) {
        get().updateSettings(context.settings);
      }
    } catch (e) {
      console.error(e);
    }
  },
  saveWorkspaceContext: async (name) => {
    await invoke('workspace_save_context', {
      name,
      context: {
        root_path: get().currentProjectRoot,
        settings: get().settings,
      },
    });
    await get().loadAvailableWorkspaces();
  },
  deleteWorkspaceContext: async (name) => {
    await invoke('workspace_delete_context', { name });
    await get().loadAvailableWorkspaces();
  },
  refreshSystemStats: async () => {
    try {
      const stats = await invoke<any>('get_sys_info');
      set({ systemStats: stats });
    } catch (e) {
      console.error(e);
    }
  },
  replacePathReference: (_oldPath, _newEntry) => {
    // Component logic uses this to update internal structures when a file is renamed
    // This is often a no-op at the store level unless we track specific things like breakpoints by path
  },
  removePathReferences: (_path) => {
    // No-op shim
  },
  setSearchFilterPath: (path) => set({ searchFilterPath: path }),
});
