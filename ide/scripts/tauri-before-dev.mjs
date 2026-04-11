import { spawn } from "node:child_process";
import { setTimeout as delay } from "node:timers/promises";

const DEV_PORT = 1420;
const DEV_URLS = [
  `http://localhost:${DEV_PORT}`,
  `http://127.0.0.1:${DEV_PORT}`,
];
const STARTUP_TIMEOUT_MS = 45000;
const CHECK_INTERVAL_MS = 750;

let child = null;
let shuttingDown = false;

async function readDevServerState() {
  for (const url of DEV_URLS) {
    try {
      const response = await fetch(url, { signal: AbortSignal.timeout(1200) });
      const body = await response.text();
      return {
        reachable: true,
        isZenith: body.includes("Zenith Keter") || body.includes("/src/main.tsx"),
        status: response.status,
        url,
      };
    } catch {
      // Try the next loopback URL.
    }
  }

  return {
    reachable: false,
    isZenith: false,
    status: 0,
    url: DEV_URLS[0],
  };
}

async function waitUntilReady() {
  const deadline = Date.now() + STARTUP_TIMEOUT_MS;

  while (Date.now() < deadline) {
    const state = await readDevServerState();
    if (state.reachable && state.isZenith) {
      return state;
    }

    if (child?.exitCode !== null && child?.exitCode !== undefined) {
      throw new Error(`The Vite dev server exited early with code ${child.exitCode}.`);
    }

    await delay(CHECK_INTERVAL_MS);
  }

  return null;
}

function waitForever() {
  return new Promise(() => {
    setInterval(() => {
      // Keep the process alive while Tauri owns this lifecycle.
    }, 60000);
  });
}

function shutdown(exitCode = 0) {
  if (shuttingDown) return;
  shuttingDown = true;

  if (child && child.exitCode === null) {
    child.kill("SIGTERM");
  }

  setTimeout(() => process.exit(exitCode), 250);
}

process.on("SIGINT", () => shutdown(0));
process.on("SIGTERM", () => shutdown(0));

const existingServer = await readDevServerState();
if (existingServer.reachable) {
  if (!existingServer.isZenith) {
    throw new Error(`Port ${DEV_PORT} is already serving another application. Stop it or change src-tauri/tauri.conf.json.`);
  }

  console.log(`[tauri-before-dev] Reusing existing Zenith dev server at ${existingServer.url}.`);
  await waitForever();
}

child = spawn("npm run dev", {
  stdio: "inherit",
  shell: true,
});

child.on("exit", (code) => {
  if (!shuttingDown) {
    process.exit(code ?? 0);
  }
});

const readyState = await waitUntilReady();
if (!readyState) {
  shutdown(1);
  throw new Error(`Timed out waiting for the Zenith dev server on port ${DEV_PORT}.`);
}

console.log(`[tauri-before-dev] Dev server ready at ${readyState.url}.`);
await waitForever();