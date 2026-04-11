import { useEffect, useRef } from "react";
import { Terminal } from "xterm";
import { FitAddon } from "xterm-addon-fit";
import { invoke } from "../utils/tauri";
import { getTerminalTheme } from "../utils/themeEngine";
import "xterm/css/xterm.css";

interface TerminalInstanceProps {
  session: {
    id: number;
    shell: string;
    cwd?: string;
  };
  output: string;
  active: boolean;
  focused?: boolean;
  theme: any;
  onMount: (term: Terminal, fit: FitAddon) => void;
}

export function TerminalInstance({ 
  session, 
  output,
  active, 
  focused = false,
  theme, 
  onMount 
}: TerminalInstanceProps) {
  const containerRef = useRef<HTMLDivElement>(null);
  const terminalRef = useRef<Terminal | null>(null);
  const fitAddonRef = useRef<FitAddon | null>(null);
  const resizeObserverRef = useRef<ResizeObserver | null>(null);
  const renderedLengthRef = useRef(0);

  useEffect(() => {
    if (!containerRef.current || terminalRef.current) return;

    const term = new Terminal({
      cursorBlink: true,
      fontSize: 12,
      lineHeight: 1.2,
      fontFamily: "JetBrains Mono, 'Fira Code', Menlo, Monaco, 'Courier New', monospace",
      theme: getTerminalTheme(theme),
      allowProposedApi: true,
      scrollback: 5000,
      drawBoldTextInBrightColors: true,
    });

    const fitAddon = new FitAddon();
    term.loadAddon(fitAddon);
    term.open(containerRef.current);
    
    terminalRef.current = term;
    fitAddonRef.current = fitAddon;

    const disposableOnData = term.onData((data) => {
      void invoke("terminal_write", {
        sessionId: session.id,
        data,
      });
    });

    // Resize Synchronization
    const handleResizeSync = () => {
      if (!fitAddonRef.current || !terminalRef.current) return;
      
      fitAddonRef.current.fit();
      const { cols, rows } = terminalRef.current;
      
      void invoke("terminal_resize", {
        sessionId: session.id,
        cols,
        rows
      });
    };

    // Use ResizeObserver for accurate container tracking
    const observer = new ResizeObserver(() => {
        requestAnimationFrame(() => {
            if (active) handleResizeSync();
        });
    });
    
    observer.observe(containerRef.current);
    resizeObserverRef.current = observer;

    onMount(term, fitAddon);

    // Initial fit
    setTimeout(handleResizeSync, 100);

    return () => {
      disposableOnData.dispose();
      observer.disconnect();
      term.dispose();
      terminalRef.current = null;
      renderedLengthRef.current = 0;
    };
  }, []);

  // Update theme dynamically
  useEffect(() => {
    if (terminalRef.current) {
        terminalRef.current.options.theme = getTerminalTheme(theme);
    }
  }, [theme]);

  // Refit when switched back to active or focused
  useEffect(() => {
    if (active && fitAddonRef.current && terminalRef.current) {
        setTimeout(() => {
            fitAddonRef.current?.fit();
            const { cols, rows } = terminalRef.current!;
            void invoke("terminal_resize", {
              sessionId: session.id,
              cols,
              rows
            });
            if (focused) terminalRef.current?.focus();
        }, 50);
    }
  }, [active, focused]);

  useEffect(() => {
    const term = terminalRef.current;
    if (!term) return;

    const safeOutput = output || "";

    if (renderedLengthRef.current > safeOutput.length) {
      renderedLengthRef.current = 0;
      term.reset();
    }

    if (safeOutput.length === renderedLengthRef.current) return;

    const nextChunk = safeOutput.slice(renderedLengthRef.current);
    renderedLengthRef.current = safeOutput.length;

    if (nextChunk) {
      term.write(nextChunk);
    }
  }, [output]);

  return (
    <div 
      ref={containerRef}
      onMouseDown={() => {
        terminalRef.current?.focus();
      }}
      className={`terminal-surface h-full w-full overflow-hidden bg-ide-bg transition-colors ${
        focused 
          ? "ring-1 ring-inset ring-primary/20" 
          : "hover:bg-ide-panel/35"
      } ${active ? "opacity-100" : "opacity-0 pointer-events-none"}`}
    />
  );
}
