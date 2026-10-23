// Genuine XNA managed/native-result boundary, not a GPU overflow experiment.
using System;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.Serialization;
using Microsoft.Xna.Framework.Graphics;

internal static unsafe class OcclusionCountXnaOracle
{
    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    private delegate int GetDataDelegate(IntPtr self, IntPtr data, uint size, uint flags);
    private static uint sample;
    private static int calls;
    private static bool pending;
    private static IntPtr expectedSelf;
    private static readonly GetDataDelegate getData = GetData;

    private static int GetData(IntPtr self, IntPtr data, uint size, uint flags)
    {
        if (self != expectedSelf || size != 4 || flags != 1)
            return unchecked((int)0x80004005);
        ++calls;
        Marshal.WriteInt32(data, unchecked((int)sample));
        return pending ? 1 : 0;
    }

    private static void Require(bool condition, string message)
    {
        if (!condition) throw new Exception(message);
    }

    private static int Main()
    {
        Require(IntPtr.Size == 4, "Requires genuine x86 mixed-mode XNA");
        Console.WriteLine("assembly=" + typeof(OcclusionQuery).Assembly.FullName);
        Console.WriteLine("mvid=" + typeof(OcclusionQuery).Module.ModuleVersionId);
        FieldInfo native = typeof(OcclusionQuery).GetField("pComPtr", BindingFlags.Instance | BindingFlags.NonPublic);
        FieldInfo begun = typeof(OcclusionQuery).GetField("_hasCalledBegin", BindingFlags.Instance | BindingFlags.NonPublic);
        Require(native != null && begun != null, "Reference layout changed");
        IntPtr table = Marshal.AllocHGlobal(8 * IntPtr.Size);
        IntPtr instance = Marshal.AllocHGlobal(IntPtr.Size);
        OcclusionQuery query = null;
        try
        {
            for (int i = 0; i < 8; ++i) Marshal.WriteIntPtr(table, i * IntPtr.Size, IntPtr.Zero);
            Marshal.WriteIntPtr(table, 7 * IntPtr.Size, Marshal.GetFunctionPointerForDelegate(getData));
            Marshal.WriteIntPtr(instance, table);
            expectedSelf = instance;
            query = (OcclusionQuery)FormatterServices.GetUninitializedObject(typeof(OcclusionQuery));
            GC.SuppressFinalize(query);
            native.SetValue(query, Pointer.Box(instance.ToPointer(), native.FieldType));
            begun.SetValue(query, true);
            foreach (uint value in new uint[] { 0, 1, 0x7fffffff, 0x80000000, 0xffffffff })
            {
                sample = value;
                Require(query.IsComplete, "Native S_OK must complete");
                int result = query.PixelCount;
                Console.WriteLine("native=0x" + value.ToString("x8") + " PixelCount=" + result);
                Require(result == unchecked((int)value), "XNA DWORD to Int32 conversion mismatch");
            }
            pending = true;
            Require(!query.IsComplete, "Native S_FALSE must remain incomplete");
            bool refused = false;
            try { int unused = query.PixelCount; GC.KeepAlive(unused); }
            catch (InvalidOperationException) { refused = true; }
            Require(refused, "Incomplete PixelCount must refuse");
            Require(calls == 12, "Unexpected native GetData dispatch count");
            Console.WriteLine("OBSERVATION_COMPLETE: five DWORD boundaries and two pending controls; calls=" + calls);
            return 0;
        }
        finally
        {
            if (query != null) native.SetValue(query, Pointer.Box(null, native.FieldType));
            GC.KeepAlive(getData);
            GC.KeepAlive(query);
            Marshal.FreeHGlobal(instance);
            Marshal.FreeHGlobal(table);
        }
    }
}
