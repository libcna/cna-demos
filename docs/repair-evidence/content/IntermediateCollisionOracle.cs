// ISSUE-088: XNA's actual registration path rejects conflicting XML shortcuts.
using System;
using System.IO;
using System.Reflection;
using System.Text;
using System.Xml;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.Content.Pipeline.Serialization.Intermediate;

public class CollisionPayload { public string Text; }
public class FirstPayload : CollisionPayload { }
public class SecondPayload : CollisionPayload { }
public class CollisionSerializer<T> : ContentTypeSerializer<T> where T : CollisionPayload, new()
{
    public CollisionSerializer(string alias) : base(alias) { }
    protected override void Serialize(IntermediateWriter output, T value, ContentSerializerAttribute format)
    { output.Xml.WriteString(value.Text); }
    protected override T Deserialize(IntermediateReader input, ContentSerializerAttribute format, T existing)
    { return new T { Text = input.Xml.ReadContentAsString() }; }
}
static class IntermediateCollisionOracle
{
    static int checks, failed;
    static void Check(bool condition, string label)
    {
        ++checks;
        if (!condition) { ++failed; Console.WriteLine("FAIL " + label); }
    }
    // Reflection invokes XNA's genuine registration entry; public serialization and
    // deserialization below verify the resulting maps, not a replacement registry.
    static void Add(IntermediateSerializer owner, ContentTypeSerializer serializer)
    {
        try
        {
            typeof(IntermediateSerializer).GetMethod("AddTypeSerializer", BindingFlags.NonPublic | BindingFlags.Instance)
                .Invoke(owner, new object[] {serializer});
        }
        catch (TargetInvocationException error) { throw error.InnerException; }
    }
    static string Write<T>(T value)
    {
        var text = new StringBuilder();
        using (var writer = XmlWriter.Create(new StringWriter(text))) IntermediateSerializer.Serialize(writer, value, null);
        return text.ToString();
    }
    static T Read<T>(string text)
    {
        using (var reader = XmlReader.Create(new StringReader(text))) return IntermediateSerializer.Deserialize<T>(reader, null);
    }
    static int Main()
    {
        try
        {
            Console.WriteLine(typeof(IntermediateSerializer).Assembly.FullName);
            var owner = (IntermediateSerializer)typeof(IntermediateSerializer)
                .GetProperty("SingletonInstance", BindingFlags.NonPublic | BindingFlags.Static).GetValue(null, null);
            var first = new CollisionSerializer<FirstPayload>("collision");
            Add(owner, first);
            Check(Object.ReferenceEquals(first, owner.GetTypeSerializer(typeof(FirstPayload))), "first type identity");
            string original = Write(new FirstPayload { Text = "first" });
            Check(original.Contains("Type=\"collision\""), "first emitted shortcut");
            Exception rejected = null;
            try { Add(owner, new CollisionSerializer<SecondPayload>("collision")); }
            catch (Exception error) { rejected = error; }
            Check(rejected != null && rejected.GetType() == typeof(InvalidOperationException), "collision exception category");
            Check(rejected != null && rejected.Message.Contains("XmlTypeName") && rejected.Message.Contains("collision"), "collision diagnostic identifies shortcut");
            Check(Object.ReferenceEquals(first, owner.GetTypeSerializer(typeof(FirstPayload))), "first mapping unchanged");
            var second = new CollisionSerializer<SecondPayload>("distinct");
            bool recovered = true;
            try { Add(owner, second); }
            catch (Exception error) { recovered = false; Console.WriteLine("RECOVERY " + error); }
            Check(recovered, "rejected type can register with a distinct shortcut");
            Check(Object.ReferenceEquals(second, owner.GetTypeSerializer(typeof(SecondPayload))), "recovered target identity");
            Check(original == Write(new FirstPayload { Text = "first" }), "first output unchanged after rejection and recovery");
            Check(Read<FirstPayload>(original).Text == "first", "first exact round trip");
            string xml = Write(new SecondPayload { Text = "second" });
            Check(xml.Contains("Type=\"distinct\""), "second emitted shortcut");
            Check(Read<SecondPayload>(xml).Text == "second", "second exact round trip");
            Console.WriteLine("checks=" + checks + " failed=" + failed);
            return failed == 0 && checks == 11 ? 0 : 1;
        }
        catch (Exception error) { Console.WriteLine("SETUP/BEHAVIOR " + error); return 2; }
    }
}
