// Genuine .NET writer prerequisite for CNA-DEMOS-ISSUE-147.
using System;
using System.IO;
using System.Text;
using System.Xml;

static class XmlWriterNulOracle
{
    static int checks, failed;
    static readonly string[] Doors = { "WriteString", "WriteAttributeString", "WriteCData",
        "WriteComment", "WriteProcessingInstruction", "WriteElementString", "WriteDocType" };
    static void Check(bool value, string name)
    {
        ++checks;
        if (!value) { ++failed; Console.WriteLine("FAIL " + name); }
    }
    static void Write(XmlWriter writer, int door, string value)
    {
        if (door == 6) { writer.WriteDocType("r", null, null, value); return; }
        if (door == 5) { writer.WriteElementString("r", value); return; }
        writer.WriteStartElement("r");
        switch (door)
        {
            case 0: writer.WriteString(value); break;
            case 1: writer.WriteAttributeString("a", value); break;
            case 2: writer.WriteCData(value); break;
            case 3: writer.WriteComment(value); break;
            case 4: writer.WriteProcessingInstruction("p", value); break;
        }
        writer.WriteEndElement();
    }
    static Type Outcome(int door, string value, bool checkCharacters, out string output)
    {
        var text = new StringBuilder();
        var settings = new XmlWriterSettings { OmitXmlDeclaration = true, CheckCharacters = checkCharacters };
        var writer = XmlWriter.Create(new StringWriter(text), settings);
        Type caught = null;
        try { Write(writer, door, value); }
        catch (Exception error) { caught = error.GetType(); }
        try { writer.Close(); }
        catch (Exception error) { if (caught == null) caught = error.GetType(); }
        output = text.ToString();
        return caught;
    }
    static int Main()
    {
        Console.WriteLine("runtime=" + Environment.Version + " OS=" + Environment.OSVersion.Platform);
        for (int door = 0; door < Doors.Length; ++door)
        {
            string output;
            foreach (string value in new[] { "a\0b", "\0ab" })
            {
                Type caught = Outcome(door, value, true, out output);
                Check(caught == typeof(ArgumentException), Doors[door] + " default NUL category actual=" + caught);
            }
            Check(Outcome(door, "retained", true, out output) == null && output.Contains("retained"),
                  Doors[door] + " ordinary-content control");
            Type disabled = Outcome(door, "a\0b", false, out output);
            Console.WriteLine("DIAGNOSTIC CheckCharacters=false " + Doors[door] + " type=" +
                (disabled == null ? "none" : disabled.FullName) + " utf16=" + BitConverter.ToString(Encoding.Unicode.GetBytes(output)));
        }
        Console.WriteLine("checks=" + checks + " failed=" + failed);
        return failed == 0 ? 0 : 1;
    }
}
