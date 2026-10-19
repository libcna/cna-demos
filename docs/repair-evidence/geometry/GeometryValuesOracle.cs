// SPDX-License-Identifier: MS-PL
using System;
using System.Globalization;
using Microsoft.Xna.Framework;

static class GeometryValuesOracle
{
    static int checks, failed;
    static void Check(bool value, string name) { ++checks; if (!value) { ++failed; Console.WriteLine("FAIL " + name); } }
    static float NegativeZero() { return BitConverter.ToSingle(BitConverter.GetBytes(unchecked((int)0x80000000)),0); }
    static int Bits(float value) { return BitConverter.ToInt32(BitConverter.GetBytes(value), 0); }
    static void Same(float actual, float expected, string name)
    {
        bool same = float.IsNaN(expected) ? float.IsNaN(actual) : Bits(actual) == Bits(expected);
        if (!same) Console.WriteLine(name+" actual="+Bits(actual).ToString("X8")+" expected="+Bits(expected).ToString("X8"));
        Check(same, name);
    }
    static void Hit(Ray ray, BoundingBox box, float? expected)
    {
        float? a = ray.Intersects(box), b, c = box.Intersects(ray), d;
        ray.Intersects(ref box, out b); box.Intersects(ref ray, out d);
        foreach (float? value in new float?[] {a,b,c,d}) {
            Check(value.HasValue == expected.HasValue, "ray/box presence");
            if (value.HasValue && expected.HasValue) Same(value.Value, expected.Value, "ray/box parameter");
        }
    }
    static int Main()
    {
        System.Threading.Thread.CurrentThread.CurrentCulture = CultureInfo.InvariantCulture;
        foreach (float radius in new float[] {-1, -float.Epsilon, float.NegativeInfinity}) {
            try { new BoundingSphere(Vector3.Zero, radius); Check(false, "negative radius accepted"); }
            catch (Exception e) { Check(e.GetType() == typeof(ArgumentException), "negative radius exception"); }
        }
        foreach (float radius in new float[] {0, NegativeZero(), 1, float.MaxValue, float.PositiveInfinity, float.NaN}) {
            BoundingSphere s = new BoundingSphere(new Vector3(1,2,3), radius);
            Same(s.Radius, radius, "accepted radius bits"); Check(s.Center == new Vector3(1,2,3), "center preserved");
        }
        try { BoundingSphere.CreateFromPoints(new Vector3[0]); Check(false, "empty points accepted"); }
        catch (Exception e) { Check(e.GetType() == typeof(ArgumentException), "empty point exception"); }
        BoundingSphere singleton = BoundingSphere.CreateFromPoints(new Vector3[] {new Vector3(3,-4,5)});
        Check(singleton.Center == new Vector3(3,-4,5), "singleton center"); Same(singleton.Radius, 0, "singleton radius");
        BoundingSphere mutated = new BoundingSphere(); mutated.Center = new Vector3(1,2,3); mutated.Radius = -1;
        BoundingSphere transformed = mutated.Transform(Matrix.Identity);
        Check(transformed.Center == mutated.Center && transformed.Radius == -1, "mutable radius transform");
        Check(mutated.ToString() == "{Center:{X:1 Y:2 Z:3} Radius:-1}", "mutable radius text");

        BoundingBox box = new BoundingBox(new Vector3(4,-1,-1), new Vector3(6,1,1));
        foreach (float x in new float[] {3,4,5,6,7}) foreach (float y in new float[] {-2,-1,0,1,2})
        foreach (float z in new float[] {-2,-1,0,1,2}) {
            bool inside = x >= 4 && x <= 6 && Math.Abs(y) <= 1 && Math.Abs(z) <= 1;
            Hit(new Ray(new Vector3(x,y,z), Vector3.Zero), box, inside ? (float?)0 : null);
            Hit(new Ray(new Vector3(x,y,z), new Vector3(NegativeZero())), box, inside ? (float?)0 : null);
        }
        Hit(new Ray(Vector3.Zero, Vector3.UnitX), box, 4);
        Hit(new Ray(Vector3.Zero, new Vector3(2,0,0)), box, 2);
        Hit(new Ray(new Vector3(4,0,0), -Vector3.UnitX), box, 0);
        Hit(new Ray(new Vector3(6,0,0), Vector3.UnitX), box, 0);
        Hit(new Ray(new Vector3(5,0,0), -Vector3.UnitX), box, 0);
        Hit(new Ray(Vector3.Zero, -Vector3.UnitX), box, null);
        Hit(new Ray(Vector3.Zero, new Vector3(0.5e-6f,0,0)), box, null);
        Hit(new Ray(new Vector3(5,0,0), new Vector3(0.5e-6f,0,0)), box, 0);
        Hit(new Ray(Vector3.Zero, new Vector3(1e-6f,0,0)), box, 4000000);
        Hit(new Ray(new Vector3(0,2,0), Vector3.UnitX), box, null);

        Check(new BoundingBox(new Vector3(-3,-1,-6), new Vector3(2,4,2)).ToString() ==
            "{Min:{X:-3 Y:-1 Z:-6} Max:{X:2 Y:4 Z:2}}", "box braces");
        Check(new Ray(new Vector3(-1,2,-3), new Vector3(4,-5,6)).ToString() ==
            "{Position:{X:-1 Y:2 Z:-3} Direction:{X:4 Y:-5 Z:6}}", "ray braces");

        BoundingFrustum frustum = new BoundingFrustum(Matrix.Identity);
        Plane[] planes = {frustum.Near, frustum.Far, frustum.Left, frustum.Right, frustum.Top, frustum.Bottom};
        float[][] expected = {new float[]{NegativeZero(),NegativeZero(),-1,NegativeZero()}, new float[]{0,0,1,-1}, new float[]{-1,NegativeZero(),NegativeZero(),-1},
            new float[]{1,0,0,-1}, new float[]{0,1,0,-1}, new float[]{NegativeZero(),-1,NegativeZero(),-1}};
        for (int i=0;i<6;++i) {
            Same(planes[i].Normal.X, expected[i][0], "frustum plane X bits");
            Same(planes[i].Normal.Y, expected[i][1], "frustum plane Y bits");
            Same(planes[i].Normal.Z, expected[i][2], "frustum plane Z bits");
            Same(planes[i].D, expected[i][3], "frustum plane D bits");
        }
        Check(frustum.ToString() == "{Near:{Normal:{X:0 Y:0 Z:-1} D:0} Far:{Normal:{X:0 Y:0 Z:1} D:-1} " +
            "Left:{Normal:{X:-1 Y:0 Z:0} D:-1} Right:{Normal:{X:1 Y:0 Z:0} D:-1} " +
            "Top:{Normal:{X:0 Y:1 Z:0} D:-1} Bottom:{Normal:{X:0 Y:-1 Z:0} D:-1}}", "frustum text");
        float nz = BitConverter.ToSingle(BitConverter.GetBytes(unchecked((int)0x80000000)),0);
        Check(new Vector2(nz).ToString() == Vector2.Zero.ToString(), "Vector2 negative zero text");
        Check(new Vector3(nz).ToString() == Vector3.Zero.ToString(), "Vector3 negative zero text");
        Check(new Vector4(nz).ToString() == Vector4.Zero.ToString(), "Vector4 negative zero text");
        Check(new Quaternion(nz,nz,nz,nz).ToString() == new Quaternion().ToString(), "Quaternion negative zero text");
        Check(new Plane(nz,nz,nz,nz).ToString() == new Plane().ToString(), "Plane negative zero text");
        Check(new BoundingSphere(new Vector3(nz),nz).ToString() == new BoundingSphere().ToString(), "Sphere negative zero text");
        Matrix negative = new Matrix(nz,nz,nz,nz,nz,nz,nz,nz,nz,nz,nz,nz,nz,nz,nz,nz);
        Check(negative.ToString() == new Matrix().ToString(), "Matrix negative zero text");
        Console.WriteLine("checks="+checks+" failed="+failed); return failed == 0 ? 0 : 1;
    }
}
