import os
import subprocess

c_files = []
for root, dirs, files in os.walk('compiler'):
    for f in files:
        if f.endswith('.c') and f != 'lsp.c':
            c_files.append(os.path.join(root, f))

cmd = ['gcc', '-O0', '-Wall', '-Wextra', '-I.', '-o', 'zt.exe'] + c_files
print("Building with:", " ".join(cmd))
res = subprocess.run(cmd)
print("Exit code:", res.returncode)
if res.returncode == 0:
    print("SUCCESS")
else:
    print("FAIL")