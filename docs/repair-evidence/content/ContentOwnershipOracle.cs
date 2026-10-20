// Genuine XNA reference probe for ISSUE-011/012/039/045/087 and disposed-load ordering.
// Uses real ContentManager/ContentReader and a small in-memory XNB, not replacement readers.
using System;
using System.Collections.Generic;
using System.IO;
using Microsoft.Xna.Framework.Content;

public sealed class OwnershipItem : IDisposable
{
    public static readonly List<OwnershipItem> Created = new List<OwnershipItem>();
    public readonly string Label;
    public int Disposals;
    public bool ThrowOnDispose;
    public OwnershipItem(string label) { Label = label; Created.Add(this); }
    public void Dispose()
    {
        ++Disposals;
        if (ThrowOnDispose) throw new InvalidOperationException("dispose refusal");
    }
}
public sealed class OwnershipRoot
{
    public OwnershipItem Inline, First, Second, Alias;
}
public sealed class OwnershipItemReader : ContentTypeReader<OwnershipItem>
{
    public static bool ReuseOne;
    public static OwnershipItem Reused;
    public static int FailureMode;
    protected override OwnershipItem Read(ContentReader input, OwnershipItem existing)
    {
        if (FailureMode == 1) throw new IOException("reader IO sentinel");
        if (FailureMode == 2) throw new InvalidOperationException("reader user sentinel");
        string label = input.ReadString();
        if (ReuseOne) return Reused ?? (Reused = new OwnershipItem(label));
        return new OwnershipItem(label);
    }
}
public sealed class OwnershipRootReader : ContentTypeReader<OwnershipRoot>
{
    protected override OwnershipRoot Read(ContentReader input, OwnershipRoot existing)
    {
        var root = new OwnershipRoot { Inline = input.ReadObject<OwnershipItem>() };
        input.ReadSharedResource<OwnershipItem>(value => root.First = value);
        input.ReadSharedResource<OwnershipItem>(value => root.Second = value);
        input.ReadSharedResource<OwnershipItem>(value => root.Alias = value);
        return root;
    }
}
sealed class EmptyOwnershipServices : IServiceProvider
{
    public object GetService(Type type) { return null; }
}
sealed class OwnershipStream : MemoryStream
{
    public bool Closed;
    public OwnershipStream(byte[] bytes) : base(bytes, false) { }
    protected override void Dispose(bool disposing) { Closed = true; base.Dispose(disposing); }
}
sealed class OwnershipManager : ContentManager
{
    readonly byte[] bytes;
    public int Opens;
    public OwnershipStream LastStream;
    public OwnershipManager(byte[] bytes) : base(new EmptyOwnershipServices()) { this.bytes = bytes; }
    protected override Stream OpenStream(string name)
    {
        ++Opens;
        LastStream = new OwnershipStream(bytes);
        return LastStream;
    }
    public OwnershipRoot Raw(Action<IDisposable> recorder)
    {
        return ReadAsset<OwnershipRoot>("asset", recorder);
    }
}
static class ContentOwnershipOracle
{
    static int checks, failures;
    static void Check(bool condition, string label)
    {
        ++checks;
        if (!condition) { ++failures; Console.WriteLine("FAIL " + label); }
    }
    static void Throws<T>(Action operation, string label) where T : Exception
    {
        Exception observed = null;
        try { operation(); } catch (Exception error) { observed = error; }
        Check(observed != null && observed.GetType() == typeof(T), label);
    }
    static byte[] Xnb(bool omitLastShared)
    {
        var body = new MemoryStream();
        var writer = new BinaryWriter(body);
        writer.Write((byte)2); // Small 7-bit encoded reader count.
        writer.Write(typeof(OwnershipRootReader).AssemblyQualifiedName); writer.Write(0);
        writer.Write(typeof(OwnershipItemReader).AssemblyQualifiedName); writer.Write(0);
        writer.Write((byte)2); // Two shared resources.
        writer.Write((byte)1); // Root reader.
        writer.Write((byte)2); writer.Write("inline");
        writer.Write((byte)1); writer.Write((byte)2); writer.Write((byte)1);
        writer.Write((byte)2); writer.Write("first");
        if (!omitLastShared) { writer.Write((byte)2); writer.Write("second"); }
        writer.Flush();
        var file = new MemoryStream();
        var header = new BinaryWriter(file);
        header.Write(new byte[] { 88, 78, 66, 119, 5, 0 });
        header.Write(checked((int)body.Length + 10));
        header.Write(body.ToArray()); header.Flush();
        return file.ToArray();
    }
    static void Reset()
    {
        OwnershipItem.Created.Clear();
        OwnershipItemReader.ReuseOne = false;
        OwnershipItemReader.Reused = null;
        OwnershipItemReader.FailureMode = 0;
    }
    static void Graph(OwnershipRoot root, string label)
    {
        Check(root.Inline.Label == "inline" && root.First.Label == "first" &&
              root.Second.Label == "second", label + " complete payload");
        Check(Object.ReferenceEquals(root.First, root.Alias), label + " shared alias identity");
    }
    static int Main()
    {
        try
        {
            Console.WriteLine(typeof(ContentManager).Assembly.FullName);
            byte[] bytes = Xnb(false);
            Reset();
            using (var manager = new OwnershipManager(bytes))
            {
                var recorded = new List<IDisposable>();
                var root = manager.Raw(recorded.Add);
                Graph(root, "caller recorder");
                Check(recorded.Count == 3, "recorder receives inline and both shared values exactly once");
                Check(recorded.Count == 3 && Object.ReferenceEquals(recorded[0], root.Inline) &&
                      Object.ReferenceEquals(recorded[1], root.First) &&
                      Object.ReferenceEquals(recorded[2], root.Second), "recorder order and identities");
                Check(manager.LastStream.Closed, "successful stream closed");
                manager.Unload();
                Check(root.Inline.Disposals == 0 && root.First.Disposals == 0 && root.Second.Disposals == 0,
                      "custom recorder owns disposal");
                foreach (var item in recorded) item.Dispose();
            }
            Reset();
            using (var manager = new OwnershipManager(bytes))
            {
                var root = manager.Load<OwnershipRoot>("asset");
                Graph(root, "default recorder");
                Check(manager.Opens == 1, "ordinary Load uses OpenStream override");
                Check(Object.ReferenceEquals(root, manager.Load<OwnershipRoot>("asset")) && manager.Opens == 1,
                      "Load cache preserves identity");
                Throws<InvalidOperationException>(() => manager.RootDirectory = "new", "loaded root change refused");
                Throws<InvalidOperationException>(() => manager.RootDirectory = manager.RootDirectory,
                                                  "even same loaded root assignment refused");
                manager.Unload();
                Check(root.Inline.Disposals == 1 && root.First.Disposals == 1 && root.Second.Disposals == 1,
                      "Unload disposes inline and both shared values");
                manager.Unload();
                Check(root.Inline.Disposals == 1 && root.First.Disposals == 1 && root.Second.Disposals == 1,
                      "second Unload has no remaining records");
                manager.RootDirectory = "new";
                Check(manager.RootDirectory == "new", "root can change after Unload");
                Check(!Object.ReferenceEquals(root, manager.Load<OwnershipRoot>("asset")) && manager.Opens == 2,
                      "Unload clears loaded cache");
            }
            Reset();
            using (var manager = new OwnershipManager(bytes))
            {
                OwnershipItemReader.ReuseOne = true;
                var root = manager.Raw(null);
                Check(Object.ReferenceEquals(root.Inline, root.First) && Object.ReferenceEquals(root.First, root.Second),
                      "reader can return one identity for separate materializations");
                manager.RootDirectory = "uncached";
                Check(manager.RootDirectory == "uncached", "ReadAsset does not populate loaded cache");
                manager.Unload();
                Check(root.Inline.Disposals == 3, "default recording is per materialization, not identity-deduplicated");
            }
            Reset();
            using (var manager = new OwnershipManager(Xnb(true)))
            {
                Throws<ContentLoadException>(() => manager.Raw(null), "truncated shared payload wrapped");
                Check(manager.LastStream.Closed, "failed stream closed");
                Check(OwnershipItem.Created.Count == 2, "partial read materialized two resources");
                manager.Unload();
                Check(OwnershipItem.Created.Count == 2 && OwnershipItem.Created.TrueForAll(x => x.Disposals == 1),
                      "failed read retains default disposal records");
            }
            Reset();
            using (var manager = new OwnershipManager(bytes))
            {
                var root = manager.Load<OwnershipRoot>("asset");
                root.Inline.ThrowOnDispose = true;
                Throws<InvalidOperationException>(manager.Unload, "Unload propagates disposal failure");
                Check(root.Inline.Disposals == 1 && root.First.Disposals == 0 && root.Second.Disposals == 0,
                      "Unload stops at first disposal failure");
                manager.Unload();
                Check(root.Inline.Disposals == 1 && root.First.Disposals == 0,
                      "failed Unload still clears all records");
                manager.RootDirectory = "cleared";
                Check(manager.RootDirectory == "cleared", "failed Unload also clears loaded cache");
            }
            Reset();
            using (var manager = new OwnershipManager(bytes))
            {
                IDisposable captured = null;
                Throws<InvalidOperationException>(() => manager.Raw(value => {
                    captured = value;
                    throw new InvalidOperationException("caller recorder refusal");
                }), "caller recorder exception is not wrapped as an I/O failure");
                Check(captured != null && OwnershipItem.Created.Count == 1, "refused callback receives inline object");
                Check(manager.LastStream.Closed, "callback refusal closes stream");
                manager.Unload();
                Check(OwnershipItem.Created.Count == 1 && OwnershipItem.Created[0].Disposals == 0,
                      "callback refusal does not transfer lifetime back to the manager");
                if (captured != null) captured.Dispose();
            }
            using (var manager = new OwnershipManager(new byte[] { 88, 78, 66 }))
            {
                Throws<ContentLoadException>(() => manager.Load<OwnershipRoot>("short"), "truncated XNB header wrapped");
                Check(manager.LastStream.Closed, "header refusal closes stream");
                manager.Dispose();
                Throws<ObjectDisposedException>(() => manager.Load<OwnershipRoot>(""), "disposed Load precedes empty-name check");
                Throws<ObjectDisposedException>(manager.Unload, "Unload after Dispose refused");
            }
            for (int length = 0; length < 10; ++length)
            {
                byte[] shortHeader = new byte[length];
                Array.Copy(bytes, shortHeader, length);
                using (var manager = new OwnershipManager(shortHeader))
                {
                    Throws<ContentLoadException>(() => manager.Load<OwnershipRoot>("short"),
                                                 "Load wraps header length " + length);
                    Check(manager.LastStream.Closed, "Load closes header length " + length);
                    Throws<ContentLoadException>(() => manager.Raw(null),
                                                 "ReadAsset wraps header length " + length);
                    Check(manager.LastStream.Closed, "ReadAsset closes header length " + length);
                    manager.RootDirectory = "after-refusal";
                    Check(manager.RootDirectory == "after-refusal", "failed Load leaves root mutable " + length);
                }
            }
            for (int mode = 1; mode <= 2; ++mode)
            {
                Reset();
                OwnershipItemReader.FailureMode = mode;
                using (var manager = new OwnershipManager(bytes))
                {
                    if (mode == 1)
                        Throws<ContentLoadException>(() => manager.Raw(null), "reader IOException wrapped");
                    else
                        Throws<InvalidOperationException>(() => manager.Raw(null), "reader user exception preserved");
                    Check(manager.LastStream.Closed, "reader refusal closes stream mode " + mode);
                }
            }
            Console.WriteLine("checks=" + checks + " failed=" + failures);
            return failures == 0 ? 0 : 1;
        }
        catch (Exception error) { Console.WriteLine("SETUP/BEHAVIOR " + error); return 2; }
    }
}
