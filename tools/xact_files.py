# SPDX-License-Identifier: MS-PL
"""Minimal XACT project files for the audio.xact demos: settings (.xgs), a wave bank (.xwb) and a
sound bank (.xsb), all original data.

XNA games got these files from the XACT authoring tool, which no longer exists for current
platforms. The byte layout written here is the one CNA's own XACT example generates
(cna/modules/audio/examples/demo_xact/src/XactFileGen.hpp, Ms-PL) and CNA's XACT parser reads:
non-compact wave banks, simple cues (one sound, one wave each), two categories, writable/read-only
global variables and one cue variable. music.xwb is a streaming wave bank (flag set, wave data aligned to 2048-byte sectors), the
kind a game streams from disc instead of loading. limits.xgs reuses the tone banks with
authored category limits: Music rejects a second voice and SFX replaces the oldest.
release.xsb authors a 500 ms fade-out on a complex single-sound cue backed by a three-second tone.
release_rpc.xgs/.xsb bind a volume RPC to XACT's built-in ReleaseTime variable, giving a simple cue
a 150 ms public Stopping phase without an authored cue fade.

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

# The streamed music: one cue, a three-second arpeggio, in the Music category.
MUSIC_NOTES = [261.6, 329.6, 392.0, 523.3, 392.0, 329.6, 261.6, 196.0]
MUSIC_NOTE_SECONDS = 0.375


def sine(frequency: float, seconds: float) -> bytes:
    count = int(SAMPLE_RATE * seconds)
    fade = int(0.02 * SAMPLE_RATE)
    samples = []
    for i in range(count):
        envelope = min(1.0, i / fade, (count - i) / fade)
        samples.append(int(math.sin(2.0 * math.pi * frequency * i / SAMPLE_RATE) * envelope * 20000.0))
    return struct.pack("<%dh" % count, *samples)


def melody(frequencies, note_seconds: float) -> bytes:
    return b"".join(sine(frequency, note_seconds) for frequency in frequencies)


def name64(text: str) -> bytes:
    return text.encode("ascii")[:63].ljust(64, b"\0")


def xgs(category_limits=((255, 0), (255, 0)),
        category_parents=(0xFFFF, 0xFFFF), category_names=("Music", "SFX")) -> bytes:
    """Settings: two named categories (independent Music/SFX by default, full volume); writable
    SpeedOfSound and read-only ReadOnlyLevel globals plus cue-local Intensity. Category limits are
    (count, policy): 0 = Fail, 2 = ReplaceOldest; policies occupy bits 3..7 in XGS."""
    assert len(category_limits) == len(category_parents) == len(category_names) == 2
    assert all(parent in (0, 1, 0xFFFF) for parent in category_parents)
    assert all(name and "\0" not in name for name in category_names)
    names = b"".join(name.encode("ascii") + b"\0" for name in category_names)
    cat_off, var_off = 80, 100
    catname_off = var_off + 3 * 13
    varname_off = catname_off + len(names)
    out = bytearray()
    out += struct.pack("<IHHH", 0x46534758, 46, 0, 0) + bytes(8) + b"\0"            # "XGSF", versions, time, platform
    out += struct.pack("<7H", 2, 3, 0, 0, 0, 0, 0)                                 # 2 categories, 3 variables
    out += struct.pack("<8I", cat_off, var_off, 0, 0, 0, 0, catname_off, varname_off)
    out = out.ljust(80, b"\0")
    for (count, policy), parent in zip(category_limits, category_parents):
        # Instance limit, zero fades, behaviour, authored parent, volume byte 180 = 0 dB, public.
        out += struct.pack("<BHHBHBB", count, 0, 0, policy << 3, parent, 180, 1)
    out += struct.pack("<Bfff", 0x01, 343.0, 0.0, 1000.0)                          # PUBLIC: initial, min, max
    out += struct.pack("<Bfff", 0x01 | 0x04, 50.0, 0.0, 100.0)                     # PUBLIC | CUE
    out += struct.pack("<Bfff", 0x01 | 0x02, 25.0, 0.0, 100.0)                     # PUBLIC | READONLY
    out += names + b"SpeedOfSound\0Intensity\0ReadOnlyLevel\0"
    assert len(out) == 176 + len(names)
    return bytes(out)


def xwb(bank: str, waves, streaming: bool = False) -> bytes:
    """A non-compact wave bank of mono 16-bit PCM waves; a streaming bank aligns its waves to
    2048-byte sectors, as the XACT tool does."""
    alignment = 2048 if streaming else 4
    offsets, total = [], 0
    for wave in waves:
        total = (total + alignment - 1) & ~(alignment - 1)
        offsets.append(total)
        total += len(wave)
    seg0_off, seg0_len = 52, 96
    seg1_off, seg1_len = seg0_off + seg0_len, 24 * len(waves)
    seg4_off = (seg1_off + seg1_len + alignment - 1) & ~(alignment - 1)
    out = bytearray(struct.pack("<III", 0x444E4257, 46, 44))                      # "WBND", version, header version
    out += struct.pack("<10I", seg0_off, seg0_len, seg1_off, seg1_len, 0, 0, 0, 0, seg4_off, total)
    flags = 1 if streaming else 0                                                  # WAVEBANK_TYPE_STREAMING
    out += struct.pack("<II", flags, len(waves)) + name64(bank) + struct.pack("<IIII", 24, 0, alignment, 0) + bytes(8)
    fmt = (SAMPLE_RATE << 5) | (1 << 31)                                           # PCM, mono, 16-bit
    for offset, wave in zip(offsets, waves):
        out += struct.pack("<6I", 0, fmt, offset, len(wave), 0, 0)
    out = out.ljust(seg4_off, b"\0")
    for offset, wave in zip(offsets, waves):
        out = out.ljust(seg4_off + offset, b"\0")
        out += wave
    return bytes(out)


def xsb(bank: str, wave_bank: str, cues, fade_out_ms: int | None = None,
        cue_limits=None, priorities=None) -> bytes:
    """One cue -> one sound -> one wave per entry. An authored fade uses complex
    single-sound cue records; simple cues have no fade or instance-limit fields.
    Optional per-cue (count, policy) limits use the same complex record."""
    assert fade_out_ms is None or 0 <= fade_out_ms <= 65535
    count = len(cues)
    assert cue_limits is None or (len(cue_limits) == count and
        all(0 <= limit <= 255 and 0 <= policy <= 4 for limit, policy in cue_limits))
    assert priorities is None or (len(priorities) == count and all(0 <= priority <= 255 for priority in priorities))
    complex_cues = fade_out_ms is not None or cue_limits is not None
    header = 138
    sound_off = header
    cue_off = sound_off + 12 * count
    wbname_off = cue_off + (15 if complex_cues else 5) * count
    nameindex_off = wbname_off + 64
    names_off = nameindex_off + 6 * count
    out = bytearray(struct.pack("<IHHH", 0x4B424453, 46, 0, 0) + bytes(8) + b"\0")  # "SDBK"
    out += struct.pack("<HHHHBHHH", 0 if complex_cues else count, count if complex_cues else 0, 0, 0, 1, count, 0, 0)
    out += struct.pack("<10i", -1 if complex_cues else cue_off, cue_off if complex_cues else -1, -1, -1, -1, -1, wbname_off, -1, nameindex_off, sound_off)
    out += name64(bank)
    assert len(out) == header
    for index, (_, _, _, category) in enumerate(cues):
        priority = 255 if priorities is None else priorities[index]
        out += struct.pack("<BHBhBHHB", 0, category, 180, 0, priority, 0, index, 0) # simple sound
    for index in range(count):
        if complex_cues:
            # SINGLE_SOUND, sound offset, no transition, instance limit, no fade-in,
            # authored fade-out milliseconds and max-instance policy in bits 3..7.
            limit, policy = (255, 0) if cue_limits is None else cue_limits[index]
            out += struct.pack("<BIIBHHB", 0x04, sound_off + 12 * index, 0, limit, 0,
                               fade_out_ms or 0, policy << 3)
        else:
            out += struct.pack("<BI", 0, sound_off + 12 * index)                   # simple cue -> sound
    out += name64(wave_bank)
    position = names_off
    for index, (name, _, _, _) in enumerate(cues):
        out += struct.pack("<IH", position, index)
        position += len(name) + 1
    for name, _, _, _ in cues:
        out += name.encode("ascii") + b"\0"
    return bytes(out)


def release_rpc_xgs() -> bytes:
    """One category and one built-in ReleaseTime variable driving a volume RPC. The curve reaches
    its -20 dB floor at 30 ms and extends to 150 ms; the final x value defines the release phase."""
    header_size = 69
    category_offset = header_size
    variable_offset = category_offset + 10
    category_name_offset = variable_offset + 13
    variable_name_offset = category_name_offset + len(b"Default\0")
    rpc_offset = variable_name_offset + len(b"ReleaseTime\0")

    out = bytearray(struct.pack("<IHHH", 0x46534758, 46, 0, 0) + bytes(8) + b"\x03")
    out += struct.pack("<7H", 1, 1, 0, 0, 1, 0, 0)
    out += struct.pack("<9I", category_offset, variable_offset, 0, 0, 0, 0,
                       category_name_offset, variable_name_offset, rpc_offset)
    assert len(out) == header_size

    out += struct.pack("<BHHBHBB", 255, 0, 0, 0, 0xFFFF, 255, 0)
    out += struct.pack("<Bfff", 0x03, 0.0, 0.0, 100000.0)
    out += b"Default\0ReleaseTime\0"
    assert len(out) == rpc_offset

    out += struct.pack("<HBH", 0, 3, 0)  # variable 0, three points, volume parameter
    out += struct.pack("<ffB", 0.0, 0.0, 0)
    out += struct.pack("<ffB", 30.0, -2000.0, 0)
    out += struct.pack("<ffB", 150.0, -2000.0, 0)
    return bytes(out)


def release_rpc_xsb(rpc_code: int = 112) -> bytes:
    """A simple ReleaseTimeCue using Release wave zero and one sound-level RPC code."""
    header = 138
    wave_bank_name_offset = header
    sound_offset = wave_bank_name_offset + 64
    sound_size = 12 + 7
    cue_offset = sound_offset + sound_size
    name_index_offset = cue_offset + 5
    name_offset = name_index_offset + 6

    out = bytearray(struct.pack("<IHHH", 0x4B424453, 46, 0, 0) + bytes(8) + b"\0")
    out += struct.pack("<HHHHBHHH", 1, 0, 0, 0, 1, 1, 0, 0)
    out += struct.pack("<10i", cue_offset, -1, -1, 0, -1, 0, wave_bank_name_offset, 0,
                       name_index_offset, sound_offset)
    out += name64("ReleaseRpcSounds")
    assert len(out) == header
    out += name64("Release")
    out += struct.pack("<BHBhBHHB", 0x02, 0, 0x64, 0, 0, 0, 0, 0)
    out += struct.pack("<HBI", 7, 1, rpc_code)
    out += struct.pack("<BI", 0, sound_offset)
    out += struct.pack("<IH", name_offset, 0)
    out += b"ReleaseTimeCue\0"
    return bytes(out)


def write_all(assets_root: Path) -> None:
    folder = assets_root / "xact"
    folder.mkdir(parents=True, exist_ok=True)
    files = {
        "demo.xgs": xgs(),
        "limits.xgs": xgs(((1, 0), (1, 2))),
        "priority.xgs": xgs(((255, 0), (2, 4))),
        "hierarchy.xgs": xgs(category_parents=(0xFFFF, 0), category_names=("Master", "SFX")),
        "release.xwb": xwb("Release", [sine(329.6, 3.0)]),
        "release.xsb": xsb("ReleaseSounds", "Release", [("ReleaseTone", 329.6, 3.0, 0)], fade_out_ms=500),
        "release_rpc.xgs": release_rpc_xgs(),
        "release_rpc.xsb": release_rpc_xsb(),
        "tones.xwb": xwb("Tones", [sine(frequency, seconds) for _, frequency, seconds, _ in CUES]),
        "tones.xsb": xsb("ToneSounds", "Tones", CUES),
        "cue_limits.xsb": xsb("LimitedToneSounds", "Tones", CUES,
                              cue_limits=((1, 0), (255, 0), (255, 0), (255, 0))),
        "cue_replace.xsb": xsb("ReplacingToneSounds", "Tones", CUES,
                               cue_limits=((255, 0), (1, 2), (255, 0), (255, 0))),
        "priority.xsb": xsb("PriorityToneSounds", "Tones",
                            (("PriorityHigh", 0.0, 0.0, 1),
                             ("PriorityLow", 0.0, 0.0, 1),
                             ("PriorityNew", 0.0, 0.0, 1)),
                            priorities=(200, 10, 100)),
        "music.xwb": xwb("Music", [melody(MUSIC_NOTES, MUSIC_NOTE_SECONDS)], streaming=True),
        "music.xsb": xsb("MusicSounds", "Music", [("Theme", 0.0, 0.0, 0)]),
    }
    for name, data in files.items():
        (folder / name).write_bytes(data)
        print(f"wrote assets/xact/{name} ({len(data)} bytes)")
