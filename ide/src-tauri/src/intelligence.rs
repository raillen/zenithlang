use serde::Serialize;
use std::fs;
use std::path::Path;
use crate::fs_ops::{WorkspaceIndex, workspace_root};
use regex::Regex;

#[derive(Serialize)]
pub struct SearchMatch {
    pub line_number: usize,
    pub line_content: String,
}

#[derive(Serialize)]
pub struct FileResult {
    pub file_path: String,
    pub matches: Vec<SearchMatch>,
}

#[derive(Serialize)]
#[serde(rename_all = "camelCase")]
pub struct FileNameResult {
    pub name: String,
    pub path: String,
}

#[derive(Serialize, Clone, Debug)]
pub enum SymbolKind {
    Function,
    Struct,
    Variable,
    Constant,
    Interface,
}

#[derive(Serialize, Clone, Debug)]
pub struct Symbol {
    pub name: String,
    pub kind: SymbolKind,
    pub line: usize,
    pub col: usize,
}

#[tauri::command]
pub fn search_in_files(
    state: tauri::State<'_, WorkspaceIndex>,
    query: String,
    is_regex: bool,
    match_case: bool,
    whole_word: bool,
    path_filter: Option<String>,
) -> Result<Vec<FileResult>, String> {
    let mut all_results = Vec::new();
    let root = state.root.read().map_err(|_| "Failed to lock root")?;
    let files = state.files.read().map_err(|_| "Failed to lock index")?;

    let filter_path = path_filter.map(|p| root.join(p.replace("\\", "/")));
    let query_lower = if !match_case { query.to_lowercase() } else { query.clone() };

    let re = if is_regex || whole_word {
        let pattern = if is_regex {
            query.clone()
        } else {
            format!(r"\b{}\b", regex::escape(&query))
        };

        match regex::RegexBuilder::new(&pattern).case_insensitive(!match_case).build() {
            Ok(r) => Some(r),
            Err(e) => return Err(format!("Invalid search pattern: {}", e)),
        }
    } else {
        None
    };

    for path_str in files.iter() {
        let path = Path::new(path_str);

        if let Some(ref filter) = filter_path {
            if !path.starts_with(filter) {
                continue;
            }
        }

        let name = path.file_name().map(|n| n.to_string_lossy().to_string()).unwrap_or_default();
        if name.ends_with(".db") || name.ends_with(".png") || name.ends_with(".jpg") || name.ends_with(".woff2") || name.ends_with(".exe") {
            continue;
        }

        if let Ok(content) = fs::read_to_string(&path) {
            let mut file_matches = Vec::new();
            for (i, line) in content.lines().enumerate() {
                let matched = if let Some(ref r) = re {
                    r.is_match(line)
                } else if match_case {
                    line.contains(&query)
                } else {
                    line.to_lowercase().contains(&query_lower)
                };

                if matched {
                    file_matches.push(SearchMatch {
                        line_number: i + 1,
                        line_content: line.trim().to_string(),
                    });
                }

                if file_matches.len() >= 100 { break; }
            }

            if !file_matches.is_empty() {
                let clean_path = path.strip_prefix(&*root).unwrap_or(&path).to_string_lossy().to_string();
                all_results.push(FileResult {
                    file_path: clean_path.replace("\\", "/"),
                    matches: file_matches,
                });
            }
        }

        if all_results.len() >= 200 { break; }
    }

    all_results.sort_by(|a, b| a.file_path.cmp(&b.file_path));
    Ok(all_results)
}

#[tauri::command]
pub fn replace_in_files(
    state: tauri::State<'_, WorkspaceIndex>,
    query: String,
    replacement: String,
    is_regex: bool,
    match_case: bool,
    whole_word: bool,
    target_files: Vec<String>,
) -> Result<usize, String> {
    let root = state.root.read().map_err(|_| "Failed to lock root")?;
    let mut replaced_count = 0;

    let re = if is_regex || whole_word {
        let pattern = if is_regex {
            query.clone()
        } else {
            format!(r"\b{}\b", regex::escape(&query))
        };

        match regex::RegexBuilder::new(&pattern).case_insensitive(!match_case).build() {
            Ok(r) => Some(r),
            Err(e) => return Err(format!("Invalid search pattern: {}", e)),
        }
    } else {
        None
    };

    for rel_path in target_files {
        let abs_path = root.join(rel_path.replace("\\", "/"));
        if !abs_path.exists() { continue; }

        if let Ok(content) = fs::read_to_string(&abs_path) {
            let new_content = if let Some(ref r) = re {
                r.replace_all(&content, &*replacement).to_string()
            } else if match_case {
                content.replace(&query, &replacement)
            } else {
                let r_lite = regex::RegexBuilder::new(&regex::escape(&query))
                    .case_insensitive(true)
                    .build()
                    .map_err(|e| e.to_string())?;
                r_lite.replace_all(&content, &*replacement).to_string()
            };

            if new_content != content {
                fs::write(&abs_path, new_content).map_err(|e| format!("Failed to write to file: {}", e))?;
                replaced_count += 1;
            }
        }
    }

    Ok(replaced_count)
}

#[tauri::command]
pub fn search_file_names(
    state: tauri::State<'_, WorkspaceIndex>,
    query: String
) -> Result<Vec<FileNameResult>, String> {
    let query_lower = query.to_lowercase();
    let files = state.files.read().map_err(|_| "Failed to lock index")?;
    let root = state.root.read().map_err(|_| "Failed to lock root")?;

    let mut results = Vec::new();

    for path_str in files.iter() {
        let path = Path::new(path_str);
        let name = path.file_name().map(|n| n.to_string_lossy().to_string()).unwrap_or_default();

        if query_lower.is_empty() || name.to_lowercase().contains(&query_lower) {
            let relative_path = path.strip_prefix(&*root).unwrap_or(path).to_string_lossy().to_string();
            results.push(FileNameResult {
                name,
                path: relative_path.replace("\\", "/"),
            });
        }

        if results.len() >= 100 {
            break;
        }
    }

    results.sort_by(|a, b| a.name.cmp(&b.name));
    Ok(results)
}

#[tauri::command]
pub fn get_file_symbols(content: String) -> Result<Vec<Symbol>, String> {
    let mut symbols = Vec::new();

    let func_re = Regex::new(r"(?m)^func\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*\(").unwrap();
    let struct_re = Regex::new(r"(?m)^struct\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*").unwrap();
    let const_re = Regex::new(r"(?m)^const\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*[:=]").unwrap();
    let var_re = Regex::new(r"(?m)^var\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*[:=]").unwrap();
    let interface_re = Regex::new(r"(?m)^interface\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*\{").unwrap();

    let patterns = [
        (&func_re, SymbolKind::Function),
        (&struct_re, SymbolKind::Struct),
        (&const_re, SymbolKind::Constant),
        (&var_re, SymbolKind::Variable),
        (&interface_re, SymbolKind::Interface),
    ];

    for (re, kind) in patterns {
        for cap in re.captures_iter(&content) {
            let name = cap[1].to_string();
            let pos = cap.get(1).unwrap().start();

            let line = content[..pos].lines().count();
            let col = content[..pos].lines().last().map(|l| l.len() + 1).unwrap_or(1);

            symbols.push(Symbol {
                name,
                kind: kind.clone(),
                line,
                col,
            });
        }
    }

    symbols.sort_by_key(|s| s.line);
    Ok(symbols)
}

#[tauri::command]
pub fn run_compiler(input_path: String) -> Result<String, String> {
    use std::process::Command;
    let resolved_input_path = crate::fs_ops::resolve_path(&input_path)?;
    let project_root = workspace_root();
    let compiler_path = project_root.join("ztc.lua");

    if !compiler_path.exists() {
        return Err(format!("Compiler script not found at {}", compiler_path.display()));
    }

    let output = Command::new("lua")
        .arg(&compiler_path)
        .arg(&resolved_input_path)
        .current_dir(&project_root)
        .output()
        .map_err(|e| format!("Failed to run compiler: {}", e))?;

    let stdout = String::from_utf8_lossy(&output.stdout).to_string();
    let stderr = String::from_utf8_lossy(&output.stderr).to_string();

    if !output.status.success() {
        return Err(stderr);
    }
    Ok(stdout)
}

#[tauri::command]
pub fn run_diagnostics(_path: String) -> Result<String, String> {
    // Basic local diagnostics or hooks for external tools
    Ok("No diagnostics".to_string())
}
