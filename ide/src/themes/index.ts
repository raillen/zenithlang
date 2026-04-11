export interface ZenithTheme {
  id: string;
  name: string;
  type: 'light' | 'dark';
  ui: {
    glass: boolean;       // Determines if the theme uses backdrop-blurs
    background: string;   // Main IDE background
    panel: string;        // Sidebar, Terminal bg
    border: string;       // All dividers
    textMain: string;     // Primary text
    textMuted: string;    // Dimmed text
    primary: string;      // Accents, active tabs
  };
  syntax: {
    keyword: string;
    string: string;
    function: string;
    number: string;
    type: string;
    comment: string;
  };
}

export const defaultLight: ZenithTheme = {
  id: 'light-prism',
  name: 'Standard Light',
  type: 'light',
  ui: {
    glass: true,
    background: '#ffffff',
    panel: 'rgba(236, 238, 238, 0.72)', // Xcode-like
    border: 'rgba(0, 0, 0, 0.08)',
    textMain: '#1d1d1f',
    textMuted: '#86868b',
    primary: '#0071E3',
  },
  syntax: {
    keyword: '#D7008F', // Xcode pink
    string: '#DF0000',  // Xcode red
    function: '#326D74', // Xcode teal
    number: '#1C00CF',  // Xcode blue
    type: '#4B21B0',    // Xcode purple
    comment: '#707F8C', // Xcode gray
  }
};

export const defaultDark: ZenithTheme = {
  id: 'dark-modern',
  name: 'Modern Dark',
  type: 'dark',
  ui: {
    glass: false,
    background: '#18181b', // zinc-900
    panel: '#27272a',      // zinc-800
    border: '#3f3f46',     // zinc-700
    textMain: '#fafafa',   // zinc-50
    textMuted: '#a1a1aa',  // zinc-400
    primary: '#3b82f6',    // blue-500
  },
  syntax: {
    keyword: '#ff7b72', // GitHub dark pink
    string: '#a5d6ff',  // GitHub dark light blue
    function: '#d2a8ff', // GitHub dark purple
    number: '#79c0ff',  // GitHub dark blue
    type: '#ff7b72',    
    comment: '#8b949e', 
  }
};

export const THEMES: Record<string, ZenithTheme> = {
  'light-prism': defaultLight,
  'dark-modern': defaultDark,
};
