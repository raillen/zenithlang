import { create } from 'zustand';
import React from 'react';

export interface Command {
    id: string;
    title: string;
    category?: string;
    icon?: React.ReactNode;
    action: () => void;
}

interface CommandState {
    commands: Record<string, Command>;
    isPaletteOpen: boolean;
    initialQuery: string;
    isSettingsOpen: boolean;
    
    registerCommand: (command: Command) => void;
    unregisterCommand: (id: string) => void;
    executeCommand: (id: string) => void;
    setPaletteOpen: (open: boolean, initialQuery?: string) => void;
    setSettingsOpen: (open: boolean) => void;
}

export const useCommandStore = create<CommandState>((set, get) => ({
    commands: {},
    isPaletteOpen: false,
    initialQuery: "",
    isSettingsOpen: false,
    
    registerCommand: (command) => set((state) => ({
        commands: { ...state.commands, [command.id]: command }
    })),
    
    unregisterCommand: (id) => set((state) => {
        const newCmds = { ...state.commands };
        delete newCmds[id];
        return { commands: newCmds };
    }),

    executeCommand: (id) => {
        const cmd = get().commands[id];
        if (cmd) {
            // Close palette if executing a command
            set({ isPaletteOpen: false });
            cmd.action();
        } else {
            console.warn(`Command '${id}' not found or not registered in current context.`);
        }
    },

    setPaletteOpen: (open, initialQuery = "") => set({ isPaletteOpen: open, initialQuery }),
    setSettingsOpen: (open) => set({ isSettingsOpen: open })
}));
