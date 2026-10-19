// Genuine XNA 4.0: existing ray consumers affected by the point-containment fix.
// Does not authorize implementing the owner-reserved CNA ray/frustum method.
using System;
using Microsoft.Xna.Framework;

static class FrustumBoundaryRayOracle
{
    static int Main()
    {
        int checks = 0, failed = 0;
        var frustum = new BoundingFrustum(Matrix.Identity);
        foreach (var origin in new[] {
            new Vector3(0,0,0), new Vector3(0,0,1),
            new Vector3(-1,0,0.5f), new Vector3(1,0,0.5f),
            new Vector3(0,-1,0.5f), new Vector3(0,1,0.5f),
            new Vector3(1,1,1), new Vector3(0,0,0.5f) })
        foreach (var direction in new[] { Vector3.Zero, Vector3.UnitZ, -Vector3.UnitZ, Vector3.One })
        {
            var ray = new Ray(origin, direction);
            float? output;
            frustum.Intersects(ref ray, out output);
            foreach (var actual in new[] { frustum.Intersects(ray), output, ray.Intersects(frustum) })
            {
                ++checks;
                if (!actual.HasValue || actual.Value != 0) {
                    ++failed;
                    Console.WriteLine("FAIL " + origin + " " + direction + " " + actual);
                }
            }
        }
        Console.WriteLine("Frustum boundary ray: " + checks + " checks, " + failed + " failures");
        return failed == 0 ? 0 : 1;
    }
}
