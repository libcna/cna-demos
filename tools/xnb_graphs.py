# SPDX-License-Identifier: MS-PL
"""Original uncompressed XNB graphs for public ContentReader shared-resource callbacks."""
import struct
from pathlib import Path


def write_all(assets_root: Path) -> None:
    def number(value):
        data = bytearray()
        while True:
            part = value & 127
            value >>= 7
            data.append(part | (128 if value else 0))
            if not value:
                return bytes(data)
    def string(value):
        data = value.encode('utf-8')
        return number(len(data)) + data
    def item(name, value, next_index, previous_index):
        return number(2) + string(name) + struct.pack('<i', value) + number(next_index) + number(previous_index)
    readers = ['CnaDemos.RuntimeGraphReader, CnaDemos', 'CnaDemos.RuntimeItemReader, CnaDemos',
               'CnaDemos.RuntimeLabelReader, CnaDemos']
    table = number(len(readers)) + b''.join(string(name) + struct.pack('<i', 0) for name in readers)
    root = assets_root / 'graphs'
    root.mkdir(parents=True, exist_ok=True)
    for name in ['shared_party', 'bad_shared_index', 'bad_shared_type']:
        # Two shared resources follow the root object. All reference indices are 1-based; 0 is null.
        data = table + number(2) + number(1)
        data += number(3 if name == 'bad_shared_index' else 1) + number(1) + number(0)
        data += item('Sword', 7, 0, 0)  # ordinary nested object: same values, separate identity
        # The wrong-type case still has a valid XNB object: a LabelReader result in slot one.
        data += number(3) + string('not an item') if name == 'bad_shared_type' else item('Sword', 7, 2, 0)
        data += item('Gem', 11, 0, 1)
        payload = b'XNBw' + bytes([5, 0]) + struct.pack('<I', 10 + len(data)) + data
        (root / f'{name}.xnb').write_bytes(payload)
