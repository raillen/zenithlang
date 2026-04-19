import os, subprocess

c_files = [os.path.join(r, f) for r, d, files in os.walk('compiler') for f in files if f.endswith('.c') and 'lsp' not in f]
cmd = ['gcc', '-Wall', '-Wno-unused-function', '-I', '.', '-I', 'runtime/c', '-o', 'zt.exe'] + c_files

print(f"Compiling {len(c_files)} files...")
result = subprocess.run(cmd, capture_output=True, text=True)

with open('build_errors.txt', 'w', encoding='utf-8') as f:
    f.write(result.stderr)

# Count error/warning lines
errors = [l for l in result.stderr.split('\n') if ': error:' in l or 'undefined reference' in l or 'multiple definition' in l]
print(f"Error lines: {len(errors)}")
for e in errors[:20]:
    print(f"  {e.strip()[:150]}")

print(f"\nExit code: {result.returncode}")
if result.returncode == 0 and os.path.exists('zt.exe'):
    print(f"SUCCESS! zt.exe size: {os.path.getsize('zt.exe')}")
