# SPDX-License-Identifier: MS-PL
"""Original tagged tones for isolated MediaLibrary import demos.

All PCM samples and tags are authored here. The embedded-art tracks also carry this project's
generated PNGs. A compact format matrix covers MediaLibrary's Ogg/OGA, MP3, FLAC and
Opus tag paths; the demos use an existing WAV for the filename fallback. ffmpeg encodes the
original audio and muxes that image; committed fixtures let demo users run without the encoder
installed.
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

EMBEDDED_MP3_TRACK = (
    'track_embedded_mp3.mp3', 'Pocket', 'MP3 Ensemble', 'ID3 Pictures', 1, 280.0, 3600,
    'textures/checker.png',
)

FORMAT_TRACKS = (
    ('format_vorbis.ogg', 'libvorbis', 'Vorbis Song', 'Format Ensemble', 1, 300.0, True),
    ('format_alias.oga', 'libvorbis', 'OGA Alias', 'FORMAT ENSEMBLE', 2, 350.0, False),
    ('format_id3.mp3', 'libmp3lame', 'ID3 Song', 'Format Ensemble', 3, 400.0, False),
    ('format_flac.flac', 'flac', 'FLAC Song', 'Format Ensemble', 4, 450.0, False),
    ('format_opus.opus', 'libopus', 'Opus Song', 'Format Ensemble', 5, 500.0, False),
)

PARTIAL_TRACK = ('format_partial.ogg', 'Partial Tag', 520.0)


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

    name, title, artist, album, track, frequency, count, art = EMBEDDED_MP3_TRACK
    pcm = b''.join(struct.pack('<h', int(12000 * math.sin(2 * math.pi * frequency * i / 16000)
                     * min(1.0, i / 80, (count - i) / 80))) for i in range(count))
    destination = folder / name
    command = [encoder, '-loglevel', 'error', '-y', '-f', 's16le', '-ar', '16000', '-ac', '1',
               '-i', 'pipe:0', '-i', str(assets_root / art), '-map', '0:a', '-map', '1:v:0',
               '-map_metadata', '-1', '-fflags', '+bitexact', '-flags:a', '+bitexact',
               '-c:a', 'libmp3lame', '-b:a', '48k', '-id3v2_version', '4', '-write_id3v1', '0',
               '-c:v', 'copy', '-disposition:v', 'attached_pic',
               '-metadata:s:v', 'title=Album cover', '-metadata:s:v', 'comment=Cover (front)',
               '-metadata', 'title=' + title, '-metadata', 'artist=' + artist,
               '-metadata', 'album=' + album, '-metadata', 'genre=Instrumental',
               '-metadata', 'track=' + str(track), str(destination)]
    subprocess.run(command, input=pcm, check=True)
    print(f'wrote {destination} ({destination.stat().st_size} bytes, embedded {art})')

    format_folder = assets_root / 'media' / 'library_formats'
    format_folder.mkdir(parents=True, exist_ok=True)
    sample_count = 2400
    for name, codec, title, artist, track, frequency, rated in FORMAT_TRACKS:
        pcm = b''.join(struct.pack('<h', int(10000 * math.sin(2 * math.pi * frequency * i / 16000)
                         * min(1.0, i / 80, (sample_count - i) / 80))) for i in range(sample_count))
        destination = format_folder / name
        command = [encoder, '-loglevel', 'error', '-y', '-f', 's16le', '-ar', '16000', '-ac', '1',
                   '-i', 'pipe:0', '-map_metadata', '-1', '-fflags', '+bitexact', '-flags:a', '+bitexact',
                   '-c:a', codec, '-metadata', 'title=' + title,
                   '-metadata', 'artist=' + artist, '-metadata', 'album=Format Collection',
                   '-metadata', 'genre=Signals', '-metadata', 'track=' + str(track)]
        if rated:
            command.extend(('-metadata', 'rating=80'))
        if codec == 'libmp3lame':
            command.extend(('-b:a', '48k', '-id3v2_version', '4', '-write_id3v1', '0'))
        elif codec == 'libopus':
            command.extend(('-b:a', '32k'))
        elif codec == 'libvorbis':
            command.extend(('-q:a', '2'))
        command.append(str(destination))
        subprocess.run(command, input=pcm, check=True)
        print(f'wrote {destination} ({destination.stat().st_size} bytes, {codec})')

    name, title, frequency = PARTIAL_TRACK
    pcm = b''.join(struct.pack('<h', int(10000 * math.sin(2 * math.pi * frequency * i / 16000)
                     * min(1.0, i / 80, (sample_count - i) / 80))) for i in range(sample_count))
    destination = format_folder / name
    command = [encoder, '-loglevel', 'error', '-y', '-f', 's16le', '-ar', '16000', '-ac', '1',
               '-i', 'pipe:0', '-map_metadata', '-1', '-fflags', '+bitexact', '-flags:a', '+bitexact',
               '-c:a', 'libvorbis', '-q:a', '2', '-metadata', 'title=' + title, str(destination)]
    subprocess.run(command, input=pcm, check=True)
    print(f'wrote {destination} ({destination.stat().st_size} bytes, partial Vorbis tags)')
