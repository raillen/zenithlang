import { defineConfig, devices } from "@playwright/test";

const port = Number(process.env.ZENITH_TEST_PORT || 1420);
const baseURL = process.env.PLAYWRIGHT_BASE_URL || `http://127.0.0.1:${port}`;
const zenBrowserPath = process.env.ZEN_BROWSER_PATH;

const projects = [
  {
    name: "firefox",
    use: {
      ...devices["Desktop Firefox"],
      browserName: "firefox" as const,
      launchOptions: {
        firefoxUserPrefs: {
          "gfx.webrender.all": false,
          "gfx.webrender.software": true,
          "layers.acceleration.disabled": true,
        },
      },
    },
  },
];

if (zenBrowserPath) {
  projects.push({
    name: "zen",
    use: {
      ...devices["Desktop Firefox"],
      browserName: "firefox" as const,
      launchOptions: {
        executablePath: zenBrowserPath,
      },
    },
  });
}

export default defineConfig({
  testDir: "./tests/e2e",
  fullyParallel: false,
  forbidOnly: !!process.env.CI,
  retries: process.env.CI ? 2 : 0,
  workers: 1,
  reporter: process.env.CI ? [["github"], ["html", { open: "never" }]] : [["list"], ["html", { open: "never" }]],
  timeout: 30_000,
  expect: {
    timeout: 10_000,
  },
  use: {
    baseURL,
    trace: "retain-on-failure",
    screenshot: "only-on-failure",
    video: "retain-on-failure",
  },
  webServer: process.env.PLAYWRIGHT_BASE_URL
    ? undefined
    : {
        command: `npm run dev -- --host 127.0.0.1 --port ${port} --strictPort`,
        url: baseURL,
        reuseExistingServer: true,
        timeout: 60_000,
      },
  projects,
});
