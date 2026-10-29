import sys, json

for line in sys.stdin:
    try:
        m = json.loads(line)
        out = f"{m['cmd'].upper()} {m.get('id','')} {m.get('to','')}"
    except Exception:
        out = "ERR bad json"
    print(out, flush=True)