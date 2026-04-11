import { Group, Panel, Separator } from "react-resizable-panels";
import { Toolbar } from "./Toolbar";
import { FileNavigator } from "./FileNavigator";
import { TerminalPanel } from "./TerminalPanel";
import { ConsolePanel } from "./ConsolePanel";
import { ProblemsPanel } from "./ProblemsPanel";
import { GlobalSearch } from "./GlobalSearch";
import { CommandPalette } from "./CommandPalette";
import { SettingsDialog } from "./SettingsDialog";
import { useCommandStore } from "../store/useCommandStore";
import { useWorkspaceStore } from "../store/useWorkspaceStore";
import { WorkspaceManager } from "./WorkspaceManager";
import { useKeyboardShortcuts } from "../hooks/useKeyboardShortcuts";
import { Layout, Files, Search, GitBranch, Bug, AlertCircle, List } from "lucide-react";
import { useTranslation } from "../utils/i18n";
import { OutlinePanel } from "./OutlinePanel";
import { InspectorPanel } from "./InspectorPanel";
import { SourceControlPanel } from "./SourceControlPanel";
import { DebugPanel } from "./DebugPanel";
import { StatusBar } from "./StatusBar";
import { ZenithHub } from "./ZenithHub";

export function MainLayout({ children }: { children: React.ReactNode }) {
  const {
    isBottomPanelOpen,
    activeSidebarTab,
    setSidebarTab,
    isSidebarOpen,
    setSidebarOpen,
    activeBottomTab,
    setBottomTab,
    diagnosticsMap,
    isInspectorOpen,
    isZenMode,
  } = useWorkspaceStore();
  const { isSettingsOpen, setSettingsOpen } = useCommandStore();

  useKeyboardShortcuts();
  const { t } = useTranslation();

  const problemCount = Object.values(diagnosticsMap).reduce(
    (acc, curr) => acc + (Array.isArray(curr) ? curr.length : 0),
    0
  );

  const handleTabClick = (tab: string) => {
    if (activeSidebarTab === tab) {
      setSidebarOpen(!isSidebarOpen);
    } else {
      setSidebarTab(tab);
      if (!isSidebarOpen) setSidebarOpen(true);
    }
  };

  return (
    <div
      data-testid="workbench-root"
      className={`flex flex-col h-full w-full overflow-hidden transition-colors duration-200 ${isZenMode ? 'bg-ide-bg-sanctuary' : 'bg-ide-bg'}`}
    >
      {!isZenMode && (
        <div className="relative z-50">
          <Toolbar />
        </div>
      )}

      <div className="flex-1 flex overflow-hidden">
        {!isZenMode && (
          <div className="w-12 bg-ide-panel border-r border-ide-border flex flex-col items-center py-4 gap-4 transition-all duration-300">
            <ActivityIcon testId="activity-navigator" icon={<Files size={18} />} active={activeSidebarTab === "navigator" && isSidebarOpen} onClick={() => handleTabClick("navigator")} title={t("sidebar.navigator")} />
            <ActivityIcon icon={<Layout size={18} />} active={activeSidebarTab === "contexts" && isSidebarOpen} onClick={() => handleTabClick("contexts")} title="Contexts" />
            <ActivityIcon icon={<Search size={18} />} active={activeSidebarTab === "search" && isSidebarOpen} onClick={() => handleTabClick("search")} title={t("sidebar.search")} />
            <ActivityIcon icon={<GitBranch size={18} />} active={activeSidebarTab === "source" && isSidebarOpen} onClick={() => handleTabClick("source")} title={t("sidebar.source")} />
            <ActivityIcon icon={<List size={18} />} active={activeSidebarTab === "outline" && isSidebarOpen} onClick={() => handleTabClick("outline")} title="Outline" />
            <ActivityIcon icon={<Bug size={18} />} active={activeSidebarTab === "debug" && isSidebarOpen} onClick={() => handleTabClick("debug")} title={t("sidebar.debug")} />
          </div>
        )}

        <Group orientation="horizontal" className="flex-1">
          {isSidebarOpen && !isZenMode && (
            <>
              <Panel defaultSize={20} minSize={15} className="xcode-sidebar flex flex-col">
                <div className="h-8 flex items-center px-4 text-[10px] font-bold uppercase tracking-widest text-ide-text-dim border-b border-ide-border transition-colors duration-200">
                  {activeSidebarTab === "navigator" ? t("sidebar.navigator_title") : activeSidebarTab}
                </div>
                <div className="flex-1 overflow-hidden h-full flex flex-col">
                  {activeSidebarTab === "navigator" ? (
                    <FileNavigator />
                  ) : activeSidebarTab === "search" ? (
                    <GlobalSearch />
                  ) : activeSidebarTab === "outline" ? (
                    <OutlinePanel />
                  ) : activeSidebarTab === "source" ? (
                    <SourceControlPanel />
                  ) : activeSidebarTab === "contexts" ? (
                    <WorkspaceManager />
                  ) : activeSidebarTab === "debug" ? (
                    <DebugPanel />
                  ) : (
                    <div className="p-8 text-[11px] text-ide-text-dim text-center italic">
                      {t("common.coming_soon")}
                    </div>
                  )}
                </div>
              </Panel>

              <Separator className="w-[1px] bg-ide-border hover:bg-primary/30 transition-colors" />
            </>
          )}

          <Panel defaultSize={isSidebarOpen && isInspectorOpen ? 58 : isInspectorOpen ? 76 : 80} className="flex flex-col overflow-hidden bg-ide-bg transition-colors duration-200">
            <Group orientation="vertical">
              <Panel defaultSize={75} className="relative flex-1 overflow-hidden">
                {children}
              </Panel>

              {isBottomPanelOpen && !isZenMode && (
                <>
                  <Separator className="h-[1px] bg-ide-border hover:bg-primary/30 transition-colors" />
                  <Panel defaultSize={25} minSize={10} className="bg-ide-panel flex flex-col transition-colors duration-200">
                    <div className="h-8 border-b border-ide-border flex items-center px-4 gap-6">
                      <BottomTab testId="bottom-tab-console" label={t("bottom_tabs.console")} active={activeBottomTab === "console"} onClick={() => setBottomTab("console")} />
                      <BottomTab testId="bottom-tab-terminal" label={t("bottom_tabs.terminal")} active={activeBottomTab === "terminal"} onClick={() => setBottomTab("terminal")} />
                      <BottomTab testId="bottom-tab-problems" label={`${t("bottom_tabs.problems")} ${problemCount > 0 ? `(${problemCount})` : ""}`} active={activeBottomTab === "problems"} onClick={() => setBottomTab("problems")} icon={problemCount > 0 ? <AlertCircle size={10} className="ml-1 text-red-500" /> : null} />
                    </div>
                    <div className="relative flex-1 min-h-0 overflow-hidden">
                      <div
                        className={`absolute inset-0 transition-opacity ${activeBottomTab === "console" ? "opacity-100" : "pointer-events-none opacity-0"}`}
                        aria-hidden={activeBottomTab !== "console"}
                      >
                        <ConsolePanel />
                      </div>
                      <div
                        className={`absolute inset-0 transition-opacity ${activeBottomTab === "terminal" ? "opacity-100" : "pointer-events-none opacity-0"}`}
                        aria-hidden={activeBottomTab !== "terminal"}
                      >
                        <TerminalPanel />
                      </div>
                      {activeBottomTab === "problems" && (
                        <div className="absolute inset-0">
                          <ProblemsPanel />
                        </div>
                      )}
                    </div>
                  </Panel>
                </>
              )}
            </Group>
          </Panel>

          {isInspectorOpen && !isZenMode && (
            <>
              <Separator className="w-[1px] bg-ide-border hover:bg-primary/30 transition-colors" />
              <Panel defaultSize={22} minSize={16} className="flex flex-col overflow-hidden bg-ide-panel transition-colors duration-200">
                <InspectorPanel />
              </Panel>
            </>
          )}
        </Group>
      </div>

      {!isZenMode && <StatusBar />}
      <ZenithHub />
      <CommandPalette />
      {isSettingsOpen && <SettingsDialog onClose={() => setSettingsOpen(false)} />}
    </div>
  );
}

function ActivityIcon({ icon, active, onClick, title, testId }: { icon: React.ReactNode; active: boolean; onClick: () => void; title?: string; testId?: string }) {
  return (
    <button
      data-testid={testId}
      onClick={onClick}
      title={title}
      className={`p-2 rounded-lg transition-all ${active ? "bg-primary/10 text-primary shadow-sm ring-1 ring-primary/20" : "text-ide-text-dim hover:text-ide-text hover:bg-black/5"}`}
    >
      {icon}
    </button>
  );
}


function BottomTab({ label, active, onClick, icon, testId }: { label: string; active: boolean; onClick: () => void; icon?: React.ReactNode; testId?: string }) {
  return (
    <button
      data-testid={testId}
      onClick={onClick}
      className={`text-[10px] font-bold uppercase tracking-widest h-full flex items-center transition-colors border-b-2 ${active ? "text-primary border-primary" : "text-ide-text-dim border-transparent hover:text-ide-text"}`}
    >
      {label}
      {icon}
    </button>
  );
}
