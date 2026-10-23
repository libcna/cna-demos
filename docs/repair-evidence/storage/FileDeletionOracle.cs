// Actual .NET exception and preservation reference for CNA-DEMOS-ISSUE-010.
using System;
using System.IO;

static class FileDeletionOracle
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
        string root = args[0]; Directory.CreateDirectory(root);
        Console.WriteLine("Runtime=" + Environment.Version + " OS=" + Environment.OSVersion.Platform);
        try {
            string directory = Path.Combine(root, "directory"); Directory.CreateDirectory(directory);
            Throws<UnauthorizedAccessException>(() => File.Delete(directory), "directory refused");
            Check(Directory.Exists(directory), "directory preserved");
            Throws<DirectoryNotFoundException>(() => File.Delete(Path.Combine(root, "missing/file")), "missing parent");
            File.Delete(Path.Combine(root, "missing-file")); Check(true, "missing leaf accepted");
            string file = Path.Combine(root, "sentinel"); File.WriteAllText(file, "keep");
            Throws<ArgumentException>(() => File.Delete(file + "\0suffix"), "NUL refused");
            Check(File.ReadAllText(file) == "keep", "NUL prefix preserved");
            File.Delete(file); Check(!File.Exists(file), "file removed");
            Console.WriteLine("checks=" + checks + " failures=" + failures);
            return failures == 0 ? 0 : 1;
        } finally { Directory.Delete(root, true); }
    }
}
