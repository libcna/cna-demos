# SPDX-License-Identifier: MS-PL
"""Original glTF 2.0 scenes for the cna-demos model demos, written as JSON with base64 buffers.

tools/generate_assets.py calls write_all(); the files land in content/models/, where CNA's content
pipeline (CNA.GltfImporter -> CNA.ModelProcessor) compiles them into Model assets, and one copy in
assets/models/ that is loaded as a loose glTF file at run time.

* robot_arm.gltf -- three boxes in a node hierarchy (Base -> UpperArm -> Forearm). Every vertex
  carries COLOR_0 with the lighting baked in (brighter top, darker sides), which makes CNA give
  each part (with a KHR_materials_unlit material) a BasicEffect with vertex colours: the model
  looks the same on every renderer and needs no lights.
* bending_strip.gltf -- a vertical strip skinned to two joints (LowerJoint -> UpperJoint) with an
  animation "Bend" that rotates UpperJoint back and forth. It has NORMAL and COLOR_0, so CNA gives it a
  SkinnedEffect.
"""

import base64
import json
import math
import struct
from pathlib import Path

FLOAT = 5126
UNSIGNED_SHORT = 5123
UNSIGNED_BYTE = 5121
ARRAY_BUFFER = 34962
ELEMENT_ARRAY_BUFFER = 34963


class Document:
    def __init__(self):
        self.gltf = {"asset": {"version": "2.0", "generator": "cna-demos tools/gltf_models.py"},
                     "accessors": [], "bufferViews": [], "buffers": [], "meshes": [], "nodes": []}
        self.data = bytearray()

    def view(self, payload: bytes, target=None) -> int:
        while len(self.data) % 4:
            self.data.append(0)
        view = {"buffer": 0, "byteOffset": len(self.data), "byteLength": len(payload)}
        if target is not None:
            view["target"] = target
        self.data += payload
        self.gltf["bufferViews"].append(view)
        return len(self.gltf["bufferViews"]) - 1

    def accessor(self, values, kind: str, component=FLOAT, target=ARRAY_BUFFER, minmax=False) -> int:
        width = {"SCALAR": 1, "VEC2": 2, "VEC3": 3, "VEC4": 4, "MAT4": 16}[kind]
        flat = [c for v in values for c in (v if width > 1 else [v])]
        fmt = {FLOAT: "f", UNSIGNED_SHORT: "H", UNSIGNED_BYTE: "B"}[component]
        accessor = {"bufferView": self.view(struct.pack("<" + fmt * len(flat), *flat), target),
                    "componentType": component, "count": len(values), "type": kind}
        if minmax:
            accessor["min"] = [min(v[i] for v in values) for i in range(width)]
            accessor["max"] = [max(v[i] for v in values) for i in range(width)]
        self.gltf["accessors"].append(accessor)
        return len(self.gltf["accessors"]) - 1

    def save(self, path: Path) -> None:
        self.gltf["buffers"] = [{"byteLength": len(self.data),
                                 "uri": "data:application/octet-stream;base64," + base64.b64encode(bytes(self.data)).decode()}]
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(self.gltf, indent=1) + "\n")
        print(f"wrote {path.relative_to(path.parents[2])} ({len(self.gltf['nodes'])} nodes)")


# Face brightness for the baked lighting: light from above, slightly from the front right.
SHADE = {"+y": 1.0, "+z": 0.82, "+x": 0.7, "-x": 0.55, "-z": 0.6, "-y": 0.4}


def box(lo, hi, colour):
    """Positions, colours and indices of an axis-aligned box. Triangles are counter-clockwise
    seen from outside (glTF's front faces)."""
    (x0, y0, z0), (x1, y1, z1) = lo, hi
    faces = {
        "+x": [(x1, y0, z1), (x1, y0, z0), (x1, y1, z0), (x1, y1, z1)],
        "-x": [(x0, y0, z0), (x0, y0, z1), (x0, y1, z1), (x0, y1, z0)],
        "+y": [(x0, y1, z1), (x1, y1, z1), (x1, y1, z0), (x0, y1, z0)],
        "-y": [(x0, y0, z0), (x1, y0, z0), (x1, y0, z1), (x0, y0, z1)],
        "+z": [(x0, y0, z1), (x1, y0, z1), (x1, y1, z1), (x0, y1, z1)],
        "-z": [(x1, y0, z0), (x0, y0, z0), (x0, y1, z0), (x1, y1, z0)],
    }
    positions, colours, indices = [], [], []
    for name, corners in faces.items():
        base = len(positions)
        positions += corners
        shade = SHADE[name]
        colours += [(colour[0] * shade, colour[1] * shade, colour[2] * shade, 1.0)] * 4
        indices += [base, base + 1, base + 2, base, base + 2, base + 3]
    return positions, colours, indices


def robot_arm(path: Path) -> None:
    doc = Document()
    parts = [
        ("Base", (-1.0, 0.0, -1.0), (1.0, 0.5, 1.0), (0.6, 0.6, 0.65), None),
        ("UpperArm", (-0.25, 0.0, -0.25), (0.25, 2.0, 0.25), (0.95, 0.55, 0.15), (0.0, 0.5, 0.0)),
        ("Forearm", (-0.18, 0.0, -0.18), (0.18, 1.5, 0.18), (0.2, 0.5, 0.95), (0.0, 2.0, 0.0)),
    ]
    for index, (name, lo, hi, colour, translation) in enumerate(parts):
        positions, colours, indices = box(lo, hi, colour)
        doc.gltf["meshes"].append({"name": name + "Mesh", "primitives": [{
            "attributes": {"POSITION": doc.accessor(positions, "VEC3", minmax=True),
                           "COLOR_0": doc.accessor(colours, "VEC4")},
            "indices": doc.accessor(indices, "SCALAR", UNSIGNED_SHORT, ELEMENT_ARRAY_BUFFER),
            "material": 0}]})
        node = {"name": name, "mesh": index}
        if translation is not None:
            node["translation"] = list(translation)
        if index + 1 < len(parts):
            node["children"] = [index + 1]
        doc.gltf["nodes"].append(node)
    # Unlit: the colours are the final ones (the shading is baked into COLOR_0).
    doc.gltf["materials"] = [{"name": "BakedColours", "extensions": {"KHR_materials_unlit": {}}}]
    doc.gltf["extensionsUsed"] = ["KHR_materials_unlit"]
    doc.gltf["scenes"] = [{"name": "RobotArm", "nodes": [0]}]
    doc.gltf["scene"] = 0
    doc.save(path)


def bending_strip(path: Path) -> None:
    doc = Document()
    # A strip 0.4 wide and 4 high, in 8 segments; the lower half follows LowerJoint, the upper half
    # UpperJoint,
    # blended over the middle so the bend is smooth.
    positions, normals, colours, joints, weights, indices = [], [], [], [], [], []
    segments = 8
    for row in range(segments + 1):
        y = 4.0 * row / segments
        tip = min(1.0, max(0.0, (y - 1.5) / 1.0))
        for x in (-0.2, 0.2):
            positions.append((x, y, 0.0))
            normals.append((0.0, 0.0, 1.0))
            shade = 0.6 + 0.4 * row / segments
            colours.append((0.3 * shade, 0.85 * shade, 0.4 * shade, 1.0))
            joints.append((0, 1, 0, 0))
            weights.append((1.0 - tip, tip, 0.0, 0.0))
    for row in range(segments):
        a = row * 2
        indices += [a, a + 1, a + 3, a, a + 3, a + 2]
    mesh = {"name": "StripMesh", "primitives": [{
        "attributes": {"POSITION": doc.accessor(positions, "VEC3", minmax=True),
                       "NORMAL": doc.accessor(normals, "VEC3"),
                       "COLOR_0": doc.accessor(colours, "VEC4"),
                       "JOINTS_0": doc.accessor(joints, "VEC4", UNSIGNED_BYTE),
                       "WEIGHTS_0": doc.accessor(weights, "VEC4")},
        "indices": doc.accessor(indices, "SCALAR", UNSIGNED_SHORT, ELEMENT_ARRAY_BUFFER),
        "material": 0}]}
    # Unlit + skinned + COLOR_0 is what CNA maps to XNA's SkinnedEffect.
    doc.gltf["materials"] = [{"name": "StripColours", "extensions": {"KHR_materials_unlit": {}}}]
    doc.gltf["extensionsUsed"] = ["KHR_materials_unlit"]
    doc.gltf["meshes"].append(mesh)
    # Nodes: 0 Strip (mesh, skinned), 1 LowerJoint, 2 UpperJoint at y = 2.
    doc.gltf["nodes"] = [
        {"name": "Strip", "mesh": 0, "skin": 0},
        {"name": "LowerJoint", "children": [2]},
        {"name": "UpperJoint", "translation": [0.0, 2.0, 0.0]},
    ]
    identity = [1.0, 0, 0, 0, 0, 1.0, 0, 0, 0, 0, 1.0, 0, 0, 0, 0, 1.0]
    tip_inverse = [1.0, 0, 0, 0, 0, 1.0, 0, 0, 0, 0, 1.0, 0, 0, -2.0, 0, 1.0]  # column-major: translate -2 in y
    doc.gltf["skins"] = [{"name": "StripRig", "joints": [1, 2], "skeleton": 1,
                          "inverseBindMatrices": doc.accessor([identity, tip_inverse], "MAT4", target=None)}]
    # "Bend": UpperJoint rotates about Z to +50 degrees, to -50 degrees and back, over 2 seconds.
    times = [0.0, 0.5, 1.0, 1.5, 2.0]
    angles = [0.0, 50.0, 0.0, -50.0, 0.0]
    rotations = [(0.0, 0.0, math.sin(math.radians(a) / 2), math.cos(math.radians(a) / 2)) for a in angles]
    doc.gltf["animations"] = [{"name": "Bend",
                               "samplers": [{"input": doc.accessor(times, "SCALAR", target=None, minmax=False),
                                             "output": doc.accessor(rotations, "VEC4", target=None),
                                             "interpolation": "LINEAR"}],
                               "channels": [{"sampler": 0, "target": {"node": 2, "path": "rotation"}}]}]
    # glTF requires min/max on animation input accessors.
    doc.gltf["accessors"][doc.gltf["animations"][0]["samplers"][0]["input"]].update({"min": [0.0], "max": [2.0]})
    doc.gltf["scenes"] = [{"name": "BendingStrip", "nodes": [0, 1]}]
    doc.gltf["scene"] = 0
    doc.save(path)


def write_all(content_root: Path, assets_root: Path) -> None:
    robot_arm(content_root / "models" / "robot_arm.gltf")
    bending_strip(content_root / "models" / "bending_strip.gltf")
    # The same arm as a loose file: CNA can also load glTF at run time, without the pipeline.
    robot_arm(assets_root / "models" / "loose_robot_arm.gltf")
