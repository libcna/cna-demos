// Genuine XNA4 probe: typed/erased/raw dispatch, existing-instance identity and disposal recording.
using System;
using System.Collections.Generic;
using System.IO;
using Microsoft.Xna.Framework.Content;

public sealed class DispatchItem : IDisposable
{
    public int Value;
    public void Dispose() { }
}
public sealed class DispatchRoot { public object Item; }
public sealed class DispatchItemReader : ContentTypeReader<DispatchItem>
{
    public static bool ReplaceExisting, ReturnNull;
    protected override DispatchItem Read(ContentReader input, DispatchItem existing)
    {
        int value = input.ReadInt32();
        if (ReturnNull) return null;
        var result = existing != null && !ReplaceExisting ? existing : new DispatchItem();
        result.Value = value;
        return result;
    }
}
public sealed class DispatchRootReader : ContentTypeReader<DispatchRoot>
{
    public static int Mode;
    public static DispatchItem Existing;
    ContentTypeReader itemReader;
    protected override void Initialize(ContentTypeReaderManager manager)
    {
        itemReader = manager.GetTypeReader(typeof(DispatchItem));
    }
    protected override DispatchRoot Read(ContentReader input, DispatchRoot existing)
    {
        object item;
        switch (Mode)
        {
            case 0: item = input.ReadObject<DispatchItem>(); break;
            case 1: item = input.ReadObject<DispatchItem>(Existing); break;
            case 2: item = input.ReadRawObject<DispatchItem>(); break;
            case 3: item = input.ReadRawObject<DispatchItem>(itemReader); break;
            case 4: item = input.ReadRawObject<DispatchItem>(Existing); break;
            case 5: item = input.ReadRawObject<DispatchItem>(itemReader, Existing); break;
            case 6: item = input.ReadObject<object>(); break;
            case 7: item = input.ReadObject<DispatchItem>(itemReader); break;
            case 8: item = input.ReadObject<DispatchItem>(itemReader, Existing); break;
            case 9: item = input.ReadObject<DispatchItem>(Existing); break;
            default: throw new InvalidOperationException("unknown dispatch mode");
        }
        return new DispatchRoot { Item = item };
    }
}
sealed class DispatchServices : IServiceProvider
{
    public object GetService(Type type) { return null; }
}
sealed class DispatchManager : ContentManager
{
    readonly byte[] bytes;
    public DispatchManager(byte[] bytes) : base(new DispatchServices()) { this.bytes = bytes; }
    protected override Stream OpenStream(string name) { return new MemoryStream(bytes, false); }
    public DispatchRoot Raw(Action<IDisposable> record) { return ReadAsset<DispatchRoot>("asset", record); }
}
static class ContentDispatchOracle
{
    static int checks, failures;
    static void Check(bool condition, string label)
    {
        ++checks;
        Console.WriteLine((condition ? "PASS " : "FAIL ") + label);
        if (!condition) ++failures;
    }
    static bool HasExisting(int mode) { return mode == 1 || mode == 4 || mode == 5 || mode == 8; }
    static byte[] Xnb(int mode)
    {
        var body = new MemoryStream();
        var writer = new BinaryWriter(body);
        writer.Write((byte)2);
        writer.Write(typeof(DispatchRootReader).AssemblyQualifiedName); writer.Write(0);
        writer.Write(typeof(DispatchItemReader).AssemblyQualifiedName); writer.Write(0);
        writer.Write((byte)0); // No shared resources.
        writer.Write((byte)1); // Root reader.
        if (mode < 2 || mode >= 6) writer.Write((byte)(mode == 9 ? 0 : 2));
        if (mode != 9) writer.Write(73);
        writer.Flush();
        var file = new MemoryStream();
        var header = new BinaryWriter(file);
        header.Write(new byte[] { 88, 78, 66, 119, 5, 0 });
        header.Write(checked((int)body.Length + 10));
        header.Write(body.ToArray()); header.Flush();
        return file.ToArray();
    }
    static int Main()
    {
        try
        {
            Console.WriteLine(typeof(ContentReader).Assembly.FullName);
            for (int mode = 0; mode <= 8; ++mode)
            {
                DispatchRootReader.Mode = mode;
                DispatchRootReader.Existing = new DispatchItem { Value = -1 };
                var recorded = new List<IDisposable>();
                using (var manager = new DispatchManager(Xnb(mode)))
                {
                    var root = manager.Raw(recorded.Add);
                    Check(root.Item is DispatchItem && ((DispatchItem)root.Item).Value == 73,
                          "mode " + mode + " decodes exact payload");
                    Check(HasExisting(mode) ? Object.ReferenceEquals(root.Item, DispatchRootReader.Existing) :
                          !Object.ReferenceEquals(root.Item, DispatchRootReader.Existing),
                          "mode " + mode + " preserves expected identity");
                    Check(HasExisting(mode) ? recorded.Count == 0 :
                          recorded.Count == 1 && Object.ReferenceEquals(recorded[0], root.Item),
                          "mode " + mode + " records only a new disposable");
                }
            }
            foreach (int mode in new int[] { 1, 4, 5, 8 })
            {
                DispatchRootReader.Mode = mode;
                DispatchItemReader.ReplaceExisting = true;
                var recorded = new List<IDisposable>();
                using (var manager = new DispatchManager(Xnb(mode)))
                {
                    Exception observed = null;
                    try { manager.Raw(recorded.Add); } catch (Exception error) { observed = error; }
                    Check(observed != null && observed.GetType() == typeof(InvalidOperationException),
                          "mode " + mode + " rejects replacement of existing instance");
                    Check(recorded.Count == 0, "mode " + mode + " does not record rejected replacement");
                }
            }
            DispatchItemReader.ReplaceExisting = false;
            DispatchRootReader.Mode = 9;
            using (var manager = new DispatchManager(Xnb(9)))
            {
                var recorded = new List<IDisposable>();
                Check(manager.Raw(recorded.Add).Item == null, "null type index does not return existing instance");
                Check(recorded.Count == 0, "null type index records nothing");
            }
            DispatchRootReader.Mode = 0;
            DispatchItemReader.ReturnNull = true;
            using (var manager = new DispatchManager(Xnb(0)))
            {
                var recorded = new List<IDisposable>();
                Check(manager.Raw(recorded.Add).Item == null, "reader may return null for a new instance");
                Check(recorded.Count == 0, "reader-produced null records nothing");
            }
            Console.WriteLine("checks=" + checks + " failed=" + failures);
            return failures == 0 ? 0 : 1;
        }
        catch (Exception error) { Console.WriteLine("SETUP/BEHAVIOR " + error); return 2; }
    }
}
