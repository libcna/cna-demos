// SPDX-License-Identifier: MS-PL
using System;
using Microsoft.Xna.Framework;
class RangeOracle
{
    static int checks, failures;
    static int Bits(float x) { return BitConverter.ToInt32(BitConverter.GetBytes(x),0); }
    static void Emit(string name, params float[] values)
    {
        Console.Write(name);
        foreach(float value in values) Console.Write(" "+Bits(value).ToString("X8"));
        Console.WriteLine();
    }
    static void Q(string name, Quaternion q) { Emit(name,q.X,q.Y,q.Z,q.W); }
    static void V2(string name, Vector2 v) { Emit(name,v.X,v.Y); }
    static void V3(string name, Vector3 v) { Emit(name,v.X,v.Y,v.Z); }
    static void V4(string name, Vector4 v) { Emit(name,v.X,v.Y,v.Z,v.W); }
    static void Check(bool condition,string label)
    { ++checks; if(!condition) { ++failures; Console.WriteLine("FAIL "+label); } }
    public static int Main()
    {
        Console.WriteLine(typeof(Vector2).Assembly.FullName);
        float[] samples={1e20f,1e-30f,float.MaxValue,float.Epsilon,1f,3f,0f,
                         float.PositiveInfinity,float.NaN,-1e20f,-1e-30f};
        for(int i=0;i<samples.Length;i++)
        {
            float m=samples[i]; string tag="s"+i;
            Emit(tag+".input",m);
            Vector2 a=new Vector2(m,0), a0=Vector2.Zero, ar;
            Vector3 b=new Vector3(m,0,0), b0=Vector3.Zero, br;
            Vector4 c=new Vector4(m,0,0,0), c0=Vector4.Zero, cr;
            Quaternion q=new Quaternion(m,0,0,0), qr, identity=Quaternion.Identity;
            float ad,bd,cd;
            Vector2.Distance(ref a,ref a0,out ad);
            Vector3.Distance(ref b,ref b0,out bd);
            Vector4.Distance(ref c,ref c0,out cd);
            Emit(tag+".v2.measure",a.Length(),a.LengthSquared(),Vector2.Distance(a,a0),ad,Vector2.DistanceSquared(a,a0));
            Emit(tag+".v3.measure",b.Length(),b.LengthSquared(),Vector3.Distance(b,b0),bd,Vector3.DistanceSquared(b,b0));
            Emit(tag+".v4.measure",c.Length(),c.LengthSquared(),Vector4.Distance(c,c0),cd,Vector4.DistanceSquared(c,c0));
            Emit(tag+".q.measure",q.Length(),q.LengthSquared());
            V2(tag+".v2.norm.value",Vector2.Normalize(a)); Vector2.Normalize(ref a,out ar); V2(tag+".v2.norm.out",ar);
            ar=a; ar.Normalize(); V2(tag+".v2.norm.member",ar); Vector2.Normalize(ref a,out a); V2(tag+".v2.norm.alias",a);
            V3(tag+".v3.norm.value",Vector3.Normalize(b)); Vector3.Normalize(ref b,out br); V3(tag+".v3.norm.out",br);
            br=b; br.Normalize(); V3(tag+".v3.norm.member",br); Vector3.Normalize(ref b,out b); V3(tag+".v3.norm.alias",b);
            V4(tag+".v4.norm.value",Vector4.Normalize(c)); Vector4.Normalize(ref c,out cr); V4(tag+".v4.norm.out",cr);
            cr=c; cr.Normalize(); V4(tag+".v4.norm.member",cr); Vector4.Normalize(ref c,out c); V4(tag+".v4.norm.alias",c);
            Q(tag+".q.norm.value",Quaternion.Normalize(q)); Quaternion.Normalize(ref q,out qr); Q(tag+".q.norm.out",qr);
            qr=q; qr.Normalize(); Q(tag+".q.norm.member",qr); qr=q; Quaternion.Normalize(ref qr,out qr); Q(tag+".q.norm.alias",qr);
            Q(tag+".q.inverse.value",Quaternion.Inverse(q)); Quaternion.Inverse(ref q,out qr); Q(tag+".q.inverse.out",qr);
            qr=q; Quaternion.Inverse(ref qr,out qr); Q(tag+".q.inverse.alias",qr);
            Q(tag+".q.divide.value",Quaternion.Divide(identity,q)); Quaternion.Divide(ref identity,ref q,out qr); Q(tag+".q.divide.out",qr);
            qr=q; Quaternion.Divide(ref identity,ref qr,out qr); Q(tag+".q.divide.alias",qr);
            if(i==0||i==1)
            {
                Check(new Vector2(m,0).Length()==m,tag+" Vector2 finite length");
                Check(new Vector3(m,0,0).Length()==m,tag+" Vector3 finite length");
                Check(new Vector4(m,0,0,0).Length()==m,tag+" Vector4 finite length");
                Check(q.Length()==m,tag+" Quaternion finite length");
                Check(ad==m&&bd==m&&cd==m,tag+" finite distances");
                Quaternion inverse=Quaternion.Inverse(q);
                Check(!float.IsNaN(inverse.X)&&!float.IsInfinity(inverse.X)&&inverse.X<0&&
                      inverse.Y==0&&inverse.Z==0&&inverse.W==0,tag+" finite inverse");
            }
        }
        foreach(Quaternion q in new Quaternion[] {new Quaternion(1,2,3,4),new Quaternion(0.3f,-0.7f,2.2f,-3.1f)})
        {
            Q("ordinary.input",q); Q("ordinary.norm",Quaternion.Normalize(q)); Q("ordinary.inverse",Quaternion.Inverse(q));
        }
        Console.WriteLine("checks="+checks+" failures="+failures);
        return failures==0?0:1;
    }
}
