#!/usr/bin/env python3
# SPDX-License-Identifier: MS-PL
"""Captures the last frame of Game demos and lays them out as one labelled contact sheet.

Developer tool for reviewing many graphical demos at once (requires Pillow).

    python3 tools/contact_sheet.py --exe build/linux-full/bin/cna-demos --renderer SOFTWARE \\
        --prefix graphics.sprite_batch --out /tmp/sheet.png [--xvfb]

Only demos of kind "game" whose ID starts with --prefix are captured. Each screenshot is taken
with `--automated --screenshot`; failures are shown as red tiles with the status line.
"""

import argparse
import json
import subprocess
import sys
import tempfile
from pathlib import Path

from PIL import Image, ImageDraw


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--exe", required=True)
    parser.add_argument("--renderer", default="SOFTWARE")
    parser.add_argument("--prefix", default="")
    parser.add_argument("--out", required=True)
    parser.add_argument("--xvfb", action="store_true", help="run under xvfb-run (windowed renderers)")
    parser.add_argument("--columns", type=int, default=3)
    parser.add_argument("--tile", type=int, default=400, help="tile width in pixels")
    args = parser.parse_args()

    catalog = json.loads(subprocess.run([args.exe, "--catalog-json"], check=True, capture_output=True, text=True).stdout)
    demos = [d for d in catalog["demos"] if d["kind"] == "game" and d["id"].startswith(args.prefix)
             and d["automation"] != "manual"]
    if not demos:
        print("no matching demos", file=sys.stderr)
        return 1

    tiles = []
    with tempfile.TemporaryDirectory() as tmp:
        for demo in demos:
            shot = Path(tmp) / f"{demo['id']}.png"
            command = [args.exe, demo["id"], "--automated", "--quiet", "--renderer", args.renderer,
                       "--screenshot", str(shot)]
            if args.xvfb:
                command = ["xvfb-run", "-a", "-s", "-screen 0 1280x1024x24"] + command
            result = subprocess.run(command, capture_output=True, text=True)
            status = (result.stdout.strip().splitlines() or ["?"])[-1]
            image = Image.open(shot).convert("RGB") if shot.exists() else None
            tiles.append((demo["id"], status, image))
            print(f"{demo['id']}: {status}")

    width = args.tile
    height = int(width * 0.6) + 18
    rows = (len(tiles) + args.columns - 1) // args.columns
    sheet = Image.new("RGB", (width * args.columns, height * rows), (40, 40, 40))
    draw = ImageDraw.Draw(sheet)
    for index, (demo_id, status, image) in enumerate(tiles):
        x = (index % args.columns) * width
        y = (index // args.columns) * height
        if image is not None:
            thumb = image.resize((width - 4, int(width * 0.6) - 4))
            sheet.paste(thumb, (x + 2, y + 18))
        else:
            draw.rectangle([x + 2, y + 18, x + width - 3, y + height - 3], fill=(120, 20, 20))
        draw.text((x + 4, y + 3), f"{demo_id}  {status.replace('== ', '')}"[:70], fill=(255, 255, 255))
    sheet.save(args.out)
    print(f"wrote {args.out} ({len(tiles)} demos)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
