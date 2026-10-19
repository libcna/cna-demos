// SPDX-License-Identifier: MS-PL
// Compile/run against genuine Microsoft XNA 4.0, not a replacement framework.
using System;
using Microsoft.Xna.Framework;

class PerspectiveOracle
{
    delegate void Projection(float near, float far, out Matrix result);
    static int checks, failures;
    static void Check(bool condition, string label)
    {
        ++checks;
        if (!condition) { ++failures; Console.WriteLine("FAIL " + label); }
    }
    static void Refuses(Action action, string parameter, string label)
    {
        try { action(); Check(false, label + " did not throw"); }
        catch (ArgumentOutOfRangeException e)
        { Check(e.ParamName == parameter, label + " ParamName=" + e.ParamName); }
        catch (Exception e) { Check(false, label + " type=" + e.GetType().FullName); }
    }
    static void Depth(string name, Func<float,float,Matrix> value, Projection output)
    {
        float[] near = {0, -1, 1, 10, 10, 0, float.NaN, 0};
        float[] far = {10, 10, 0, 10, 1, 0, 0, float.NaN};
        string[] parameter = {"nearPlaneDistance", "nearPlaneDistance", "farPlaneDistance",
            "nearPlaneDistance", "nearPlaneDistance", "nearPlaneDistance",
            "farPlaneDistance", "nearPlaneDistance"};
        for (int i=0; i<near.Length; ++i)
        {
            float n=near[i], f=far[i];
            Refuses(delegate { value(n,f); }, parameter[i], name+" value "+i);
            Matrix sentinel=Matrix.CreateTranslation(3,4,5), result=sentinel;
            Refuses(delegate { output(n,f,out result); }, parameter[i], name+" out "+i);
            Check(result==sentinel, name+" refused output unchanged "+i);
        }
    }
    static void Size(float n, float f, out Matrix result)
    { Matrix.CreatePerspective(2,2,n,f,out result); }
    static void Fov(float n, float f, out Matrix result)
    { Matrix.CreatePerspectiveFieldOfView(1,1,n,f,out result); }
    static void Off(float n, float f, out Matrix result)
    { Matrix.CreatePerspectiveOffCenter(-1,1,-1,1,n,f,out result); }
    public static int Main()
    {
        Console.WriteLine(typeof(Matrix).Assembly.FullName);
        Depth("size", delegate(float n,float f) { return Matrix.CreatePerspective(2,2,n,f); }, Size);
        Depth("fov", delegate(float n,float f) { return Matrix.CreatePerspectiveFieldOfView(1,1,n,f); }, Fov);
        Depth("off", delegate(float n,float f) { return Matrix.CreatePerspectiveOffCenter(-1,1,-1,1,n,f); }, Off);
        foreach(float angle in new float[] {0,-1,MathHelper.Pi,float.PositiveInfinity,float.NegativeInfinity})
        {
            float a=angle;
            Refuses(delegate { Matrix.CreatePerspectiveFieldOfView(a,1,0,0); }, "fieldOfView", "angle before planes");
            Matrix result=Matrix.Identity;
            Refuses(delegate { Matrix.CreatePerspectiveFieldOfView(a,1,1,10,out result); }, "fieldOfView", "angle out");
            Check(result==Matrix.Identity, "angle refused output unchanged");
        }
        int piBits=BitConverter.ToInt32(BitConverter.GetBytes(MathHelper.Pi),0);
        float below=BitConverter.ToSingle(BitConverter.GetBytes(piBits-1),0);
        Check(Matrix.CreatePerspectiveFieldOfView(below,1,1,10).M22>0,"nearest float below Pi accepted");
        Check(float.IsNaN(Matrix.CreatePerspectiveFieldOfView(float.NaN,1,1,10).M22),"NaN angle propagates");
        Refuses(delegate { Matrix.CreatePerspectiveFieldOfView(float.NaN,1,0,10); }, "nearPlaneDistance", "NaN angle does not bypass plane check");
        Check(float.IsPositiveInfinity(Matrix.CreatePerspectiveFieldOfView(1,0,1,10).M11),"zero aspect accepted");
        Check(Matrix.CreatePerspectiveFieldOfView(1,-1,1,10).M11<0,"negative aspect accepted");
        Check(float.IsPositiveInfinity(Matrix.CreatePerspective(0,1,1,10).M11),"zero width accepted");
        Check(float.IsPositiveInfinity(Matrix.CreatePerspective(1,0,1,10).M22),"zero height accepted");
        Check(float.IsPositiveInfinity(Matrix.CreatePerspectiveOffCenter(1,1,-1,1,1,10).M11),"collapsed x accepted");
        Check(float.IsPositiveInfinity(Matrix.CreatePerspectiveOffCenter(-1,1,1,1,1,10).M22),"collapsed y accepted");
        Console.WriteLine("checks="+checks+" failures="+failures+" piBits="+piBits.ToString("X8"));
        return failures==0?0:1;
    }
}
