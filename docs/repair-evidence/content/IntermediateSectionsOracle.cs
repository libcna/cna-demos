// ISSUE-148: genuine XNA section traversal, separate from the XML parser defects.
using System;
using System.IO;
using System.Xml;
using Microsoft.Xna.Framework.Content.Pipeline;
using Microsoft.Xna.Framework.Content.Pipeline.Serialization.Intermediate;
using Microsoft.Xna.Framework.Content;

public class SectionNode { public int Value; }
public class SectionRoot
{
    [ContentSerializer(SharedResource = true)] public SectionNode Shared;
    public ExternalReference<SectionNode> External;
}

static class IntermediateSectionsOracle
{
    static int checks, failed;
    static void ObserveFixups(string name, string shared, string sections, string expectedMessage)
    {
        ++checks;
        string xml = "<XnaContent><Asset Type=\"SectionRoot\">" + shared +
            "<External><Reference>#e</Reference></External></Asset>" + sections + "</XnaContent>";
        try
        {
            using (var reader = XmlReader.Create(new StringReader(xml)))
                IntermediateSerializer.Deserialize<SectionRoot>(reader, null);
            ++failed;
            Console.WriteLine(name + " UNEXPECTED ACCEPT");
        }
        catch (Exception error)
        {
            Console.WriteLine(name + " REJECT " + error.GetType().FullName + " " + error.Message);
            bool diagnosticMatches = expectedMessage.StartsWith("There was an error", StringComparison.Ordinal)
                ? error.Message.StartsWith(expectedMessage, StringComparison.Ordinal)
                : error.Message == expectedMessage;
            if (error.GetType() != typeof(InvalidContentException) || !diagnosticMatches) ++failed;
        }
    }
    static void Observe(string name, string sections, string rejectedNode)
    {
        ++checks;
        string xml = "<XnaContent><Asset Type=\"int\">37</Asset>" + sections + "</XnaContent>";
        try
        {
            using (var reader = XmlReader.Create(new StringReader(xml)))
            {
                int result = IntermediateSerializer.Deserialize<int>(reader, null);
                Console.WriteLine(name + " ACCEPT " + result);
                if (rejectedNode != null || result != 37) ++failed;
            }
        }
        catch (Exception error)
        {
            Console.WriteLine(name + " REJECT " + error.GetType().FullName + " " + error.Message);
            string prefix = "There was an error while deserializing intermediate XML. '" +
                            rejectedNode + "' is an invalid XmlNodeType.";
            if (rejectedNode == null || error.GetType() != typeof(InvalidContentException) ||
                !error.Message.StartsWith(prefix, StringComparison.Ordinal)) ++failed;
        }
    }
    static int Main()
    {
        Console.WriteLine(typeof(IntermediateSerializer).Assembly.FullName);
        Console.WriteLine(typeof(IntermediateSerializer).Assembly.Location);
        Observe("absent", "", null);
        Observe("resources-paired", "<Resources></Resources>", null);
        Observe("externals-paired", "<ExternalReferences></ExternalReferences>", null);
        Observe("both-paired", "<Resources></Resources><ExternalReferences></ExternalReferences>", null);
        Observe("both-paired-with-comments", "<Resources><!--a--></Resources><!--b--><ExternalReferences> </ExternalReferences>", null);
        Observe("resources-selfclosing", "<Resources />", "None");
        Observe("externals-selfclosing", "<ExternalReferences />", "None");
        Observe("resources-selfclosing-before-paired", "<Resources /><ExternalReferences></ExternalReferences>", "Element");
        Observe("externals-selfclosing-after-paired", "<Resources></Resources><ExternalReferences />", "None");
        Observe("both-selfclosing", "<Resources /><ExternalReferences />", "Element");
        Observe("resources-selfclosing-with-comment", "<Resources /><!--tail-->", "None");
        Observe("externals-selfclosing-with-comment", "<ExternalReferences /><!--tail-->", "None");
        Observe("reverse-paired-order", "<ExternalReferences></ExternalReferences><Resources></Resources>", "Element");
        ObserveFixups("shared-before-missing-external-section", "<Shared>#s</Shared>", "<Resources />", "Missing shared resource \"#s\".");
        ObserveFixups("shared-before-invalid-external-type", "<Shared>#s</Shared>",
            "<Resources></Resources><ExternalReferences><ExternalReference ID=\"#e\" TargetType=\"No.Such.Type\">C:\\a.bin</ExternalReference></ExternalReferences>",
            "Missing shared resource \"#s\".");
        ObserveFixups("section-end-before-shared-fixup", "<Shared>#s</Shared>", "<Resources /><Unexpected />",
            "There was an error while deserializing intermediate XML. 'Element' is an invalid XmlNodeType.");
        ObserveFixups("external-before-end-document", "<Shared Null=\"true\" />", "<ExternalReferences />", "Missing external reference \"#e\".");
        ObserveFixups("external-before-trailing-element", "<Shared Null=\"true\" />", "<ExternalReferences></ExternalReferences><Unexpected />", "Missing external reference \"#e\".");
        Console.WriteLine("checks=" + checks + " failed=" + failed);
        return failed == 0 ? 0 : 1;
    }
}
