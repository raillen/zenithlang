# E2E Tests

Firefox is the default and recommended runner:

```powershell
npm run test:e2e:firefox
```

Watch the browser navigation live:

```powershell
npm run test:e2e:firefox:headed
```

Use the Playwright interactive runner:

```powershell
npm run test:e2e:firefox:ui
```

Install the Firefox runtime used by Playwright:

```powershell
npm run test:e2e:install
```

General variants:

```powershell
npm run test:e2e
npm run test:e2e:headed
npm run test:e2e:ui
```

Optional Zen Browser support still exists if you ever need it:

```powershell
$env:ZEN_BROWSER_PATH="C:\Path\To\zen.exe"
npm run test:e2e:zen
```

Notes:

- Firefox is the default because it avoids the Chromium sandbox issues we were hitting locally.
- The `zen` Playwright project is only enabled when `ZEN_BROWSER_PATH` is set.
- The suite starts a Vite server on `http://127.0.0.1:1420` automatically unless `PLAYWRIGHT_BASE_URL` is already defined.
