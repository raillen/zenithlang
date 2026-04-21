#!/usr/bin/env bash
set -euo pipefail

cc_bin="${CC:-}"
if [[ -z "$cc_bin" ]]; then
    if command -v gcc >/dev/null 2>&1; then
        cc_bin="gcc"
    elif command -v clang >/dev/null 2>&1; then
        cc_bin="clang"
    else
        echo "FAIL"
        echo "No C compiler found. Install gcc or clang, or set CC."
        exit 1
    fi
fi

mapfile -t c_files < <(find compiler -type f -name '*.c' ! -name 'lsp.c' | sort)
if [[ ${#c_files[@]} -eq 0 ]]; then
    echo "FAIL"
    echo "No C files found under compiler/."
    exit 1
fi

cmd=("$cc_bin" -O0 -Wall -Wextra -I. -o zt.exe "${c_files[@]}")
echo "Building with: ${cmd[*]}"

"${cmd[@]}"
rc=$?
echo "Exit code: $rc"
if [[ $rc -eq 0 ]]; then
    echo "SUCCESS"
else
    echo "FAIL"
fi
exit "$rc"
