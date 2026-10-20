// Genuine XNA4 ISSUE-145 contract; fixtures are relative to this executable.
using System;
using System.IO;
using System.Reflection;
using Microsoft.Xna.Framework;

static class TitlePathsOracle
{
    static int checks, failed;
    static void Check(bool ok, string name)
    {
        ++checks;
        if (!ok) { ++failed; Console.WriteLine("FAIL " + name); }
    }
    static void Reject<T>(string path) where T : Exception
    {
        Type caught = null;
        try { using (var stream = TitleContainer.OpenStream(path)) {} }
        catch (Exception e) { caught = e.GetType(); }
        Check(caught == typeof(T), "path=" + path + " expected=" + typeof(T).Name + " actual=" + caught);
    }
    static int Main()
    {
        const string name = "title-paths-xna-fixture";
        string root = Path.Combine(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location), name);
        string cwd = Environment.CurrentDirectory;
        if (Directory.Exists(root)) return 2;
        Directory.CreateDirectory(Path.Combine(root, "child"));
        try {
            File.WriteAllBytes(Path.Combine(root, "seed.bin"), new byte[] {1, 2, 3, 4});
            Environment.CurrentDirectory = Path.Combine(root, "child");
            foreach (string relative in new[] {name + "/seed.bin", name + "\\seed.bin",
                     name + "/./seed.bin", name + "/child/../seed.bin"}) {
                using (var stream = TitleContainer.OpenStream(relative))
                    Check(stream.Length == 4 && stream.ReadByte() == 1 && stream.CanRead &&
                          stream.CanSeek && !stream.CanWrite, "relative content/control " + relative);
            }
            Reject<ArgumentNullException>("");
            Reject<ArgumentException>(Path.Combine(root, "seed.bin"));
            Reject<ArgumentException>("/rooted.bin");
            Reject<ArgumentException>("\\rooted.bin");
            Reject<ArgumentException>("C:drive-relative.bin");
            Reject<ArgumentException>("../outside.bin");
            foreach (char bad in new[] {':', '*', '?', '"', '<', '>', '|'})
                Reject<ArgumentException>(name + "/bad" + bad + ".bin");
            Reject<FileNotFoundException>(name + "/missing.bin");
            Reject<FileNotFoundException>(name + "/missing/child.bin");
            Reject<InvalidOperationException>(name + "/child");
            foreach (string cleanedEmpty in new[] {".", "./", name + "/..", name + "/../"})
                Reject<InvalidOperationException>(cleanedEmpty);
            Reject<ArgumentException>(new string('a', 65520));
            Reject<InvalidOperationException>(new string('a', 65519));
            string astral = char.ConvertFromUtf32(0x1F600);
            string astralLong = new System.Text.StringBuilder().Insert(0, astral, 32760).ToString();
            Reject<ArgumentException>(astralLong);
            Reject<InvalidOperationException>(astralLong.Substring(2));
            Console.WriteLine("checks=" + checks + " failed=" + failed);
            return failed == 0 ? 0 : 1;
        }
        finally { Environment.CurrentDirectory = cwd; Directory.Delete(root, true); }
    }
}
