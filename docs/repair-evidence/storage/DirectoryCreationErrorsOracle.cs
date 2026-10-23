// Actual .NET Directory.CreateDirectory native-failure categories on Unix, the lower-layer
// contract behind StorageContainer construction (StorageContainerCreationOracle.cs).
// args[0] must not exist; it becomes this run's private fixture.
using System;
using System.IO;
using System.Runtime.InteropServices;

static class DirectoryCreationErrorsOracle
{
    static int checks, failures;
    static void Check(bool condition, string label)
    {
        ++checks;
        Console.WriteLine((condition ? "PASS " : "FAIL ") + label);
        if (!condition) ++failures;
    }
    static void Throws<T>(Action action, string label) where T : Exception
    {
        Exception caught = null;
        try { action(); } catch (Exception e) { caught = e; }
        Check(caught != null && caught.GetType() == typeof(T),
              label + " actual=" + (caught == null ? "none" : caught.GetType().FullName));
    }
    static int Main(string[] args)
    {
        if (args.Length != 1 || File.Exists(args[0]) || Directory.Exists(args[0])) return 2;
        if (!RuntimeInformation.IsOSPlatform(OSPlatform.Linux)) return 2;
        Console.WriteLine("Runtime=" + Environment.Version + " OS=" + Environment.OSVersion.Platform);
        string root = Path.GetFullPath(args[0]);
        Directory.CreateDirectory(root);
        string denied = Path.Combine(root, "denied");
        try
        {
            string seed = Path.Combine(root, "seed");
            File.WriteAllText(seed, "seed");
            Throws<IOException>(() => Directory.CreateDirectory(seed), "file at the leaf");
            Throws<IOException>(() => Directory.CreateDirectory(Path.Combine(seed, "child", "grandchild")),
                                "file as an ancestor");
            Check(File.ReadAllText(seed) == "seed", "file preserved");

            Directory.CreateDirectory(denied);
            File.SetUnixFileMode(denied, UnixFileMode.UserRead | UnixFileMode.UserExecute);
            Throws<UnauthorizedAccessException>(() => Directory.CreateDirectory(Path.Combine(denied, "child")),
                                                "unwritable parent");
            Check(!Directory.Exists(Path.Combine(denied, "child")), "nothing created under the unwritable parent");

            Throws<PathTooLongException>(() => Directory.CreateDirectory(Path.Combine(root, new string('a', 300))),
                                         "component longer than NAME_MAX");
        }
        finally
        {
            if (Directory.Exists(denied))
                File.SetUnixFileMode(denied, UnixFileMode.UserRead | UnixFileMode.UserWrite | UnixFileMode.UserExecute);
            Directory.Delete(root, true); // Only this invocation's newly created fixture.
        }
        Console.WriteLine("checks=" + checks + " failures=" + failures);
        return failures == 0 ? 0 : 1;
    }
}
