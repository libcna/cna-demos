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
* turntable.gltf, morph_flag.gltf, camera_rig.gltf, textured_crate.gltf, lit_sphere.gltf,
  slide_rig.gltf, loose_paint_toy.gltf -- node-animation clips, morph targets with a weights animation,
  glTF cameras, an embedded base-colour texture, KHR_lights_punctual lights,
  STEP/LINEAR/CUBICSPLINE samplers, KHR_materials_variants; see each function.
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


def turntable(path: Path) -> None:
    """A display stand: a fixed base, a platform spinning on it with a crate on top, and the crate's
    lid. Two clips animate ordinary scene nodes (no skin): "Spin" turns the platform once in four
    seconds, "Open" swings the lid up and back in two."""
    doc = Document()
    parts = [
        ("Base", (-1.2, 0.0, -1.2), (1.2, 0.3, 1.2), (0.45, 0.45, 0.5)),
        ("Platform", (-1.0, 0.0, -1.0), (1.0, 0.15, 1.0), (0.85, 0.75, 0.3)),
        ("Crate", (-0.5, 0.0, -0.5), (0.5, 0.8, 0.5), (0.65, 0.4, 0.2)),
        ("Lid", (0.0, 0.0, -0.55), (1.1, 0.12, 0.55), (0.9, 0.3, 0.25)),
    ]
    for name, lo, hi, colour in parts:
        positions, colours, indices = box(lo, hi, colour)
        doc.gltf["meshes"].append({"name": name + "Mesh", "primitives": [{
            "attributes": {"POSITION": doc.accessor(positions, "VEC3", minmax=True),
                           "COLOR_0": doc.accessor(colours, "VEC4")},
            "indices": doc.accessor(indices, "SCALAR", UNSIGNED_SHORT, ELEMENT_ARRAY_BUFFER),
            "material": 0}]})
    # Nodes: 0 Base -> 1 Platform (on the base) -> 2 Crate -> 3 Lid (hinged at the crate's left top edge).
    doc.gltf["nodes"] = [
        {"name": "Base", "mesh": 0, "children": [1]},
        {"name": "Platform", "mesh": 1, "translation": [0.0, 0.3, 0.0], "children": [2]},
        {"name": "Crate", "mesh": 2, "translation": [0.0, 0.15, 0.0], "children": [3]},
        {"name": "Lid", "mesh": 3, "translation": [-0.55, 0.8, 0.0]},
    ]

    def clip(name, node, times, rotations):
        sampler_input = doc.accessor(times, "SCALAR", target=None)
        doc.gltf["accessors"][sampler_input].update({"min": [times[0]], "max": [times[-1]]})
        return {"name": name,
                "samplers": [{"input": sampler_input, "output": doc.accessor(rotations, "VEC4", target=None),
                              "interpolation": "LINEAR"}],
                "channels": [{"sampler": 0, "target": {"node": node, "path": "rotation"}}]}

    def about_y(degrees):
        return (0.0, math.sin(math.radians(degrees) / 2), 0.0, math.cos(math.radians(degrees) / 2))

    def about_z(degrees):
        return (0.0, 0.0, math.sin(math.radians(degrees) / 2), math.cos(math.radians(degrees) / 2))

    doc.gltf["animations"] = [
        clip("Spin", 1, [0.0, 1.0, 2.0, 3.0, 4.0], [about_y(a) for a in (0, 90, 180, 270, 360)]),
        clip("Open", 3, [0.0, 1.0, 2.0], [about_z(a) for a in (0, 110, 0)]),
    ]
    doc.gltf["materials"] = [{"name": "BakedColours", "extensions": {"KHR_materials_unlit": {}}}]
    doc.gltf["extensionsUsed"] = ["KHR_materials_unlit"]
    doc.gltf["scenes"] = [{"name": "Turntable", "nodes": [0]}]
    doc.gltf["scene"] = 0
    doc.save(path)


def morph_flag(path: Path) -> None:
    """A vertical flag (a 6x4 grid of quads, coloured by COLOR_0, unlit) with two morph targets --
    "Stretch" lifts the top edge, "Shear" slides the upper rows sideways -- and an animation
    "Morph" whose channel drives the mesh's weights (glTF path "weights"): (0,0) -> (1,0) -> (1,1)
    -> (0,0) over three seconds."""
    doc = Document()
    columns, rows = 6, 4
    positions, colours, stretch, shear = [], [], [], []
    for row in range(rows + 1):
        for column in range(columns + 1):
            u, v = column / columns, row / rows
            positions.append((-1.5 + 3.0 * u, 2.0 * v, 0.0))
            colours.append((0.2 + 0.8 * u, 0.3 + 0.5 * v, 0.9 - 0.6 * u, 1.0))
            stretch.append((0.0, 0.8 * v * v, 0.0))
            shear.append((0.9 * v * v, 0.0, 0.0))
    indices = []
    for row in range(rows):
        for column in range(columns):
            a = row * (columns + 1) + column
            b, c, d = a + 1, a + columns + 1, a + columns + 2
            indices += [a, b, d, a, d, c]  # counter-clockwise seen from +Z
    doc.gltf["meshes"] = [{"name": "FlagMesh", "weights": [0.0, 0.0],
                           "extras": {"targetNames": ["Stretch", "Shear"]},
                           "primitives": [{
                               "attributes": {"POSITION": doc.accessor(positions, "VEC3", minmax=True),
                                              "COLOR_0": doc.accessor(colours, "VEC4")},
                               "targets": [{"POSITION": doc.accessor(stretch, "VEC3", minmax=True)},
                                           {"POSITION": doc.accessor(shear, "VEC3", minmax=True)}],
                               "indices": doc.accessor(indices, "SCALAR", UNSIGNED_SHORT, ELEMENT_ARRAY_BUFFER),
                               "material": 0}]}]
    doc.gltf["nodes"] = [{"name": "Flag", "mesh": 0}]
    times = [0.0, 1.0, 2.0, 3.0]
    weights = [0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 0.0]  # one value per target per key
    sampler_input = doc.accessor(times, "SCALAR", target=None)
    doc.gltf["accessors"][sampler_input].update({"min": [times[0]], "max": [times[-1]]})
    doc.gltf["animations"] = [{"name": "Morph",
                               "samplers": [{"input": sampler_input,
                                             "output": doc.accessor(weights, "SCALAR", target=None),
                                             "interpolation": "LINEAR"}],
                               "channels": [{"sampler": 0, "target": {"node": 0, "path": "weights"}}]}]
    doc.gltf["materials"] = [{"name": "VertexColours", "doubleSided": True,
                              "extensions": {"KHR_materials_unlit": {}}}]
    doc.gltf["extensionsUsed"] = ["KHR_materials_unlit"]
    doc.gltf["scenes"] = [{"name": "MorphFlag", "nodes": [0]}]
    doc.gltf["scene"] = 0
    doc.save(path)


def camera_rig(path: Path) -> None:
    """Three boxes on a floor and two glTF cameras: "Front", a perspective camera (yfov 0.7 rad,
    aspect ratio 400:480, znear 0.1, zfar 50) placed in front and tilted down 12 degrees, and
    "Top", an orthographic camera (xmag 3, ymag 3.6) looking straight down."""
    doc = Document()
    parts = [
        ("Floor", (-2.5, -0.1, -2.5), (2.5, 0.0, 2.5), (0.4, 0.42, 0.45)),
        ("RedBox", (-1.8, 0.0, -0.4), (-1.0, 0.8, 0.4), (0.9, 0.25, 0.2)),
        ("GreenBox", (-0.4, 0.0, -1.6), (0.4, 1.4, -0.8), (0.3, 0.8, 0.3)),
        ("BlueBox", (1.0, 0.0, 0.6), (1.8, 0.5, 1.4), (0.25, 0.4, 0.95)),
    ]
    for name, lo, hi, colour in parts:
        positions, colours, indices = box(lo, hi, colour)
        doc.gltf["meshes"].append({"name": name + "Mesh", "primitives": [{
            "attributes": {"POSITION": doc.accessor(positions, "VEC3", minmax=True),
                           "COLOR_0": doc.accessor(colours, "VEC4")},
            "indices": doc.accessor(indices, "SCALAR", UNSIGNED_SHORT, ELEMENT_ARRAY_BUFFER),
            "material": 0}]})
    tilt = math.radians(-12.0)
    down = math.radians(-90.0)
    doc.gltf["nodes"] = [{"name": name, "mesh": index} for index, (name, *_rest) in enumerate(parts)]
    doc.gltf["nodes"] += [
        # A glTF camera looks down its node's -Z with +Y up.
        {"name": "FrontCameraNode", "camera": 0, "translation": [0.0, 1.8, 6.0],
         "rotation": [math.sin(tilt / 2), 0.0, 0.0, math.cos(tilt / 2)]},
        {"name": "TopCameraNode", "camera": 1, "translation": [0.0, 8.0, 0.0],
         "rotation": [math.sin(down / 2), 0.0, 0.0, math.cos(down / 2)]},
    ]
    doc.gltf["cameras"] = [
        {"name": "Front", "type": "perspective",
         "perspective": {"yfov": 0.7, "aspectRatio": 400.0 / 480.0, "znear": 0.1, "zfar": 50.0}},
        {"name": "Top", "type": "orthographic",
         "orthographic": {"xmag": 3.0, "ymag": 3.6, "znear": 0.1, "zfar": 20.0}},
    ]
    doc.gltf["materials"] = [{"name": "BakedColours", "extensions": {"KHR_materials_unlit": {}}}]
    doc.gltf["extensionsUsed"] = ["KHR_materials_unlit"]
    doc.gltf["scenes"] = [{"name": "CameraRig", "nodes": list(range(len(doc.gltf["nodes"])))}]
    doc.gltf["scene"] = 0
    doc.save(path)


def crate_texture_png() -> bytes:
    """A 64x64 crate face: wooden planks with a darker frame and a diagonal brace."""
    import io
    from PIL import Image, ImageDraw
    image = Image.new("RGBA", (64, 64), (176, 122, 66, 255))
    draw = ImageDraw.Draw(image)
    for y in range(0, 64, 16):
        draw.line([(0, y), (63, y)], fill=(120, 80, 40, 255), width=1)
    draw.rectangle([0, 0, 63, 63], outline=(92, 58, 28, 255), width=6)
    draw.line([(6, 57), (57, 6)], fill=(92, 58, 28, 255), width=6)
    out = io.BytesIO()
    image.save(out, format="PNG", optimize=False)
    return out.getvalue()


def textured_crate(path: Path) -> None:
    """A cube with TEXCOORD_0 and a base-colour texture embedded in the file (a PNG in a buffer
    view), unlit: CNA's importer turns the image into a Texture2D the model owns and gives the
    part a textured BasicEffect."""
    doc = Document()
    positions, _colours, indices = box((-1.0, -1.0, -1.0), (1.0, 1.0, 1.0), (1.0, 1.0, 1.0))
    uvs = [(0.0, 1.0), (1.0, 1.0), (1.0, 0.0), (0.0, 0.0)] * 6  # each face's corners, glTF UV origin top-left
    doc.gltf["meshes"] = [{"name": "CrateMesh", "primitives": [{
        "attributes": {"POSITION": doc.accessor(positions, "VEC3", minmax=True),
                       "TEXCOORD_0": doc.accessor(uvs, "VEC2")},
        "indices": doc.accessor(indices, "SCALAR", UNSIGNED_SHORT, ELEMENT_ARRAY_BUFFER),
        "material": 0}]}]
    doc.gltf["nodes"] = [{"name": "Crate", "mesh": 0}]
    doc.gltf["images"] = [{"name": "CrateFace", "mimeType": "image/png", "bufferView": doc.view(crate_texture_png())}]
    doc.gltf["samplers"] = [{"magFilter": 9728, "minFilter": 9728}]  # NEAREST: crisp texels
    doc.gltf["textures"] = [{"source": 0, "sampler": 0}]
    doc.gltf["materials"] = [{"name": "Crate", "pbrMetallicRoughness": {"baseColorTexture": {"index": 0}},
                              "extensions": {"KHR_materials_unlit": {}}}]
    doc.gltf["extensionsUsed"] = ["KHR_materials_unlit"]
    doc.gltf["scenes"] = [{"name": "TexturedCrate", "nodes": [0]}]
    doc.gltf["scene"] = 0
    doc.save(path)


def sampler_quad(path: Path) -> None:
    """An unlit quad with out-of-range U and explicit nearest/clamp sampler metadata."""
    import io
    from PIL import Image
    doc = Document()
    positions = [(-.3, -.5, 0), (.3, -.5, 0), (.3, .5, 0), (-.3, .5, 0)]
    doc.gltf["meshes"] = [{"name": "SamplerQuad", "primitives": [{
        "attributes": {"POSITION": doc.accessor(positions, "VEC3", minmax=True),
                       "TEXCOORD_0": doc.accessor([(1.125, .5)] * 4, "VEC2")},
        "indices": doc.accessor([0, 1, 2, 0, 2, 3], "SCALAR", UNSIGNED_SHORT, ELEMENT_ARRAY_BUFFER),
        "material": 0}]}]
    image = Image.new("RGBA", (2, 1))
    image.putdata([(255, 0, 0, 255), (0, 255, 0, 255)])
    encoded = io.BytesIO()
    image.save(encoded, format="PNG", optimize=False)
    doc.gltf["images"] = [{"mimeType": "image/png", "bufferView": doc.view(encoded.getvalue())}]
    # glTF NEAREST (9728), CLAMP_TO_EDGE (33071), explicitly on both axes.
    doc.gltf["samplers"] = [{"magFilter": 9728, "minFilter": 9728, "wrapS": 33071, "wrapT": 33071}]
    doc.gltf["textures"] = [{"source": 0, "sampler": 0}]
    doc.gltf["materials"] = [{"name": "Clamp", "pbrMetallicRoughness": {"baseColorTexture": {"index": 0}},
                              "extensions": {"KHR_materials_unlit": {}}}]
    doc.gltf["extensionsUsed"] = ["KHR_materials_unlit"]
    doc.gltf["nodes"] = [{"name": "ClampQuad", "mesh": 0}]
    doc.gltf["scenes"] = [{"name": "Sampler", "nodes": [0]}]
    doc.gltf["scene"] = 0
    doc.save(path)


def slide_rig(path: Path) -> None:
    """Three boxes in rows sliding from x = -2 to 0 to 2 in two seconds, one clip "Slide" with one
    channel per box: the top box's sampler is STEP, the middle one's LINEAR, the bottom one's
    CUBICSPLINE with zero tangents (an ease in and out)."""
    doc = Document()
    rows = [("StepBox", 1.0, (0.9, 0.35, 0.3)), ("LinearBox", 0.0, (0.35, 0.8, 0.4)),
            ("CubicBox", -1.0, (0.35, 0.5, 0.95))]
    for name, _y, colour in rows:
        positions, colours, indices = box((-0.3, -0.3, -0.3), (0.3, 0.3, 0.3), colour)
        doc.gltf["meshes"].append({"name": name + "Mesh", "primitives": [{
            "attributes": {"POSITION": doc.accessor(positions, "VEC3", minmax=True),
                           "COLOR_0": doc.accessor(colours, "VEC4")},
            "indices": doc.accessor(indices, "SCALAR", UNSIGNED_SHORT, ELEMENT_ARRAY_BUFFER),
            "material": 0}]})
    doc.gltf["nodes"] = [{"name": name, "mesh": index, "translation": [-2.0, y, 0.0]}
                         for index, (name, y, _colour) in enumerate(rows)]
    times = [0.0, 1.0, 2.0]
    sampler_input = doc.accessor(times, "SCALAR", target=None)
    doc.gltf["accessors"][sampler_input].update({"min": [times[0]], "max": [times[-1]]})
    samplers, channels = [], []
    for index, (_name, y, _colour) in enumerate(rows):
        values = [(-2.0, y, 0.0), (0.0, y, 0.0), (2.0, y, 0.0)]
        interpolation = ["STEP", "LINEAR", "CUBICSPLINE"][index]
        if interpolation == "CUBICSPLINE":
            zero = (0.0, 0.0, 0.0)
            values = [v for value in values for v in (zero, value, zero)]  # in-tangent, value, out-tangent
        samplers.append({"input": sampler_input, "output": doc.accessor(values, "VEC3", target=None),
                         "interpolation": interpolation})
        channels.append({"sampler": index, "target": {"node": index, "path": "translation"}})
    doc.gltf["animations"] = [{"name": "Slide", "samplers": samplers, "channels": channels}]
    doc.gltf["materials"] = [{"name": "BakedColours", "extensions": {"KHR_materials_unlit": {}}}]
    doc.gltf["extensionsUsed"] = ["KHR_materials_unlit"]
    doc.gltf["scenes"] = [{"name": "SlideRig", "nodes": [0, 1, 2]}]
    doc.gltf["scene"] = 0
    doc.save(path)


def paint_toy(path: Path) -> None:
    """A toy car: a body and four wheels, unlit. The body's default material is red; the file
    declares two KHR_materials_variants, "Ocean" (blue body) and "Gold" (gold body). The wheels
    have no variant mapping, so they stay black whatever variant is chosen."""
    doc = Document()
    body_positions, _c, body_indices = box((-1.2, 0.3, -0.6), (1.2, 0.9, 0.6), (1.0, 1.0, 1.0))
    wheel_parts = []
    for x in (-0.75, 0.75):
        for z in (-0.65, 0.65):
            wheel_parts.append(box((x - 0.25, 0.0, z - 0.1), (x + 0.25, 0.5, z + 0.1), (1.0, 1.0, 1.0)))
    doc.gltf["meshes"] = [{"name": "BodyMesh", "primitives": [{
        "attributes": {"POSITION": doc.accessor(body_positions, "VEC3", minmax=True)},
        "indices": doc.accessor(body_indices, "SCALAR", UNSIGNED_SHORT, ELEMENT_ARRAY_BUFFER),
        "material": 0,
        "extensions": {"KHR_materials_variants": {"mappings": [
            {"material": 1, "variants": [0]}, {"material": 2, "variants": [1]}]}}}]}]
    positions, indices = [], []
    for wheel_positions, _colours, wheel_indices in wheel_parts:
        base = len(positions)
        positions += wheel_positions
        indices += [base + i for i in wheel_indices]
    doc.gltf["meshes"].append({"name": "WheelsMesh", "primitives": [{
        "attributes": {"POSITION": doc.accessor(positions, "VEC3", minmax=True)},
        "indices": doc.accessor(indices, "SCALAR", UNSIGNED_SHORT, ELEMENT_ARRAY_BUFFER),
        "material": 3}]})
    doc.gltf["nodes"] = [{"name": "Body", "mesh": 0}, {"name": "Wheels", "mesh": 1}]

    def paint(name, rgb):
        return {"name": name, "pbrMetallicRoughness": {"baseColorFactor": [*rgb, 1.0]},
                "extensions": {"KHR_materials_unlit": {}}}

    doc.gltf["materials"] = [paint("Red", (0.85, 0.12, 0.1)), paint("Blue", (0.1, 0.3, 0.9)),
                             paint("Gold", (0.95, 0.75, 0.15)), paint("Rubber", (0.05, 0.05, 0.05))]
    doc.gltf["extensions"] = {"KHR_materials_variants": {"variants": [{"name": "Ocean"}, {"name": "Gold"}]}}
    doc.gltf["extensionsUsed"] = ["KHR_materials_unlit", "KHR_materials_variants"]
    doc.gltf["scenes"] = [{"name": "PaintToy", "nodes": [0, 1]}]
    doc.gltf["scene"] = 0
    doc.save(path)


def painted_column(path: Path) -> None:
    """The bending strip's rig and "Bend" clip on a square column (0.5 wide, 4 high, 8 segments,
    flat-shaded sides) with an orange painted (dielectric) metallic-roughness material and no unlit
    extension: skinned + PBR is what CNA maps to its SkinnedPbrEffect. No lights in the file, so the
    importer enables the default lighting. (A metal would look dark here: with no environment to
    reflect, it shows only its highlights.)"""
    doc = Document()
    positions, normals, joints, weights, indices = [], [], [], [], []
    segments = 8
    # Outward normal and the direction to the right as seen from outside, per side.
    sides = [((0.0, 0.0, 1.0), (1.0, 0.0, 0.0)), ((1.0, 0.0, 0.0), (0.0, 0.0, -1.0)),
             ((0.0, 0.0, -1.0), (-1.0, 0.0, 0.0)), ((-1.0, 0.0, 0.0), (0.0, 0.0, 1.0))]
    for normal, right in sides:
        first = len(positions)
        for row in range(segments + 1):
            y = 4.0 * row / segments
            tip = min(1.0, max(0.0, (y - 1.5) / 1.0))
            for side in (-1.0, 1.0):
                positions.append((0.25 * normal[0] + 0.25 * side * right[0], y,
                                  0.25 * normal[2] + 0.25 * side * right[2]))
                normals.append(normal)
                joints.append((0, 1, 0, 0))
                weights.append((1.0 - tip, tip, 0.0, 0.0))
        for row in range(segments):
            a = first + row * 2
            indices += [a, a + 1, a + 3, a, a + 3, a + 2]  # counter-clockwise from outside (glTF)
    mesh = {"name": "ColumnMesh", "primitives": [{
        "attributes": {"POSITION": doc.accessor(positions, "VEC3", minmax=True),
                       "NORMAL": doc.accessor(normals, "VEC3"),
                       "JOINTS_0": doc.accessor(joints, "VEC4", UNSIGNED_BYTE),
                       "WEIGHTS_0": doc.accessor(weights, "VEC4")},
        "indices": doc.accessor(indices, "SCALAR", UNSIGNED_SHORT, ELEMENT_ARRAY_BUFFER),
        "material": 0}]}
    doc.gltf["materials"] = [{"name": "OrangePaint", "pbrMetallicRoughness": {
        "baseColorFactor": [0.9, 0.42, 0.12, 1.0], "metallicFactor": 0.0, "roughnessFactor": 0.5}}]
    doc.gltf["meshes"].append(mesh)
    doc.gltf["nodes"] = [
        {"name": "Column", "mesh": 0, "skin": 0},
        {"name": "LowerJoint", "children": [2]},
        {"name": "UpperJoint", "translation": [0.0, 2.0, 0.0]},
    ]
    identity = [1.0, 0, 0, 0, 0, 1.0, 0, 0, 0, 0, 1.0, 0, 0, 0, 0, 1.0]
    tip_inverse = [1.0, 0, 0, 0, 0, 1.0, 0, 0, 0, 0, 1.0, 0, 0, -2.0, 0, 1.0]
    doc.gltf["skins"] = [{"name": "ColumnRig", "joints": [1, 2], "skeleton": 1,
                          "inverseBindMatrices": doc.accessor([identity, tip_inverse], "MAT4", target=None)}]
    times = [0.0, 0.5, 1.0, 1.5, 2.0]
    angles = [0.0, 50.0, 0.0, -50.0, 0.0]
    rotations = [(0.0, 0.0, math.sin(math.radians(a) / 2), math.cos(math.radians(a) / 2)) for a in angles]
    doc.gltf["animations"] = [{"name": "Bend",
                               "samplers": [{"input": doc.accessor(times, "SCALAR", target=None, minmax=False),
                                             "output": doc.accessor(rotations, "VEC4", target=None),
                                             "interpolation": "LINEAR"}],
                               "channels": [{"sampler": 0, "target": {"node": 2, "path": "rotation"}}]}]
    doc.gltf["accessors"][doc.gltf["animations"][0]["samplers"][0]["input"]].update({"min": [0.0], "max": [2.0]})
    doc.gltf["scenes"] = [{"name": "PaintedColumn", "nodes": [0, 1]}]
    doc.gltf["scene"] = 0
    doc.save(path)


def lit_sphere(path: Path) -> None:
    """A white sphere with a metallic-roughness material (so CNA gives it a PbrEffect) and two
    KHR_lights_punctual directional lights: "Warm" (1, 0.35, 0.2) shining from the left (towards
    +X) and "Cool" (0.2, 0.45, 1) from the right (towards -X). A directional light shines down its
    node's -Z."""
    doc = Document()
    rings, segments = 24, 48
    positions, normals, indices = [], [], []
    for ring in range(rings + 1):
        phi = math.pi * ring / rings
        for segment in range(segments + 1):
            theta = 2.0 * math.pi * segment / segments
            n = (math.sin(phi) * math.cos(theta), math.cos(phi), math.sin(phi) * math.sin(theta))
            normals.append(n)
            positions.append(n)
    for ring in range(rings):
        for segment in range(segments):
            a = ring * (segments + 1) + segment
            b = a + segments + 1
            indices += [a, a + 1, b, a + 1, b + 1, b]
    doc.gltf["meshes"] = [{"name": "SphereMesh", "primitives": [{
        "attributes": {"POSITION": doc.accessor(positions, "VEC3", minmax=True),
                       "NORMAL": doc.accessor(normals, "VEC3")},
        "indices": doc.accessor(indices, "SCALAR", UNSIGNED_SHORT, ELEMENT_ARRAY_BUFFER),
        "material": 0}]}]
    def about_y(degrees):
        half = math.radians(degrees) / 2.0
        return [0.0, math.sin(half), 0.0, math.cos(half)]
    doc.gltf["nodes"] = [
        {"name": "Sphere", "mesh": 0},
        {"name": "WarmLight", "rotation": about_y(-90.0), "extensions": {"KHR_lights_punctual": {"light": 0}}},
        {"name": "CoolLight", "rotation": about_y(90.0), "extensions": {"KHR_lights_punctual": {"light": 1}}},
    ]
    doc.gltf["extensions"] = {"KHR_lights_punctual": {"lights": [
        {"name": "Warm", "type": "directional", "color": [1.0, 0.35, 0.2], "intensity": 1.0},
        {"name": "Cool", "type": "directional", "color": [0.2, 0.45, 1.0], "intensity": 1.0},
    ]}}
    doc.gltf["materials"] = [{"name": "White", "pbrMetallicRoughness": {
        "baseColorFactor": [1.0, 1.0, 1.0, 1.0], "metallicFactor": 0.0, "roughnessFactor": 0.7}}]
    doc.gltf["extensionsUsed"] = ["KHR_lights_punctual"]
    doc.gltf["scenes"] = [{"name": "LitSphere", "nodes": [0, 1, 2]}]
    doc.gltf["scene"] = 0
    doc.save(path)


def multiple_skins(path: Path) -> None:
    doc = Document()
    positions = [(-.2, -.15, .5), (.2, -.15, .5), (.2, .15, .5), (-.2, .15, .5)]
    attributes = {"POSITION": doc.accessor(positions, "VEC3", minmax=True),
                  "NORMAL": doc.accessor([(0, 0, 1)] * 4, "VEC3"),
                  "TEXCOORD_0": doc.accessor([(.5, .5)] * 4, "VEC2"),
                  "JOINTS_0": doc.accessor([(0, 0, 0, 0)] * 4, "VEC4", UNSIGNED_BYTE),
                  "WEIGHTS_0": doc.accessor([(1, 0, 0, 0)] * 4, "VEC4")}
    doc.gltf["meshes"] = [{"name": "SharedPatch", "primitives": [{"attributes": attributes,
                           "indices": doc.accessor([0, 1, 2, 0, 2, 3], "SCALAR", UNSIGNED_SHORT, ELEMENT_ARRAY_BUFFER),
                           "material": 0}]}]
    doc.gltf["materials"] = [{"name": "SharedWhite", "extensions": {"KHR_materials_unlit": {}}}]
    doc.gltf["extensionsUsed"] = ["KHR_materials_unlit"]
    identity = [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]
    inverse = doc.accessor([identity], "MAT4", target=None)
    # Identity inverse binds leave the quad in joint space; the joint scene ancestors place it.
    doc.gltf["skins"] = [{"name": "LeftRig", "joints": [1], "skeleton": 1, "inverseBindMatrices": inverse},
                         {"name": "RightRig", "joints": [3], "skeleton": 3, "inverseBindMatrices": inverse}]
    doc.gltf["nodes"] = [{"name": "LeftScene", "translation": [-.5, 0, 0], "children": [1]},
                         {"name": "LeftJoint"},
                         {"name": "RightScene", "translation": [.5, 0, 0], "children": [3]},
                         {"name": "RightJoint"},
                         {"name": "LeftMesh", "mesh": 0, "skin": 0, "translation": [4, 0, 0]},
                         {"name": "RightMesh", "mesh": 0, "skin": 1, "translation": [-4, 0, 0]}]
    doc.gltf["scenes"] = [{"name": "IndependentSkins", "nodes": [0, 2, 4, 5]}]
    doc.gltf["scene"] = 0
    doc.save(path)


def imported_topology(path: Path) -> None:
    doc = Document()
    shapes = [
        ('Loop', 2, [(-.9, -.3, .5), (-.5, -.3, .5), (-.5, .3, .5), (-.9, .3, .5)],
         [0, 1, 2, 3], (1, 215/255, 0, 1)),
        ('Strip', 5, [(-.25, -.3, .5), (.25, -.3, .5), (-.25, .3, .5), (.25, .3, .5)],
         [0, 1, 2, 3], (1, 0, 0, 1)),
        ('Fan', 6, [(.45, -.3, .5), (.9, -.3, .5), (.9, .1, .5), (.7, .35, .5), (.45, .1, .5)],
         [0, 1, 2, 3, 4], (0, 1, 0, 1)),
        ('Points', 0, [(-.8, .65, .5), (-.65, .55, .5), (-.5, .7, .5)],
         [0, 1, 2], (0, 1, 1, 1)),
        ('OpenLineStrip', 3, [(.25, .45, .5), (.34, .75, .5), (.43, .45, .5)],
         [0, 1, 2], (0, 0, 1, 1)),
        # The last index in each following run deliberately cannot make a whole primitive. CNA
        # drops and reports the line-list singleton and triangle-list singleton independently.
        ('IncompleteLines', 1, [(-.2, .55, .5), (.2, .55, .5), (.2, .75, .5)],
         [0, 1, 2], (1, 0, 1, 1)),
        ('IncompleteTriangles', 4, [(.45, .45, .5), (.9, .45, .5), (.675, .8, .5), (.675, .62, .5)],
         [0, 1, 2, 3], (1, .5, 0, 1)),
    ]
    doc.gltf['materials'] = []
    for i, (name, mode, points, indices, color) in enumerate(shapes):
        attrs = {'POSITION': doc.accessor(points, 'VEC3', minmax=True),
                 'COLOR_0': doc.accessor([color] * len(points), 'VEC4')}
        primitive = {'attributes': attrs, 'mode': mode, 'material': i,
                     'indices': doc.accessor(indices, 'SCALAR', UNSIGNED_SHORT, ELEMENT_ARRAY_BUFFER)}
        doc.gltf['meshes'].append({'name': name, 'primitives': [primitive]})
        doc.gltf['nodes'].append({'name': name, 'mesh': i})
        doc.gltf['materials'].append({'name': name, 'extensions': {'KHR_materials_unlit': {}}})
    doc.gltf['extensionsUsed'] = ['KHR_materials_unlit']
    doc.gltf['scenes'] = [{'name': 'Imported topology', 'nodes': list(range(len(shapes)))}]
    doc.gltf['scene'] = 0
    doc.save(path)


def write_all(content_root: Path, assets_root: Path) -> None:
    robot_arm(content_root / "models" / "robot_arm.gltf")
    bending_strip(content_root / "models" / "bending_strip.gltf")
    turntable(content_root / "models" / "turntable.gltf")
    morph_flag(content_root / "models" / "morph_flag.gltf")
    camera_rig(content_root / "models" / "camera_rig.gltf")
    textured_crate(content_root / "models" / "textured_crate.gltf")
    lit_sphere(content_root / "models" / "lit_sphere.gltf")
    slide_rig(content_root / "models" / "slide_rig.gltf")
    painted_column(content_root / "models" / "painted_column.gltf")
    # The same arm as a loose file: CNA can also load glTF at run time, without the pipeline.
    robot_arm(assets_root / "models" / "loose_robot_arm.gltf")
    # Loose copies of the camera rig and the textured crate: the compiled models lose the cameras
    # (CNA-DEMOS-ISSUE-023) and the embedded texture (CNA-DEMOS-ISSUE-022), the run-time import keeps both.
    camera_rig(assets_root / "models" / "loose_camera_rig.gltf")
    textured_crate(assets_root / "models" / "loose_textured_crate.gltf")
    # Material variants only as a loose file: CNA's content pipeline refuses them (the compiled
    # model format cannot carry them).
    paint_toy(assets_root / "models" / "loose_paint_toy.gltf")
    sampler_quad(assets_root / "models" / "loose_sampler_clamp.gltf")
    multiple_skins(assets_root / "models" / "loose_multi_skin.gltf")
    imported_topology(assets_root / "models" / "loose_topology.gltf")
