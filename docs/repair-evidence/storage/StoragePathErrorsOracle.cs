// Genuine XNA public storage methods with a fresh isolated root. The constructor-free
// seam does not claim device selection or container construction coverage.
using System;
using System.IO;
using System.Reflection;
using System.Runtime.Serialization;
using Microsoft.Xna.Framework.Storage;

static class StoragePathErrorsOracle
{
    static int checks, failed;
    static void Check(bool ok, string label)
    {
        ++checks; if (!ok) ++failed;
        Console.WriteLine((ok ? "PASS " : "FAIL ") + label);
    }
    static void Throws<T>(Action action, string label, string parameter) where T : Exception
    {
        Exception caught = null;
        try { action(); } catch (Exception e) { caught = e; }
        bool ok = caught != null && caught.GetType() == typeof(T);
        if (parameter != null)
            ok = ok && caught is ArgumentException && ((ArgumentException)caught).ParamName == parameter;
        Check(ok, label + " actual=" + (caught == null ? "none" : caught.GetType().Name)
            + (caught is ArgumentException ? " param=" + ((ArgumentException)caught).ParamName : ""));
    }
    static StorageContainer Container(string root)
    {
        var c = (StorageContainer)FormatterServices.GetUninitializedObject(typeof(StorageContainer));
        typeof(StorageContainer).GetField("_rootPath", BindingFlags.Instance | BindingFlags.NonPublic)
            .SetValue(c, Path.GetFullPath(root));
        return c;
    }
    static int Main(string[] args)
    {
        if (args.Length != 1 || Directory.Exists(args[0]) || File.Exists(args[0])) return 2;
        Directory.CreateDirectory(args[0]);
        Console.WriteLine(typeof(StorageContainer).Assembly.FullName);
        Console.WriteLine(typeof(StorageContainer).Assembly.Location);
        try
        {
            using (var c = Container(args[0]))
            {
                string[] names = { "CreateDirectory", "DirectoryExists", "DeleteDirectory", "CreateFile",
                    "FileExists", "DeleteFile", "OpenFile mode", "OpenFile access", "OpenFile share" };
                string[] parameters = { "directory", "directory", "directory", "file", "file", "file", "file", "file", "file" };
                Action[] empty = {
                    () => c.CreateDirectory(""), () => c.DirectoryExists(""), () => c.DeleteDirectory(""),
                    () => { using (var s = c.CreateFile("")) {} }, () => c.FileExists(""), () => c.DeleteFile(""),
                    () => { using (var s = c.OpenFile("", (FileMode)(-1))) {} },
                    () => { using (var s = c.OpenFile("", (FileMode)(-1), (FileAccess)(-1))) {} },
                    () => { using (var s = c.OpenFile("", (FileMode)(-1), (FileAccess)(-1), (FileShare)(-1))) {} }
                };
                for (int i = 0; i < empty.Length; ++i)
                    Throws<ArgumentNullException>(empty[i], names[i] + " empty before other arguments", parameters[i]);
                File.WriteAllText(Path.Combine(args[0], "seed.txt"), "seed");
                Directory.CreateDirectory(Path.Combine(args[0], "child"));
                Check(c.GetFileNames("").Length == 0, "empty file pattern has no matches");
                Check(c.GetDirectoryNames("").Length == 0, "empty directory pattern has no matches");
                Check(c.GetFileNames().Length == 1 && c.GetDirectoryNames().Length == 1,
                    "unfiltered listing still finds fixtures");
                File.WriteAllText(Path.Combine(args[0], "child", "kept.txt"), "keep");
                Throws<IOException>(() => c.DeleteDirectory("child"), "non-empty directory refuses with exact IOException", null);
                Check(File.ReadAllText(Path.Combine(args[0], "child", "kept.txt")) == "keep", "refusal preserves contents");
                Throws<DirectoryNotFoundException>(() => c.DeleteDirectory("missing"), "missing directory", null);
                c.CreateDirectory("empty"); c.DeleteDirectory("empty");
                Check(!c.DirectoryExists("empty"), "empty directory deletes successfully");
                c.Dispose();
                for (int i = 0; i < empty.Length; ++i)
                    Throws<ObjectDisposedException>(empty[i], names[i] + " disposed before empty", null);
                Throws<ObjectDisposedException>(() => c.GetFileNames(""), "disposed before empty file pattern", null);
                Throws<ObjectDisposedException>(() => c.GetDirectoryNames(""), "disposed before empty directory pattern", null);
            }
            using (var missing = Container(Path.Combine(args[0], "missing-root")))
            {
                Check(missing.GetFileNames("").Length == 0, "empty file pattern before missing-root access");
                Check(missing.GetDirectoryNames("").Length == 0, "empty directory pattern before missing-root access");
            }
            Console.WriteLine("checks=" + checks + " failed=" + failed);
            return failed == 0 ? 0 : 1;
        }
        finally { Directory.Delete(args[0], true); } // Only the newly created fixture.
    }
}
