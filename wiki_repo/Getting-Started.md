# Getting Started

Requirements:

- Python 3
- GCC or Clang in PATH
- PowerShell, bash, or equivalent shell

Build the compiler:

```bash
python build.py
# or
bash build.sh
# or (Windows)
build.bat
```

Run help:

```bash
./zt.exe
```

First checks:

```bash
./zt.exe check my_app/zenith.ztproj
./zt.exe build my_app/zenith.ztproj
./zt.exe run my_app/zenith.ztproj
```

Reference:

- https://github.com/raillen/zenithlang/blob/master/README.md