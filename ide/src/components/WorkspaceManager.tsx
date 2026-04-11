import { useState } from "react";
import { useWorkspaceStore } from "../store/useWorkspaceStore";
import { Layout, Plus, Trash2, RotateCcw } from "lucide-react";

export function WorkspaceManager() {
  const {
    availableWorkspaces,
    saveWorkspaceContext,
    loadWorkspaceContext,
    deleteWorkspaceContext
  } = useWorkspaceStore();

  const [newContextName, setNewContextName] = useState("");
  const [isSaving, setIsSaving] = useState(false);

  const handleSave = async () => {
    if (!newContextName.trim()) return;
    setIsSaving(true);
    await saveWorkspaceContext(newContextName.trim());
    setNewContextName("");
    setIsSaving(false);
  };

  return (
    <div className="flex flex-col h-full bg-ide-panel">
      {/* Background Decor */}
      <div className="absolute inset-0 opacity-[0.02] pointer-events-none">
        <Layout className="w-64 h-64 -bottom-10 -right-10 absolute rotate-12" />
      </div>

      <div className="p-4 flex flex-col gap-4 relative z-10">
        <div className="flex flex-col gap-2">
          <label className="text-[10px] font-bold uppercase tracking-wider text-ide-text-dim">
            Save Current Context
          </label>
          <div className="flex gap-1">
            <input
              type="text"
              value={newContextName}
              onChange={(e) => setNewContextName(e.target.value)}
              placeholder="Context name..."
              className="flex-1 bg-black/10 border border-ide-border rounded px-2 py-1.5 text-[12px] focus:outline-none focus:ring-1 focus:ring-primary/30"
              onKeyDown={(e) => e.key === "Enter" && handleSave()}
            />
            <button
              onClick={handleSave}
              disabled={isSaving || !newContextName.trim()}
              className="bg-primary/20 hover:bg-primary/30 text-primary p-1.5 rounded transition-colors disabled:opacity-50"
            >
              <Plus size={16} />
            </button>
          </div>
        </div>

        <div className="flex flex-col gap-2">
          <label className="text-[10px] font-bold uppercase tracking-wider text-ide-text-dim mt-2">
            Saved Contexts
          </label>
          {availableWorkspaces.length === 0 ? (
            <div className="p-4 border border-dashed border-ide-border rounded text-center">
              <p className="text-[11px] text-ide-text-dim italic">No contexts saved yet.</p>
            </div>
          ) : (
            <div className="flex flex-col gap-1">
              {availableWorkspaces.map((name) => (
                <div
                  key={name}
                  className="group flex items-center justify-between p-2 rounded hover:bg-black/10 transition-all border border-transparent hover:border-ide-border"
                >
                  <button
                    onClick={() => loadWorkspaceContext(name)}
                    className="flex items-center gap-2 flex-1 text-left"
                  >
                    <div className="w-6 h-6 rounded bg-primary/10 flex items-center justify-center text-primary">
                      <Layout size={12} />
                    </div>
                    <span className="text-[12px] font-medium text-ide-text truncate">
                      {name}
                    </span>
                  </button>

                  <div className="flex items-center opacity-0 group-hover:opacity-100 transition-opacity gap-1">
                    <button
                      onClick={() => loadWorkspaceContext(name)}
                      title="Load Context"
                      className="p-1 hover:text-primary transition-colors"
                    >
                      <RotateCcw size={14} />
                    </button>
                    <button
                      onClick={() => deleteWorkspaceContext(name)}
                      title="Delete Context"
                      className="p-1 hover:text-red-500 transition-colors"
                    >
                      <Trash2 size={14} />
                    </button>
                  </div>
                </div>
              ))}
            </div>
          )}
        </div>
      </div>
    </div>
  );
}