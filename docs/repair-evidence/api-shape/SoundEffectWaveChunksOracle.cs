// SPDX-License-Identifier: MS-PL
// ISSUE-029: genuine managed WAV chunk/loop rules; no native audio creation.
using System;
using System.IO;
using System.Reflection;
using Microsoft.Xna.Framework;

static class SoundEffectWaveChunksOracle
{
    static int checks, failures;
    static readonly Type Wav = typeof(Curve).Assembly.GetType("Microsoft.Xna.Framework.Audio.WavFile", true);
    static readonly MethodInfo Open = Wav.GetMethod("Open", BindingFlags.Static | BindingFlags.NonPublic);
    sealed class Tracked : MemoryStream
    {
        public bool Closed;
        public Tracked(byte[] bytes) : base(bytes) { }
        protected override void Dispose(bool disposing) { Closed = true; base.Dispose(disposing); }
    }
    static void Check(bool value, string label)
    {
        ++checks; if (!value) ++failures;
        Console.WriteLine((value ? "PASS " : "FAIL ") + label);
    }
    static byte[] Bytes(Action<BinaryWriter> write)
    {
        using (MemoryStream stream = new MemoryStream())
        {
            BinaryWriter writer = new BinaryWriter(stream);
            write(writer); writer.Flush(); return stream.ToArray();
        }
    }
    static byte[] Chunk(string tag, byte[] data)
    {
        return Bytes(delegate(BinaryWriter writer) {
            writer.Write(System.Text.Encoding.ASCII.GetBytes(tag));
            writer.Write(data.Length); writer.Write(data);
            if ((data.Length & 1) != 0) writer.Write((byte)0);
        });
    }
    static byte[] Format(int channels, int bits, int rate, int align)
    {
        return Chunk("fmt ", Bytes(delegate(BinaryWriter writer) {
            writer.Write((ushort)1); writer.Write((ushort)channels); writer.Write(rate);
            writer.Write(rate * align); writer.Write((ushort)align); writer.Write((ushort)bits);
        }));
    }
    static byte[] Wave(params byte[][] chunks)
    {
        int size = 4; foreach (byte[] chunk in chunks) size += chunk.Length;
        return Bytes(delegate(BinaryWriter writer) {
            writer.Write(System.Text.Encoding.ASCII.GetBytes("RIFF")); writer.Write(size);
            writer.Write(System.Text.Encoding.ASCII.GetBytes("WAVE"));
            foreach (byte[] chunk in chunks) writer.Write(chunk);
        });
    }
    static byte[] Smpl(int type, int start, int end)
    {
        return Chunk("smpl", Bytes(delegate(BinaryWriter writer) {
            for (int i = 0; i < 7; ++i) writer.Write(0);
            writer.Write(1); writer.Write(0); writer.Write(0); writer.Write(type);
            writer.Write(start); writer.Write(end); writer.Write(0); writer.Write(0);
        }));
    }
    static byte[] Wsmp(int start, int length)
    {
        return Chunk("wsmp", Bytes(delegate(BinaryWriter writer) {
            for (int i = 0; i < 4; ++i) writer.Write(0);
            writer.Write(1); writer.Write(16); writer.Write(0); writer.Write(start); writer.Write(length);
        }));
    }
    static object Property(object value, string name)
    {
        return Wav.GetProperty(name, BindingFlags.Instance | BindingFlags.NonPublic).GetValue(value, null);
    }
    static void Accepted(string label, byte[] bytes, int dataBytes, int loopStart, int loopLength, long ticks)
    {
        using (Tracked stream = new Tracked(bytes))
        {
            object wav = null; Exception error = null;
            try { wav = Open.Invoke(null, new object[] { stream }); }
            catch (TargetInvocationException caught) { error = caught.InnerException; }
            Check(error == null && wav != null, label + ": accepted");
            Check(stream.Closed, label + ": closes source");
            Check(wav != null && ((byte[])Property(wav, "Data")).Length == dataBytes,
                  label + ": exact complete-frame payload");
            Check(wav != null && (int)Property(wav, "LoopStart") == loopStart &&
                  (int)Property(wav, "LoopLength") == loopLength, label + ": exact loop");
            Check(wav != null && ((TimeSpan)Property(wav, "Duration")).Ticks == ticks,
                  label + ": exact rounded duration");
            if (error != null) Console.WriteLine("exception=" + error.GetType().FullName);
        }
    }
    static void Rejected(string label, byte[] bytes, Type expected)
    {
        using (Tracked stream = new Tracked(bytes))
        {
            Exception error = null;
            try { Open.Invoke(null, new object[] { stream }); }
            catch (TargetInvocationException caught) { error = caught.InnerException; }
            Check(error != null && error.GetType() == expected, label + ": exact exception");
            Check(!stream.Closed, label + ": leaves source open");
            if (error != null && error.GetType() != expected)
                Console.WriteLine("exception=" + error.GetType().FullName);
        }
    }
    static int Main()
    {
        Console.WriteLine("assembly=" + typeof(Curve).Assembly.FullName);
        Console.WriteLine("mvid=" + typeof(Curve).Module.ModuleVersionId);
        byte[] pcm16 = Format(1,16,8000,2), data16 = Chunk("data", new byte[16]);
        Accepted("PCM8 mono", Wave(Format(1,8,8000,1),data16),16,0,16,20000);
        Accepted("PCM8 stereo", Wave(Format(2,8,8000,2),data16),16,0,8,10000);
        Accepted("PCM16 stereo half millisecond", Wave(Format(2,16,8000,4),data16),16,0,4,10000);
        Accepted("48000 Hz rounded duration", Wave(Format(1,16,48000,2),data16),16,0,8,0);
        Accepted("odd unknown chunk", Wave(Chunk("JUNK",new byte[3]),pcm16,data16),16,0,8,10000);
        Accepted("partial final frame", Wave(pcm16,Chunk("data",new byte[17])),16,0,8,10000);
        Accepted("SMPL inclusive end", Wave(pcm16,data16,Smpl(0,2,5)),16,2,4,10000);
        Accepted("SMPL unsupported type", Wave(pcm16,data16,Smpl(1,2,5)),16,0,8,10000);
        Accepted("WSMP preferred over SMPL", Wave(pcm16,data16,Smpl(0,2,5),Wsmp(1,3)),16,1,3,10000);
        Accepted("invalid WSMP prevents SMPL fallback", Wave(pcm16,data16,Smpl(0,2,5),Wsmp(20,3)),16,0,8,10000);
        Accepted("ignored incomplete trailing chunk", Wave(pcm16,data16,new byte[]{74,85,78,75}),16,0,8,10000);
        Rejected("data precedes format",Wave(data16,pcm16),typeof(ArgumentException));
        Rejected("three channels",Wave(Format(3,16,8000,6),data16),typeof(ArgumentException));
        Rejected("rate below range",Wave(Format(1,16,7999,2),data16),typeof(ArgumentException));
        Rejected("rate above range",Wave(Format(1,16,48001,2),data16),typeof(ArgumentException));
        Rejected("PCM24",Wave(Format(1,24,8000,3),data16),typeof(ArgumentException));
        Rejected("wrong block alignment",Wave(Format(1,16,8000,1),data16),typeof(ArgumentException));
        Rejected("truncated SMPL metadata",Wave(pcm16,data16,Chunk("smpl",new byte[20])),typeof(EndOfStreamException));
        byte[] badSize = Wave(pcm16,data16); badSize[4] = 0;
        Rejected("RIFF length mismatch",badSize,typeof(InvalidOperationException));
        Console.WriteLine("checks=" + checks + " failed=" + failures);
        return failures == 0 ? 0 : 1;
    }
}
