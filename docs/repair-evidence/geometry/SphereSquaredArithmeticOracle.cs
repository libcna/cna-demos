// Genuine method-entry checks: explicit output storage is not an x87 return register.
using System;
using System.Reflection;
using Microsoft.Xna.Framework;

static class SphereSquaredArithmeticOracle
{
    static int checks,failed;
    static uint state=0x12345678;
    static float Sample() { state=unchecked(state*1664525+1013904223); return ((int)(state>>8)%8193-4096)/1024f; }
    static float Single(double value) { return BitConverter.ToSingle(BitConverter.GetBytes((float)value),0); }
    static void Check(bool ok,string name) { ++checks; if(!ok) { if(failed<12)Console.WriteLine("FAIL "+name); ++failed; } }
    static int Main()
    {
        Type vr=typeof(Vector3).MakeByRefType();
        var distance=typeof(Vector3).GetMethod("DistanceSquared",new[]{typeof(Vector3),typeof(Vector3)});
        var distanceOut=typeof(Vector3).GetMethod("DistanceSquared",new[]{vr,vr,typeof(float).MakeByRefType()});
        var length=typeof(Vector3).GetMethod("LengthSquared",Type.EmptyTypes);
        var contains=typeof(BoundingSphere).GetMethod("Contains",new[]{typeof(Vector3)});
        var containsOut=typeof(BoundingSphere).GetMethod("Contains",new[]{vr,typeof(ContainmentType).MakeByRefType()});
        var hit=typeof(BoundingSphere).GetMethod("Intersects",new[]{typeof(BoundingSphere)});
        var hitOut=typeof(BoundingSphere).GetMethod("Intersects",new[]{typeof(BoundingSphere).MakeByRefType(),typeof(bool).MakeByRefType()});
        foreach(float scale in new[]{1f,0.00001f,100000f,1e20f,1e-20f})
        for(int i=0;i<256;++i) {
            var center=new Vector3(Sample()*scale,Sample()*scale,Sample()*scale);
            var point=new Vector3(Sample()*scale,Sample()*scale,Sample()*scale);
            double x=(double)point.X-center.X,y=(double)point.Y-center.Y,z=(double)point.Z-center.Z;
            double wide=x*x+y*y+z*z;
            float stored=Single(wide);
            Check((float)distance.Invoke(null,new object[]{point,center})==stored,"DistanceSquared value");
            object[] ds={point,center,0f}; distanceOut.Invoke(null,ds);
            Check((float)ds[2]==stored,"DistanceSquared output");
            double norm=(double)point.X*point.X+(double)point.Y*point.Y+(double)point.Z*point.Z;
            Check((float)length.Invoke(point,new object[0])==Single(norm),"LengthSquared value");
            float baseRadius=Single(Math.Sqrt(wide));
            int bits=BitConverter.ToInt32(BitConverter.GetBytes(baseRadius),0);
            for(int delta=-1;delta<=1;++delta) {
                float radius=BitConverter.ToSingle(BitConverter.GetBytes(bits+delta),0);
                var sphere=new BoundingSphere(center,radius);
                double squaredRadius=(double)radius*radius;
                bool value=(ContainmentType)contains.Invoke(sphere,new object[]{point})==ContainmentType.Contains;
                object[] output={point,ContainmentType.Disjoint}; containsOut.Invoke(sphere,output);
                Check(value==(wide<squaredRadius),"Contains point unrounded value");
                Check(((ContainmentType)output[1]==ContainmentType.Contains)==(stored<squaredRadius),"Contains point stored output");
                var other=new BoundingSphere(point,0);
                Check((bool)hit.Invoke(sphere,new object[]{other})==(squaredRadius>stored),"sphere intersection stored value");
                object[] hits={other,false}; hitOut.Invoke(sphere,hits);
                Check((bool)hits[1]==(squaredRadius>stored),"sphere intersection stored output");
            }
        }
        Console.WriteLine("checks="+checks+" failed="+failed);
        return failed==0?0:1;
    }
}
