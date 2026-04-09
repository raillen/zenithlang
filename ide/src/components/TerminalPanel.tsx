import { useCallback, useEffect, useMemo, useRef } from 'react';
import { ArrowClockwise, Pulse, TerminalWindow, WarningCircle } from '@phosphor-icons/react';
import { FitAddon } from '@xterm/addon-fit';
import { Terminal } from '@xterm/xterm';
import '@xterm/xterm/css/xterm.css';
import { useProjectStore } from '../store/useProjectStore';
import { type TerminalCreateResponse } from '../utils/terminal';
import { isTauriRuntime, safeInvoke as invoke } from '../utils/tauri';
import { uiMarker } from '../utils/debugSelectors';

function getTerminalTheme(node: HTMLElement) {
  const styles = getComputedStyle(node);
  const read = (name: string, fallback: string) => styles.getPropertyValue(name).trim() || fallback;

  return {
    background: 'transparent',
    foreground: read('--text-primary', '#1d1d1f'),
    cursor: read('--accent', '#0071e3'),
    cursorAccent: read('--ide-panel', '#ffffff'),
    selectionBackground: read('--accent-soft', 'rgba(0, 113, 227, 0.12)'),
    black: read('--text-primary', '#1d1d1f'),
    brightBlack: read('--text-tertiary', '#7d7d84'),
    red: read('--danger', '#cf4b63'),
    brightRed: read('--danger', '#cf4b63'),
    green: read('--success', '#2f9e73'),
    brightGreen: read('--success', '#2f9e73'),
    yellow: read('--warning', '#b7791f'),
    brightYellow: read('--warning', '#b7791f'),
    blue: read('--accent', '#0071e3'),
    brightBlue: read('--accent-strong', '#0077ed'),
    magenta: read('--accent-strong', '#0077ed'),
    brightMagenta: read('--accent', '#0071e3'),
    cyan: read('--accent', '#0071e3'),
    brightCyan: read('--accent-strong', '#0077ed'),
    white: read('--text-primary', '#ffffff'),
    brightWhite: read('--text-primary', '#ffffff'),
  };
}

export function TerminalPanel() {
  const {
    workspaceRoot,
    theme,
    editorFontSize,
    terminalSessionId,
    terminalSessionState,
    terminalSnapshot,
    terminalError,
    terminalResetNonce,
    setTerminalSessionState,
    setTerminalError,
    requestTerminalReset,
  } = useProjectStore();

  const containerRef = useRef<HTMLDivElement | null>(null);
  const terminalRef = useRef<Terminal | null>(null);
  const fitAddonRef = useRef<FitAddon | null>(null);
  const resizeObserverRef = useRef<ResizeObserver | null>(null);
  const writtenLengthRef = useRef(0);
  const lastResetNonceRef = useRef(terminalResetNonce);

  const isDesktopShell = useMemo(() => isTauriRuntime(), []);

  const syncTerminalSize = useCallback(() => {
    const fitAddon = fitAddonRef.current;
    const terminal = terminalRef.current;
    if (!fitAddon || !terminal) {
      return;
    }

    fitAddon.fit();

    const currentSessionId = useProjectStore.getState().terminalSessionId;
    if (currentSessionId === null || terminal.cols <= 0 || terminal.rows <= 0) {
      return;
    }

    void invoke('terminal_resize', {
      sessionId: currentSessionId,
      cols: terminal.cols,
      rows: terminal.rows,
    }).catch((error) => {
      console.error('Unable to resize terminal session:', error);
    });
  }, []);

  const applyTerminalTheme = useCallback(() => {
    const node = containerRef.current;
    const terminal = terminalRef.current;

    if (!node || !terminal) {
      return;
    }

    terminal.options.theme = getTerminalTheme(node);
    terminal.options.fontFamily = getComputedStyle(node).getPropertyValue('--editor-font-family').trim() || 'monospace';
    terminal.options.fontSize = editorFontSize;
    terminal.refresh(0, Math.max(terminal.rows - 1, 0));
  }, [editorFontSize]);

  const createSession = useCallback(async () => {
    if (!isDesktopShell || terminalRef.current === null) {
      return;
    }

    const currentState = useProjectStore.getState();
    if (currentState.terminalSessionId !== null || currentState.terminalSessionState === 'starting') {
      return;
    }

    setTerminalError(null);
    setTerminalSessionState('starting');
    syncTerminalSize();

    try {
      const terminal = terminalRef.current;
      const response = await invoke<TerminalCreateResponse>('terminal_create', {
        cwd: workspaceRoot,
        cols: terminal?.cols ?? 120,
        rows: terminal?.rows ?? 32,
      });

      useProjectStore.getState().setTerminalSession(response.sessionId, 'ready');
      syncTerminalSize();
    } catch (error) {
      const message = error instanceof Error ? error.message : String(error);
      console.error('Unable to create terminal session:', error);
      useProjectStore.getState().setTerminalError(message);
      useProjectStore.getState().setTerminalSession(null, 'error');
    }
  }, [isDesktopShell, setTerminalError, setTerminalSessionState, syncTerminalSize, workspaceRoot]);

  const restartSession = useCallback(async () => {
    const currentSessionId = useProjectStore.getState().terminalSessionId;

    if (currentSessionId !== null) {
      try {
        await invoke('terminal_kill', { sessionId: currentSessionId });
      } catch (error) {
        console.error('Unable to stop current terminal session:', error);
      }
    }

    requestTerminalReset();
  }, [requestTerminalReset]);

  useEffect(() => {
    const container = containerRef.current;
    if (!container) {
      return;
    }

    const terminal = new Terminal({
      allowTransparency: true,
      cursorBlink: true,
      cursorStyle: 'bar',
      convertEol: false,
      scrollback: 5000,
      fontSize: editorFontSize,
      lineHeight: 1.35,
      letterSpacing: 0.2,
      theme: getTerminalTheme(container),
      fontFamily: getComputedStyle(container).getPropertyValue('--editor-font-family').trim() || 'monospace',
    });

    const fitAddon = new FitAddon();
    terminal.loadAddon(fitAddon);
    terminal.open(container);

    terminalRef.current = terminal;
    fitAddonRef.current = fitAddon;
    writtenLengthRef.current = 0;

    terminal.onData((data) => {
      const currentSessionId = useProjectStore.getState().terminalSessionId;
      if (currentSessionId === null) {
        return;
      }

      void invoke('terminal_write', { sessionId: currentSessionId, data }).catch((error) => {
        console.error('Unable to write to terminal session:', error);
        useProjectStore.getState().setTerminalError(
          error instanceof Error ? error.message : String(error)
        );
      });
    });

    applyTerminalTheme();
    syncTerminalSize();

    const observer = new ResizeObserver(() => {
      window.requestAnimationFrame(syncTerminalSize);
    });

    observer.observe(container);
    resizeObserverRef.current = observer;

    return () => {
      observer.disconnect();
      resizeObserverRef.current = null;
      terminal.dispose();
      terminalRef.current = null;
      fitAddonRef.current = null;
    };
  }, [applyTerminalTheme, editorFontSize, syncTerminalSize]);

  useEffect(() => {
    applyTerminalTheme();
  }, [applyTerminalTheme, theme]);

  useEffect(() => {
    const terminal = terminalRef.current;
    if (!terminal) {
      return;
    }

    if (terminalSnapshot.length < writtenLengthRef.current) {
      terminal.reset();
      writtenLengthRef.current = 0;
    }

    const nextChunk = terminalSnapshot.slice(writtenLengthRef.current);
    if (nextChunk.length > 0) {
      terminal.write(nextChunk);
      writtenLengthRef.current = terminalSnapshot.length;
    }
  }, [terminalSnapshot]);

  useEffect(() => {
    if (!isDesktopShell) {
      return;
    }

    if (terminalResetNonce !== lastResetNonceRef.current) {
      lastResetNonceRef.current = terminalResetNonce;
    }

    if (terminalSessionId === null) {
      void createSession();
    }
  }, [createSession, isDesktopShell, terminalResetNonce, terminalSessionId]);

  if (!isDesktopShell) {
    return (
      <div className="flex h-full items-center justify-center px-6 py-8" {...uiMarker('terminal-panel-browser-fallback')}>
        <div className="max-w-lg rounded-[1.4rem] border px-5 py-4 text-sm leading-6" style={{ borderColor: 'var(--border)', color: 'var(--text-secondary)' }} {...uiMarker('terminal-panel-browser-fallback-message')}>
          The interactive shell is available in the Tauri desktop runtime. In browser preview, this panel keeps the layout and theme without spawning a local process.
        </div>
      </div>
    );
  }

  return (
    <div className="flex h-full min-h-0 flex-col" {...uiMarker('terminal-panel', { sessionState: terminalSessionState, sessionId: terminalSessionId ?? 'none' })}>
      <div
        className="flex shrink-0 items-center justify-between gap-3 border-b px-4 py-3"
        style={{ borderColor: 'var(--border)', background: 'color-mix(in srgb, var(--ide-toolbar) 78%, white 22%)' }}
        {...uiMarker('terminal-panel-header')}
      >
        <div className="flex min-w-0 items-center gap-3" {...uiMarker('terminal-panel-session')}>
          <div className="flex h-8 w-8 items-center justify-center rounded-2xl border" style={{ borderColor: 'var(--border)', background: 'color-mix(in srgb, var(--ide-panel) 70%, white 30%)', color: 'var(--text-secondary)' }}>
            <TerminalWindow size={15} />
          </div>
          <div className="min-w-0">
            <div className="flex items-center gap-2 text-sm" style={{ color: 'var(--text-primary)' }}>
              <span>Integrated shell</span>
              <span
                className={`terminal-status-pill ${
                  terminalSessionState === 'ready'
                    ? 'terminal-status-ready'
                    : terminalSessionState === 'starting'
                      ? 'terminal-status-starting'
                      : terminalSessionState === 'error'
                        ? 'terminal-status-error'
                        : 'terminal-status-idle'
                }`}
                {...uiMarker('terminal-panel-status-pill', { state: terminalSessionState })}
              >
                <Pulse size={10} weight="fill" />
                <span>{terminalSessionState}</span>
              </span>
            </div>
            <div className="truncate text-[11px]" style={{ color: 'var(--text-tertiary)' }}>
              {terminalSessionId === null ? 'Starting a new shell session...' : `Workspace: ${workspaceRoot}`}
            </div>
          </div>
        </div>

        <button
          type="button"
          onClick={() => void restartSession()}
          className="ide-button flex items-center gap-2 rounded-full px-3 py-2 text-[12px]"
          {...uiMarker('terminal-panel-restart')}
        >
          <ArrowClockwise size={13} />
          <span>Restart shell</span>
        </button>
      </div>

      {terminalError ? (
        <div
          className="flex items-center gap-2 border-b px-4 py-2 text-xs"
          style={{
            borderColor: 'color-mix(in srgb, var(--danger) 18%, var(--border))',
            background: 'color-mix(in srgb, var(--danger) 10%, white 90%)',
            color: 'color-mix(in srgb, var(--danger) 72%, black 28%)',
          }}
          {...uiMarker('terminal-panel-error')}
        >
          <WarningCircle size={13} weight="fill" />
          <span>{terminalError}</span>
        </div>
      ) : null}

      <div className="terminal-host min-h-0 flex-1 overflow-hidden" {...uiMarker('terminal-panel-host')}>
        <div ref={containerRef} className="h-full w-full" {...uiMarker('terminal-panel-xterm-container')} />
      </div>
    </div>
  );
}
