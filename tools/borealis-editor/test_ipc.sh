#!/usr/bin/env bash
# Test script for Borealis Editor IPC

echo "Building borealis-editor..."
cargo build --manifest-path tools/borealis-editor/Cargo.toml

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo "Build successful!"
echo ""
echo "To run the editor:"
echo "  cargo run --manifest-path tools/borealis-editor/Cargo.toml"
echo ""
echo "Note: The preview will try to spawn './zt.exe run packages/borealis/zenith.ztproj'"
echo "Make sure zt.exe is available in the current directory or set ZENITH_HOME"
