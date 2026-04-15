import { type Dispatch, type SetStateAction } from "react";
import { type ZenithTheme } from "../../themes";

export type ThemePillar = "soul" | "anatomy" | "engine" | "gallery";

export interface ThemeLabModuleProps {
  theme: ZenithTheme;
  onChange: Dispatch<SetStateAction<ZenithTheme>>;
  onHover?: (region: string | undefined) => void;
}

export interface PillarOption {
  id: ThemePillar;
  label: string;
  description: string;
  icon: React.ReactNode;
}
