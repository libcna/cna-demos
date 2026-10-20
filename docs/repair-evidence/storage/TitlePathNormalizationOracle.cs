// Genuine XNA4 ISSUE-145: record normalization before deciding native path rules.
using System;
using System.IO;
using System.Reflection;
using System.Text;
using Microsoft.Xna.Framework;

static class TitlePathNormalizationOracle
{
    static int Main()
    {
        Console.WriteLine("framework=" + typeof(TitleContainer).Assembly.FullName);
        Console.WriteLine("frameworkPath=" + typeof(TitleContainer).Assembly.Location);
        MethodInfo clean = typeof(TitleContainer).GetMethod("GetCleanPath", BindingFlags.Static | BindingFlags.NonPublic);
        MethodInfo absolute = typeof(TitleContainer).GetMethod("IsPathAbsolute", BindingFlags.Static | BindingFlags.NonPublic);
        if (clean == null || absolute == null) return 2;
        string root = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
        const string fixture = "title-normalization-fixture";
        string directory = Path.Combine(root, fixture);
        if (Directory.Exists(directory)) return 2;
        Directory.CreateDirectory(Path.Combine(directory, "child"));
        try {
            File.WriteAllBytes(Path.Combine(directory, "seed.bin"), new byte[] {37});
            string[] paths = {
                "seed.bin", "./seed.bin", "././seed.bin", "child/../seed.bin",
                "missing/../seed.bin", "bad*/../seed.bin", "../seed.bin", "../../seed.bin",
                "child/../../seed.bin", "../child/../../seed.bin", ".//seed.bin",
                "child//../seed.bin", "child/././../seed.bin", "child/.././seed.bin",
                "seed.bin/.", "child/..", "child/../", ".", "./", "..", "../..",
                "./..", "child/../..", "child/../../", "", "/seed.bin", "C:seed.bin",
                "bad?.bin", "bad\0.bin", "bad\n.bin", "seed.bin ", "SEED.BIN"
            };
            foreach (string suffix in paths) {
                string path = fixture + "/" + suffix;
                string normalized = (string)clean.Invoke(null, new object[] {path});
                bool rooted = (bool)absolute.Invoke(null, new object[] {path});
                string result;
                try {
                    using (Stream stream = TitleContainer.OpenStream(path))
                        result = "OPEN:" + stream.ReadByte();
                }
                catch (Exception error) {
                    result = error.GetType().Name;
                    if (error.InnerException != null) result += ":inner=" + error.InnerException.GetType().Name;
                }
                Console.WriteLine(Convert.ToBase64String(Encoding.UTF8.GetBytes(path)) + "\t" +
                    Convert.ToBase64String(Encoding.UTF8.GetBytes(normalized)) + "\t" + rooted + "\t" + result);
            }
            return 0;
        }
        finally { Directory.Delete(directory, true); }
    }
}
