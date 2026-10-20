// ISSUE-088: XML shortcuts take precedence over full names in either registration order.
using System;
using System.IO;
using System.Reflection;
using System.Text;
using System.Xml;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.Content.Pipeline.Serialization.Intermediate;
public class Payload { public string Text; }
public class FirstFull : Payload { }
public class AliasSecond : Payload { }
public class AliasFirst : Payload { }
public class SecondFull : Payload { }
public class AliasSerializer<T> : ContentTypeSerializer<T> where T : Payload, new()
{
    public AliasSerializer(string alias) : base(alias) { }
    protected override void Serialize(IntermediateWriter output, T value, ContentSerializerAttribute format) { output.Xml.WriteString(value.Text); }
    protected override T Deserialize(IntermediateReader input, ContentSerializerAttribute format, T existing) { return new T { Text = input.Xml.ReadContentAsString() }; }
}
class IntermediateNamePriorityOracle
{
    static IntermediateSerializer owner;
    static int checks, failed;
    static void Check(bool value, string label)
    {
        ++checks;
        if (!value) { ++failed; Console.WriteLine("FAIL " + label); }
    }
    static void Add(ContentTypeSerializer serializer)
    {
        typeof(IntermediateSerializer).GetMethod("AddTypeSerializer", BindingFlags.NonPublic | BindingFlags.Instance).Invoke(owner, new object[] { serializer });
        Check(Object.ReferenceEquals(serializer, owner.GetTypeSerializer(serializer.TargetType)), "registered type identity " + serializer.TargetType);
    }
    static void RoundTrip<T>(T value, string alias) where T : Payload
    {
        var text = new StringBuilder();
        using (var writer = XmlWriter.Create(new StringWriter(text))) IntermediateSerializer.Serialize(writer, value, null);
        Check(text.ToString().Contains("Type=\"" + alias + "\""), "emitted alias " + alias);
        using (var reader = XmlReader.Create(new StringReader(text.ToString())))
            Check(IntermediateSerializer.Deserialize<T>(reader, null).Text == value.Text, "exact round trip " + alias);
    }
    static int Main()
    {
        try
        {
            owner = (IntermediateSerializer)typeof(IntermediateSerializer).GetProperty("SingletonInstance", BindingFlags.NonPublic | BindingFlags.Static).GetValue(null, null);
            Add(new AliasSerializer<FirstFull>("firstFullAlias"));
            Add(new AliasSerializer<AliasSecond>(typeof(FirstFull).FullName));
            RoundTrip(new FirstFull { Text = "first" }, "firstFullAlias");
            RoundTrip(new AliasSecond { Text = "second" }, typeof(FirstFull).FullName);
            Add(new AliasSerializer<AliasFirst>(typeof(SecondFull).FullName));
            Add(new AliasSerializer<SecondFull>("secondFullAlias"));
            RoundTrip(new AliasFirst { Text = "third" }, typeof(SecondFull).FullName);
            RoundTrip(new SecondFull { Text = "fourth" }, "secondFullAlias");
            Console.WriteLine("checks=" + checks + " failed=" + failed);
            return failed == 0 && checks == 12 ? 0 : 1;
        }
        catch (Exception error) { Console.WriteLine("BEHAVIOR " + error); return 2; }
    }
}
