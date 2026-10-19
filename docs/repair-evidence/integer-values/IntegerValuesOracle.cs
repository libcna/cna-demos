// SPDX-License-Identifier: MS-PL
using System;
using Microsoft.Xna.Framework;
class IntegerValuesOracle
{
    static int checks,failures;
    static void Check(bool ok,string label){++checks;if(!ok){++failures;Console.WriteLine("FAIL "+label);}}
    static int Main()
    {
        Console.WriteLine(typeof(Point).Assembly.FullName);
        int[] values={int.MinValue,int.MaxValue,-2,-1,0,1,3,7};
        foreach(int x in values)foreach(int y in values)
            Check(new Point(x,y).GetHashCode()==unchecked(x+y),"point sum "+x+","+y);
        foreach(int x in values)foreach(int y in values)foreach(int w in values)foreach(int h in values)
            Check(new Rectangle(x,y,w,h).GetHashCode()==unchecked(x+y+w+h),"rectangle sum");
        Rectangle[] boxes={new Rectangle(0,0,12,12),new Rectangle(10,10,5,5),new Rectangle(30,40,2,3),
            Rectangle.Empty,new Rectangle(-10,-8,7,3),new Rectangle(-4,-3,-2,-1),new Rectangle(5,5,0,0)};
        foreach(Rectangle first in boxes)foreach(Rectangle second in boxes)
        {
            int x=Math.Min(first.X,second.X),y=Math.Min(first.Y,second.Y);
            Rectangle expected=new Rectangle(x,y,Math.Max(first.Right,second.Right)-x,Math.Max(first.Bottom,second.Bottom)-y);
            Check(Rectangle.Union(first,second)==expected,"union value");
            Rectangle a=first,b=second,r;
            Rectangle.Union(ref a,ref b,out r);Check(r==expected&&a==first&&b==second,"union out and source preservation");
            a=first;b=second;Rectangle.Union(ref a,ref b,out a);Check(a==expected&&b==second,"union left alias");
            a=first;b=second;Rectangle.Union(ref a,ref b,out b);Check(b==expected&&a==first,"union right alias");
            expected=Rectangle.Intersect(first,second);
            a=first;b=second;Rectangle.Intersect(ref a,ref b,out r);Check(r==expected,"intersect out");
            a=first;b=second;Rectangle.Intersect(ref a,ref b,out a);Check(a==expected,"intersect left alias");
            a=first;b=second;Rectangle.Intersect(ref a,ref b,out b);Check(b==expected,"intersect right alias");
        }
        foreach(Rectangle box in boxes){Rectangle a=box;Rectangle.Union(ref a,ref a,out a);Check(a==box,"union triple alias");}
        Console.WriteLine("checks="+checks+" failures="+failures);return failures==0?0:1;
    }
}
