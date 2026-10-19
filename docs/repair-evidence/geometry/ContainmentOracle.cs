// Genuine XNA 4.0 probe for the NEXT batch (127/129/133), not a geometry-values closure.
using System;
using Microsoft.Xna.Framework;

static class ContainmentOracle
{
    static int checks, failed;
    static void Check(bool value, string name)
    {
        ++checks;
        if (!value) { ++failed; Console.WriteLine("FAIL " + name); }
    }
    static void Point(BoundingSphere sphere, Vector3 point, ContainmentType expected)
    {
        ContainmentType output;
        sphere.Contains(ref point, out output);
        Check(sphere.Contains(point) == expected, "sphere point value " + point);
        Check(output == expected, "sphere point output " + point);
    }
    static void Spheres(BoundingSphere outer, BoundingSphere inner, ContainmentType expected, bool intersects)
    {
        ContainmentType output; bool hit;
        outer.Contains(ref inner, out output);
        outer.Intersects(ref inner, out hit);
        Check(outer.Contains(inner) == expected, "sphere containment value");
        Check(output == expected, "sphere containment output");
        Check(outer.Intersects(inner) == intersects, "sphere intersection value");
        Check(hit == intersects, "sphere intersection output");
    }
    static void FrustumPoint(BoundingFrustum frustum, Vector3 point, ContainmentType expected)
    {
        ContainmentType output;
        frustum.Contains(ref point, out output);
        Check(frustum.Contains(point) == expected, "frustum point value " + point);
        Check(output == expected, "frustum point output " + point);
    }
    static int Main()
    {
        var sphere = new BoundingSphere(Vector3.Zero, 5);
        foreach (float x in new float[] {-6,-5,-4,0,4,5,6,float.NaN,float.PositiveInfinity})
            Point(sphere, new Vector3(x,0,0), Math.Abs(x) < 5 ? ContainmentType.Contains : ContainmentType.Disjoint);
        Point(new BoundingSphere(Vector3.Zero,0),Vector3.Zero,ContainmentType.Disjoint);
        Point(new BoundingSphere(Vector3.Zero,float.NaN),Vector3.Zero,ContainmentType.Disjoint);
        foreach (float r in new float[] {0,1,2,5,6,10}) foreach (float x in new float[] {0,1,3,5,6,7,10,15,16})
        {
            var expected = x > 5+r ? ContainmentType.Disjoint : x <= 5-r ? ContainmentType.Contains : ContainmentType.Intersects;
            Spheres(sphere,new BoundingSphere(new Vector3(x,0,0),r),expected,x < 5+r);
        }
        Spheres(sphere,new BoundingSphere(Vector3.Zero,float.NaN),ContainmentType.Disjoint,false);
        // A containing volume includes its corners even though Contains(point)
        // excludes the exact spherical surface. Do not implement one via the other.
        var flatBox = new BoundingBox(new Vector3(-3,-4,0),new Vector3(3,4,0));
        Check(sphere.Contains(flatBox)==ContainmentType.Contains,"sphere includes exact box corners");
        ContainmentType boxResult; sphere.Contains(ref flatBox,out boxResult);
        Check(boxResult==ContainmentType.Contains,"sphere includes exact box corners output");

        var f = new BoundingFrustum(Matrix.Identity);
        foreach (float x in new float[] {-2,-1,0,1,2}) foreach (float y in new float[] {-2,-1,0,1,2})
        foreach (float z in new float[] {-1,0,0.5f,1,2})
            FrustumPoint(f,new Vector3(x,y,z),Math.Abs(x)<=1 && Math.Abs(y)<=1 && z>=0 && z<=1 ? ContainmentType.Contains : ContainmentType.Disjoint);
        FrustumPoint(f,new Vector3(1+0.5e-5f,0,0.5f),ContainmentType.Contains);
        FrustumPoint(f,new Vector3(1+2e-5f,0,0.5f),ContainmentType.Disjoint);
        Check(f.Contains(f)==ContainmentType.Contains,"frustum self");
        Check(f.Contains(new BoundingFrustum(Matrix.Identity))==ContainmentType.Contains,"frustum equal geometry");
        var matchingBox=new BoundingBox(new Vector3(-1,-1,0),new Vector3(1,1,1));
        Check(matchingBox.Contains(f)==ContainmentType.Contains,"box includes frustum boundary");
        Check(new BoundingBox(new Vector3(10),new Vector3(11)).Contains(f)==ContainmentType.Disjoint,"distant box frustum");
        Check(new BoundingSphere(new Vector3(10),1).Contains(f)==ContainmentType.Disjoint,"distant sphere frustum");
        // Identity frustum corners are exactly distance1.5 from this center.
        Check(new BoundingSphere(new Vector3(0,0,0.5f),1.5f).Contains(f)==ContainmentType.Contains,"sphere includes exact frustum corners");
        var cornerMiss=new BoundingSphere(new Vector3(1.8f,1.8f,0.5f),1);
        Check(f.Contains(cornerMiss)==ContainmentType.Intersects,"plane containment is conservative at corner");
        Check(!f.Intersects(cornerMiss),"frustum intersection separates corner miss");
        Check(cornerMiss.Contains(f)==ContainmentType.Disjoint,"sphere contains uses actual frustum separation");
        var points=new Vector3[]{new Vector3(-2,0,0),new Vector3(2,0,0),new Vector3(0,1,0)};
        var fromPoints=BoundingSphere.CreateFromPoints(points);
        Check(fromPoints.Contains(points[0])==ContainmentType.Disjoint,"CreateFromPoints surface point is excluded by point API");
        Check(Vector3.DistanceSquared(fromPoints.Center,points[0])<=fromPoints.Radius*fromPoints.Radius,"CreateFromPoints still geometrically encloses point");
        Console.WriteLine("checks="+checks+" failed="+failed);
        return failed==0?0:1;
    }
}
