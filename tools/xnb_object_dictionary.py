# SPDX-License-Identifier: MS-PL
"""Deterministic hand-authored XNB dictionary for the C object-dictionary member audit."""
import struct
from pathlib import Path


READERS = (
    "Microsoft.Xna.Framework.Content.DictionaryReader`2[[System.String],[System.Object]]",
    "Microsoft.Xna.Framework.Content.StringReader",
    "Microsoft.Xna.Framework.Content.BooleanReader",
    "Microsoft.Xna.Framework.Content.Int32Reader",
    "Microsoft.Xna.Framework.Content.SingleReader",
    "Microsoft.Xna.Framework.Content.DoubleReader",
    "Microsoft.Xna.Framework.Content.Vector2Reader",
    "Microsoft.Xna.Framework.Content.Vector3Reader",
    "Microsoft.Xna.Framework.Content.Vector4Reader",
    "Microsoft.Xna.Framework.Content.MatrixReader",
    "Microsoft.Xna.Framework.Content.QuaternionReader",
    "Microsoft.Xna.Framework.Content.ColorReader",
    "Microsoft.Xna.Framework.Content.BoundingSphereReader",
    "Microsoft.Xna.Framework.Content.BoundingBoxReader",
    "Microsoft.Xna.Framework.Content.ArrayReader`1[[Microsoft.Xna.Framework.Vector3]]",
    "Microsoft.Xna.Framework.Content.ReflectiveReader`1[[CNA.Demos.DictionaryEntry]]",
    "Microsoft.Xna.Framework.Content.UInt32Reader",
)


def seven_bit(value: int) -> bytes:
    result = bytearray()
    while value >= 0x80:
        result.append((value & 0x7f) | 0x80)
        value >>= 7
    result.append(value)
    return bytes(result)


def text(value: str) -> bytes:
    encoded = value.encode("utf-8")
    return seven_bit(len(encoded)) + encoded


def reader(name: str) -> bytes:
    return text(name) + struct.pack("<i", 0)


def entry(key: str, reader_index: int, payload: bytes) -> bytes:
    # Keys and values are reference-shaped objects, so each carries its 1-based reader index.
    return seven_bit(2) + text(key) + seven_bit(reader_index) + payload


def write_all(assets_root: Path) -> None:
    body = bytearray()
    body += seven_bit(len(READERS))
    for name in READERS:
        body += reader(name)
    body += seven_bit(0)  # shared resources
    body += seven_bit(1)  # root object: DictionaryReader<string, object>

    entries = (
        entry("Array", 15, struct.pack("<I9f", 3, 0.0, 0.0, 0.0,
                                        1.0, 0.0, 0.0, 0.0, 1.0, 0.0)),
        entry("Boolean", 3, b"\x01"),
        entry("Box", 14, struct.pack("<6f", -1.0, -2.0, -3.0, 4.0, 5.0, 6.0)),
        entry("Color", 12, bytes((10, 20, 30, 40))),
        entry("Custom", 16, struct.pack("<if", 1234, 0.5)),
        entry("Double", 6, struct.pack("<d", 2.5)),
        entry("Integer", 4, struct.pack("<i", -7)),
        entry("Matrix", 10, struct.pack("<16f", *[float(i) for i in range(1, 17)])),
        entry("Name", 2, text("triangles")),
        entry("Quaternion", 11, struct.pack("<4f", 0.25, 0.5, 0.75, 1.0)),
        entry("Single", 5, struct.pack("<f", 1.25)),
        entry("Sphere", 13, struct.pack("<4f", 1.0, 2.0, 3.0, 4.0)),
        entry("UnknownUInt32", 17, struct.pack("<I", 77)),
        entry("Vector2", 7, struct.pack("<2f", 2.0, 3.0)),
        entry("Vector3", 8, struct.pack("<3f", 4.0, 5.0, 6.0)),
        entry("Vector4", 9, struct.pack("<4f", 7.0, 8.0, 9.0, 10.0)),
    )
    body += struct.pack("<I", len(entries))
    for value in entries:
        body += value

    header = b"XNBw" + bytes((5, 0))
    data = header + struct.pack("<I", len(header) + 4 + len(body)) + body
    path = assets_root / "dictionaries" / "member_audit.xnb"
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(data)
