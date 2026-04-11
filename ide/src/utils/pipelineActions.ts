import { ExtensionConfig, FileEntry, useWorkspaceStore } from "../store/useWorkspaceStore";
import { PipelineAction, TerminalSessionMeta, useExecutionStore } from "../store/useExecutionStore";
import {
  buildBuildPlan,
  buildRunPlan,
  getDefaultRunTarget,
  getRuntimeForFile,
  wrapPipelineCommandForShell,
} from "./pipelineTargets";
import {
  RuntimeProfile,
  RuntimeProfileId,
  getRuntimeProfileForSettingsKey,
} from "./runtimeProfiles";
import { invoke } from "./tauri";

function runtimeProfileToExtension(profile: RuntimeProfile): ExtensionConfig {
  return {
    compilerPath: profile.compilerPlaceholder,
    lspPath: profile.lspPlaceholder,
    buildCommand: profile.buildPlaceholder,
    isEnabled: false,
  };
}

interface TerminalSessionInfo {
  sessionId: number;
  cwd: string;
  shell: string;
}

export function getActivePipelineFile(): FileEntry | null {
  const file = useWorkspaceStore.getState().activeFile;
  return file && !file.is_directory ? (file as FileEntry) : null;
}

export function getActivePipelineRuntimeId(): RuntimeProfileId {
  const file = getActivePipelineFile();
  return file ? getRuntimeForFile(file) : "generic";
}

export function getSelectedTargetForRuntime(runtimeId: RuntimeProfileId) {
  const { selectedTargets } = useExecutionStore.getState();
  return selectedTargets[runtimeId] || getDefaultRunTarget(runtimeId);
}

function getRuntimeExtensionConfig(runtimeId: RuntimeProfileId) {
  const workspace = useWorkspaceStore.getState();
  const profile = getRuntimeProfileForSettingsKey(runtimeId);

  return workspace.settings.extensions?.[runtimeId] || runtimeProfileToExtension(profile);
}

function getPreferredTerminalSession() {
  const execution = useExecutionStore.getState();
  return (
    execution.sessions.find((session) => session.id === execution.activeSessionId) ??
    execution.sessions[0] ??
    null
  );
}

async function ensurePipelineTerminalSession(preferredCwd: string): Promise<TerminalSessionMeta | null> {
  const existingSession = getPreferredTerminalSession();
  if (existingSession) {
    return existingSession;
  }

  try {
    const session = await invoke<TerminalSessionInfo>("terminal_create", {
      cwd: preferredCwd,
    });

    const execution = useExecutionStore.getState();
    execution.addSession(session.sessionId, session.shell, undefined, undefined, session.cwd);

    return useExecutionStore
      .getState()
      .sessions.find((entry) => entry.id === session.sessionId) ?? null;
  } catch (error) {
    console.error("Failed to create terminal session for pipeline action", error);
    return null;
  }
}

export async function executePipelineAction(action: PipelineAction) {
  const workspace = useWorkspaceStore.getState();
  const execution = useExecutionStore.getState();
  const file = getActivePipelineFile();

  if (!file || execution.activeTask) {
    return false;
  }

  const runtimeId = getRuntimeForFile(file);
  const targetId = getSelectedTargetForRuntime(runtimeId);
  const extensionConfig = getRuntimeExtensionConfig(runtimeId);

  const plan =
    action === "run"
      ? buildRunPlan({
          file,
          fileTree: workspace.fileTree,
          currentProjectRoot: workspace.currentProjectRoot,
          extensionConfig,
          runtimeId,
          targetId,
        })
      : buildBuildPlan({
          file,
          fileTree: workspace.fileTree,
          currentProjectRoot: workspace.currentProjectRoot,
          extensionConfig,
          runtimeId,
        });

  if (!plan) {
    return false;
  }

  workspace.setBottomPanelOpen(true);
  workspace.setBottomTab("console");

  const activeSession = await ensurePipelineTerminalSession(plan.workingDir || workspace.currentProjectRoot || ".");
  if (!activeSession) {
    execution.appendConsoleEntry({
      level: "warning",
      text: "No terminal session is available yet.",
    });
    return false;
  }

  const taskId = `${Date.now()}-${Math.random().toString(36).slice(2, 8)}`;
  const wrappedCommand = wrapPipelineCommandForShell({
    shell: activeSession.shell,
    taskId,
    command: plan.command,
    workingDir: plan.workingDir,
  });

  execution.startTask({
    taskId,
    sessionId: activeSession.id,
    action,
    runtimeId: plan.runtimeId,
    targetId: action === "run" ? targetId : plan.targetId,
    label: plan.label,
    command: plan.command,
  });

  if (activeSession.isReady) {
    try {
      await invoke("terminal_write", {
        sessionId: activeSession.id,
        data: wrappedCommand,
      });
      return true;
    } catch (error) {
      console.error("Failed to dispatch pipeline command", error);
      useExecutionStore.getState().clearActiveTask();
      return false;
    }
  }

  execution.appendConsoleEntry({
    taskId,
    level: "info",
    text: "Waiting for terminal session to become ready...",
  });
  execution.enqueueCommand(activeSession.id, wrappedCommand);
  return true;
}

export async function stopActivePipelineTask() {
  const execution = useExecutionStore.getState();
  const activeTask = execution.activeTask;

  if (!activeTask) {
    return false;
  }

  const activeSession = getPreferredTerminalSession();

  if (activeSession) {
    execution.clearPendingCommands(activeSession.id);
  }

  execution.appendConsoleEntry({
    taskId: activeTask.taskId,
    level: "warning",
    text: "Stop requested. Sending Ctrl+C to the active terminal task...",
  });
  execution.finishTask(activeTask.taskId, 130);

  if (activeSession?.isReady) {
    try {
      await invoke("terminal_write", {
        sessionId: activeSession.id,
        data: "\u0003",
      });
    } catch (error) {
      console.error("Failed to stop active task", error);
    }
  }

  return true;
}
