import sys, json

try:
    with open(sys.argv[1], encoding="utf-8") as f:
        players = json.load(f)["players"]          # or a plain list, adjust to your file
    rows = []
    for p in players:
        fields = [str(p["playerId"]), str(p["name"]),
                  str(int(p["squadNumber"])), str(float(p["price"]))]
        if any("\t" in x or "\n" in x for x in fields):
            raise ValueError("tab or newline in a field")
        rows.append("\t".join(fields))
except Exception as e:
    print(e, file=sys.stderr)
    sys.exit(1)                                    # non-zero = C knows it failed

print("\n".join(rows))