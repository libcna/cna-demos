// SPDX-License-Identifier: MS-PL
using System;
using Microsoft.Xna.Framework;
class BoundsOracle
{
    static int checks,failures;
    static float FromBits(uint x){return BitConverter.ToSingle(BitConverter.GetBytes(x),0);}
    static int Bits(float x){return BitConverter.ToInt32(BitConverter.GetBytes(x),0);}
    static void Check(bool ok,string label){++checks;if(!ok){++failures;Console.WriteLine("FAIL "+label);}}
    static void E2(Vector2 q,float e,string label){Check(Bits(q.X)==Bits(e)&&Bits(q.Y)==Bits(e),label);}
    static void E3(Vector3 q,float e,string label){Check(Bits(q.X)==Bits(e)&&Bits(q.Y)==Bits(e)&&Bits(q.Z)==Bits(e),label);}
    static void E4(Vector4 q,float e,string label){Check(Bits(q.X)==Bits(e)&&Bits(q.Y)==Bits(e)&&Bits(q.Z)==Bits(e)&&Bits(q.W)==Bits(e),label);}
    static int Main()
    {
        Console.WriteLine(typeof(Vector2).Assembly.FullName);
        float[] samples={float.NegativeInfinity,-1,FromBits(0x80000000),0,1,float.PositiveInfinity,FromBits(0x7fc00001),FromBits(0xffc12345)};
        foreach(float a in samples)foreach(float b in samples)
        {
            float min=a<b?a:b,max=a>b?a:b;
            Vector2 a2=new Vector2(a),b2=new Vector2(b),r2;
            Vector3 a3=new Vector3(a),b3=new Vector3(b),r3;
            Vector4 a4=new Vector4(a),b4=new Vector4(b),r4;
            E2(Vector2.Min(a2,b2),min,"v2 min");E2(Vector2.Max(a2,b2),max,"v2 max");
            Vector2.Min(ref a2,ref b2,out r2);E2(r2,min,"v2 min out");Vector2.Max(ref a2,ref b2,out r2);E2(r2,max,"v2 max out");
            r2=a2;Vector2.Min(ref r2,ref b2,out r2);E2(r2,min,"v2 min alias");r2=b2;Vector2.Max(ref a2,ref r2,out r2);E2(r2,max,"v2 max alias");
            E3(Vector3.Min(a3,b3),min,"v3 min");E3(Vector3.Max(a3,b3),max,"v3 max");
            Vector3.Min(ref a3,ref b3,out r3);E3(r3,min,"v3 min out");Vector3.Max(ref a3,ref b3,out r3);E3(r3,max,"v3 max out");
            r3=a3;Vector3.Min(ref r3,ref b3,out r3);E3(r3,min,"v3 min alias");r3=b3;Vector3.Max(ref a3,ref r3,out r3);E3(r3,max,"v3 max alias");
            E4(Vector4.Min(a4,b4),min,"v4 min");E4(Vector4.Max(a4,b4),max,"v4 max");
            Vector4.Min(ref a4,ref b4,out r4);E4(r4,min,"v4 min out");Vector4.Max(ref a4,ref b4,out r4);E4(r4,max,"v4 max out");
            r4=a4;Vector4.Min(ref r4,ref b4,out r4);E4(r4,min,"v4 min alias");r4=b4;Vector4.Max(ref a4,ref r4,out r4);E4(r4,max,"v4 max alias");
            Check(Bits(MathHelper.Min(a,b))==Bits(float.IsNaN(a)?a:min),"scalar min first NaN");
            Check(Bits(MathHelper.Max(a,b))==Bits(float.IsNaN(a)?a:max),"scalar max first NaN");
        }
        foreach(float value in samples)foreach(float min in samples)foreach(float max in samples)
        {
            float expected=value>max?max:value;expected=expected<min?min:expected;
            Vector2 v2=new Vector2(value),lo2=new Vector2(min),hi2=new Vector2(max),r2;
            E2(Vector2.Clamp(v2,lo2,hi2),expected,"v2 clamp");Vector2.Clamp(ref v2,ref lo2,ref hi2,out r2);E2(r2,expected,"v2 clamp out");
            r2=lo2;Vector2.Clamp(ref v2,ref r2,ref hi2,out r2);E2(r2,expected,"v2 clamp min alias");
            Vector3 v3=new Vector3(value),lo3=new Vector3(min),hi3=new Vector3(max),r3;
            E3(Vector3.Clamp(v3,lo3,hi3),expected,"v3 clamp");Vector3.Clamp(ref v3,ref lo3,ref hi3,out r3);E3(r3,expected,"v3 clamp out");
            r3=lo3;Vector3.Clamp(ref v3,ref r3,ref hi3,out r3);E3(r3,expected,"v3 clamp min alias");
            Vector4 v4=new Vector4(value),lo4=new Vector4(min),hi4=new Vector4(max),r4;
            E4(Vector4.Clamp(v4,lo4,hi4),expected,"v4 clamp");Vector4.Clamp(ref v4,ref lo4,ref hi4,out r4);E4(r4,expected,"v4 clamp out");
            r4=lo4;Vector4.Clamp(ref v4,ref r4,ref hi4,out r4);E4(r4,expected,"v4 clamp min alias");
            Check(Bits(MathHelper.Clamp(value,min,max))==Bits(expected),"scalar clamp");
        }
        Vector4 mixed=Vector4.Clamp(new Vector4(2,5,7,-6),new Vector4(-2,3,8,-9),new Vector4(1,7,6,-4));
        Console.WriteLine("mixed clamp="+mixed);
        Check(mixed==new Vector4(1,5,8,-6),"demo mixed Z interval is reversed, so Z is 8 not 6");
        Console.WriteLine("checks="+checks+" failures="+failures);return failures==0?0:1;
    }
}
