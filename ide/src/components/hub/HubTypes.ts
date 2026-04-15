import { ReactNode } from "react";
import { ZenithHubSectionId, ZenithHubSection } from "../../content/zenithHubContent";

export interface HubSectionProps {
  activeSection: ZenithHubSection;
  setActiveSectionId: (id: ZenithHubSectionId) => void;
  onClose: () => void;
  onOpenBrowserManual?: () => void;
}

export interface QuickAction {
  title: string;
  body: string;
  icon: ReactNode;
  onClick: () => void;
}
