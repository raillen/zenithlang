import { invoke as tauriInvoke } from '@tauri-apps/api/core';

/**
 * Wrapper seguro para o invoke do Tauri.
 * Se a aplicação estiver rodando em um navegador comum (sem o bridge do Tauri),
 * ele retorna um mock ou um erro amigável em vez de crashar a aplicação.
 */
export async function safeInvoke<T>(command: string, args?: Record<string, unknown>): Promise<T> {
  const isTauri = !!(window as any).__TAURI_INTERNALS__;

  if (!isTauri) {
    console.warn(`[Tauri Mock] Chamada ao comando "${command}" ignorada (fora do ambiente Tauri).`);
    
    // Mocks básicos para desenvolvimento no browser
    if (command === 'get_file_tree') {
      return [
        { name: 'src', path: 'src', is_directory: true, children: [] },
        { name: 'README.md', path: 'README.md', is_directory: false }
      ] as unknown as T;
    }
    
    if (command === 'read_file') {
      return "// Mock content for browser preview\nprint('Hello Zenith!')" as unknown as T;
    }

    return null as unknown as T;
  }

  return await tauriInvoke<T>(command, args);
}
