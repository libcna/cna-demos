// SPDX-License-Identifier: MS-PL
// Genuine XNA ISSUE-028 path resolution before native audio is reached.
using System;
using System.IO;
using System.Reflection;
using System.Runtime.Serialization;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Audio;

static class WaveBankPathOracle
{
    static int checks, failures;
    static void Check(bool value, string label)
    {
        ++checks;
        if (!value) ++failures;
        Console.WriteLine((value ? "PASS " : "FAIL ") + label);
    }
    static Exception Attempt(AudioEngine engine, string path, bool streaming)
    {
        try
        {
            // Every fixture is missing or has an invalid header. The genuine constructor must
            // refuse before reading engineHandle or creating any native audio resource.
            using (WaveBank bank = streaming ? new WaveBank(engine, path, 0, 16) :
                                               new WaveBank(engine, path)) { }
            return null;
        }
        catch (Exception error) { return error; }
    }
    static bool MissingAt(Exception error, string path)
    {
        return error != null && error.GetType() == typeof(FileNotFoundException) &&
            String.Equals(((FileNotFoundException)error).FileName, Path.GetFullPath(path),
                          StringComparison.OrdinalIgnoreCase);
    }
    static int Main()
    {
        string original = Directory.GetCurrentDirectory();
        string relative = "cna-xna-wave-path-" + Guid.NewGuid().ToString("N");
        // Unlike CNA's extension, genuine XNA's TitleLocation type is internal.
        Type titleLocation = typeof(Curve).Assembly.GetType("Microsoft.Xna.Framework.TitleLocation", true);
        string titlePath = (string)titleLocation.GetProperty("Path",
            BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic).GetValue(null, null);
        string title = Path.Combine(titlePath, relative);
        string working = Path.Combine(title, "working");
        if (Directory.Exists(title)) throw new IOException("Private fixture directory already exists");
        Directory.CreateDirectory(title);
        Directory.CreateDirectory(Path.Combine(working, relative));
        string titleFile = Path.Combine(title, "asset.xwb");
        File.WriteAllBytes(titleFile, new byte[] { 0, 1, 2, 3, 4 });
        // No AudioEngine constructor, device acquisition, playback or service call. This object
        // is only the non-null token needed to reach the constructor's managed file validation.
        try
        {
            AudioEngine engine = (AudioEngine)FormatterServices.GetUninitializedObject(typeof(AudioEngine));
            GC.SuppressFinalize(engine);
            Directory.SetCurrentDirectory(working);
            foreach (bool streaming in new[] { false, true })
            {
                string kind = streaming ? "streaming" : "memory";
                string requested = Path.Combine(relative, "asset.xwb");
                string cwdFile = Path.Combine(working, requested);
                Check(MissingAt(Attempt(engine, requested, streaming), cwdFile),
                      kind + " relative path targets cwd, not the existing title file");
                File.WriteAllBytes(cwdFile, new byte[] { 0, 1, 2, 3, 4 });
                Exception invalid = Attempt(engine, requested, streaming);
                Check(invalid != null && invalid.GetType() == typeof(ArgumentException),
                      kind + " relative cwd file reaches managed header validation");
                invalid = Attempt(engine, titleFile, streaming);
                Check(invalid != null && invalid.GetType() == typeof(ArgumentException),
                      kind + " absolute title file reaches the same header validation");
                File.Delete(cwdFile);
                Check(MissingAt(Attempt(engine, requested, streaming), cwdFile),
                      kind + " subsequent relative lookup still uses cwd");
                string missing = Path.Combine(title, "missing.xwb");
                Check(MissingAt(Attempt(engine, missing, streaming), missing),
                      kind + " missing absolute file retains its exact name");
                Exception nullEngine = Attempt(null, requested, streaming);
                Check(nullEngine != null && nullEngine.GetType() == typeof(ArgumentNullException),
                      kind + " null engine is refused before file lookup");
                Exception empty = Attempt(engine, "", streaming);
                Check(empty != null && empty.GetType() == typeof(ArgumentNullException),
                      kind + " empty filename is refused before file lookup");
            }
        }
        finally
        {
            Directory.SetCurrentDirectory(original);
            Directory.Delete(title, true); // Only this invocation's unique owned fixture tree.
        }
        Console.WriteLine("checks=" + checks + " failed=" + failures);
        return failures == 0 ? 0 : 1;
    }
}
