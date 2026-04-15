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

// --- STANDARD THEMES ---

export const defaultLight: ZenithTheme = {
  id: 'light-prism',
  name: 'Standard Light',
  type: 'light',
  ui: {
    glass: true,
    background: '#ffffff',
    panel: 'rgba(236, 238, 238, 0.72)',
    border: 'rgba(0, 0, 0, 0.08)',
    textMain: '#1d1d1f',
    textMuted: '#86868b',
    primary: '#0071E3',
  },
  syntax: {
    keyword: '#D7008F',
    string: '#DF0000',
    function: '#326D74',
    number: '#1C00CF',
    type: '#4B21B0',
    comment: '#707F8C',
  }
};

export const defaultDark: ZenithTheme = {
  id: 'dark-modern',
  name: 'Modern Dark',
  type: 'dark',
  ui: {
    glass: false,
    background: '#18181b',
    panel: '#27272a',
    border: '#3f3f46',
    textMain: '#fafafa',
    textMuted: '#a1a1aa',
    primary: '#3b82f6',
  },
  syntax: {
    keyword: '#ff7b72',
    string: '#a5d6ff',
    function: '#d2a8ff',
    number: '#79c0ff',
    type: '#ff7b72',
    comment: '#8b949e',
  }
};

// --- NORD ---

export const nordDark: ZenithTheme = {
  id: 'nord-dark',
  name: 'Nord Night',
  type: 'dark',
  ui: {
    glass: true,
    background: '#2E3440',
    panel: '#3B4252',
    border: '#434C5E',
    textMain: '#D8DEE9',
    textMuted: '#4C566A',
    primary: '#88C0D0',
  },
  syntax: {
    keyword: '#81A1C1',
    string: '#A3BE8C',
    function: '#88C0D0',
    number: '#B48EAD',
    type: '#8FBCBB',
    comment: '#616E88',
  }
};

export const nordLight: ZenithTheme = {
  id: 'nord-light',
  name: 'Nord Snow',
  type: 'light',
  ui: {
    glass: true,
    background: '#ECEFF4',
    panel: '#E5E9F0',
    border: '#D8DEE9',
    textMain: '#2E3440',
    textMuted: '#4C566A',
    primary: '#5E81AC',
  },
  syntax: {
    keyword: '#5E81AC',
    string: '#A3BE8C',
    function: '#81A1C1',
    number: '#B48EAD',
    type: '#8FBCBB',
    comment: '#96A3B2',
  }
};

// --- ROSÉ PINE ---

export const rosePineDark: ZenithTheme = {
  id: 'rose-pine-dark',
  name: 'Rosé Pine',
  type: 'dark',
  ui: {
    glass: true,
    background: '#191724',
    panel: '#1f1d2e',
    border: '#26233a',
    textMain: '#e0def4',
    textMuted: '#908caa',
    primary: '#ebbcba',
  },
  syntax: {
    keyword: '#31748f',
    string: '#f6c177',
    function: '#9ccfd8',
    number: '#ebbcba',
    type: '#c4a7e7',
    comment: '#6e6a86',
  }
};

export const rosePineDawn: ZenithTheme = {
  id: 'rose-pine-light',
  name: 'Rosé Pine Dawn',
  type: 'light',
  ui: {
    glass: true,
    background: '#faf4ed',
    panel: '#fffaf3',
    border: '#f2e9e1',
    textMain: '#575279',
    textMuted: '#797593',
    primary: '#d7827e',
  },
  syntax: {
    keyword: '#286983',
    string: '#ea9d34',
    function: '#56949f',
    number: '#d7827e',
    type: '#907aa9',
    comment: '#9893a5',
  }
};

// --- MONOKAI / SUBLIME ---

export const monokai: ZenithTheme = {
  id: 'sublime-monokai',
  name: 'Sublime Monokai',
  type: 'dark',
  ui: {
    glass: false,
    background: '#272822',
    panel: '#1e1f1c',
    border: '#3e3d32',
    textMain: '#f8f8f2',
    textMuted: '#75715e',
    primary: '#f92672',
  },
  syntax: {
    keyword: '#f92672',
    string: '#e6db74',
    function: '#a6e22e',
    number: '#ae81ff',
    type: '#66d9ef',
    comment: '#75715e',
  }
};

export const mariana: ZenithTheme = {
  id: 'sublime-mariana',
  name: 'Sublime Mariana',
  type: 'dark',
  ui: {
    glass: true,
    background: '#343d46',
    panel: '#303841',
    border: '#4f5b66',
    textMain: '#d8dee9',
    textMuted: '#65737e',
    primary: '#5fb3b3',
  },
  syntax: {
    keyword: '#c594c5',
    string: '#99c794',
    function: '#6699cc',
    number: '#f99157',
    type: '#5fb3b3',
    comment: '#65737e',
  }
};

// --- GITHUB ---

export const githubDark: ZenithTheme = {
  id: 'github-dark',
  name: 'GitHub Dark',
  type: 'dark',
  ui: {
    glass: false,
    background: '#0d1117',
    panel: '#161b22',
    border: '#30363d',
    textMain: '#c9d1d9',
    textMuted: '#8b949e',
    primary: '#58a6ff',
  },
  syntax: {
    keyword: '#ff7b72',
    string: '#a5d6ff',
    function: '#d2a8ff',
    number: '#79c0ff',
    type: '#ffa657',
    comment: '#8b949e',
  }
};

export const githubLight: ZenithTheme = {
  id: 'github-light',
  name: 'GitHub Light',
  type: 'light',
  ui: {
    glass: false,
    background: '#ffffff',
    panel: '#f6f8fa',
    border: '#d0d7de',
    textMain: '#24292f',
    textMuted: '#57606a',
    primary: '#0969da',
  },
  syntax: {
    keyword: '#cf222e',
    string: '#0a3069',
    function: '#8250df',
    number: '#0550ae',
    type: '#953800',
    comment: '#6e7781',
  }
};

// --- JETBRAINS DARCULA ---

export const darcula: ZenithTheme = {
  id: 'jetbrains-darcula',
  name: 'JetBrains Darcula',
  type: 'dark',
  ui: {
    glass: false,
    background: '#2b2b2b',
    panel: '#3c3f41',
    border: '#323232',
    textMain: '#a9b7c6',
    textMuted: '#808080',
    primary: '#3b78e7',
  },
  syntax: {
    keyword: '#cc7832',
    string: '#6a8759',
    function: '#ffc66d',
    number: '#6897bb',
    type: '#a9b7c6',
    comment: '#808080',
  }
};

export const intellijLight: ZenithTheme = {
  id: 'jetbrains-light',
  name: 'IntelliJ Light',
  type: 'light',
  ui: {
    glass: false,
    background: '#ffffff',
    panel: '#f2f2f2',
    border: '#d1d1d1',
    textMain: '#000000',
    textMuted: '#808080',
    primary: '#3b78e7',
  },
  syntax: {
    keyword: '#0033b3',
    string: '#067d17',
    function: '#00627a',
    number: '#1750eb',
    type: '#000000',
    comment: '#8c8c8c',
  }
};

// --- NEON CITY / SYNTHWAVE ---

export const synthNight: ZenithTheme = {
  id: 'synth-night',
  name: 'Neon City Night',
  type: 'dark',
  ui: {
    glass: true,
    background: '#0d0d17',
    panel: 'rgba(23, 14, 43, 0.8)',
    border: '#39215e',
    textMain: '#ffffff',
    textMuted: '#a277ff',
    primary: '#ff00ff',
  },
  syntax: {
    keyword: '#ff00ff',
    string: '#ff853b',
    function: '#00ccff',
    number: '#ffe600',
    type: '#00ffcc',
    comment: '#624b80',
  }
};

export const vaporDay: ZenithTheme = {
  id: 'vapor-day',
  name: 'Neon City Day',
  type: 'light',
  ui: {
    glass: true,
    background: '#fef1f8',
    panel: 'rgba(255, 255, 255, 0.7)',
    border: '#ffd0eb',
    textMain: '#4b1d3f',
    textMuted: '#8c597d',
    primary: '#ff49db',
  },
  syntax: {
    keyword: '#ff49db',
    string: '#4bc0c0',
    function: '#764ba2',
    number: '#feca57',
    type: '#4834d4',
    comment: '#b8a1b2',
  }
};

// --- NO HAPPINESS HERE ---

export const despairDark: ZenithTheme = {
  id: 'despair-dark',
  name: 'No Happiness (Dark)',
  type: 'dark',
  ui: {
    glass: false,
    background: '#0a0a0a',
    panel: '#121212',
    border: '#1a1a1a',
    textMain: '#888888',
    textMuted: '#444444',
    primary: '#333333',
  },
  syntax: {
    keyword: '#666666',
    string: '#555555',
    function: '#777777',
    number: '#444444',
    type: '#666666',
    comment: '#222222',
  }
};

export const despairLight: ZenithTheme = {
  id: 'despair-light',
  name: 'No Happiness (Light)',
  type: 'light',
  ui: {
    glass: false,
    background: '#dfdfdf',
    panel: '#d0d0d0',
    border: '#c0c0c0',
    textMain: '#444444',
    textMuted: '#888888',
    primary: '#666666',
  },
  syntax: {
    keyword: '#222222',
    string: '#333333',
    function: '#111111',
    number: '#444444',
    type: '#222222',
    comment: '#999999',
  }
};

// --- CATPPUCCIN ---

export const catFrappe: ZenithTheme = {
  id: 'cat-frappe',
  name: 'Catppuccin Frappé',
  type: 'dark',
  ui: {
    glass: true,
    background: '#303446',
    panel: '#292c3c',
    border: '#414559',
    textMain: '#c6d0f5',
    textMuted: '#838ba7',
    primary: '#8caaee',
  },
  syntax: {
    keyword: '#e78284',
    string: '#a6d189',
    function: '#8caaee',
    number: '#ef9f76',
    type: '#e5c890',
    comment: '#737994',
  }
};

export const catLatte: ZenithTheme = {
  id: 'cat-latte',
  name: 'Catppuccin Latte',
  type: 'light',
  ui: {
    glass: true,
    background: '#eff1f5',
    panel: '#e6e9ef',
    border: '#ccd0da',
    textMain: '#4c4f69',
    textMuted: '#8c8fa1',
    primary: '#1e66f5',
  },
  syntax: {
    keyword: '#d20f39',
    string: '#40a02b',
    function: '#1e66f5',
    number: '#fe640b',
    type: '#df8e1d',
    comment: '#9ca0b0',
  }
};

export const THEMES: Record<string, ZenithTheme> = {
  'light-prism': defaultLight,
  'dark-modern': defaultDark,
  'nord-dark': nordDark,
  'nord-light': nordLight,
  'rose-pine-dark': rosePineDark,
  'rose-pine-light': rosePineDawn,
  'sublime-monokai': monokai,
  'sublime-mariana': mariana,
  'github-dark': githubDark,
  'github-light': githubLight,
  'jetbrains-darcula': darcula,
  'jetbrains-light': intellijLight,
  'synth-night': synthNight,
  'vapor-day': vaporDay,
  'despair-dark': despairDark,
  'despair-light': despairLight,
  'cat-frappe': catFrappe,
  'cat-latte': catLatte
};

export function resolveThemeById(id: string, customs?: Record<string, ZenithTheme>): ZenithTheme {
  return customs?.[id] || THEMES[id] || defaultDark;
}
