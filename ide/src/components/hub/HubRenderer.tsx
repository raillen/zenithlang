import { ZenithHubSectionId } from "../../content/zenithHubContent";
import { HubSectionProps } from "./HubTypes";
import { OverviewSection } from "./OverviewSection";
import { DocSection } from "./DocSection";
import { WorkbenchModule } from "./WorkbenchModule";
import { EditorModule } from "./EditorModule";
import { InterfaceModule } from "./InterfaceModule";
import { KeymapModule } from "./KeymapModule";
import { ExtensionsModule } from "./ExtensionsModule";
import { ManualModule } from "./ManualModule";
import { MarketplaceModule } from "./MarketplaceModule";
import { ThemeLabModule } from "./ThemeLabModule";

interface HubRendererProps extends HubSectionProps {
  activeSectionId: ZenithHubSectionId;
}

export function HubRenderer(props: HubRendererProps) {
  const { activeSectionId } = props;

  switch (activeSectionId) {
    case "overview":
      return <OverviewSection {...props} />;
    case "marketplace":
      return <MarketplaceModule />;
    case "manual":
      return <ManualModule />;
    case "workbench":
      return <WorkbenchModule />;
    case "editor-ui":
      return <EditorModule />;
    case "interface":
      return <InterfaceModule />;
    case "extensions":
      return <ExtensionsModule />;
    case "keymap":
      return <KeymapModule />;
    case "theme-lab":
      return <ThemeLabModule {...props} />;
    case "about":
      return <DocSection {...props} />;
    default:
      return <OverviewSection {...props} />;
  }
}
