// Actual .NET directory semantics reference for CNA-DEMOS-ISSUE-010.
using System;
using System.IO;

static class DirectoryOperationsOracle
{
    static int checks, failures;
    static void Check(bool value, string label) {
        ++checks; if (!value) ++failures;
        Console.WriteLine((value ? "PASS " : "FAIL ") + label);
    }
    static void Throws<T>(Action action, string label) where T : Exception {
        Exception caught = null;
        try { action(); } catch (Exception error) { caught = error; }
        Check(caught != null && caught.GetType() == typeof(T), label + " actual=" +
            (caught == null ? "none" : caught.GetType().FullName));
    }
    static int Main(string[] args) {
        if (args.Length != 1 || File.Exists(args[0]) || Directory.Exists(args[0])) return 2;
        string root = Path.GetFullPath(args[0]); Directory.CreateDirectory(root);
        Console.WriteLine("Runtime=" + Environment.Version + " OS=" + Environment.OSVersion.Platform);
        try {
            string prefix = Path.Combine(root, "prefix");
            Throws<ArgumentException>(() => Directory.CreateDirectory(prefix + "\0suffix"), "NUL create refused");
            Check(!Directory.Exists(prefix), "NUL prefix not created");
            Throws<ArgumentException>(() => Directory.GetFiles(root + "\0suffix"), "NUL files refused");
            Throws<ArgumentException>(() => Directory.GetFiles(root + "\0suffix", "*"), "NUL filtered files refused");
            Throws<ArgumentException>(() => Directory.GetDirectories(root + "\0suffix"), "NUL directories refused");
            string parent = Path.Combine(root, "žluťoučký-世界");
            string child = Path.Combine(parent, "😀");
            Directory.CreateDirectory(child);
            Check(Directory.Exists(child), "Unicode child created");
            string file = Path.Combine(parent, "soubor-世界.bin"); File.WriteAllText(file, "keep");
            Check(String.Join("|", Directory.GetFiles(parent)) == file, "Unicode files returned");
            Check(String.Join("|", Directory.GetFiles(parent, "*.bin")) == file, "Unicode filtered files returned");
            Check(String.Join("|", Directory.GetDirectories(parent)) == child, "Unicode directories returned");
            Throws<IOException>(() => Directory.CreateDirectory(file), "existing file refused");
            Check(File.ReadAllText(file) == "keep", "file preserved");
            Directory.CreateDirectory(child);
            Check(Directory.Exists(child), "existing directory accepted");
            Console.WriteLine("checks=" + checks + " failures=" + failures);
            return failures == 0 ? 0 : 1;
        } finally { Directory.Delete(root, true); }
    }
}
