// SPDX-License-Identifier: MS-PL
// Run against the original Microsoft XNA 4.0 assembly, not a replacement.
using System;
using Microsoft.Xna.Framework;

static class FrustumRayOracle
{
    static int checks;

    static void Check(string name, BoundingFrustum frustum, Ray ray, float? expected)
    {
        float? output;
        frustum.Intersects(ref ray, out output);
        float?[] actual = { frustum.Intersects(ray), output, ray.Intersects(frustum) };
        foreach (float? value in actual)
        {
            ++checks;
            if (value != expected)
                throw new Exception(name + ": expected " + expected + ", got " + value);
        }
        Console.WriteLine("PASS " + name + ": " + (expected.HasValue ? expected.Value.ToString("R") : "null"));
    }

    static int Main()
    {
        Console.WriteLine(typeof(BoundingFrustum).Assembly.FullName);
        Console.WriteLine(typeof(BoundingFrustum).Assembly.Location);
        if (typeof(BoundingFrustum).Assembly.GetName().Version != new Version(4, 0, 0, 0))
            throw new Exception("Not XNA 4.0");
        var f = new BoundingFrustum(Matrix.Identity);
        Check("near entry", f, new Ray(new Vector3(0, 0, -3), Vector3.UnitZ), 3);
        Check("far entry", f, new Ray(new Vector3(0, 0, 4), -Vector3.UnitZ), 3);
        Check("left entry", f, new Ray(new Vector3(-4, 0, .5f), Vector3.UnitX), 3);
        Check("right entry", f, new Ray(new Vector3(4, 0, .5f), -Vector3.UnitX), 3);
        Check("bottom entry", f, new Ray(new Vector3(0, -4, .5f), Vector3.UnitY), 3);
        Check("top entry", f, new Ray(new Vector3(0, 4, .5f), -Vector3.UnitY), 3);
        Check("nonunit direction", f, new Ray(new Vector3(0, 0, -3), new Vector3(0, 0, 2)), 1.5f);
        Check("behind ray", f, new Ray(new Vector3(0, 0, -3), -Vector3.UnitZ), null);
        Check("parallel outside", f, new Ray(new Vector3(3, 0, -3), Vector3.UnitZ), null);
        Check("disjoint intervals", f, new Ray(new Vector3(3, 0, -3), new Vector3(-1, 0, 3)), null);
        Check("edge tangent", f, new Ray(new Vector3(1, 1, -3), Vector3.UnitZ), 3);
        Check("corner contact", f, new Ray(new Vector3(2, 0, -1), new Vector3(-1, 1, 1)), 1);
        Check("boundary inward", f, new Ray(Vector3.Zero, Vector3.UnitZ), 0);
        Check("boundary outward", f, new Ray(Vector3.Zero, -Vector3.UnitZ), 0);
        Check("boundary stationary", f, new Ray(Vector3.Zero, Vector3.Zero), 0);
        Check("inside stationary", f, new Ray(new Vector3(0, 0, .5f), Vector3.Zero), 0);
        Check("outside stationary", f, new Ray(new Vector3(0, 0, -3), Vector3.Zero), null);
        Check("containment tolerance", f, new Ray(new Vector3(0, 0, -1e-6f), -Vector3.UnitZ), 0);
        Check("beyond containment tolerance", f, new Ray(new Vector3(0, 0, -1e-3f), Vector3.UnitZ), 1e-3f);
        Check("direction below parallel epsilon", f, new Ray(new Vector3(0, 0, -1), new Vector3(0, 0, 5e-6f)), null);
        Check("direction at parallel epsilon", f, new Ray(new Vector3(0, 0, -1), new Vector3(0, 0, 1e-5f)), 100000);
        Check("parallel offset beyond plane but inside tolerance", f,
              new Ray(new Vector3(1 + 1e-6f, 0, -3), Vector3.UnitZ), null);
        var moved = new BoundingFrustum(Matrix.CreateTranslation(-10, 20, -30));
        Check("translated entry", moved, new Ray(new Vector3(10, -20, 27), Vector3.UnitZ), 3);
        var ortho = new BoundingFrustum(Matrix.CreateLookAt(new Vector3(0, 0, 5),
                          Vector3.Zero, Vector3.Up) * Matrix.CreateOrthographic(2, 2, 1, 5));
        Check("orthographic camera entry", ortho, new Ray(new Vector3(0, 0, 7), -Vector3.UnitZ), 3);
        Console.WriteLine("PASS " + checks + " checks");
        return 0;
    }
}
