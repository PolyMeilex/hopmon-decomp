#!/usr/bin/env python3
"""Verify that Hopmon.exe and Hopmon.pdb match the expected SHA256 hashes"""
import argparse
import hashlib
import os
import sys

EXE_SHA256 = "6f4acb933e1692d97cc18819204c605287f72a71b9f62e1210ff8908be85930b"
PDB_SHA256 = "21a2e9972ea50ce5e421b4f9b248ccf4f182224aa5dffcaecd8b68e274b49884"

def sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        while chunk := f.read(65536):
            h.update(chunk)
    return h.hexdigest()


def check_file(path, expected, name):
    if not os.path.exists(path):
        print(f"error: {name} not found: {path}", file=sys.stderr)
        return False
    actual = sha256(path)
    if actual != expected:
        print(f"error: {name} is not the expected version", file=sys.stderr)
        return False
    return True


def main():
    ap = argparse.ArgumentParser(description="Verify original Hopmon.exe and Hopmon.pdb SHA256 hashes")
    ap.add_argument("--exe", default="../Hopmon.exe", help="Path to Hopmon.exe")
    ap.add_argument("--pdb", default="../Hopmon.pdb", help="Path to Hopmon.pdb")
    args = ap.parse_args()

    ok = True
    if not check_file(args.exe, EXE_SHA256, os.path.basename(args.exe) or "Hopmon.exe"):
        ok = False
    if not check_file(args.pdb, PDB_SHA256, os.path.basename(args.pdb) or "Hopmon.pdb"):
        ok = False

    if not ok:
        sys.exit(1)

    print(f"{args.exe}: OK")
    print(f"{args.pdb}: OK")

if __name__ == "__main__":
    main()
