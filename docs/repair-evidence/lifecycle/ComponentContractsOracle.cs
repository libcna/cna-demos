// Genuine XNA 4.0 reproduction for 141/142/143/144; no graphics device required.
using System;
using System.Collections.Generic;
using System.Reflection;
using Microsoft.Xna.Framework;

static class ComponentContractsOracle
{
    static int checks, failed;
    static void Check(bool value, string name)
    {
        ++checks;
        if (!value) { ++failed; Console.WriteLine("FAIL " + name); }
    }
    sealed class Component : DrawableGameComponent
    {
        public readonly List<string> Trace = new List<string>();
        public bool CallBase = true;
        public Component() : base(null) {}
        protected override void OnEnabledChanged(object sender, EventArgs e)
        { Trace.Add("enabled hook"); if (CallBase) base.OnEnabledChanged(sender,e); }
        protected override void OnUpdateOrderChanged(object sender, EventArgs e)
        { Trace.Add("update hook"); if (CallBase) base.OnUpdateOrderChanged(sender,e); }
        protected override void OnVisibleChanged(object sender, EventArgs e)
        { Trace.Add("visible hook"); if (CallBase) base.OnVisibleChanged(sender,e); }
        protected override void OnDrawOrderChanged(object sender, EventArgs e)
        { Trace.Add("draw hook"); if (CallBase) base.OnDrawOrderChanged(sender,e); }
        public void FinalizePath() { base.Dispose(false); }
        public void BaseEnabledHook(object sender, EventArgs e) { base.OnEnabledChanged(sender,e); }
    }
    sealed class Stub : IGameComponent { public void Initialize() {} }
    static void Expect<T>(Action action,string name) where T : Exception
    {
        Type caught = null;
        try { action(); } catch (Exception e) { caught = e.GetType(); }
        Check(caught == typeof(T), name);
    }
    static int Main()
    {
        var c = new Component();
        c.EnabledChanged += (s,e) => { Check(s==c && e==EventArgs.Empty,"enabled identity"); c.Trace.Add("enabled event"); };
        c.UpdateOrderChanged += (s,e) => { Check(s==c && e==EventArgs.Empty,"update identity"); c.Trace.Add("update event"); };
        c.VisibleChanged += (s,e) => { Check(s==c && e==EventArgs.Empty,"visible identity"); c.Trace.Add("visible event"); };
        c.DrawOrderChanged += (s,e) => { Check(s==c && e==EventArgs.Empty,"draw identity"); c.Trace.Add("draw event"); };
        c.Enabled=false; c.UpdateOrder=1; c.Visible=false; c.DrawOrder=1;
        Check(string.Join(",",c.Trace)=="enabled hook,enabled event,update hook,update event,visible hook,visible event,draw hook,draw event","hooks dispatch events");
        c.Trace.Clear(); c.Enabled=false; c.UpdateOrder=1; c.Visible=false; c.DrawOrder=1;
        Check(c.Trace.Count==0,"equal values silent");
        c.CallBase=false; c.Enabled=true; c.UpdateOrder=2; c.Visible=true; c.DrawOrder=2;
        Check(string.Join(",",c.Trace)=="enabled hook,update hook,visible hook,draw hook","override suppresses events");
        c.Trace.Clear(); c.BaseEnabledHook(new object(),EventArgs.Empty);
        Check(string.Join(",",c.Trace)=="enabled event","base hook uses component sender");
        int disposals=0;
        c.Disposed += (s,e) => { Check(s==c && e==EventArgs.Empty,"disposed identity"); ++disposals; };
        c.Dispose(); c.Dispose(); c.FinalizePath();
        Check(disposals==2,"each explicit Dispose dispatches; finalizer path silent");

        var collection = new GameComponentCollection();
        var first = new Stub(); var second = new Stub();
        collection.Add(first);
        Expect<ArgumentOutOfRangeException>(()=>collection.Insert(2,first),"invalid duplicate index first");
        Expect<ArgumentOutOfRangeException>(()=>collection.Insert(2,second),"invalid unique index");
        Expect<ArgumentException>(()=>collection.Insert(0,first),"valid duplicate index");
        Check(collection.Count==1 && collection[0]==first,"rejected inserts atomic");
        int added=0, removed=0;
        collection.ComponentAdded += (s,e)=>++added;
        collection.ComponentRemoved += (s,e)=>++removed;
        collection.Add(null); collection.Remove(null);
        Check(added==0 && removed==0,"null Add and Remove silent");
        collection.Add(null); collection.Add(second);
        var removal = new List<IGameComponent>();
        collection.ComponentRemoved += (s,e)=> {
            Check(s==collection && collection.Count==3,"Clear observes original collection");
            removal.Add(e.GameComponent);
        };
        collection.Clear();
        Check(removal.Count==3 && removal[0]==first && removal[1]==null && removal[2]==second,"Clear forward/null events");
        Check(collection.Count==0 && removed==3,"Clear removes all");

        var parameters = new LaunchParameters(); parameters.Clear();
        var parse = typeof(LaunchParameters).GetMethod("ParseCommandLineArguments",BindingFlags.Instance|BindingFlags.NonPublic);
        parse.Invoke(parameters,new object[]{new[]{"ignored.exe","/level:3","-difficulty:hard","--windowed","/level:9","plain:value","/mixed:a:b","-Case:Upper","-case:lower","/empty:",":ignored","---",""}});
        Check(parameters.Count==8,"eight nonempty argument keys, skip argv0");
        Check(parameters["level"]=="3" && parameters["mixed"]=="a:b","first duplicate and first colon");
        Check(parameters["windowed"]=="" && parameters["empty"]=="","empty values retained");
        Check(parameters["Case"]=="Upper" && parameters["case"]=="lower","case-sensitive keys");
        Expect<ArgumentException>(()=>parameters.Add("level","9"),"direct duplicate Add");
        Check(parameters["level"]=="3","duplicate value unchanged");
        parameters.Add("","empty key"); parameters.Add("blank","");
        Check(parameters[""]=="empty key" && parameters["blank"]=="","direct empty keys and values");
        Console.WriteLine("checks="+checks+" failed="+failed);
        return failed==0?0:1;
    }
}
