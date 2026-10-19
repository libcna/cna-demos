// Next math batch, ISSUE-132. Kept separate from the geometry closure evidence.
// Genuine XNA Color uses fixed-point factors, not just special NaN/Infinity branches.
using System;
using Microsoft.Xna.Framework;

static class ColorFixedPointOracle
{
    static int checks, failed;
    static int LerpFactor(float amount)
    {
        float scaled=amount*65536f;
        if(float.IsNaN(scaled)||scaled<0) return 0;
        if(scaled>65536) return 65536;
        return (int)Math.Round(scaled,MidpointRounding.ToEven);
    }
    static Color LerpModel(Color a,Color b,float amount)
    {
        int f=LerpFactor(amount);
        return new Color(a.R+((b.R-a.R)*f>>16),a.G+((b.G-a.G)*f>>16),
                         a.B+((b.B-a.B)*f>>16),a.A+((b.A-a.A)*f>>16));
    }
    static Color MultiplyModel(Color color,float scale)
    {
        float scaled=scale*65536f;
        uint f=float.IsNaN(scaled)||scaled<0 ? 0U : scaled>16777215f ? 16777215U : (uint)scaled;
        return new Color((int)Math.Min(255U,((uint)color.R*f)>>16),
                         (int)Math.Min(255U,((uint)color.G*f)>>16),
                         (int)Math.Min(255U,((uint)color.B*f)>>16),
                         (int)Math.Min(255U,((uint)color.A*f)>>16));
    }
    static void Check(Color actual,Color expected,string name)
    {
        ++checks;
        if(actual!=expected) { ++failed; Console.WriteLine("FAIL "+name+" actual="+actual+" expected="+expected); }
    }
    static int Main()
    {
        Color[] colors={new Color(0,0,0,0),new Color(255,255,255,255),new Color(1,2,3,4),
            new Color(10,20,30,40),new Color(110,220,130,240),new Color(0,1,254,255),
            new Color(255,254,1,0),new Color(127,128,129,130),new Color(23,137,91,253)};
        float[] amounts={float.NaN,float.NegativeInfinity,float.PositiveInfinity,-1,-0.0f,0,
            float.Epsilon,1f/131072f,3f/131072f,0.1f,0.5f,0.99999f,1,1.1f,2,255,256,1e20f};
        foreach(Color a in colors) foreach(float amount in amounts)
        {
            Check(Color.Multiply(a,amount),MultiplyModel(a,amount),"Multiply "+amount);
            Check(a*amount,MultiplyModel(a,amount),"operator "+amount);
            foreach(Color b in colors) Check(Color.Lerp(a,b,amount),LerpModel(a,b,amount),"Lerp "+amount);
        }
        Console.WriteLine("checks="+checks+" failed="+failed);
        return failed==0?0:1;
    }
}
