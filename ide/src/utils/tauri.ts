import { invoke as tauriInvoke } from '@tauri-apps/api/core';

export function isTauriRuntime() {
  return !!(window as Window & { __TAURI_INTERNALS__?: unknown }).__TAURI_INTERNALS__;
}

/**
 * Safe wrapper around the Tauri invoke bridge.
 * In browser-only mode it returns explicit mocks for supported flows and
 * throws for unsupported commands so failures do not get silently swallowed.
 */
export async function safeInvoke<T>(command: string, args?: Record<string, unknown>): Promise<T> {
  const isTauri = isTauriRuntime();

  if (!isTauri) {
    console.warn(`[Tauri Mock] Command "${command}" ignored outside the Tauri runtime.`);

    if (command === 'get_file_tree') {
      return [
        { name: 'src', path: 'src', is_directory: true, children: [] },
        { name: 'README.md', path: 'README.md', is_directory: false }
      ] as unknown as T;
    }

    if (command === 'read_file') {
      return "// Mock content for browser preview\nprint('Hello Zenith!')" as unknown as T;
    }

    if (command === 'write_file') {
      return undefined as T;
    }

    if (command === 'run_compiler') {
      return "Browser preview mode: the Zenith compiler is only available inside the Tauri desktop app." as unknown as T;
    }

    if (command === 'pick_file' || command === 'pick_folder' || command === 'pick_save_path') {
      return null as T;
    }

    throw new Error(`[Tauri Mock] Unsupported command "${command}" outside the Tauri runtime.`);
  }

  return await tauriInvoke<T>(command, args);
}
