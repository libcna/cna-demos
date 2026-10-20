// ISSUE-046: external TargetType uses XML namespace scope, not a global alias.
using System;
using System.IO;
using System.Xml;
using Microsoft.Xna.Framework.Content.Pipeline;
using Microsoft.Xna.Framework.Content.Pipeline.Serialization.Intermediate;

namespace Repair.External
{
    public class Target { private Target() {} }
    public class Scene { public ExternalReference<Target> Reference; }
    public class GenericScene { public ExternalReference<System.Collections.Generic.List<Target>> Reference; }
}
namespace Repair.Other { public class Target {} }

static class IntermediateExternalAliasOracle
{
    static int checks, failed;
    static void Observe<T>(string label, string root, string target, string localNamespace,
                           string expectedFailure) where T : class
    {
        ++checks;
        string xml = "<XnaContent xmlns:R=\"Repair.External\"><Asset Type=\"R:" + root +
            "\"><Reference><Reference>#e</Reference></Reference></Asset><ExternalReferences>" +
            "<ExternalReference ID=\"#e\" TargetType=\"" + target + "\"" + localNamespace +
            ">C:\\references\\payload.bin</ExternalReference></ExternalReferences></XnaContent>";
        try
        {
            T result;
            using (var reader = XmlReader.Create(new StringReader(xml)))
                result = IntermediateSerializer.Deserialize<T>(reader, null);
            object reference = typeof(T).GetField("Reference").GetValue(result);
            string filename = (string)reference.GetType().GetProperty("Filename").GetValue(reference, null);
            Console.WriteLine(label + " ACCEPT " + filename);
            if (expectedFailure != null || filename != "C:\\references\\payload.bin") ++failed;
        }
        catch (Exception error)
        {
            Console.WriteLine(label + " REJECT " + error.GetType().FullName + " " + error.Message);
            if (expectedFailure == null || error.GetType() != typeof(InvalidContentException) ||
                !error.Message.Contains(expectedFailure)) ++failed;
        }
    }
    static int Main()
    {
        Console.WriteLine(typeof(IntermediateSerializer).Assembly.FullName);
        Console.WriteLine(typeof(IntermediateSerializer).Assembly.Location);
        Observe<Repair.External.Scene>("canonical-no-target-serializer", "Scene", "Repair.External.Target", "", null);
        Observe<Repair.External.Scene>("root-alias", "Scene", "R:Target", "", null);
        Observe<Repair.External.Scene>("entry-local-alias", "Scene", "L:Target", " xmlns:L=\"Repair.External\"", null);
        Observe<Repair.External.Scene>("canonical-assembly-qualified", "Scene", "Repair.External.Target, IntermediateExternalAliasOracle", "", null);
        Observe<Repair.External.Scene>("local-shadow", "Scene", "R:Target", " xmlns:R=\"Repair.Other\"", "wrong type for external reference");
        Observe<Repair.External.Scene>("unknown-prefix", "Scene", "Missing:Target", "", "Cannot find type");
        Observe<Repair.External.Scene>("unknown-target", "Scene", "R:Absent", "", "Cannot find type");
        Observe<Repair.External.Scene>("wrong-shortcut", "Scene", "int", "", "wrong type for external reference");
        Observe<Repair.External.Scene>("canonical-whitespace", "Scene", " Repair.External.Target ", "", "Cannot find type");
        Observe<Repair.External.GenericScene>("generic-alias-argument", "GenericScene", "System.Collections.Generic.List[R:Target]", "", null);
        Observe<Repair.External.GenericScene>("generic-local-alias", "GenericScene", "G:List[L:Target]", " xmlns:G=\"System.Collections.Generic\" xmlns:L=\"Repair.External\"", null);
        Observe<Repair.External.GenericScene>("generic-canonical", "GenericScene", "System.Collections.Generic.List[Repair.External.Target]", "", null);
        Observe<Repair.External.Scene>("assembly-trailing-whitespace", "Scene", "Repair.External.Target, IntermediateExternalAliasOracle ", "", null);
        Observe<Repair.External.Scene>("qualified-leading-whitespace", "Scene", " Repair.External.Target, IntermediateExternalAliasOracle", "", null);
        Observe<Repair.External.GenericScene>("generic-trailing-whitespace", "GenericScene", "System.Collections.Generic.List[R:Target] ", "", "Cannot find type");
        Console.WriteLine("checks=" + checks + " failed=" + failed);
        return failed == 0 ? 0 : 1;
    }
}
