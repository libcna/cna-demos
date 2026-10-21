// SPDX-License-Identifier: MS-PL
// ISSUE-099: managed validation/retention with an explicitly invalid decoder, never native playback.
using System;
using System.Reflection;
using System.Runtime.Serialization;
using Microsoft.Xna.Framework.Media;

static class VideoPlayerGuardsOracle
{
    static int checks, failures;
    static void Check(bool value, string label)
    {
        ++checks; if (!value) ++failures;
        Console.WriteLine((value ? "PASS " : "FAIL ") + label);
    }
    static void Field(VideoPlayer player, string name, object value)
    {
        typeof(VideoPlayer).GetField(name, BindingFlags.Instance | BindingFlags.NonPublic).SetValue(player, value);
    }
    static VideoPlayer Player(Video video)
    {
        VideoPlayer player = (VideoPlayer)FormatterServices.GetUninitializedObject(typeof(VideoPlayer));
        GC.SuppressFinalize(player);
        Field(player, "decoderHandleLock", new object());
        Field(player, "decoderHandle", uint.MaxValue);
        Field(player, "volume", 0.25f);
        Field(player, "looping", true);
        Field(player, "muted", true);
        Field(player, "activeVideo", video);
        FieldInfo textures = typeof(VideoPlayer).GetField("frameTextures", BindingFlags.Instance | BindingFlags.NonPublic);
        textures.SetValue(player, Array.CreateInstance(textures.FieldType.GetElementType(), 2));
        return player;
    }
    static void Refuse(Action action, Type expected, string label, string parameter = null)
    {
        Exception error = null;
        try { action(); } catch (Exception caught) { error = caught; }
        Check(error != null && error.GetType() == expected, label + ": exact exception");
        if (parameter != null)
        {
            ArgumentException argument = error as ArgumentException;
            Check(argument != null && argument.ParamName == parameter, label + ": parameter name");
        }
        if (expected == typeof(ObjectDisposedException))
        {
            ObjectDisposedException disposed = error as ObjectDisposedException;
            Check(disposed != null && disposed.ObjectName == "VideoPlayer", label + ": object name");
        }
    }
    static int Main()
    {
        Console.WriteLine("assembly=" + typeof(VideoPlayer).Assembly.FullName);
        Console.WriteLine("mvid=" + typeof(VideoPlayer).Module.ModuleVersionId);
        VideoPlayer fresh = Player(null);
        Refuse(delegate { fresh.GetTexture(); }, typeof(InvalidOperationException), "fresh texture");
        Refuse(delegate { fresh.Play(null); }, typeof(ArgumentNullException), "null video", "video");
        foreach (float value in new float[] { -0.1f, 1.1f })
        {
            Refuse(delegate { fresh.Volume = value; }, typeof(ArgumentOutOfRangeException), "invalid volume " + value, "value");
            Check(fresh.Volume == 0.25f, "invalid volume retains prior value " + value);
        }
        foreach (float value in new float[] { 0f, 1f })
        {
            fresh.Volume = value;
            Check(fresh.Volume == value, "volume boundary " + value);
        }
        fresh.Volume = float.NaN;
        Check(float.IsNaN(fresh.Volume), "managed volume permits NaN before native assignment");
        fresh.Dispose();

        Video token = (Video)FormatterServices.GetUninitializedObject(typeof(Video));
        GC.SuppressFinalize(token);
        VideoPlayer retained = Player(token);
        retained.Stop();
        Check(Object.ReferenceEquals(retained.Video, token), "Stop retains video identity");
        retained.Dispose();
        Check(retained.IsDisposed, "Dispose marks player");
        Check(Object.ReferenceEquals(retained.Video, token), "Dispose retains video identity");
        retained.Dispose();
        Check(retained.IsDisposed, "second Dispose remains safe");
        Check(retained.Volume == 0.25f, "disposed volume getter remains readable");
        Check(retained.IsLooped, "disposed loop getter remains readable");
        Check(retained.IsMuted, "disposed mute getter remains readable");
        Refuse(delegate { object value = retained.State; GC.KeepAlive(value); }, typeof(ObjectDisposedException), "disposed state");
        Refuse(delegate { object value = retained.PlayPosition; GC.KeepAlive(value); }, typeof(ObjectDisposedException), "disposed position");
        Refuse(delegate { retained.IsLooped = false; }, typeof(ObjectDisposedException), "disposed loop setter");
        Refuse(delegate { retained.IsMuted = false; }, typeof(ObjectDisposedException), "disposed mute setter");
        Refuse(delegate { retained.Volume = 0.25f; }, typeof(ObjectDisposedException), "disposed unchanged volume");
        Refuse(delegate { retained.Volume = -1f; }, typeof(ObjectDisposedException), "disposal precedes range");
        Refuse(delegate { retained.GetTexture(); }, typeof(ObjectDisposedException), "disposed texture");
        Refuse(delegate { retained.Play(null); }, typeof(ObjectDisposedException), "disposal precedes null video");
        Refuse(delegate { retained.Stop(); }, typeof(ObjectDisposedException), "disposed stop");
        Refuse(delegate { retained.Pause(); }, typeof(ObjectDisposedException), "disposed pause");
        Refuse(delegate { retained.Resume(); }, typeof(ObjectDisposedException), "disposed resume");
        Console.WriteLine("checks=" + checks + " failed=" + failures);
        return failures == 0 ? 0 : 1;
    }
}
