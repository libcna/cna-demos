// Diagnostic: determine the radius-product comparison boundary before changing CNA.
using System;
using System.Reflection;
using Microsoft.Xna.Framework;

static class SpherePointRadiusPrecisionOracle
{
    static int Main()
    {
        MethodInfo contains = typeof(BoundingSphere).GetMethod("Contains",new[]{typeof(Vector3)});
        MethodInfo containsOut = typeof(BoundingSphere).GetMethod("Contains",new[]{typeof(Vector3).MakeByRefType(),typeof(ContainmentType).MakeByRefType()});
        MethodInfo squaredOut = typeof(Vector3).GetMethod("DistanceSquared",new[]{typeof(Vector3).MakeByRefType(),typeof(Vector3).MakeByRefType(),typeof(float).MakeByRefType()});
        int checks=0, wideMismatch=0, narrowMismatch=0, overloadMismatch=0;
        for (int i=0;i<1024;++i)
        {
            float radius=BitConverter.ToSingle(BitConverter.GetBytes(0x3f800000+i*7919),0);
            foreach (float sign in new[]{1f,-1f}) {
                var sphere=new BoundingSphere(Vector3.Zero,radius);
                var point=new Vector3(sign*radius,0,0);
                object[] squaredArgs={point,Vector3.Zero,0f};
                squaredOut.Invoke(null,squaredArgs);
                float squared=(float)squaredArgs[2];
                float narrowProduct=radius*radius;
                bool wide=squared<(double)radius*radius, narrow=squared<narrowProduct;
                bool actual=(ContainmentType)contains.Invoke(sphere,new object[]{point})==ContainmentType.Contains;
                object[] outputArgs={point,ContainmentType.Disjoint};
                containsOut.Invoke(sphere,outputArgs);
                if (((ContainmentType)outputArgs[1]==ContainmentType.Contains)!=actual)++overloadMismatch;
                if(actual!=wide)++wideMismatch;
                if(actual!=narrow)++narrowMismatch;
                ++checks;
            }
        }
        Console.WriteLine("checks="+checks+" wideMismatch="+wideMismatch+" narrowMismatch="+narrowMismatch+" overloadMismatch="+overloadMismatch);
        return overloadMismatch==0?0:1;
    }
}
