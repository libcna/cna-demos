// Genuine XNA4 Game.Tick/ResetElapsedTime contract probe for ISSUE-019.
// Constructor-free clock-state injection isolates timing, without creating a window/device.
// Game.Run/platform initialization are separately checked by ResetElapsedTimeLiveOracle.cs.
using System;
using System.Diagnostics;
using System.Reflection;
using System.Runtime.Serialization;
using Microsoft.Xna.Framework;

static class ResetElapsedTimeOracle
{
    const BindingFlags Fields = BindingFlags.Instance | BindingFlags.NonPublic;
    static int checks, failures;
    sealed class Probe : Game
    {
        public int Updates;
        public long LastElapsed;
        public bool LastSlow;
        public Action<GameTime> OnUpdate;
        protected override void Update(GameTime time) {
            ++Updates; LastElapsed = time.ElapsedGameTime.Ticks; LastSlow = time.IsRunningSlowly;
            if (OnUpdate != null) OnUpdate(time);
            SuppressDraw();
        }
    }
    static void Set(object target, Type type, string name, object value) {
        type.GetField(name, Fields).SetValue(target, value);
    }
    static object Get(object target, Type type, string name) {
        return type.GetField(name, Fields).GetValue(target);
    }
    static void Check(bool value, string label) {
        ++checks; if (!value) ++failures;
        Console.WriteLine((value ? "PASS " : "FAIL ") + label);
    }
    static Probe Create(bool fixedStep, long accumulatedTicks) {
        var game = (Probe)FormatterServices.GetUninitializedObject(typeof(Probe));
        GC.SuppressFinalize(game);
        Type clockType = typeof(Game).GetField("clock", Fields).FieldType;
        object clock = Activator.CreateInstance(clockType, true);
        Set(game, typeof(Game), "clock", clock);
        Set(game, typeof(Game), "gameTime", new GameTime());
        Set(game, typeof(Game), "isActive", true);
        Set(game, typeof(Game), "maximumElapsedTime", TimeSpan.FromMilliseconds(500));
        Set(game, typeof(Game), "targetElapsedTime", TimeSpan.FromTicks(166667));
        Set(game, typeof(Game), "isFixedTimeStep", fixedStep);
        Set(game, typeof(Game), "accumulatedElapsedGameTime", TimeSpan.FromTicks(accumulatedTicks));
        Set(game, typeof(Game), "drawRunningSlowly", true);
        Set(game, typeof(Game), "updatesSinceRunningSlowly1", 0);
        Set(game, typeof(Game), "updatesSinceRunningSlowly2", 0);
        Set(clock, clockType, "lastRealTimeValid", true);
        Set(clock, clockType, "lastRealTime", Stopwatch.GetTimestamp() - Stopwatch.Frequency / 5);
        return game;
    }
    static int Main() {
        try { return Run(); }
        catch (Exception error) { Console.WriteLine(error); return 2; }
    }
    static int Run() {
        Console.WriteLine(typeof(Game).Assembly.FullName);
        Console.WriteLine(typeof(Game).Assembly.Location);
        Console.WriteLine("Reset IL=" + BitConverter.ToString(typeof(Game).GetMethod("ResetElapsedTime")
            .GetMethodBody().GetILAsByteArray()));
        Probe fixedGame = Create(true, 0);
        object clock = Get(fixedGame, typeof(Game), "clock");
        Console.WriteLine("Clock assembly=" + clock.GetType().Assembly.Location);
        object previous = Get(clock, clock.GetType(), "lastRealTime");
        fixedGame.ResetElapsedTime();
        Check((bool)Get(fixedGame, typeof(Game), "forceElapsedTimeToZero"), "fixed reset sets pending zero flag");
        Check(!(bool)Get(fixedGame, typeof(Game), "drawRunningSlowly"), "reset clears draw slow state");
        Check((int)Get(fixedGame, typeof(Game), "updatesSinceRunningSlowly1") == Int32.MaxValue &&
              (int)Get(fixedGame, typeof(Game), "updatesSinceRunningSlowly2") == Int32.MaxValue,
              "reset clears both lag histories");
        fixedGame.Tick();
        Check(fixedGame.Updates == 0, "first fixed Tick after reset makes no update with no accumulated debt");
        Check(!previous.Equals(Get(clock, clock.GetType(), "lastRealTime")), "reset advances past the injected stale frame-clock origin");
        Check(!(bool)Get(fixedGame, typeof(Game), "forceElapsedTimeToZero"), "zero-step Tick consumes reset flag");

        Probe debt = Create(true, 2 * 166667 + 1234);
        debt.ResetElapsedTime(); debt.Tick();
        Check(debt.Updates == 2, "reset preserves pre-existing whole-step debt");
        Check(((TimeSpan)Get(debt, typeof(Game), "accumulatedElapsedGameTime")).Ticks == 1234,
              "reset preserves fractional accumulated debt");
        Check(debt.LastElapsed == 166667 && !debt.LastSlow, "retained steps use ordinary elapsed time and reset slow history");

        Probe largeDebt = Create(true, 20 * 166667 + 1234);
        largeDebt.ResetElapsedTime(); largeDebt.Tick();
        Console.WriteLine("Large debt: updates=" + largeDebt.Updates + ", lastSlow=" + largeDebt.LastSlow +
            ", drawSlow=" + Get(largeDebt, typeof(Game), "drawRunningSlowly") +
            ", history1=" + Get(largeDebt, typeof(Game), "updatesSinceRunningSlowly1") +
            ", history2=" + Get(largeDebt, typeof(Game), "updatesSinceRunningSlowly2"));
        Check(largeDebt.Updates == 20 && !largeDebt.LastSlow,
              "large retained debt still uses reset update slow history");
        Check(!(bool)Get(largeDebt, typeof(Game), "drawRunningSlowly"),
              "large retained debt does not immediately restore slow draw history");

        Probe variable = Create(false, 0);
        variable.ResetElapsedTime(); variable.Tick();
        Check(variable.Updates == 1 && variable.LastElapsed == 0, "variable reset produces one zero-elapsed update");
        Check(!variable.LastSlow, "variable update is not slow after reset");

        Probe inside = Create(true, 0);
        bool beforeReset = false, afterReset = false, nextUpdate = true;
        inside.OnUpdate = delegate(GameTime time) {
            if (inside.Updates == 1) {
                beforeReset = time.IsRunningSlowly;
                inside.ResetElapsedTime();
                afterReset = time.IsRunningSlowly;
            } else if (inside.Updates == 2) nextUpdate = time.IsRunningSlowly;
        };
        inside.Tick();
        Check(beforeReset, "catch-up control starts with slow history");
        Check(afterReset, "reset does not mutate the current Update GameTime slow flag");
        Check(!nextUpdate, "next already-planned Update observes cleared slow history");
        Check(inside.Updates >= 10 && (bool)Get(inside, typeof(Game), "forceElapsedTimeToZero"),
              "in-Update reset preserves planned steps and remains pending until next Tick");
        Console.WriteLine("checks=" + checks + " failures=" + failures);
        return failures == 0 ? 0 : 1;
    }
}
