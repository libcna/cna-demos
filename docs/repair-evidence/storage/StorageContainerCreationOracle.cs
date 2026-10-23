// Genuine XNA StorageDevice.EndOpenContainer, i.e. real StorageContainer construction, against an
// isolated Wine user profile whose Documents folder is the empty fixture named by args[0]. The
// device folder, title folder and container chain come from XNA itself; nothing is injected.
// Mode "create" blocks each level of the chain with a file. Mode "denied" opens beneath a
// directory the runner made unwritable; that maps a Unix permission through Wine, not a
// Windows ACL, and is reported as such.
using System;
using System.IO;
using System.Reflection;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Storage;

static class StorageContainerCreationOracle
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
        bool ok = caught != null && caught.GetType() == typeof(T) && caught.InnerException == null;
        Check(ok, label + " actual=" + (caught == null ? "none" : caught.GetType().FullName)
            + (caught != null && caught.InnerException != null
                ? " inner=" + caught.InnerException.GetType().FullName : ""));
        if (caught != null) Console.WriteLine("  message=" + caught.Message);
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
        IAsyncResult result = device.BeginOpenContainer(name, null, null);
        return device.EndOpenContainer(result);
    }
    static string OnlyDirectory(string parent)
    {
        string[] entries = Directory.GetDirectories(parent);
        return entries.Length == 1 ? Path.GetFileName(entries[0]) : null;
    }
    // Begin must not touch the filesystem; End constructs and must raise the creation failure.
    static void BlockedOpen<T>(StorageDevice device, string name, string blocker, string label)
        where T : Exception
    {
        bool callback = false;
        IAsyncResult result = null;
        Exception beginError = null;
        try { result = device.BeginOpenContainer(name, r => callback = true, null); }
        catch (Exception e) { beginError = e; }
        Check(beginError == null && result != null && result.IsCompleted && callback,
              label + ": Begin completes without filesystem access");
        if (result == null) return;
        Throws<T>(() => device.EndOpenContainer(result), label + ": End reports the creation failure");
        Throws<InvalidOperationException>(() => device.EndOpenContainer(result),
                                          label + ": the failed End consumed its result");
        if (blocker != null)
            Check(File.Exists(blocker) && File.ReadAllText(blocker) == "seed",
                  label + ": the blocking file is preserved");
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
        Console.WriteLine(typeof(StorageContainer).Assembly.Location);
        string title = Path.GetFileNameWithoutExtension(Assembly.GetEntryAssembly().Location);
        StorageDevice shared = Device(null);

        if (args[1] == "denied")
        {
            // Runner-prepared: <saved>\<title>\denied exists and is not writable.
            BlockedOpen<UnauthorizedAccessException>(shared, "denied", null, "unwritable container parent");
            Console.WriteLine("checks=" + checks + " failed=" + failed);
            return failed == 0 ? 0 : 1;
        }

        if (Directory.GetFileSystemEntries(personal).Length != 0)
        {
            Console.WriteLine("REFUSED non-empty fixture " + personal);
            return 2;
        }

        using (StorageContainer control = Open(shared, "control"))
            Check(!control.IsDisposed && control.DisplayName == "control", "control container opens");
        string saved = OnlyDirectory(personal);
        string savedPath = saved == null ? null : Path.Combine(personal, saved);
        string titlePath = savedPath == null ? null : Path.Combine(savedPath, title);
        string allPlayers = titlePath == null ? null : OnlyDirectory(Path.Combine(titlePath, "control"));
        Check(saved != null && Directory.Exists(titlePath) && allPlayers != null,
              "construction creates the whole device/saved/title/name/player chain");
        if (allPlayers == null)
        {
            Console.WriteLine("checks=" + checks + " failed=" + failed);
            return 1;
        }

        string leaf = Path.Combine(titlePath, "blocked");
        File.WriteAllText(leaf, "seed");
        BlockedOpen<IOException>(shared, "blocked", leaf, "file at the container name");
        File.Delete(leaf);
        using (Open(shared, "blocked")) { }
        Check(Directory.Exists(Path.Combine(leaf, allPlayers)), "a later open succeeds once the file is gone");

        StorageDevice one = Device(PlayerIndex.One);
        using (Open(one, "player-control")) { }
        string playerOne = OnlyDirectory(Path.Combine(titlePath, "player-control"));
        Check(playerOne != null && playerOne != allPlayers, "a player device uses its own player folder");
        if (playerOne != null)
        {
            string playerParent = Path.Combine(titlePath, "player-blocked");
            Directory.CreateDirectory(playerParent);
            string playerLeaf = Path.Combine(playerParent, playerOne);
            File.WriteAllText(playerLeaf, "seed");
            BlockedOpen<IOException>(one, "player-blocked", playerLeaf, "file at the player folder");
        }

        Directory.Delete(titlePath, true);
        File.WriteAllText(titlePath, "seed");
        BlockedOpen<IOException>(shared, "any", titlePath, "file at the title folder");
        File.Delete(titlePath);

        Directory.Delete(savedPath, true);
        File.WriteAllText(savedPath, "seed");
        BlockedOpen<IOException>(shared, "any", savedPath, "file at the saved-games folder");
        File.Delete(savedPath);

        Console.WriteLine("folders saved=" + saved + " title=" + title + " all=" + allPlayers
                          + " one=" + playerOne);
        Console.WriteLine("checks=" + checks + " failed=" + failed);
        return failed == 0 ? 0 : 1;
    }
}
