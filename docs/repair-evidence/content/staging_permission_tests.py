#!/usr/bin/env python3
"""Unprivileged Linux refusal controls for the actual staging lifecycle fixture."""
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile


def main():
    if len(sys.argv) != 2:
        return 2
    capabilities = Path("/proc/self/status").read_text()
    if os.geteuid() == 0 or not re.search(r"^CapEff:\s*0000000000000000$", capabilities, re.M):
        raise RuntimeError("Permission controls require a non-root UID with no effective capabilities")
    executable = str(Path(sys.argv[1]).resolve())
    root = Path(tempfile.mkdtemp(prefix="cna-staging-permissions-"))
    marker = root / ".cna-staging.lock"
    checks, evidence, errors = [], [], []

    def run(mode, expected, label):
        process = subprocess.run([executable, mode, str(root)], capture_output=True, text=True, timeout=30)
        evidence.append(dict(label=label, exit=process.returncode, stdout=process.stdout, stderr=process.stderr))
        if process.returncode != expected:
            raise RuntimeError(label)
        if mode == "inspect" and expected == 1 and not re.search(
                r"diagnostics=1 removed=0 active=0 inspected=0", process.stdout):
            raise RuntimeError("Refused parent must not be scanned: " + label)
        checks.append(label)

    try:
        root.chmod(0o500)
        run("inspect", 1, "readonly_parent_without_marker_refuses_scan")
        run("once", 2, "readonly_parent_without_marker_refuses_reservation")
        if list(root.iterdir()):
            raise RuntimeError("Readonly parent was mutated")
        root.chmod(0o700)
        run("once", 0, "writable_parent_recovers")
        identity = marker.stat().st_ino
        marker.chmod(0)
        run("inspect", 1, "unreadable_marker_refuses_scan")
        run("once", 2, "unreadable_marker_refuses_reservation")
        marker.chmod(0o600)
        root.chmod(0o500)
        run("once", 2, "readonly_parent_with_marker_refuses_reservation")
        root.chmod(0o700)
        run("once", 0, "permissions_restored_without_replacing_identity")
        if marker.stat().st_ino != identity or marker.stat().st_size != 0 or marker.stat().st_nlink != 1:
            raise RuntimeError("Coordination marker identity or shape changed")
        if sorted(p.name for p in root.iterdir()) != [".cna-staging.lock"]:
            raise RuntimeError("Unexpected leftover entries")
        marker.unlink()  # All child processes exited; private quiescent fixture only.
        root.rmdir()
    except Exception as error:
        errors.append(str(error))
    finally:
        if root.exists():
            root.chmod(0o700)
            if marker.exists() and not marker.is_symlink():
                marker.chmod(0o600)
        print(json.dumps(dict(checks=checks, evidence=evidence, errors=errors,
                              uid=os.geteuid(), temporary=str(root), cleaned=not root.exists()), indent=2))
    return 1 if errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
