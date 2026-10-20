// Fresh direct-deserialization evidence for CNA-DEMOS-ISSUE-148/149.
using System;
using System.IO;
using System.Text;
using System.Xml;
using Microsoft.Xna.Framework.Content.Pipeline;
using Microsoft.Xna.Framework.Content.Pipeline.Serialization.Intermediate;

public class ContentEnvelope
{
    public string Name = "retained";
    public int Value = 3;
}
static class IntermediateEnvelopeOracle
{
    static int checks, failed;
    static void Check(bool value, string label)
    {
        ++checks;
        if (!value) { ++failed; Console.WriteLine("FAIL " + label); }
    }
    static string Serialize(ContentEnvelope value)
    {
        var text = new StringBuilder();
        using (var writer = XmlWriter.Create(new StringWriter(text), new XmlWriterSettings { Indent = false }))
            IntermediateSerializer.Serialize(writer, value, null);
        return text.ToString();
    }
    static void Observe(string label, string xml, Type expected = null, bool expectNull = false,
                        string expectedName = "retained")
    {
        try
        {
            ContentEnvelope value;
            using (var reader = XmlReader.Create(new StringReader(xml)))
                value = IntermediateSerializer.Deserialize<ContentEnvelope>(reader, null);
            Console.WriteLine(label + " DESERIALIZE PASS null=" + (value == null) +
                (value == null ? "" : " name=" + value.Name + " value=" + value.Value));
            Check(expected == null && (value == null) == expectNull &&
                  (value == null || (value.Name == expectedName && value.Value == 3)), label + " direct read");
            try
            {
                Console.WriteLine(label + " RESERIALIZE PASS " + Serialize(value));
                Check(!expectNull, label + " reserialization");
            }
            catch (Exception error)
            {
                Console.WriteLine(label + " RESERIALIZE " + error.GetType().FullName + " " + error.Message);
                Check(expectNull && error.GetType() == typeof(ArgumentNullException), label + " reserialization category");
            }
        }
        catch (Exception error)
        {
            Console.WriteLine(label + " DESERIALIZE " + error.GetType().FullName + " " + error.Message);
            Check(error.GetType() == expected, label + " direct read category");
        }
    }
    static int Run()
    {
        Console.WriteLine(typeof(IntermediateSerializer).Assembly.FullName);
        Console.WriteLine(typeof(IntermediateSerializer).Assembly.Location);
        var original = Serialize(new ContentEnvelope());
        Console.WriteLine("ORIGINAL " + original);
        if (!original.Contains("<Value>3</Value>") || !original.Contains("<Name>retained</Name>")) return 2;
        Observe("baseline", original);
        Observe("processing-instruction", original.Replace("<Value>", "<?pi x?><Value>"));
        Observe("empty-resources", original.Replace("</XnaContent>", "<Resources /></XnaContent>"), typeof(InvalidContentException));
        Observe("empty-externals", original.Replace("</XnaContent>", "<ExternalReferences /></XnaContent>"), typeof(InvalidContentException));
        Observe("expanded-sections", original.Replace("</XnaContent>", "<Resources></Resources><ExternalReferences></ExternalReferences></XnaContent>"));
        Observe("string-BOM", "\uFEFF" + original, typeof(InvalidContentException));
        Observe("null-value", original.Replace("<Value>3</Value>", "<Value Null=\"true\" />"), typeof(NullReferenceException));
        Observe("null-value-false", original.Replace("<Value>3</Value>", "<Value Null=\"false\">3</Value>"));
        Observe("null-root", "<XnaContent><Asset Null=\"true\" /></XnaContent>", null, true);
        Observe("null-string", original.Replace("<Name>retained</Name>", "<Name Null=\"true\" />"), null, false, null);
        Console.WriteLine("checks=" + checks + " failed=" + failed);
        return failed == 0 ? 0 : 1;
    }
    static int Main()
    {
        try { return Run(); }
        catch (Exception error) { Console.WriteLine("SETUP ERROR " + error); return 2; }
    }
}
