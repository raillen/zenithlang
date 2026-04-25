import sys
from pathlib import Path

path = Path('runtime/c/zenith_rt_templates.h')
content = path.read_bytes()

# Clean up common macro corruption patterns
content = content.replace(b'\\\\`r`n', b'\\\n')
content = content.replace(b'\\`r`n', b'\\\n')
content = content.replace(b'\\\\r\\n', b'\\\n')

# Ensure all line continuations are followed by a newline (no trailing spaces)
lines = content.splitlines()
new_lines = []
for line in lines:
    stripped = line.rstrip()
    if stripped.endswith(b'\\'):
        new_lines.append(stripped)
    else:
        new_lines.append(line)

path.write_bytes(b'\n'.join(new_lines) + b'\n')
print("zenith_rt_templates.h cleaned")
