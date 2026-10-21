// SPDX-License-Identifier: MS-PL
// Actual XNA disposal callbacks on constructor-free SpriteBatch objects.
// No device, shader, buffer, draw, or native GPU teardown is exercised.
using System;
using System.Runtime.Serialization;
using Microsoft.Xna.Framework.Graphics;

static class GraphicsResourceEventOrderOracle
{
    static int checks, failed;
    static void Check(bool condition, string label)
    {
        ++checks;
        if (!condition) ++failed;
        Console.WriteLine((condition ? "PASS " : "FAIL ") + label);
    }
    static SpriteBatch Resource()
    {
        SpriteBatch resource = (SpriteBatch)FormatterServices.GetUninitializedObject(typeof(SpriteBatch));
        GC.SuppressFinalize(resource);
        return resource;
    }
    static Exception Capture(Action action)
    {
        try { action(); return null; }
        catch (Exception error) { return error; }
    }
    static int Main()
    {
        Console.WriteLine("assembly=" + typeof(SpriteBatch).Assembly.FullName);
        Console.WriteLine("location=" + typeof(SpriteBatch).Assembly.Location);
        Console.WriteLine("mvid=" + typeof(SpriteBatch).Module.ModuleVersionId);
        Console.WriteLine("scope=constructor-free managed disposal callbacks; no GPU runtime");
        SpriteBatch ordinary = Resource();
        int ordinaryCalls = 0;
        bool sawDisposed = false, senderMatches = false;
        ordinary.Disposing += delegate(object sender, EventArgs args)
        {
            ++ordinaryCalls;
            sawDisposed = ordinary.IsDisposed;
            senderMatches = Object.ReferenceEquals(sender, ordinary);
        };
        ordinary.Dispose();
        Check(ordinaryCalls == 1 && sawDisposed && senderMatches,
              "Disposing sees published state and original sender exactly once");
        Check(ordinary.IsDisposed, "successful callback preserves disposed state");
        ordinary.Dispose();
        Check(ordinaryCalls == 1, "later Dispose does not repeat the normal event");

        SpriteBatch reentrant = Resource();
        int recursiveCalls = 0;
        reentrant.Disposing += delegate(object sender, EventArgs args)
        {
            ++recursiveCalls;
            if (recursiveCalls == 1) reentrant.Dispose();
        };
        Exception recursion = Capture(delegate { reentrant.Dispose(); });
        Check(recursion == null && reentrant.IsDisposed, "reentrant Dispose completes without exception");
        Check(recursiveCalls == 1, "reentrant Dispose does not raise a second event");

        SpriteBatch throwing = Resource();
        int throwingCalls = 0;
        Exception sentinel = new InvalidOperationException("original callback failure");
        throwing.Disposing += delegate(object sender, EventArgs args)
        {
            ++throwingCalls;
            throw sentinel;
        };
        Exception first = Capture(delegate { throwing.Dispose(); });
        Check(Object.ReferenceEquals(first, sentinel), "the original callback exception escapes unchanged");
        Check(throwing.IsDisposed, "a throwing callback does not undo disposal");
        Exception second = Capture(delegate { throwing.Dispose(); });
        Check(second == null && throwingCalls == 1, "later Dispose does not repeat a throwing callback");
        Console.WriteLine("checks=" + checks + " failed=" + failed);
        return failed == 0 ? 0 : 1;
    }
}
