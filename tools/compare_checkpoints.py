#!/usr/bin/env python3
# SPDX-License-Identifier: MS-PL
"""Compare complete smoke evidence, including failures hidden by a known-issue demo status.

Run once per renderer. JSON supplies IDs/statuses; raw logs supply assertion multiplicities,
including standalone output prefixed with host:, joiner:, etc. A nonzero exit requests review,
not automatic acceptance of either the old or new behavior. No baseline files are modified.
"""
import argparse
from collections import Counter
import json
from pathlib import Path
import sys


def read_results(path):
    document = json.loads(path.read_text())
    if not isinstance(document, dict) or document.get("schema") != "cna-demos-results/1":
        raise ValueError(f"{path}: unsupported results schema")
    rows = document["results"]
    results = {}
    for row in rows:
        identifier = row["id"]
        if not isinstance(identifier, str) or not identifier or identifier in results:
            raise ValueError(f"{path}: empty or duplicate demo ID: {identifier!r}")
        if row["status"] not in {"PASSED", "FAILED", "SKIPPED", "NOT RUN"}:
            raise ValueError(f"{path}: invalid status for {identifier}")
        results[identifier] = row
    if not results:
        raise ValueError(f"{path}: empty results")
    return results


def failed_assertions(path):
    # Do not anchor at the start: standalone child prefixes otherwise disappear silently.
    marker = "CHECK FAILED:"
    return Counter(line.split(marker, 1)[1].strip() for line in path.read_text().splitlines()
                   if marker in line)


def compare(baseline, current, before, after):
    review = False
    for identifier in sorted(baseline.keys() - current.keys()):
        print(f"REMOVED ID: {identifier}")
        review = True
    for identifier in sorted(current.keys() - baseline.keys()):
        status = current[identifier]["status"]
        print(f"NEW ID: {identifier}: {status}")
        review = review or status == "FAILED"
    for identifier in sorted(baseline.keys() & current.keys()):
        old, new = baseline[identifier], current[identifier]
        for field in ("status", "knownIssue"):
            if old.get(field, "") != new.get(field, ""):
                print(f"CHANGED {field}: {identifier}: {old.get(field, '')} -> {new.get(field, '')}")
                review = True
        if old.get("checks") != new.get("checks"):
            print(f"CHECK COUNT: {identifier}: {old.get('checks')} -> {new.get('checks')}")
    # Removed assertions also need review: fewer failures could be a fix, or a check that no
    # longer executes. Whole-demo WILL_FAIL cannot distinguish those cases.
    for label, difference in (("ADDED", after - before), ("REMOVED", before - after)):
        for message, count in sorted(difference.items()):
            print(f"{label} FAILED ASSERTION ({count}x): {message}")
            review = True
    print(f"Compared {len(baseline)} -> {len(current)} IDs; "
          f"failed assertions {sum(before.values())} -> {sum(after.values())}.")
    print("Review required; retain both evidence sets." if review else
          "Existing statuses and failed-assertion messages/multiplicities are unchanged.")
    return 1 if review else 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("baseline", type=Path, help="Archived baseline results JSON")
    parser.add_argument("current", type=Path, help="New complete results JSON")
    parser.add_argument("--baseline-log", type=Path, required=True)
    parser.add_argument("--current-log", type=Path, required=True)
    parser.add_argument("--catalog", type=Path, help="Current --catalog-json output; require exact ID coverage")
    args = parser.parse_args()
    try:
        baseline, current = read_results(args.baseline), read_results(args.current)
        if args.catalog:
            demos = json.loads(args.catalog.read_text())["demos"]
            ids = [row["id"] for row in demos]
            if len(ids) != len(set(ids)) or set(ids) != current.keys():
                raise ValueError("Current report does not match every catalog ID exactly once")
        return compare(baseline, current, failed_assertions(args.baseline_log),
                       failed_assertions(args.current_log))
    except (OSError, ValueError, KeyError, TypeError) as error:
        print(f"Cannot compare checkpoint evidence: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
