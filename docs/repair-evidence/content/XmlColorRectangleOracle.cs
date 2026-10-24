// Museum Pipeline Workshop item 9 oracle (2026-10-24). Compiled with mono's mcs against the
// genuine XNA 4.0 Microsoft.Xna.Framework.dll and run under .NET Framework 4.0 in the
// ~/.wine-cna-xna40 prefix:
//   mcs -sdk:4 -platform:x86 -target:exe -r:Microsoft.Xna.Framework.dll -out:Probe.exe XmlColorRectangleOracle.cs
//   WINEPREFIX=~/.wine-cna-xna40 wine Probe.exe
// Output, as measured:
//   Color=<Color><R>10</R><G>20</G><B>30</B><A>40</A><PackedValue>673059850</PackedValue></Color>
//   Rectangle=<Rectangle><X>1</X><Y>2</Y><Width>3</Width><Height>4</Height><Location><X>1</X><Y>2</Y></Location></Rectangle>
//   Point=<Point><X>5</X><Y>6</Y></Point>
//   Vector2=<Vector2><X>1.5</X><Y>2.5</Y></Vector2>
//   ColorPartial=7,8,0,0 packed=2055
//   ColorPackedLast=255,0,0,255
//   RectangleLocationLast=9,8,3,4
//   RectanglePartial=0,0,3,0
// CNA pins these in modules/math/tests/Microsoft/Xna/Framework/XmlSerializationEXTTests.cpp.
using System;
using System.IO;
using System.Text;
using System.Xml;
using System.Xml.Serialization;
using Microsoft.Xna.Framework;

static class Probe
{
    static string Write<T>(T value)
    {
        var serializer = new XmlSerializer(typeof(T));
        var settings = new XmlWriterSettings { Indent = false, OmitXmlDeclaration = true };
        var text = new StringBuilder();
        using (var writer = XmlWriter.Create(text, settings))
        {
            var names = new XmlSerializerNamespaces();
            names.Add("", "");
            serializer.Serialize(writer, value, names);
        }
        return text.ToString();
    }

    static T Read<T>(string xml)
    {
        return (T)new XmlSerializer(typeof(T)).Deserialize(new StringReader(xml));
    }

    static void Main()
    {
        Console.WriteLine("Color=" + Write(new Color(10, 20, 30, 40)));
        Console.WriteLine("Rectangle=" + Write(new Rectangle(1, 2, 3, 4)));
        Console.WriteLine("Point=" + Write(new Point(5, 6)));
        Console.WriteLine("Vector2=" + Write(new Vector2(1.5f, 2.5f)));

        Color partial = Read<Color>("<Color><R>7</R><G>8</G></Color>");
        Console.WriteLine("ColorPartial=" + partial.R + "," + partial.G + "," + partial.B + "," + partial.A +
                          " packed=" + partial.PackedValue);
        Color packedLast = Read<Color>("<Color><R>1</R><G>2</G><B>3</B><A>4</A><PackedValue>4278190335</PackedValue></Color>");
        Console.WriteLine("ColorPackedLast=" + packedLast.R + "," + packedLast.G + "," + packedLast.B + "," +
                          packedLast.A);
        Rectangle located = Read<Rectangle>(
            "<Rectangle><X>1</X><Y>2</Y><Width>3</Width><Height>4</Height><Location><X>9</X><Y>8</Y></Location></Rectangle>");
        Console.WriteLine("RectangleLocationLast=" + located.X + "," + located.Y + "," + located.Width + "," +
                          located.Height);
        Rectangle partialRect = Read<Rectangle>("<Rectangle><Width>3</Width></Rectangle>");
        Console.WriteLine("RectanglePartial=" + partialRect.X + "," + partialRect.Y + "," + partialRect.Width + "," +
                          partialRect.Height);
    }
}
