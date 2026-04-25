import os
import subprocess

common_files = []
for root, dirs, files in os.walk('compiler'):
    for f in files:
        if f.endswith('.c') and f not in ['main.c', 'zpm_main.c', 'lsp.c']:
            common_files.append(os.path.join(root, f))

def run_build(output, main_file, common):
    cmd = ['gcc', '-O0', '-Wall', '-Wextra', '-I.', '-o', output, main_file] + common
    print(f"Building {output} with: {' '.join(cmd)}")
    res = subprocess.run(cmd)
    return res.returncode

rc_zt = run_build('zt.exe', 'compiler/driver/main.c', common_files)
rc_zpm = run_build('zpm.exe', 'compiler/driver/zpm_main.c', common_files)

if rc_zt == 0 and rc_zpm == 0:
    print("SUCCESS")
    exit(0)
else:
    print(f"FAIL (zt: {rc_zt}, zpm: {rc_zpm})")
    exit(1)