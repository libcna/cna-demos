// SPDX-License-Identifier: MS-PL
// Genuine XNA public calls on a real device. Run each case in a fresh process.
using System;
using System.Reflection;
using System.Windows.Forms;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

static class SpriteBatchDisposedGpuObservation
{
    static void Observe(string name, Action action)
    {
        try { action(); Console.WriteLine("OBSERVE " + name + " accepted"); }
        catch (Exception error)
        {
            Console.WriteLine("OBSERVE " + name + " exception=" + error.GetType().FullName
                + " object=" + (error is ObjectDisposedException ? ((ObjectDisposedException)error).ObjectName : "")
                + " message=" + error.Message);
        }
    }
    static void State(string label, GraphicsDevice device, SpriteBatch batch)
    {
        const BindingFlags flags = BindingFlags.Instance | BindingFlags.NonPublic;
        Console.WriteLine("STATE " + label + " disposed=" + batch.IsDisposed
            + " pair=" + typeof(SpriteBatch).GetField("inBeginEndPair", flags).GetValue(batch)
            + " queued=" + typeof(SpriteBatch).GetField("spriteQueueCount", flags).GetValue(batch)
            + " active=" + typeof(GraphicsDevice).GetField("spriteBeginCount", flags).GetValue(device)
            + " immediate=" + typeof(GraphicsDevice).GetField("spriteImmediateBeginCount", flags).GetValue(device));
    }
    [STAThread]
    static int Main(string[] args)
    {
        if (args.Length != 1) return 2;
        Console.WriteLine("assembly=" + typeof(SpriteBatch).Assembly.FullName);
        Console.WriteLine("mvid=" + typeof(SpriteBatch).Module.ModuleVersionId);
        using (Form window = new Form())
        {
            var pp = new PresentationParameters();
            pp.DeviceWindowHandle = window.Handle;
            pp.BackBufferWidth = pp.BackBufferHeight = 64;
            pp.IsFullScreen = false;
            pp.DepthStencilFormat = DepthFormat.None;
            pp.PresentationInterval = PresentInterval.Immediate;
            using (var device = new GraphicsDevice(GraphicsAdapter.DefaultAdapter, GraphicsProfile.HiDef, pp))
            using (var texture = new Texture2D(device, 1, 1))
            using (var batch = new SpriteBatch(device))
            {
                texture.SetData(new Color[] { Color.White });
                string mode = args[0];
                if (mode == "open-empty" || mode == "open-queued") batch.Begin();
                if (mode == "open-queued") batch.Draw(texture, Vector2.Zero, Color.White);
                if (mode == "immediate-end" || mode == "immediate-draw")
                    batch.Begin(SpriteSortMode.Immediate, null);
                batch.Dispose();
                State("disposed", device, batch);
                if (mode == "closed")
                {
                    Observe("End without pair", delegate { batch.End(); });
                    Observe("Draw without pair", delegate { batch.Draw(texture, Vector2.Zero, Color.White); });
                    Observe("null Draw without pair", delegate { batch.Draw(null, Vector2.Zero, Color.White); });
                    Observe("deferred Begin", delegate { batch.Begin(); });
                    Observe("deferred Draw", delegate { batch.Draw(texture, Vector2.Zero, Color.White); });
                }
                else if (mode == "closed-immediate")
                    Observe("immediate Begin", delegate { batch.Begin(SpriteSortMode.Immediate, null); });
                else if (mode == "immediate-draw")
                    Observe("immediate Draw", delegate { batch.Draw(texture, Vector2.Zero, Color.White); });
                else if (mode != "open-empty" && mode != "open-queued" && mode != "immediate-end")
                    return 2;
                if (mode != "closed-immediate")
                {
                    Observe("End", delegate { batch.End(); });
                    State("after End", device, batch);
                    Observe("repeated Begin", delegate { batch.Begin(); });
                    State("after Begin", device, batch);
                }
                else State("after immediate Begin", device, batch);
                batch.Dispose();
                State("repeated Dispose", device, batch);
            }
        }
        Console.WriteLine("OBSERVATION_COMPLETE");
        return 0;
    }
}
