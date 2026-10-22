// Genuine XNA public Begin/End calls. No container is successfully opened and no
// application storage directory is created: invalid names stop before filesystem work.
using System;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Storage;

static class StorageDeviceAsyncOracle
{
    static int checks, failed;
    static void Check(bool condition, string label)
    {
        ++checks;
        Console.WriteLine((condition ? "PASS " : "FAIL ") + label);
        if (!condition) ++failed;
    }
    static void Throws<T>(Action action, string label, string parameter) where T : Exception
    {
        Exception caught = null;
        try { action(); } catch (Exception e) { caught = e; }
        bool ok = caught != null && caught.GetType() == typeof(T);
        if (parameter != null)
            ok = ok && caught is ArgumentException && ((ArgumentException)caught).ParamName == parameter;
        Check(ok, label + " actual=" + (caught == null ? "none" : caught.GetType().Name)
            + (caught is ArgumentException ? " param=" + ((ArgumentException)caught).ParamName : ""));
    }
    static StorageDevice Device()
    {
        IAsyncResult result = StorageDevice.BeginShowSelector(null, null);
        try { return StorageDevice.EndShowSelector(result); }
        finally { result.AsyncWaitHandle.Close(); }
    }
    static int Main()
    {
        Console.WriteLine(typeof(StorageDevice).Assembly.FullName);
        Console.WriteLine(typeof(StorageDevice).Assembly.Location);
        foreach (int player in new int[] { 0, 1, 2, 3, 255 })
        {
            object state = new object();
            IAsyncResult observed = null;
            IAsyncResult result = StorageDevice.BeginShowSelector((PlayerIndex)player, 0, -1,
                r => { observed = r; Check(r.IsCompleted && r.CompletedSynchronously &&
                    object.ReferenceEquals(r.AsyncState, state), "selector callback " + player); }, state);
            Check(object.ReferenceEquals(result, observed) && result.AsyncWaitHandle.WaitOne(0),
                  "synchronous signalled selector " + player);
            Check(StorageDevice.EndShowSelector(result) != null, "selector accepts player " + player);
            Throws<InvalidOperationException>(() => StorageDevice.EndShowSelector(result),
                "selector cannot end twice " + player, null);
            result.AsyncWaitHandle.Close();
        }
        foreach (int player in new int[] { -1, 4, 254, 256 })
            Throws<ArgumentOutOfRangeException>(() => StorageDevice.BeginShowSelector((PlayerIndex)player,
                -1, -1, null, null), "invalid player precedes invalid size " + player, "player");
        bool callback = false;
        Throws<ArgumentOutOfRangeException>(() => StorageDevice.BeginShowSelector(-1, 0,
            r => callback = true, null), "negative size without player", "sizeInBytes");
        Throws<ArgumentOutOfRangeException>(() => StorageDevice.BeginShowSelector(PlayerIndex.One, -1, 0,
            r => callback = true, null), "negative size with player", "sizeInBytes");
        Check(!callback, "invalid selector arguments do not invoke callback");
        Throws<ArgumentNullException>(() => StorageDevice.EndShowSelector(null), "null selector result", "result");

        StorageDevice owner = Device(), other = Device();
        Throws<ArgumentNullException>(() => owner.EndOpenContainer(null), "null container result", "result");
        foreach (string name in new string[] { null, "" })
        {
            object state = new object();
            IAsyncResult observed = null;
            IAsyncResult result = owner.BeginOpenContainer(name, r => observed = r, state);
            Check(object.ReferenceEquals(observed, result) && object.ReferenceEquals(result.AsyncState, state)
                && result.IsCompleted && result.CompletedSynchronously && result.AsyncWaitHandle.WaitOne(0),
                "invalid display name is deferred until End (" + (name == null ? "null" : "empty") + ")");
            Throws<ArgumentNullException>(() => owner.EndOpenContainer(result), "invalid display name at End",
                "You must pass in a non-null title name.");
            Throws<InvalidOperationException>(() => owner.EndOpenContainer(result), "failed End consumes container result", null);
            result.AsyncWaitHandle.Close();
        }
        IAsyncResult mismatch = owner.BeginOpenContainer("", null, null);
        Throws<ArgumentException>(() => other.EndOpenContainer(mismatch), "wrong owner precedes invalid display name",
            "The IAsyncResult parameter to this \"End\" function must be obtained by calling the matching \"Begin.\"");
        Throws<InvalidOperationException>(() => owner.EndOpenContainer(mismatch), "wrong-owner End still consumes result", null);
        mismatch.AsyncWaitHandle.Close();

        IAsyncResult selector = StorageDevice.BeginShowSelector(null, null);
        Throws<ArgumentNullException>(() => owner.EndOpenContainer(selector), "selector passed to container End", "result");
        Check(StorageDevice.EndShowSelector(selector) != null, "wrong End type does not consume selector");
        selector.AsyncWaitHandle.Close();
        IAsyncResult container = owner.BeginOpenContainer("", null, null);
        Throws<ArgumentNullException>(() => StorageDevice.EndShowSelector(container), "container passed to selector End", "result");
        Throws<ArgumentNullException>(() => owner.EndOpenContainer(container), "wrong End type does not consume container", null);
        Throws<InvalidOperationException>(() => owner.EndOpenContainer(container), "matching failed End consumes container", null);
        container.AsyncWaitHandle.Close();

        IAsyncResult callbackResult = StorageDevice.BeginShowSelector(r =>
            Check(StorageDevice.EndShowSelector(r) != null, "selector may End inside callback"), null);
        Throws<InvalidOperationException>(() => StorageDevice.EndShowSelector(callbackResult),
            "End inside callback consumes selector", null);
        callbackResult.AsyncWaitHandle.Close();
        IAsyncResult callbackContainer = owner.BeginOpenContainer("", r =>
            Throws<ArgumentNullException>(() => owner.EndOpenContainer(r), "container may End inside callback", null), null);
        Throws<InvalidOperationException>(() => owner.EndOpenContainer(callbackContainer),
            "failed End inside callback consumes container", null);
        callbackContainer.AsyncWaitHandle.Close();
        Console.WriteLine("checks=" + checks + " failed=" + failed);
        return failed == 0 ? 0 : 1;
    }
}
