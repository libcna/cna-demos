// SPDX-License-Identifier: MS-PL
using System;
using Microsoft.Xna.Framework;
class QuaternionInverseOracle
{
    static int checks,failures;
    static int Bits(float x) { return BitConverter.ToInt32(BitConverter.GetBytes(x),0); }
    static void Check(bool value,string label)
    { ++checks;if(!value){++failures;Console.WriteLine("FAIL "+label);} }
    static void Emit(string name,Quaternion q)
    {Console.WriteLine(name+" "+Bits(q.X).ToString("X8")+" "+Bits(q.Y).ToString("X8")+" "+Bits(q.Z).ToString("X8")+" "+Bits(q.W).ToString("X8"));}
    static bool Same(Quaternion a,Quaternion b)
    {return Bits(a.X)==Bits(b.X)&&Bits(a.Y)==Bits(b.Y)&&Bits(a.Z)==Bits(b.Z)&&Bits(a.W)==Bits(b.W);}
    public static int Main()
    {
        Console.WriteLine(typeof(Quaternion).Assembly.FullName);
        foreach(float m in new float[]{1e20f,1e-30f,-1e20f,-1e-30f,float.MaxValue})
        {
            string label=Bits(m).ToString("X8");
            Quaternion q=new Quaternion(m,0,0,0),identity=Quaternion.Identity,result;
            Quaternion inverse=Quaternion.Inverse(q);Emit(label+".inverse",inverse);
            Check(!float.IsNaN(inverse.X)&&!float.IsInfinity(inverse.X)&&inverse.X!=0&&
                inverse.Y==0&&inverse.Z==0&&inverse.W==0,label+" inverse finite");
            Quaternion.Inverse(ref q,out result);Check(Same(inverse,result),label+" inverse out");
            result=q;Quaternion.Inverse(ref result,out result);Check(Same(inverse,result),label+" inverse alias");
            Quaternion division=Quaternion.Divide(identity,q);Emit(label+".divide",division);
            Check(division.X==inverse.X&&division.Y==0&&division.Z==0&&division.W==0,label+" divide finite");
            Quaternion.Divide(ref identity,ref q,out result);Check(Same(division,result),label+" divide out");
            result=q;Quaternion.Divide(ref identity,ref result,out result);Check(Same(division,result),label+" divide right alias");
            result=identity;Quaternion.Divide(ref result,ref q,out result);Check(Same(division,result),label+" divide left alias");
            Quaternion self=Quaternion.Divide(q,q);Emit(label+".self",self);
            Check(self==Quaternion.Identity,label+" self identity");
        }
        // The tiny inverse component must survive until multiplication by the large numerator.
        Quaternion numerator=new Quaternion(0,1e20f,0,0),denominator=new Quaternion(1e20f,1e-20f,0,0);
        Quaternion mixed=Quaternion.Divide(numerator,denominator);Emit("mixed.divide",mixed);
        Check(mixed.X==0&&mixed.Y==0&&mixed.Z==1&&mixed.W>0&&!float.IsInfinity(mixed.W),"mixed contribution survives");
        Console.WriteLine("checks="+checks+" failures="+failures);
        return failures==0?0:1;
    }
}
