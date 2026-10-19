// SPDX-License-Identifier: MS-PL
// Run against genuine Microsoft XNA 4.0, not a substitute framework.
using System;
using System.Globalization;
using Microsoft.Xna.Framework;

class MatrixInvertOracle
{
    static string Bits(float value)
    {
        return BitConverter.ToUInt32(BitConverter.GetBytes(value), 0)
            .ToString("X8", CultureInfo.InvariantCulture);
    }

    static void Dump(string label, Matrix value)
    {
        float[] values = {value.M11,value.M12,value.M13,value.M14,value.M21,value.M22,value.M23,value.M24,
                          value.M31,value.M32,value.M33,value.M34,value.M41,value.M42,value.M43,value.M44};
        Console.Write(label + " bits:");
        foreach (float v in values) Console.Write(" " + Bits(v));
        Console.WriteLine();
        Console.WriteLine("{0} M22PositiveInfinity={1} M42NegativeInfinity={2} M22Equals8={3} M42EqualsMinus8={4}",
            label, float.IsPositiveInfinity(value.M22), float.IsNegativeInfinity(value.M42),
            value.M22 == 8.0f, value.M42 == -8.0f);
        Console.WriteLine("{0} original-format: M22={1:R} M42={2:R}", label, value.M22, value.M42);
    }

    static int Main()
    {
        Console.WriteLine("assembly=" + typeof(Matrix).Assembly.FullName);
        Console.WriteLine("culture=" + CultureInfo.CurrentCulture.Name + " outputCodePage=" + Console.OutputEncoding.CodePage);
        Console.Write("positive-infinity-symbol UTF16:");
        foreach (char c in CultureInfo.CurrentCulture.NumberFormat.PositiveInfinitySymbol)
            Console.Write(" " + ((int)c).ToString("X4", CultureInfo.InvariantCulture));
        Console.WriteLine();
        Matrix input = Matrix.CreateScale(1,0,1) * Matrix.CreateTranslation(7,8,9);
        Matrix byReference;
        Matrix.Invert(ref input, out byReference);
        Dump("value", Matrix.Invert(input));
        Dump("out", byReference);
        return 0;
    }
}
