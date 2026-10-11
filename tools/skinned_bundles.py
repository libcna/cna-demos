# SPDX-License-Identifier: MS-PL
"""Original loose SkinnedModelEXT bundle, matching CNA's documented sidecar format."""
import json
import struct
from pathlib import Path
from PIL import Image


def write_all(assets_root: Path) -> None:
    root = assets_root / "models" / "skinned_bundle"
    root.mkdir(parents=True, exist_ok=True)
    def matrix(y=0):
        return struct.pack("<16f", 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, y, 0, 1)
    # Two bones, child translated +0.2 in bind pose, inverse GLOBAL bind translated -0.2.
    skeleton = struct.pack("<3i", 2, -1, 0) + matrix() + matrix(.2) + matrix() + matrix(-.2)
    (root / "rig.skeleton.bin").write_bytes(skeleton)
    vertices = bytearray()
    for joint, x0, y0, u in [(0, -.6, -.6, .25), (1, .2, .2, .75)]:
        for x, y in [(x0, y0), (x0 + .4, y0), (x0 + .4, y0 + .4), (x0, y0 + .4)]:
            # 52-byte stock layout: position, normal, UV, four weights, four byte joint indices.
            vertices += struct.pack("<12f4B", x, y, .5, 0, 0, 1, u, .5, 1, 0, 0, 0, joint, 0, 0, 0)
    (root / "patches.vertices.bin").write_bytes(vertices)
    (root / "patches.indices.bin").write_bytes(struct.pack("<12H", 0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7))
    texture = Image.new("RGBA", (2, 1))
    texture.putdata([(255, 0, 0, 255), (0, 255, 0, 255)])
    texture.save(root / "patches.png", optimize=False)
    # One second, one track for joint 1; translations are LOCAL, not already skin matrices.
    clip = struct.pack("<diii", 1.0, 1, 1, 2)
    for time, x in [(0, 0), (1, -.4)]:
        clip += struct.pack("<d10f", time, x, .2, 0, 0, 0, 0, 1, 1, 1, 1)
    (root / "slide.clip.bin").write_bytes(clip)
    manifest = {"skeleton": "rig.skeleton.bin", "parts": [{"name": "Patches",
                "vertices": "patches.vertices.bin", "indices": "patches.indices.bin",
                "vertexStride": 52, "texture": "patches.png"}],
                "animations": [{"name": "Slide", "clip": "slide.clip.bin"}]}
    (root / "patches.skinnedmodel.json").write_text(json.dumps(manifest, indent=2) + "\n")
    # A header claims two bones but contains no arrays; the public reader must refuse the bundle.
    (root / "truncated.skeleton.bin").write_bytes(struct.pack("<i", 2))
    (root / "truncated.skinnedmodel.json").write_text(json.dumps({"skeleton": "truncated.skeleton.bin"}) + "\n")
