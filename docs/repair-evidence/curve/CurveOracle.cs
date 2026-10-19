// SPDX-License-Identifier: MS-PL
using System;
using Microsoft.Xna.Framework;

class CurveOracle
{
    static int checks, failures;
    static void Check(bool ok, string label)
    {
        ++checks;
        if (!ok) ++failures;
        Console.WriteLine((ok ? "PASS " : "FAIL ") + label);
    }
    static int Main()
    {
        Console.WriteLine(typeof(Curve).Assembly.FullName);
        float[,] bounds = { {-4f, -2f}, {10f, 14f}, {0f, .25f} };
        for (int i = 0; i < bounds.GetLength(0); ++i)
        {
            Curve step = new Curve();
            step.Keys.Add(new CurveKey(bounds[i, 0], 7f, 0f, 0f, CurveContinuity.Step));
            step.Keys.Add(new CurveKey(bounds[i, 1], 9f));
            Check(step.Evaluate(bounds[i, 0]) == 7f, "step start " + i);
            Check(step.Evaluate((bounds[i, 0] + bounds[i, 1]) * .5f) == 7f, "step midpoint " + i);
            Check(step.Evaluate(bounds[i, 1]) == 9f, "step end " + i);
        }
        Curve linear = new Curve();
        linear.Keys.Add(new CurveKey(0f, 0f, 0f, 2f));
        linear.Keys.Add(new CurveKey(1f, 1f, 0f, 3f));
        linear.PostLoop = CurveLoopType.Linear;
        Check(linear.Evaluate(2f) == 4f, "post-loop last tangent");
        Check(linear.Evaluate(1.25f) == 1.75f, "post-loop fractional distance");

        Curve duplicate = new Curve();
        duplicate.Keys.Add(new CurveKey(0f, 0f));
        duplicate.Keys.Add(new CurveKey(0f, 10f));
        duplicate.Keys.Add(new CurveKey(1f, 20f));
        duplicate.ComputeTangents(CurveTangent.Linear);
        Check(duplicate.Evaluate(0f) == 0f, "duplicate exact");
        Check(duplicate.Evaluate(float.Epsilon) == 10f, "duplicate next positive float");
        foreach (CurveContinuity continuity in new[] {CurveContinuity.Smooth, CurveContinuity.Step})
        {
            Curve tiny = new Curve();
            tiny.Keys.Add(new CurveKey(0f, 7f, 0f, 0f, continuity));
            tiny.Keys.Add(new CurveKey(1e-11f, 9f));
            Check(tiny.Evaluate(5e-12f) == 7f, "tiny midpoint " + continuity);
            Check(tiny.Evaluate(1e-11f) == 7f, "tiny endpoint " + continuity);
        }
        Curve ordinary = new Curve();
        ordinary.Keys.Add(new CurveKey(0f, 7f));
        ordinary.Keys.Add(new CurveKey(1e-9f, 9f));
        Check(ordinary.Evaluate(5e-10f) == 8f, "above tiny-span threshold");

        Curve nan = new Curve();
        Check(nan.Evaluate(float.NaN) == 0f, "empty NaN");
        nan.Keys.Add(new CurveKey(0f, 7f));
        Check(nan.Evaluate(float.NaN) == 7f, "single NaN");
        nan.Keys.Add(new CurveKey(1f, 9f));
        Check(float.IsNaN(nan.Evaluate(float.NaN)), "smooth NaN");
        nan.Keys[1].Continuity = CurveContinuity.Step;
        Check(nan.Evaluate(float.NaN) == 9f, "step NaN");
        CurveKey nanKey = new CurveKey(float.NaN, 0f);
        CurveKey realKey = new CurveKey(1f, 0f);
        Check(nanKey.CompareTo(realKey) == 1, "NaN versus real");
        Check(realKey.CompareTo(nanKey) == 1, "real versus NaN");
        Check(nanKey.CompareTo(nanKey) == 1, "NaN versus NaN");
        Check(new CurveKey(-0f, 0f).CompareTo(new CurveKey(0f, 1f)) == 0, "signed zero positions");

        // CNA's MachineEpsilonFloat is half the spacing after 1, not C++ numeric_limits::epsilon.
        const float machineEpsilon = 5.9604644775390625e-8f;
        CurveKeyCollection nearby = new CurveKeyCollection();
        nearby.Add(new CurveKey(0f, 0f));
        nearby.Add(new CurveKey(machineEpsilon * .5f, 1f));
        nearby[0] = new CurveKey(machineEpsilon * .75f, 2f);
        Check(nearby[0].Position == machineEpsilon * .5f && nearby[1].Position == machineEpsilon * .75f,
              "nearby replacement reinserted");
        CurveKeyCollection alias = new CurveKeyCollection();
        alias.Add(new CurveKey(0f, 0f));
        alias.Add(new CurveKey(1f, 10f));
        alias.Add(new CurveKey(2f, 20f));
        alias[0] = alias[1];
        Check(alias[0].Value == 10f && alias[1].Value == 10f && alias[2].Value == 20f,
              "replacement from another collection element");
        Console.WriteLine("checks=" + checks + " failures=" + failures);
        return failures == 0 ? 0 : 1;
    }
}
