# SPDX-License-Identifier: MS-PL
"""Original XNB level with aliased owned tile blocks and a null optional block."""
import struct
from pathlib import Path


def write_all(assets_root: Path) -> None:
    def string(value):
        encoded = value.encode('utf-8')
        assert len(encoded) < 128
        return bytes([len(encoded)]) + encoded

    readers = ['CnaDemos.OwnedLevelReader, CnaDemos', 'CnaDemos.OwnedTilesReader, CnaDemos']
    data = bytes([len(readers)]) + b''.join(string(name) + struct.pack('<i', 0) for name in readers)
    data += bytes([0, 1]) + string('Courtyard') + struct.pack('<i', 0)  # no shared resources; root reader index one
    tiles = bytes([1, 1, 0, 2, 0, 3, 3, 0])
    encoded_tiles = bytes([2]) + struct.pack('<i', len(tiles)) + tiles
    data += encoded_tiles + encoded_tiles + bytes([0])
    root = assets_root / 'ownership'
    root.mkdir(parents=True, exist_ok=True)
    (root / 'level.xnb').write_bytes(b'XNBw' + bytes([5, 0]) + struct.pack('<I', 10 + len(data)) + data)

    short = bytes([1]) + string('CnaDemos.OwnedTilesReader, CnaDemos') + struct.pack('<i', 0)
    short += bytes([0, 1]) + struct.pack('<i', len(tiles) + 1) + tiles
    (root / 'truncated_tiles.xnb').write_bytes(
        b'XNBw' + bytes([5, 0]) + struct.pack('<I', 10 + len(short)) + short)

    oversized_rows = bytes([len(readers)]) + b''.join(
        string(name) + struct.pack('<i', 0) for name in readers)
    oversized_rows += bytes([0, 1]) + string('Untrusted') + struct.pack('<i', 10_000_001)
    (root / 'oversized_rows.xnb').write_bytes(
        b'XNBw' + bytes([5, 0]) + struct.pack('<I', 10 + len(oversized_rows)) + oversized_rows)

    oversized_tiles = bytes([1]) + string('CnaDemos.OwnedTilesReader, CnaDemos') + struct.pack('<i', 0)
    oversized_tiles += bytes([0, 1]) + struct.pack('<i', 256 * 1024 * 1024 + 1)
    (root / 'oversized_tiles.xnb').write_bytes(
        b'XNBw' + bytes([5, 0]) + struct.pack('<I', 10 + len(oversized_tiles)) + oversized_tiles)
