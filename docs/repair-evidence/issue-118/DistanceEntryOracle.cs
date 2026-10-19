// SPDX-License-Identifier: MS-PL
using System;
using System.Reflection;
using Microsoft.Xna.Framework;
class DistanceEntryOracle
{
    static int checks,failures;
    static uint Bits(float v){return BitConverter.ToUInt32(BitConverter.GetBytes(v),0);}
    static void Check(float actual,float expected,string label)
    {
        ++checks;
        if((float.IsNaN(actual)&&float.IsNaN(expected))||Bits(actual)==Bits(expected))return;
        ++failures;
        if(failures<=30)Console.WriteLine("FAIL "+label+" actual="+Bits(actual).ToString("X8")+" expected="+Bits(expected).ToString("X8"));
    }
    static void Test(Type type,int count,Vector4[] samples)
    {
        MethodInfo value=type.GetMethod("Distance",new[]{type,type});
        MethodInfo output=type.GetMethod("Distance",new[]{type.MakeByRefType(),type.MakeByRefType(),typeof(float).MakeByRefType()});
        foreach(Vector4 a in samples)foreach(Vector4 b in samples)
        {
            double x=(double)a.X-b.X,y=(double)a.Y-b.Y,z=(double)a.Z-b.Z,w=(double)a.W-b.W;
            double squared=x*x+y*y;
            if(count>=3)squared+=z*z;
            if(count==4)squared+=w*w;
            float expected=(float)Math.Sqrt(squared);
            object left=count==2?(object)new Vector2(a.X,a.Y):count==3?(object)new Vector3(a.X,a.Y,a.Z):a;
            object right=count==2?(object)new Vector2(b.X,b.Y):count==3?(object)new Vector3(b.X,b.Y,b.Z):b;
            // Reflection invokes the actual public method body, eliminating caller inlining.
            Check((float)value.Invoke(null,new[]{left,right}),expected,type.Name+" value");
            object[] args={left,right,0f};output.Invoke(null,args);
            Check((float)args[2],expected,type.Name+" out");
        }
    }
    static int Main()
    {
        Console.WriteLine(typeof(Vector2).Assembly.FullName);
        Vector4[] samples={Vector4.Zero,Vector4.One,new Vector4(3,4,0,0),new Vector4(1,2,3,4),
            new Vector4(0.3f,-0.6f,0.8f,0.1f),new Vector4(1e20f,0,0,0),new Vector4(-1e20f,0,0,0),
            new Vector4(1e-30f,0,0,0),new Vector4(-1e-30f,0,0,0),new Vector4(float.Epsilon,0,0,0),
            new Vector4(float.MaxValue,0,0,0),new Vector4(-float.MaxValue,0,0,0),
            new Vector4(float.PositiveInfinity,0,0,0),new Vector4(float.NaN,0,0,0)};
        Test(typeof(Vector2),2,samples);Test(typeof(Vector3),3,samples);Test(typeof(Vector4),4,samples);
        Console.WriteLine("checks="+checks+" failures="+failures);return failures==0?0:1;
    }
}
