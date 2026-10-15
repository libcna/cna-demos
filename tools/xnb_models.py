# SPDX-License-Identifier: MS-PL
"""Original rigid .x model for the existing FORMAT xnb content target."""
from pathlib import Path


def write_all(content_xnb_root: Path, assets_root: Path | None = None) -> None:
    def matrix(x, y):
        return f"FrameTransformMatrix {{ 1,0,0,0, 0,1,0,0, 0,0,1,0, {x},{y},0,1;; }}"
    def panel(name, x, color):
        r, g, b = color
        return f"""  Frame {name}Mount {{
    {matrix(x, 0)}
    Mesh {name}Panel {{
      4;
      -0.25;-0.4;-0.5;,
       0.25;-0.4;-0.5;,
       0.25; 0.4;-0.5;,
      -0.25; 0.4;-0.5;;
      2;
      3;0,1,2;,
      3;0,2,3;;
      MeshNormals {{
        4;
        0;0;-1;,
        0;0;-1;,
        0;0;-1;,
        0;0;-1;;
        2;
        3;0,1,2;,
        3;0,2,3;;
      }}
      MeshMaterialList {{
        1;2;0,0;;
        Material {name}Color {{
          {r};{g};{b};1;;
          1;
          0;0;0;;
          0;0;0;;
        }}
      }}
    }}
  }}
"""
    source = "xof 0303txt 0032\n\nFrame PanelRig {\n  " + matrix(0, .1) + "\n"
    source += panel("Left", -.5, (1, 0, 0)) + panel("Right", .5, (0, 1, 0)) + "}\n"
    path = content_xnb_root / "models" / "twin_panels.x"
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(source)
    if assets_root is not None:
        demo_path = assets_root / "models" / "directx_twin_panels.x"
        demo_path.parent.mkdir(parents=True, exist_ok=True)
        demo_path.write_text(source)
