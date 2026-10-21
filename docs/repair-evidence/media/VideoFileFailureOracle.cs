// SPDX-License-Identifier: MS-PL
// ISSUE-101 reference seam: genuine XNA's pre-decoder file refusal and HRESULT mapping.
// Never submits an existing file or allocates a decoder, texture, player or GPU device.
using System;
using System.IO;
using System.Reflection;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Media;

internal static class VideoFileFailureOracle
{
    private static int checks, failures;
    private static void Check(bool value, string label)
    {
        ++checks;
        if (!value) ++failures;
        Console.WriteLine((value ? "PASS " : "FAIL ") + label);
    }

    private static Exception InvokeException(MethodInfo method, object[] arguments)
    {
        try { method.Invoke(null, arguments); return null; }
        catch (TargetInvocationException wrapped) { return wrapped.InnerException; }
    }

    public static int Main()
    {
        Assembly framework = typeof(Vector3).Assembly;
        Console.WriteLine("framework=" + framework.FullName);
        Console.WriteLine("framework-mvid=" + framework.ManifestModule.ModuleVersionId);
        Console.WriteLine("video-mvid=" + typeof(VideoPlayer).Module.ModuleVersionId);
        const BindingFlags flags = BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static;
        Type wmp = framework.GetType("Microsoft.Xna.Framework.Media.WmpInterface", true);
        Type helpers = framework.GetType("Microsoft.Xna.Framework.Helpers", true);
        MethodInfo play = wmp.GetMethod("VideoDecoder_Play", flags);
        MethodInfo map = helpers.GetMethod("GetExceptionFromResult", flags);
        MethodInfo throwError = helpers.GetMethod("ThrowExceptionFromErrorCode", flags,
            null, new Type[] { typeof(int) }, null);
        if (play == null || map == null || throwError == null)
            throw new InvalidOperationException("Pinned reference seam is absent.");

        string absent = Path.Combine("build-probe", "repair_video_reference_absent_101.wmv");
        string[] paths = { null, "", absent, Path.GetFullPath(absent),
            Directory.GetCurrentDirectory(), "invalid\0name.wmv" };
        for (int i = 0; i < paths.Length; ++i)
        {
            string path = paths[i];
            bool exists = File.Exists(path);
            Check(!exists, "case" + i + " cannot reach native decoder lookup");
            if (exists) throw new InvalidOperationException("Refusing to pass an existing file to an invalid decoder.");
            int code = Convert.ToInt32(play.Invoke(null, new object[] {
                UInt32.MaxValue, path, path == null ? 0u : (uint)path.Length, 160u, 90u, false }));
            Console.WriteLine("case" + i + " HRESULT=0x" + unchecked((uint)code).ToString("X8"));
            Check(code == unchecked((int)0x80070057), "case" + i + " E_INVALIDARG");
            Exception mapped = (Exception)map.Invoke(null, new object[] { unchecked((uint)code) });
            Check(mapped != null && mapped.GetType() == typeof(ArgumentException),
                "case" + i + " maps exactly to ArgumentException");
            ArgumentException argument = mapped as ArgumentException;
            Check(argument != null && argument.ParamName == null,
                "case" + i + " has the default null parameter name");
            Exception thrown = InvokeException(throwError, new object[] { code });
            Check(thrown != null && thrown.GetType() == typeof(ArgumentException),
                "case" + i + " public Play error translator throws exact ArgumentException");
        }
        Check(map.Invoke(null, new object[] { 0u }) == null, "success maps to no exception");
        Check(InvokeException(throwError, new object[] { 0 }) == null, "success does not throw");
        Console.WriteLine("checks=" + checks + " failed=" + failures);
        Console.WriteLine("scope=pre-decoder file refusal and HRESULT translation only; no native playback or codec failure validation");
        return failures == 0 ? 0 : 1;
    }
}
