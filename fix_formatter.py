import re

with open('compiler/tooling/formatter.c', 'r') as f:
    content = f.read()

# Replace block endings and beginnings
content = content.replace('sb_append(sb, " {\\n");', 'sb_append(sb, "\\n");')
content = content.replace('sb_append(sb, "{\\n");', 'sb_append(sb, "\\n");')
content = content.replace('sb_indent(sb);\n            sb_append(sb, "}");', 'sb_indent(sb);\n            sb_append(sb, "end");')

with open('compiler/tooling/formatter.c', 'w') as f:
    f.write(content)
