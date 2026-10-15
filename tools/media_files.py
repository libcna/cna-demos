# SPDX-License-Identifier: MS-PL
"""Original compressed media for cna-demos, written into content_media/:

- videos/bounce.wmv: a ball bouncing across a gradient with a progress bar, and a short plucked
  tune, as WMV (WMV2 video, WMA2 audio) -- the format XNA's content pipeline imported;
- videos/track_matrix.mkv: one second of authored red and blue 160x90 video streams plus two
  48 kHz mono tracks, for public Video/VideoPlayer track-selection behavior;
- music/theme.ogg: a four-second tune as Ogg Vorbis, a format CNA's SongProcessor compiles.

Every frame is drawn here with Pillow and every audio sample computed here; ffmpeg (the system
package) only encodes them. Re-encoding with another ffmpeg version may change the bytes but not
the content. Used by tools/generate_assets.py; skipped with a message when ffmpeg is missing."""

import math
import shutil
import struct
import subprocess
import tempfile
import wave
from pathlib import Path

from PIL import Image, ImageDraw

WIDTH, HEIGHT, FPS, SECONDS = 160, 120, 15, 3
RATE = 22050


def frame(index: int) -> bytes:
    t = index / FPS
    image = Image.new("RGB", (WIDTH, HEIGHT))
    pixels = image.load()
    for y in range(HEIGHT):
        for x in range(WIDTH):
            pixels[x, y] = (20 + x // 2, 30 + y, 90 + (x + y) // 4)
    draw = ImageDraw.Draw(image)
    # A ball bouncing left and right, and up and down, once per second.
    bx = 20 + (WIDTH - 40) * abs(math.sin(math.pi * t / 2.0))
    by = HEIGHT - 20 - 70 * abs(math.sin(math.pi * t))
    draw.ellipse((bx - 10, by - 10, bx + 10, by + 10), fill=(250, 200, 40))
    # A bar at the bottom shows how far the clip has played.
    draw.rectangle((0, HEIGHT - 4, int(WIDTH * index / (FPS * SECONDS - 1)), HEIGHT - 1), fill=(255, 255, 255))
    return image.tobytes()


def tune(path: Path, notes=(60, 64, 67, 72, 67, 64)) -> None:
    """Plucked sine notes, half a second each (default: a C major arpeggio)."""
    per_note = RATE // 2
    samples = bytearray()
    for note in notes:
        frequency = 440.0 * 2.0 ** ((note - 69) / 12.0)
        for i in range(per_note):
            envelope = min(1.0, i / 100) * math.exp(-4.0 * i / per_note)
            samples += struct.pack("<h", int(0.4 * envelope * math.sin(2 * math.pi * frequency * i / RATE) * 32767))
    with wave.open(str(path), "wb") as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(RATE)
        out.writeframes(bytes(samples))


def tone(path: Path, rate: int, frequency: float, seconds: int = 1) -> None:
    """Write an authored mono sine tone used by the multi-track video fixture."""
    samples = bytearray()
    for i in range(rate * seconds):
        envelope = min(1.0, i / 100, (rate * seconds - i) / 100)
        samples += struct.pack("<h", int(0.25 * envelope * math.sin(2 * math.pi * frequency * i / rate) * 32767))
    with wave.open(str(path), "wb") as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(rate)
        out.writeframes(bytes(samples))


def write_all(media_root: Path) -> None:
    if shutil.which("ffmpeg") is None:
        print("skipped videos: ffmpeg not found (the committed files stay as they are)")
        return
    target = media_root / "videos" / "bounce.wmv"
    target.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory() as scratch:
        audio = Path(scratch) / "tune.wav"
        tune(audio)
        command = ["ffmpeg", "-loglevel", "error", "-y",
                   "-f", "rawvideo", "-pix_fmt", "rgb24", "-s", f"{WIDTH}x{HEIGHT}", "-r", str(FPS), "-i", "-",
                   "-i", str(audio),
                   "-c:v", "wmv2", "-b:v", "300k", "-c:a", "wmav2", "-b:a", "48k", "-ar", str(RATE), "-ac", "1",
                   "-threads", "1", "-fflags", "+bitexact", "-flags", "+bitexact", "-shortest", str(target)]
        frames = b"".join(frame(i) for i in range(FPS * SECONDS))
        subprocess.run(command, input=frames, check=True)
    print(f"wrote {target.relative_to(media_root.parent)} ({WIDTH}x{HEIGHT}, {FPS} fps, {SECONDS} s, "
          f"{target.stat().st_size} bytes)")

    track_target = media_root / "videos" / "track_matrix.mkv"
    with tempfile.TemporaryDirectory() as scratch:
        scratch = Path(scratch)
        red_frames = scratch / "red.rgb"
        blue_frames = scratch / "blue.rgb"
        red_frames.write_bytes(bytes((220, 20, 20)) * (160 * 90 * 10))
        blue_frames.write_bytes(bytes((20, 20, 220)) * (160 * 90 * 10))
        low_tone = scratch / "low.wav"
        high_tone = scratch / "high.wav"
        tone(low_tone, 48000, 440.0)
        tone(high_tone, 48000, 880.0)
        command = [
            "ffmpeg", "-loglevel", "error", "-y",
            "-f", "rawvideo", "-pixel_format", "rgb24", "-video_size", "160x90", "-framerate", "10",
            "-i", str(red_frames),
            "-f", "rawvideo", "-pixel_format", "rgb24", "-video_size", "160x90", "-framerate", "10",
            "-i", str(blue_frames),
            "-i", str(low_tone), "-i", str(high_tone),
            "-map", "0:v:0", "-map", "1:v:0", "-map", "2:a:0", "-map", "3:a:0",
            "-c:v", "ffv1", "-level", "3", "-pix_fmt", "yuv420p", "-c:a", "flac",
            "-disposition:v:0", "default", "-disposition:v:1", "0",
            "-disposition:a:0", "default", "-disposition:a:1", "0",
            "-map_metadata", "-1", "-map_chapters", "-1", "-threads", "1",
            "-fflags", "+bitexact", "-flags:v", "+bitexact", "-flags:a", "+bitexact",
            "-shortest", str(track_target),
        ]
        subprocess.run(command, check=True)
    print(f"wrote {track_target.relative_to(media_root.parent)} "
          f"(2 video + 2 audio tracks, {track_target.stat().st_size} bytes)")

    song = media_root / "music" / "theme.ogg"
    song.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory() as scratch:
        audio = Path(scratch) / "theme.wav"
        tune(audio, (69, 72, 76, 81, 79, 76, 72, 74))  # A minor, four seconds
        subprocess.run(["ffmpeg", "-loglevel", "error", "-y", "-i", str(audio), "-c:a", "libvorbis", "-q:a", "3",
                        "-ar", str(RATE), "-ac", "1", "-threads", "1", "-fflags", "+bitexact", "-flags", "+bitexact",
                        "-map_metadata", "-1", str(song)], check=True)
    print(f"wrote {song.relative_to(media_root.parent)} (4 s, {song.stat().st_size} bytes)")
