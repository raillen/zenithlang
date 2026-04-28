import { X } from "lucide-react";
import { motion } from "framer-motion";
import React from "react";
import { ICON_STROKE } from "../constants";
import type { ShortcutTemplate, SnapMode, StudioPreferences, ThemeMode } from "../types";
import { keybindsForTemplate } from "../utils/keybinds";

export function SettingsWindow({
  preferences,
  onClose,
  onPreferencesChange,
}: {
  preferences: StudioPreferences;
  onClose: () => void;
  onPreferencesChange: (patch: Partial<StudioPreferences>) => void;
}) {
  const keybindRows = keybindsForTemplate(preferences.shortcutTemplate);

  return (
    <div className="settings-backdrop" role="presentation">
      <motion.section
        animate={{ opacity: 1, scale: 1 }}
        className="settings-window"
        initial={{ opacity: 0, scale: 0.96 }}
        transition={{ duration: 0.14, ease: [0.16, 1, 0.3, 1] }}
      >
        <header className="settings-header">
          <div>
            <span>Preferences</span>
            <strong>Editor settings</strong>
          </div>
          <button className="top-icon-button" onClick={onClose} title="Close" type="button">
            <X size={15} strokeWidth={ICON_STROKE} />
          </button>
        </header>

        <div className="settings-grid">
          <section className="settings-section">
            <h3>Viewport</h3>
            <label>
              <span>Gizmo size</span>
              <input
                max={1.8}
                min={0.35}
                onChange={(event) => onPreferencesChange({ gizmoSize: Number(event.target.value) })}
                step={0.05}
                type="range"
                value={preferences.gizmoSize}
              />
              <strong>{preferences.gizmoSize.toFixed(2)}</strong>
            </label>
            <label>
              <span>PTZ speed</span>
              <input
                max={2.4}
                min={0.25}
                onChange={(event) => onPreferencesChange({ ptzSpeed: Number(event.target.value) })}
                step={0.05}
                type="range"
                value={preferences.ptzSpeed}
              />
              <strong>{preferences.ptzSpeed.toFixed(2)}</strong>
            </label>
            <label>
              <span>Default snap</span>
              <select
                onChange={(event) => onPreferencesChange({ snapMode: event.target.value as SnapMode })}
                value={preferences.snapMode}
              >
                <option value="grid">Grid</option>
                <option value="object">Object</option>
                <option value="grid-object">Grid + object</option>
              </select>
            </label>
            <label>
              <span>Theme</span>
              <select
                onChange={(event) => onPreferencesChange({ theme: event.target.value as ThemeMode })}
                value={preferences.theme}
              >
                <option value="codex">Codex</option>
                <option value="xcode">Xcode</option>
                <option value="unity-dark">Unity Dark</option>
              </select>
            </label>
          </section>

          <section className="settings-section">
            <h3>Keybinds</h3>
            <label>
              <span>Template</span>
              <select
                onChange={(event) => onPreferencesChange({ shortcutTemplate: event.target.value as ShortcutTemplate })}
                value={preferences.shortcutTemplate}
              >
                <option value="blender">Blender</option>
                <option value="3dsmax">3ds Max</option>
                <option value="maya">Maya</option>
              </select>
            </label>
            <div className="keybind-list">
              {keybindRows.map((row) => (
                <div className="keybind-row" key={`${row.action}-${row.keys}`}>
                  <span>{row.action}</span>
                  <kbd>{row.keys}</kbd>
                </div>
              ))}
            </div>
          </section>
        </div>
      </motion.section>
    </div>
  );
}
