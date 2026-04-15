use serde::{Deserialize, Serialize};
use std::process::Command;
use std::path::Path;

#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct GitStatusEntry {
    pub path: String,
    pub status: String, // "added", "modified", "deleted", "untracked", "staged"
}

#[tauri::command]
pub fn get_git_status(path: String) -> Result<Vec<GitStatusEntry>, String> {
    let output = Command::new("git")
        .args(["status", "--porcelain"])
        .current_dir(Path::new(&path))
        .output()
        .map_err(|e| format!("Failed to run git status: {}", e))?;

    if !output.status.success() {
        return Ok(Vec::new()); // Not a git repo or other error
    }

    let stdout = String::from_utf8_lossy(&output.stdout);
    let mut entries = Vec::new();

    for line in stdout.lines() {
        if line.len() < 4 { continue; }
        let status_code = &line[0..2];
        let file_path = &line[3..];

        let status = match status_code {
            "M " | "A " | "D " | "R " | "C " => "staged",
            " M" => "modified",
            " D" => "deleted",
            "??" => "untracked",
            _ => "modified",
        };

        entries.push(GitStatusEntry {
            path: file_path.to_string(),
            status: status.to_string(),
        });
    }

    Ok(entries)
}

#[tauri::command]
pub fn get_git_branch(path: String) -> Result<String, String> {
    let output = Command::new("git")
        .args(["rev-parse", "--abbrev-ref", "HEAD"])
        .current_dir(Path::new(&path))
        .output()
        .map_err(|e| format!("Failed to get git branch: {}", e))?;

    if !output.status.success() {
        return Ok("main".to_string());
    }

    Ok(String::from_utf8_lossy(&output.stdout).trim().to_string())
}

#[tauri::command]
pub fn git_stage(workspace_path: String, file_path: String) -> Result<(), String> {
    let output = Command::new("git")
        .args(["add", &file_path])
        .current_dir(Path::new(&workspace_path))
        .output()
        .map_err(|e| format!("Failed to stage file: {}", e))?;

    if !output.status.success() {
        return Err(String::from_utf8_lossy(&output.stderr).to_string());
    }
    Ok(())
}

#[tauri::command]
pub fn git_unstage(workspace_path: String, file_path: String) -> Result<(), String> {
    let output = Command::new("git")
        .args(["reset", "HEAD", &file_path])
        .current_dir(Path::new(&workspace_path))
        .output()
        .map_err(|e| format!("Failed to unstage file: {}", e))?;

    if !output.status.success() {
        return Err(String::from_utf8_lossy(&output.stderr).to_string());
    }
    Ok(())
}

#[tauri::command]
pub fn git_stage_all(workspace_path: String) -> Result<(), String> {
    let output = Command::new("git")
        .args(["add", "."])
        .current_dir(Path::new(&workspace_path))
        .output()
        .map_err(|e| format!("Failed to stage all: {}", e))?;

    if !output.status.success() {
        return Err(String::from_utf8_lossy(&output.stderr).to_string());
    }
    Ok(())
}

#[tauri::command]
pub fn git_unstage_all(workspace_path: String) -> Result<(), String> {
    let output = Command::new("git")
        .args(["reset", "HEAD"])
        .current_dir(Path::new(&workspace_path))
        .output()
        .map_err(|e| format!("Failed to unstage all: {}", e))?;

    if !output.status.success() {
        return Err(String::from_utf8_lossy(&output.stderr).to_string());
    }
    Ok(())
}

#[tauri::command]
pub fn git_commit(workspace_path: String, message: String) -> Result<(), String> {
    let output = Command::new("git")
        .args(["commit", "-m", &message])
        .current_dir(Path::new(&workspace_path))
        .output()
        .map_err(|e| format!("Failed to commit: {}", e))?;

    if !output.status.success() {
        return Err(String::from_utf8_lossy(&output.stderr).to_string());
    }
    Ok(())
}

#[tauri::command]
pub fn git_discard_changes(workspace_path: String, file_path: String) -> Result<(), String> {
    let output = Command::new("git")
        .args(["checkout", "--", &file_path])
        .current_dir(Path::new(&workspace_path))
        .output()
        .map_err(|e| format!("Failed to discard changes: {}", e))?;

    if !output.status.success() {
        return Err(String::from_utf8_lossy(&output.stderr).to_string());
    }
    Ok(())
}

#[tauri::command]
pub fn git_read_original(workspace_path: String, file_path: String) -> Result<String, String> {
    let output = Command::new("git")
        .args(["show", format!("HEAD:{}", file_path).as_str()])
        .current_dir(Path::new(&workspace_path))
        .output()
        .map_err(|e| format!("Failed to read original: {}", e))?;

    if !output.status.success() {
        return Err(String::from_utf8_lossy(&output.stderr).to_string());
    }

    Ok(String::from_utf8_lossy(&output.stdout).to_string())
}
