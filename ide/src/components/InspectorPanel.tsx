import React, { useEffect, useState } from 'react';
import {
  AlertCircle,
  CheckCircle2,
  Copy,
  Cpu,
  FileText,
  Globe,
  Info,
  Settings,
  Sparkles,
  TerminalSquare,
  Wrench,
  Zap,
  History as HistoryIcon,
  RotateCcw,
  Clock
} from 'lucide-react';
import { useWorkspaceStore, Diagnostic, ExtensionConfig, FileEntry } from '../store/useWorkspaceStore';
import { useCommandStore } from '../store/useCommandStore';
import {
  DEFAULT_RUNTIME_EXTENSION_CONFIGS,
  detectRuntimeProfileFromFile,
  getLanguageLabelForFile,
  matchesRuntimeMarker,
  RuntimeProfile,
  RuntimeProfileId,
  getRuntimeProfileForSettingsKey,
} from '../utils/runtimeProfiles';

interface WorkspaceSignalSummary {
  name: string;
  path: string;
  summary: string;
}

interface InspectorRuntimeContext {
  runtimeId: RuntimeProfileId;
  profile: RuntimeProfile;
  languageLabel: string;
  extensionConfig: ExtensionConfig;
  isConfigured: boolean;
  hasLsp: boolean;
  hasBuildTask: boolean;
  signals: WorkspaceSignalSummary[];
  recommendations: string[];
}

const EMPTY_EXTENSION_CONFIG: ExtensionConfig = {
  compilerPath: '',
  lspPath: '',
  buildCommand: '',
  isEnabled: false,
};

const INSPECTOR_TABS = [
  { id: 'file', label: 'File', icon: Info },
  { id: 'context', label: 'Context', icon: Zap },
  { id: 'attributes', label: 'Config', icon: Settings },
  { id: 'history', label: 'History', icon: HistoryIcon },
] as const;

export function InspectorPanel() {
  const {
    activeFile,
    diagnosticsMap,
    dirtyFiles,
    activeInspectorTab,
    setInspectorTab,
    settings,
    fileTree,
    activeFileHistory,
    fetchHistory
  } = useWorkspaceStore();
  const setSettingsOpen = useCommandStore((state) => state.setSettingsOpen);
  const [runtimeContext, setRuntimeContext] = useState<InspectorRuntimeContext | null>(null);
  const [isResolvingContext, setIsResolvingContext] = useState(false);

  useEffect(() => {
    let cancelled = false;

    async function resolveContext() {
      if (!activeFile) {
        setRuntimeContext(null);
        setIsResolvingContext(false);
        return;
      }

      setIsResolvingContext(true);

      const nextContext = await buildInspectorRuntimeContext(
        activeFile,
        fileTree,
        settings.extensions || {}
      );

      if (!cancelled) {
        setRuntimeContext(nextContext);
        setIsResolvingContext(false);
      }
    }

    void resolveContext();

    return () => {
      cancelled = true;
    };
  }, [activeFile?.path, fileTree, settings.extensions]);

  useEffect(() => {
    if (activeInspectorTab === 'history' && activeFile) {
        fetchHistory(activeFile.path);
    }
  }, [activeInspectorTab, activeFile?.path, fetchHistory]);

  if (!activeFile) {
    return <EmptyInspector />;
  }

  const diagnostics = diagnosticsMap[activeFile.path] || [];
  const isDirty = dirtyFiles.has(activeFile.path);
  const file = activeFile as FileEntry;
  const profile = runtimeContext?.profile ?? getRuntimeProfileForSettingsKey(detectRuntimeProfileFromFile(file));
  const languageLabel = runtimeContext?.languageLabel || getLanguageLabelForFile(file.name, profile.id);
  const runtimeReady = Boolean(runtimeContext?.isConfigured);

  const handleCopyPath = async () => {
    try {
      await navigator.clipboard.writeText(file.path);
    } catch (error) {
      console.error('Failed to copy file path', error);
    }
  };

  return (
    <aside className="flex h-full overflow-hidden border-l border-ide-border bg-ide-panel/95 text-ide-text select-none backdrop-blur-xl">
      <div className="flex min-w-0 flex-1 flex-col overflow-hidden">
        <div className="border-b border-ide-border bg-ide-bg/15">
          <div className="flex h-8 items-center justify-between border-b border-ide-border px-4 text-[10px] font-bold uppercase tracking-widest text-ide-text-dim">
            <span>Inspector</span>
            <div className="flex items-center gap-3">
              <span className="text-ide-text-dim/70">{profile.label}</span>
              <span className={`flex items-center gap-1.5 ${runtimeReady ? 'text-emerald-600' : 'text-amber-600'}`}>
                <span className="h-1.5 w-1.5 rounded-full bg-current" />
                <span>{runtimeReady ? 'Ready' : 'Setup'}</span>
              </span>
            </div>
          </div>

          <div className="px-4 py-3">
            <div className="flex items-start gap-3">
              <div className="flex h-9 w-9 shrink-0 items-center justify-center rounded-lg border border-ide-border bg-ide-panel shadow-[inset_0_1px_0_rgba(255,255,255,0.04)]">
                <FileText size={18} className="text-ide-text-dim" />
              </div>

              <div className="min-w-0 flex-1">
                <div className="flex flex-wrap items-center gap-2 text-[9px] font-bold uppercase tracking-[0.18em] text-ide-text-dim/75">
                  <span>{languageLabel}</span>
                  <span className="opacity-30">/</span>
                  <span>{file.is_directory ? 'Directory' : 'Workspace File'}</span>
                </div>
                <div className="mt-1 break-words text-[13px] font-semibold leading-tight text-ide-text">
                  {file.name}
                </div>
                <div className="mt-1 break-all text-[10px] text-ide-text-dim/75" title={file.path}>
                  {file.path}
                </div>
              </div>
            </div>
          </div>
        </div>

        <div className="min-h-0 min-w-0 flex-1 overflow-y-auto py-2">
          {activeInspectorTab === 'file' && (
            <FileTab
              file={file}
              diagnostics={diagnostics}
              isDirty={isDirty}
              context={runtimeContext}
            />
          )}

          {activeInspectorTab === 'context' && (
            <ContextTab
              diagnostics={diagnostics}
              context={runtimeContext}
              isResolving={isResolvingContext}
            />
          )}

          {activeInspectorTab === 'attributes' && (
            <AttributesTab
              context={runtimeContext}
              isResolving={isResolvingContext}
              onOpenSettings={() => setSettingsOpen(true, 'extensions')}
            />
          )}

          {activeInspectorTab === 'history' && (
            <HistoryTab history={activeFileHistory} />
          )}
        </div>

        <div className="border-t border-ide-border bg-ide-bg/10 p-3">
          <div className="grid grid-cols-1 gap-2">
            <ActionBtn
              label="Toolchain Setup"
              icon={<Wrench size={10} />}
              primary
              onClick={() => setSettingsOpen(true, 'extensions')}
            />
            <ActionBtn label="Copy Path" icon={<Copy size={10} />} onClick={handleCopyPath} />
          </div>
        </div>
      </div>

      <div
        role="tablist"
        aria-label="Inspector regions"
        className="flex w-12 shrink-0 flex-col items-center gap-4 border-l border-ide-border bg-ide-panel py-4"
      >
        {INSPECTOR_TABS.map(({ id, label, icon: Icon }) => (
          <TabButton
            key={id}
            active={activeInspectorTab === id}
            onClick={() => setInspectorTab(id)}
            icon={<Icon size={18} />}
            label={label}
            vertical
          />
        ))}
      </div>
    </aside>
  );
}

function TabButton({
  active,
  onClick,
  icon,
  label,
  vertical,
}: {
  active: boolean;
  onClick: () => void;
  icon: React.ReactNode;
  label: string;
  vertical?: boolean;
}) {
  return (
    <button
      onClick={onClick}
      title={label}
      aria-label={label}
      className={`
        ${vertical
          ? 'p-2'
          : 'flex-1 flex items-center justify-center gap-2 py-1.5'}
        rounded-lg transition-all
        ${active
          ? 'bg-primary/10 text-primary ring-1 ring-primary/20 shadow-sm'
          : 'text-ide-text-dim hover:bg-black/5 hover:text-ide-text'}
      `}
    >
      {icon}
      {!vertical && (
        <span className="text-[10px] font-bold uppercase tracking-[0.18em]">
          {label}
        </span>
      )}
    </button>
  );
}

function HistoryTab({ history }: { history: any[] }) {
    const handleRestore = (snapshot: any) => {
        window.dispatchEvent(new CustomEvent('zenith://editor-set-content', {
            detail: { content: snapshot.content }
        }));
    };

    return (
        <div className="px-4 py-2 space-y-4">
            <div className="flex items-center gap-2 mb-4 text-primary">
                <Clock size={14} />
                <h3 className="text-[11px] font-bold uppercase tracking-[0.2em]">Sanctuary Timeline</h3>
            </div>

            {!history || history.length === 0 ? (
                <div className="py-12 text-center opacity-40">
                    <p className="text-[10px] uppercase tracking-widest italic">No history preserved yet.</p>
                </div>
            ) : (
                <div className="space-y-3">
                    {history.map((snapshot) => (
                        <div key={snapshot.id} className="p-3 rounded-md border border-ide-border bg-ide-bg/40 group hover:border-primary/50 transition-all">
                            <div className="flex items-center justify-between mb-2">
                                <span className="text-[10px] font-bold text-ide-text-dim">
                                    {new Date(snapshot.timestamp).toLocaleString()}
                                </span>
                                <button 
                                    onClick={() => handleRestore(snapshot)}
                                    className="p-1 px-2 rounded-lg bg-primary/10 text-primary text-[9px] font-bold uppercase tracking-wider opacity-0 group-hover:opacity-100 transition-opacity flex items-center gap-1"
                                >
                                    <RotateCcw size={10} />
                                    Restore
                                </button>
                            </div>
                            <div className="text-[9px] font-mono text-ide-text-dim truncate opacity-60">
                                ID: {snapshot.id}
                            </div>
                        </div>
                    ))}
                </div>
            )}
        </div>
    );
}

function FileTab({
  file,
  diagnostics,
  isDirty,
  context,
}: {
  file: FileEntry;
  diagnostics: Diagnostic[];
  isDirty: boolean;
  context: InspectorRuntimeContext | null;
}) {
  const statusTone =
    diagnostics.length > 0 ? 'text-red-500' : isDirty ? 'text-amber-600' : 'text-emerald-600';
  const statusLabel = diagnostics.length > 0 ? 'Attention' : isDirty ? 'Dirty' : 'Clean';
  const languageLabel = context?.languageLabel || getLanguageLabelForFile(file.name, 'generic');

  return (
    <div className="divide-y divide-ide-border/50">
      <section className="px-4 py-4">
        <div className="flex items-center gap-2 mb-4">
          <Globe size={14} className="text-primary" />
          <h3 className="text-[11px] font-bold uppercase tracking-[0.2em] text-ide-text">Overview</h3>
        </div>

        <div className="grid grid-cols-2 gap-2 mb-4">
          <InfoCard label="Mode" value="Editable" />
          <InfoCard label="Status" value={statusLabel} color={statusTone} />
        </div>

        <div className="space-y-3">
          <Row label="Language" value={languageLabel} />
          <Row label="Runtime" value={context?.profile.label || 'Generic'} />
          <Row label="Extension" value={file.name.split('.').pop() || 'None'} />
          <Row label="Type" value={file.is_directory ? 'Directory' : 'Workspace File'} />
        </div>
      </section>

      {diagnostics.length > 0 && (
        <section className="px-4 py-4">
          <div className="flex items-center justify-between mb-4">
            <div className="flex items-center gap-2">
              <AlertCircle size={14} className="text-red-500" />
              <h3 className="text-[11px] font-bold uppercase tracking-[0.2em] text-ide-text">Diagnostics</h3>
            </div>
            <span className="text-[9px] font-bold px-1.5 py-0.5 rounded bg-red-500/10 text-red-500 border border-red-500/20">
              {diagnostics.length}
            </span>
          </div>

          <div className="space-y-2">
            {diagnostics.slice(0, 5).map((diagnostic, index) => (
              <button
                key={index}
                onClick={() =>
                  window.dispatchEvent(
                    new CustomEvent('zenith://editor-jump', {
                      detail: { line: diagnostic.line, col: diagnostic.col },
                    })
                  )
                }
                className="w-full text-left rounded-md border border-ide-border bg-ide-bg/40 p-2 hover:bg-ide-bg/80 transition-colors group"
              >
                <div className="flex items-center justify-between gap-3">
                  <span className="text-[9px] font-bold text-red-500/80 tracking-tighter">
                    {diagnostic.code}
                  </span>
                  <span className="text-[9px] font-mono text-ide-text-dim/60">
                    L{diagnostic.line}:{diagnostic.col}
                  </span>
                </div>
                <p className="mt-1 text-[10px] leading-relaxed text-ide-text-dim line-clamp-2 group-hover:text-ide-text transition-colors">
                  {diagnostic.message}
                </p>
              </button>
            ))}
          </div>
        </section>
      )}
    </div>
  );
}

function ContextTab({
  diagnostics,
  context,
  isResolving,
}: {
  diagnostics: Diagnostic[];
  context: InspectorRuntimeContext | null;
  isResolving: boolean;
}) {
  if (!context) {
    return <InspectorLoadingState label="Resolving language context" active={isResolving} />;
  }

  const insightCopy = buildInsightCopy(context);
  const stats = [
    {
      label: 'Toolchain',
      value: context.isConfigured ? 'Configured' : 'Not configured',
      status: context.isConfigured ? 'bg-emerald-500' : 'bg-amber-500',
    },
    {
      label: 'Workspace',
      value: context.signals.length > 0 ? `${context.signals.length} signals` : 'No markers',
      status: context.signals.length > 0 ? 'bg-sky-500' : 'bg-zinc-400',
    },
    {
      label: 'Diagnostics',
      value: diagnostics.length > 0 ? `${diagnostics.length} open` : 'Quiet',
      status: diagnostics.length > 0 ? 'bg-red-500' : 'bg-emerald-500',
    },
  ];

  return (
    <div className="px-4 py-2 space-y-6">
      <section>
        <div className="flex items-center gap-2 mb-4">
          <Sparkles size={14} className={context.profile.iconClass} />
          <h3 className="text-[11px] font-bold uppercase tracking-[0.2em] text-ide-text">
            {context.profile.label} Insight
          </h3>
        </div>

        <div className={`rounded-md border p-3 ${context.profile.surfaceClass}`}>
          <p className="text-[10px] leading-relaxed text-ide-text-dim">
            {insightCopy}
          </p>

          {context.recommendations[0] && (
            <div className="mt-3 rounded-md border border-ide-border/70 bg-ide-bg/50 px-3 py-2">
              <div className="text-[9px] font-bold uppercase tracking-[0.18em] text-ide-text-dim">
                Next move
              </div>
              <p className="mt-1 text-[10px] leading-relaxed text-ide-text">
                {context.recommendations[0]}
              </p>
            </div>
          )}
        </div>
      </section>

      <section>
        <div className="flex items-center gap-2 mb-3">
          <Cpu size={14} className="text-ide-text-dim" />
          <h3 className="text-[11px] font-bold uppercase tracking-[0.2em] text-ide-text">Quick Stats</h3>
        </div>

        <div className="space-y-2">
          {stats.map((stat) => (
            <StatRow key={stat.label} label={stat.label} value={stat.value} status={stat.status} />
          ))}
        </div>
      </section>
    </div>
  );
}

function AttributesTab({
  context,
  isResolving,
  onOpenSettings,
}: {
  context: InspectorRuntimeContext | null;
  isResolving: boolean;
  onOpenSettings: () => void;
}) {
  if (!context) {
    return <InspectorLoadingState label="Resolving runtime config" active={isResolving} />;
  }

  return (
    <div className="px-4 py-2 space-y-6">
      <section>
        <div className="flex items-center gap-2 mb-4 text-ide-text-dim">
          <TerminalSquare size={14} className={context.profile.iconClass} />
          <h3 className="text-[11px] font-bold uppercase tracking-[0.2em] text-ide-text">
            Runtime Resolution
          </h3>
        </div>

        <div className={`rounded-md border p-4 space-y-4 ${context.profile.surfaceClass}`}>
          <div className="flex items-start justify-between gap-3">
            <div className="space-y-1">
              <div className="text-[12px] font-bold text-ide-text">{context.profile.settingsLabel}</div>
              <p className="text-[10px] leading-relaxed text-ide-text-dim">
                {context.profile.description}
              </p>
            </div>

            <StatusBadge ready={context.isConfigured} />
          </div>

          <div className="space-y-2">
            <ConfigField
              label="Binary path"
              value={context.extensionConfig.compilerPath || context.profile.compilerPlaceholder}
              dim={!context.extensionConfig.compilerPath}
            />
            <ConfigField
              label="Build task"
              value={context.extensionConfig.buildCommand || context.profile.buildPlaceholder}
              dim={!context.extensionConfig.buildCommand}
            />
            <ConfigField
              label="LSP path"
              value={context.extensionConfig.lspPath || context.profile.lspPlaceholder}
              dim={!context.extensionConfig.lspPath}
            />
          </div>

          <button
            type="button"
            onClick={onOpenSettings}
            className="w-full rounded-md border border-ide-border bg-ide-bg/60 px-3 py-2 text-[10px] font-bold uppercase tracking-[0.22em] text-ide-text-dim transition-colors hover:text-ide-text hover:bg-ide-bg"
          >
            Open runtime toolchain
          </button>
        </div>
      </section>

      <section>
        <div className="flex items-center gap-2 mb-4 text-ide-text-dim">
          <Settings size={14} />
          <h3 className="text-[11px] font-bold uppercase tracking-[0.2em] text-ide-text">
            Workspace Signals
          </h3>
        </div>

        {context.signals.length > 0 ? (
          <div className="space-y-2">
            {context.signals.map((signal) => (
              <SignalCard key={signal.path} signal={signal} />
            ))}
          </div>
        ) : (
          <EmptySection
            title="No runtime markers nearby"
            body="Zenith did not find project files such as package.json, Cargo.toml, tsconfig, .csproj, Gemfile or runtime configs near the active file."
          />
        )}
      </section>

      <section>
        <div className="flex items-center gap-2 mb-3 text-ide-text-dim">
          <Zap size={14} />
          <h3 className="text-[11px] font-bold uppercase tracking-[0.2em] text-ide-text">
            Recommended Actions
          </h3>
        </div>

        <div className="space-y-2">
          {context.recommendations.map((recommendation, index) => (
            <HintCard key={index} body={recommendation} />
          ))}
        </div>
      </section>
    </div>
  );
}

function InspectorLoadingState({ label, active }: { label: string; active: boolean }) {
  return (
    <div className="px-4 py-4">
      <div className="rounded-md border border-ide-border bg-ide-bg/40 px-4 py-6">
        <div className="flex items-center gap-3">
          <div className={`h-2 w-2 rounded-full ${active ? 'bg-primary animate-pulse' : 'bg-ide-border'}`} />
          <div>
            <div className="text-[11px] font-bold uppercase tracking-[0.22em] text-ide-text">
              {label}
            </div>
            <p className="mt-1 text-[10px] text-ide-text-dim">
              The inspector is reading workspace markers and runtime settings for this file.
            </p>
          </div>
        </div>
      </div>
    </div>
  );
}

function InfoCard({ label, value, color }: { label: string; value: string; color?: string }) {
  return (
    <div className="rounded-md border border-ide-border bg-ide-bg/30 p-2.5">
      <div className="text-[9px] font-bold uppercase tracking-[0.15em] text-ide-text-dim opacity-60">
        {label}
      </div>
      <div className={`mt-0.5 text-[11px] font-bold ${color || 'text-ide-text'}`}>{value}</div>
    </div>
  );
}

function Row({ label, value }: { label: string; value: string }) {
  return (
    <div className="flex items-start justify-between gap-4 py-1.5">
      <span className="shrink-0 text-[10px] font-bold uppercase tracking-[0.1em] text-ide-text-dim/70">
        {label}
      </span>
      <span className="min-w-0 max-w-[160px] break-words text-right text-[11px] text-ide-text">
        {value}
      </span>
    </div>
  );
}

function StatRow({ label, value, status }: { label: string; value: string; status: string }) {
  return (
    <div className="flex items-center justify-between rounded-md border border-ide-border bg-ide-bg/30 p-2">
      <div className="flex items-center gap-2">
        <div className={`w-1.5 h-1.5 rounded-full ${status}`} />
        <span className="text-[10px] text-ide-text-dim font-medium">{label}</span>
      </div>
      <span className="text-[10px] font-bold text-ide-text">{value}</span>
    </div>
  );
}

function StatusBadge({ ready }: { ready: boolean }) {
  return (
    <div
      className={`rounded-full border px-2.5 py-1 text-[9px] font-bold uppercase tracking-[0.2em] ${ready
        ? 'border-emerald-500/20 bg-emerald-500/10 text-emerald-600'
        : 'border-amber-500/20 bg-amber-500/10 text-amber-600'}`}
    >
      {ready ? 'Configured' : 'Not configured'}
    </div>
  );
}

function ConfigField({ label, value, dim }: { label: string; value: string; dim?: boolean }) {
  return (
    <div className="rounded-md border border-ide-border bg-ide-bg/50 px-3 py-2">
      <div className="text-[9px] font-bold uppercase tracking-[0.18em] text-ide-text-dim">
        {label}
      </div>
      <div className={`mt-1 text-[11px] font-mono break-words ${dim ? 'text-ide-text-dim' : 'text-ide-text'}`}>
        {value}
      </div>
    </div>
  );
}

function SignalCard({ signal }: { signal: WorkspaceSignalSummary }) {
  return (
    <div className="rounded-md border border-ide-border bg-ide-bg/40 p-3">
      <div className="flex items-start justify-between gap-3">
        <div className="min-w-0">
          <div className="text-[10px] font-bold text-ide-text truncate">{signal.name}</div>
          <div className="mt-0.5 text-[9px] font-mono text-ide-text-dim/60 truncate">
            {signal.path}
          </div>
        </div>

        <CheckCircle2 size={12} className="text-emerald-600 shrink-0 mt-0.5" />
      </div>

      <p className="mt-2 text-[10px] leading-relaxed text-ide-text-dim">{signal.summary}</p>
    </div>
  );
}

function HintCard({ body }: { body: string }) {
  return (
    <div className="rounded-md border border-ide-border bg-ide-bg/40 px-3 py-2">
      <p className="text-[10px] leading-relaxed text-ide-text">{body}</p>
    </div>
  );
}

function EmptySection({ title, body }: { title: string; body: string }) {
  return (
    <div className="rounded-md border border-ide-border bg-ide-bg/40 px-4 py-5">
      <div className="text-[11px] font-bold uppercase tracking-[0.22em] text-ide-text">{title}</div>
      <p className="mt-2 text-[10px] leading-relaxed text-ide-text-dim">{body}</p>
    </div>
  );
}

function ActionBtn({
  label,
  icon,
  primary,
  onClick,
}: {
  label: string;
  icon: React.ReactNode;
  primary?: boolean;
  onClick: () => void;
}) {
  return (
    <button
      type="button"
      onClick={onClick}
      className={`
        flex items-center justify-center gap-2 rounded-md px-3 py-2 text-[10px] font-bold uppercase tracking-[0.14em] transition-all
        ${primary
          ? 'bg-primary text-white hover:scale-[1.01] active:scale-95'
          : 'bg-ide-bg/80 border border-ide-border text-ide-text-dim hover:bg-ide-bg hover:text-ide-text'}
      `}
    >
      {icon}
      {label}
    </button>
  );
}

function EmptyInspector() {
  return (
    <div className="h-full flex flex-col items-center justify-center p-8 bg-ide-panel/95 text-center">
      <div className="w-16 h-16 rounded-md bg-ide-bg/40 border border-ide-border flex items-center justify-center mb-6">
        <Globe size={24} className="text-ide-text-dim/30" />
      </div>
      <h3 className="text-[12px] font-bold text-ide-text/80 uppercase tracking-widest mb-2">
        No Selection
      </h3>
      <p className="text-[10px] text-ide-text-dim leading-relaxed max-w-[180px]">
        Select a file from the explorer to inspect its runtime profile, metadata and project context.
      </p>
    </div>
  );
}

async function buildInspectorRuntimeContext(
  file: FileEntry,
  fileTree: FileEntry[],
  extensions: Record<string, ExtensionConfig>
): Promise<InspectorRuntimeContext> {
  const runtimeId = detectRuntimeProfileFromFile(file);
  const profile = getRuntimeProfileForSettingsKey(runtimeId);
  const extensionConfig =
    extensions[runtimeId] ||
    (runtimeId !== 'generic' ? DEFAULT_RUNTIME_EXTENSION_CONFIGS[runtimeId] : EMPTY_EXTENSION_CONFIG);
  const signals = await resolveWorkspaceSignals(file, fileTree, profile);
  const languageLabel = getLanguageLabelForFile(file.name, runtimeId);
  const isConfigured = Boolean(
    extensionConfig?.isEnabled && extensionConfig.compilerPath && extensionConfig.buildCommand
  );

  return {
    runtimeId,
    profile,
    languageLabel,
    extensionConfig: extensionConfig || EMPTY_EXTENSION_CONFIG,
    isConfigured,
    hasLsp: Boolean(extensionConfig?.lspPath),
    hasBuildTask: Boolean(extensionConfig?.buildCommand),
    signals,
    recommendations: buildRecommendations(profile, extensionConfig || EMPTY_EXTENSION_CONFIG, signals),
  };
}

function buildRecommendations(
  profile: RuntimeProfile,
  extensionConfig: ExtensionConfig,
  signals: WorkspaceSignalSummary[]
) {
  const recommendations = [...profile.recommendations];

  if (!extensionConfig.isEnabled) {
    recommendations.unshift(
      `Enable ${profile.settingsLabel} in Settings > Extensions so the inspector can treat it as an active toolchain.`
    );
  }

  if (!extensionConfig.compilerPath) {
    recommendations.push(`Set the runtime binary path, for example "${profile.compilerPlaceholder}".`);
  }

  if (!extensionConfig.buildCommand) {
    recommendations.push(`Add a default build command such as "${profile.buildPlaceholder}".`);
  }

  if (signals.length === 0 && profile.id !== 'generic') {
    recommendations.push('Add or open project marker files near this source so Zenith can read richer runtime context.');
  }

  return Array.from(new Set(recommendations)).slice(0, 4);
}

function buildInsightCopy(context: InspectorRuntimeContext) {
  if (context.profile.id === 'generic') {
    return 'This file does not match a dedicated runtime profile yet, so the inspector is falling back to generic metadata only.';
  }

  const toolchainState = context.isConfigured
    ? 'The runtime is configured in Zenith settings.'
    : 'The runtime is recognized, but Zenith does not see it as fully configured yet.';

  const workspaceState =
    context.signals.length > 0
      ? `Workspace markers such as ${context.signals.map((signal) => signal.name).join(', ')} were detected nearby.`
      : 'No nearby project markers were detected, so the inspector is relying on the file extension alone.';

  return `${context.languageLabel} is being resolved through the ${context.profile.label} profile. ${toolchainState} ${workspaceState}`;
}

function flattenWorkspaceFiles(entries: FileEntry[]) {
  const files: FileEntry[] = [];

  for (const entry of entries) {
    if (entry.is_directory) {
      files.push(...flattenWorkspaceFiles(entry.children || []));
    } else {
      files.push(entry);
    }
  }

  return files;
}

function normalizePath(input: string) {
  return input.replace(/\\/g, '/').toLowerCase();
}

function getSharedPathScore(a: string, b: string) {
  const left = normalizePath(a).split('/').filter(Boolean);
  const right = normalizePath(b).split('/').filter(Boolean);
  let score = 0;

  while (score < left.length && score < right.length && left[score] === right[score]) {
    score += 1;
  }

  return score;
}

async function resolveWorkspaceSignals(
  activeFile: FileEntry,
  fileTree: FileEntry[],
  profile: RuntimeProfile
) {
  const workspaceFiles = flattenWorkspaceFiles(fileTree);
  const matchingFiles = workspaceFiles
    .filter((entry) => matchesRuntimeMarker(profile, entry.name))
    .sort((left, right) => {
      const rightScore = getSharedPathScore(activeFile.path, right.path);
      const leftScore = getSharedPathScore(activeFile.path, left.path);

      if (leftScore !== rightScore) {
        return rightScore - leftScore;
      }

      return left.path.length - right.path.length;
    })
    .slice(0, 4);

  const signals = await Promise.all(
    matchingFiles.map(async (entry) => ({
      name: entry.name,
      path: entry.path,
      summary: await summarizeWorkspaceSignal(entry, profile.id),
    }))
  );

  return signals;
}

async function summarizeWorkspaceSignal(entry: FileEntry, profileId: RuntimeProfileId): Promise<string> {
   return `Found ${entry.name} at project root, which Zenith uses to anchor ${profileId} context.`;
}

