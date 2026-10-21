// SPDX-License-Identifier: MS-PL
// ISSUE-082: actual managed XNA methods with an explicitly constructor-free state seam.
// No GPU object, buffer, shader, native draw, or platform disposal is exercised.
using System;
using System.Reflection;
using System.Runtime.Serialization;
using Microsoft.Xna.Framework.Graphics;

static class SpriteBatchDisposeAccountingOracle
{
    static int checks, failed;
    static void Check(bool condition, string label)
    {
        ++checks;
        if (!condition) ++failed;
        Console.WriteLine((condition ? "PASS " : "FAIL ") + label);
    }
    static FieldInfo Field(Type type, string name)
    {
        FieldInfo field = type.GetField(name, BindingFlags.Instance | BindingFlags.NonPublic);
        if (field == null) throw new InvalidOperationException("Missing reference field: " + name);
        return field;
    }
    static T Uninitialized<T>()
    {
        T value = (T)FormatterServices.GetUninitializedObject(typeof(T));
        GC.SuppressFinalize(value);
        return value;
    }
    static SpriteBatch Batch(GraphicsDevice device)
    {
        SpriteBatch value = Uninitialized<SpriteBatch>();
        Field(typeof(GraphicsResource), "_parent").SetValue(value, device);
        return value;
    }
    static int Count(GraphicsDevice device, string name)
    {
        return Convert.ToInt32(Field(typeof(GraphicsDevice), name).GetValue(device));
    }
    static bool InPair(SpriteBatch batch)
    {
        return (bool)Field(typeof(SpriteBatch), "inBeginEndPair").GetValue(batch);
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
        Console.WriteLine("scope=constructor-free managed accounting and early refusals; no GPU runtime");
        GraphicsDevice device = Uninitialized<GraphicsDevice>();
        SpriteBatch first = Batch(device);
        SpriteBatch second = Batch(device);
        Check(!first.IsDisposed && !InPair(first) && Count(device, "spriteBeginCount") == 0,
              "fresh managed state is not disposed or active");
        first.Begin(); // Genuine public deferred Begin: no native state setup in this branch.
        Check(InPair(first) && Count(device, "spriteBeginCount") == 1 &&
              Count(device, "spriteImmediateBeginCount") == 0,
              "public deferred Begin establishes exactly one active batch");
        int events = 0;
        bool senderMatches = false, disposedDuringEvent = false;
        first.Disposing += delegate(object sender, EventArgs args)
        {
            ++events;
            senderMatches = Object.ReferenceEquals(sender, first);
            disposedDuringEvent = first.IsDisposed;
        };
        first.Dispose(); // Null effect/buffers, so only the actual managed disposal path runs.
        Check(first.IsDisposed, "Dispose publishes IsDisposed");
        Check(events == 1 && senderMatches, "Dispose raises one event with original sender");
        Check(disposedDuringEvent, "genuine XNA publishes IsDisposed before its event");
        Check(Count(device, "spriteBeginCount") == 1 && Count(device, "spriteImmediateBeginCount") == 0,
              "disposing an open deferred batch retains device accounting");
        Check(InPair(first), "disposing an open batch retains its Begin End pair");
        first.Dispose();
        Check(events == 1 && Count(device, "spriteBeginCount") == 1,
              "repeated Dispose neither raises an event nor changes accounting");
        Exception immediate = Capture(delegate { second.Begin(SpriteSortMode.Immediate, null); });
        Check(immediate != null && immediate.GetType() == typeof(InvalidOperationException),
              "another Immediate Begin is refused with exact InvalidOperationException");
        Check(!InPair(second) && Count(device, "spriteBeginCount") == 1 &&
              Count(device, "spriteImmediateBeginCount") == 0,
              "refused Immediate Begin does not publish an active pair");
        Exception repeated = Capture(delegate { first.Begin(); });
        Check(repeated != null && repeated.GetType() == typeof(InvalidOperationException),
              "disposed open batch Begin encounters the existing pair refusal");
        second.Begin();
        Check(InPair(second) && Count(device, "spriteBeginCount") == 2,
              "another deferred Begin remains allowed beside the retained first pair");
        second.Dispose();
        Check(second.IsDisposed && Count(device, "spriteBeginCount") == 2,
              "disposing the second deferred batch also retains accounting");
        Console.WriteLine("checks=" + checks + " failed=" + failed);
        return failed == 0 ? 0 : 1;
    }
}
