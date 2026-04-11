import { useRef } from "react";
import { X } from "lucide-react";
import { useWorkspaceStore, FileEntry } from "../store/useWorkspaceStore";

export function TabManager() {
  const {
    openFiles,
    previewFile,
    activeFile,
    primaryFile,
    secondaryFile,
    focusedPane,
    splitMode,
    openFile,
    closeFile,
    dirtyFiles,
  } = useWorkspaceStore();
  const tabRefs = useRef<Array<HTMLButtonElement | null>>([]);

  const allTabs = [...openFiles];
  if (previewFile && !openFiles.find((file) => file.path === previewFile.path)) {
    allTabs.push(previewFile);
  }

  if (allTabs.length === 0) return null;

  const handleTabClick = (file: FileEntry) => {
    openFile(file, true);
  };

  const handleClose = (event: React.MouseEvent, path: string) => {
    event.stopPropagation();
    closeFile(path);
  };

  const focusTab = (index: number) => {
    window.requestAnimationFrame(() => {
      tabRefs.current[index]?.focus();
    });
  };

  const handleTabKeyDown = (event: React.KeyboardEvent, index: number, file: FileEntry) => {
    if (event.key === "ArrowRight") {
      event.preventDefault();
      focusTab((index + 1) % allTabs.length);
      return;
    }

    if (event.key === "ArrowLeft") {
      event.preventDefault();
      focusTab((index - 1 + allTabs.length) % allTabs.length);
      return;
    }

    if (event.key === "Home") {
      event.preventDefault();
      focusTab(0);
      return;
    }

    if (event.key === "End") {
      event.preventDefault();
      focusTab(allTabs.length - 1);
      return;
    }

    if (event.key === "Delete" || event.key === "Backspace") {
      event.preventDefault();
      closeFile(file.path);
      const nextIndex = index >= allTabs.length - 1 ? Math.max(0, index - 1) : index;
      focusTab(nextIndex);
      return;
    }

    if (event.key === "Enter" || event.key === " ") {
      event.preventDefault();
      handleTabClick(file);
    }
  };

  return (
    <div
      role="tablist"
      aria-label="Open editors"
      className="flex bg-ide-panel border-b border-ide-border h-9 overflow-x-auto no-scrollbar select-none transition-colors duration-200"
    >
      {allTabs.map((file, index) => {
        const isActive = activeFile?.path === file.path;
        const isPreview = previewFile?.path === file.path;
        const isDirty = dirtyFiles.has(file.path);
        const isInPrimaryPane = primaryFile?.path === file.path;
        const isInSecondaryPane = secondaryFile?.path === file.path;
        const paneDescription =
          isInPrimaryPane && isInSecondaryPane
            ? "open in both panes"
            : isInSecondaryPane
              ? "open in secondary pane"
              : "open in primary pane";
        const controlledPane =
          isInSecondaryPane && focusedPane === "secondary" ? "editor-pane-secondary" : "editor-pane-primary";

        return (
          <div
            key={file.path}
            onAuxClick={(event) => event.button === 1 && closeFile(file.path)}
            className={`relative flex items-stretch h-full border-r border-ide-border min-w-[148px] max-w-[240px] group ${
              isActive ? "bg-ide-bg" : "hover:bg-black/5"
            }`}
          >
            {isActive && <div className="absolute top-0 left-0 right-0 h-[2px] bg-primary" aria-hidden="true" />}

            <button
              ref={(element) => {
                tabRefs.current[index] = element;
              }}
              role="tab"
              aria-selected={isActive}
              aria-controls={controlledPane}
              aria-label={`${file.name}, ${paneDescription}${isPreview ? ", preview tab" : ""}`}
              tabIndex={isActive ? 0 : -1}
              onClick={() => handleTabClick(file)}
              onKeyDown={(event) => handleTabKeyDown(event, index, file)}
              className="flex flex-1 items-center gap-2 px-4 min-w-0"
            >
              <span
                className={`text-[11px] truncate flex-1 text-left ${
                  isActive ? "text-ide-text font-medium" : "text-ide-text-dim"
                } ${isPreview ? "italic opacity-80" : ""}`}
              >
                {file.name}
              </span>

              {splitMode !== "single" && (isInPrimaryPane || isInSecondaryPane) && (
                <div className="flex items-center gap-1" aria-hidden="true">
                  {isInPrimaryPane && <PaneBadge label="P" active={isActive && focusedPane === "primary"} />}
                  {isInSecondaryPane && <PaneBadge label="S" active={isActive && focusedPane === "secondary"} />}
                </div>
              )}

              <div className="flex items-center justify-center w-4 h-4" aria-hidden="true">
                {isDirty ? <div className="w-2 h-2 rounded-full bg-amber-500 group-hover:hidden" /> : null}
              </div>
            </button>

            <button
              type="button"
              onClick={(event) => handleClose(event, file.path)}
              aria-label={`Close ${file.name}`}
              className={`
                mr-1 self-center p-0.5 rounded-md hover:bg-black/10 text-ide-text-dim transition-opacity
                ${isDirty ? "opacity-0 group-hover:opacity-100" : "opacity-0 group-hover:opacity-100 focus:opacity-100"}
              `}
            >
              <X size={10} />
            </button>
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
