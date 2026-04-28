import type { ConsoleLine, PreviewEvent, PreviewStatus } from "../types";

export function normalizePreviewStatus(status: string): PreviewStatus {
  const knownStatuses: PreviewStatus[] = [
    "idle",
    "starting",
    "loading",
    "ready",
    "playing",
    "paused",
    "stopped",
    "exited",
    "error",
    "unavailable",
  ];
  return knownStatuses.includes(status as PreviewStatus) ? (status as PreviewStatus) : "error";
}

export function previewStatusTone(status: PreviewStatus): "neutral" | "good" | "warn" {
  if (status === "ready" || status === "playing") return "good";
  if (["paused", "error", "unavailable", "exited"].includes(status)) return "warn";
  return "neutral";
}

export function previewStatusLabel(status: PreviewStatus): string {
  const labels: Record<PreviewStatus, string> = {
    idle: "Preview idle",
    starting: "Preview starting",
    loading: "Preview loading",
    ready: "Preview ready",
    playing: "Preview playing",
    paused: "Preview paused",
    stopped: "Preview stopped",
    exited: "Preview exited",
    error: "Preview error",
    unavailable: "Preview unavailable",
  };
  return labels[status];
}

export function previewEventToConsoleLine(event: PreviewEvent, runner?: string): Omit<ConsoleLine, "id"> {
  let message = event.raw;

  if (event.kind === "hello" && runner) {
    message = `Preview connected through ${runner}`;
  } else if (event.status) {
    message = `Preview status: ${event.status}`;
    const counts = previewEventCounts(event);
    if (counts) message += ` (${counts})`;
  } else if (event.message) {
    message = event.message;
    const counts = previewEventCounts(event);
    if (counts) message += ` (${counts})`;
  } else if (event.kind) {
    message = `Preview event: ${event.kind}`;
  }

  return {
    level: event.channel === "error" || event.raw.startsWith("[stderr]") ? "error" : "info",
    source: "preview",
    message,
  };
}

export function previewEventCounts(event: PreviewEvent): string {
  const parts: string[] = [];
  if (typeof event.entityCount === "number") parts.push(`${event.entityCount} entities`);
  if (typeof event.cameraCount === "number") parts.push(`${event.cameraCount} cameras`);
  if (typeof event.lightCount === "number") parts.push(`${event.lightCount} lights`);
  if (typeof event.audioCount === "number") parts.push(`${event.audioCount} audio`);
  return parts.join(", ");
}
