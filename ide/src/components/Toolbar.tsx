import { useEffect, type ChangeEvent, type ReactNode } from "react";
import {
  AlertCircle,
  ChevronDown,
  ChevronRight,
  Columns2,
  GitBranch,
  Hammer,
  Layout,
  PanelLeftClose,
  PanelLeftOpen,
  PanelRightClose,
  PanelRightOpen,
  Play,
  Search,
  Settings,
  Square,
  Rows3,
} from "lucide-react";
import { useWorkspaceStore, FileEntry } from "../store/useWorkspaceStore";
import { useCommandStore } from "../store/useCommandStore";
import { useTranslation } from "../utils/i18n";
import { ZenithMenu } from "./ZenithMenu";
import { useExecutionStore } from "../store/useExecutionStore";
import {
  getDefaultRunTarget,
  getRunTargetOptions,
  getRuntimeForFile,
} from "../utils/pipelineTargets";
import { getRuntimeProfileForSettingsKey } from "../utils/runtimeProfiles";
import {
  executePipelineAction,
  stopActivePipelineTask,
} from "../utils/pipelineActions";

export function Toolbar() {
  const {
    dirtyFiles,
    diagnosticsMap,
    activeFile,
    currentProjectRoot,
    setBottomPanelOpen,
    setBottomTab,
    isBottomPanelOpen,
    isInspectorOpen,
    isSidebarOpen,
    setSidebarOpen,
    setSidebarTab,
    toggleSidebar,
    setInspectorOpen,
    splitMode,
    setSplitMode,
    focusedPane,
    setFocusedPane,
    isZenMode,
    settings,
    availableWorkspaces,
    loadAvailableWorkspaces,
    loadWorkspaceContext,
    gitCurrentBranch,
  } = useWorkspaceStore();
  const { activeTask, selectedTargets, lastExitCode, setSelectedTarget } =
    useExecutionStore();
  const setSettingsOpen = useCommandStore((state) => state.setSettingsOpen);
  const setPaletteOpen = useCommandStore((state) => state.setPaletteOpen);
  const executeCommand = useCommandStore((state) => state.executeCommand);
  const { t } = useTranslation();

  const file = activeFile as FileEntry | null;
  const runtimeId = file ? getRuntimeForFile(file) : "generic";
  const runtimeProfile = getRuntimeProfileForSettingsKey(runtimeId);
  const runTargets = getRunTargetOptions(runtimeId);
  const selectedTarget =
    selectedTargets[runtimeId] || getDefaultRunTarget(runtimeId);
  const relativeSegments = file
    ? getRelativePathSegments(file.path, currentProjectRoot)
    : [];
  const workspaceLabel = getPathLeaf(currentProjectRoot) || "Workspace";
  const problemCount = Object.values(diagnosticsMap).reduce(
    (count, diagnostics) => count + diagnostics.length,
    0
  );
  const showToolbarStatus = Boolean(activeTask) || typeof lastExitCode === "number";

  useEffect(() => {
    if (!runTargets.some((target) => target.id === selectedTarget)) {
      setSelectedTarget(runtimeId, getDefaultRunTarget(runtimeId));
    }
  }, [runTargets, runtimeId, selectedTarget, setSelectedTarget]);

  useEffect(() => {
    void loadAvailableWorkspaces();
  }, [loadAvailableWorkspaces]);

  const statusLabel = activeTask
    ? activeTask.action === "build"
      ? "Building..."
      : "Running..."
    : lastExitCode === 0
      ? "Succeeded"
      : lastExitCode === 130
        ? "Stopped"
        : typeof lastExitCode === "number"
          ? "Failed"
          : t("toolbar.ready");

  const statusBadgeClass = activeTask
    ? "border-amber-500/25 bg-amber-500/10 text-amber-700"
    : lastExitCode === 0
      ? "border-emerald-500/25 bg-emerald-500/10 text-emerald-700"
      : lastExitCode === 130
        ? "border-amber-500/25 bg-amber-500/10 text-amber-700"
        : typeof lastExitCode === "number"
          ? "border-red-500/25 bg-red-500/10 text-red-600"
          : "border-ide-border bg-ide-bg/80 text-ide-text-dim";

  const statusDotClass = activeTask
    ? "bg-amber-500 animate-pulse"
    : lastExitCode === 0
      ? "bg-emerald-500"
      : lastExitCode === 130
        ? "bg-amber-500"
        : typeof lastExitCode === "number"
          ? "bg-red-500"
          : "bg-ide-text-dim/50";

  const handleWorkspaceSelection = (event: ChangeEvent<HTMLSelectElement>) => {
    const value = event.target.value;

    if (value === "__manage") {
      setSidebarTab("contexts");
      setSidebarOpen(true);
      return;
    }

    if (value === "__open-folder") {
      executeCommand("zenith.workbench.action.openFolder");
      return;
    }

    if (value !== "__current") {
      void loadWorkspaceContext(value);
    }
  };

  const openProblemsPanel = () => {
    setBottomPanelOpen(true);
    setBottomTab("problems");
  };

  return (
    <header
      className={`xcode-toolbar select-none transition-all duration-700 ${
        isZenMode ? "translate-y-[-10px] opacity-0 hover:translate-y-0 hover:opacity-100" : ""
      }`}
    >
      <div className="grid w-full min-w-0 grid-cols-[auto_minmax(0,1fr)_auto] items-center gap-3">
        <div className="flex min-w-0 items-center gap-2">
          <ZenithMenu />

          <ToolbarIconButton
            testId="toolbar-toggle-sidebar"
            title={isSidebarOpen ? "Hide Sidebar" : "Show Sidebar"}
            active={isSidebarOpen}
            onClick={toggleSidebar}
          >
            {isSidebarOpen ? <PanelLeftClose size={16} /> : <PanelLeftOpen size={16} />}
          </ToolbarIconButton>

          {settings.toolbar.showWorkspaceSelector && (
            <div className="relative hidden md:flex min-w-[170px] max-w-[220px] items-center">
              <select
                value="__current"
                onChange={handleWorkspaceSelection}
                aria-label="Workspace selector"
                className="h-8 w-full appearance-none rounded-lg border border-ide-border bg-ide-bg/80 pl-3 pr-8 text-[11px] font-semibold text-ide-text outline-none transition-colors hover:border-ide-text/20"
              >
                <option value="__current">{workspaceLabel}</option>
                {availableWorkspaces.map((name) => (
                  <option key={name} value={name}>
                    {name}
                  </option>
                ))}
                <option value="__manage">Manage workspaces...</option>
                <option value="__open-folder">Open folder...</option>
              </select>
              <ChevronDown
                size={12}
                className="pointer-events-none absolute right-3 text-ide-text-dim/70"
              />
            </div>
          )}

          {settings.toolbar.showGitBranch && currentProjectRoot !== "." && (
            <button
              type="button"
              title={`Git branch: ${gitCurrentBranch || "main"}`}
              onClick={() => {
                setSidebarTab("source");
                setSidebarOpen(true);
              }}
              className="hidden lg:flex h-8 max-w-[150px] items-center gap-2 rounded-lg border border-ide-border bg-ide-bg/80 px-3 text-[10px] font-semibold text-ide-text-dim transition-colors hover:border-ide-text/20 hover:text-ide-text"
            >
              <GitBranch size={13} />
              <span className="truncate">{gitCurrentBranch || "main"}</span>
            </button>
          )}
        </div>

        <div className="flex min-w-0 items-center gap-2">
          <div className="flex h-8 min-w-0 flex-1 items-center gap-2 rounded-lg border border-ide-border bg-ide-bg/80 px-3">
            <div className="flex min-w-0 items-center gap-1.5 text-[10px] font-medium text-ide-text-dim">
              <span className="shrink-0 uppercase tracking-[0.18em] opacity-45">
                {workspaceLabel}
              </span>
              <ChevronRight size={10} className="shrink-0 opacity-20" />

              {file ? (
                <div className="flex min-w-0 items-center gap-1.5">
                  {relativeSegments.map((segment, index) => {
                    const isLast = index === relativeSegments.length - 1;

                    return (
                      <div key={`${segment}-${index}`} className="flex min-w-0 items-center gap-1.5">
                        <span
                          className={`truncate ${
                            isLast
                              ? dirtyFiles.has(file.path)
                                ? "font-bold text-amber-600"
                                : "font-semibold text-ide-text"
                              : "opacity-45"
                          }`}
                        >
                          {segment}
                        </span>
                        {!isLast && <ChevronRight size={10} className="shrink-0 opacity-20" />}
                      </div>
                    );
                  })}
                </div>
              ) : (
                <span className="truncate opacity-60">{t("toolbar.no_file")}</span>
              )}
            </div>
          </div>

          {settings.toolbar.showQuickOpen && (
            <button
              type="button"
              title="Quick Open (Ctrl+P)"
              onClick={() => setPaletteOpen(true, "")}
              className="hidden lg:flex h-8 items-center gap-2 rounded-lg border border-ide-border bg-ide-bg/80 px-3 text-[10px] font-semibold text-ide-text-dim transition-colors hover:border-ide-text/20 hover:text-ide-text"
            >
              <Search size={13} />
              <span className="hidden xl:inline">Quick Open</span>
              <span className="text-[9px] uppercase tracking-[0.18em] opacity-55">Ctrl+P</span>
            </button>
          )}
        </div>

        <div className="flex items-center justify-end gap-2">
          {settings.toolbar.showDiagnostics && (
            <button
              type="button"
              title={problemCount > 0 ? `${problemCount} diagnostics` : "No diagnostics"}
              onClick={openProblemsPanel}
              className={`flex h-8 items-center gap-2 rounded-lg border px-3 text-[10px] font-semibold transition-colors ${
                problemCount > 0
                  ? "border-red-500/20 bg-red-500/8 text-red-600 hover:border-red-500/35"
                  : "border-ide-border bg-ide-bg/80 text-ide-text-dim hover:border-ide-text/20 hover:text-ide-text"
              }`}
            >
              <AlertCircle size={13} />
              <span>{problemCount}</span>
            </button>
          )}

          {settings.toolbar.showRuntimeTarget && file && (
            <div className="hidden xl:flex h-8 items-center gap-2 rounded-lg border border-ide-border bg-ide-bg/80 px-3">
              <span className="text-[9px] font-bold uppercase tracking-[0.18em] text-ide-text-dim">
                {runtimeProfile.label}
              </span>

              {runTargets.length > 1 ? (
                <div className="relative flex items-center">
                  <select
                    value={selectedTarget}
                    onChange={(event) =>
                      setSelectedTarget(runtimeId, event.target.value)
                    }
                    className="appearance-none bg-transparent pr-4 text-[10px] font-semibold text-ide-text outline-none"
                    aria-label="Run target"
                  >
                    {runTargets.map((target) => (
                      <option key={target.id} value={target.id}>
                        {target.label}
                      </option>
                    ))}
                  </select>
                  <ChevronDown
                    size={10}
                    className="pointer-events-none absolute right-0 text-ide-text-dim/60"
                  />
                </div>
              ) : (
                <span className="text-[10px] font-semibold text-ide-text">
                  {runTargets[0]?.label ?? "Run"}
                </span>
              )}
            </div>
          )}

          {showToolbarStatus && (
            <div
              title={activeTask?.label || statusLabel}
              className={`hidden lg:flex h-8 items-center gap-2 rounded-lg border px-3 text-[9px] font-bold uppercase tracking-[0.18em] ${statusBadgeClass}`}
            >
              <span className={`h-1.5 w-1.5 rounded-full ${statusDotClass}`} />
              <span data-testid="toolbar-status">{statusLabel}</span>
            </div>
          )}

          <ToolbarCluster>
            <ToolbarIconButton
              testId="toolbar-run-stop"
              onClick={
                activeTask
                  ? () => void stopActivePipelineTask()
                  : () => void executePipelineAction("run")
              }
              disabled={!file}
              title={activeTask ? "Stop Active Task" : t("toolbar.run_tooltip")}
              active={Boolean(activeTask)}
            >
              {activeTask ? (
                <Square size={14} fill="currentColor" />
              ) : (
                <Play size={14} fill="currentColor" />
              )}
            </ToolbarIconButton>

            <ToolbarIconButton
              testId="toolbar-build"
              onClick={() => void executePipelineAction("build")}
              title={t("toolbar.build_tooltip")}
              disabled={!file || Boolean(activeTask)}
            >
              <Hammer size={14} />
            </ToolbarIconButton>
          </ToolbarCluster>

          {settings.toolbar.showPanelToggles && (
            <ToolbarCluster>
              <ToolbarIconButton
                testId="toolbar-toggle-inspector"
                onClick={() => setInspectorOpen(!isInspectorOpen)}
                title="Toggle Inspector"
                active={isInspectorOpen}
              >
                {isInspectorOpen ? (
                  <PanelRightClose size={16} />
                ) : (
                  <PanelRightOpen size={16} />
                )}
              </ToolbarIconButton>

              <ToolbarIconButton
                testId="toolbar-toggle-bottom-panel"
                onClick={() => setBottomPanelOpen(!isBottomPanelOpen)}
                title="Toggle Bottom Panel"
                active={isBottomPanelOpen}
              >
                <Layout size={16} />
              </ToolbarIconButton>
            </ToolbarCluster>
          )}

          {settings.toolbar.showLayoutControls && (
            <ToolbarCluster>
              <LayoutBtn
                testId="layout-single"
                active={splitMode === "single"}
                onClick={() => setSplitMode("single")}
                icon={<Square size={13} />}
                title="Single (Ctrl+Alt+1)"
              />
              <LayoutBtn
                testId="layout-vertical"
                active={splitMode === "vertical"}
                onClick={() => setSplitMode("vertical")}
                icon={<Columns2 size={13} />}
                title="Columns (Ctrl+Alt+V)"
              />
              <LayoutBtn
                testId="layout-horizontal"
                active={splitMode === "horizontal"}
                onClick={() => setSplitMode("horizontal")}
                icon={<Rows3 size={13} />}
                title="Rows (Ctrl+Alt+H)"
              />
            </ToolbarCluster>
          )}

          {settings.toolbar.showFocusHints && (
            <div className="hidden xl:flex items-center gap-1">
              <FocusHint
                testId="focus-primary"
                active={focusedPane === "primary"}
                onClick={() => setFocusedPane("primary")}
                label="P"
                shortcut="Alt+1"
              />
              {splitMode !== "single" && (
                <FocusHint
                  testId="focus-secondary"
                  active={focusedPane === "secondary"}
                  onClick={() => setFocusedPane("secondary")}
                  label="S"
                  shortcut="Alt+2"
                />
              )}
            </div>
          )}

          {settings.toolbar.showSettingsButton && (
            <ToolbarIconButton
              testId="toolbar-settings"
              title={t("toolbar.settings_tooltip")}
              onClick={() => setSettingsOpen(true)}
            >
              <Settings size={14} />
            </ToolbarIconButton>
          )}
        </div>
      </div>
    </header>
  );
}

function ToolbarCluster({ children }: { children: ReactNode }) {
  return (
    <div className="flex items-center gap-0.5 rounded-xl border border-ide-border bg-ide-bg/80 p-0.5">
      {children}
    </div>
  );
}

function ToolbarIconButton({
  active,
  children,
  disabled,
  onClick,
  title,
  testId,
}: {
  active?: boolean;
  children: ReactNode;
  disabled?: boolean;
  onClick: () => void;
  title: string;
  testId?: string;
}) {
  return (
    <button
      data-testid={testId}
      type="button"
      onClick={onClick}
      disabled={disabled}
      title={title}
      className={`flex items-center justify-center rounded-lg p-1.5 transition-all disabled:opacity-30 ${
        active
          ? "bg-primary/12 text-primary shadow-sm ring-1 ring-primary/20"
          : "text-ide-text-dim hover:bg-black/5 hover:text-ide-text active:scale-95"
      }`}
    >
      {children}
    </button>
  );
}

function LayoutBtn({
  active,
  onClick,
  icon,
  title,
  testId,
}: {
  active: boolean;
  onClick: () => void;
  icon: ReactNode;
  title: string;
  testId?: string;
}) {
  return (
    <button
      data-testid={testId}
      type="button"
      onClick={onClick}
      title={title}
      className={`rounded-lg p-1.5 transition-all ${
        active
          ? "bg-primary/15 text-primary shadow-sm"
          : "text-ide-text-dim hover:bg-black/5 hover:text-ide-text"
      }`}
    >
      {icon}
    </button>
  );
}

function FocusHint({
  active,
  onClick,
  label,
  shortcut,
  testId,
}: {
  active: boolean;
  onClick: () => void;
  label: string;
  shortcut: string;
  testId?: string;
}) {
  return (
    <button
      data-testid={testId}
      type="button"
      onClick={onClick}
      title={`${label}ane (${shortcut})`}
      className={`flex h-7 min-w-[34px] items-center justify-center rounded-lg border px-2 transition-all ${
        active
          ? "border-primary/30 bg-primary/10 text-primary"
          : "border-ide-border bg-ide-panel/40 text-ide-text-dim hover:border-ide-text"
      }`}
    >
      <span className="text-[9px] font-bold">{shortcut}</span>
    </button>
  );
}

function getRelativePathSegments(filePath: string, projectRoot: string) {
  let relativePath = filePath;

  if (projectRoot && projectRoot !== "." && relativePath.startsWith(projectRoot)) {
    relativePath = relativePath.replace(projectRoot, "").replace(/^[\/\\]/, "");
  }

  return relativePath.split(/[\/\\]/).filter(Boolean);
}

function getPathLeaf(path: string) {
  if (!path || path === ".") return "";

  const parts = path.split(/[\/\\]/).filter(Boolean);
  return parts[parts.length - 1] || path;
}
