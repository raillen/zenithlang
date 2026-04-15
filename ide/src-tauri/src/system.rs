use serde::{Deserialize, Serialize};
use sysinfo::System;
use tauri::{AppHandle, Manager};

#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct SysInfo {
    pub cpu: f32,
    pub memory: u64,      // total bytes
    pub memory_used: u64, // used bytes
}

#[tauri::command]
pub fn get_sys_info() -> Result<SysInfo, String> {
    let mut sys = System::new_all();
    sys.refresh_cpu_usage(); // Correct for 0.30
    sys.refresh_memory();
    
    let cpu_sum: f32 = sys.cpus().iter().map(|cpu| cpu.cpu_usage()).sum();
    let cpu_avg = if !sys.cpus().is_empty() { cpu_sum / sys.cpus().len() as f32 } else { 0.0 };
    
    Ok(SysInfo {
        cpu: cpu_avg,
        memory: sys.total_memory(),
        memory_used: sys.used_memory(),
    })
}

#[tauri::command]
pub async fn open_theme_lab(app: AppHandle) -> Result<(), String> {
    let lab_window = app.get_webview_window("theme-lab");
    
    if let Some(window) = lab_window {
        window.set_focus().map_err(|e| e.to_string())?;
    } else {
        let _ = tauri::WebviewWindowBuilder::new(
            &app,
            "theme-lab",
            tauri::WebviewUrl::App("index.html?window=theme-lab".into()),
        )
        .title("Zenith Theme Lab")
        .inner_size(480.0, 820.0)
        .resizable(true)
        .transparent(true)
        .decorations(true)
        .build()
        .map_err(|e| e.to_string())?;
    }
    
    Ok(())
}

#[tauri::command]
pub async fn pick_file() -> Result<Option<String>, String> {
    let res = tauri::async_runtime::spawn_blocking(|| {
        rfd::FileDialog::new().pick_file()
    }).await.map_err(|e| e.to_string())?;
    
    Ok(res.map(|f| f.display().to_string()))
}

#[tauri::command]
pub async fn pick_folder() -> Result<Option<String>, String> {
    let res = tauri::async_runtime::spawn_blocking(|| {
        rfd::FileDialog::new().pick_folder()
    }).await.map_err(|e| e.to_string())?;
    
    Ok(res.map(|f| f.display().to_string()))
}

#[tauri::command]
pub async fn pick_save_path() -> Result<Option<String>, String> {
    let res = tauri::async_runtime::spawn_blocking(|| {
        rfd::FileDialog::new().save_file()
    }).await.map_err(|e| e.to_string())?;
    
    Ok(res.map(|f| f.display().to_string()))
}

#[tauri::command]
pub fn greet(name: &str) -> String {
    format!("Hello, {}! You've been greeted from Zenith App!", name)
}
