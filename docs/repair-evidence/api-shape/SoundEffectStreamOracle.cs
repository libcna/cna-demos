// SPDX-License-Identifier: MS-PL
// ISSUE-029: genuine managed WAV/stream contract before native audio creation.
using System;
using System.IO;
using System.Reflection;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Audio;

static class SoundEffectStreamOracle
{
    static int checks, failures;
    static readonly Type Wav = typeof(Curve).Assembly.GetType(
        "Microsoft.Xna.Framework.Audio.WavFile", true);
    static readonly MethodInfo Open = Wav.GetMethod("Open",
        BindingFlags.Static | BindingFlags.NonPublic);

    sealed class TrackingStream : MemoryStream
    {
        public bool Closed;
        public int MaximumRead = Int32.MaxValue;
        public bool RefuseLength;
        public bool Readable = true;
        public TrackingStream(byte[] data) : base(data) { }
        public override bool CanRead { get { return Readable && base.CanRead; } }
        public override long Length
        {
            get
            {
                if (RefuseLength) throw new NotSupportedException("length witness");
                return base.Length;
            }
        }
        public override int Read(byte[] buffer, int offset, int count)
        {
            return base.Read(buffer, offset, Math.Min(count, MaximumRead));
        }
        protected override void Dispose(bool disposing)
        {
            Closed = true;
            base.Dispose(disposing);
        }
    }

    static void Check(bool condition, string label)
    {
        ++checks;
        if (!condition) ++failures;
        Console.WriteLine((condition ? "PASS " : "FAIL ") + label);
    }
    static Exception Attempt(Action action)
    {
        try { action(); return null; }
        catch (TargetInvocationException error) { return error.InnerException; }
        catch (Exception error) { return error; }
    }
    static bool Exact(Exception error, Type type)
    {
        return error != null && error.GetType() == type;
    }
    static byte[] Wave()
    {
        using (MemoryStream bytes = new MemoryStream())
        {
            BinaryWriter writer = new BinaryWriter(bytes);
            writer.Write(new byte[] { 82, 73, 70, 70 }); writer.Write(52);
            writer.Write(new byte[] { 87, 65, 86, 69, 102, 109, 116, 32 });
            writer.Write(16); writer.Write((ushort)1); writer.Write((ushort)1);
            writer.Write(8000); writer.Write(16000); writer.Write((ushort)2);
            writer.Write((ushort)16); writer.Write(new byte[] { 100, 97, 116, 97 });
            writer.Write(16); writer.Write(new byte[16]); writer.Flush();
            return bytes.ToArray();
        }
    }
    static object Read(TrackingStream stream)
    {
        return Open.Invoke(null, new object[] { stream });
    }
    static object Property(object value, string name)
    {
        return Wav.GetProperty(name, BindingFlags.Instance | BindingFlags.NonPublic)
                  .GetValue(value, null);
    }
    static int Main()
    {
        Console.WriteLine("assembly=" + typeof(Curve).Assembly.FullName);
        Console.WriteLine("mvid=" + typeof(Curve).Module.ModuleVersionId);
        Exception error = Attempt(delegate { SoundEffect.FromStream(null); });
        Check(Exact(error, typeof(ArgumentNullException)) &&
              ((ArgumentNullException)error).ParamName == "stream", "public null stream parameter");

        using (TrackingStream stream = new TrackingStream(new byte[0]))
        {
            error = Attempt(delegate { SoundEffect.FromStream(stream); });
            Check(Exact(error, typeof(ArgumentNullException)) &&
                  ((ArgumentNullException)error).ParamName == "source", "public empty stream parameter");
            Check(!stream.Closed, "empty stream remains open on failure");
        }
        foreach (int maximum in new[] { Int32.MaxValue, 1, 3 })
        {
            using (TrackingStream stream = new TrackingStream(Wave()))
            {
                stream.MaximumRead = maximum;
                object wav = null;
                error = Attempt(delegate { wav = Read(stream); });
                Check(error == null && wav != null, "managed WAV parses maximum read " + maximum);
                Check(stream.Closed, "successful parser closes source, maximum read " + maximum);
                Check(wav != null && ((byte[])Property(wav, "Data")).Length == 16,
                      "all short-read sample bytes preserved, maximum read " + maximum);
                Check(wav != null && (int)Property(wav, "LoopStart") == 0 &&
                      (int)Property(wav, "LoopLength") == 8,
                      "default loop covers eight frames, maximum read " + maximum);
            }
        }
        using (TrackingStream stream = new TrackingStream(Wave()))
        {
            stream.RefuseLength = true;
            error = Attempt(delegate { Read(stream); });
            Check(Exact(error, typeof(NotSupportedException)), "length failure propagates before parsing");
            Check(!stream.Closed, "length failure does not close source");
        }
        using (TrackingStream stream = new TrackingStream(Wave()))
        {
            stream.Readable = false;
            error = Attempt(delegate { Read(stream); });
            Check(Exact(error, typeof(ArgumentException)), "unreadable source is refused by BinaryReader");
            Check(!stream.Closed, "unreadable source remains open on failure");
        }
        byte[] invalidHeader = Wave(); invalidHeader[0] = 0;
        using (TrackingStream stream = new TrackingStream(invalidHeader))
        {
            error = Attempt(delegate { SoundEffect.FromStream(stream); });
            Check(Exact(error, typeof(InvalidOperationException)), "public bad RIFF header exact exception");
            Check(!stream.Closed, "bad RIFF header does not close source");
        }
        using (TrackingStream stream = new TrackingStream(new byte[] { 82, 73 }))
        {
            error = Attempt(delegate { SoundEffect.FromStream(stream); });
            Check(Exact(error, typeof(EndOfStreamException)), "public truncated header exact exception");
            Check(!stream.Closed, "truncated header does not close source");
        }
        byte[] invalidFormat = Wave(); invalidFormat[20] = 3;
        using (TrackingStream stream = new TrackingStream(invalidFormat))
        {
            error = Attempt(delegate { SoundEffect.FromStream(stream); });
            Check(Exact(error, typeof(ArgumentException)), "public non-PCM format exact exception");
            Check(!stream.Closed, "unsupported format does not close source");
        }
        Console.WriteLine("checks=" + checks + " failed=" + failures);
        return failures == 0 ? 0 : 1;
    }
}
