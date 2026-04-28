# Selfhost Tools

Phase 6 starts dogfooding Zenith with small repository tools written in Zenith.

These tools are intentionally small. They are not replacements for Python gates yet.
Their job is to expose language and stdlib gaps while doing useful repository checks.

## Tools

- `link-checker`: checks that important internal doc/code paths exist.
- `manifest-validator`: checks the root `zenith.ztproj` shape.
- `fixture-index`: indexes behavior fixtures and writes a small generated report.

## Run

```powershell
.\zt.exe run tools\selfhost\link-checker\zenith.ztproj
.\zt.exe run tools\selfhost\manifest-validator\zenith.ztproj
.\zt.exe run tools\selfhost\fixture-index\zenith.ztproj
```

The generated fixture index is written to `.ztc-tmp/selfhost/fixture-index.txt`.

