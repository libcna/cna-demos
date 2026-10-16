#!/usr/bin/env python3
# SPDX-License-Identifier: MS-PL
"""Capture deterministic demos and compare them with approved renderer references.

Examples:

    python3 tools/visual_regression.py --exe build/linux-full/bin/cna-demos
    python3 tools/visual_regression.py --exe build/linux-full/bin/cna-demos \
        --renderer SOFTWARE --id graphics.device.clear

Run the command under Xvfb when a selected renderer needs a window. References live at
``references/<renderer>/<demo-id>.png``. Mismatches leave the actual image, a highlighted diff,
and the demo's stdout/stderr below the output directory.
"""

import argparse
from dataclasses import dataclass
import json
from pathlib import Path
import subprocess
import sys
from typing import Iterable

from PIL import Image, ImageChops


@dataclass(frozen=True)
class Comparison:
    passed: bool
    differing_pixels: int
    max_channel_difference: int
    message: str


def compare_images(reference_path: Path, actual_path: Path, tolerance: int,
                   max_differing_pixels: int, diff_path: Path | None = None) -> Comparison:
    """Compare two PNGs and optionally write a red-on-transparent mismatch mask."""
    with Image.open(reference_path) as reference_source, Image.open(actual_path) as actual_source:
        reference = reference_source.convert("RGBA")
        actual = actual_source.convert("RGBA")

    if reference.size != actual.size:
        return Comparison(False, 0, 0,
                          f"size differs: reference {reference.size}, actual {actual.size}")

    difference = ImageChops.difference(reference, actual)
    pixels = list(difference.getdata())
    max_channel_difference = max((max(pixel) for pixel in pixels), default=0)
    mismatches = [max(pixel) > tolerance for pixel in pixels]
    differing_pixels = sum(mismatches)

    if diff_path is not None and differing_pixels:
        diff_path.parent.mkdir(parents=True, exist_ok=True)
        mask = Image.new("RGBA", reference.size)
        mask.putdata([(255, 0, 0, 255) if mismatch else (0, 0, 0, 0)
                      for mismatch in mismatches])
        mask.save(diff_path)

    passed = differing_pixels <= max_differing_pixels
    message = (f"{differing_pixels} pixel(s) differ above tolerance {tolerance}; "
               f"maximum channel difference {max_channel_difference}")
    return Comparison(passed, differing_pixels, max_channel_difference, message)


def read_catalog(executable: Path) -> dict[str, dict]:
    result = subprocess.run([str(executable), "--catalog-json"], check=True,
                            capture_output=True, text=True)
    document = json.loads(result.stdout)
    if document.get("schema") != "cna-demos-catalog/1":
        raise ValueError(f"unsupported catalog schema: {document.get('schema')!r}")
    demos = document.get("demos", [])
    indexed = {demo["id"]: demo for demo in demos}
    if len(indexed) != len(demos):
        raise ValueError("catalog contains duplicate demo IDs")
    return indexed


def selected_references(reference_root: Path, renderers: Iterable[str],
                        demo_ids: Iterable[str]) -> list[tuple[str, str, Path]]:
    selected_renderers = list(renderers)
    selected_ids = list(demo_ids)
    if not selected_renderers:
        selected_renderers = sorted(path.name for path in reference_root.iterdir()
                                    if path.is_dir()) if reference_root.exists() else []

    references = []
    for renderer in selected_renderers:
        renderer_root = reference_root / renderer
        if selected_ids:
            references.extend((renderer, demo_id, renderer_root / f"{demo_id}.png")
                              for demo_id in selected_ids)
        elif renderer_root.exists():
            references.extend((renderer, path.stem, path)
                              for path in sorted(renderer_root.glob("*.png")))
    return references


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--exe", required=True, type=Path)
    parser.add_argument("--references", type=Path,
                        default=Path(__file__).resolve().parents[1] / "references")
    parser.add_argument("--output", type=Path, default=Path("visual-regression-output"))
    parser.add_argument("--renderer", action="append", default=[])
    parser.add_argument("--id", action="append", default=[])
    parser.add_argument("--tolerance", type=int, default=0,
                        help="maximum ignored difference per channel (0-255)")
    parser.add_argument("--max-differing-pixels", type=int, default=0)
    args = parser.parse_args()

    if not 0 <= args.tolerance <= 255:
        parser.error("--tolerance must be between 0 and 255")
    if args.max_differing_pixels < 0:
        parser.error("--max-differing-pixels must not be negative")

    try:
        catalog = read_catalog(args.exe)
        references = selected_references(args.references, args.renderer, args.id)
    except (OSError, subprocess.CalledProcessError, ValueError, json.JSONDecodeError) as error:
        print(f"visual regression setup failed: {error}", file=sys.stderr)
        return 2

    if not references:
        print("no visual references selected", file=sys.stderr)
        return 2

    failures = 0
    for renderer, demo_id, reference_path in references:
        demo = catalog.get(demo_id)
        label = f"{renderer}/{demo_id}"
        if not reference_path.is_file():
            print(f"FAIL {label}: missing reference {reference_path}")
            failures += 1
            continue
        if demo is None:
            print(f"FAIL {label}: demo is absent from the catalog")
            failures += 1
            continue
        if demo.get("kind") != "game" or not demo.get("deterministic"):
            print(f"FAIL {label}: references require a deterministic Game demo")
            failures += 1
            continue
        known_issue_renderers = demo.get("knownIssueRenderers", [])
        if demo.get("knownIssue") and (not known_issue_renderers or renderer in known_issue_renderers):
            print(f"FAIL {label}: active known-issue demos cannot be visual references")
            failures += 1
            continue

        output_root = args.output / renderer
        actual_path = output_root / f"{demo_id}.png"
        diff_path = output_root / f"{demo_id}.diff.png"
        stdout_path = output_root / f"{demo_id}.stdout.log"
        stderr_path = output_root / f"{demo_id}.stderr.log"
        output_root.mkdir(parents=True, exist_ok=True)
        actual_path.unlink(missing_ok=True)
        diff_path.unlink(missing_ok=True)

        command = [str(args.exe), demo_id, "--automated", "--quiet", "--renderer", renderer,
                   "--screenshot", str(actual_path)]
        result = subprocess.run(command, capture_output=True, text=True)
        stdout_path.write_text(result.stdout)
        stderr_path.write_text(result.stderr)
        if result.returncode != 0 or not actual_path.is_file():
            print(f"FAIL {label}: capture exited {result.returncode}; see {stdout_path}")
            failures += 1
            continue

        comparison = compare_images(reference_path, actual_path, args.tolerance,
                                    args.max_differing_pixels, diff_path)
        status = "PASS" if comparison.passed else "FAIL"
        print(f"{status} {label}: {comparison.message}")
        failures += not comparison.passed

    print(f"Compared {len(references)} reference(s); {failures} failure(s).")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
