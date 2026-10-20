// ISSUE-012: genuine XNA ResourceContentManager without a Game or graphics service.
using System;
using System.Globalization;
using System.IO;
using System.Resources;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Content;

sealed class NoServices : IServiceProvider
{
    public object GetService(Type type) { return null; }
}
sealed class MemoryCurveResources : ResourceManager
{
    readonly byte[] data;
    public int Lookups;
    public MemoryCurveResources(byte[] bytes) { data = bytes; }
    public override object GetObject(string name)
    {
        ++Lookups;
        return name == "curve" ? data : null;
    }
    public override object GetObject(string name, CultureInfo culture) { return GetObject(name); }
}
static class ResourceContentColdOracle
{
    static int checks, failed;
    static void Check(bool condition, string label)
    {
        ++checks;
        if (!condition) ++failed;
        Console.WriteLine((condition ? "PASS " : "FAIL ") + label);
    }
    static byte[] CurveXnb()
    {
        byte[] payload;
        using (var body = new MemoryStream())
        using (var writer = new BinaryWriter(body))
        {
            writer.Write((byte)1);
            writer.Write("Microsoft.Xna.Framework.Content.CurveReader, " + typeof(Curve).Assembly.FullName);
            writer.Write(0);
            writer.Write((byte)0);
            writer.Write((byte)1);
            writer.Write(0); writer.Write(0); writer.Write(1);
            writer.Write(0.0f); writer.Write(42.0f); writer.Write(0.0f); writer.Write(0.0f);
            writer.Write(0);
            writer.Flush();
            payload = body.ToArray();
        }
        using (var file = new MemoryStream())
        using (var writer = new BinaryWriter(file))
        {
            writer.Write(new byte[] { (byte)'X', (byte)'N', (byte)'B', (byte)'w', 5, 0 });
            writer.Write(payload.Length + 10);
            writer.Write(payload);
            writer.Flush();
            return file.ToArray();
        }
    }
    static int Main()
    {
        try
        {
            Console.WriteLine(typeof(ContentManager).Assembly.FullName);
            var resources = new MemoryCurveResources(CurveXnb());
            using (var manager = new ResourceContentManager(new NoServices(), resources))
            {
                var first = manager.Load<Curve>("curve");
                Check(first != null, "cold resource load succeeds without Game or graphics service");
                Check(first.Keys.Count == 1, "one authored curve key");
                Check(first.PreLoop == CurveLoopType.Constant && first.PostLoop == CurveLoopType.Constant, "exact authored loop modes");
                Check(first.Evaluate(0) == 42, "exact authored key value");
                Check(first.Evaluate(-10) == 42 && first.Evaluate(10) == 42, "constant extrapolation preserves value");
                Check(Object.ReferenceEquals(first, manager.Load<Curve>("curve")), "cached resource preserves managed identity");
                Check(resources.Lookups == 1, "cached resource does not reopen resource provider");
                manager.Unload();
                var second = manager.Load<Curve>("curve");
                Check(!Object.ReferenceEquals(first, second), "Unload causes a fresh resource decode");
                Check(second.Evaluate(0) == 42, "reloaded resource retains authored value");
                Check(resources.Lookups == 2, "reload reopens provider exactly once");
                Exception missing = null;
                try { manager.Load<Curve>("missing"); }
                catch (Exception error) { missing = error; }
                Check(missing != null && missing.GetType() == typeof(ContentLoadException), "missing resource has exact ContentLoadException type");
                Check(resources.Lookups == 3, "missing resource probes provider once");
                Check(Object.ReferenceEquals(second, manager.Load<Curve>("curve")), "failed other load preserves successful cache");
                Check(resources.Lookups == 3, "failed other load does not reopen cached resource");
            }
            Console.WriteLine("checks=" + checks + " failed=" + failed);
            return checks == 14 && failed == 0 ? 0 : 1;
        }
        catch (Exception error) { Console.WriteLine("UNEXPECTED " + error); return 2; }
    }
}
