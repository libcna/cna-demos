// SPDX-License-Identifier: MS-PL
// ISSUE-073: corrected genuine refusal order, preserving the initial failed oracle separately.
using System;
using System.Reflection;
using System.Runtime.Serialization;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Audio;

static class SpatialPanRefusalOracle
{
    static int checks, failures;
    static void Check(bool value, string label)
    {
        ++checks; if (!value) ++failures;
        Console.WriteLine((value ? "PASS " : "FAIL ") + label);
    }
    static void Field(SoundEffectInstance value, string name, object content)
    {
        typeof(SoundEffectInstance).GetField(name, BindingFlags.Instance | BindingFlags.NonPublic)
                                   .SetValue(value, content);
    }
    static void Refusal(float requested, bool disposed, Type expected, string label, bool spatial = true, bool submitted = true)
    {
        SoundEffectInstance instance = (SoundEffectInstance)
            FormatterServices.GetUninitializedObject(typeof(SoundEffectInstance));
        GC.SuppressFinalize(instance);
        Field(instance,"voiceHandleLock",new object());
        Field(instance,"currentPan",-0.25f);
        Field(instance,"isPacketSubmitted",submitted);
        Field(instance,"is3d",spatial);
        Field(instance,"disposed",disposed);
        Exception error = null;
        try { instance.Pan = requested; }
        catch (Exception caught) { error = caught; }
        Check(error != null && error.GetType() == expected,label + ": exact exception");
        Check(instance.Pan == -0.25f,label + ": prior pan retained");
        bool latch = (bool)typeof(SoundEffectInstance).GetField("is3d",
            BindingFlags.Instance | BindingFlags.NonPublic).GetValue(instance);
        Check(latch == (spatial && (disposed || submitted)),label + ": 3D latch follows refusal order");
        if (expected == typeof(ArgumentOutOfRangeException))
        {
            ArgumentOutOfRangeException range = error as ArgumentOutOfRangeException;
            Check(range != null && range.ParamName == "value",label + ": parameter name");
        }
        if (expected == typeof(ObjectDisposedException))
        {
            ObjectDisposedException lifetime = error as ObjectDisposedException;
            Check(lifetime != null && lifetime.ObjectName == "SoundEffectInstance",label + ": object name");
        }
    }
    static int Main()
    {
        Console.WriteLine("assembly=" + typeof(Curve).Assembly.FullName);
        Console.WriteLine("mvid=" + typeof(Curve).Module.ModuleVersionId);
        Refusal(0.5f,false,typeof(InvalidOperationException),"live3D valid pan");
        Refusal(2f,false,typeof(InvalidOperationException),"live3D invalid range");
        Refusal(float.NaN,false,typeof(InvalidOperationException),"live3D NaN");
        Refusal(0.5f,true,typeof(ObjectDisposedException),"disposed valid pan");
        Refusal(2f,true,typeof(ObjectDisposedException),"disposed validation order");
        Refusal(2f,false,typeof(ArgumentOutOfRangeException),"non3D invalid range",false);
        Refusal(float.NaN,false,typeof(ArgumentOutOfRangeException),"non3D NaN",false);
        Refusal(2f,false,typeof(ArgumentOutOfRangeException),"prepacket invalid range",true,false);
        Refusal(float.NaN,false,typeof(ArgumentOutOfRangeException),"prepacket NaN",true,false);
        Console.WriteLine("checks=" + checks + " failed=" + failures);
        return failures == 0 ? 0 : 1;
    }
}

