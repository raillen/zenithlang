import { expect, test } from "@playwright/test";

async function openBottomPanel(page: import("@playwright/test").Page) {
  const terminalTab = page.getByTestId("bottom-tab-terminal");
  if (await terminalTab.count()) {
    return;
  }

  await page.getByTestId("toolbar-toggle-bottom-panel").click();
  await expect(terminalTab).toBeVisible();
}

test.describe("Zenith Keter smoke", () => {
  test.beforeEach(async ({ page }) => {
    await page.goto("/");
    await expect(page.getByTestId("workbench-root")).toBeVisible();
  });

  test("loads the workbench shell", async ({ page }) => {
    await expect(page.getByTestId("activity-navigator")).toBeVisible();
    await expect(page.getByTestId("toolbar-settings")).toBeVisible();
  });

  test("shows the terminal panel chrome", async ({ page }) => {
    await openBottomPanel(page);
    await page.getByTestId("bottom-tab-terminal").click();

    await expect(page.getByTestId("terminal-panel")).toBeVisible();
    await expect(page.getByTestId("terminal-new-group")).toBeVisible();
    await expect(page.getByTestId("terminal-split")).toBeVisible();
    await expect(page.locator(".terminal-surface").first()).toBeVisible();
  });

  test("focuses terminal input when the surface is clicked", async ({ page }) => {
    await openBottomPanel(page);
    await page.getByTestId("bottom-tab-terminal").click();
    await page.locator(".terminal-surface").first().click();

    await expect(page.locator(".xterm-helper-textarea").first()).toBeFocused();
  });

  test("opens settings from the toolbar", async ({ page }) => {
    await page.getByTestId("toolbar-settings").click();
    await expect(page.getByTestId("settings-dialog")).toBeVisible();
    await page.getByTestId("settings-close").click();
    await expect(page.getByTestId("settings-dialog")).toBeHidden();
  });

  test("starts create-file and create-folder flows in the explorer", async ({ page }) => {
    await expect(page.getByTestId("file-navigator")).toBeVisible();

    await page.getByTestId("explorer-new-file").click();
    const newFileInput = page.getByLabel("New file name");
    await expect(newFileInput).toBeVisible();
    await newFileInput.fill("scratch.zt");
    await newFileInput.press("Enter");
    await expect(page.getByRole("tab", { name: /scratch\.zt/i })).toBeVisible();

    await page.getByTestId("explorer-new-folder").click();
    await expect(page.getByLabel("New folder name")).toBeVisible();
  });

  test("runs and stops from the toolbar after opening a file", async ({ page }) => {
    await expect(page.getByTestId("file-navigator")).toBeVisible();
    await page.getByText("ztc.lua").click();
    await expect(page.getByRole("tab", { name: /ztc\.lua/i })).toBeVisible();

    const runStopButton = page.getByTestId("toolbar-run-stop");
    await runStopButton.click();
    await expect(page.getByTestId("toolbar-status")).toContainText("Running...");
    await expect(runStopButton).toHaveAttribute("title", "Stop Active Task");

    await runStopButton.click();
    await expect(page.getByTestId("toolbar-status")).toContainText("Stopped");
  });

  test("switches editor layout controls", async ({ page }) => {
    await expect(page.getByTestId("focus-secondary")).toHaveCount(0);
    await page.getByTestId("layout-vertical").click();
    await expect(page.getByTestId("focus-secondary")).toBeVisible();

    await page.getByTestId("layout-horizontal").click();
    await expect(page.getByTestId("focus-secondary")).toBeVisible();

    await page.getByTestId("layout-single").click();
    await expect(page.getByTestId("focus-secondary")).toHaveCount(0);
  });
});
