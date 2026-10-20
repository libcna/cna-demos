// The same direct FileStream matrix under Framework4/Windows and modern .NET/Unix.
// Nonzero exit reports disagreement with the genuine Windows contract, not a pass.
using System;
using System.IO;

static class FileSharePlatformOracle
{
    static int Main(string[] args)
    {
        if (args.Length != 1 || File.Exists(args[0])) return 2;
        File.WriteAllBytes(args[0], new byte[] {1, 2, 3, 4});
        int checks = 0, mismatches = 0, opened = 0, refused = 0, exclusiveErrors = 0;
        try {
            FileAccess[] accesses = {FileAccess.Read, FileAccess.Write, FileAccess.ReadWrite};
            FileShare[] shares = {FileShare.None, FileShare.Read, FileShare.Write, FileShare.ReadWrite};
            foreach (var a1 in accesses) foreach (var s1 in shares)
            foreach (var a2 in accesses) foreach (var s2 in shares) {
                bool actual = false;
                using (var first = new FileStream(args[0], FileMode.Open, a1, s1)) {
                    try { using (var second = new FileStream(args[0], FileMode.Open, a2, s2)) { actual = true; } }
                    catch (IOException) {}
                }
                bool windows = ((int)s1 & (int)a2) == (int)a2 && ((int)s2 & (int)a1) == (int)a1;
                ++checks;
                if (actual) ++opened; else ++refused;
                if (actual != windows) ++mismatches;
                if (actual && (s1 == FileShare.None || s2 == FileShare.None)) ++exclusiveErrors;
                Console.WriteLine((int)a1 + "/" + (int)s1 + "->" + (int)a2 + "/" + (int)s2 +
                                  " actual=" + (actual ? "OPEN" : "REFUSED") +
                                  " windows=" + (windows ? "OPEN" : "REFUSED"));
            }
            Console.WriteLine("runtime=" + Environment.Version + " platform=" + Environment.OSVersion.Platform);
            Console.WriteLine("checks=" + checks + " open=" + opened + " refused=" + refused +
                              " windows_mismatches=" + mismatches + " exclusive_errors=" + exclusiveErrors);
            return mismatches == 0 ? 0 : 1;
        }
        finally { File.Delete(args[0]); }
    }
}
