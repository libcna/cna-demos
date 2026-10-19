// SPDX-License-Identifier: MS-PL
using System;
using Microsoft.Xna.Framework;
class PlaneNormalizeOracle
{
    static int checks, failures;
    static float Round(double v) { return BitConverter.ToSingle(BitConverter.GetBytes((float)v),0); }
    static float Float(uint bits) { return BitConverter.ToSingle(BitConverter.GetBytes(bits),0); }
    static uint Bits(float value) { return BitConverter.ToUInt32(BitConverter.GetBytes(value),0); }
    static bool Same(float a,float b) { return (float.IsNaN(a)&&float.IsNaN(b))||Bits(a)==Bits(b); }
    static string Describe(Plane p) { return Bits(p.Normal.X).ToString("X8")+","+Bits(p.Normal.Y).ToString("X8")+","+Bits(p.Normal.Z).ToString("X8")+","+Bits(p.D).ToString("X8"); }
    static void Check(Plane actual,Plane expected,string label)
    {
        ++checks;
        if(Same(actual.Normal.X,expected.Normal.X)&&Same(actual.Normal.Y,expected.Normal.Y)&&
           Same(actual.Normal.Z,expected.Normal.Z)&&Same(actual.D,expected.D))return;
        ++failures;
        if(failures<=30)Console.WriteLine("FAIL "+label+" actual="+Describe(actual)+" expected="+Describe(expected));
    }
    static Plane Expected(Plane p)
    {
        double x=p.Normal.X,y=p.Normal.Y,z=p.Normal.Z;
        double squared=x*x+y*y+z*z;
        if(Math.Abs(squared-1)<1.1920928955078125e-7)return p;
        // XNA explicitly converts the square root to Single, but keeps the reciprocal wide.
        double inverse=1/Round(Math.Sqrt(squared));
        return new Plane((float)(x*inverse),(float)(y*inverse),(float)(z*inverse),(float)(p.D*inverse));
    }
    static void Test(Plane p)
    {
        Plane expected=Expected(p),input=p,result;
        Check(Plane.Normalize(input),expected,"value "+Describe(p));
        Plane.Normalize(ref input,out result);Check(result,expected,"out "+Describe(p));
        Check(input,p,"source unchanged");
        Plane.Normalize(ref input,out input);Check(input,expected,"alias "+Describe(p));
        input=p;input.Normalize();Check(input,expected,"member "+Describe(p));
    }
    static int Main()
    {
        Console.WriteLine(typeof(Plane).Assembly.FullName);
        float[] samples={0,Float(0x80000000),1,-1,0.3f,-0.6f,3,4,1e20f,1e-30f,float.MaxValue,
            Float(1),float.PositiveInfinity,float.NegativeInfinity,Float(0x7fc00001)};
        foreach(float x in samples)foreach(float y in samples)
            foreach(float z in new[]{0f,0.8f,1e-30f})Test(new Plane(x,y,z,-2));
        foreach(float x in new[]{Float(0x3f7ffffe),Float(0x3f7fffff),1f,Float(0x3f800001),Float(0x3f800002)})
            Test(new Plane(x,0,0,-2));
        Test(new Plane(0.8414709568023682f,0.5403022766113281f,0,-2));
        Console.WriteLine("checks="+checks+" failures="+failures);
        return failures==0?0:1;
    }
}
