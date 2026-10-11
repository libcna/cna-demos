# SPDX-License-Identifier: MS-PL
"""One original compiled rules value that competes with editable sources in a private directory."""
import struct
from pathlib import Path


def write_all(assets_root: Path) -> None:
    reader = b'CnaDemos.ResolutionRulesReader, CnaDemos'
    data = bytes([1, len(reader)]) + reader + struct.pack('<i', 0)
    data += bytes([0, 1]) + struct.pack('<i', 400)
    root = assets_root / 'resolution'
    root.mkdir(parents=True, exist_ok=True)
    (root / 'rules.xnb').write_bytes(b'XNBw' + bytes([5, 0]) + struct.pack('<I', 10 + len(data)) + data)
