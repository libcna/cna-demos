// SPDX-License-Identifier: MS-PL
// ISSUE-101 dependency contract: URI classification/local paths, no file or network access.
using System;
using System.Text;

internal static class UriLocalPathOracle
{
    private static string Encode(string text)
    {
        return Convert.ToBase64String(Encoding.UTF8.GetBytes(text));
    }

    private static string Read(Func<string> action)
    {
        try { return "VALUE:" + Encode(action()); }
        catch (Exception error) { return "THROW:" + error.GetType().FullName; }
    }

    public static int Main()
    {
        Console.WriteLine("runtime=" + Environment.Version);
        Console.WriteLine("uri=" + typeof(Uri).Assembly.FullName);
        Console.WriteLine("mvid=" + typeof(Uri).Module.ModuleVersionId);
        string[] inputs = {
            "file:///tmp/movie.ogv",
            "FILE:///tmp/%63hroma%20clip.mkv",
            "file:///tmp/a+b%23c%3Fd%25e",
            "file:///tmp/%C4%8D%F0%9F%98%80",
            "file:///tmp/%FF/%C0%AF/%E2%82/%ED%A0%80",
            "file:///tmp/%00.mkv",
            "file:///tmp/a/../movie.ogv",
            "file:///tmp/a/%2E%2E/movie.ogv",
            "file:///tmp/a%2Fb%5Cc",
            "file:///tmp/a//b",
            "file://localhost/tmp/clip.mkv",
            "file://LOCALHOST/tmp/clip.mkv",
            "file://server/share/a%20b.mkv",
            "file:///C:/dir/a%20b.mkv",
            "file:///C|/dir/a.mkv",
            "file:/tmp/clip.mkv",
            "file:relative.mkv",
            "file:////server/share/clip.mkv",
            "file:///tmp/%252F/%zz/%",
            "file:///tmp/clip.mkv?query#fragment",
            "https://example.invalid/a%20b?query#fragment",
            "https://example.invalid/a/../b",
            "https://example.invalid/a%2Fb%5Cc",
            "mailto:user@example.invalid",
            "urn:clip:a%20b",
            "assets/video.mkv",
            "",
            "/tmp/clip.mkv",
            "C:\\dir\\clip.mkv",
            "file:///tmp/a\\b",
            "file://server/share/a/../../b",
            "file:///C:/a/../../b",
            "file:///tmp/%E2%82%AC%FF%41",
            "file:///tmp/%C4x%8D"
        };
        foreach (string input in inputs)
        {
            try
            {
                Uri uri = new Uri(input, UriKind.RelativeOrAbsolute);
                Console.WriteLine("CASE\t" + Encode(input) + "\t" + uri.IsAbsoluteUri +
                    "\t" + Read(delegate { return uri.IsFile.ToString(); }) +
                    "\t" + Read(delegate { return uri.LocalPath; }));
            }
            catch (Exception error)
            {
                Console.WriteLine("CASE\t" + Encode(input) + "\tCONSTRUCTOR:" +
                    error.GetType().FullName);
            }
        }
        return 0;
    }
}
