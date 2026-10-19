// Measures the NEXT containment batch against installed genuine XNA 4.0.
// Each compact row records intersection, outer containment, reverse containment.
// It is evidence generation, not an oracle that assumes the three APIs agree.
using System;
using System.Text;
using Microsoft.Xna.Framework;

static class FrustumSeparationOracle
{
    static int checks, failed;
    static char Code(ContainmentType c) { return (char)('0'+(int)c); }
    static void Check(bool ok) { ++checks; if (!ok) ++failed; }
    static Matrix Frame(int kind)
    {
        Matrix m=Matrix.Identity;
        if(kind==1) { m.M11=2; m.M22=2; m.M33=2; m.M43=-0.5f; }
        if(kind==2) { m.M11=0.6f; m.M12=0.8f; m.M21=-0.8f; m.M22=0.6f; m.M41=0.25f; m.M42=-0.5f; }
        if(kind==3) { m.M33=-8.0f/7.0f; m.M34=-1; m.M43=-8.0f/7.0f; m.M44=0; }
        return m;
    }
    static int Main()
    {
        float[] xy={-3,-1.5f,-1,-0.5f,0,0.5f,1,1.5f,3};
        float[] zs={-3,-0.25f,0,0.5f,1,1.25f};
        for(int kind=0;kind<4;++kind)
        {
            var frustum=new BoundingFrustum(Frame(kind));
            var spheres=new StringBuilder(); var boxes=new StringBuilder();
            foreach(float x in xy) foreach(float y in xy) foreach(float z in zs)
            {
                var center=new Vector3(x,y,z);
                foreach(float radius in new float[]{0,0.25f,1,3})
                {
                    var sphere=new BoundingSphere(center,radius);
                    bool hit=frustum.Intersects(sphere), output;
                    frustum.Intersects(ref sphere,out output);
                    Check(hit==output);
                    ContainmentType inside=frustum.Contains(sphere), insideOutput;
                    frustum.Contains(ref sphere,out insideOutput);
                    Check(inside==insideOutput);
                    spheres.Append(hit?'1':'0').Append(Code(inside)).Append(Code(sphere.Contains(frustum)));
                }
                foreach(float extent in new float[]{0.125f,0.5f,2})
                {
                    var box=new BoundingBox(center-new Vector3(extent),center+new Vector3(extent));
                    bool hit=frustum.Intersects(box), output;
                    frustum.Intersects(ref box,out output); Check(hit==output);
                    ContainmentType inside=frustum.Contains(box), insideOutput;
                    frustum.Contains(ref box,out insideOutput); Check(inside==insideOutput);
                    boxes.Append(hit?'1':'0').Append(Code(inside)).Append(Code(box.Contains(frustum)));
                }
            }
            Console.WriteLine("sphere"+kind+" "+spheres);
            Console.WriteLine("box"+kind+" "+boxes);
            var frusta=new StringBuilder();
            for(int other=0;other<4;++other) foreach(float x in new float[]{-3,-1,0,1,3})
            foreach(float y in new float[]{-3,0,3}) foreach(float z in new float[]{-2,0,2})
            {
                // Apply translation explicitly, avoiding matrix-product differences in the generator.
                Matrix m=Frame(other); m.M41+=x; m.M42+=y; m.M43+=z;
                var rhs=new BoundingFrustum(m);
                frusta.Append(frustum.Intersects(rhs)?'1':'0').Append(Code(frustum.Contains(rhs))).Append(Code(rhs.Contains(frustum)));
            }
            Console.WriteLine("frustum"+kind+" "+frusta);
        }
        Console.WriteLine("overload_checks="+checks+" failed="+failed);
        return failed==0?0:1;
    }
}
