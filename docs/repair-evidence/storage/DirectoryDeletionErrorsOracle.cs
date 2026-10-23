// Actual .NET recursive Directory.Delete and Exists queries on Unix with permission-denied
// components: the lower-layer contract behind StorageDevice.DeleteContainer and the container's
// Exists queries (StorageResidualPathsOracle.cs measures XNA through Wine, whose deferred unlink
// turns an undeletable file into "Directory is not empty"). args[0] must not exist.
using System;
using System.IO;
using System.Runtime.InteropServices;

static class DirectoryDeletionErrorsOracle
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
    const UnixFileMode All = UnixFileMode.UserRead | UnixFileMode.UserWrite | UnixFileMode.UserExecute;
    static int Main(string[] args)
    {
        if (args.Length != 1 || File.Exists(args[0]) || Directory.Exists(args[0])) return 2;
        if (!RuntimeInformation.IsOSPlatform(OSPlatform.Linux)) return 2;
        Console.WriteLine("Runtime=" + Environment.Version + " OS=" + Environment.OSVersion.Platform);
        string root = Path.GetFullPath(args[0]);
        string inner = Path.Combine(root, "tree", "inner"), locked = Path.Combine(root, "locked");
        Directory.CreateDirectory(inner);
        Directory.CreateDirectory(Path.Combine(locked, "sub"));
        try
        {
            File.WriteAllText(Path.Combine(inner, "file.bin"), "seed");
            File.SetUnixFileMode(inner, UnixFileMode.UserRead | UnixFileMode.UserExecute);
            Throws<UnauthorizedAccessException>(() => Directory.Delete(Path.Combine(root, "tree"), true),
                                                "recursive delete with an undeletable file");
            Check(File.Exists(Path.Combine(inner, "file.bin")), "the undeletable file remains");

            File.WriteAllText(Path.Combine(locked, "file.bin"), "seed");
            File.SetUnixFileMode(locked, UnixFileMode.UserRead | UnixFileMode.UserWrite);
            Check(!File.Exists(Path.Combine(locked, "file.bin")), "File.Exists beneath an unsearchable directory is false");
            Check(!Directory.Exists(Path.Combine(locked, "sub")), "Directory.Exists beneath an unsearchable directory is false");
            Throws<UnauthorizedAccessException>(() => Directory.Delete(Path.Combine(locked, "sub")),
                                                "Directory.Delete beneath an unsearchable directory");
        }
        finally
        {
            File.SetUnixFileMode(inner, All);
            File.SetUnixFileMode(locked, All);
            Directory.Delete(root, true); // Only this invocation's newly created fixture.
        }
        Console.WriteLine("checks=" + checks + " failures=" + failures);
        return failures == 0 ? 0 : 1;
    }
}
