// Genuine live-Game companion for ISSUE-141; no Run or graphics device creation.
using System;
using System.Collections.Generic;
using Microsoft.Xna.Framework;

static class ComponentDisposalOracle
{
    static int checks,failed;
    static void Check(bool ok,string name) { ++checks; if(!ok){++failed;Console.WriteLine("FAIL "+name);} }
    static int Main()
    {
        using(var game=new Game()) {
            var c=new GameComponent(game);
            game.Components.Add(c);
            var trace=new List<string>();
            game.Components.ComponentRemoved += (s,e)=> {
                Check(e.GameComponent==c && game.Components.Count==0,"remove precedes notification");
                trace.Add("remove");
            };
            c.Disposed += (s,e)=> {
                Check(s==c && e==EventArgs.Empty && c.Game==game && game.Components.Count==0,"disposed observes removed component");
                trace.Add("dispose");
            };
            c.Dispose(); c.Dispose();
            Check(string.Join(",",trace)=="remove,dispose,dispose","self removal precedes repeated events");
        }
        using(var game=new Game()) {
            var first=new GameComponent(game); var second=new GameComponent(game); var third=new GameComponent(game);
            game.Components.Add(first); game.Components.Add(second); game.Components.Add(third);
            var trace=new List<string>();
            first.Disposed += (s,e)=> { trace.Add("first"); game.Components.Remove(second); };
            second.Disposed += (s,e)=>trace.Add("second");
            third.Disposed += (s,e)=>trace.Add("third");
            game.Dispose();
            Check(string.Join(",",trace)=="first,second,third","Game disposes original snapshot despite removal");
            Check(game.Components.Count==0,"snapshot disposal leaves collection empty");
        }
        var reentrant=new GameComponent(null);
        int count=0;
        reentrant.Disposed += (s,e)=> { if(++count==1) reentrant.Dispose(); };
        reentrant.Dispose();
        Check(count==2,"explicit reentrant disposal follows caller guard");
        var throwing=new GameComponent(null);
        count=0;
        throwing.Disposed += (s,e)=> { if(++count==1)throw new InvalidOperationException("handler"); };
        bool caught=false;
        try { throwing.Dispose(); } catch(InvalidOperationException) { caught=true; }
        throwing.Dispose();
        Check(caught && count==2,"handler exception does not suppress subsequent disposal event");
        Console.WriteLine("checks="+checks+" failed="+failed);
        return failed==0?0:1;
    }
}
