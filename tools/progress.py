#!/usr/bin/env python3
"""Reports decompilation progress"""
import argparse
import csv
import subprocess
import sys
from collections import defaultdict

GRAY = "\033[90m"
RESET = "\033[0m"


def gray(text):
    return f"{GRAY}{text}{RESET}"


def print_row(name, m, is_gray=False):
    row = f"{name:<16} {m[0]:>5} / {m[1]:<5} {m[2]:>7} / {m[3]:<7}"
    print(gray(row) if is_gray else row)


def defined_functions(obj):
    out = subprocess.run(["llvm-nm", "--defined-only", obj], check=True, capture_output=True, text=True).stdout
    for line in out.splitlines():
        parts = line.split(maxsplit=2)
        if parts[1] != "T": continue

        yield parts[2]
        if parts[2].startswith("??_G"):
            yield "??_E" + parts[2][4:]
        elif parts[2].startswith("??_E"):
            yield "??_G" + parts[2][4:]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--functions", default="vendor/functions.csv")
    ap.add_argument("objs", nargs="*")
    args = ap.parse_args()

    with open(args.functions) as f:
        by_name = {r["decorated"]: r for r in csv.DictReader(f)}
    game = [r for r in by_name.values() if r["game"] == "1"]

    done, unknown = set(), []
    for obj in args.objs:
        for sym in defined_functions(obj):
            if sym in by_name: done.add(sym)
            else: unknown.append((obj, sym))

    for obj, sym in unknown:
        demangled = subprocess.run(["llvm-undname", sym], capture_output=True, text=True).stdout.strip().splitlines()[1]
        print(f"warning: {obj}, fn is not in Hopmon.pdb:\n{demangled}\n", file=sys.stderr)

    per_mod = defaultdict(lambda: [0, 0, 0, 0]) # done fns, total fns, done bytes, total bytes
    for r in game:
        m = per_mod[r["module"]]
        size = int(r["size"])
        m[1] += 1
        m[3] += size
        if r["decorated"] in done:
            m[0] += 1
            m[2] += size

    total = [sum(m[i] for m in per_mod.values()) for i in range(4)]

    print(f"{'module':<16} {'functions':>12} {'bytes':>16}")
    for mod in sorted(per_mod):
        m = per_mod[mod]
        if m[0]:
            print_row(mod, m, m[0] == m[1])

    print_row("TOTAL", total)


if __name__ == "__main__":
    main()
