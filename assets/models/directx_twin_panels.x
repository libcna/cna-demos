xof 0303txt 0032

Frame PanelRig {
  FrameTransformMatrix { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0.1,0,1;; }
  Frame LeftMount {
    FrameTransformMatrix { 1,0,0,0, 0,1,0,0, 0,0,1,0, -0.5,0,0,1;; }
    Mesh LeftPanel {
      4;
      -0.25;-0.4;-0.5;,
       0.25;-0.4;-0.5;,
       0.25; 0.4;-0.5;,
      -0.25; 0.4;-0.5;;
      2;
      3;0,1,2;,
      3;0,2,3;;
      MeshNormals {
        4;
        0;0;-1;,
        0;0;-1;,
        0;0;-1;,
        0;0;-1;;
        2;
        3;0,1,2;,
        3;0,2,3;;
      }
      MeshMaterialList {
        1;2;0,0;;
        Material LeftColor {
          1;0;0;1;;
          1;
          0;0;0;;
          0;0;0;;
        }
      }
    }
  }
  Frame RightMount {
    FrameTransformMatrix { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0.5,0,0,1;; }
    Mesh RightPanel {
      4;
      -0.25;-0.4;-0.5;,
       0.25;-0.4;-0.5;,
       0.25; 0.4;-0.5;,
      -0.25; 0.4;-0.5;;
      2;
      3;0,1,2;,
      3;0,2,3;;
      MeshNormals {
        4;
        0;0;-1;,
        0;0;-1;,
        0;0;-1;,
        0;0;-1;;
        2;
        3;0,1,2;,
        3;0,2,3;;
      }
      MeshMaterialList {
        1;2;0,0;;
        Material RightColor {
          0;1;0;1;;
          1;
          0;0;0;;
          0;0;0;;
        }
      }
    }
  }
}
