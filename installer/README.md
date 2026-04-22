# Zenith Installers

This folder documents installer flows for Zenith.

## Windows (Inno Setup)

The Windows flow uses Inno Setup with admin elevation and environment setup.

## Prerequisite

- Inno Setup installed (`iscc` available in `PATH`)

## Files

- `installer/zenith.iss`: installer definition
- `tools/build_installer.ps1`: stages files and builds installer + checksums
- `tools/validate_installer_install.ps1`: validates installed binary with hello-world (`check/build/run`)

## Build Installer

From repository root:

```powershell
powershell -ExecutionPolicy Bypass -File tools\build_installer.ps1 -Version 0.3.0-alpha.2
```

Output:

- `dist/installer/zenith-<version>-windows-amd64-setup.exe`
- `dist/installer/zenith-<version>-windows-amd64-setup.checksums.txt`

## Validate Installed Package

After installation:

```powershell
powershell -ExecutionPolicy Bypass -File tools\validate_installer_install.ps1
```

Optional custom install path:

```powershell
powershell -ExecutionPolicy Bypass -File tools\validate_installer_install.ps1 -InstallRoot "C:\Program Files\Zenith"
```

## Linux (`.deb`, `.rpm`, Arch package)

The Linux flow uses `fpm` and generates:

- `.deb` (Ubuntu and Debian-based)
- `.rpm` (Fedora and RPM-based)
- `.pkg.tar.zst` (Arch-based)

Prerequisites:

- Linux host (or CI runner)
- `fpm` installed and available in `PATH`
- Linux `zt` binary already compiled (default expected path: `./zt`)

Build (from repository root):

```bash
python3 tools/build_linux_packages.py --version 0.3.0-alpha.2
```

Outputs:

- `dist/linux/zenith-<version>-linux-amd64.deb`
- `dist/linux/zenith-<version>-linux-x86_64.rpm`
- `dist/linux/zenith-<version>-linux-x86_64.pkg.tar.zst`
- `dist/linux/checksums.txt`

Global environment:

- The package installs `/etc/profile.d/zenith.sh`.
- It exports `ZENITH_HOME="/usr/local/lib/zenith"` globally for login shells.
