// ISSUE-090: genuine build-host initialization may query nested writers publicly.
using System;
using System.Reflection;
using Microsoft.Xna.Framework.Content.Pipeline;
using Microsoft.Xna.Framework.Content.Pipeline.Serialization.Compiler;

public class InitializationPayload { public string Value; }
[ContentTypeWriter]
public class InitializationPayloadWriter : ContentTypeWriter<InitializationPayload>
{
    public int Calls;
    public ContentTypeWriter Nested;
    protected override void Initialize(ContentCompiler compiler)
    {
        ++Calls;
        Nested = compiler.GetTypeWriter(typeof(string));
    }
    protected override void Write(ContentWriter output, InitializationPayload value) { }
    public override string GetRuntimeReader(TargetPlatform platform) { return "Probe.Reader, Probe"; }
}
static class CompilerInitializationOracle
{
    static int checks, failed;
    static void Check(bool condition, string label)
    {
        ++checks;
        if (!condition) { ++failed; Console.WriteLine("FAIL " + label); }
    }
    // This is XNA's build-host entry, not a fake compiler. Public GetTypeWriter records
    // a dependency and is only valid inside an active initialization context there.
    static ContentTypeWriter HostLookup(ContentCompiler compiler, Type type)
    {
        return (ContentTypeWriter)typeof(ContentCompiler)
            .GetMethod("GetTypeWriterInternal", BindingFlags.Instance | BindingFlags.NonPublic)
            .Invoke(compiler, new object[] { type });
    }
    static int Main()
    {
        try
        {
            Console.WriteLine(typeof(ContentCompiler).Assembly.FullName);
            var compiler = (ContentCompiler)Activator.CreateInstance(typeof(ContentCompiler), true);
            var writer = (InitializationPayloadWriter)HostLookup(compiler, typeof(InitializationPayload));
            Check(writer.Calls == 1, "discovered writer initialized once");
            Check(writer.Nested != null && writer.Nested.TargetType == typeof(string), "public nested lookup completes");
            Check(Object.ReferenceEquals(writer, HostLookup(compiler, typeof(InitializationPayload))), "writer cache identity");
            Check(writer.Calls == 1, "cached lookup does not initialize again");
            Check(Object.ReferenceEquals(writer.Nested, HostLookup(compiler, typeof(string))), "nested cache identity");
            var secondCompiler = (ContentCompiler)Activator.CreateInstance(typeof(ContentCompiler), true);
            var second = (InitializationPayloadWriter)HostLookup(secondCompiler, typeof(InitializationPayload));
            Check(!Object.ReferenceEquals(writer, second), "each compiler owns its writer");
            Check(second.Calls == 1, "second compiler initializes once");
            Check(second.Nested != null && second.Nested.TargetType == typeof(string), "second callback nested lookup");
            Console.WriteLine("checks=" + checks + " failed=" + failed);
            return failed == 0 ? 0 : 1;
        }
        catch (Exception error) { Console.WriteLine("SETUP/BEHAVIOR " + error); return 2; }
    }
}
