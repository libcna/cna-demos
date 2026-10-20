// ISSUE-030: execute genuine XNA's managed simulator with a scripted Random.
// This is not a network-session, transport, Xbox LIVE or hardware validation.
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Reflection;

static class NetworkSimulatorOracle
{
    sealed class ScriptedRandom : Random
    {
        public readonly Queue<double> Values = new Queue<double>();
        public int Draws;
        public override double NextDouble()
        {
            ++Draws;
            if (Values.Count == 0) throw new InvalidOperationException("Unexpected random draw");
            return Values.Dequeue();
        }
    }
    static int checks, failed;
    static void Check(bool value, string label)
    {
        ++checks;
        if (!value) ++failed;
        Console.WriteLine((value ? "PASS " : "FAIL ") + label);
    }
    static int Main(string[] args)
    {
        try
        {
            var assembly = Assembly.LoadFrom(args[0]);
            Console.WriteLine(assembly.FullName);
            Console.WriteLine(assembly.Location);
            const BindingFlags flags = BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance;
            var type = assembly.GetType("Microsoft.Xna.Framework.Net.NetworkSimulator", true);
            var options = assembly.GetType("Microsoft.Xna.Framework.Net.SendDataOptions", true);
            var simulator = Activator.CreateInstance(type, true);
            var random = new ScriptedRandom();
            type.GetField("random", flags).SetValue(simulator, random);
            var loss = type.GetProperty("SimulatedPacketLoss", flags);
            var latency = type.GetProperty("SimulatedLatency", flags);
            var send = type.GetMethod("SendData", flags);
            Check((float)loss.GetValue(simulator, null) == 0, "default loss is zero");
            Check((TimeSpan)latency.GetValue(simulator, null) == TimeSpan.Zero, "default latency is zero");
            loss.SetValue(simulator, 1.0f, null);
            for (int i = 0; i < 8; ++i)
            {
                if ((i & 1) == 0) random.Values.Enqueue(0.5);
                bool consumed = (bool)send.Invoke(simulator, new object[] { new byte[] { 37 }, 0, 1,
                    Enum.ToObject(options, i), null, null });
                Check(consumed == ((i & 1) == 0), "complete loss exempts Reliable flags=" + i);
            }
            Check(random.Draws == 4 && random.Values.Count == 0, "Reliable never draws loss randomness");
            loss.SetValue(simulator, 0.0f, null);
            for (int i = 0; i < 8; ++i)
                Check(!(bool)send.Invoke(simulator, new object[] { new byte[] { 37 }, 0, 1,
                    Enum.ToObject(options, i), null, null }), "zero loss immediately forwards flags=" + i);
            Check(random.Draws == 4, "zero loss consumes no random values");

            latency.SetValue(simulator, TimeSpan.FromMilliseconds(200), null);
            long mean = (long)type.GetField("simulatedLatencyTicks", flags).GetValue(simulator);
            Check(mean == TimeSpan.FromMilliseconds(200).Ticks * Stopwatch.Frequency / 10000000,
                "latency uses Stopwatch clock units");
            var sample = type.GetMethod("NormallyDistributedRandomLatency", flags);
            double[] draws = { Math.Exp(-2), 0.5, Math.Exp(-2), 0, 0.25, 0.25, 1e-300, 0.5, 1e-300, 0 };
            foreach (double value in draws) random.Values.Enqueue(value);
            long[] expected = { mean - 2 * (mean / 4), mean + 2 * (mean / 4), mean, 0, 2 * mean };
            for (int i = 0; i < expected.Length; ++i)
            {
                long actual = (long)sample.Invoke(simulator, null);
                Console.WriteLine("latency sample=" + actual + " mean=" + mean);
                // The floating Box-Muller result truncates to integral Stopwatch ticks.
                Check(Math.Abs(actual - expected[i]) <= 1, "scripted normal/clamp case=" + i);
            }
            Check(random.Draws == 14 && random.Values.Count == 0, "all scripted latency draws consumed exactly");
            Console.WriteLine("checks=" + checks + " failed=" + failed);
            return failed == 0 ? 0 : 1;
        }
        catch (Exception error) { Console.WriteLine("SETUP ERROR " + error); return 2; }
    }
}
