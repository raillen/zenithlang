import { CaretRight, TerminalWindow, WarningCircle, TextAlignLeft } from '@phosphor-icons/react';
import { TerminalPanel } from './TerminalPanel';
import { type BottomPanelTab, type PanelDock, useProjectStore } from '../store/useProjectStore';
import { uiMarker } from '../utils/debugSelectors';

function extractProblems(buildLogs: string) {
  if (!buildLogs.trim()) {
    return [];
  }

  return buildLogs
    .split('\n')
    .map((line) => line.trim())
    .filter((line) => /(error|failed|unexpected|panic)/i.test(line))
    .slice(0, 12);
}

const panelTabs: Array<{ id: BottomPanelTab; label: string }> = [
  { id: 'output', label: 'Output' },
  { id: 'problems', label: 'Problems' },
  { id: 'terminal', label: 'Terminal' },
];

export function BottomPanel() {
  const {
    activeBottomPanelTab,
    setActiveBottomPanelTab,
    buildLogs,
    isBuilding,
    focusMode,
    utilityPanelDock,
    setUtilityPanelDock,
  } = useProjectStore();
  const problems = extractProblems(buildLogs);

  return (
    <div className="ide-panel-surface flex h-full min-h-0 flex-col" {...uiMarker('bottom-panel', { activeTab: activeBottomPanelTab, dock: utilityPanelDock })}>
      <div className="ide-toolbar-surface flex h-10 shrink-0 items-center justify-between border-b px-3" {...uiMarker('bottom-panel-toolbar')}>
        <div className="flex items-center gap-1" {...uiMarker('bottom-panel-tabs')}>
          {panelTabs.map((tab) => {
            const isActive = tab.id === activeBottomPanelTab;

            return (
              <button
                key={tab.id}
                type="button"
                onClick={() => setActiveBottomPanelTab(tab.id)}
                className={`rounded-full px-3 py-1.5 text-[12px] transition ${isActive ? 'ide-segment-active' : 'ide-button-ghost'}`}
                style={isActive ? undefined : { color: 'var(--text-secondary)' }}
                {...uiMarker('bottom-panel-tab', { tab: tab.id, active: isActive })}
              >
                {tab.label}
              </button>
            );
          })}
        </div>

        <div className="flex items-center gap-3" {...uiMarker('bottom-panel-toolbar-meta')}>
          <DockSwitcher dock={utilityPanelDock} onChange={setUtilityPanelDock} />
          {!focusMode ? (
            <div className="text-[11px]" style={{ color: 'var(--text-tertiary)' }} {...uiMarker('bottom-panel-status')}>
              {isBuilding ? 'Build running' : `Docked ${utilityPanelDock}`}
            </div>
          ) : null}
        </div>
      </div>

      <div className="min-h-0 flex-1 overflow-hidden" {...uiMarker('bottom-panel-body')}>
        {activeBottomPanelTab === 'output' ? (
          <PanelSurface icon={TextAlignLeft} title="Compiler output" description="Logs from the latest build or command run.">
            <pre className="h-full whitespace-pre-wrap break-words px-4 py-4 font-mono text-[12px] leading-6" style={{ color: 'var(--text-secondary)' }} {...uiMarker('bottom-panel-output')}>
              {buildLogs || 'Run the compiler from the toolbar to populate this panel.'}
            </pre>
          </PanelSurface>
        ) : activeBottomPanelTab === 'problems' ? (
          <PanelSurface icon={WarningCircle} title="Problems" description="Build output lines that look actionable.">
            {problems.length === 0 ? (
              <div className="px-4 py-5 text-sm" style={{ color: 'var(--text-tertiary)' }} {...uiMarker('bottom-panel-problems-empty')}>
                No problems detected in the current output.
              </div>
            ) : (
              <div className="overflow-y-auto" {...uiMarker('bottom-panel-problems-list', { count: problems.length })}>
                {problems.map((problem, index) => (
                  <div
                    key={`${problem}-${index}`}
                    className="border-b px-4 py-3 text-sm"
                    style={{ borderColor: 'var(--border)', color: 'var(--text-secondary)' }}
                    {...uiMarker('bottom-panel-problem', { index })}
                  >
                    <div className="flex items-center gap-2 text-[11px]" style={{ color: 'var(--warning)' }}>
                      <WarningCircle size={13} weight="fill" />
                      <span>Problem {index + 1}</span>
                    </div>
                    <div className="mt-2 leading-6">{problem}</div>
                  </div>
                ))}
              </div>
            )}
          </PanelSurface>
        ) : (
          <PanelSurface icon={TerminalWindow} title="Terminal" description="Interactive terminal routed through Tauri and themed from the active interface.">
            <TerminalPanel />
          </PanelSurface>
        )}
      </div>
    </div>
  );
}

function DockSwitcher({
  dock,
  onChange,
}: {
  dock: PanelDock;
  onChange: (dock: PanelDock) => void;
}) {
  const items: Array<{ id: PanelDock; label: string }> = [
    { id: 'left', label: 'Left' },
    { id: 'bottom', label: 'Bottom' },
    { id: 'right', label: 'Right' },
  ];

  return (
    <div className="ide-segment flex items-center rounded-full p-1" {...uiMarker('bottom-panel-dock-switcher', { dock })}>
      {items.map((item) => {
        const isActive = item.id === dock;

        return (
          <button
            key={item.id}
            type="button"
            onClick={() => onChange(item.id)}
            className={`rounded-full px-2.5 py-1 text-[11px] transition ${isActive ? 'ide-segment-active' : 'ide-button-ghost'}`}
            style={isActive ? undefined : { color: 'var(--text-secondary)' }}
            {...uiMarker('bottom-panel-dock-option', { dock: item.id, active: isActive })}
          >
            {item.label}
          </button>
        );
      })}
    </div>
  );
}

function PanelSurface({
  icon: Icon,
  title,
  description,
  children,
}: {
  icon: typeof TextAlignLeft;
  title: string;
  description: string;
  children: React.ReactNode;
}) {
  return (
    <div className="flex h-full min-h-0 flex-col" {...uiMarker('bottom-panel-surface', { title: title.toLowerCase().replace(/\s+/g, '-') })}>
      <div
        className="flex shrink-0 items-center gap-3 border-b px-4 py-3"
        style={{ borderColor: 'var(--border)', background: 'color-mix(in srgb, var(--ide-panel) 78%, white 22%)' }}
        {...uiMarker('bottom-panel-surface-header')}
      >
        <div className="ide-soft-surface flex h-7 w-7 items-center justify-center rounded-xl" style={{ color: 'var(--text-secondary)' }}>
          <Icon size={14} />
        </div>
        <div className="min-w-0">
          <div className="text-sm" style={{ color: 'var(--text-primary)' }}>
            {title}
          </div>
          <div className="text-[11px]" style={{ color: 'var(--text-tertiary)' }}>
            {description}
          </div>
        </div>
        <CaretRight size={12} className="ml-auto" style={{ color: 'var(--text-quaternary)' }} />
      </div>
      <div className="min-h-0 flex-1 overflow-hidden" {...uiMarker('bottom-panel-surface-body')}>{children}</div>
    </div>
  );
}
