// SPDX-License-Identifier: MS-PL
using System;
using Microsoft.Xna.Framework;
class RangeOracle
{
    static int checks, failures;
    static void Verify<T>(string label, Action call, T[] source, T[] destination, T guard, int si, int di, int length)
    {
        string expected = 2L < (long)si + length || 2L < (long)di + length ? "ArgumentException" :
            length > 0 && (si < 0 || di < 0) ? "IndexOutOfRangeException" : "none";
        string actual = "none";
        try { call(); } catch(Exception ex) { actual = ex.GetType().Name; }
        bool values = true;
        for(int i=0;i<destination.Length;++i)
        {
            T wanted = expected == "none" && length > 0 && i >= di && (long)i < (long)di + length ?
                source[si+i-di] : guard;
            values &= destination[i].Equals(wanted);
        }
        ++checks;
        if(actual!=expected || !values) { ++failures;Console.WriteLine("FAIL "+label+" "+si+","+di+","+length+" expected="+expected+" actual="+actual+" values="+values); }
    }
    static int Main()
    {
        Console.WriteLine(typeof(Vector2).Assembly.FullName);
        Matrix matrix=Matrix.Identity; Quaternion rotation=Quaternion.Identity;
        int[][] cases={new[]{0,0,-1},new[]{2,2,-1},new[]{3,3,-1},new[]{4,0,-1},new[]{0,4,-1},
            new[]{-1,-1,-1},new[]{int.MaxValue,int.MaxValue,int.MinValue},new[]{0,0,int.MinValue},
            new[]{2,2,0},new[]{3,0,0},new[]{0,3,0},new[]{-1,-1,0},new[]{int.MinValue,int.MinValue,0},
            new[]{0,0,1},new[]{1,1,1},new[]{0,0,2},new[]{-1,0,1},new[]{0,-1,1},
            new[]{2,0,1},new[]{0,2,1},new[]{int.MaxValue,0,1},new[]{0,int.MaxValue,1},
            new[]{0,0,int.MaxValue},new[]{int.MinValue,0,int.MaxValue},new[]{0,int.MinValue,int.MaxValue}};
        foreach(int[] c in cases)
        {
            int si=c[0],di=c[1],length=c[2];
            Vector2[] a2={new Vector2(1,2),new Vector2(3,4)};Vector2 guard2=new Vector2(99);
            Vector3[] a3={new Vector3(1,2,3),new Vector3(4,5,6)};Vector3 guard3=new Vector3(99);
            Vector4[] a4={new Vector4(1,2,3,4),new Vector4(5,6,7,8)};Vector4 guard4=new Vector4(99);
            Vector2[] d2={guard2,guard2};
            Verify("v2 matrix",()=>Vector2.Transform(a2,si,ref matrix,d2,di,length),a2,d2,guard2,si,di,length);
            d2=new[]{guard2,guard2};Verify("v2 quaternion",()=>Vector2.Transform(a2,si,ref rotation,d2,di,length),a2,d2,guard2,si,di,length);
            d2=new[]{guard2,guard2};Verify("v2 normal",()=>Vector2.TransformNormal(a2,si,ref matrix,d2,di,length),a2,d2,guard2,si,di,length);
            Vector3[] d3={guard3,guard3};
            Verify("v3 matrix",()=>Vector3.Transform(a3,si,ref matrix,d3,di,length),a3,d3,guard3,si,di,length);
            d3=new[]{guard3,guard3};Verify("v3 quaternion",()=>Vector3.Transform(a3,si,ref rotation,d3,di,length),a3,d3,guard3,si,di,length);
            d3=new[]{guard3,guard3};Verify("v3 normal",()=>Vector3.TransformNormal(a3,si,ref matrix,d3,di,length),a3,d3,guard3,si,di,length);
            Vector4[] d4={guard4,guard4};
            Verify("v4 matrix",()=>Vector4.Transform(a4,si,ref matrix,d4,di,length),a4,d4,guard4,si,di,length);
            d4=new[]{guard4,guard4};Verify("v4 quaternion",()=>Vector4.Transform(a4,si,ref rotation,d4,di,length),a4,d4,guard4,si,di,length);
        }
        Console.WriteLine("checks="+checks+" failures="+failures);return failures==0?0:1;
    }
}
