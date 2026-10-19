// Distinguishes the inline XNA frustum/sphere plane expression from a rounded Dot call.
using System;
using Microsoft.Xna.Framework;
static class FrustumSpherePlaneOracle
{
    static int Main()
    {
        Matrix m=Matrix.Identity;
        m.M11=0.6f; m.M12=0.8f; m.M21=-0.8f; m.M22=0.6f; m.M41=0.25f; m.M42=-0.5f;
        var f=new BoundingFrustum(m);
        Plane[] planes={f.Near,f.Far,f.Left,f.Right,f.Top,f.Bottom};
        int checks=0,wideMismatch=0,roundedMismatch=0;
        foreach(float x in new float[]{-3,-1.5f,-1,-0.5f,0,0.5f,1,1.5f,3})
        foreach(float y in new float[]{-3,-1.5f,-1,-0.5f,0,0.5f,1,1.5f,3})
        foreach(float z in new float[]{-3,-0.25f,0,0.5f,1,1.25f})
        foreach(float radius in new float[]{0,0.25f,1,3})
        {
            var sphere=new BoundingSphere(new Vector3(x,y,z),radius);
            var actual=f.Contains(sphere);
            var wide=ContainmentType.Contains; var rounded=ContainmentType.Contains;
            foreach(Plane p in planes)
            {
                double distance=(double)p.Normal.X*x+(double)p.Normal.Y*y+(double)p.Normal.Z*z+p.D;
                // Force the same observable narrowing as a public Dot(ref,out) result.
                float dot=(float)((double)p.Normal.X*x+(double)p.Normal.Y*y+(double)p.Normal.Z*z);
                float roundedDistance=dot+p.D;
                if(distance>radius)wide=ContainmentType.Disjoint;
                else if(wide!=ContainmentType.Disjoint && !(distance < -radius))wide=ContainmentType.Intersects;
                if(roundedDistance>radius)rounded=ContainmentType.Disjoint;
                else if(rounded!=ContainmentType.Disjoint && !(roundedDistance < -radius))rounded=ContainmentType.Intersects;
            }
            ++checks;
            if(actual!=wide){++wideMismatch;Console.WriteLine("FAIL wide "+sphere);}
            if(actual!=rounded)++roundedMismatch;
        }
        Console.WriteLine("checks="+checks+" wide_mismatch="+wideMismatch+" rounded_mismatch="+roundedMismatch);
        return wideMismatch==0 && roundedMismatch>0?0:1;
    }
}
