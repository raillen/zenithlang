import { useEffect, useMemo, useState, type CSSProperties } from 'react';
import { Group as PanelGroup, Panel, Separator as PanelResizeHandle } from 'react-resizable-panels';
import { Check, FileText, FloppyDisk, FolderOpen, Keyboard, X } from '@phosphor-icons/react';
import { ActivityBar } from './ActivityBar';
import { SidebarPane } from './SidebarPane';
import { StatusBar } from './StatusBar';
import { useProjectStore } from '../store/useProjectStore';
import { editorFontOptions } from '../utils/editorPreferences';
import { uiMarker } from '../utils/debugSelectors';
import { isTauriRuntime } from '../utils/tauri';

interface MainLayoutProps {
  children: React.ReactNode;
}

export function MainLayout({ children }: MainLayoutProps) {
  const {
    openFiles,
    activeFilePath,
    activeSidebarView,
    panelLayout,
    utilityPanelDock,
    dirtyFiles,
    setActiveFile,
    closeFile,
    setPanelLayout,
    setActiveSidebarView,
    theme,
    setTheme,
    fontFamily,
    setFontFamily,
    editorFontFamily,
    setEditorFontFamily,
    editorFontSize,
    setEditorFontSize,
    acrylicOpacity,
    setAcrylicOpacity,
    acrylicBlur,
    setAcrylicBlur,
    setUtilityPanelDock,
    focusMode,
    setFocusMode,
  } = useProjectStore();
  const [isSettingsOpen, setIsSettingsOpen] = useState(false);
  const runtime = useMemo(() => (isTauriRuntime() ? 'tauri' : 'web'), []);

  useEffect(() => {
    document.body.dataset.runtime = runtime;

    return () => {
      delete document.body.dataset.runtime;
    };
  }, [runtime]);

  const activeFileName = useMemo(
    () => (activeFilePath ? activeFilePath.split(/[/\\]/).pop() || activeFilePath : null),
    [activeFilePath]
  );
  const shellStyle = useMemo(
    () =>
      ({
        ['--acrylic-opacity-percent' as '--acrylic-opacity-percent']: `${acrylicOpacity}%`,
        ['--acrylic-blur-radius' as '--acrylic-blur-radius']: `${acrylicBlur}px`,
        ['--editor-font-family' as '--editor-font-family']:
          editorFontOptions.find((option) => option.id === editorFontFamily)?.stack ?? editorFontOptions[0].stack,
        ['--editor-font-size' as '--editor-font-size']: `${editorFontSize}px`,
      }) as CSSProperties,
    [acrylicBlur, acrylicOpacity, editorFontFamily, editorFontSize]
  );

  return (
    <div
      className="ide-shell flex h-full min-h-full w-full overflow-hidden transition-colors duration-300"
      data-theme={theme}
      data-font={fontFamily}
      data-focus-mode={focusMode ? 'true' : 'false'}
      data-runtime={runtime}
      style={shellStyle}
      {...uiMarker('ide-shell', { theme, fontFamily, focusMode, runtime })}
    >
      <PanelGroup
        id="ide-main-layout"
        orientation="horizontal"
        defaultLayout={{
          'ide-sidebar-shell': panelLayout.sidebar,
          'ide-editor-shell': panelLayout.editor,
        }}
        onLayoutChanged={(layout) => {
          setPanelLayout({
            sidebar: layout['ide-sidebar-shell'] ?? panelLayout.sidebar,
            editor: layout['ide-editor-shell'] ?? panelLayout.editor,
          });
        }}
        resizeTargetMinimumSize={{ fine: 12, coarse: 24 }}
        className="h-full min-h-0 w-full"
        {...uiMarker('main-layout-panels', { orientation: 'horizontal' })}
      >
        <Panel
          id="ide-sidebar-shell"
          defaultSize="330px"
          minSize="280px"
          maxSize="560px"
          groupResizeBehavior="preserve-pixel-size"
          className="ide-glass-sidebar flex min-h-0 min-w-0 flex-row overflow-hidden border-r"
          {...uiMarker('sidebar-shell', { activeView: activeSidebarView })}
        >
          <ActivityBar
            activeView={activeSidebarView}
            onViewChange={setActiveSidebarView}
            onOpenSettings={() => setIsSettingsOpen(true)}
          />
          <div className="flex min-h-0 min-w-0 flex-1 flex-col overflow-hidden" {...uiMarker('sidebar-content-shell', { activeView: activeSidebarView })}>
            <div
              className="h-4 shrink-0 border-b"
              style={{ borderColor: 'var(--border)', background: 'color-mix(in srgb, var(--ide-toolbar) 86%, white 14%)' }}
              {...uiMarker('sidebar-drag-region')}
            />
            <SidebarPane activeView={activeSidebarView} />
          </div>
        </Panel>

        <PanelResizeHandle
          className="group relative -mx-2 z-10 flex w-4 shrink-0 cursor-col-resize items-stretch justify-center bg-transparent"
          {...uiMarker('main-layout-resize-handle', { orientation: 'horizontal' })}
        >
          <div
            className="pointer-events-none absolute inset-y-0 left-1/2 w-px -translate-x-1/2 transition-colors duration-200 group-hover:bg-xcode-accent group-active:bg-xcode-accent"
            style={{ background: 'var(--border-strong)' }}
          />
        </PanelResizeHandle>

        <Panel id="ide-editor-shell" className="ide-glass-editor flex min-h-0 min-w-0 flex-col" {...uiMarker('editor-shell', { activeFile: activeFilePath ?? 'none' })}>
          <header className="ide-toolbar-surface flex h-11 shrink-0 flex-col border-b" {...uiMarker('editor-shell-header')}>
            <div className="h-3 shrink-0" {...uiMarker('editor-shell-drag-region')} />
            <div className="flex min-w-0 flex-1 items-center overflow-hidden" {...uiMarker('editor-shell-command-row')}>
              <div className="flex shrink-0 items-center gap-1 border-r px-2" style={{ borderColor: 'var(--border)' }} {...uiMarker('editor-shell-primary-actions')}>
                <HeaderCommandButton
                  label="Open folder"
                  onClick={() => window.dispatchEvent(new CustomEvent('ide:open-folder-dialog'))}
                  debugId="open-folder"
                >
                  <FolderOpen size={14} />
                </HeaderCommandButton>
                <HeaderCommandButton
                  label="Open file"
                  onClick={() => window.dispatchEvent(new CustomEvent('ide:open-file-dialog'))}
                  debugId="open-file"
                >
                  <FileText size={14} />
                </HeaderCommandButton>
                <HeaderCommandButton
                  label="Save"
                  onClick={() => window.dispatchEvent(new CustomEvent('ide:save-active-document'))}
                  disabled={!activeFilePath}
                  active={!!activeFilePath && dirtyFiles.includes(activeFilePath)}
                  debugId="save-file"
                >
                  <FloppyDisk size={14} weight={activeFilePath && dirtyFiles.includes(activeFilePath) ? 'fill' : 'regular'} />
                </HeaderCommandButton>
                <HeaderCommandButton
                  label="Command palette"
                  onClick={() => window.dispatchEvent(new CustomEvent('ide:toggle-command-palette'))}
                  debugId="command-palette"
                >
                  <Keyboard size={14} />
                </HeaderCommandButton>
              </div>

              <div className="min-w-0 flex-1 overflow-x-auto no-scrollbar" {...uiMarker('editor-tabs-scroll-region')}>
                <div className="flex min-w-max items-center" {...uiMarker('editor-tabs-list')}>
              {openFiles.length === 0 ? (
                <div className="px-4 text-[12px]" style={{ color: 'var(--text-quaternary)' }} {...uiMarker('editor-tabs-empty-state')}>
                  Open a file from Explorer to begin
                </div>
              ) : (
                openFiles.map((path) => {
                  const fileName = path.split(/[/\\]/).pop() || path;
                  const isActive = activeFilePath === path;
                  const isDirty = dirtyFiles.includes(path);

                  return (
                    <button
                      key={path}
                      type="button"
                      onClick={() => setActiveFile(path)}
                      className={`group flex h-full max-w-[220px] shrink-0 items-center gap-2 border-r px-4 text-left text-xs transition ${
                        isActive
                          ? 'text-[color:var(--text-primary)]'
                          : 'text-[color:var(--text-secondary)] hover:text-[color:var(--text-primary)]'
                      }`}
                      {...uiMarker('editor-tab', { filePath: path, active: isActive, dirty: isDirty })}
                      style={{
                        borderColor: 'var(--border)',
                        background: isActive
                          ? 'color-mix(in srgb, var(--ide-panel) 72%, white 28%)'
                          : 'transparent',
                      }}
                    >
                      <span
                        className="h-2 w-2 shrink-0 rounded-full border"
                        style={{
                          background: isDirty ? 'var(--accent)' : 'transparent',
                          borderColor: isDirty ? 'var(--accent)' : 'var(--border-strong)',
                        }}
                      />
                      <span className="truncate">{fileName}</span>
                      {activeFileName === fileName && isActive && (
                        <span
                          className="rounded-full px-1.5 py-0.5 text-[10px]"
                          style={{
                            background: 'var(--accent-soft)',
                            color: 'var(--accent)',
                          }}
                          {...uiMarker('editor-tab-badge', { filePath: path, state: 'active' })}
                        >
                          Active
                        </span>
                      )}
                      <X
                        size={11}
                        weight="bold"
                        className="ml-auto shrink-0 opacity-0 transition group-hover:opacity-100"
                        style={{ color: 'var(--text-tertiary)' }}
                        onClick={(event) => {
                          event.stopPropagation();
                          closeFile(path);
                        }}
                        {...uiMarker('editor-tab-close', { filePath: path })}
                      />
                    </button>
                  );
                })
              )}
                </div>
              </div>
            </div>
          </header>

          <div className="relative isolate z-0 min-h-0 flex-1 overflow-hidden" {...uiMarker('editor-shell-body')}>
            {children}
          </div>
          <StatusBar />
        </Panel>
      </PanelGroup>

      {isSettingsOpen && (
        <div
          className="fixed inset-0 z-50 flex items-center justify-center px-6"
          style={{ background: 'color-mix(in srgb, var(--ide-overlay) 72%, black 28%)' }}
          {...uiMarker('settings-overlay')}
        >
          <div
            className="w-full max-w-xl border"
            style={{
              borderColor: 'var(--border)',
              background: 'color-mix(in srgb, var(--ide-panel) 82%, white 18%)',
              boxShadow: 'var(--panel-shadow-strong)',
              backdropFilter: 'blur(26px)',
            }}
            {...uiMarker('settings-dialog')}
          >
            <div className="flex items-center justify-between border-b px-5 py-4" style={{ borderColor: 'var(--border)' }} {...uiMarker('settings-header')}>
              <div>
                <div className="text-[11px] font-medium" style={{ color: 'var(--text-tertiary)' }}>
                  Preferences
                </div>
                <div className="mt-1 text-sm" style={{ color: 'var(--text-primary)' }}>Editor and shell configuration</div>
              </div>
              <button
                type="button"
                onClick={() => setIsSettingsOpen(false)}
                className="ide-button rounded-full p-2"
                {...uiMarker('settings-close-button')}
              >
                <X size={14} weight="bold" />
              </button>
            </div>

            <div className="grid gap-6 px-5 py-5 md:grid-cols-[1.2fr_1fr]" {...uiMarker('settings-content')}>
              <section {...uiMarker('settings-column', { column: 'appearance' })}>
                <div className="text-[12px] font-medium" style={{ color: 'var(--text-tertiary)' }}>
                  Theme
                </div>
                <div className="mt-3 grid grid-cols-5 gap-2" {...uiMarker('theme-options')}>
                  {(['zenith', 'nord', 'neon', 'dracula', 'neuro'] as const).map((value) => (
                    <button
                      key={value}
                      type="button"
                      onClick={() => setTheme(value)}
                      title={value}
                      className={`h-12 rounded-2xl border transition ${
                        theme === value ? 'ring-1 ring-xcode-accent/35' : ''
                      }`}
                      style={{
                        borderColor: theme === value ? 'var(--accent)' : 'var(--border)',
                        background:
                          value === 'zenith'
                            ? '#ECEEEE'
                            : value === 'nord'
                              ? '#88c0d0'
                              : value === 'neon'
                                ? '#ff00ff'
                                : value === 'dracula'
                                  ? '#bd93f9'
                              : '#f5f5dc',
                      }}
                      {...uiMarker('theme-option', { themeOption: value, active: theme === value })}
                    />
                  ))}
                </div>

                <div className="ide-soft-surface mt-5 rounded-[1.25rem] p-4" {...uiMarker('settings-card', { section: 'acrylic-surface' })}>
                  <div className="text-sm" style={{ color: 'var(--text-primary)' }}>Acrylic Surface</div>
                  <div className="mt-1 text-[12px] leading-5" style={{ color: 'var(--text-secondary)' }}>
                    Control how translucent and how blurred the shell surfaces feel.
                  </div>

                  <div className="mt-4">
                    <div className="flex items-center justify-between text-[11px]" style={{ color: 'var(--text-secondary)' }}>
                      <span>Opacity</span>
                      <span>{acrylicOpacity}%</span>
                    </div>
                    <input
                      type="range"
                      min="20"
                      max="100"
                      step="1"
                      value={acrylicOpacity}
                      onChange={(event) => setAcrylicOpacity(Number(event.target.value))}
                      className="ide-range mt-2 w-full"
                      {...uiMarker('settings-range', { control: 'acrylic-opacity' })}
                    />
                  </div>

                  <div className="mt-4">
                    <div className="flex items-center justify-between text-[11px]" style={{ color: 'var(--text-secondary)' }}>
                      <span>Blur</span>
                      <span>{acrylicBlur}px</span>
                    </div>
                    <input
                      type="range"
                      min="0"
                      max="40"
                      step="1"
                      value={acrylicBlur}
                      onChange={(event) => setAcrylicBlur(Number(event.target.value))}
                      className="ide-range mt-2 w-full"
                      {...uiMarker('settings-range', { control: 'acrylic-blur' })}
                    />
                  </div>
                </div>

                <div className="ide-soft-surface mt-5 rounded-[1.25rem] p-4" {...uiMarker('settings-card', { section: 'utility-panel-dock' })}>
                  <div className="text-sm" style={{ color: 'var(--text-primary)' }}>Utility Panel Dock</div>
                  <div className="mt-1 text-[12px] leading-5" style={{ color: 'var(--text-secondary)' }}>
                    Choose whether output, problems and terminal live below the editor or on one side.
                  </div>

                  <select
                    value={utilityPanelDock}
                    onChange={(event) => setUtilityPanelDock(event.target.value as typeof utilityPanelDock)}
                    className="ide-input mt-3 h-10 w-full rounded-xl px-3 text-sm"
                    {...uiMarker('settings-select', { control: 'utility-panel-dock' })}
                  >
                    <option value="left">Left</option>
                    <option value="bottom">Bottom</option>
                    <option value="right">Right</option>
                  </select>
                </div>
              </section>

              <section className="space-y-5" {...uiMarker('settings-column', { column: 'accessibility' })}>
                <div className="text-[12px] font-medium" style={{ color: 'var(--text-tertiary)' }}>
                  Accessibility
                </div>
                <div className="ide-soft-surface rounded-[1.25rem] p-4" {...uiMarker('settings-card', { section: 'cognitive-focus' })}>
                  <div className="flex items-start justify-between gap-4">
                    <div>
                      <div className="text-sm" style={{ color: 'var(--text-primary)' }}>Cognitive Focus</div>
                      <div className="mt-1 text-[12px] leading-5" style={{ color: 'var(--text-secondary)' }}>
                        Reduces decorative emphasis, calms hover states and increases breathing room for attention and reading.
                      </div>
                    </div>
                    <button
                      type="button"
                      onClick={() => setFocusMode(!focusMode)}
                      className="mt-0.5 flex h-8 w-14 items-center rounded-full p-1"
                      style={{ background: 'var(--accent-faint)' }}
                      aria-pressed={focusMode}
                      {...uiMarker('settings-toggle', { control: 'focus-mode', active: focusMode })}
                    >
                      <span
                        className={`flex h-6 w-6 items-center justify-center rounded-full transition-transform ${
                          focusMode ? 'translate-x-6' : ''
                        }`}
                        style={{ background: focusMode ? 'var(--accent)' : 'white', color: focusMode ? 'white' : 'var(--text-tertiary)' }}
                      >
                        {focusMode ? <Check size={12} weight="bold" /> : null}
                      </span>
                    </button>
                  </div>
                </div>
                <div className="ide-soft-surface mt-3 rounded-[1.25rem] p-4" {...uiMarker('settings-card', { section: 'reading-profile' })}>
                  <div className="text-sm" style={{ color: 'var(--text-primary)' }}>Interface Reading Profile</div>
                  <div className="mt-1 text-[12px] leading-5" style={{ color: 'var(--text-secondary)' }}>
                    Switch the interface between the default profile and OpenDyslexic.
                  </div>
                  <button
                    type="button"
                    onClick={() => setFontFamily(fontFamily === 'SF Mono' ? 'OpenDyslexic' : 'SF Mono')}
                    className="mt-4 flex h-8 w-14 items-center rounded-full p-1"
                    style={{ background: 'var(--accent-faint)' }}
                    {...uiMarker('settings-toggle', { control: 'reading-profile', active: fontFamily === 'OpenDyslexic' })}
                  >
                    <span
                      className={`h-6 w-6 rounded-full transition-transform ${
                        fontFamily === 'OpenDyslexic' ? 'translate-x-6' : ''
                      }`}
                      style={{ background: fontFamily === 'OpenDyslexic' ? 'var(--accent)' : 'white' }}
                    />
                  </button>
                </div>

                <div className="ide-soft-surface rounded-[1.25rem] p-4" {...uiMarker('settings-card', { section: 'code-font' })}>
                  <div className="text-sm" style={{ color: 'var(--text-primary)' }}>Code Font Family</div>
                  <div className="mt-1 text-[12px] leading-5" style={{ color: 'var(--text-secondary)' }}>
                    Applies to code editors and markdown code blocks.
                  </div>
                  <select
                    value={editorFontFamily}
                    onChange={(event) => setEditorFontFamily(event.target.value as (typeof editorFontOptions)[number]['id'])}
                    className="ide-input mt-3 h-10 w-full rounded-xl px-3 text-sm"
                    {...uiMarker('settings-select', { control: 'code-font-family' })}
                  >
                    {editorFontOptions.map((option) => (
                      <option key={option.id} value={option.id}>
                        {option.label}
                      </option>
                    ))}
                  </select>

                  <div className="mt-4">
                    <div className="flex items-center justify-between text-[11px]" style={{ color: 'var(--text-secondary)' }}>
                      <span>Code Font Size</span>
                      <span>{editorFontSize}px</span>
                    </div>
                    <input
                      type="range"
                      min="11"
                      max="24"
                      step="1"
                      value={editorFontSize}
                      onChange={(event) => setEditorFontSize(Number(event.target.value))}
                      className="ide-range mt-2 w-full"
                      {...uiMarker('settings-range', { control: 'code-font-size' })}
                    />
                    <div className="mt-3 rounded-xl border px-3 py-2 text-[12px]" style={{ borderColor: 'var(--border)', color: 'var(--text-secondary)', fontFamily: 'var(--editor-font-family)', fontSize: 'var(--editor-font-size)' }} {...uiMarker('settings-font-preview')}>
                      function zenith_preview() {'{'} return true {'}'}
                    </div>
                  </div>
                </div>
              </section>
            </div>
          </div>
        </div>
      )}
    </div>
  );
}

function HeaderCommandButton({
  label,
  onClick,
  disabled = false,
  active = false,
  debugId,
  children,
}: {
  label: string;
  onClick: () => void;
  disabled?: boolean;
  active?: boolean;
  debugId: string;
  children: React.ReactNode;
}) {
  return (
    <button
      type="button"
      onClick={onClick}
      disabled={disabled}
      title={label}
      aria-label={label}
      className={`flex h-7 w-7 items-center justify-center rounded-lg border transition ${
        disabled ? 'cursor-not-allowed opacity-45' : ''
      }`}
      style={{
        borderColor: active ? 'color-mix(in srgb, var(--accent) 22%, var(--border))' : 'transparent',
        background: active ? 'var(--accent-soft)' : 'transparent',
        color: active ? 'var(--accent)' : 'var(--text-secondary)',
      }}
      {...uiMarker('editor-shell-action', { action: debugId, disabled, active })}
    >
      {children}
    </button>
  );
}
