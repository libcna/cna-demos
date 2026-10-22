// Observe genuine Microsoft XNA 4.0 distinct-query overlap; do not emulate the API.
using System;
using System.Diagnostics;
using System.Windows.Forms;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

class OcclusionOverlapXnaOracle
{
    static GraphicsDevice device;
    static BasicEffect effect;
    static readonly VertexPositionColor[] triangle = {
        new VertexPositionColor(new Vector3(-0.5f, -0.5f, 0.5f), Color.White),
        new VertexPositionColor(new Vector3(0, 0.5f, 0.5f), Color.White),
        new VertexPositionColor(new Vector3(0.5f, -0.5f, 0.5f), Color.White)
    };
    static void Draw(int times)
    {
        effect.CurrentTechnique.Passes[0].Apply();
        for (int i = 0; i < times; ++i)
            device.DrawUserPrimitives(PrimitiveType.TriangleList, triangle, 0, 1);
    }
    static int Result(OcclusionQuery query)
    {
        device.Present();
        Stopwatch deadline = Stopwatch.StartNew();
        while (!query.IsComplete)
        {
            if (deadline.ElapsedMilliseconds > 5000) throw new Exception("Query completion timeout");
            device.Present();
        }
        return query.PixelCount;
    }
    static void Observe(string name, Action action)
    {
        try { action(); Console.WriteLine("OBSERVE " + name + " accepted"); }
        catch (Exception error) { Console.WriteLine("OBSERVE " + name + " exception=" + error.GetType().FullName); }
    }
    static void Overlap(string mode)
    {
        using (OcclusionQuery owner = new OcclusionQuery(device))
        using (OcclusionQuery contender = new OcclusionQuery(device))
        {
            device.Clear(Color.Black);
            owner.Begin(); Draw(1);
            bool accepted = false;
            Observe(mode + " distinct-Begin", delegate { contender.Begin(); accepted = true; });
            if (!accepted) { owner.End(); Console.WriteLine("RESULT " + mode + " owner=" + Result(owner)); return; }
            Draw(2);
            if (mode == "nested")
            {
                contender.End(); Draw(4); owner.End();
                Console.WriteLine("RESULT nested owner=" + Result(owner) + " contender=" + Result(contender));
            }
            else if (mode == "crossed")
            {
                owner.End(); Draw(4); contender.End();
                Console.WriteLine("RESULT crossed owner=" + Result(owner) + " contender=" + Result(contender));
            }
            else if (mode == "dispose-inner")
            {
                contender.Dispose(); Draw(4); owner.End();
                Console.WriteLine("RESULT dispose-inner owner=" + Result(owner));
            }
            else
            {
                owner.Dispose(); Draw(4); contender.End();
                Console.WriteLine("RESULT dispose-outer contender=" + Result(contender));
            }
        }
        using (OcclusionQuery reused = new OcclusionQuery(device))
        {
            reused.Begin(); Draw(1); reused.End();
            Console.WriteLine("RESULT " + mode + " fresh=" + Result(reused));
            reused.Begin(); Draw(2); reused.End();
            Console.WriteLine("RESULT " + mode + " reused=" + Result(reused));
        }
    }
    [STAThread]
    static int Main()
    {
        Console.WriteLine("assembly=" + typeof(OcclusionQuery).Assembly.FullName);
        Console.WriteLine("mvid=" + typeof(OcclusionQuery).Module.ModuleVersionId);
        try
        {
            using (Form window = new Form())
            {
                PresentationParameters pp = new PresentationParameters();
                pp.DeviceWindowHandle = window.Handle;
                pp.IsFullScreen = false;
                pp.BackBufferWidth = pp.BackBufferHeight = 64;
                pp.DepthStencilFormat = DepthFormat.None;
                pp.MultiSampleCount = 0;
                pp.PresentationInterval = PresentInterval.Immediate;
                using (device = new GraphicsDevice(GraphicsAdapter.DefaultAdapter, GraphicsProfile.HiDef, pp))
                using (effect = new BasicEffect(device))
                {
                    Console.WriteLine("device=" + device.Adapter.Description);
                    device.RasterizerState = RasterizerState.CullNone;
                    device.DepthStencilState = DepthStencilState.None;
                    device.BlendState = BlendState.Opaque;
                    effect.VertexColorEnabled = true;
                    using (OcclusionQuery control = new OcclusionQuery(device))
                    {
                        Observe("End-before-Begin", delegate { control.End(); });
                        control.Begin();
                        Observe("same-object-Begin", delegate { control.Begin(); });
                        Draw(1); control.End();
                        Observe("reuse-before-IsComplete", delegate { control.Begin(); });
                        Console.WriteLine("RESULT control=" + Result(control));
                    }
                    foreach (string mode in new string[] { "nested", "crossed", "dispose-inner", "dispose-outer" })
                        Overlap(mode);
                }
            }
            Console.WriteLine("OBSERVATION_COMPLETE"); return 0;
        }
        catch (Exception error) { Console.WriteLine("FATAL " + error); return 1; }
    }
}
