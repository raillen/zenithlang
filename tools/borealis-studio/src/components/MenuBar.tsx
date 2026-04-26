import React from "react";
import type { BridgeStatus } from "../types";
import { StatusPill } from "./shared";

export function MenuBar({
  bridgeStatus,
  projectName,
  onOpenDefaultProject,
  onOpenSettings,
  onShowHome,
}: {
  bridgeStatus: BridgeStatus;
  projectName: string;
  onOpenDefaultProject: () => void;
  onOpenSettings: () => void;
  onShowHome: () => void;
}) {
  const menus = [
    { label: "Start", onClick: onShowHome },
    { label: "Open", onClick: () => void onOpenDefaultProject() },
    { label: "Edit", onClick: onOpenSettings },
    { label: "Assets" },
    { label: "GameObject" },
    { label: "Component" },
    { label: "Window" },
    { label: "Help", onClick: onShowHome },
  ];

  return (
    <nav aria-label="Application menu" className="menu-bar visible" data-tauri-drag-region>
      <div className="menu-items">
        {menus.map((menu) => (
          <button key={menu.label} onClick={menu.onClick}>
            {menu.label}
          </button>
        ))}
      </div>
      <span className="menu-project">{projectName}</span>
      <StatusPill tone={bridgeStatus === "tauri" ? "good" : "warn"}>
        {bridgeStatus === "tauri" ? "Tauri" : "Browser"}
      </StatusPill>
    </nav>
  );
}
