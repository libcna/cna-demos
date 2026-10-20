// Reference evidence for ISSUE-148: PI positions and string/byte BOM boundaries.
using System;
using System.IO;
using System.Xml;

static class XmlReaderEnvelopeOracle
{
    static int checks, failed;
    static void Check(bool condition, string label)
    {
        ++checks;
        if (!condition) { ++failed; Console.WriteLine("FAIL " + label); }
    }
    static void Document(string label, string text, bool accepted, int instructions)
    {
        Exception caught = null;
        bool created = false;
        int seen = 0;
        try
        {
            using (var reader = XmlReader.Create(new StringReader(text)))
            {
                created = true;
                while (reader.Read())
                    if (reader.NodeType == XmlNodeType.ProcessingInstruction) ++seen;
            }
        }
        catch (Exception error) { caught = error; }
        Check(accepted ? caught == null && seen == instructions :
              caught != null && caught.GetType() == typeof(XmlException), label + " reader");
        Console.WriteLine("DIAGNOSTIC " + label + " reader-created=" + created +
                          " PI=" + seen + " exception=" + (caught == null ? "none" : caught.GetType().Name));
        caught = null;
        try
        {
            var document = new XmlDocument();
            document.LoadXml(text);
            Check(accepted && document.SelectNodes("//processing-instruction()").Count == instructions,
                  label + " DOM");
        }
        catch (Exception error)
        {
            caught = error;
            Check(!accepted && error.GetType() == typeof(XmlException), label + " DOM rejection");
        }
    }
    static void FaultTiming(string label, string text)
    {
        try
        {
            using (var reader = XmlReader.Create(new StringReader(text)))
            {
                Check(reader.ReadState == ReadState.Initial, label + " initial state");
                Exception caught = null;
                try { while (reader.Read()) { } }
                catch (Exception error) { caught = error; }
                Check(caught != null && caught.GetType() == typeof(XmlException), label + " read exception");
                Check(reader.ReadState == ReadState.Error, label + " error state");
                Check(!reader.Read(), label + " subsequent read returns false");
                reader.Close();
                Check(reader.ReadState == ReadState.Closed, label + " closed state");
                Check(!reader.Read(), label + " closed read returns false");
            }
        }
        catch (Exception error) { Check(false, label + " unexpected boundary " + error.GetType()); }
    }
    static int Main()
    {
        Console.WriteLine("runtime=" + Environment.Version + " OS=" + Environment.OSVersion.Platform);
        Document("leading", "<?p d?><r/>", true, 1);
        Document("nested", "<r><?p d?></r>", true, 1);
        Document("trailing", "<r/><?p d?>", true, 1);
        Document("after-comment", "<!--c--><?p d?><r/>", true, 1);
        Document("declaration-then-PI", "<?xml version=\"1.0\"?><?p d?><r/>", true, 1);
        Document("all-positions", "<?before?><r>x<?inside d?><child/><?after-child?></r><?after?>", true, 4);
        Document("xml-prefix-not-reserved", "<r><?xml-stylesheet href=\"x\"?></r>", true, 1);
        Document("comment-literal", "<r><!--<?p d?>--></r>", true, 0);
        Document("CDATA-literal", "<r><![CDATA[<?p d?>]]></r>", true, 0);
        Document("escaped-attribute", "<r a=\"&lt;?p d?>\"/>", true, 0);
        Document("nested-declaration", "<r><?xml version=\"1.0\"?></r>", false, 0);
        Document("trailing-declaration", "<r/><?xml version=\"1.0\"?>", false, 0);
        Document("reserved-uppercase", "<?XML version=\"1.0\"?><r/>", false, 0);
        Document("unterminated", "<r><?p d</r>", false, 0);
        Document("string-BOM", "\uFEFF<?xml version=\"1.0\"?><r/>", false, 0);
        try
        {
            byte[] bytes = System.Text.Encoding.UTF8.GetBytes("\uFEFF<?xml version=\"1.0\"?><r/>");
            using (var reader = XmlReader.Create(new MemoryStream(bytes)))
                while (reader.Read()) { }
            Check(true, "byte-stream BOM accepted");
        }
        catch (Exception error) { Check(false, "byte-stream BOM " + error.GetType()); }
        FaultTiming("BOM timing", "\uFEFF<r/>");
        FaultTiming("nested declaration timing", "<r><?xml version=\"1.0\"?></r>");
        FaultTiming("unterminated timing", "<r><?p d</r>");
        FaultTiming("invalid root timing", "not XML");
        FaultTiming("empty document timing", "");
        Console.WriteLine("checks=" + checks + " failed=" + failed);
        return failed == 0 ? 0 : 1;
    }
}
