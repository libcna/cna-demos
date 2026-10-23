// Expanded genuine reference: Delete/Inheritable masks and append boundaries.
// Prints measured policy; does not assume Windows and Unix share the same matrix.
using System;
using System.IO;

static class FileShareFlagsOracle
{
    static int Main(string[] args)
    {
        if (args.Length != 1 || File.Exists(args[0]) || Directory.Exists(args[0])) return 2;
        string path = args[0];
        File.WriteAllBytes(path, new byte[] {1, 2, 3, 4});
        int opened = 0, refused = 0, mismatches = 0, checks = 0;
        bool windows = Environment.OSVersion.Platform == PlatformID.Win32NT;
        try {
            for (int firstMask = 0; firstMask < 24; ++firstMask) {
                if ((firstMask & 8) != 0) continue;
                for (int nextMask = 0; nextMask < 24; ++nextMask) {
                    if ((nextMask & 8) != 0) continue;
                    for (int a = 1; a <= 3; ++a) for (int b = 1; b <= 3; ++b) {
                        bool admitted = false;
                        using (var first = new FileStream(path, FileMode.Open, (FileAccess)a, (FileShare)firstMask)) {
                            try { using (var next = new FileStream(path, FileMode.Open, (FileAccess)b, (FileShare)nextMask)) { admitted = true; } }
                            catch (IOException) {}
                        }
                        bool expected = windows ? (b & ~firstMask) == 0 && (a & ~nextMask) == 0
                                                : firstMask != 0 && nextMask != 0;
                        ++checks;
                        if (admitted) ++opened; else ++refused;
                        if (admitted != expected) {
                            ++mismatches;
                            Console.WriteLine("MISMATCH " + a + "/" + firstMask + "->" + b + "/" + nextMask + " admitted=" + admitted);
                        }
                    }
                }
            }
            int appendFailures = 0;
            using (var append = new FileStream(path, FileMode.Append)) {
                if (append.Position != 4) ++appendFailures;
                try { append.Position = 3; ++appendFailures; } catch (IOException) {}
                if (append.Position != 4) ++appendFailures;
                try { append.SetLength(3); ++appendFailures; } catch (IOException) {}
                append.WriteByte(5); append.WriteByte(6);
                append.Position = 4; append.WriteByte(7);
                if (append.Length != 6) ++appendFailures;
            }
            byte[] data = File.ReadAllBytes(path);
            if (data.Length != 6 || data[0] != 1 || data[3] != 4 || data[4] != 7 || data[5] != 6) ++appendFailures;
            Console.WriteLine("runtime=" + Environment.Version + " platform=" + Environment.OSVersion.Platform);
            Console.WriteLine("checks=" + checks + " open=" + opened + " refused=" + refused +
                              " policy_mismatches=" + mismatches + " append_failures=" + appendFailures);
            return mismatches == 0 && appendFailures == 0 ? 0 : 1;
        }
        finally { File.Delete(path); }
    }
}
