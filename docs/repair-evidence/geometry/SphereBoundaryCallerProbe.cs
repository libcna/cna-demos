// Retained FAILED exploratory caller model: 492 mismatched pairs, not a passing oracle.
// Genuine method-entry probe for overflow/NaN and mutable-radius containment.
using System;
using System.Reflection;
using Microsoft.Xna.Framework;
static class SphereBoundaryCallerProbe
{
    static int Main()
    {
        MethodInfo contains=typeof(BoundingSphere).GetMethod("Contains",new Type[]{typeof(BoundingSphere)});
        MethodInfo intersects=typeof(BoundingSphere).GetMethod("Intersects",new Type[]{typeof(BoundingSphere)});
        float[] positions={0,-0.0f,1,5,1e20f,-1e20f,float.MaxValue,-float.MaxValue,float.PositiveInfinity,float.NaN};
        float[] radii={0,1,1e20f,float.MaxValue,float.PositiveInfinity,float.NaN,-1,float.NegativeInfinity};
        int checks=0,failed=0,roundedDifferences=0;
        foreach(float x in positions)foreach(float y in positions)foreach(float a in radii)foreach(float b in radii)
        {
            BoundingSphere outer=default(BoundingSphere),inner=default(BoundingSphere);
            outer.Center=new Vector3(x,0,0);outer.Radius=a;
            inner.Center=new Vector3(y,0,0);inner.Radius=b;
            float distance=BitConverter.ToSingle(BitConverter.GetBytes(Vector3.Distance(outer.Center,inner.Center)),0);
            float square=BitConverter.ToSingle(BitConverter.GetBytes(Vector3.DistanceSquared(outer.Center,inner.Center)),0);
            double da=a,db=b;
            var expected=!(da+db>=distance)?ContainmentType.Disjoint:
                !(da-db>=distance)?ContainmentType.Intersects:ContainmentType.Contains;
            bool expectedHit=da*da+2.0*da*db+db*db>square;
            var actual=(ContainmentType)contains.Invoke(outer,new object[]{inner});
            bool actualHit=(bool)intersects.Invoke(outer,new object[]{inner});
            checks+=2;
            if(actual!=expected||actualHit!=expectedHit)
            {
                ++failed;
                Console.WriteLine("FAIL positions="+x+","+y+" radii="+a+","+b+
                    " contains="+actual+" expected="+expected+" hit="+actualHit+" expectedHit="+expectedHit);
            }
            float sum=BitConverter.ToSingle(BitConverter.GetBytes(a+b),0),difference=BitConverter.ToSingle(BitConverter.GetBytes(a-b),0);
            var rounded=!(sum>=distance)?ContainmentType.Disjoint:!(difference>=distance)?ContainmentType.Intersects:ContainmentType.Contains;
            if(actual!=rounded)++roundedDifferences;
        }
        Console.WriteLine("checks="+checks+" failed_pairs="+failed+" rounded_differences="+roundedDifferences);
        return failed==0?0:1;
    }
}
