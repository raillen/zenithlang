import { invoke as tauriInvoke } from "@tauri-apps/api/core";

// Check if we are running inside a Tauri container
const isTauri = !!(window as any).__TAURI_INTERNALS__;

const MOCK_DATA: Record<string, any> = {
  "get_file_tree": [
    {
      name: "src",
      path: "../src",
      is_directory: true,
      children: [
        { name: "main.zt", path: "../src/main.zt", is_directory: false },
        { name: "compiler.zt", path: "../src/compiler.zt", is_directory: false },
      ]
    },
    {
      name: "tests",
      path: "../tests",
      is_directory: true,
      children: [
        { name: "test_syntax.zt", path: "../tests/test_syntax.zt", is_directory: false },
      ]
    },
    { name: "ztc.lua", path: "../ztc.lua", is_directory: false },
    { name: "README.md", path: "../README.md", is_directory: false },
  ],
  "read_file": `// Zenith Language Prototype
// Welcome to the Retina IDE Preview Mode

func main() {
    print("Zenith is running in Browser Mode!");
    
    let compiler = check Zenith.Compiler.init();
    match compiler.status {
        .Ready => print("Stable Architecture"),
        .Error(e) => print("Error: {e}"),
    }
}
`
};

export async function invoke<T>(command: string, args?: any): Promise<T> {
  if (!isTauri) {
    console.warn(`[Tauri Mock] Command "${command}" invoked in browser mode. Returning mock data.`);
    
    // Simulate latency
    await new Promise(resolve => setTimeout(resolve, 300));

    if (MOCK_DATA[command]) {
      return MOCK_DATA[command] as T;
    }

    if (command === "read_file") return MOCK_DATA["read_file"] as T;
    
    return [] as unknown as T;
  }

  try {
    return await tauriInvoke<T>(command, args);
  } catch (error) {
    console.error(`[Tauri Invoke Error] ${command}:`, error);
    throw error;
  }
}
