import { ThemeLab as ModularThemeLab } from "./theme-lab/ThemeLab";

interface ThemeLabProps {
  onOpenEditorSettings: () => void;
  fullScreen?: boolean;
}

export function ThemeLab(props: ThemeLabProps) {
  return <ModularThemeLab {...props} />;
}
