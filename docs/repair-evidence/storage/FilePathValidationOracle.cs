// Genuine Framework4: invalid path rejection must precede truncation/open side effects.
using System;
using System.IO;

static class FilePathValidationOracle
{
    static int checks, failed;
    static void Check(bool value, string name)
    {
        ++checks;
        if (!value) { ++failed; Console.WriteLine("FAIL " + name); }
    }
    static Type Error(string path, FileMode mode, FileAccess access)
    {
        try { using (var stream = new FileStream(path, mode, access)) {} return null; }
        catch (Exception error) { return error.GetType(); }
    }
    static int Main(string[] args)
    {
        bool framework4 = args.Length == 2 && args[1] == "--framework4";
        if ((args.Length != 1 && !framework4) || Directory.Exists(args[0])) return 2;
        Directory.CreateDirectory(args[0]);
        string seed = Path.Combine(args[0], "seed.bin");
        try {
            foreach (FileMode mode in new[] {FileMode.CreateNew, FileMode.Create, FileMode.Open,
                                             FileMode.OpenOrCreate, FileMode.Truncate, FileMode.Append})
            foreach (FileAccess access in new[] {FileAccess.Read, FileAccess.Write, FileAccess.ReadWrite}) {
                File.WriteAllText(seed, "retained");
                Check(Error(seed + "\0suffix", mode, access) == typeof(ArgumentException), "NUL " + mode + "/" + access);
                Check(File.ReadAllText(seed) == "retained", "bytes preserved " + mode + "/" + access);
                Check(Error("", mode, access) == typeof(ArgumentException), "empty " + mode + "/" + access);
                // Framework4 checks Append/ReadWrite only after full-path validation;
                // current .NET checks it before. Keep the version difference explicit.
                bool invalid = (mode == FileMode.Append && access != FileAccess.Write && !framework4) ||
                    (access == FileAccess.Read && (mode == FileMode.CreateNew || mode == FileMode.Create ||
                                                 mode == FileMode.Truncate || mode == FileMode.Append));
                Type longError = Error(new string('a', 65519), mode, access);
                Check(longError == (invalid ? typeof(ArgumentException) : typeof(PathTooLongException)),
                    "long " + mode + "/" + access + " actual=" + longError);
            }
            Console.WriteLine("checks=" + checks + " failed=" + failed);
            return failed == 0 ? 0 : 1;
        }
        finally { File.Delete(seed); Directory.Delete(args[0]); }
    }
}
