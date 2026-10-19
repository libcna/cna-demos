// Genuine ClearItems uses a live index loop, not a snapshot or fail-fast enumerator.
using System;
using System.Collections.Generic;
using Microsoft.Xna.Framework;
static class CollectionMutationOracle
{
    sealed class Item : IGameComponent { public void Initialize() {} }
    static int Main()
    {
        int checks=0, failed=0;
        Action<bool,string> check=(ok,name)=>{++checks;if(!ok){++failed;Console.WriteLine("FAIL "+name);}};
        var c=new GameComponentCollection();
        var a=new Item(); var b=new Item(); var d=new Item();
        c.Add(a);c.Add(b);c.Add(d);
        var trace=new List<IGameComponent>();
        bool removed=false;
        c.ComponentRemoved+=(s,e)=> {
            trace.Add(e.GameComponent);
            if(!removed && e.GameComponent==a) { removed=true;check(c.Remove(a),"nested Remove"); }
        };
        c.Clear();
        check(trace.Count==3 && trace[0]==a && trace[1]==a && trace[2]==d,"live-index Clear trace");
        check(c.Count==0,"Clear final empty collection");
        Console.WriteLine("checks="+checks+" failed="+failed);
        return failed==0?0:1;
    }
}
