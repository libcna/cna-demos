// SPDX-License-Identifier: MS-PL
using System;
using System.Reflection;
using Microsoft.Xna.Framework;
class MagnitudeEntryOracle
{
    static int checks,failures;
    static uint Bits(float v){return BitConverter.ToUInt32(BitConverter.GetBytes(v),0);}
    static float Round(double v){return BitConverter.ToSingle(BitConverter.GetBytes((float)v),0);}
    static void Check(float actual,float expected,string label)
    {
        ++checks;
        if((float.IsNaN(actual)&&float.IsNaN(expected))||Bits(actual)==Bits(expected))return;
        ++failures;if(failures<=30)Console.WriteLine("FAIL "+label+" actual="+Bits(actual).ToString("X8")+" expected="+Bits(expected).ToString("X8"));
    }
    static float[] Fields(object value)
    {
        if(value is Vector2){Vector2 v=(Vector2)value;return new[]{v.X,v.Y};}
        if(value is Vector3){Vector3 v=(Vector3)value;return new[]{v.X,v.Y,v.Z};}
        if(value is Vector4){Vector4 v=(Vector4)value;return new[]{v.X,v.Y,v.Z,v.W};}
        Quaternion q=(Quaternion)value;return new[]{q.X,q.Y,q.Z,q.W};
    }
    static void CheckFields(object actual,float[] expected,string label)
    {float[] values=Fields(actual);for(int i=0;i<values.Length;++i)Check(values[i],expected[i],label+"/"+i);}
    static object Value(Type type,Vector4 v)
    {
        if(type==typeof(Vector2))return new Vector2(v.X,v.Y);
        if(type==typeof(Vector3))return new Vector3(v.X,v.Y,v.Z);
        if(type==typeof(Vector4))return v;
        return new Quaternion(v.X,v.Y,v.Z,v.W);
    }
    static void Test(Type type,Vector4[] samples)
    {
        MethodInfo length=type.GetMethod("Length",Type.EmptyTypes);
        MethodInfo value=type.GetMethod("Normalize",new[]{type});
        MethodInfo output=type.GetMethod("Normalize",new[]{type.MakeByRefType(),type.MakeByRefType()});
        MethodInfo member=type.GetMethod("Normalize",Type.EmptyTypes);
        foreach(Vector4 sample in samples)
        {
            object source=Value(type,sample);float[] original=Fields(source);
            double squared=0;foreach(float f in original)squared+=(double)f*f;
            float expectedLength=Round(Math.Sqrt(squared));
            Check((float)length.Invoke(source,null),expectedLength,type.Name+" Length");
            double inverse=1.0/(double)expectedLength;
            float[] expected=new float[original.Length];for(int i=0;i<original.Length;++i)expected[i]=Round(original[i]*inverse);
            CheckFields(value.Invoke(null,new[]{source}),expected,type.Name+" Normalize value");
            object[] args={source,Activator.CreateInstance(type)};output.Invoke(null,args);
            CheckFields(args[1],expected,type.Name+" Normalize out");CheckFields(args[0],original,"source unchanged");
            object boxed=Value(type,sample);member.Invoke(boxed,null);CheckFields(boxed,expected,type.Name+" Normalize member");
        }
        foreach(float magnitude in new[]{1e20f,1e-30f})
        {
            object source=Value(type,new Vector4(magnitude,0,0,0));
            float squared=(float)type.GetMethod("LengthSquared",Type.EmptyTypes).Invoke(source,null);
            Check(squared,magnitude>1?float.PositiveInfinity:0,type.Name+" separately returned LengthSquared");
        }
    }
    static int Main()
    {
        Console.WriteLine(typeof(Vector2).Assembly.FullName);
        Vector4[] samples={Vector4.Zero,new Vector4(-0.0f),Vector4.One,Vector4.UnitX,Vector4.UnitW,
            new Vector4(3,4,0,0),new Vector4(1,2,3,4),new Vector4(0.3f,-0.6f,0.8f,0.1f),
            new Vector4(0,1,0.8f,0),new Vector4(0.8414709568023682f,0.5403022766113281f,0,0),
            new Vector4(1e20f,0,0,0),new Vector4(-1e20f,0,0,0),new Vector4(1e20f,1e20f,1e20f,1e20f),
            new Vector4(1e-30f,0,0,0),new Vector4(-1e-30f,0,0,0),new Vector4(1e-30f,1e-30f,1e-30f,1e-30f),
            new Vector4(float.Epsilon,0,0,0),new Vector4(float.MaxValue,0,0,0),new Vector4(float.MaxValue),
            new Vector4(float.PositiveInfinity,0,0,0),new Vector4(float.NaN,0,0,0)};
        foreach(Type type in new[]{typeof(Vector2),typeof(Vector3),typeof(Vector4),typeof(Quaternion)})Test(type,samples);
        Console.WriteLine("checks="+checks+" failures="+failures);return failures==0?0:1;
    }
}
