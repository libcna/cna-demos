#!/usr/bin/env python3
# SPDX-License-Identifier: MS-PL
"""Regenerates the original procedural assets under assets/.

Every file this script writes is original cna-demos content (no third-party
input), so it is covered by the project's own licence. The generated files are
committed; run this script only to change or add one, then commit the result:

    python3 tools/generate_assets.py

Requires Pillow for the PNG files (pip install pillow). WAV files use only the
standard library. Output is deterministic: running it twice yields identical
bytes, so an unchanged asset produces no Git diff.
"""

import math
import struct
import wave
from pathlib import Path

from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parent.parent / "assets"


def save_png(image: Image.Image, relative: str) -> None:
    path = ROOT / relative
    path.parent.mkdir(parents=True, exist_ok=True)
    # optimize=False and no metadata keep the bytes stable across Pillow runs.
    image.save(path, format="PNG", optimize=False)
    print(f"wrote {path.relative_to(ROOT.parent)} ({image.width}x{image.height})")


def checker(size: int, cell: int, a, b) -> Image.Image:
    image = Image.new("RGBA", (size, size))
    pixels = image.load()
    for y in range(size):
        for x in range(size):
            pixels[x, y] = a if (x // cell + y // cell) % 2 == 0 else b
    return image


def arrow(size: int) -> Image.Image:
    """An arrow pointing right with a marker dot in the top-left corner: any
    rotation or mirroring of it is unambiguous on screen."""
    image = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    s = size / 64.0
    draw.rectangle([8 * s, 26 * s, 40 * s, 38 * s], fill=(255, 200, 40, 255))
    draw.polygon([(40 * s, 14 * s), (58 * s, 32 * s), (40 * s, 50 * s)], fill=(255, 200, 40, 255))
    draw.ellipse([4 * s, 4 * s, 14 * s, 14 * s], fill=(220, 40, 40, 255))
    return image


def gradient(width: int, height: int) -> Image.Image:
    image = Image.new("RGBA", (width, height))
    pixels = image.load()
    for y in range(height):
        for x in range(width):
            pixels[x, y] = (x * 255 // (width - 1), y * 255 // (height - 1), 128, 255)
    return image


def tone(relative: str, frequency: float, seconds: float, rate: int = 22050, volume: float = 0.5) -> None:
    path = ROOT / relative
    path.parent.mkdir(parents=True, exist_ok=True)
    frames = int(rate * seconds)
    fade = int(rate * 0.01)  # 10 ms fade in/out avoids clicks
    samples = bytearray()
    for i in range(frames):
        envelope = min(1.0, i / fade, (frames - 1 - i) / fade)
        value = volume * envelope * math.sin(2.0 * math.pi * frequency * i / rate)
        samples += struct.pack("<h", int(value * 32767))
    with wave.open(str(path), "wb") as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(rate)
        out.writeframes(bytes(samples))
    print(f"wrote {path.relative_to(ROOT.parent)} ({frequency} Hz, {seconds} s, {rate} Hz mono)")


def main() -> None:
    save_png(checker(64, 8, (255, 255, 255, 255), (72, 61, 139, 255)), "textures/checker.png")
    save_png(arrow(64), "textures/arrow.png")
    save_png(gradient(128, 128), "textures/gradient.png")
    tone("sounds/tone_440.wav", 440.0, 0.5)
    tone("sounds/tone_660.wav", 660.0, 0.25)


if __name__ == "__main__":
    main()
