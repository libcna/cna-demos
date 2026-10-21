// SPDX-License-Identifier: MS-PL
// ISSUE-038: execute genuine managed disposal guards without creating a native voice.
using System;
using System.Reflection;
using System.Runtime.Serialization;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Audio;

static class DynamicAudioDisposedOracle
{
    static int checks, failures;
    static void Check(bool value, string label)
    {
        ++checks; if (!value) ++failures;
        Console.WriteLine((value ? "PASS " : "FAIL ") + label);
    }
    static void Disposed(Action action, string label)
    {
        Exception result = null;
        try { action(); }
        catch (Exception error) { result = error; }
        Check(result != null && result.GetType() == typeof(ObjectDisposedException),
              label + ": exact ObjectDisposedException");
        ObjectDisposedException disposed = result as ObjectDisposedException;
        Check(disposed != null && disposed.ObjectName == "DynamicSoundEffectInstance",
              label + ": dynamic object name");
    }
    static int Main()
    {
        Console.WriteLine("assembly=" + typeof(Curve).Assembly.FullName);
        Console.WriteLine("mvid=" + typeof(Curve).Module.ModuleVersionId);
        DynamicSoundEffectInstance instance = (DynamicSoundEffectInstance)
            FormatterServices.GetUninitializedObject(typeof(DynamicSoundEffectInstance));
        GC.SuppressFinalize(instance);
        // Only fields read before the guarded managed refusal are populated. No live native
        // voice, constructor, disposal implementation or hardware behaviour is being validated.
        Type parent = typeof(SoundEffectInstance);
        parent.GetField("voiceHandleLock", BindingFlags.Instance | BindingFlags.NonPublic)
              .SetValue(instance, new object());
        Check(!instance.IsLooped, "non-disposed managed getter returns false");
        parent.GetField("disposed", BindingFlags.Instance | BindingFlags.NonPublic).SetValue(instance, true);
        Check(instance.IsDisposed, "reference fixture is explicitly disposed");
        Disposed(delegate { GC.KeepAlive(instance.PendingBufferCount); }, "PendingBufferCount");
        Disposed(delegate { GC.KeepAlive(instance.IsLooped); }, "IsLooped");
        Disposed(delegate { GC.KeepAlive(((SoundEffectInstance)instance).IsLooped); }, "base IsLooped");
        Disposed(delegate { instance.GetSampleDuration(0); }, "GetSampleDuration zero");
        Disposed(delegate { instance.GetSampleDuration(4000); }, "GetSampleDuration positive");
        Disposed(delegate { instance.GetSampleDuration(-1); }, "GetSampleDuration validation order");
        Disposed(delegate { instance.GetSampleSizeInBytes(TimeSpan.Zero); }, "GetSampleSizeInBytes zero");
        Disposed(delegate { instance.GetSampleSizeInBytes(TimeSpan.FromSeconds(1)); }, "GetSampleSizeInBytes positive");
        Disposed(delegate { instance.GetSampleSizeInBytes(TimeSpan.FromSeconds(-1)); }, "GetSampleSizeInBytes validation order");
        Disposed(delegate { instance.IsLooped = false; }, "IsLooped false setter");
        Disposed(delegate { instance.IsLooped = true; }, "IsLooped true setter");
        Console.WriteLine("checks=" + checks + " failed=" + failures);
        return failures == 0 ? 0 : 1;
    }
}
