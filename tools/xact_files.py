# SPDX-License-Identifier: MS-PL
"""Minimal XACT project files for the audio.xact demos: settings (.xgs), a wave bank (.xwb) and a
sound bank (.xsb), all original data.

XNA games got these files from the XACT authoring tool, which no longer exists for current
platforms. The byte layout written here is the one CNA's own XACT example generates
(cna/modules/audio/examples/demo_xact/src/XactFileGen.hpp, Ms-PL) and CNA's XACT parser reads:
non-compact wave bank, simple cues (one sound, one wave each), two categories and one global
variable.

tools/generate_assets.py calls write_all(); the files land in assets/xact/.
"""

import math
import struct
from pathlib import Path

SAMPLE_RATE = 22050

# name, frequency (Hz), seconds, category index (0 = Music, 1 = SFX)
CUES = [
    ("LowTone", 261.6, 0.6, 0),
    ("MidTone", 329.6, 0.6, 0),
    ("HighTone", 392.0, 0.4, 1),
    ("Beep", 880.0, 0.2, 1),
]


def sine(frequency: float, seconds: float) -> bytes:
    count = int(SAMPLE_RATE * seconds)
    fade = int(0.02 * SAMPLE_RATE)
    samples = []
    for i in range(count):
        envelope = min(1.0, i / fade, (count - i) / fade)
        samples.append(int(math.sin(2.0 * math.pi * frequency * i / SAMPLE_RATE) * envelope * 20000.0))
    return struct.pack("<%dh" % count, *samples)


def name64(text: str) -> bytes:
    return text.encode("ascii")[:63].ljust(64, b"\0")


def xgs() -> bytes:
    """Settings: categories Music and SFX (no parent, full volume), variable SpeedOfSound."""
    cat_off, var_off = 80, 100
    catname_off = var_off + 13
    varname_off = catname_off + len(b"Music\0SFX\0")
    out = bytearray()
    out += struct.pack("<IHHH", 0x46534758, 46, 0, 0) + bytes(8) + b"\0"            # "XGSF", versions, time, platform
    out += struct.pack("<7H", 2, 1, 0, 0, 0, 0, 0)                                 # 2 categories, 1 variable
    out += struct.pack("<8I", cat_off, var_off, 0, 0, 0, 0, catname_off, varname_off)
    out = out.ljust(80, b"\0")
    for _ in range(2):                                                             # instance limit, fades, behaviour,
        out += struct.pack("<BHHBHBB", 255, 0, 0, 0, 0xFFFF, 180, 1)               # no parent, volume byte 180 = 0 dB
    out += struct.pack("<Bfff", 0x01, 343.0, 0.0, 1000.0)                          # PUBLIC, initial, min, max
    out += b"Music\0SFX\0SpeedOfSound\0"
    assert len(out) == 136
    return bytes(out)


def xwb(bank: str, waves) -> bytes:
    """A non-compact wave bank of mono 16-bit PCM waves."""
    offsets, total = [], 0
    for wave in waves:
        offsets.append(total)
        total += len(wave)
    seg0_off, seg0_len = 52, 96
    seg1_off, seg1_len = seg0_off + seg0_len, 24 * len(waves)
    seg4_off = (seg1_off + seg1_len + 3) & ~3
    out = bytearray(struct.pack("<III", 0x444E4257, 46, 44))                      # "WBND", version, header version
    out += struct.pack("<10I", seg0_off, seg0_len, seg1_off, seg1_len, 0, 0, 0, 0, seg4_off, total)
    out += struct.pack("<II", 0, len(waves)) + name64(bank) + struct.pack("<IIII", 24, 0, 4, 0) + bytes(8)
    fmt = (SAMPLE_RATE << 5) | (1 << 31)                                           # PCM, mono, 16-bit
    for offset, wave in zip(offsets, waves):
        out += struct.pack("<6I", 0, fmt, offset, len(wave), 0, 0)
    out = out.ljust(seg4_off, b"\0")
    for wave in waves:
        out += wave
    return bytes(out)


def xsb(bank: str, wave_bank: str, cues) -> bytes:
    """A sound bank with one simple cue -> one sound -> one wave per entry."""
    count = len(cues)
    header = 138
    sound_off = header
    cue_off = sound_off + 12 * count
    wbname_off = cue_off + 5 * count
    nameindex_off = wbname_off + 64
    names_off = nameindex_off + 6 * count
    out = bytearray(struct.pack("<IHHH", 0x4B424453, 46, 0, 0) + bytes(8) + b"\0")  # "SDBK"
    out += struct.pack("<HHHHBHHH", count, 0, 0, 0, 1, count, 0, 0)
    out += struct.pack("<10i", cue_off, -1, -1, -1, -1, -1, wbname_off, -1, nameindex_off, sound_off)
    out += name64(bank)
    assert len(out) == header
    for index, (_, _, _, category) in enumerate(cues):
        out += struct.pack("<BHBhBHHB", 0, category, 180, 0, 255, 0, index, 0)     # simple sound
    for index in range(count):
        out += struct.pack("<BI", 0, sound_off + 12 * index)                       # simple cue -> sound
    out += name64(wave_bank)
    position = names_off
    for index, (name, _, _, _) in enumerate(cues):
        out += struct.pack("<IH", position, index)
        position += len(name) + 1
    for name, _, _, _ in cues:
        out += name.encode("ascii") + b"\0"
    return bytes(out)


def write_all(assets_root: Path) -> None:
    folder = assets_root / "xact"
    folder.mkdir(parents=True, exist_ok=True)
    files = {
        "demo.xgs": xgs(),
        "tones.xwb": xwb("Tones", [sine(frequency, seconds) for _, frequency, seconds, _ in CUES]),
        "tones.xsb": xsb("ToneSounds", "Tones", CUES),
    }
    for name, data in files.items():
        (folder / name).write_bytes(data)
        print(f"wrote assets/xact/{name} ({len(data)} bytes)")
