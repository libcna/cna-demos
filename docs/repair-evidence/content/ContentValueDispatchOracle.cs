// Genuine XNA4 value-type control for the reference-type dispatch probe.
using System;
using System.IO;
using Microsoft.Xna.Framework.Content;

public struct DispatchValue : IDisposable
{
    public int Number;
    public void Dispose() { }
}
public sealed class DispatchValueReader : ContentTypeReader<DispatchValue>
{
    protected override DispatchValue Read(ContentReader input, DispatchValue existing)
    { return new DispatchValue { Number = input.ReadInt32() }; }
}
public sealed class ValueDispatchRoot { public object Value; }
public sealed class ValueDispatchRootReader : ContentTypeReader<ValueDispatchRoot>
{
    public static int Mode;
    ContentTypeReader valueReader;
    protected override void Initialize(ContentTypeReaderManager manager)
    { valueReader = manager.GetTypeReader(typeof(DispatchValue)); }
    protected override ValueDispatchRoot Read(ContentReader input, ValueDispatchRoot existing)
    {
        var value = new DispatchValue { Number = -1 };
        object result;
        switch (Mode)
        {
            case 0: result = input.ReadObject<DispatchValue>(); break;
            case 1: result = input.ReadObject<DispatchValue>(value); break;
            case 2: result = input.ReadObject<DispatchValue>(valueReader); break;
            case 3: result = input.ReadObject<DispatchValue>(valueReader, value); break;
            case 4: result = input.ReadRawObject<DispatchValue>(); break;
            case 5: result = input.ReadRawObject<DispatchValue>(value); break;
            case 6: result = input.ReadRawObject<DispatchValue>(valueReader); break;
            case 7: result = input.ReadRawObject<DispatchValue>(valueReader, value); break;
            case 8: result = input.ReadObject<object>(); break;
            default: throw new InvalidOperationException("unknown value mode");
        }
        return new ValueDispatchRoot { Value = result };
    }
}
sealed class ValueDispatchServices : IServiceProvider
{
    public object GetService(Type type) { return null; }
}
sealed class ValueDispatchManager : ContentManager
{
    readonly byte[] bytes;
    public ValueDispatchManager(byte[] bytes) : base(new ValueDispatchServices()) { this.bytes = bytes; }
    protected override Stream OpenStream(string name) { return new MemoryStream(bytes, false); }
    public ValueDispatchRoot Raw(Action<IDisposable> record)
    { return ReadAsset<ValueDispatchRoot>("asset", record); }
}
static class ContentValueDispatchOracle
{
    static int checks, failures;
    static void Check(bool condition, string label)
    {
        ++checks;
        Console.WriteLine((condition ? "PASS " : "FAIL ") + label);
        if (!condition) ++failures;
    }
    static byte[] Xnb(int mode)
    {
        var body = new MemoryStream();
        var writer = new BinaryWriter(body);
        writer.Write((byte)2);
        writer.Write(typeof(ValueDispatchRootReader).AssemblyQualifiedName); writer.Write(0);
        writer.Write(typeof(DispatchValueReader).AssemblyQualifiedName); writer.Write(0);
        writer.Write((byte)0); writer.Write((byte)1);
        if (mode == 0 || mode == 1 || mode == 8) writer.Write((byte)2);
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
            Console.WriteLine(typeof(ContentReader).Assembly.FullName);
            for (int mode = 0; mode <= 8; ++mode)
            {
                ValueDispatchRootReader.Mode = mode;
                using (var manager = new ValueDispatchManager(Xnb(mode)))
                {
                    int recorded = 0;
                    Exception observed = null;
                    ValueDispatchRoot result = null;
                    try { result = manager.Raw(item => ++recorded); }
                    catch (Exception error) { observed = error; }
                    if (mode == 1 || mode == 3 || mode == 5 || mode == 7)
                        Check(observed != null && observed.GetType() == typeof(InvalidOperationException),
                              "mode " + mode + " refuses boxed existing-value identity");
                    else
                        Check(observed == null && result != null && result.Value is DispatchValue &&
                              ((DispatchValue)result.Value).Number == 73,
                              "mode " + mode + " decodes exact value payload");
                    Check(recorded == 0, "mode " + mode + " never records IDisposable value type");
                }
            }
            Console.WriteLine("checks=" + checks + " failed=" + failures);
            return failures == 0 ? 0 : 1;
        }
        catch (Exception error) { Console.WriteLine("SETUP/BEHAVIOR " + error); return 2; }
    }
}
