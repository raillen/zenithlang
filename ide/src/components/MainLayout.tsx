import { Group, Panel, Separator } from "react-resizable-panels";
import { Toolbar } from "./Toolbar";
import { FileNavigator } from "./FileNavigator";
import { TerminalPanel } from "./TerminalPanel";
import { ProblemsPanel } from "./ProblemsPanel";
import { GlobalSearch } from "./GlobalSearch";
import { CommandPalette } from "./CommandPalette";
import { SettingsDialog } from "./SettingsDialog";
import { useCommandStore } from "../store/useCommandStore";
import { useWorkspaceStore } from "../store/useWorkspaceStore";
import { Files, Search, GitBranch, Bug, AlertCircle } from "lucide-react";
import { useTranslation } from "../utils/i18n";
import { BrandLogo } from "./BrandLogo";
import { OutlinePanel } from "./OutlinePanel";
import { List } from "lucide-react";


export function MainLayout({ children }: { children: React.ReactNode }) {
  const { 
    isBottomPanelOpen, 
    activeSidebarTab, 
    setSidebarTab,
    isSidebarOpen,
    setSidebarOpen,
    activeBottomTab,
    setBottomTab,
    diagnosticsMap
  } = useWorkspaceStore();
  const { isSettingsOpen, setSettingsOpen } = useCommandStore();
  const { t } = useTranslation();

  const problemCount = Object.values(diagnosticsMap).reduce((acc, curr) => acc + (Array.isArray(curr) ? curr.length : 0), 0);

  const handleTabClick = (tab: string) => {
    if (activeSidebarTab === tab) {
      setSidebarOpen(!isSidebarOpen);
    } else {
      setSidebarTab(tab);
      if (!isSidebarOpen) setSidebarOpen(true);
    }
  };

  return (
    <div className="flex flex-col h-full w-full overflow-hidden bg-ide-bg transition-colors duration-200">
      <div className="relative z-50">
        <Toolbar />
      </div>
      
      <div className="flex-1 flex overflow-hidden">
        {/* Activity Bar */}
        <div className="w-12 bg-ide-panel border-r border-ide-border flex flex-col items-center py-4 gap-4 transition-colors duration-200">
          <ActivityIcon icon={<Files size={18} />} active={activeSidebarTab === 'navigator' && isSidebarOpen} onClick={() => handleTabClick('navigator')} title={t('sidebar.navigator')} />
          <ActivityIcon icon={<Search size={18} />} active={activeSidebarTab === 'search' && isSidebarOpen} onClick={() => handleTabClick('search')} title={t('sidebar.search')} />
          <ActivityIcon icon={<GitBranch size={18} />} active={activeSidebarTab === 'source' && isSidebarOpen} onClick={() => handleTabClick('source')} title={t('sidebar.source')} />
          <ActivityIcon icon={<List size={18} />} active={activeSidebarTab === 'outline' && isSidebarOpen} onClick={() => handleTabClick('outline')} title="Outline" />
          <ActivityIcon icon={<Bug size={18} />} active={activeSidebarTab === 'debug' && isSidebarOpen} onClick={() => handleTabClick('debug')} title={t('sidebar.debug')} />

        </div>

        <Group orientation="horizontal" className="flex-1">
          {isSidebarOpen && (
            <>
              {/* Navigator Sidebar */}
              <Panel defaultSize={20} minSize={15} className="xcode-sidebar flex flex-col">
                <div className="h-8 flex items-center px-4 text-[10px] font-bold uppercase tracking-widest text-ide-text-dim border-b border-ide-border transition-colors duration-200">
                  {activeSidebarTab === 'navigator' ? t('sidebar.navigator_title') : activeSidebarTab}
                </div>
                <div className="flex-1 overflow-hidden h-full flex flex-col">
                  {activeSidebarTab === 'navigator' ? <FileNavigator /> : 
                   activeSidebarTab === 'search' ? <GlobalSearch /> : 
                   activeSidebarTab === 'outline' ? <OutlinePanel /> :
                   <div className="p-8 text-[11px] text-ide-text-dim text-center italic">{t('common.coming_soon')}</div>}
                </div>

              </Panel>

              <Separator className="w-[1px] bg-ide-border hover:bg-primary/30 transition-colors" />
            </>
          )}

          {/* Main Editor & Bottom Panel */}
          <Panel defaultSize={80} className="flex flex-col overflow-hidden bg-ide-bg transition-colors duration-200">
            <Group orientation="vertical">
              <Panel defaultSize={75} className="relative flex-1 overflow-hidden">
                {children}
              </Panel>

              {isBottomPanelOpen && (
                <>
                  <Separator className="h-[1px] bg-ide-border hover:bg-primary/30 transition-colors" />
                  <Panel defaultSize={25} minSize={10} className="bg-ide-panel flex flex-col transition-colors duration-200">
                    <div className="h-8 border-b border-ide-border flex items-center px-4 gap-6">
                        <BottomTab 
                          label={t('bottom_tabs.console')} 
                          active={activeBottomTab === 'console'} 
                          onClick={() => setBottomTab('console')} 
                        />
                        <BottomTab 
                          label={t('bottom_tabs.terminal')} 
                          active={activeBottomTab === 'terminal'} 
                          onClick={() => setBottomTab('terminal')} 
                        />
                        <BottomTab 
                          label={`${t('bottom_tabs.problems')} ${problemCount > 0 ? `(${problemCount})` : ''}`} 
                          active={activeBottomTab === 'problems'} 
                          onClick={() => setBottomTab('problems')} 
                          icon={problemCount > 0 ? <AlertCircle size={10} className="ml-1 text-red-500" /> : null}
                        />
                    </div>
                    <div className="flex-1 min-h-0">
                       {activeBottomTab === 'problems' ? <ProblemsPanel /> : <TerminalPanel />}
                    </div>
                  </Panel>
                </>
              )}
            </Group>
          </Panel>
        </Group>
      </div>

      <StatusBar />
      <CommandPalette />
      {isSettingsOpen && <SettingsDialog onClose={() => setSettingsOpen(false)} />}
    </div>
  );
}

function ActivityIcon({ icon, active, onClick, title }: { icon: React.ReactNode, active: boolean, onClick: () => void, title?: string }) {
  return (
    <button 
      onClick={onClick}
      title={title}
      className={`p-2 rounded-lg transition-all ${active ? 'bg-primary/10 text-primary shadow-sm ring-1 ring-primary/20' : 'text-ide-text-dim hover:text-ide-text hover:bg-black/5'}`}
    >
      {icon}
    </button>
  );
}

function StatusBar() {
  return (
    <footer className="xcode-status-bar justify-between">
      <div className="flex items-center gap-4">
        <div className="flex items-center gap-1.5">
           <div className="w-2 h-2 rounded-full bg-green-500" />
           <span>Environment: Lua 5.4</span>
        </div>
      </div>
      <div className="flex items-center gap-4 pr-2">
        <span>UTF-8</span>
        <div className="flex items-center gap-1.5">
          <BrandLogo variant="icon" className="h-3.5 w-3.5 opacity-90" alt="Zenith" />
          <span>Zenith v0.2-alpha</span>
        </div>
      </div>
    </footer>
  )
}

function BottomTab({ label, active, onClick, icon }: { label: string, active: boolean, onClick: () => void, icon?: React.ReactNode }) {
  return (
    <button 
      onClick={onClick}
      className={`text-[10px] font-bold uppercase tracking-widest h-full flex items-center transition-colors border-b-2 ${active ? 'text-primary border-primary' : 'text-ide-text-dim border-transparent hover:text-ide-text'}`}
    >
      {label}
      {icon}
    </button>
  );
}
