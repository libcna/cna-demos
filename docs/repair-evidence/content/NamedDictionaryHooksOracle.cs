// Genuine XNA4 protected mutation hooks adjacent to ISSUE-146.
using System;
using Microsoft.Xna.Framework.Content.Pipeline;

sealed class ObservedDictionary : NamedValueDictionary<object>
{
    public int Adds, Sets;
    protected override void AddItem(string key, object value) { ++Adds; base.AddItem(key, value); }
    protected override void SetItem(string key, object value) { ++Sets; base.SetItem(key, value); }
    public void DirectSet(string key, object value) { base.SetItem(key, value); }
}
static class NamedDictionaryHooksOracle
{
    static int checks, failed;
    static void Check(bool value, string name)
    {
        ++checks;
        if (!value) { ++failed; Console.WriteLine("FAIL " + name); }
    }
    static void Reject(Action action, string parameter)
    {
        Type caught = null; string actual = null;
        try { action(); }
        catch (ArgumentException error) { caught = error.GetType(); actual = error.ParamName; }
        catch (Exception error) { caught = error.GetType(); }
        Check(caught == typeof(ArgumentNullException) && actual == parameter,
              "ArgumentNullException/" + parameter + " actual=" + caught + "/" + actual);
    }
    static int Main()
    {
        var data = new ObservedDictionary();
        data["New"] = 1;
        Check(data.Sets == 1 && data.Adds == 0 && (int)data["New"] == 1, "new-key setter dispatches only SetItem");
        data["New"] = 2;
        Check(data.Sets == 2 && data.Adds == 0 && (int)data["New"] == 2, "existing setter dispatches only SetItem");
        data.Add("Added", 3);
        Check(data.Adds == 1 && data.Sets == 2, "Add dispatches only AddItem");
        data.DirectSet("Direct", 4);
        Check(data.Count == 3 && (int)data["Direct"] == 4, "protected base setter inserts missing key");
        Reject(() => data.DirectSet("", null), "key");
        Reject(() => data.DirectSet("Missing", null), "value");
        Check(data.Count == 3 && String.Join(",", data.Keys) == "New,Added,Direct", "failed direct setters preserve entries and order");
        Console.WriteLine("checks=" + checks + " failed=" + failed);
        return failed == 0 ? 0 : 1;
    }
}
