// Genuine Microsoft XNA 4.0 reference for the museum cx-capi input-state strings.
using System;
using System.Globalization;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Input;

internal static class InputStateStringsOracle
{
    private static int checks, failed;
    private static void Equal(string actual, string expected)
    {
        ++checks;
        bool ok = actual == expected;
        if (!ok) ++failed;
        Console.WriteLine((ok ? "PASS " : "FAIL ") + actual + " expected " + expected);
    }
    private static ButtonState Button(int mask, int bit)
    {
        return (mask & bit) != 0 ? ButtonState.Pressed : ButtonState.Released;
    }
    private static int Main()
    {
        Console.WriteLine(typeof(MouseState).Assembly.FullName);
        Console.WriteLine(typeof(MouseState).Assembly.Location);
        if (typeof(MouseState).Assembly.GetName().Version != new Version(4, 0, 0, 0))
            return 2;
        foreach (string culture in new string[] { "en-US", "cs-CZ" })
        {
            System.Threading.Thread.CurrentThread.CurrentCulture = new CultureInfo(culture);
            for (int mask = 0; mask < 32; ++mask)
            {
                var state = new MouseState(10, -20, 120, Button(mask, 1), Button(mask, 4),
                    Button(mask, 2), Button(mask, 8), Button(mask, 16));
                string buttons = "";
                string[] names = { "Left", "Right", "Middle", "XButton1", "XButton2" };
                for (int bit = 0; bit < 5; ++bit)
                    if ((mask & (1 << bit)) != 0)
                        buttons += (buttons.Length == 0 ? "" : " ") + names[bit];
                if (buttons.Length == 0) buttons = "None";
                Equal(state.ToString(), "{X:10 Y:-20 Buttons:" + buttons + " Wheel:120}");
            }
            Equal(new MouseState().ToString(), "{X:0 Y:0 Buttons:None Wheel:0}");
            Equal(new GamePadState().ToString(), "{IsConnected:False}");
            Equal(new GamePadState(Vector2.Zero, Vector2.Zero, 0, 0, Buttons.A).ToString(),
                "{IsConnected:True}");
        }
        Console.WriteLine("checks=" + checks + " failed=" + failed);
        return failed == 0 ? 0 : 1;
    }
}
