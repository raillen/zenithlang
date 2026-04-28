import { Grid3X3, GripHorizontal, Keyboard, MonitorCog, Palette, RotateCcw, X } from "lucide-react";
import { motion } from "framer-motion";
import React, { PointerEvent, useRef, useState } from "react";
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
  const [dragOffset, setDragOffset] = useState({ x: 0, y: 0 });
  const dragStart = useRef<{ pointerId: number; startX: number; startY: number; originX: number; originY: number } | null>(null);

  function beginDrag(event: PointerEvent<HTMLElement>) {
    if ((event.target as Element).closest("button")) return;
    event.currentTarget.setPointerCapture(event.pointerId);
    dragStart.current = {
      pointerId: event.pointerId,
      startX: event.clientX,
      startY: event.clientY,
      originX: dragOffset.x,
      originY: dragOffset.y,
    };
  }

  function moveDrag(event: PointerEvent<HTMLElement>) {
    const current = dragStart.current;
    if (!current || current.pointerId !== event.pointerId) return;
    setDragOffset({
      x: current.originX + event.clientX - current.startX,
      y: current.originY + event.clientY - current.startY,
    });
  }

  function endDrag() {
    dragStart.current = null;
  }

  function resetPreferences() {
    onPreferencesChange({
      gizmoSize: 0.75,
      gridColor: "#4b8dff",
      gridOpacity: 0.55,
      ptzSpeed: 1,
      shortcutTemplate: "blender",
      showGrid: true,
      snapMode: "grid-object",
      theme: "unity-light",
    });
  }

  return (
    <div className="settings-backdrop" role="presentation">
      <motion.section
        animate={{ opacity: 1, scale: 1, x: "-50%", y: "-50%" }}
        className="settings-window"
        initial={{ opacity: 0, scale: 0.96, x: "-50%", y: "-50%" }}
        style={
          {
            "--settings-x": `${dragOffset.x}px`,
            "--settings-y": `${dragOffset.y}px`,
          } as React.CSSProperties
        }
        transition={{ duration: 0.14, ease: [0.16, 1, 0.3, 1] }}
      >
        <header
          className="settings-header"
          onDoubleClick={() => setDragOffset({ x: 0, y: 0 })}
          onPointerCancel={endDrag}
          onPointerDown={beginDrag}
          onPointerMove={moveDrag}
          onPointerUp={endDrag}
        >
          <GripHorizontal className="settings-drag-handle" size={16} strokeWidth={ICON_STROKE} />
          <div>
            <span>Preferences</span>
            <strong>Editor settings</strong>
          </div>
          <button className="settings-reset-button" onClick={resetPreferences} type="button">
            <RotateCcw size={14} strokeWidth={ICON_STROKE} />
            Reset
          </button>
          <button className="top-icon-button" onClick={onClose} title="Close" type="button">
            <X size={15} strokeWidth={ICON_STROKE} />
          </button>
        </header>

        <div className="settings-grid">
          <section className="settings-section">
            <h3><MonitorCog size={15} strokeWidth={ICON_STROKE} /> Viewport</h3>
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
              <span>Pan / orbit / zoom speed</span>
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
          </section>

          <section className="settings-section">
            <h3><Grid3X3 size={15} strokeWidth={ICON_STROKE} /> Grid</h3>
            <label className="settings-toggle-line">
              <span>Show grid</span>
              <input
                checked={preferences.showGrid}
                onChange={(event) => onPreferencesChange({ showGrid: event.target.checked })}
                type="checkbox"
              />
            </label>
            <label>
              <span>Grid color</span>
              <input
                className="settings-color-input"
                onChange={(event) => onPreferencesChange({ gridColor: event.target.value })}
                type="color"
                value={preferences.gridColor}
              />
              <strong>{preferences.gridColor}</strong>
            </label>
            <label>
              <span>Grid opacity</span>
              <input
                max={1}
                min={0.05}
                onChange={(event) => onPreferencesChange({ gridOpacity: Number(event.target.value) })}
                step={0.05}
                type="range"
                value={preferences.gridOpacity}
              />
              <strong>{Math.round(preferences.gridOpacity * 100)}%</strong>
            </label>
          </section>

          <section className="settings-section">
            <h3><Palette size={15} strokeWidth={ICON_STROKE} /> Appearance</h3>
            <label>
              <span>Theme</span>
              <select
                onChange={(event) => onPreferencesChange({ theme: event.target.value as ThemeMode })}
                value={preferences.theme}
              >
                <option value="unity-light">Unity Light</option>
                <option value="codex">Codex</option>
                <option value="xcode">Xcode</option>
                <option value="unity-dark">Unity Dark</option>
              </select>
            </label>
          </section>

          <section className="settings-section">
            <h3><Keyboard size={15} strokeWidth={ICON_STROKE} /> Keybinds</h3>
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
