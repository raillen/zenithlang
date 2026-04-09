export const TERMINAL_DATA_EVENT = 'zenith://terminal-data';
export const TERMINAL_EXIT_EVENT = 'zenith://terminal-exit';

export interface TerminalCreateResponse {
  sessionId: number;
  cwd: string;
  shell: string;
}

export interface TerminalDataEvent {
  sessionId: number;
  data: string;
}

export interface TerminalExitEvent {
  sessionId: number;
  exitCode: number | null;
}
