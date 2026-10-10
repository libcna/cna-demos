#!/usr/bin/env python3
# SPDX-License-Identifier: MS-PL
"""Regenerates the original procedural assets under assets/, content/ and content_xnb/.

assets/ holds files demos load as they are (PNG, WAV). content/ holds sources that CNA's content
pipeline compiles at build time (cna_add_content) into CNA's own CNB format, such as the pixel-font
sheet; content_xnb/ holds sources compiled into XNA's XNB format instead.

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

import pixel_font

ROOT = Path(__file__).resolve().parent.parent / "assets"
CONTENT_ROOT = Path(__file__).resolve().parent.parent / "content"
CONTENT_XNB_ROOT = Path(__file__).resolve().parent.parent / "content_xnb"


def save_png(image: Image.Image, relative: str, root: Path = ROOT) -> None:
    path = root / relative
    path.parent.mkdir(parents=True, exist_ok=True)
    # optimize=False and no metadata keep the bytes stable across Pillow runs.
    image.save(path, format="PNG", optimize=False)
    print(f"wrote {path.relative_to(root.parent)} ({image.width}x{image.height})")


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


def soft_circle(size: int) -> Image.Image:
    """White disc whose alpha falls off smoothly towards the edge (straight, not premultiplied,
    alpha -- what an image editor saves)."""
    image = Image.new("RGBA", (size, size))
    pixels = image.load()
    centre = (size - 1) / 2.0
    for y in range(size):
        for x in range(size):
            d = math.hypot(x - centre, y - centre) / (size / 2.0)
            alpha = max(0.0, min(1.0, (1.0 - d) * 2.0))
            pixels[x, y] = (255, 255, 255, int(round(alpha * 255)))
    return image


def keyed_ship(size: int) -> Image.Image:
    """A small ship on a magenta (255, 0, 255) background -- the colour XNA's texture processor
    keys out by default -- for the colour-key demo."""
    image = Image.new("RGBA", (size, size), (255, 0, 255, 255))
    draw = ImageDraw.Draw(image)
    s = size / 64.0
    draw.polygon([(32 * s, 4 * s), (56 * s, 56 * s), (32 * s, 44 * s), (8 * s, 56 * s)], fill=(200, 210, 230, 255))
    draw.ellipse([26 * s, 20 * s, 38 * s, 34 * s], fill=(60, 140, 230, 255))
    draw.rectangle([20 * s, 52 * s, 44 * s, 58 * s], fill=(255, 140, 40, 255))
    return image


def framed_gradient(width: int, height: int) -> Image.Image:
    """A gradient with a one-pixel white frame: resizing shows in the frame's thickness."""
    image = gradient(width, height)
    draw = ImageDraw.Draw(image)
    draw.rectangle([0, 0, width - 1, height - 1], outline=(255, 255, 255, 255))
    return image


def colour_wheel(size: int) -> Image.Image:
    """Hues around the centre, white towards it: smooth colour that block compression approximates."""
    image = Image.new("RGBA", (size, size))
    pixels = image.load()
    centre = (size - 1) / 2.0
    for y in range(size):
        for x in range(size):
            angle = math.atan2(y - centre, x - centre)
            radius = min(1.0, math.hypot(x - centre, y - centre) / centre)
            channel = [0.5 + 0.5 * math.cos(angle - k * 2.0 * math.pi / 3.0) for k in range(3)]
            pixels[x, y] = tuple(int(round(255 * (1.0 - radius + radius * c))) for c in channel) + (255,)
    return image


def sprite_strip(frame: int, frames: int) -> Image.Image:
    """A horizontal strip of `frames` square frames; frame i shows i + 1 vertical bars, so the
    current animation frame is readable on screen."""
    image = Image.new("RGBA", (frame * frames, frame), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    colors = [(230, 80, 80, 255), (80, 200, 90, 255), (80, 120, 230, 255), (230, 200, 60, 255)]
    for i in range(frames):
        x0 = i * frame
        draw.rectangle([x0 + 1, 1, x0 + frame - 2, frame - 2], outline=colors[i % len(colors)], width=2)
        bar = max(2, frame // 10)
        for b in range(i + 1):
            bx = x0 + 6 + b * (bar * 2)
            draw.rectangle([bx, 8, bx + bar - 1, frame - 9], fill=colors[i % len(colors)])
    return image


def font_sheet(scale: int, columns: int = 16) -> Image.Image:
    """The pixel font as an XNA "sprite font texture": one cell per character from U+0020 on,
    left to right and top to bottom, cells separated by 1-pixel magenta lines. A cell is the
    glyph's advance: ink (white) plus transparent padding -- one empty column to the right and one
    empty row below each glyph -- which the processor keeps as cropping."""
    characters = [chr(code) for code in range(32, 127)]
    cell_w = (pixel_font.WIDTH + 1) * scale
    cell_h = (pixel_font.HEIGHT + 1) * scale
    rows = (len(characters) + columns - 1) // columns
    magenta = (255, 0, 255, 255)
    image = Image.new("RGBA", (columns * (cell_w + 1) + 1, rows * (cell_h + 1) + 1), magenta)
    pixels = image.load()
    for index, character in enumerate(characters):
        left = 1 + (index % columns) * (cell_w + 1)
        top = 1 + (index // columns) * (cell_h + 1)
        glyph = pixel_font.rows(character)
        for y in range(cell_h):
            for x in range(cell_w):
                gx, gy = x // scale, y // scale
                ink = gx < pixel_font.WIDTH and gy < pixel_font.HEIGHT and glyph[gy][gx] == "#"
                pixels[left + x, top + y] = (255, 255, 255, 255) if ink else (0, 0, 0, 0)
    return image


def tone(relative: str, frequency: float, seconds: float, rate: int = 22050, volume: float = 0.5,
         root: Path = ROOT) -> None:
    path = root / relative
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
    print(f"wrote {path.relative_to(root.parent)} ({frequency} Hz, {seconds} s, {rate} Hz mono)")


def melody(relative: str, notes, note_seconds: float, rate: int = 22050, volume: float = 0.4) -> None:
    """A short tune: each MIDI note number is a sine with a plucked (decaying) envelope."""
    path = ROOT / relative
    path.parent.mkdir(parents=True, exist_ok=True)
    per_note = int(rate * note_seconds)
    attack = int(rate * 0.005)
    samples = bytearray()
    for note in notes:
        frequency = 440.0 * 2.0 ** ((note - 69) / 12.0)
        for i in range(per_note):
            envelope = min(1.0, i / attack) * math.exp(-3.0 * i / per_note) * min(1.0, (per_note - 1 - i) / attack)
            value = volume * envelope * math.sin(2.0 * math.pi * frequency * i / rate)
            samples += struct.pack("<h", int(value * 32767))
    with wave.open(str(path), "wb") as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(rate)
        out.writeframes(bytes(samples))
    print(f"wrote {path.relative_to(ROOT.parent)} ({len(notes)} notes, {len(notes) * note_seconds} s)")


def main() -> None:
    save_png(checker(64, 8, (255, 255, 255, 255), (72, 61, 139, 255)), "textures/checker.png")
    save_png(arrow(64), "textures/arrow.png")
    save_png(gradient(128, 128), "textures/gradient.png")
    save_png(soft_circle(64), "textures/soft_circle.png")
    save_png(sprite_strip(32, 4), "textures/sprite_strip.png")
    save_png(font_sheet(2), "fonts/pixel_font.png", CONTENT_ROOT)
    save_png(keyed_ship(64), "textures/keyed_ship.png", CONTENT_ROOT)
    save_png(soft_circle(64), "textures/soft_circle_compiled.png", CONTENT_ROOT)
    save_png(framed_gradient(100, 60), "textures/odd_size.png", CONTENT_ROOT)
    save_png(checker(128, 8, (255, 255, 255, 255), (20, 20, 20, 255)), "textures/mipmapped_checker.png", CONTENT_ROOT)
    save_png(colour_wheel(128), "textures/colour_wheel.png", CONTENT_XNB_ROOT)
    save_png(colour_wheel(128), "textures/colour_wheel_dxt.png", CONTENT_XNB_ROOT)
    tone("sounds/chime.wav", 880.0, 0.3, root=CONTENT_ROOT)
    tone("sounds/tone_440.wav", 440.0, 0.5)
    tone("sounds/tone_660.wav", 660.0, 0.25)
    melody("music/rising.wav", [60, 64, 67, 72, 64, 67, 72, 76], 0.25)   # C major, upwards
    melody("music/falling.wav", [81, 76, 72, 69, 76, 72, 69, 64], 0.25)  # A minor, downwards


if __name__ == "__main__":
    main()
