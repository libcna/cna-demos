// Reference contract for XML NUL values through direct and XmlWriter LINQ doors.
using System;
using System.IO;
using System.Xml;
using System.Xml.Linq;

static class XLinqNulOracle
{
    static int checks, failed;
    static void Check(bool value, string label)
    {
        ++checks;
        if (!value) { ++failed; Console.WriteLine("FAIL " + label); }
    }
    static string ThroughWriter(XNode node)
    {
        var text = new StringWriter();
        using (var writer = XmlWriter.Create(text))
        {
            writer.WriteStartElement("holder");
            node.WriteTo(writer);
            writer.WriteEndElement();
        }
        return text.ToString();
    }
    static void Door(string label, Func<string, string> write)
    {
        foreach (string value in new[] { "\0ab", "a\0b", "ab\0", "\0" })
        {
            Type caught = null;
            try { write(value); }
            catch (Exception error) { caught = error.GetType(); }
            Check(caught == typeof(ArgumentException), label + " NUL category actual=" + caught);
        }
        try { Check(write("retained").Contains("retained"), label + " ordinary content"); }
        catch (Exception error) { Check(false, label + " ordinary content " + error.GetType()); }
    }
    static int Main()
    {
        Console.WriteLine("runtime=" + Environment.Version + " OS=" + Environment.OSVersion.Platform);
        Door("XText direct", v => new XText(v).ToString(SaveOptions.DisableFormatting));
        Door("XCData direct", v => new XCData(v).ToString(SaveOptions.DisableFormatting));
        Door("XComment direct", v => new XComment(v).ToString(SaveOptions.DisableFormatting));
        Door("XProcessingInstruction direct", v => new XProcessingInstruction("p", v).ToString(SaveOptions.DisableFormatting));
        Door("XDocumentType subset", v => new XDocumentType("r", null, null, v).ToString(SaveOptions.DisableFormatting));
        Door("XElement text", v => new XElement("r", v).ToString(SaveOptions.DisableFormatting));
        Door("XElement attribute", v => new XElement("r", new XAttribute("a", v)).ToString(SaveOptions.DisableFormatting));
        Door("XAttribute direct", v => new XAttribute("a", v).ToString());
        foreach (string value in new[] { "\0ab", "a\0b", "ab\0", "\0" })
        {
            string output = new XAttribute(XName.Get("a", "urn:" + value), "retained").ToString();
            Console.WriteLine("DIAGNOSTIC XAttribute namespace: " + output);
            Check(!output.Contains("\0") && output.Contains("retained"), "standalone attribute omits its namespace URI");
        }
        Door("XElement namespace", v => new XElement(XName.Get("r", "urn:" + v), "retained").ToString(SaveOptions.DisableFormatting));
        Door("XStreamingElement direct", v => new XStreamingElement("r", v).ToString(SaveOptions.DisableFormatting));
        Door("XText writer", v => ThroughWriter(new XText(v)));
        Door("XCData writer", v => ThroughWriter(new XCData(v)));
        Door("XComment writer", v => ThroughWriter(new XComment(v)));
        Door("XProcessingInstruction writer", v => ThroughWriter(new XProcessingInstruction("p", v)));
        Console.WriteLine("checks=" + checks + " failed=" + failed);
        return failed == 0 ? 0 : 1;
    }
}
