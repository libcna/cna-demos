# SPDX-License-Identifier: MS-PL
"""Original versioned XNB stat records: legacy Health, current Armor and a future unsupported field."""
import struct
from pathlib import Path


def write_all(assets_root: Path) -> None:
    name = b'CnaDemos.VersionedStatsReader, CnaDemos'
    schemas = [('legacy_stats', 0, 0, 80, None), ('current_stats', 1, 1, 120, 25.0),
               ('future_stats', 2, 2, 100, 1.5)]
    root = assets_root / 'versions'
    root.mkdir(parents=True, exist_ok=True)
    for asset, version, fields, health, extra in schemas:
        data = bytes([1, len(name)]) + name + struct.pack('<i', version) + bytes([0, 1])
        data += bytes([fields]) + struct.pack('<i', health)
        if extra is not None:
            data += struct.pack('<f', extra)
        (root / f'{asset}.xnb').write_bytes(b'XNBw' + bytes([5, 0]) + struct.pack('<I', 10 + len(data)) + data)
