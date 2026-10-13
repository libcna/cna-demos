# SPDX-License-Identifier: MS-PL
"""Original DXT1 DDS fixtures for 2D mip and cube content-loading demonstrations."""
import struct
from pathlib import Path


def write_all(assets_root: Path) -> None:
    def block(rgb565):
        return struct.pack('<HHI', rgb565, 0, 0)  # opaque endpoint zero at all sixteen texels
    header = bytearray(128)
    header[:4] = b'DDS '
    fields = {4: 124, 8: 0xA1007, 12: 8, 16: 16, 20: 64, 28: 5,
              76: 32, 80: 4, 84: int.from_bytes(b'DXT1', 'little'), 108: 0x401008}
    for offset, value in fields.items():
        struct.pack_into('<I', header, offset, value)
    # 16x8: red, yellow, lime, blue, each four texels wide, repeated for both block rows.
    payload = b''.join(block(c) for c in [0xF800, 0xFFE0, 0x07E0, 0x001F] * 2)
    # Full chain: 8x4 magenta (two blocks), 4x2 cyan, 2x1 white, 1x1 red.
    payload += block(0xF81F) * 2 + block(0x07FF) + block(0xFFFF) + block(0xF800)
    path = assets_root / 'textures' / 'striped_mips.dds'
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(header + payload)

    # A minimal 4x4 DXT1 cube: one block for each face. The legacy caps2 mask marks all six faces.
    cube_header = bytearray(128)
    cube_header[:4] = b'DDS '
    cube_fields = {4: 124, 8: 0x6, 12: 4, 16: 4, 28: 1,
                   76: 32, 80: 4, 84: int.from_bytes(b'DXT1', 'little'),
                   108: 0x1000, 112: 0xFE00}
    for offset, value in cube_fields.items():
        struct.pack_into('<I', cube_header, offset, value)
    cube_payload = b''.join(block(color) for color in
                            (0xF800, 0x07E0, 0x001F, 0xFFE0, 0xF81F, 0x07FF))
    (path.parent / 'cube_faces.dds').write_bytes(cube_header + cube_payload)
