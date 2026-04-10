import { X } from "lucide-react";
import { useWorkspaceStore, FileEntry } from "../store/useWorkspaceStore";

export function TabManager() {
  const {
    openFiles,
    previewFile,
    activeFile,
    primaryFile,
    secondaryFile,
    splitMode,
    openFile,
    closeFile,
    dirtyFiles,
  } = useWorkspaceStore();

  const handleTabClick = (file: FileEntry) => {
    openFile(file, true); // Clicking a tab makes it fixed if it wasn't
  };

  const handleClose = (e: React.MouseEvent, path: string) => {
    e.stopPropagation();
    closeFile(path);
  };

  // Combine preview and open files for the tab bar
  const allTabs = [...openFiles];
  if (previewFile && !openFiles.find(f => f.path === previewFile.path)) {
    allTabs.push(previewFile);
  }

  if (allTabs.length === 0) return null;

  return (
    <div className="flex bg-ide-panel border-b border-ide-border h-9 overflow-x-auto no-scrollbar select-none transition-colors duration-200">
      {allTabs.map((file) => {
        const isActive = activeFile?.path === file.path;
        const isPreview = previewFile?.path === file.path;
        const isDirty = dirtyFiles.has(file.path);
        const isInPrimaryPane = primaryFile?.path === file.path;
        const isInSecondaryPane = secondaryFile?.path === file.path;

        return (
          <div
            key={file.path}
            onClick={() => handleTabClick(file)}
            onAuxClick={(e) => e.button === 1 && closeFile(file.path)} // Middle click to close
            className={`
              relative flex items-center h-full px-4 gap-2 border-r border-ide-border cursor-pointer transition-all
              min-w-[120px] max-w-[200px] group
              ${isActive ? 'bg-ide-bg' : 'hover:bg-black/5'}
            `}
          >
            {/* Active Indicator Line */}
            {isActive && (
              <div className="absolute top-0 left-0 right-0 h-[2px] bg-primary" />
            )}

            <span className={`
              text-[11px] truncate flex-1
              ${isActive ? 'text-ide-text font-medium' : 'text-ide-text-dim'}
              ${isPreview ? 'italic opacity-80' : ''}
            `}>
              {file.name}
            </span>

            {splitMode !== "single" && (isInPrimaryPane || isInSecondaryPane) && (
              <div className="flex items-center gap-1">
                {isInPrimaryPane && <PaneBadge label="P" active={isActive && !isInSecondaryPane} />}
                {isInSecondaryPane && <PaneBadge label="S" active={isActive && !isInPrimaryPane} />}
              </div>
            )}

            {/* Dirty/Close Button */}
            <div className="flex items-center justify-center w-4 h-4">
              {isDirty ? (
                <div className="w-2 h-2 rounded-full bg-amber-500 group-hover:hidden" />
              ) : null}
              
              <button
                onClick={(e) => handleClose(e, file.path)}
                className={`
                  p-0.5 rounded-md hover:bg-black/10 text-ide-text-dim opacity-0 group-hover:opacity-100 transition-opacity
                  ${isDirty ? 'hidden group-hover:block' : 'block'}
                `}
              >
                <X size={10} />
              </button>
            </div>
          </div>
        );
      })}
    </div>
  );
}

function PaneBadge({ label, active }: { label: string; active: boolean }) {
  return (
    <span
      className={`
        inline-flex h-4 min-w-4 items-center justify-center rounded-md px-1 text-[9px] font-bold uppercase tracking-wide
        ${active ? "bg-primary/15 text-primary" : "bg-ide-bg text-ide-text-dim"}
      `}
    >
      {label}
    </span>
  );
}
