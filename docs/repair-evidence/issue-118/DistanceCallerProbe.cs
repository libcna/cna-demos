// SPDX-License-Identifier: MS-PL
using System;
using System.Runtime.CompilerServices;
using Microsoft.Xna.Framework;
class DistanceOracle
{
    static void Emit(string name,params float[] values)
    {
        Console.Write(name);
        foreach(float x in values) Console.Write(" "+BitConverter.ToInt32(BitConverter.GetBytes(x),0).ToString("X8"));
        Console.WriteLine();
    }
    [MethodImpl(MethodImplOptions.NoInlining)]
    static float V2(float m) { return Vector2.Distance(Vector2.Zero,new Vector2(m,0)); }
    [MethodImpl(MethodImplOptions.NoInlining)]
    static float O2(float m) { Vector2 a=Vector2.Zero,b=new Vector2(m,0);float r;Vector2.Distance(ref a,ref b,out r);return r; }
    [MethodImpl(MethodImplOptions.NoInlining)]
    static float V3(float m) { return Vector3.Distance(Vector3.Zero,new Vector3(m,0,0)); }
    [MethodImpl(MethodImplOptions.NoInlining)]
    static float O3(float m) { Vector3 a=Vector3.Zero,b=new Vector3(m,0,0);float r;Vector3.Distance(ref a,ref b,out r);return r; }
    [MethodImpl(MethodImplOptions.NoInlining)]
    static float V4(float m) { return Vector4.Distance(Vector4.Zero,new Vector4(m,0,0,0)); }
    [MethodImpl(MethodImplOptions.NoInlining)]
    static float O4(float m) { Vector4 a=Vector4.Zero,b=new Vector4(m,0,0,0);float r;Vector4.Distance(ref a,ref b,out r);return r; }
    public static int Main()
    {
        Console.WriteLine(typeof(Vector2).Assembly.FullName);
        foreach(float m in new float[]{1e20f,1e-30f,3f})
        {
            Emit("input",m);
            Emit("isolated.v2.value",V2(m)); Emit("isolated.v2.out",O2(m));
            Emit("isolated.v3.value",V3(m)); Emit("isolated.v3.out",O3(m));
            Emit("isolated.v4.value",V4(m)); Emit("isolated.v4.out",O4(m));
            Vector2 a=new Vector2(m,0),a0=Vector2.Zero;
            Vector3 b=new Vector3(m,0,0),b0=Vector3.Zero;
            Vector4 c=new Vector4(m,0,0,0),c0=Vector4.Zero;
            float ad,bd,cd;
            Vector2.Distance(ref a,ref a0,out ad);Vector3.Distance(ref b,ref b0,out bd);Vector4.Distance(ref c,ref c0,out cd);
            Emit("grouped.v2",a.Length(),a.LengthSquared(),Vector2.Distance(a,a0),ad,Vector2.DistanceSquared(a,a0));
            Emit("grouped.v3",b.Length(),b.LengthSquared(),Vector3.Distance(b,b0),bd,Vector3.DistanceSquared(b,b0));
            Emit("grouped.v4",c.Length(),c.LengthSquared(),Vector4.Distance(c,c0),cd,Vector4.DistanceSquared(c,c0));
        }
        return 0;
    }
}
