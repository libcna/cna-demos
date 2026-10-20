// Genuine XNA4 reference for ISSUE-043: reader class and target type have distinct names.
using System;
using System.IO;
using Microsoft.Xna.Framework.Content;

public sealed class LookupOracleItem { public int Value; }
public sealed class LookupOracleRoot { public LookupOracleItem Item; }
public sealed class LookupOracleItemReader : ContentTypeReader<LookupOracleItem>
{
    protected override LookupOracleItem Read(ContentReader input, LookupOracleItem existing)
    {
        return new LookupOracleItem { Value = input.ReadInt32() };
    }
}
public sealed class LookupOracleRootReader : ContentTypeReader<LookupOracleRoot>
{
    public static ContentTypeReader Resolved;
    public static bool SelfResolved;
    public static bool SameLookupIdentity;
    protected override void Initialize(ContentTypeReaderManager manager)
    {
        Resolved = manager.GetTypeReader(typeof(LookupOracleItem));
        SelfResolved = Object.ReferenceEquals(this, manager.GetTypeReader(typeof(LookupOracleRoot)));
        SameLookupIdentity = Object.ReferenceEquals(Resolved, manager.GetTypeReader(typeof(LookupOracleItem)));
    }
    protected override LookupOracleRoot Read(ContentReader input, LookupOracleRoot existing)
    {
        return new LookupOracleRoot { Item = input.ReadRawObject<LookupOracleItem>() };
    }
}
sealed class LookupOracleServices : IServiceProvider
{
    public object GetService(Type type) { return null; }
}
sealed class LookupOracleManager : ContentManager
{
    readonly byte[] bytes;
    public LookupOracleManager(byte[] bytes) : base(new LookupOracleServices()) { this.bytes = bytes; }
    protected override Stream OpenStream(string assetName) { return new MemoryStream(bytes, false); }
}
static class ReaderTargetLookupOracle
{
    static int checks, failures;
    static void Check(bool value, string label)
    {
        ++checks;
        Console.WriteLine((value ? "PASS " : "FAIL ") + label);
        if (!value) ++failures;
    }
    static byte[] Xnb()
    {
        var body = new MemoryStream();
        var writer = new BinaryWriter(body);
        writer.Write((byte)2);
        writer.Write(typeof(LookupOracleRootReader).AssemblyQualifiedName); writer.Write(0);
        writer.Write(typeof(LookupOracleItemReader).AssemblyQualifiedName); writer.Write(0);
        writer.Write((byte)0); // No shared resources.
        writer.Write((byte)1); // Root reader followed by raw item payload, without a type index.
        writer.Write(73); writer.Flush();
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
            Console.WriteLine(typeof(ContentManager).Assembly.FullName);
            using (var manager = new LookupOracleManager(Xnb()))
            {
                var root = manager.Load<LookupOracleRoot>("lookup");
                Check(LookupOracleRootReader.Resolved is LookupOracleItemReader,
                      "Initialize resolves distinct target and reader names");
                Check(LookupOracleRootReader.Resolved.TargetType == typeof(LookupOracleItem),
                      "resolved target identity is preserved");
                Check(root.Item.Value == 73, "implicit raw read resolves target without consuming an index");
                Check(LookupOracleRootReader.SelfResolved, "Initialize can resolve its own table reader");
                Check(LookupOracleRootReader.SameLookupIdentity,
                      "genuine XNA returns the same reader identity (CNA documents fresh ownership)");
            }
        }
        catch (Exception error) { ++failures; Console.WriteLine(error); }
        Console.WriteLine("checks=" + checks + " failures=" + failures);
        return checks == 5 && failures == 0 ? 0 : 1;
    }
}
