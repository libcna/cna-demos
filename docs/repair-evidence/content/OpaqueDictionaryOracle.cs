// Genuine XNA4 issue146: null categories, validation order and mutation semantics.
using System;
using System.Reflection;
using System.Runtime.CompilerServices;
using Microsoft.Xna.Framework.Content.Pipeline;

static class OpaqueDictionaryOracle
{
    static int checks, failed;
    static void Check(bool value, string name)
    {
        ++checks;
        if (!value) { ++failed; Console.WriteLine("FAIL " + name); }
    }
    static void Reject<T>(Action action, string parameter) where T : ArgumentException
    {
        Type caught = null; string actual = null;
        try { action(); }
        catch (ArgumentException error) { caught = error.GetType(); actual = error.ParamName; }
        catch (Exception error) { caught = error.GetType(); }
        Check(caught == typeof(T) && actual == parameter, typeof(T).Name + "/" + parameter + " actual=" + caught + "/" + actual);
    }
    [MethodImpl(MethodImplOptions.NoInlining)]
    static int Run()
    {
        Console.WriteLine(typeof(OpaqueDataDictionary).Assembly.FullName);
        Console.WriteLine(typeof(OpaqueDataDictionary).Assembly.Location);
        var data = new OpaqueDataDictionary();
        data.Add("Seed", 1); data.Add("Text", "retained");
        string original = data.GetContentAsXml();
        Reject<ArgumentNullException>(() => data.Add("New", null), "value");
        Reject<ArgumentNullException>(() => data.Add("Seed", null), "value");
        Reject<ArgumentNullException>(() => data["Seed"] = null, "value");
        Reject<ArgumentNullException>(() => data["New"] = null, "value");
        Reject<ArgumentNullException>(() => data.Add("", null), "key");
        Reject<ArgumentNullException>(() => data[""] = null, "key");
        Reject<ArgumentNullException>(() => data.Remove(""), "key");
        Check(data.Count == 2 && (int)data["Seed"] == 1 && data.GetContentAsXml() == original,
              "all rejections preserve entries and exact XML");
        data["New"] = 2;
        Check(data.Count == 3 && (int)data["New"] == 2, "setter inserts missing key");
        data["Seed"] = 3;
        Check(String.Join(",", data.Keys) == "Seed,Text,New", "replacement preserves insertion order");
        var typed = new NamedValueDictionary<object>();
        Reject<ArgumentNullException>(() => typed.Add("", null), "key");
        Reject<ArgumentNullException>(() => typed[""] = null, "key");
        Reject<ArgumentNullException>(() => typed.Add("Value", null), "value");
        typed["Value"] = 4;
        Check(typed.Count == 1 && (int)typed["Value"] == 4, "base setter inserts missing key");
        Console.WriteLine("checks=" + checks + " failed=" + failed);
        return failed == 0 ? 0 : 1;
    }
    static int Main()
    {
        // Run beside the reference DLLs in isolated content-probe staging, never
        // beside all other campaign probes (which would change their reference runtime).
        try { return Run(); }
        catch (Exception error) { Console.WriteLine("SETUP ERROR " + error); return 2; }
    }
}
