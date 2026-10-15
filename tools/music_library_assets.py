# SPDX-License-Identifier: MS-PL
"""Original tagged FLAC tones for isolated MediaLibrary import demos.

All PCM samples and tags are authored here. The embedded-art track also carries one of this
project's generated PNGs. ffmpeg encodes the original audio and muxes that image; committed
fixtures let demo users run without the encoder installed.
"""
import math
from pathlib import Path
import shutil
import struct
import subprocess

TRACKS = (
    ('track_a.flac', 'Dawn', 'Demo Ensemble', 'Field Notes', 1, 440.0, 4000),
    ('track_b.flac', 'Dusk', 'Demo Ensemble', 'Field Notes', 2, 660.0, 8000),
    ('track_c.flac', 'Echo', 'Second Ensemble', 'Field Notes', 1, 550.0, 6000),
)

EMBEDDED_TRACK = (
    'track_embedded.flac', 'Inside', 'Embedded Ensemble', 'Inner Notes', 1, 330.0, 5000,
    'textures/gradient.png',
)


def write_all(assets_root: Path) -> None:
    encoder = shutil.which('ffmpeg')
    if encoder is None:
        raise RuntimeError('ffmpeg is required to regenerate the original music-library fixtures')
    folder = assets_root / 'media' / 'library_music'
    folder.mkdir(parents=True, exist_ok=True)
    for name, title, artist, album, track, frequency, count in TRACKS:
        pcm = b''.join(struct.pack('<h', int(12000 * math.sin(2 * math.pi * frequency * i / 16000)
                         * min(1.0, i / 80, (count - i) / 80))) for i in range(count))
        destination = folder / name
        command = [encoder, '-loglevel', 'error', '-y', '-f', 's16le', '-ar', '16000', '-ac', '1',
                   '-i', 'pipe:0', '-map_metadata', '-1', '-fflags', '+bitexact', '-flags:a', '+bitexact',
                   '-c:a', 'flac', '-metadata', 'title=' + title, '-metadata', 'artist=' + artist,
                   '-metadata', 'album=' + album, '-metadata', 'genre=Instrumental',
                   '-metadata', 'track=' + str(track), str(destination)]
        subprocess.run(command, input=pcm, check=True)
        print(f'wrote {destination} ({destination.stat().st_size} bytes)')

    name, title, artist, album, track, frequency, count, art = EMBEDDED_TRACK
    pcm = b''.join(struct.pack('<h', int(12000 * math.sin(2 * math.pi * frequency * i / 16000)
                     * min(1.0, i / 80, (count - i) / 80))) for i in range(count))
    destination = folder / name
    command = [encoder, '-loglevel', 'error', '-y', '-f', 's16le', '-ar', '16000', '-ac', '1',
               '-i', 'pipe:0', '-i', str(assets_root / art), '-map', '0:a', '-map', '1:v:0',
               '-map_metadata', '-1', '-fflags', '+bitexact', '-flags:a', '+bitexact',
               '-c:a', 'flac', '-c:v', 'copy', '-disposition:v', 'attached_pic',
               '-metadata', 'title=' + title, '-metadata', 'artist=' + artist,
               '-metadata', 'album=' + album, '-metadata', 'genre=Instrumental',
               '-metadata', 'track=' + str(track), str(destination)]
    subprocess.run(command, input=pcm, check=True)
    print(f'wrote {destination} ({destination.stat().st_size} bytes, embedded {art})')
