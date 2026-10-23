// Genuine XNA/D3D9 query counter boundary on a real GPU through Wine/DXVK.
// Complements, but does not replace, the exact managed DWORD-return seam.
using System;
using System.Diagnostics;
using System.Windows.Forms;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

internal static class OcclusionCountGpuXnaOracle
{
    private static readonly VertexPositionColor[] quad = {
        new VertexPositionColor(new Vector3(-1,-1,0.5f), Color.White),
        new VertexPositionColor(new Vector3(-1, 1,0.5f), Color.White),
        new VertexPositionColor(new Vector3( 1,-1,0.5f), Color.White),
        new VertexPositionColor(new Vector3( 1,-1,0.5f), Color.White),
        new VertexPositionColor(new Vector3(-1, 1,0.5f), Color.White),
        new VertexPositionColor(new Vector3( 1, 1,0.5f), Color.White)
    };

    [STAThread]
    private static int Main()
    {
        Console.WriteLine("assembly=" + typeof(OcclusionQuery).Assembly.FullName);
        Console.WriteLine("mvid=" + typeof(OcclusionQuery).Module.ModuleVersionId);
        using (Form window = new Form())
        {
            PresentationParameters pp = new PresentationParameters();
            pp.DeviceWindowHandle = window.Handle;
            pp.IsFullScreen = false;
            pp.BackBufferWidth = pp.BackBufferHeight = 64;
            pp.DepthStencilFormat = DepthFormat.None;
            pp.MultiSampleCount = 0;
            pp.PresentationInterval = PresentInterval.Immediate;
            using (GraphicsDevice device = new GraphicsDevice(GraphicsAdapter.DefaultAdapter, GraphicsProfile.HiDef, pp))
            using (BasicEffect effect = new BasicEffect(device))
            using (RenderTarget2D target = new RenderTarget2D(device, 1024, 1024, false, SurfaceFormat.Color, DepthFormat.None))
            using (BlendState noColor = new BlendState())
            {
                Console.WriteLine("device=" + device.Adapter.Description);
                noColor.ColorWriteChannels = ColorWriteChannels.None;
                device.BlendState = noColor;
                device.DepthStencilState = DepthStencilState.None;
                device.RasterizerState = RasterizerState.CullNone;
                effect.VertexColorEnabled = true;
                foreach (int draws in new int[] {1, 2048, 4096, 4097})
                using (OcclusionQuery query = new OcclusionQuery(device))
                {
                    Console.WriteLine("START draws=" + draws);
                    device.SetRenderTarget(target);
                    device.Clear(Color.Black);
                    effect.CurrentTechnique.Passes[0].Apply();
                    query.Begin();
                    for (int i = 0; i < draws; ++i)
                        device.DrawUserPrimitives(PrimitiveType.TriangleList, quad, 0, 2);
                    query.End();
                    device.SetRenderTarget(null);
                    device.Present();
                    Stopwatch timeout = Stopwatch.StartNew();
                    while (!query.IsComplete)
                    {
                        if (timeout.ElapsedMilliseconds > 60000) throw new Exception("Query completion deadline");
                        device.Present();
                    }
                    int expected = unchecked((int)((ulong)draws * 1048576UL));
                    Console.WriteLine("RESULT draws=" + draws + " count=" + query.PixelCount + " expected=" + expected);
                    if (query.PixelCount != expected) throw new Exception("GPU DWORD count boundary differs");
                }
            }
        }
        Console.WriteLine("OBSERVATION_COMPLETE: exact control, signed boundary, uint32 wrap and post-wrap");
        return 0;
    }
}
