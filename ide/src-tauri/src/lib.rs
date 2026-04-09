use serde::{Serialize, Deserialize};
use std::fs;
use std::path::Path;

#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct FileEntry {
    name: String,
    path: String,
    is_directory: bool,
    children: Option<Vec<FileEntry>>,
}

#[tauri::command]
fn get_file_tree(root_path: String) -> Result<Vec<FileEntry>, String> {
    let base_path = if root_path.is_empty() || root_path == "." {
        std::env::current_dir().map_err(|e| e.to_string())?
    } else {
        Path::new(&root_path).to_path_buf()
    };

    if !base_path.exists() {
        return Err(format!("Path does not exist: {}", base_path.display()));
    }
    
    scan_dir(&base_path)
}

fn scan_dir(path: &Path) -> Result<Vec<FileEntry>, String> {
    let mut entries = Vec::new();

    if let Ok(read_dir) = fs::read_dir(path) {
        for entry in read_dir.flatten() {
            let path = entry.path();
            let name = entry.file_name().to_string_lossy().to_string();

            if name == ".git" || name == "node_modules" || name == "target" || name == "dist" || name == ".tauri" {
                continue;
            }

            let is_directory = path.is_dir();
            let children = if is_directory {
                Some(scan_dir(&path)?)
            } else {
                None
            };

            entries.push(FileEntry {
                name,
                path: path.to_string_lossy().to_string(),
                is_directory,
                children,
            });
        }
    }

    entries.sort_by(|a, b| {
        if a.is_directory != b.is_directory {
            b.is_directory.cmp(&a.is_directory)
        } else {
            a.name.to_lowercase().cmp(&b.name.to_lowercase())
        }
    });

    Ok(entries)
}

#[tauri::command]
fn read_file(path: String) -> Result<String, String> {
    fs::read_to_string(path).map_err(|e| e.to_string())
}

#[tauri::command]
fn run_compiler(input_path: String) -> Result<String, String> {
    use std::process::Command;
    
    let output = Command::new("lua")
        .arg("../ztc.lua")
        .arg(input_path)
        .output()
        .map_err(|e| format!("Failed to execute lua: {}", e))?;

    let stdout = String::from_utf8_lossy(&output.stdout).to_string();
    let stderr = String::from_utf8_lossy(&output.stderr).to_string();

    if output.status.success() {
        Ok(stdout)
    } else {
        Err(format!("Error: {}\n{}", stdout, stderr))
    }
}

#[tauri::command]
fn greet(name: &str) -> String {
    format!("Hello, {}! You've been greeted from Zenith App!", name)
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .invoke_handler(tauri::generate_handler![
            greet, 
            get_file_tree, 
            read_file, 
            run_compiler
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
