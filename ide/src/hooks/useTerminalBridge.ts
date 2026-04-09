import { useEffect } from 'react';
import { listen, type UnlistenFn } from '@tauri-apps/api/event';
import { useProjectStore } from '../store/useProjectStore';
import {
  TERMINAL_DATA_EVENT,
  TERMINAL_EXIT_EVENT,
  type TerminalDataEvent,
  type TerminalExitEvent,
} from '../utils/terminal';
import { isTauriRuntime } from '../utils/tauri';

export function useTerminalBridge() {
  useEffect(() => {
    if (!isTauriRuntime()) {
      return;
    }

    let active = true;
    const cleanups: UnlistenFn[] = [];

    void (async () => {
      const unlistenData = await listen<TerminalDataEvent>(TERMINAL_DATA_EVENT, ({ payload }) => {
        const state = useProjectStore.getState();
        if (state.terminalSessionId !== payload.sessionId) {
          return;
        }

        state.appendTerminalSnapshot(payload.data);
        if (state.terminalSessionState === 'starting') {
          state.setTerminalSessionState('ready');
        }
      });

      if (!active) {
        unlistenData();
        return;
      }

      cleanups.push(unlistenData);

      const unlistenExit = await listen<TerminalExitEvent>(TERMINAL_EXIT_EVENT, ({ payload }) => {
        const state = useProjectStore.getState();
        if (state.terminalSessionId !== payload.sessionId) {
          return;
        }

        state.appendTerminalSnapshot(
          `\r\n\r\n[session finished${payload.exitCode === null ? '' : ` with code ${payload.exitCode}`}]\r\n`
        );
        state.setTerminalSession(null, 'exited');
      });

      if (!active) {
        unlistenExit();
        return;
      }

      cleanups.push(unlistenExit);
    })();

    return () => {
      active = false;
      cleanups.forEach((cleanup) => cleanup());
    };
  }, []);
}
