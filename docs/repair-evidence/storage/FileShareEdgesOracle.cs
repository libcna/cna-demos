// .NET Framework4 reference for FileShare lifetime, alias and pre-mutation refusal.
using System;
using System.Diagnostics;
using System.IO;
using System.Reflection;

static class FileShareEdgesOracle
{
    static int checks, failed;
    static readonly byte[] seed = {1, 2, 3, 4};
    static void Check(bool ok, string name)
    {
        ++checks;
        if (!ok) { ++failed; Console.WriteLine("FAIL " + name); }
    }
    static bool Refuses(string path, FileMode mode, FileAccess access, FileShare share)
    {
        try { using (var stream = new FileStream(path, mode, access, share)) {} return false; }
        catch (IOException) { return true; }
    }
    static bool SeedUnchanged(string path)
    {
        byte[] bytes = File.ReadAllBytes(path);
        return bytes.Length == 4 && bytes[0] == 1 && bytes[1] == 2 && bytes[2] == 3 && bytes[3] == 4;
    }
    static int Child(string path)
    {
        var info = new ProcessStartInfo(Assembly.GetExecutingAssembly().Location,
                                       "--child \"" + path + "\"");
        info.UseShellExecute = false;
        using (var child = Process.Start(info)) {
            child.WaitForExit();
            return child.ExitCode;
        }
    }
    static int Main(string[] args)
    {
        if (args.Length == 2 && args[0] == "--child")
            return Refuses(args[1], FileMode.Open, FileAccess.Read, FileShare.ReadWrite) ? 17 : 0;
        if (args.Length != 1 || Directory.Exists(args[0])) return 2;
        Directory.CreateDirectory(args[0]);
        string path = Path.GetFullPath(Path.Combine(args[0], "seed.bin"));
        try {
            Directory.CreateDirectory(Path.Combine(args[0], "child"));
            foreach (FileMode mode in new[] {FileMode.Create, FileMode.Truncate}) {
                File.WriteAllBytes(path, seed);
                using (var first = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.None)) {
                    Check(Refuses(path, mode, FileAccess.Write, FileShare.ReadWrite), mode + " denied while exclusive");
                    Check(first.Length == 4 && first.ReadByte() == 1, mode + " refusal preserves open handle data");
                }
                Check(SeedUnchanged(path), mode + " refusal preserves file bytes");
            }
            using (var first = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.None)) {
                string alias = Path.Combine(args[0], "child", "..", "seed.bin");
                Check(Refuses(alias, FileMode.Open, FileAccess.Read, FileShare.ReadWrite), "normalized alias observes exclusive handle");
                Check(Child(path) == 17, "exclusive policy reaches another process");
                first.Close(); first.Close();
                Check(!Refuses(path, FileMode.Open, FileAccess.Read, FileShare.None), "Close releases sharing and is idempotent");
            }
            Check(Child(path) == 0, "child opens after exclusive handle closes");
            try { using (var missing = new FileStream(path + ".missing", FileMode.Open, FileAccess.Read, FileShare.None)) {} }
            catch (FileNotFoundException) {}
            using (var created = new FileStream(path + ".missing", FileMode.CreateNew, FileAccess.Write, FileShare.None))
                Check(created.CanWrite, "failed constructor does not retain sharing reservation");
            foreach (FileMode mode in new[] {FileMode.Open, FileMode.OpenOrCreate}) {
                File.WriteAllBytes(path, seed);
                using (var writer = new FileStream(path, mode, FileAccess.Write, FileShare.ReadWrite)) {
                    Check(writer.Length == 4 && writer.Position == 0, mode + " write-only open preserves length and starts at zero");
                    writer.WriteByte(9);
                }
                byte[] bytes = File.ReadAllBytes(path);
                Check(bytes.Length == 4 && bytes[0] == 9 && bytes[1] == 2 && bytes[3] == 4,
                      mode + " write-only overwrite does not truncate remaining bytes");
            }
            Console.WriteLine("checks=" + checks + " failed=" + failed);
            return failed == 0 ? 0 : 1;
        }
        finally { Directory.Delete(args[0], true); } // Only this newly created isolated fixture.
    }
}
