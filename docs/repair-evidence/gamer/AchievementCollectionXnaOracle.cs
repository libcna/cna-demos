// Original regression oracle for the genuine XNA 4.0 Windows assembly.
using System;
using System.Collections.Generic;
using System.Reflection;
using Microsoft.Xna.Framework.GamerServices;

internal static class AchievementCollectionXnaOracle
{
    private static int checks;
    private static int failures;

    private static void Check(bool condition, string name)
    {
        ++checks;
        if (!condition) ++failures;
        Console.WriteLine((condition ? "PASS " : "FAIL ") + name);
    }

    private static void Throws<T>(Action action, string name) where T : Exception
    {
        try { action(); Check(false, name + " (no exception)"); }
        catch (Exception error) { Check(error.GetType() == typeof(T), name + " -> " + error.GetType().FullName); }
    }

    private static int Main()
    {
        try
        {
            Assembly assembly = typeof(AchievementCollection).Assembly;
            Console.WriteLine("CLR " + Environment.Version + " " + typeof(object).Assembly.Location);
            Console.WriteLine("XNA " + assembly.FullName + " " + assembly.Location);
            Check(Type.GetType("Mono.Runtime") == null, "genuine Framework CLR");
            Check(assembly.GetName().Version == new Version(4, 0, 0, 0), "XNA version4");
            Check(BitConverter.ToString(assembly.GetName().GetPublicKeyToken()) == "84-2C-F8-BE-1D-E5-05-53",
                "Microsoft XNA public key token");
            ConstructorInfo constructor = typeof(AchievementCollection).GetConstructor(
                BindingFlags.Instance | BindingFlags.NonPublic, null,
                new Type[] { typeof(List<Achievement>) }, null);
            Check(constructor != null, "internal collection constructor exists");
            if (constructor == null) return 2;
            AchievementCollection collection = (AchievementCollection)constructor.Invoke(
                new object[] { new List<Achievement>() });
            ICollection<Achievement> items = collection;
            IList<Achievement> list = collection;
            Check(items.IsReadOnly, "IsReadOnly");
            Check(collection.Count == 0, "empty collection count");
            Throws<NotSupportedException>(delegate { items.Add(null); }, "Add null refuses");
            Throws<NotSupportedException>(delegate { list.Insert(-1, null); }, "Insert invalid index refuses modification first");
            Throws<NotSupportedException>(delegate { list.Insert(0, null); }, "Insert valid index refuses");
            Throws<NotSupportedException>(delegate { items.Remove(null); }, "Remove absent value refuses");
            Throws<NotSupportedException>(delegate { list.RemoveAt(-1); }, "RemoveAt negative refuses modification first");
            Throws<NotSupportedException>(delegate { list.RemoveAt(0); }, "RemoveAt empty refuses modification first");
            Throws<NotSupportedException>(delegate { items.Clear(); }, "Clear empty refuses");
            Throws<NotSupportedException>(delegate { list[0] = null; }, "index assignment refuses");
            Throws<ArgumentOutOfRangeException>(delegate { GC.KeepAlive(collection[0]); }, "numeric missing index");
            Throws<ArgumentOutOfRangeException>(delegate { GC.KeepAlive(collection[-1]); }, "numeric negative index");
            Throws<KeyNotFoundException>(delegate { GC.KeepAlive(collection["missing"]); }, "missing string key");
            Throws<KeyNotFoundException>(delegate { GC.KeepAlive(collection[""]); }, "empty string key");
            Throws<ArgumentNullException>(delegate { GC.KeepAlive(collection[(string)null]); }, "null string key");
            Check(!items.Contains(null), "Contains absent value");
            Check(list.IndexOf(null) == -1, "IndexOf absent value");
            items.CopyTo(new Achievement[0], 0);
            Check(collection.Count == 0, "CopyTo and refused mutations preserve contents");
            collection.Dispose();
            Check(collection.IsDisposed, "Dispose marks collection");
            Check(items.IsReadOnly, "Dispose preserves read-only contract");
            Console.WriteLine("RESULT checks=" + checks + " failures=" + failures);
            return failures == 0 ? 0 : 1;
        }
        catch (Exception error)
        {
            Console.WriteLine("UNEXPECTED " + error);
            return 2;
        }
    }
}
