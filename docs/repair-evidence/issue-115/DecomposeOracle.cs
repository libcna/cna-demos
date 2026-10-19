// SPDX-License-Identifier: MS-PL
using System;
using Microsoft.Xna.Framework;
class DecomposeOracle
{
    static int checks, failures;
    static string Bits(float x) { return BitConverter.ToInt32(BitConverter.GetBytes(x),0).ToString("X8"); }
    static void Check(bool ok,string label) { ++checks; if(!ok){++failures;Console.WriteLine("FAIL "+label);} }
    static bool Near(float a,float b) { return Math.Abs(a-b)<0.0001f; }
    static float Difference(Matrix a,Matrix b)
    {
        float[] x={a.M11,a.M12,a.M13,a.M21,a.M22,a.M23,a.M31,a.M32,a.M33,a.M41,a.M42,a.M43};
        float[] y={b.M11,b.M12,b.M13,b.M21,b.M22,b.M23,b.M31,b.M32,b.M33,b.M41,b.M42,b.M43};
        float d=0;for(int i=0;i<x.Length;++i)d=Math.Max(d,Math.Abs(x[i]-y[i]));return d;
    }
    static bool Run(string label,Matrix m,out Vector3 s,out Quaternion q)
    {
        Vector3 t;bool ok=m.Decompose(out s,out q,out t);
        Matrix rebuilt=Matrix.CreateScale(s)*Matrix.CreateFromQuaternion(q)*Matrix.CreateTranslation(t);
        Console.WriteLine(label+" ok="+ok+" scale="+Bits(s.X)+","+Bits(s.Y)+","+Bits(s.Z)+
            " rotation="+Bits(q.X)+","+Bits(q.Y)+","+Bits(q.Z)+","+Bits(q.W)+
            " translation="+Bits(t.X)+","+Bits(t.Y)+","+Bits(t.Z)+" difference="+Difference(m,rebuilt));
        Check(t==new Vector3(m.M41,m.M42,m.M43),label+" translation");
        return ok;
    }
    static int Main()
    {
        Console.WriteLine(typeof(Matrix).Assembly.FullName);
        Vector3 s;Quaternion q;
        foreach(Vector3 input in new Vector3[]{new Vector3(-2,3,4),new Vector3(-5,3,4),
            new Vector3(2,-5,4),new Vector3(2,3,-5),new Vector3(-2,-3,4),new Vector3(-2,-3,-4),
            new Vector3(-1,1,1),new Vector3(-2,2,1),new Vector3(-1,2,2),new Vector3(-2,1,2)})
        {
            Matrix m=Matrix.CreateScale(input)*Matrix.CreateRotationY(0.7f)*Matrix.CreateTranslation(7,8,9);
            bool ok=Run("reflection "+input,m,out s,out q);Check(ok,"reflection succeeds");
            Vector3 expected=new Vector3(Math.Abs(input.X),Math.Abs(input.Y),Math.Abs(input.Z));
            if(input.X*input.Y*input.Z<0)
            {
                if(expected.X>=expected.Y&&expected.X>=expected.Z)expected.X=-expected.X;
                else if(expected.Y>=expected.Z)expected.Y=-expected.Y;
                else expected.Z=-expected.Z;
            }
            Check(Near(s.X,expected.X)&&Near(s.Y,expected.Y)&&Near(s.Z,expected.Z),"largest-axis sign and ties");
            Check(Difference(m,Matrix.CreateScale(s)*Matrix.CreateFromQuaternion(q)*Matrix.CreateTranslation(7,8,9))<0.00001f,"reflection recomposes");
        }
        foreach(Vector3 input in new Vector3[]{new Vector3(1,0,1),new Vector3(0,1,1),new Vector3(1,1,0),
            new Vector3(1,0,0),new Vector3(0,1,0),new Vector3(0,0,1),Vector3.Zero,
            new Vector3(0.00001f,1,1),new Vector3(0.0001f,1,1),new Vector3(0.00010001f,1,1)})
        {
            bool ok=Run("degenerate "+input,Matrix.CreateScale(input)*Matrix.CreateTranslation(7,8,9),out s,out q);
            Check(s==input,"degenerate scale retained");
            if(input.X==0||input.Y==0||input.Z==0)
                Check(ok&&float.IsNaN(q.X)&&float.IsNaN(q.Y)&&float.IsNaN(q.Z)&&float.IsNaN(q.W),"zero axis succeeds with NaN rotation");
            else Check(ok==(input.X>=0.0001f)&&q==Quaternion.Identity,"tiny-axis boundary and rotation");
        }
        foreach(float shear in new float[]{0.05f,0.1f,0.2f,0.5f,1.0f})
        {
            Matrix m=Matrix.Identity;m.M12=shear;bool ok=Run("shear "+shear,m,out s,out q);
            Check(ok==(shear<=0.1f),"shear determinant refusal");
            if(!ok)Check(q==Quaternion.Identity,"refused rotation identity");
        }
        Matrix other=Matrix.Identity;other.M14=3;other.M24=-4;other.M34=5;other.M44=7;
        Check(Run("nonaffine",other,out s,out q)&&s==Vector3.One&&q==Quaternion.Identity,"fourth column ignored");
        other=Matrix.Identity;other.M11=float.NaN;
        Check(Run("nan",other,out s,out q)&&float.IsNaN(q.X)&&float.IsNaN(q.Y)&&float.IsNaN(q.Z)&&float.IsNaN(q.W),"NaN propagation");
        other=Matrix.Identity;other.M11=float.PositiveInfinity;
        Check(Run("infinity",other,out s,out q)&&float.IsNaN(q.X)&&float.IsNaN(q.Y)&&float.IsNaN(q.Z)&&float.IsNaN(q.W),"infinity propagation");
        other=Matrix.CreateScale(-2,3,4)*Matrix.CreateTranslation(7,8,9);
        Check(other.Decompose(out s,out q,out s)&&s==new Vector3(2,3,-4),"scale/translation output alias");
        Check(Run("all-zero matrix",new Matrix(),out s,out q)&&s==Vector3.Zero&&
            float.IsNaN(q.X)&&float.IsNaN(q.Y)&&float.IsNaN(q.Z)&&float.IsNaN(q.W),"all-zero matrix succeeds with undefined rotation");
        Console.WriteLine("checks="+checks+" failures="+failures);return failures==0?0:1;
    }
}
