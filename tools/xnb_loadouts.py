# SPDX-License-Identifier: MS-PL
"""Original XNB loadout mixing indexed objects, raw payloads and null object references."""
import struct
from pathlib import Path


def write_all(assets_root: Path) -> None:
    def string(value):
        encoded = value.encode('utf-8')
        assert len(encoded) < 128  # all authored schema names use one-byte 7-bit lengths
        return bytes([len(encoded)]) + encoded

    readers = ['CnaDemos.LoadoutReader, CnaDemos', 'CnaDemos.LoadoutWeaponReader, CnaDemos']
    data = bytes([len(readers)]) + b''.join(string(r) + struct.pack('<i', 0) for r in readers)
    data += bytes([0, 1])  # no shared resources; indexed root object
    data += bytes([2]) + string('Sword') + struct.pack('<i', 12)  # indexed existing primary
    data += string('Bow') + struct.pack('<i', 6)  # raw existing secondary: no reader index
    data += bytes([0])  # retain the supplied fallback object
    data += bytes([2]) + string('Sword') + struct.pack('<i', 12)  # fresh, type-erased tag
    data += bytes([0]) + struct.pack('<i', 0x12345678)  # empty tag and alignment trailer
    directory = assets_root / 'loadouts'
    directory.mkdir(parents=True, exist_ok=True)
    (directory / 'existing_slots.xnb').write_bytes(b'XNBw' + bytes([5, 0]) + struct.pack('<I', 10 + len(data)) + data)
