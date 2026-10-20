// ISSUE-148: genuine XNA section traversal, separate from the XML parser defects.
using System;
using System.IO;
using System.Xml;
using Microsoft.Xna.Framework.Content.Pipeline;
using Microsoft.Xna.Framework.Content.Pipeline.Serialization.Intermediate;

static class IntermediateSectionsOracle
{
    static int checks, failed;
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
        Console.WriteLine("checks=" + checks + " failed=" + failed);
        return failed == 0 ? 0 : 1;
    }
}
