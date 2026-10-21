// SPDX-License-Identifier: MS-PL
// Observes the real XNA decoder seam through its normal proxy thread.
// No fabricated decoder handle, GraphicsDevice, texture, or public Play call.
using System;
using System.IO;
using System.Reflection;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Media;

internal static class VideoCodecFailureOracle
{
    private static string Describe(Exception error)
    {
        while (error is TargetInvocationException && error.InnerException != null)
            error = error.InnerException;
        return error.GetType().FullName + ": " + error.Message.Replace('\r', ' ').Replace('\n', ' ');
    }

    public static int Main(string[] args)
    {
        if (args.Length != 2) throw new ArgumentException("Expected fixture directory and existing text file.");
        Assembly framework = typeof(Vector3).Assembly;
        Console.WriteLine("framework-mvid=" + framework.ManifestModule.ModuleVersionId);
        Console.WriteLine("video-mvid=" + typeof(VideoPlayer).Module.ModuleVersionId);
        const BindingFlags flags = BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic;
        Type native = framework.GetType("Microsoft.Xna.Framework.Media.UnsafeNativeMethods", true);
        Type helpers = framework.GetType("Microsoft.Xna.Framework.Helpers", true);
        MethodInfo play = native.GetMethod("VideoDecoder_Play", flags);
        MethodInfo map = helpers.GetMethod("GetExceptionFromResult", flags);
        FieldInfo handleField = typeof(VideoPlayer).GetField("decoderHandle", BindingFlags.Instance | BindingFlags.NonPublic);
        if (play == null || map == null || handleField == null)
            throw new InvalidOperationException("Pinned reference seam is absent.");

        string[] names = { "missing", "text", "empty", "truncated", "valid-silent" };
        string[] paths = {
            Path.Combine(args[0], "repair_reference_absent_101.wmv"), args[1],
            Path.Combine(args[0], "empty.wmv"), Path.Combine(args[0], "truncated.wmv"),
            Path.Combine(args[0], "wmv_64x48_15fps_silent.wmv") };
        for (int i = 0; i < paths.Length; ++i)
            if (File.Exists(paths[i]) != (i != 0))
                throw new InvalidOperationException("Unexpected fixture existence: " + names[i]);

        int observations = 0;
        bool positiveControl = false;
        for (int i = 0; i < paths.Length; ++i)
        {
            VideoPlayer player;
            try { player = new VideoPlayer(); }
            catch (Exception error)
            {
                Console.WriteLine("UNAVAILABLE constructor " + Describe(error));
                Console.WriteLine("observations=" + observations + " positive-control=false");
                return 77;
            }
            using (player)
            {
                uint handle = (uint)handleField.GetValue(player);
                if (handle == UInt32.MaxValue)
                    throw new InvalidOperationException("Normal constructor did not publish a decoder handle.");
                object result = play.Invoke(null, new object[] {
                    handle, paths[i], (uint)paths[i].Length, 64u, 48u, false });
                int code = unchecked((int)Convert.ToInt64(result));
                Exception mapped = code < 0
                    ? (Exception)map.Invoke(null, new object[] { unchecked((uint)code) }) : null;
                Console.WriteLine("OBSERVE " + names[i] + " HRESULT=0x" + unchecked((uint)code).ToString("X8") +
                    " exception=" + (mapped == null ? "none" : mapped.GetType().FullName));
                ++observations;
                if (i == 0 && (code != unchecked((int)0x80070057) || mapped == null ||
                    mapped.GetType() != typeof(ArgumentException)))
                    throw new InvalidOperationException("Missing-file control differs from verified XNA guard.");
                if (i == paths.Length - 1) positiveControl = code >= 0;
            }
        }
        Console.WriteLine("observations=" + observations + " positive-control=" + positiveControl.ToString().ToLowerInvariant());
        Console.WriteLine("scope=decoder-open return through normal proxy; no public Play, texture, pixel, asynchronous decode or playback claim");
        return positiveControl ? 0 : 77;
    }
}
