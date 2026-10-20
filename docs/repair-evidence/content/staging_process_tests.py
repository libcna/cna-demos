#!/usr/bin/env python3
"""Exercise the actual staging header across independent native/Win32 processes.

The only retained housekeeping file is an exact empty, singly linked regular
.cna-staging.lock. It is removed only after every private fixture child has exited.
No production staging directory is scanned or cleaned by this driver.
"""
import argparse
import json
import os
from pathlib import Path
import re
import select
import stat
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("executable")
    parser.add_argument("--wine", action="store_true")
    args = parser.parse_args()
    executable = str(Path(args.executable).resolve())
    temporary = Path(tempfile.mkdtemp(prefix="cna-staging-process-"))
    root = temporary / "real"
    root.mkdir()
    alias = temporary / "alias"
    alias.symlink_to(root, target_is_directory=True)
    children, evidence, checks, errors, terminated, limitations = [], [], [], [], [], []

    def command(mode, parent=root):
        location = str(parent)
        if args.wine:
            location = "Z:" + location.replace("/", "\\")
        return (["wine"] if args.wine else []) + [executable, mode, location]

    def start(mode, parent=root):
        child = subprocess.Popen(command(mode, parent), stdin=subprocess.PIPE,
                                 stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        children.append(child)
        if not select.select([child.stdout], [], [], 30)[0]:
            raise RuntimeError("Fixture did not announce readiness: " + mode)
        line = child.stdout.readline()
        if line.strip() != "READY":
            raise RuntimeError("Unexpected readiness: " + mode + " " + repr(line))
        return child

    def finish(child, label, text=None):
        out, err = child.communicate(input=text, timeout=60)
        evidence.append(dict(label=label, exit=child.returncode, stdout=out, stderr=err))
        if child.returncode != 0 or err:
            raise RuntimeError("Fixture failed: " + label)
        return out

    def run(mode, label, parent=root, expected=0):
        child = subprocess.Popen(command(mode, parent), stdin=subprocess.DEVNULL,
                                 stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        children.append(child)
        out, err = child.communicate(timeout=60)
        evidence.append(dict(label=label, exit=child.returncode, stdout=out, stderr=err))
        if child.returncode != expected or (expected == 0 and err):
            raise RuntimeError("Unexpected fixture result: " + label)
        return out

    def result(out):
        match = re.search(r"^owners=(\d+) scans=(\d+) errors=(\d+) diagnostics=(\d+) removed=(\d+) active=(\d+) inspected=(\d+)$", out, re.M)
        if not match:
            raise RuntimeError("Missing numeric fixture result")
        return dict(zip(("owners", "scans", "errors", "diagnostics", "removed", "active", "inspected"), map(int, match.groups())))

    def require(condition, label):
        if not condition:
            raise RuntimeError(label)
        checks.append(label)

    try:
        holder = start("parent")
        require(run("try", "separate_process_exclusion", alias).strip() == "locked=1",
                "separate_process_and_alias_excluded")
        finish(holder, "parent_exit_without_destructor", "ABANDON\n")
        require(run("try", "parent_after_abrupt_exit").strip() == "locked=0",
                "parent_lock_released_by_os")

        scanner = start("scan")
        owners = [start("owners", root), start("owners", alias)]
        for child in [scanner] + owners:
            child.stdin.write("START\n")
            child.stdin.flush()
        for index, child in enumerate(owners):
            row = result(finish(child, "concurrent_owner_" + str(index)))
            require(row["owners"] == 1000 and row["errors"] == 0 and
                    row["diagnostics"] == 0 and row["removed"] == 0,
                    "owner_" + str(index) + "_1000_clean_lifecycles")
        row = result(finish(scanner, "concurrent_scavenger", "STOP\n"))
        require(row["scans"] > 0 and row["errors"] == 0 and row["diagnostics"] == 0 and
                row["removed"] == 0, "concurrent_scavenger_no_false_diagnostics_or_removals")

        active = start("old-owner")
        row = result(run("inspect", "active_old_owner"))
        require(row["active"] == 1 and row["removed"] == 0 and row["diagnostics"] == 0,
                "old_active_owner_retained_without_serializing_its_build")
        finish(active, "old_owner_exit_without_destructor", "ABANDON\n")
        row = result(run("inspect", "abandoned_old_owner"))
        require(row["active"] == 0 and row["removed"] == 1 and row["diagnostics"] == 0,
                "old_abandoned_owner_safely_collected")

        marker = root / ".cna-staging.lock"
        before = marker.stat()
        run("once", "reuse_parent_identity")
        require(marker.stat().st_ino == before.st_ino, "parent_coordination_identity_never_replaced")
        marker.write_bytes(b"unsafe")
        row = result(run("inspect", "nonempty_parent_marker", expected=1))
        require(row["inspected"] == 0 and row["diagnostics"] == 1 and row["removed"] == 0,
                "unsafe_parent_marker_prevents_scavenging")
        run("once", "nonempty_marker_prevents_owner", expected=2)
        require(marker.read_bytes() == b"unsafe", "unsafe_parent_marker_not_overwritten")
        marker.write_bytes(b"")
        marker.unlink()  # All fixture users exited; construct an explicitly unsafe fixture.
        keep = temporary / "keep"
        keep.write_bytes(b"")
        link = subprocess.run(command("make-symlink"), capture_output=True, text=True, timeout=60)
        evidence.append(dict(label="native_symlink_creation", exit=link.returncode,
                             stdout=link.stdout, stderr=link.stderr))
        if link.returncode == 77 and link.stdout.startswith("unsupported=") and not link.stderr:
            limitations.append("native_symlink_unavailable: " + link.stdout.strip())
            require(not marker.exists() and not marker.is_symlink(), "unavailable_symlink_left_no_marker")
        else:
            require(link.returncode == 0 and link.stdout.strip() == "native_symlink=1" and not link.stderr,
                    "native_symlink_created_and_recognized")
            run("inspect", "empty_target_symlink_parent_marker", expected=1)
            run("once", "empty_target_symlink_prevents_owner", expected=2)
            require(keep.read_bytes() == b"", "empty_symlink_target_not_modified")
            marker.unlink()
        os.link(keep, marker)
        run("inspect", "hardlinked_parent_marker", expected=1)
        run("once", "hardlink_marker_prevents_owner", expected=2)
        require(marker.stat().st_nlink == 2 and keep.read_bytes() == b"", "hardlinked_marker_untouched")
        marker.unlink()
        keep.unlink()
        run("once", "final_owner")
        info = marker.lstat()
        require(sorted(p.name for p in root.iterdir()) == [".cna-staging.lock"] and
                stat.S_ISREG(info.st_mode) and info.st_size == 0 and info.st_nlink == 1 and
                info.st_uid == os.geteuid(), "exact_empty_regular_parent_marker_only")
        require(all(child.poll() is not None for child in children), "all_private_children_exited")
        marker.unlink()  # Quiescent private fixture only; production never unlinks this identity.
        root.rmdir()
        alias.unlink()
        temporary.rmdir()
    except Exception as error:
        errors.append(str(error))
    finally:
        for child in children:
            if child.poll() is None:
                terminated.append(child.pid)
                child.kill()  # Only a driver-created fixture after its own timeout/failure.
                child.communicate(timeout=30)
        print(json.dumps(dict(checks=checks, evidence=evidence, errors=errors,
                              terminatedFixturePids=terminated, temporary=str(temporary),
                              cleaned=not temporary.exists(), limitations=limitations), indent=2))
    return 1 if errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
