// Genuine XNA4 live-Game control for CNA-DEMOS-ISSUE-019.
// Run only through CNA's private display runner; deliberately models a 200ms loading frame.
using System;
using System.Threading;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

static class ResetElapsedTimeLiveOracle
{
    static int checks, failures;
    static void Check(bool value, string label) {
        ++checks; if (!value) ++failures;
        Console.WriteLine((value ? "PASS " : "FAIL ") + label);
    }
    sealed class Probe : Game
    {
        readonly bool reset;
        int updates, updatesThisFrame, draws;
        bool afterLoad;
        public bool Observed;
        public int AfterLoadUpdates;
        public long AfterLoadElapsed;
        public bool AfterLoadSlow;
        public Probe(bool useReset) {
            reset = useReset;
            var graphics = new GraphicsDeviceManager(this);
            graphics.PreferredBackBufferWidth = 128;
            graphics.PreferredBackBufferHeight = 128;
            graphics.SynchronizeWithVerticalRetrace = false;
        }
        protected override void Update(GameTime time) {
            ++updates; ++updatesThisFrame;
            if (afterLoad && !Observed) {
                AfterLoadElapsed = time.ElapsedGameTime.Ticks;
                AfterLoadSlow = time.IsRunningSlowly;
                Observed = true;
            }
            if (updates == 10) {
                Thread.Sleep(200);
                if (reset) ResetElapsedTime();
                afterLoad = true;
            }
            base.Update(time);
        }
        protected override void Draw(GameTime time) {
            if (Observed && AfterLoadUpdates == 0) AfterLoadUpdates = updatesThisFrame;
            updatesThisFrame = 0;
            GraphicsDevice.Clear(Color.Black);
            base.Draw(time);
            // Window paint frames may draw before the loading witness has run.
            // Bound the measured post-load phase rather than terminating that witness early.
            if (Observed && ++draws >= 25) Exit();
        }
    }
    static int Main(string[] args) {
        try {
            if (args.Length != 1 || (args[0] != "control" && args[0] != "reset")) return 2;
            Console.WriteLine(typeof(Game).Assembly.FullName);
            Console.WriteLine(typeof(Game).Assembly.Location);
            // Each scenario owns a fresh GameHost/message loop. A second Run in one process
            // returned before its witness in this Wine environment, independently of reset.
            if (args[0] == "control") using (var control = new Probe(false)) {
                control.Run();
                Console.WriteLine("no reset updates=" + control.AfterLoadUpdates);
                Check(control.Observed, "control reached the post-load update");
                Check(control.AfterLoadUpdates >= 3, "without reset the loading frame creates catch-up updates");
            }
            else using (var repaired = new Probe(true)) {
                repaired.Run();
                Console.WriteLine("reset updates=" + repaired.AfterLoadUpdates +
                    " elapsed=" + repaired.AfterLoadElapsed + " slow=" + repaired.AfterLoadSlow);
                Check(repaired.Observed, "reset run reached the post-load update");
                Check(repaired.Observed && repaired.AfterLoadUpdates >= 1 && repaired.AfterLoadUpdates <= 2,
                      "reset avoids a burst of catch-up updates");
                Check(repaired.AfterLoadElapsed == repaired.TargetElapsedTime.Ticks, "next update uses one ordinary step");
                Check(!repaired.AfterLoadSlow, "next update is not running slowly");
            }
            Console.WriteLine("checks=" + checks + " failures=" + failures);
            return failures == 0 ? 0 : 1;
        } catch (Exception error) { Console.WriteLine(error); return 2; }
    }
}
