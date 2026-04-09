import { useEffect, useRef } from 'react';
import { Terminal } from 'xterm';
import { FitAddon } from 'xterm-addon-fit';
import { listen, UnlistenFn } from '@tauri-apps/api/event';
import { invoke } from '../utils/tauri';
import 'xterm/css/xterm.css';

interface TerminalDataPayload {
  sessionId: number;
  data: string;
}

export function TerminalPanel() {
  const terminalRef = useRef<HTMLDivElement>(null);
  const xtermRef = useRef<Terminal|null>(null);
  const sessionIdRef = useRef<number|null>(null);

  useEffect(() => {
    if (!terminalRef.current) return;

    // Initialize xterm
    const term = new Terminal({
      cursorBlink: true,
      fontSize: 12,
      fontFamily: "JetBrains Mono, Menlo, Monaco, 'Courier New', monospace",
      theme: {
        background: 'transparent',
        foreground: '#3f3f46', // zinc-600
        cursor: '#18181b',
        selectionBackground: 'rgba(0, 0, 0, 0.1)',
      },
      allowProposedApi: true,
    });

    const fitAddon = new FitAddon();
    term.loadAddon(fitAddon);
    term.open(terminalRef.current);
    fitAddon.fit();
    xtermRef.current = term;

    let unlistenData: UnlistenFn | null = null;

    async function initTerminal() {
      try {
        // Create actual PTY session
        const session = await invoke<{ sessionId: number }>('terminal_create', {
          rows: term.rows,
          cols: term.cols
        });
        
        sessionIdRef.current = session.sessionId;

        // Listen to data from Rust
        unlistenData = await listen<TerminalDataPayload>('zenith://terminal-data', (event) => {
          if (event.payload.sessionId === sessionIdRef.current) {
            term.write(event.payload.data);
          }
        });

        // Handle user input
        term.onData(async (data) => {
          if (sessionIdRef.current !== null) {
            await invoke('terminal_write', {
              sessionId: sessionIdRef.current,
              data: data
            });
          }
        });

        // Handle resize handler
        const resizeHandler = () => {
          fitAddon.fit();
          if (sessionIdRef.current !== null) {
            invoke('terminal_resize', {
              sessionId: sessionIdRef.current,
              rows: term.rows,
              cols: term.cols
            });
          }
        };

        window.addEventListener('resize', resizeHandler);
        term.writeln("\x1b[1;34mZenith Console v0.2 READY\x1b[0m");

        return () => {
            window.removeEventListener('resize', resizeHandler);
        };
      } catch (err) {
        term.writeln(`\x1b[1;31mError initializing terminal: ${err}\x1b[0m`);
      }
    }

    const cleanupPromise = initTerminal();

    return () => {
      if (unlistenData) unlistenData();
      cleanupPromise.then(cleanup => cleanup && cleanup());
      term.dispose();
    };
  }, []);

  return (
    <div className="flex-1 w-full h-full bg-black/[0.02] p-2 overflow-hidden">
      <div ref={terminalRef} className="w-full h-full" />
    </div>
  );
}
