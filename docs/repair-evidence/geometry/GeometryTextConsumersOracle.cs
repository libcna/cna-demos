// SPDX-License-Identifier: MS-PL
using System;
using System.Globalization;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Design;
using Microsoft.Xna.Framework.Graphics;

static class GeometryTextConsumersOracle
{
    static int checks, failed;
    static void Check(string name, string actual, string expected)
    {
        ++checks;
        if (actual != expected) { ++failed; Console.WriteLine("FAIL "+name+" actual="+actual+" expected="+expected); }
    }
    static int Main()
    {
        System.Threading.Thread.CurrentThread.CurrentCulture = CultureInfo.InvariantCulture;
        Check("box converter", new BoundingBoxConverter().ConvertToInvariantString(
            new BoundingBox(new Vector3(-1),Vector3.One)), "{Min:{X:-1 Y:-1 Z:-1} Max:{X:1 Y:1 Z:1}}");
        Check("ray converter", new RayConverter().ConvertToInvariantString(
            new Ray(Vector3.Zero,Vector3.Forward)), "{Position:{X:0 Y:0 Z:0} Direction:{X:0 Y:0 Z:-1}}");
        Vector3 p = new Vector3(1,2,3), n = Vector3.Up;
        Vector2 uv = new Vector2(4,5);
        Color color = new Color(10,20,30,40);
        Check("VertexPositionColor", new VertexPositionColor(p,color).ToString(),
            "{Position:{X:1 Y:2 Z:3} Color:{R:10 G:20 B:30 A:40}}");
        Check("VertexPositionColorTexture", new VertexPositionColorTexture(p,color,uv).ToString(),
            "{Position:{X:1 Y:2 Z:3} Color:{R:10 G:20 B:30 A:40} TextureCoordinate:{X:4 Y:5}}");
        Check("VertexPositionNormalTexture", new VertexPositionNormalTexture(p,n,uv).ToString(),
            "{Position:{X:1 Y:2 Z:3} Normal:{X:0 Y:1 Z:0} TextureCoordinate:{X:4 Y:5}}");
        Check("VertexPositionTexture", new VertexPositionTexture(p,uv).ToString(),
            "{Position:{X:1 Y:2 Z:3} TextureCoordinate:{X:4 Y:5}}");
        Console.WriteLine("checks="+checks+" failed="+failed); return failed == 0 ? 0 : 1;
    }
}
