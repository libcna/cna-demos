// Genuine XNA4 ISSUE-151: disposed access from inside the Disposing event.
// Root injection deliberately does not claim device-selection/constructor coverage.
using System;
using System.IO;
using System.Reflection;
using System.Runtime.Serialization;
using Microsoft.Xna.Framework.Storage;

static class StorageDisposingOracle
{
    static int checks, failed;
    static void Check(bool ok, string name)
    {
        ++checks;
        if (!ok) { ++failed; Console.WriteLine("FAIL " + name); }
    }
    static void Disposed(Action action, string name)
    {
        Type caught = null;
        try { action(); } catch (Exception error) { caught = error.GetType(); }
        Check(caught == typeof(ObjectDisposedException), name);
    }
    static int Main(string[] args)
    {
        if (args.Length != 1 || Directory.Exists(args[0])) return 2;
        Directory.CreateDirectory(args[0]);
        try {
            string root = Path.GetFullPath(args[0]);
            string file = Path.Combine(root, "seed.bin");
            File.WriteAllBytes(file, new byte[] {17});
            var container = (StorageContainer)FormatterServices.GetUninitializedObject(typeof(StorageContainer));
            typeof(StorageContainer).GetField("_rootPath", BindingFlags.Instance | BindingFlags.NonPublic).SetValue(container, root);
            int events = 0;
            using (Stream stream = container.OpenFile("seed.bin", FileMode.Open, FileAccess.Read)) {
                container.Disposing += (sender, e) => {
                    ++events;
                    Check(sender == container && e == EventArgs.Empty && container.IsDisposed, "event identity and disposed state");
                    Disposed(() => container.FileExists("seed.bin"), "FileExists is guarded inside the event");
                    Disposed(() => container.GetFileNames(), "listing is guarded inside the event");
                    Check(File.Exists(file), "disposal does not delete backing content");
                    Check(stream.ReadByte() == 17, "an existing stream remains readable during the event");
                };
                container.Dispose();
                container.Dispose();
                Check(events == 1, "event runs only once");
                stream.Position = 0;
                Check(stream.ReadByte() == 17, "an existing stream remains readable after container disposal");
            }
            Console.WriteLine("checks=" + checks + " failed=" + failed);
            return failed == 0 ? 0 : 1;
        }
        finally { Directory.Delete(args[0], true); }
    }
}
