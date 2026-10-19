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
        int checks=0, valueWideMismatch=0, valueNarrowMismatch=0, outWideMismatch=0, outNarrowMismatch=0;
        int valueContains=0, outContains=0;
        for (int i=0;i<1024;++i)
        {
            float radius=BitConverter.ToSingle(BitConverter.GetBytes(0x3f800000+i*7919),0);
            foreach (float sign in new[]{1f,-1f}) {
                var sphere=new BoundingSphere(Vector3.Zero,radius);
                var point=new Vector3(sign*radius,0,0);
                object[] squaredArgs={point,Vector3.Zero,0f};
                squaredOut.Invoke(null,squaredArgs);
                float squared=(float)squaredArgs[2];
                // A local float can remain extended in the 32-bit CLR JIT. Force
                // an actual Single serialization boundary for the narrow model.
                float narrowProduct=BitConverter.ToSingle(BitConverter.GetBytes(radius*radius),0);
                bool wide=squared<(double)radius*radius, narrow=squared<narrowProduct;
                bool actual=(ContainmentType)contains.Invoke(sphere,new object[]{point})==ContainmentType.Contains;
                object[] outputArgs={point,ContainmentType.Disjoint};
                containsOut.Invoke(sphere,outputArgs);
                bool output=(ContainmentType)outputArgs[1]==ContainmentType.Contains;
                if(actual)++valueContains;
                if(output)++outContains;
                if(actual!=wide)++valueWideMismatch;
                if(actual!=narrow)++valueNarrowMismatch;
                if(output!=wide)++outWideMismatch;
                if(output!=narrow)++outNarrowMismatch;
                if(actual!=output && checks<20)
                    Console.WriteLine("radiusBits="+BitConverter.ToInt32(BitConverter.GetBytes(radius),0).ToString("X8")+" value="+actual+" output="+output+" wide="+wide+" narrow="+narrow);
                ++checks;
            }
        }
        Console.WriteLine("checks="+checks+" valueContains="+valueContains+" outContains="+outContains+" valueWideMismatch="+valueWideMismatch+" valueNarrowMismatch="+valueNarrowMismatch+" outWideMismatch="+outWideMismatch+" outNarrowMismatch="+outNarrowMismatch);
        return 0; // Diagnostic model comparison, not an assumed overload-parity oracle.
    }
}
