// Genuine XNA cache/GraphicsResource metadata probe. No graphics device is required:
// a custom reader creates an ordinary unbound BlendState through its real constructor.
using System;
using System.IO;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.Graphics;

public sealed class MetadataStateReader : ContentTypeReader<BlendState>
{
    protected override BlendState Read(ContentReader input, BlendState existing)
    {
        return new BlendState();
    }
}
sealed class MetadataServices : IServiceProvider
{
    public object GetService(Type type) { return null; }
}
sealed class MetadataManager : ContentManager
{
    readonly byte[] bytes;
    public int Opens;
    public MetadataManager(byte[] bytes) : base(new MetadataServices()) { this.bytes = bytes; }
    protected override Stream OpenStream(string name) { ++Opens; return new MemoryStream(bytes, false); }
}
static class ContentMetadataOracle
{
    static int checks, failed;
    static void Check(bool condition, string label)
    {
        ++checks;
        Console.WriteLine((condition ? "PASS " : "FAIL ") + label);
        if (!condition) ++failed;
    }
    static byte[] Xnb()
    {
        var body = new MemoryStream();
        var writer = new BinaryWriter(body);
        writer.Write((byte)1);
        writer.Write(typeof(MetadataStateReader).AssemblyQualifiedName);
        writer.Write(0);
        writer.Write((byte)0);
        writer.Write((byte)1);
        writer.Flush();
        var file = new MemoryStream();
        var header = new BinaryWriter(file);
        header.Write(new byte[] { 88, 78, 66, 119, 5, 0 });
        header.Write(checked((int)body.Length + 10));
        header.Write(body.ToArray());
        header.Flush();
        return file.ToArray();
    }
    static int Main()
    {
        try
        {
            Console.WriteLine(typeof(ContentManager).Assembly.FullName);
            Console.WriteLine(typeof(GraphicsResource).Assembly.FullName);
            byte[] bytes = Xnb();
            using (var manager = new MetadataManager(bytes))
            using (var other = new MetadataManager(bytes))
            {
                var first = manager.Load<BlendState>("asset");
                var separate = other.Load<BlendState>("asset");
                string initialName = separate.Name;
                object tagA = new object(), tagB = new object();
                first.Name = "first"; first.Tag = tagA;
                var second = manager.Load<BlendState>("asset");
                Check(Object.ReferenceEquals(first, second), "cached identity");
                Check(manager.Opens == 1, "single read");
                Check(second.Name == "first", "cached name");
                Check(Object.ReferenceEquals(second.Tag, tagA), "cached tag");
                GraphicsResource writer = second;
                writer.Name = "second"; writer.Tag = tagB;
                Check(first.Name == "second", "reverse name");
                Check(Object.ReferenceEquals(first.Tag, tagB), "reverse tag");
                var third = manager.Load<BlendState>("asset");
                Check(third.Name == "second", "later cached name");
                Check(Object.ReferenceEquals(third.Tag, tagB), "later cached tag");
                Check(manager.Opens == 1, "later cache avoids read");
                second.Name = ""; second.Tag = null;
                Check(first.Name == "" && third.Name == "", "empty name propagates");
                Check(first.Tag == null && third.Tag == null, "null tag propagates");
                Check(separate.Name == initialName && separate.Tag == null, "other manager independent");
                first.Name = "old lifetime"; first.Tag = tagA;
                manager.Unload();
                var reloaded = manager.Load<BlendState>("asset");
                Check(!Object.ReferenceEquals(first, reloaded) && manager.Opens == 2, "reload new identity");
                Check(reloaded.Name == initialName && reloaded.Tag == null, "reload new metadata");
                reloaded.Name = "new lifetime";
                Check(first.Name == "old lifetime" && Object.ReferenceEquals(first.Tag, tagA), "old metadata retained");
            }
        }
        catch (Exception error) { Console.WriteLine(error); return 2; }
        Console.WriteLine("checks=" + checks + " failed=" + failed);
        return failed == 0 ? 0 : 1;
    }
}
