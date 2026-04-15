/**
 * Zenith Theme Utilities for Web
 * Syncs with IDE Sovereign Engine v0.3.0
 */

export const DEFAULT_THEME_DARK = {
  id: "custom-dark-web",
  name: "New Zenith Dark",
  type: "dark",
  meta: {
    author: "Zenith Designer",
    description: "Created via Zenith Theme Lab Web",
    version: "1.0.0"
  },
  ui: {
    glass: true,
    background: "#0A0A0A",
    panel: "rgba(20, 20, 23, 0.85)",
    border: "rgba(255, 255, 255, 0.08)",
    textMain: "#E0E0E0",
    textMuted: "#888888",
    primary: "#0F766E"
  },
  syntax: {
    keyword: "#CF222E",
    string: "#7EE787",
    function: "#D2A8FF",
    number: "#79C0FF",
    type: "#FFA657",
    comment: "#8B949E"
  }
};

export const DEFAULT_THEME_LIGHT = {
  id: "custom-light-web",
  name: "New Zenith Light",
  type: "light",
  meta: {
    author: "Zenith Designer",
    description: "Created via Zenith Theme Lab Web",
    version: "1.0.0"
  },
  ui: {
    glass: true,
    background: "#F5F7F7",
    panel: "rgba(232, 236, 236, 0.82)",
    border: "rgba(23, 32, 36, 0.1)",
    textMain: "#172024",
    textMuted: "#66747B",
    primary: "#0F8F83"
  },
  syntax: {
    keyword: "#B4245D",
    string: "#0B8F6A",
    function: "#0F6BB6",
    number: "#B86A00",
    type: "#6D59C9",
    comment: "#7A8A91"
  }
};

/**
 * Very basic hex to hsl simplified for harmony
 */
function hexToHsl(hex) {
    let r = parseInt(hex.slice(1, 3), 16) / 255;
    let g = parseInt(hex.slice(3, 5), 16) / 255;
    let b = parseInt(hex.slice(5, 7), 16) / 255;
    let max = Math.max(r, g, b), min = Math.min(r, g, b);
    let h, s, l = (max + min) / 2;
    if (max === min) { h = s = 0; }
    else {
        let d = max - min;
        s = l > 0.5 ? d / (2 - max - min) : d / (max + min);
        switch (max) {
            case r: h = (g - b) / d + (g < b ? 6 : 0); break;
            case g: h = (b - r) / d + 2; break;
            case b: h = (r - g) / d + 4; break;
        }
        h /= 6;
    }
    return [h * 360, s * 100, l * 100];
}

function hslToHex(h, s, l) {
    l /= 100;
    const a = s * Math.min(l, 1 - l) / 100;
    const f = n => {
        const k = (n + h / 30) % 12;
        const color = l - a * Math.max(Math.min(k - 3, 9 - k, 1), -1);
        return Math.round(255 * color).toString(16).padStart(2, '0');
    };
    return `#${f(0)}${f(8)}${f(4)}`;
}

/**
 * Derives a full theme from a primary color
 */
export function harmonizeTheme(primaryHex, type) {
    const isDark = type === 'dark';
    const [h, s, l] = hexToHsl(primaryHex);
    
    return {
        type,
        ui: {
            glass: true,
            primary: primaryHex,
            background: isDark ? hslToHex(h, 15, 6) : hslToHex(h, 10, 97),
            panel: isDark ? `rgba(${parseInt(primaryHex.slice(1,3), 16)}, ${parseInt(primaryHex.slice(3,5), 16)}, ${parseInt(primaryHex.slice(5,7), 16)}, 0.08)` : `rgba(${parseInt(primaryHex.slice(1,3), 16)}, ${parseInt(primaryHex.slice(3,5), 16)}, ${parseInt(primaryHex.slice(5,7), 16)}, 0.05)`,
            border: isDark ? "rgba(255, 255, 255, 0.08)" : "rgba(0, 0, 0, 0.08)",
            textMain: isDark ? "#E0E0E0" : "#172024",
            textMuted: isDark ? "#888888" : "#66747B",
        },
        syntax: {
            keyword: hslToHex((h + 180) % 360, s, isDark ? 65 : 45), // Complementary
            string: hslToHex((h + 90) % 360, s, isDark ? 70 : 40),   // Analogous
            function: hslToHex(h, Math.max(0, s - 20), isDark ? 75 : 35),
            number: hslToHex((h + 45) % 360, s, isDark ? 80 : 30),
            type: hslToHex((h + 220) % 360, s, isDark ? 70 : 45),
            comment: isDark ? "#6A737D" : "#7A8A91"
        }
    };
}

/**
 * Generates a full ZenithTheme object compatible with the IDE Hub
 */
export function buildFullTheme(quickTheme) {
  const isDark = quickTheme.type === "dark";
  
  // Basic token generation logic similar to IDE's index.ts
  return {
    ...quickTheme,
    tokens: {
      surface: {
        canvas: quickTheme.ui.background,
        panel: quickTheme.ui.panel,
        elevated: isDark ? "#1A1A1E" : "#FFFFFF",
        subtle: isDark ? "rgba(255,255,255,0.04)" : "rgba(0,0,0,0.03)",
        overlay: isDark ? "rgba(9,10,14,0.72)" : "rgba(255,255,255,0.78)",
      },
      border: {
        subtle: quickTheme.ui.border,
        strong: isDark ? "rgba(255,255,255,0.14)" : "rgba(0,0,0,0.16)",
        focus: quickTheme.ui.primary,
      },
      text: {
        base: quickTheme.ui.textMain,
        muted: quickTheme.ui.textMuted,
        soft: isDark ? "rgba(250,250,250,0.68)" : "rgba(29,29,31,0.68)",
        inverse: isDark ? "#111111" : "#ffffff",
      },
      accent: {
        base: quickTheme.ui.primary,
        soft: isDark ? "rgba(15,118,110,0.16)" : "rgba(15,118,110,0.12)",
        contrast: "#ffffff",
      },
      chrome: {
        toolbarBackground: isDark ? "rgba(30,30,35,0.92)" : "rgba(240,240,240,0.82)",
        sidebarBackground: isDark ? "rgba(25,25,28,0.96)" : "rgba(245,245,245,0.92)",
        inspectorBackground: isDark ? "rgba(20,20,24,0.95)" : "rgba(250,250,250,0.94)",
        statusBarBackground: quickTheme.ui.primary,
      },
      editor: {
        activeLineBackground: "rgba(255,255,255,0.05)",
        activeLineBorder: "rgba(255,255,255,0.1)",
        gutterActiveForeground: quickTheme.ui.textMain,
      }
    },
    terminal: {
        background: quickTheme.ui.background,
        foreground: quickTheme.ui.textMain,
        cursor: quickTheme.ui.primary,
        red: quickTheme.syntax.keyword,
        green: "#22c55e",
        blue: quickTheme.syntax.function,
        magenta: quickTheme.syntax.type,
        cyan: "#22d3ee",
        white: "#e5e7eb"
    }
  };
}

export function serializeTheme(theme) {
  return JSON.stringify(buildFullTheme(theme), null, 2);
}

export function downloadTheme(theme) {
  const data = serializeTheme(theme);
  const blob = new Blob([data], { type: 'application/json' });
  const url = URL.createObjectURL(blob);
  const link = document.createElement('a');
  link.href = url;
  link.download = `${theme.id || 'zenith-theme'}.json`;
  link.click();
  URL.revokeObjectURL(url);
}
