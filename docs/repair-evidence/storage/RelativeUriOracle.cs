// ISSUE-145 dependency diagnostic: genuine .NET Framework 4 relative URI validation.
using System;
using System.Text;

static class RelativeUriOracle
{
    static int Main()
    {
        string[] inputs = { "", " ", "\t\r\n", ".", "./", "seed.bin", "bad\0.bin", "bad\n.bin",
                            "a#b", "a%b", new string('a', 65519), new string('a', 65520) };
        foreach (string input in inputs)
        {
            foreach (UriKind kind in new[] { UriKind.Relative, UriKind.Absolute, UriKind.RelativeOrAbsolute })
            {
                string label = input.Length < 32 ? Convert.ToBase64String(Encoding.UTF8.GetBytes(input)) : "length=" + input.Length;
                try {
                    Uri uri = new Uri(input, kind);
                    Console.WriteLine(label + "\t" + kind + "\tOK\t" + uri.IsAbsoluteUri + "\t" +
                        (input.Length < 32 ? Convert.ToBase64String(Encoding.UTF8.GetBytes(uri.OriginalString)) : uri.OriginalString.Length.ToString()));
                }
                catch (Exception error) { Console.WriteLine(label + "\t" + kind + "\t" + error.GetType().Name); }
            }
        }
        return 0;
    }
}
