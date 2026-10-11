# SPDX-License-Identifier: MS-PL
"""Original XNB card parents, compiled label and loose image external-reference fixtures."""
import struct
from pathlib import Path
from PIL import Image


def write_all(assets_root: Path) -> None:
    def string(value):
        encoded = value.encode('utf-8')
        assert len(encoded) < 128
        return bytes([len(encoded)]) + encoded

    def xnb(reader, payload):
        data = bytes([1]) + string(reader + ', CnaDemos') + struct.pack('<i', 0) + bytes([0, 1]) + payload
        return b'XNBw' + bytes([5, 0]) + struct.pack('<I', 10 + len(data)) + data

    root = assets_root / 'external'
    for name in ['entries', 'labels', 'images']:
        (root / name).mkdir(parents=True, exist_ok=True)
    paths = {'card_a': '../images/banner', 'card_b': '..\\images\\banner',
             'escape': '../../../cna-demos-outside', 'absolute': '/cna-demos-outside', 'missing': '../images/absent'}
    for name, path in paths.items():
        payload = string(path) + string('') + string('../labels/knight') + string('')
        (root / 'entries' / f'{name}.xnb').write_bytes(xnb('CnaDemos.ExternalCardReader', payload))
    (root / 'labels' / 'knight.xnb').write_bytes(xnb('CnaDemos.ExternalCardLabelReader', string('Knight') + struct.pack('<i', 4)))
    image = Image.new('RGBA', (2, 1));image.putdata([(255, 0, 0, 255), (0, 0, 255, 255)])
    image.save(root / 'images' / 'banner.png', optimize=False)
