// Genuine XNA StorageDevice.DeleteContainer and StorageContainer path operations whose native
// failures CNA mapped differently (CNA-DEMOS-ISSUE-010 residual audit). Runs like
// StorageContainerCreationOracle: real device/container code in an isolated Wine profile whose
// Documents folder is the empty fixture args[0]. Mode "create" measures deletion scope and
// prepares two containers; the runner then removes search permission from AllPlayers\locked and
// write permission from AllPlayers\inner and AllPlayers\ro (Unix permissions mapped by Wine, not
// Windows ACLs); mode "denied" measures the operations beneath them.
using System;
using System.IO;
using System.Reflection;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Storage;

static class StorageResidualPathsOracle
{
    static int checks, failed;
    static void Check(bool condition, string label)
    {
        ++checks;
        Console.WriteLine((condition ? "PASS " : "FAIL ") + label);
        if (!condition) ++failed;
    }
    static void Throws<T>(Action action, string label) where T : Exception
    {
        Exception caught = null;
        try { action(); } catch (Exception e) { caught = e; }
        Check(caught != null && caught.GetType() == typeof(T),
              label + " actual=" + (caught == null ? "none" : caught.GetType().FullName));
        if (caught != null) Console.WriteLine("  message=" + caught.Message);
    }
    static void Returns(Func<bool> query, bool expected, string label)
    {
        Exception caught = null;
        bool value = !expected;
        try { value = query(); } catch (Exception e) { caught = e; }
        Check(caught == null && value == expected,
              label + " actual=" + (caught == null ? value.ToString() : caught.GetType().FullName));
    }
    static StorageDevice Device(PlayerIndex? player)
    {
        IAsyncResult result = player.HasValue
            ? StorageDevice.BeginShowSelector(player.Value, null, null)
            : StorageDevice.BeginShowSelector(null, null);
        try { return StorageDevice.EndShowSelector(result); }
        finally { result.AsyncWaitHandle.Close(); }
    }
    static StorageContainer Open(StorageDevice device, string name)
    {
        return device.EndOpenContainer(device.BeginOpenContainer(name, null, null));
    }

    static int Main(string[] args)
    {
        if (args.Length != 2 || (args[1] != "create" && args[1] != "denied")) return 2;
        string personal = Environment.GetFolderPath(Environment.SpecialFolder.Personal);
        string fixture = Path.GetFullPath(args[0]).TrimEnd('\\');
        if (personal.Length == 0 ||
            !string.Equals(Path.GetFullPath(personal).TrimEnd('\\'), fixture,
                           StringComparison.OrdinalIgnoreCase))
        {
            Console.WriteLine("REFUSED personal=" + personal + " fixture=" + fixture);
            return 2;
        }
        Console.WriteLine(typeof(StorageContainer).Assembly.FullName);
        string title = Path.GetFileNameWithoutExtension(Assembly.GetEntryAssembly().Location);
        string titlePath = Path.Combine(Path.Combine(personal, "SavedGames"), title);
        StorageDevice shared = Device(null), one = Device(PlayerIndex.One);

        if (args[1] == "create")
        {
            if (Directory.GetFileSystemEntries(personal).Length != 0)
            {
                Console.WriteLine("REFUSED non-empty fixture " + personal);
                return 2;
            }
            using (StorageContainer c = Open(shared, "both")) using (Stream s = c.CreateFile("shared.bin")) { }
            using (StorageContainer c = Open(one, "both")) using (Stream s = c.CreateFile("one.bin")) { }
            shared.DeleteContainer("both");
            Check(!Directory.Exists(Path.Combine(titlePath, "both\\AllPlayers")),
                  "shared DeleteContainer removes the shared player folder");
            Check(File.Exists(Path.Combine(titlePath, "both\\Player1\\one.bin")),
                  "shared DeleteContainer keeps another player's folder");
            Check(Directory.Exists(Path.Combine(titlePath, "both")), "DeleteContainer keeps the named folder");
            one.DeleteContainer("both");
            Check(!Directory.Exists(Path.Combine(titlePath, "both\\Player1")),
                  "a player device removes its own player folder");

            Returns(() => { shared.DeleteContainer("missing"); return true; }, true, "missing container is a no-op");
            Check(!Directory.Exists(Path.Combine(titlePath, "missing")), "deleting a missing container creates nothing");

            File.WriteAllText(Path.Combine(titlePath, "blocked"), "seed");
            Returns(() => { shared.DeleteContainer("blocked"); return true; }, true, "file at the name is a no-op");
            Check(File.ReadAllText(Path.Combine(titlePath, "blocked")) == "seed", "file at the name is kept");

            Directory.CreateDirectory(Path.Combine(titlePath, "AllPlayers"));
            File.WriteAllText(Path.Combine(titlePath, "AllPlayers\\title-level.bin"), "seed");
            Returns(() => { shared.DeleteContainer(""); return true; }, true, "empty name does not throw");
            Console.WriteLine("  empty name removed title\\AllPlayers="
                              + !Directory.Exists(Path.Combine(titlePath, "AllPlayers")));

            using (StorageContainer c = Open(shared, "queries"))
            {
                using (Stream s = c.CreateFile("seed.bin")) { }
                Returns(() => c.FileExists("seed.bin\\child"), false, "FileExists beneath a file");
                Returns(() => c.DirectoryExists("seed.bin\\child"), false, "DirectoryExists beneath a file");
            }

            using (StorageContainer c = Open(shared, "locked"))
            {
                c.CreateDirectory("locked\\sub");
                using (Stream s = c.CreateFile("locked\\file.bin")) { }
                c.CreateDirectory("ro\\victim");
            }
            using (StorageContainer c = Open(shared, "undeletable"))
            {
                c.CreateDirectory("inner");
                using (Stream s = c.CreateFile("inner\\file.bin")) { }
            }
            Console.WriteLine("prepared title=" + title);
        }
        else
        {
            using (StorageContainer c = Open(shared, "locked"))
            {
                Returns(() => c.FileExists("locked\\file.bin"), false, "FileExists beneath an unsearchable directory");
                Returns(() => c.DirectoryExists("locked\\sub"), false, "DirectoryExists beneath an unsearchable directory");
                Throws<UnauthorizedAccessException>(() => c.OpenFile("locked\\file.bin", FileMode.Open).Dispose(),
                                                    "OpenFile beneath an unsearchable directory");
                Throws<UnauthorizedAccessException>(() => c.CreateFile("locked\\new.bin").Dispose(),
                                                    "CreateFile beneath an unsearchable directory");
                Throws<UnauthorizedAccessException>(() => c.DeleteFile("locked\\file.bin"),
                                                    "DeleteFile beneath an unsearchable directory");
                Throws<UnauthorizedAccessException>(() => c.CreateDirectory("locked\\new"),
                                                    "CreateDirectory beneath an unsearchable directory");
                Throws<UnauthorizedAccessException>(() => c.DeleteDirectory("locked\\sub"),
                                                    "DeleteDirectory beneath an unsearchable directory");
                Throws<IOException>(() => c.DeleteDirectory("ro\\victim"),
                                    "DeleteDirectory in an unwritable parent");
                Returns(() => c.DirectoryExists("ro\\victim"), true, "the refused directory remains");
            }
            // Wine defers unlink, so XNA's recursive delete sees "Directory is not empty" here.
            Throws<IOException>(() => shared.DeleteContainer("undeletable"),
                                "DeleteContainer with an undeletable file");
            Console.WriteLine("  undeletable after: file="
                              + File.Exists(Path.Combine(titlePath, "undeletable\\AllPlayers\\inner\\file.bin"))
                              + " folder=" + Directory.Exists(Path.Combine(titlePath, "undeletable\\AllPlayers")));
        }
        Console.WriteLine("checks=" + checks + " failed=" + failed);
        return failed == 0 ? 0 : 1;
    }
}
