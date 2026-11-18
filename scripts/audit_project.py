import os
import re

print("=== AUDITING FRONTEND API ENDPOINT USAGE ===")
fetch_pattern = re.compile(r'fetch\([\'\"`](/api[^?\'\"`\s]+)')
method_pattern = re.compile(r'method:\s*[\'\"`]([A-Z]+)[\'\"`]')

api_calls = []

for root, dirs, files in os.walk('app'):
    # skip app/api because we want to see what pages/components call
    if 'api' in dirs:
        dirs.remove('api')
    for f in files:
        if f.endswith(('.tsx', '.ts')):
            p = os.path.join(root, f)
            with open(p, 'r', encoding='utf-8') as fh:
                content = fh.read()
                # find fetch calls
                for m in re.finditer(r'fetch\(\s*([`\'\"][^`\'\"]+[`\'\"])(\s*,\s*\{[^}]*\})?', content):
                    url_expr = m.group(1)
                    options = m.group(2) or ""
                    method = "GET"
                    meth_match = method_pattern.search(options)
                    if meth_match:
                        method = meth_match.group(1)
                    elif "POST" in options:
                        method = "POST"
                    elif "PATCH" in options:
                        method = "PATCH"
                    elif "DELETE" in options:
                        method = "DELETE"
                    api_calls.append((p, method, url_expr))

for p, method, url in sorted(set(api_calls)):
    print(f"[{method}] {url} in {p}")

print("\n=== AUDITING EXISTING NEXT.JS API HANDLERS ===")
for root, dirs, files in os.walk('app/api'):
    for f in files:
        if f.endswith(('.tsx', '.ts')):
            p = os.path.join(root, f)
            with open(p, 'r', encoding='utf-8') as fh:
                content = fh.read()
                methods = []
                for m in ["GET", "POST", "PATCH", "PUT", "DELETE"]:
                    if f"export async function {m}" in content:
                        methods.append(m)
                print(f"{p} handles: {', '.join(methods)}")
