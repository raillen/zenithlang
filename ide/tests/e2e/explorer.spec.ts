import { expect, test, type Page } from "@playwright/test";

function explorerItemTestId(path: string) {
  const normalized = path
    .replace(/\\/g, "/")
    .replace(/^\.(\/|$)/, "")
    .replace(/^\.\.\//, "")
    .replace(/[^a-zA-Z0-9]+/g, "-")
    .replace(/^-+|-+$/g, "")
    .toLowerCase();

  return `explorer-item-${normalized || "root"}`;
}

async function openBottomPanel(page: Page) {
  const terminalTab = page.getByTestId("bottom-tab-terminal");
  if (await terminalTab.count()) {
    return;
  }

  await page.getByTestId("toolbar-toggle-bottom-panel").click();
  await expect(terminalTab).toBeVisible();
}

async function createExplorerEntry(page: Page, kind: "file" | "folder", name: string) {
  const label = kind === "file" ? "New file name" : "New folder name";
  await page.getByTestId(kind === "file" ? "explorer-new-file" : "explorer-new-folder").click();
  const input = page.getByLabel(label);
  await expect(input).toBeVisible();
  await input.fill(name);
  await input.press("Enter");
  await expect(page.getByLabel(label)).toHaveCount(0);
}

async function ensureFolderExpanded(page: Page, path: string) {
  const folder = page.getByTestId(explorerItemTestId(path));
  await expect(folder).toBeVisible();
  if ((await folder.getAttribute("aria-expanded")) !== "true") {
    await folder.click();
  }
}

async function pasteIntoFolderFromContext(page: Page, folderTestId: string) {
  await page.getByTestId(folderTestId).click({ button: "right" });
  await page.getByRole("menuitem", { name: /Paste into folder/i }).click();
}

test.describe("Explorer CRUD", () => {
  test.beforeEach(async ({ page }) => {
    await page.goto("/");
    await expect(page.getByTestId("file-navigator")).toBeVisible();
  });

  test("persists file create, rename and delete in the explorer", async ({ page }) => {
    const initialName = "notes.zt";
    const renamedName = "notes-final.zt";
    const initialId = explorerItemTestId(`./${initialName}`);
    const renamedId = explorerItemTestId(`./${renamedName}`);

    await createExplorerEntry(page, "file", initialName);
    await expect(page.getByTestId(initialId)).toBeVisible();
    await expect(page.getByRole("tab", { name: new RegExp(initialName.replace(".", "\\."), "i") })).toBeVisible();

    await page.getByTestId(initialId).click();
    await page.keyboard.press("F2");
    const renameInput = page.getByLabel("Rename item");
    await expect(renameInput).toBeVisible();
    await renameInput.fill(renamedName);
    await renameInput.press("Enter");

    await expect(page.getByTestId(initialId)).toHaveCount(0);
    await expect(page.getByTestId(renamedId)).toBeVisible();
    await expect(page.getByRole("tab", { name: /notes-final\.zt/i })).toBeVisible();

    await page.evaluate(() => {
      window.confirm = () => true;
    });
    await page.getByTestId(renamedId).click({ button: "right" });
    await page.getByRole("menuitem", { name: /^Delete$/ }).click();

    await expect(page.getByTestId(renamedId)).toHaveCount(0);
    await expect(page.getByRole("tab", { name: /notes-final\.zt/i })).toHaveCount(0);
  });

  test("duplicates, copies and moves explorer items across folders", async ({ page }) => {
    const fileName = "clipboard-demo.zt";
    const duplicateName = "clipboard-demo copy.zt";
    const copiesFolder = "copies";
    const movedFolder = "moved";

    await createExplorerEntry(page, "file", fileName);
    await createExplorerEntry(page, "folder", copiesFolder);
    await createExplorerEntry(page, "folder", movedFolder);

    const sourceId = explorerItemTestId(`./${fileName}`);
    const duplicateId = explorerItemTestId(`./${duplicateName}`);
    const copiesFolderId = explorerItemTestId(`./${copiesFolder}`);
    const movedFolderId = explorerItemTestId(`./${movedFolder}`);
    const copiedIntoFolderId = explorerItemTestId(`./${copiesFolder}/${fileName}`);
    const movedIntoFolderId = explorerItemTestId(`./${movedFolder}/${fileName}`);

    await page.getByTestId(sourceId).click();
    await page.getByTestId("explorer-duplicate").click();
    await expect(page.getByTestId(duplicateId)).toBeVisible();

    await page.getByTestId(sourceId).click();
    await page.getByTestId("explorer-copy").click();
    await expect(page.getByTestId("explorer-clipboard-status")).toContainText("Copied 1 item");

    await pasteIntoFolderFromContext(page, copiesFolderId);
    await ensureFolderExpanded(page, `./${copiesFolder}`);
    await expect(page.getByTestId(copiedIntoFolderId)).toBeVisible();

    await page.getByTestId(sourceId).click();
    await page.getByTestId("explorer-cut").click();
    await expect(page.getByTestId("explorer-clipboard-status")).toContainText("Cut 1 item");

    await pasteIntoFolderFromContext(page, movedFolderId);
    await ensureFolderExpanded(page, `./${movedFolder}`);
    await expect(page.getByTestId(sourceId)).toHaveCount(0);
    await expect(page.getByTestId(movedIntoFolderId)).toBeVisible();
  });

  test("opens a folder terminal from the explorer", async ({ page }) => {
    const srcFolderId = explorerItemTestId("../src");

    await page.getByTestId(srcFolderId).click({ button: "right" });
    await page.getByRole("menuitem", { name: /Open terminal here/i }).click();

    await openBottomPanel(page);
    await page.getByTestId("bottom-tab-terminal").click();
    await expect(page.getByTestId("terminal-panel")).toBeVisible();
    await expect(page.getByTestId("terminal-panel").getByText("src", { exact: true })).toBeVisible();
  });
});
