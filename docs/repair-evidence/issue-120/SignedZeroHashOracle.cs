// SPDX-License-Identifier: MS-PL
using System;
using System.Reflection;
using Microsoft.Xna.Framework;
class SignedZeroHashOracle
{
    static int checks,failures;
    static float FromBits(uint b) { return BitConverter.ToSingle(BitConverter.GetBytes(b),0); }
    static void Check(bool ok,string label) { ++checks;if(!ok){++failures;Console.WriteLine("FAIL "+label);} }
    static void Pair(object a,object b,string label)
    { Check(a.Equals(b),label+" equals");Check(a.GetHashCode()==b.GetHashCode(),label+" hash"); }
    static int Main()
    {
        Console.WriteLine(typeof(Vector2).Assembly.FullName);
        float nz=FromBits(0x80000000);
        for(int mask=0;mask<16;++mask)
        {
            float x=(mask&1)!=0?nz:0,y=(mask&2)!=0?nz:0,z=(mask&4)!=0?nz:0,w=(mask&8)!=0?nz:0;
            Pair(new Vector2(0,0),new Vector2(x,y),"Vector2 "+mask);
            Pair(new Vector3(0,0,0),new Vector3(x,y,z),"Vector3 "+mask);
            Pair(new Vector4(0,0,0,0),new Vector4(x,y,z,w),"Vector4 "+mask);
            Pair(new Quaternion(0,0,0,0),new Quaternion(x,y,z,w),"Quaternion "+mask);
            Pair(new CurveKey(0,0,0,0,CurveContinuity.Step),new CurveKey(x,y,z,w,CurveContinuity.Step),"CurveKey "+mask);
        }
        foreach(FieldInfo field in typeof(Matrix).GetFields(BindingFlags.Instance|BindingFlags.Public))
        {
            object boxed=new Matrix();field.SetValue(boxed,nz);Pair(new Matrix(),boxed,"Matrix "+field.Name);
        }
        Check(new Vector2(1,nz).GetHashCode()==1065353216,"Vector2 unit exact");
        Check(new Vector3(1,nz,0).GetHashCode()==1065353216,"Vector3 unit exact");
        Check(new Vector4(1,nz,0,0).GetHashCode()==1065353216,"Vector4 unit exact");
        Check(new Quaternion(nz,0,0,1).GetHashCode()==1065353216,"Quaternion identity exact");
        Matrix identity=Matrix.Identity;identity.M12=nz;Check(identity.GetHashCode()==-33554432,"Matrix identity exact");
        Check(new CurveKey(nz,1,2,3).GetHashCode()==-1077936128,"CurveKey ordinary exact");
        foreach(float[] values in new float[][]{new float[]{-1,-2,-3,-4},new float[]{1,1,1,1},new float[]{nz,1,2,3}})
        foreach(CurveContinuity continuity in new CurveContinuity[]{CurveContinuity.Smooth,CurveContinuity.Step})
        {
            CurveKey key=new CurveKey(values[0],values[1],values[2],values[3],continuity);
            int expected=unchecked(values[0].GetHashCode()+values[1].GetHashCode()+values[2].GetHashCode()+values[3].GetHashCode()+continuity.GetHashCode());
            Console.WriteLine("CurveKey sum="+key.GetHashCode().ToString("X8")+" continuity="+continuity);
            Check(key.GetHashCode()==expected,"CurveKey wrapped sum");
        }
        foreach(uint bits in new uint[]{0,0x80000000,0x3f800000,0xbf800000,0x7f800000,0xff800000,0x7fc00001,0xffc12345})
        {
            float f=FromBits(bits);int expected=(bits&0x7fffffff)==0?0:unchecked((int)bits);
            int hash=new Vector2(f,0).GetHashCode();
            Console.WriteLine("bits="+bits.ToString("X8")+" Single="+f.GetHashCode().ToString("X8")+" Vector2="+hash.ToString("X8"));
            Check(hash==expected,"raw nonzero bits "+bits.ToString("X8"));
        }
        Console.WriteLine("checks="+checks+" failures="+failures);return failures==0?0:1;
    }
}
