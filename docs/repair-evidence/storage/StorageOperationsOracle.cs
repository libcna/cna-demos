// Genuine XNA filesystem exception/data-preservation oracle for ISSUE-010.
// Constructor-free root injection does not validate platform selection or mounting.
using System;
using System.IO;
using System.Reflection;
using System.Runtime.Serialization;
using Microsoft.Xna.Framework.Storage;

static class StorageOperationsOracle
{
    static int checks, failures;
    static void Check(bool ok, string label)
    {
        ++checks; if (!ok) ++failures;
        Console.WriteLine((ok ? "PASS " : "FAIL ") + label);
    }
    static void Throws<T>(Action action, string label) where T : Exception
    {
        Exception caught = null;
        try { action(); } catch (Exception error) { caught = error; }
        Check(caught != null && caught.GetType() == typeof(T), label + " actual=" +
              (caught == null ? "none" : caught.GetType().FullName));
    }
    static StorageContainer Container(string root)
    {
        var container = (StorageContainer)FormatterServices.GetUninitializedObject(typeof(StorageContainer));
        typeof(StorageContainer).GetField("_rootPath", BindingFlags.Instance | BindingFlags.NonPublic)
            .SetValue(container, Path.GetFullPath(root));
        return container;
    }
    static int Main(string[] args)
    {
        if (args.Length != 1 || File.Exists(args[0]) || Directory.Exists(args[0])) return 2;
        string root = args[0];
        Directory.CreateDirectory(root);
        Console.WriteLine(typeof(StorageContainer).Assembly.FullName);
        Console.WriteLine(typeof(StorageContainer).Assembly.Location);
        try {
            using (var container = Container(root)) {
                Directory.CreateDirectory(Path.Combine(root, "empty-directory"));
                Throws<UnauthorizedAccessException>(() => container.DeleteFile("empty-directory"),
                    "DeleteFile refuses a directory with the exact XNA exception");
                Check(Directory.Exists(Path.Combine(root, "empty-directory")), "DeleteFile preserves the directory");
                File.WriteAllText(Path.Combine(root, "seed.bin"), "seed");
                Throws<IOException>(() => container.CreateDirectory("seed.bin"), "CreateDirectory refuses an existing file");
                Check(File.ReadAllText(Path.Combine(root, "seed.bin")) == "seed", "CreateDirectory preserves the file");
                Throws<IOException>(() => container.CreateDirectory("seed.bin/child"), "CreateDirectory refuses a file as parent");
                Throws<DirectoryNotFoundException>(() => container.DeleteFile("absent-parent/file.bin"), "DeleteFile distinguishes missing parent");
                container.DeleteFile("absent-file.bin");
                Check(!container.FileExists("absent-file.bin"), "DeleteFile accepts a missing leaf");
                container.DeleteFile("seed.bin");
                Check(!container.FileExists("seed.bin"), "DeleteFile still removes a real file");
            }
            using (var missing = Container(Path.Combine(root, "absent-root"))) {
                Throws<DirectoryNotFoundException>(() => missing.GetFileNames(), "GetFileNames missing root");
                Throws<DirectoryNotFoundException>(() => missing.GetFileNames("*"), "GetFileNames pattern missing root");
                Throws<DirectoryNotFoundException>(() => missing.GetDirectoryNames(), "GetDirectoryNames missing root");
                Throws<DirectoryNotFoundException>(() => missing.GetDirectoryNames("*"), "GetDirectoryNames pattern missing root");
            }
            Console.WriteLine("checks=" + checks + " failures=" + failures);
            return failures == 0 ? 0 : 1;
        }
        finally { Directory.Delete(root, true); } // Only this invocation's newly created fixture.
    }
}
