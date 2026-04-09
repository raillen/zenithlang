import { Group, Panel, Separator } from "react-resizable-panels";
import { Toolbar } from "./Toolbar";
import { FileNavigator } from "./FileNavigator";
import { TerminalPanel } from "./TerminalPanel";
import { useWorkspaceStore } from "../store/useWorkspaceStore";
import { Files, Search, GitBranch, Bug } from "lucide-react";

export function MainLayout({ children }: { children: React.ReactNode }) {
  const { isBottomPanelOpen, activeSidebarTab, setSidebarTab } = useWorkspaceStore();

  return (
    <div className="flex flex-col h-full w-full overflow-hidden">
      <Toolbar />
      
      <div className="flex-1 flex overflow-hidden">
        {/* Activity Bar */}
        <div className="w-12 bg-white/40 border-r border-ide-border flex flex-col items-center py-4 gap-4 backdrop-blur-md">
          <ActivityIcon icon={<Files size={18} />} active={activeSidebarTab === 'navigator'} onClick={() => setSidebarTab('navigator')} />
          <ActivityIcon icon={<Search size={18} />} active={activeSidebarTab === 'search'} onClick={() => setSidebarTab('search')} />
          <ActivityIcon icon={<GitBranch size={18} />} active={activeSidebarTab === 'source'} onClick={() => setSidebarTab('source')} />
          <ActivityIcon icon={<Bug size={18} />} active={activeSidebarTab === 'debug'} onClick={() => setSidebarTab('debug')} />
        </div>

        <Group orientation="horizontal" className="flex-1">
          {/* Navigator Sidebar */}
          <Panel defaultSize={20} minSize={15} className="xcode-sidebar flex flex-col">
            <div className="h-8 flex items-center px-4 text-[10px] font-bold uppercase tracking-widest text-zinc-400 border-b border-black/[0.03]">
              {activeSidebarTab === 'navigator' ? 'Project Navigator' : activeSidebarTab}
            </div>
            <div className="flex-1 overflow-auto">
              {activeSidebarTab === 'navigator' ? <FileNavigator /> : <div className="p-8 text-[11px] text-zinc-400 text-center italic">Coming soon</div>}
            </div>
          </Panel>

          <Separator className="w-[1px] bg-ide-border hover:bg-primary/30 transition-colors" />

          {/* Main Editor & Bottom Panel */}
          <Panel defaultSize={80} className="flex flex-col overflow-hidden bg-white/60">
            <Group orientation="vertical">
              <Panel defaultSize={75} className="relative flex-1 overflow-hidden">
                {children}
              </Panel>

              {isBottomPanelOpen && (
                <>
                  <Separator className="h-[1px] bg-ide-border hover:bg-primary/30 transition-colors" />
                  <Panel defaultSize={25} minSize={10} className="bg-white/40 backdrop-blur-lg flex flex-col">
                    <div className="h-8 border-b border-ide-border flex items-center px-4 gap-4">
                        <span className="text-[10px] font-bold uppercase tracking-widest text-primary border-b-2 border-primary h-full flex items-center">Console</span>
                        <span className="text-[10px] font-bold uppercase tracking-widest text-zinc-400 h-full flex items-center hover:text-zinc-600 transition-colors">Terminal</span>
                    </div>
                    <div className="flex-1 min-h-0">
                       <TerminalPanel />
                    </div>
                  </Panel>
                </>
              )}
            </Group>
          </Panel>
        </Group>
      </div>

      <StatusBar />
    </div>
  );
}

function ActivityIcon({ icon, active, onClick }: { icon: React.ReactNode, active: boolean, onClick: () => void }) {
  return (
    <button 
      onClick={onClick}
      className={`p-2 rounded-lg transition-all ${active ? 'bg-primary/10 text-primary shadow-sm ring-1 ring-primary/20' : 'text-zinc-400 hover:text-zinc-600 hover:bg-black/5'}`}
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
        <span>Zenith v0.2-alpha</span>
      </div>
    </footer>
  )
}
