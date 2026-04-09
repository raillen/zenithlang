import { useEffect } from "react";
import { MainLayout } from "./components/MainLayout";
import { useProjectStore } from "./store/useProjectStore";
import { Group as PanelGroup, Panel, Separator as PanelResizeHandle } from "react-resizable-panels";
import { EditorPanel } from "./components/EditorPanel";
import { CommandPalette } from "./components/CommandPalette";
import { MarkdownDocument } from "./components/MarkdownDocument";
import { TopToolbar } from "./components/TopToolbar";
import { BottomPanel } from "./components/BottomPanel";
import { useTranslation } from "react-i18next";
import { safeInvoke as invoke } from "./utils/tauri";
import { useTerminalBridge } from "./hooks/useTerminalBridge";
import { uiMarker } from "./utils/debugSelectors";

function App() 
  const utilityPanelEnabled = false;
  const {
    activeFilePath,
    openFile,
    setWorkspaceRoot,
    isBuilding,
    setIsBuilding,
    setBuildLogs,
    isBottomPanelOpen,
    setBottomPanelOpen,
    mainVerticalLayout,
    setMainVerticalLayout,
    sidePanelLayout,
    setSidePanelLayout,
    utilityPanelDock,
    dirtyFiles,
  } = useProjectStore();
  const { t } = useTranslation();
  useTerminalBridge();

  const runBuild = async () => {
    if (!activeFilePath || isBuilding) return;
    
    setIsBuilding(true);
    setBuildLogs(t('loading'));
    
    try {
      const output = await invoke<string>('run_compiler', { inputPath: activeFilePath });
      setBuildLogs(output);
    } catch (err: any) {
      setBuildLogs(err.toString());
    } finally {
      setIsBuilding(false);
    }
  };

  const isMarkdown = activeFilePath?.endsWith('.md');
  const isUtilityDockBottom = utilityPanelDock === 'bottom';
  const isUtilityPanelVisible = utilityPanelEnabled && isBottomPanelOpen;

  useEffect(() => {
    const handleOpenFileDialog = async () => {
      try {
        const pickedPath = await invoke<string | null>('pick_file');
        if (pickedPath) {
          openFile(pickedPath);
        }
      } catch (error) {
        console.error('Failed to open file picker:', error);
      }
    };

    const handleOpenFolderDialog = async () => {
      try {
        const pickedPath = await invoke<string | null>('pick_folder');
        if (pickedPath) {
          setWorkspaceRoot(pickedPath);
        }
      } catch (error) {
        console.error('Failed to open folder picker:', error);
      }
    };

    window.addEventListener('ide:open-file-dialog', handleOpenFileDialog);
    window.addEventListener('ide:open-folder-dialog', handleOpenFolderDialog);

    return () => {
      window.removeEventListener('ide:open-file-dialog', handleOpenFileDialog);
      window.removeEventListener('ide:open-folder-dialog', handleOpenFolderDialog);
    };
  }, [openFile, setWorkspaceRoot]);

  useEffect(() => {
    if (!utilityPanelEnabled && isBottomPanelOpen) {
      setBottomPanelOpen(false);
    }
  }, [isBottomPanelOpen, setBottomPanelOpen, utilityPanelEnabled]);

  return (
    <MainLayout>
      <CommandPalette />
      <div className="flex min-h-0 flex-1 flex-col" {...uiMarker("app-content", { activeFile: activeFilePath ?? "none" })}>
        <TopToolbar
          activeFilePath={activeFilePath}
          isMarkdown={!!isMarkdown}
          isBuilding={isBuilding}
          isPanelOpen={isUtilityPanelVisible}
          isDirty={activeFilePath ? dirtyFiles.includes(activeFilePath) : false}
          onRunBuild={runBuild}
          onTogglePanel={() => {
            if (!utilityPanelEnabled) {
              return;
            }
            setBottomPanelOpen(!isBottomPanelOpen);
          }}
        />

        {isUtilityPanelVisible && isUtilityDockBottom ? (
          <PanelGroup
            id="ide-main-vertical"
            orientation="vertical"
            defaultLayout={{
              'ide-document-surface': mainVerticalLayout.editor,
              'ide-bottom-panel': mainVerticalLayout.bottom,
            }}
            onLayoutChanged={(layout) => {
              setMainVerticalLayout({
                editor: layout['ide-document-surface'] ?? mainVerticalLayout.editor,
                bottom: layout['ide-bottom-panel'] ?? mainVerticalLayout.bottom,
              });
            }}
            className="min-h-0 flex-1"
            {...uiMarker("app-workspace-layout", { orientation: "vertical", utilityDock: utilityPanelDock })}
          >
            <Panel id="ide-document-surface" minSize="45px" className="min-h-0">
              <DocumentSurface activeFilePath={activeFilePath} isMarkdown={!!isMarkdown} t={t} />
            </Panel>

            {isUtilityPanelVisible && (
              <>
                <PanelResizeHandle
                  className="group relative z-10 flex h-3 shrink-0 cursor-row-resize items-center justify-center bg-transparent"
                  {...uiMarker("bottom-panel-resize-handle", { orientation: "vertical" })}
                >
                  <div
                    className="pointer-events-none h-px w-full transition-colors duration-200 group-hover:bg-xcode-accent group-active:bg-xcode-accent"
                    style={{ background: 'var(--border-strong)' }}
                  />
                </PanelResizeHandle>
                <Panel id="ide-bottom-panel" minSize="15px" maxSize="45px" className="min-h-0">
                  <BottomPanel />
                </Panel>
              </>
            )}
          </PanelGroup>
        ) : isUtilityPanelVisible ? (
          <PanelGroup
            id="ide-main-horizontal-utility"
            orientation="horizontal"
            defaultLayout={{
              'ide-document-surface': sidePanelLayout.editor,
              'ide-side-utility-panel': sidePanelLayout.utility,
            }}
            onLayoutChanged={(layout) => {
              setSidePanelLayout({
                editor: layout['ide-document-surface'] ?? sidePanelLayout.editor,
                utility: layout['ide-side-utility-panel'] ?? sidePanelLayout.utility,
              });
            }}
            className="min-h-0 flex-1"
            {...uiMarker("app-workspace-layout", { orientation: "horizontal", utilityDock: utilityPanelDock })}
          >
            {utilityPanelDock === 'left' ? (
              <>
                <Panel id="ide-side-utility-panel" minSize="18px" maxSize="46px" className="min-h-0 min-w-0">
                  <BottomPanel />
                </Panel>
                <PanelResizeHandle
                  className="group relative -mx-2 z-10 flex w-4 shrink-0 cursor-col-resize items-stretch justify-center bg-transparent"
                  {...uiMarker("utility-panel-resize-handle", { orientation: "horizontal", dock: "left" })}
                >
                  <div
                    className="pointer-events-none absolute inset-y-0 left-1/2 w-px -translate-x-1/2 transition-colors duration-200 group-hover:bg-xcode-accent group-active:bg-xcode-accent"
                    style={{ background: 'var(--border-strong)' }}
                  />
                </PanelResizeHandle>
              </>
            ) : null}

            <Panel id="ide-document-surface" minSize="45px" className="min-h-0 min-w-0">
              <DocumentSurface activeFilePath={activeFilePath} isMarkdown={!!isMarkdown} t={t} />
            </Panel>

            {utilityPanelDock === 'right' ? (
              <>
                <PanelResizeHandle
                  className="group relative -mx-2 z-10 flex w-4 shrink-0 cursor-col-resize items-stretch justify-center bg-transparent"
                  {...uiMarker("utility-panel-resize-handle", { orientation: "horizontal", dock: "right" })}
                >
                  <div
                    className="pointer-events-none absolute inset-y-0 left-1/2 w-px -translate-x-1/2 transition-colors duration-200 group-hover:bg-xcode-accent group-active:bg-xcode-accent"
                    style={{ background: 'var(--border-strong)' }}
                  />
                </PanelResizeHandle>
                <Panel id="ide-side-utility-panel" minSize="18px" maxSize="46px" className="min-h-0 min-w-0">
                  <BottomPanel />
                </Panel>
              </>
            ) : null}
          </PanelGroup>
        ) : (
          <div className="relative z-10 min-h-0 flex-1" {...uiMarker("app-workspace-layout", { orientation: "single", utilityDock: utilityPanelDock })}>
            <DocumentSurface activeFilePath={activeFilePath} isMarkdown={!!isMarkdown} t={t} />
          </div>
        )}
      </div>
    </MainLayout>
  );
}

function DocumentSurface({
  activeFilePath,
  isMarkdown,
  t,
}: {
  activeFilePath: string | null;
  isMarkdown: boolean;
  t: (key: string) => string;
}) {
  return (
    <div
      className="relative isolate z-10 flex h-full min-h-0 flex-1 flex-col"
      style={{ background: 'color-mix(in srgb, var(--ide-editor) 76%, white 24%)' }}
      {...uiMarker("document-surface", {
        state: activeFilePath ? (isMarkdown ? "markdown" : "editor") : "empty",
        filePath: activeFilePath ?? "none",
      })}
    >
      {!activeFilePath ? (
        <div className="flex h-full min-h-0 flex-1 flex-col items-center justify-center" {...uiMarker("document-empty-state")}>
          <img
            src="/logo-with-text.svg"
            alt="Zenith"
            className="mb-5 h-12 w-auto opacity-80"
            {...uiMarker("document-empty-logo")}
          />
          <p className="text-sm" style={{ color: 'var(--text-quaternary)' }} {...uiMarker("document-empty-message")}>
            {t('no_file_selected')}
          </p>
        </div>
      ) : isMarkdown ? (
        <div className="relative z-10 min-h-0 flex-1" {...uiMarker("markdown-surface-host", { filePath: activeFilePath })}>
          <MarkdownDocument path={activeFilePath} />
        </div>
      ) : (
        <div className="relative z-10 min-h-0 flex-1 select-text" {...uiMarker("editor-surface-host", { filePath: activeFilePath })}>
          <EditorPanel path={activeFilePath} />
        </div>
      )}
    </div>
  );
}

export default App;
