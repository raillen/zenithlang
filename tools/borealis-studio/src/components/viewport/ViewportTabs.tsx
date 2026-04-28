import { ChevronDown } from "lucide-react";
import React from "react";
import { ICON_STROKE } from "../../constants";
import type { ViewportTab } from "../../types";

export function ViewportTabs({ activeTab, onChange }: { activeTab: ViewportTab; onChange: (tab: ViewportTab) => void }) {
  return (
    <div className="viewport-tab-strip">
      {(["scene", "game"] as ViewportTab[]).map((tab) => (
        <button
          className={activeTab === tab ? "active" : ""}
          key={tab}
          onClick={() => onChange(tab)}
          type="button"
        >
          <span>{tab === "scene" ? "Scene" : "Game"}</span>
        </button>
      ))}
      <button className="viewport-tab-dropdown" title="Viewport tabs" type="button">
        <ChevronDown size={13} strokeWidth={ICON_STROKE} />
      </button>
    </div>
  );
}
