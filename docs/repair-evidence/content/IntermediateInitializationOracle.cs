// ISSUE-089: genuine XNA serializer initialization and cache lifetime.
using System;
using System.IO;
using System.Text;
using System.Xml;
using System.Collections.Generic;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.Content.Pipeline.Serialization.Intermediate;

public class InitializedPayload { public string Text = "retained"; }
[ContentTypeSerializer]
public class InitializedPayloadSerializer : ContentTypeSerializer<InitializedPayload>
{
    public static List<InitializedPayloadSerializer> Instances = new List<InitializedPayloadSerializer>();
    public int Calls, Scans, Writes, Reads;
    public IntermediateSerializer Owner;
    public ContentTypeSerializer Nested;
    public InitializedPayloadSerializer() { Instances.Add(this); }
    protected override void Initialize(IntermediateSerializer serializer)
    {
        ++Calls;
        Owner = serializer;
        Nested = serializer.GetTypeSerializer(typeof(string));
        IntermediateInitializationOracle.Check(Calls == 1, "one Initialize invocation");
        IntermediateInitializationOracle.Check(Nested.TargetType == typeof(string), "nested lookup inside Initialize");
    }
    protected override void ScanChildren(IntermediateSerializer serializer, ChildCallback callback, InitializedPayload value)
    {
        ++Scans;
        IntermediateInitializationOracle.Check(Calls == 1, "initialized before child scan");
        IntermediateInitializationOracle.Check(Object.ReferenceEquals(Owner, serializer), "scan uses initialization owner");
    }
    protected override void Serialize(IntermediateWriter output, InitializedPayload value, ContentSerializerAttribute format)
    {
        ++Writes;
        IntermediateInitializationOracle.Check(Calls == 1, "initialized before write");
        IntermediateInitializationOracle.Check(Object.ReferenceEquals(Owner, output.Serializer), "write uses initialization owner");
        output.Xml.WriteString(value.Text);
    }
    protected override InitializedPayload Deserialize(IntermediateReader input, ContentSerializerAttribute format, InitializedPayload existing)
    {
        ++Reads;
        IntermediateInitializationOracle.Check(Calls == 1, "initialized before read");
        IntermediateInitializationOracle.Check(Object.ReferenceEquals(Owner, input.Serializer), "read uses initialization owner");
        return new InitializedPayload { Text = input.Xml.ReadContentAsString() };
    }
}
public static class IntermediateInitializationOracle
{
    static int checks, failed;
    public static void Check(bool condition, string label)
    {
        ++checks;
        if (!condition) { ++failed; Console.WriteLine("FAIL " + label); }
    }
    static int Main()
    {
        try
        {
            Console.WriteLine(typeof(IntermediateSerializer).Assembly.FullName);
            string previous = null;
            for (int pass = 0; pass != 2; ++pass)
            {
                var text = new StringBuilder();
                using (var writer = XmlWriter.Create(new StringWriter(text)))
                    IntermediateSerializer.Serialize(writer, new InitializedPayload(), null);
                if (previous != null) Check(previous == text.ToString(), "repeated output is identical");
                previous = text.ToString();
                using (var reader = XmlReader.Create(new StringReader(previous)))
                    Check(IntermediateSerializer.Deserialize<InitializedPayload>(reader, null).Text == "retained", "exact round trip");
            }
            Check(InitializedPayloadSerializer.Instances.Count == 1, "one cached custom serializer across four operations");
            var item = InitializedPayloadSerializer.Instances[0];
            Check(item.Calls == 1 && item.Scans == 2 && item.Writes == 2 && item.Reads == 2, "exact callback totals");
            Check(Object.ReferenceEquals(item, item.Owner.GetTypeSerializer(typeof(InitializedPayload))), "owner and custom cache survive all operations");
            Check(Object.ReferenceEquals(item.Nested, item.Owner.GetTypeSerializer(typeof(string))), "nested cached identity survives all operations");
            Console.WriteLine("checks=" + checks + " failed=" + failed);
            return failed == 0 && checks == 21 ? 0 : 1;
        }
        catch (Exception error) { Console.WriteLine("SETUP/BEHAVIOR " + error); return 2; }
    }
}
