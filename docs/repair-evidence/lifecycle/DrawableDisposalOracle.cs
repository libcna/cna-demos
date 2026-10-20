// Genuine derived-component companion to ISSUE-141; no Run or graphics device.
using System;
using System.Collections.Generic;
using Microsoft.Xna.Framework;

static class DrawableDisposalOracle
{
    sealed class Item : DrawableGameComponent
    {
        public readonly List<string> Trace=new List<string>();
        public bool ThrowOnce;
        public Item(Game game) : base(game) { Disposed+=(s,e)=>Trace.Add("event"); }
        public void FinalizerPath() { Dispose(false); }
        protected override void UnloadContent()
        {
            Trace.Add("unload");
            if(ThrowOnce) { ThrowOnce=false; throw new InvalidOperationException("unload"); }
        }
    }
    static int checks,failed;
    static void Check(bool ok,string name) { ++checks;if(!ok){++failed;Console.WriteLine("FAIL "+name);} }
    static int Main()
    {
        using(var game=new Game()) {
            var item=new Item(game);
            item.FinalizerPath();
            Check(item.Trace.Count==0,"false path is silent before Initialize");
            item.Dispose();item.Dispose();
            Check(string.Join(",",item.Trace)=="unload,event,unload,event","each explicit call unloads before event even without Initialize");
            var throwing=new Item(game);throwing.ThrowOnce=true;
            bool caught=false;
            try { throwing.Dispose(); } catch(InvalidOperationException) { caught=true; }
            Check(caught && string.Join(",",throwing.Trace)=="unload","unload exception precedes base event");
            throwing.Dispose();
            Check(string.Join(",",throwing.Trace)=="unload,unload,event","later explicit call still unloads and raises event");
        }
        Console.WriteLine("checks="+checks+" failed="+failed);
        return failed==0?0:1;
    }
}
