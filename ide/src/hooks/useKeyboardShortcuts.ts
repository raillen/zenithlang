import { useEffect, useRef } from 'react';
import { useWorkspaceStore } from '../store/useWorkspaceStore';

export function useKeyboardShortcuts() {
  const {
    toggleSidebar,
    toggleBottomPanel,
    toggleInspector,
    toggleZenMode,
    setFocusedPane,
  } = useWorkspaceStore();

  const lastKeyRef = useRef<string | null>(null);
  const chordTimeoutRef = useRef<number | null>(null);

  useEffect(() => {
    const handleKeyDown = (event: KeyboardEvent) => {
      const activeElement = document.activeElement;
      const isTypingContext =
        activeElement?.tagName === "INPUT" ||
        activeElement?.tagName === "TEXTAREA" ||
        (activeElement instanceof HTMLElement &&
          (activeElement.isContentEditable ||
            !!activeElement.closest(".monaco-editor") ||
            !!activeElement.closest(".xterm")));

      if (isTypingContext) {
        return;
      }

      const isCtrl = event.ctrlKey || event.metaKey;
      const isAlt = event.altKey;
      const key = event.key.toLowerCase();

      // Clear chord timeout if another key is pressed
      if (chordTimeoutRef.current) {
        window.clearTimeout(chordTimeoutRef.current);
      }

      // Handle Ctrl+K chords
      if (lastKeyRef.current === 'k' && isCtrl) {
        if (key === 'z') {
          event.preventDefault();
          toggleZenMode();
          lastKeyRef.current = null;
          return;
        }
      }

      // Sidebar (Ctrl+B)
      if (isCtrl && key === 'b') {
        event.preventDefault();
        toggleSidebar();
      }

      // Bottom Panel (Ctrl+J)
      if (isCtrl && key === 'j') {
        event.preventDefault();
        toggleBottomPanel();
      }

      // Inspector (Ctrl+Alt+I)
      if (isCtrl && isAlt && key === 'i') {
        event.preventDefault();
        toggleInspector();
      }

      // Focus Primary (Alt+1)
      if (isAlt && key === '1') {
        event.preventDefault();
        setFocusedPane('primary');
      }

      // Focus Secondary (Alt+2)
      if (isAlt && key === '2') {
        event.preventDefault();
        setFocusedPane('secondary');
      }

      // Menu Zenith (Alt+Z)
      if (isAlt && key === 'z') {
        event.preventDefault();
        // Dispatch custom event for the ZenithMenu to listen to
        window.dispatchEvent(new CustomEvent('zenith://toggle-menu'));
      }

      // Track last key for chords
      if (isCtrl && key === 'k') {
        lastKeyRef.current = 'k';
        chordTimeoutRef.current = window.setTimeout(() => {
          lastKeyRef.current = null;
        }, 1000);
      } else {
        lastKeyRef.current = null;
      }
    };

    window.addEventListener('keydown', handleKeyDown);
    return () => {
      window.removeEventListener('keydown', handleKeyDown);
      if (chordTimeoutRef.current) window.clearTimeout(chordTimeoutRef.current);
    };
  }, [toggleSidebar, toggleBottomPanel, toggleInspector, toggleZenMode, setFocusedPane]);
}
