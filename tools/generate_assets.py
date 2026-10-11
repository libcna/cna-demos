#!/usr/bin/env python3
# SPDX-License-Identifier: MS-PL
"""Regenerates the original procedural assets under assets/, content/ and content_xnb/.

assets/ holds files demos load as they are (PNG, WAV). content/ holds sources that CNA's content
pipeline compiles at build time (cna_add_content) into CNA's own CNB format, such as the pixel-font
sheet; content_xnb/ holds sources compiled into XNA's XNB format instead; content_project/ holds the
sources of an XNA content project (.contentproj) built as it is.

Every file this script writes is original cna-demos content (no third-party
input), so it is covered by the project's own licence. The generated files are
committed; run this script only to change or add one, then commit the result:

    python3 tools/generate_assets.py

Requires Pillow for the PNG files (pip install pillow). WAV files use only the
standard library. Output is deterministic: running it twice yields identical
bytes, so an unchanged asset produces no Git diff.
"""

import json
import math
import struct
import wave
from pathlib import Path

from PIL import Image, ImageDraw

import gltf_models
import media_files
import xact_files
import pixel_font

ROOT = Path(__file__).resolve().parent.parent / "assets"
CONTENT_ROOT = Path(__file__).resolve().parent.parent / "content"
CONTENT_XNB_ROOT = Path(__file__).resolve().parent.parent / "content_xnb"
CONTENT_MEDIA_ROOT = Path(__file__).resolve().parent.parent / "content_media"
CONTENT_PROJECT_ROOT = Path(__file__).resolve().parent.parent / "content_project"


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


# Seven-segment digits for a score display: segment name -> rectangle (x, y, w, h) in a 10x16 cell.
SEGMENTS = {
    "a": (2, 0, 6, 2), "b": (8, 2, 2, 5), "c": (8, 9, 2, 5), "d": (2, 14, 6, 2),
    "e": (0, 9, 2, 5), "f": (0, 2, 2, 5), "g": (2, 7, 6, 2),
}
DIGIT_SEGMENTS = ["abcdef", "bc", "abged", "abgcd", "fgbc", "afgcd", "afgedc", "abc", "abcdefg", "abcdfg"]
SEGMENT_CELL = (10, 16)


def score_digits(relative_png: str, relative_cnj: str, root: Path) -> None:
    """A bitmap font drawn by hand rather than rasterised: the digits 0-9 and ':' of a
    seven-segment display, scaled 2x, in one atlas row, plus the CNJ document that names each
    glyph's rectangle in the atlas, its cropping and its kerning (left bearing, width, right
    bearing) -- what CNA's content pipeline compiles into a SpriteFont."""
    scale = 2
    cell_w, cell_h = SEGMENT_CELL[0] * scale, SEGMENT_CELL[1] * scale
    colon_w = 2 * scale
    width = 10 * cell_w + colon_w
    image = Image.new("RGBA", (width, cell_h), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    glyphs = []
    for digit, segments in enumerate(DIGIT_SEGMENTS):
        left = digit * cell_w
        for name in segments:
            x, y, w, h = SEGMENTS[name]
            draw.rectangle([left + x * scale, y * scale, left + (x + w) * scale - 1, (y + h) * scale - 1],
                           fill=(255, 255, 255, 255))
        glyphs.append({"char": ord("0") + digit, "source": [left, 0, cell_w, cell_h], "crop": [0, 0, cell_w, cell_h],
                       "kerning": [scale, cell_w, scale]})
    colon_left = 10 * cell_w
    for y in (4, 10):
        draw.rectangle([colon_left, y * scale, colon_left + colon_w - 1, (y + 2) * scale - 1], fill=(255, 255, 255, 255))
    glyphs.append({"char": ord(":"), "source": [colon_left, 0, colon_w, cell_h], "crop": [0, 0, colon_w, cell_h],
                   "kerning": [scale, colon_w, scale]})
    save_png(image, relative_png, root)
    document = {"cnjVersion": 1, "type": "SpriteFont", "texture": Path(relative_png).name,
                "lineSpacing": cell_h + 2 * scale, "spacing": 0.0, "defaultCharacter": "0", "glyphs": glyphs}
    path = root / relative_cnj
    lines = ["{"]
    for key in ("cnjVersion", "type", "texture", "lineSpacing", "spacing", "defaultCharacter"):
        lines.append(f"  {json.dumps(key)}: {json.dumps(document[key])},")
    lines.append('  "glyphs": [')
    lines.extend(f"    {json.dumps(glyph)}," for glyph in glyphs)
    lines[-1] = lines[-1].rstrip(",")
    lines.append("  ]")
    lines.append("}")
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"wrote {path.relative_to(root.parent)}")


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


CASTLE_ROWS = [
    "##########",
    "#G...#..G#",
    "#.##.#.#.#",
    "#G.....#G#",
    "##########",
]


def custom_level_xnb(relative: str) -> None:
    """An XNA 4.0 .xnb file holding the game's own type, the way a custom ContentTypeWriter would
    write it: the type-reader table names "CnaDemos.LevelReader, CnaDemos" (version 0), and the
    root object (reader 1) is a name, a row count, the rows and the number of gems. Strings are
    .NET BinaryWriter strings (7-bit encoded length + UTF-8)."""
    def seven_bit(value: int) -> bytes:
        out = bytearray()
        while True:
            byte = value & 0x7F
            value >>= 7
            out.append(byte | (0x80 if value else 0))
            if not value:
                return bytes(out)

    def string(text: str) -> bytes:
        data = text.encode("utf-8")
        return seven_bit(len(data)) + data

    body = bytearray()
    body += seven_bit(1) + string("CnaDemos.LevelReader, CnaDemos") + struct.pack("<i", 0)  # readers
    body += seven_bit(0)                       # shared resources
    body += seven_bit(1)                       # root object: reader 1
    body += string("Castle") + struct.pack("<i", len(CASTLE_ROWS))
    for row in CASTLE_ROWS:
        body += string(row)
    body += struct.pack("<i", sum(row.count("G") for row in CASTLE_ROWS))
    header = b"XNBw" + bytes([5, 0])           # Windows, XNA 4.0, not compressed, Reach
    data = header + struct.pack("<I", len(header) + 4 + len(body)) + bytes(body)
    path = ROOT / relative
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(data)
    print(f"wrote assets/{relative} ({len(data)} bytes)")


def custom_enemy_cnj() -> None:
    """Original payloads for two named CNJ factories, including one deliberate schema error."""
    documents = {
        "guard": {"cnjVersion": 1, "type": "CnaDemos.Enemy",
                  "data": {"name": "Mossling", "health": 12}},
        "elite": {"cnjVersion": 1, "type": "CnaDemos.EliteEnemy",
                  "data": {"name": "Stone Guard", "baseHealth": 50}},
        "missing_health": {"cnjVersion": 1, "type": "CnaDemos.Enemy",
                           "data": {"name": "Incomplete enemy"}},
    }
    directory = ROOT / "custom"
    directory.mkdir(parents=True, exist_ok=True)
    for name, document in documents.items():
        (directory / f"{name}.cnj").write_text(json.dumps(document, indent=2) + "\n", encoding="utf-8")


def custom_patrol_route() -> None:
    """A 3-4-5 triangle: open route length 7, closed patrol length 12."""
    directory = ROOT / "custom"
    directory.mkdir(parents=True, exist_ok=True)
    (directory / "patrol.route").write_text("0 0\n3 0\n3 4\n", encoding="utf-8")
    (directory / "patrol.scale").write_text("2.0\n", encoding="utf-8")


def main() -> None:
    save_png(checker(64, 8, (255, 255, 255, 255), (72, 61, 139, 255)), "textures/checker.png")
    save_png(arrow(64), "textures/arrow.png")
    save_png(gradient(128, 128), "textures/gradient.png")
    save_png(soft_circle(64), "textures/soft_circle.png")
    save_png(sprite_strip(32, 4), "textures/sprite_strip.png")
    save_png(font_sheet(2), "fonts/pixel_font.png", CONTENT_ROOT)
    score_digits("fonts/score_digits_atlas.png", "fonts/score_digits.cnj", CONTENT_ROOT)
    save_png(keyed_ship(64), "textures/keyed_ship.png", CONTENT_ROOT)
    save_png(soft_circle(64), "textures/soft_circle_compiled.png", CONTENT_ROOT)
    save_png(framed_gradient(100, 60), "textures/odd_size.png", CONTENT_ROOT)
    save_png(checker(128, 8, (255, 255, 255, 255), (20, 20, 20, 255)), "textures/mipmapped_checker.png", CONTENT_ROOT)
    save_png(colour_wheel(128), "textures/colour_wheel.png", CONTENT_XNB_ROOT)
    save_png(colour_wheel(128), "textures/colour_wheel_dxt.png", CONTENT_XNB_ROOT)
    save_png(font_sheet(2), "fonts/pixel_font.png", CONTENT_XNB_ROOT)
    tone("sounds/chime.wav", 880.0, 0.3, root=CONTENT_XNB_ROOT)
    tone("sounds/chime.wav", 880.0, 0.3, root=CONTENT_ROOT)
    gltf_models.write_all(CONTENT_ROOT, ROOT)
    xact_files.write_all(ROOT)
    media_files.write_all(CONTENT_MEDIA_ROOT)
    tone("sounds/tone_440.wav", 440.0, 0.5)
    tone("sounds/tone_660.wav", 660.0, 0.25)
    melody("music/rising.wav", [60, 64, 67, 72, 64, 67, 72, 76], 0.25)   # C major, upwards
    melody("music/falling.wav", [81, 76, 72, 69, 76, 72, 69, 64], 0.25)  # A minor, downwards
    custom_level_xnb("levels/castle.xnb")
    custom_enemy_cnj()
    custom_patrol_route()
    # The sources of the XNA content project content_project/Game.contentproj (written by hand).
    save_png(keyed_ship(64), "Textures/ship_source.png", CONTENT_PROJECT_ROOT)
    save_png(checker(64, 4, (255, 255, 255, 255), (30, 30, 30, 255)), "Textures/grid.png", CONTENT_PROJECT_ROOT)
    tone("Sounds/blip.wav", 1320.0, 0.15, root=CONTENT_PROJECT_ROOT)


if __name__ == "__main__":
    main()
