// SPDX-License-Identifier: MS-PL
using System;
using Microsoft.Xna.Framework;
class ProductOracle
{
    static int checks, failures;
    static float Float(uint bits) { return BitConverter.ToSingle(BitConverter.GetBytes(bits),0); }
    static uint Bits(float value) { return BitConverter.ToUInt32(BitConverter.GetBytes(value),0); }
    static void Check(float actual,float expected,string label)
    {
        ++checks;
        if((float.IsNaN(actual)&&float.IsNaN(expected))||Bits(actual)==Bits(expected))return;
        ++failures;
        if(failures<=30)Console.WriteLine("FAIL "+label+" actual="+Bits(actual).ToString("X8")+" expected="+Bits(expected).ToString("X8"));
    }
    static void Check(Vector3 actual,Vector3 expected,string label)
    {
        Check(actual.X,expected.X,label+" X");Check(actual.Y,expected.Y,label+" Y");Check(actual.Z,expected.Z,label+" Z");
    }
    static int Main()
    {
        Console.WriteLine(typeof(Vector3).Assembly.FullName);
        Vector4[] samples={Vector4.Zero,new Vector4(-0f,Float(0x80000000),0,Float(0x80000000)),
            Vector4.One,Vector4.UnitX,Vector4.UnitY,Vector4.UnitZ,Vector4.UnitW,
            new Vector4(1,-2,3,-4),new Vector4(5,6,-7,-8),new Vector4(0.3f,-0.6f,0.8f,0.1f),
            new Vector4(1e20f,1e20f,1,2),new Vector4(1e20f,-1e20f,1,3),
            new Vector4(1e20f,1e20f,2,3),new Vector4(1e-30f,1e-30f,1e-30f,1e-30f),
            new Vector4(float.MaxValue,float.MaxValue,1,2),new Vector4(float.MaxValue,-float.MaxValue,1,3),
            new Vector4(float.PositiveInfinity,0,0,0),new Vector4(Float(0x7fc00001),1,2,3)};
        foreach(Vector4 originalA in samples)foreach(Vector4 originalB in samples)
        {
            Vector4 a4=originalA,b4=originalB;
            Vector3 a3=new Vector3(a4.X,a4.Y,a4.Z),b3=new Vector3(b4.X,b4.Y,b4.Z);
            Vector2 a2=new Vector2(a4.X,a4.Y),b2=new Vector2(b4.X,b4.Y);
            double p0=(double)a4.X*b4.X,p1=(double)a4.Y*b4.Y,p2=(double)a4.Z*b4.Z,p3=(double)a4.W*b4.W;
            float d2=(float)(p0+p1),d3=(float)(p0+p1+p2),d4=(float)(p0+p1+p2+p3),result;
            Check(Vector2.Dot(a2,b2),d2,"Dot2 value");Vector2.Dot(ref a2,ref b2,out result);Check(result,d2,"Dot2 out");
            Check(Vector3.Dot(a3,b3),d3,"Dot3 value");Vector3.Dot(ref a3,ref b3,out result);Check(result,d3,"Dot3 out");
            Check(Vector4.Dot(a4,b4),d4,"Dot4 value");Vector4.Dot(ref a4,ref b4,out result);Check(result,d4,"Dot4 out");
            Vector3 expected=new Vector3((float)((double)a3.Y*b3.Z-(double)a3.Z*b3.Y),
                (float)((double)a3.Z*b3.X-(double)a3.X*b3.Z),(float)((double)a3.X*b3.Y-(double)a3.Y*b3.X));
            Check(Vector3.Cross(a3,b3),expected,"Cross value");
            Vector3 output;Vector3.Cross(ref a3,ref b3,out output);Check(output,expected,"Cross out");
            Vector3 left=a3,right=b3;
            Vector3.Cross(ref left,ref right,out left);Check(left,expected,"Cross left alias");Check(right,b3,"Cross right unchanged");
            left=a3;right=b3;
            Vector3.Cross(ref left,ref right,out right);Check(right,expected,"Cross right alias");Check(left,a3,"Cross left unchanged");
            Vector2.Dot(ref a2,ref b2,out a2.X);Check(a2.X,d2,"Dot2 component alias");
            Vector3.Dot(ref a3,ref b3,out b3.Y);Check(b3.Y,d3,"Dot3 component alias");
            Vector4.Dot(ref a4,ref b4,out a4.W);Check(a4.W,d4,"Dot4 component alias");
        }
        Console.WriteLine("checks="+checks+" failures="+failures);
        return failures==0?0:1;
    }
}
