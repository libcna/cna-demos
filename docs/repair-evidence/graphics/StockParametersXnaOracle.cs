// Genuine Microsoft XNA 4.0 device/effect observation; never emulate reflection.
using System;
using System.Windows.Forms;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

class StockParametersXnaOracle
{
    static int checks, failed;
    static void Check(bool ok, string label)
    {
        checks++; if (!ok) failed++;
        Console.WriteLine((ok ? "PASS " : "FAIL ") + label);
    }
    static void Refuses(Action action, string label)
    {
        try { action(); Check(false, label); }
        catch (InvalidCastException) { Check(true, label); }
    }
    static void Shape(Effect effect)
    {
        string name = effect.GetType().Name;
        EffectParameter vector = effect.Parameters["DiffuseColor"];
        EffectParameter matrix = effect.Parameters["WorldViewProj"];
        Refuses(delegate { vector.GetValueSingle(); }, name + " vector Single");
        Refuses(delegate { vector.GetValueInt32(); }, name + " vector Int32");
        Refuses(delegate { vector.GetValueBoolean(); }, name + " vector Boolean");
        Refuses(delegate { matrix.GetValueSingle(); }, name + " matrix Single");
        Refuses(delegate { matrix.GetValueInt32(); }, name + " matrix Int32");
        Refuses(delegate { matrix.GetValueBoolean(); }, name + " matrix Boolean");
    }
    static void PrintMatrix(string label, Matrix m)
    {
        Console.WriteLine(label + "=" + m.M11 + "," + m.M12 + "," + m.M13 + "," + m.M14 +
            ";" + m.M21 + "," + m.M22 + "," + m.M23 + "," + m.M24 +
            ";" + m.M31 + "," + m.M32 + "," + m.M33 + "," + m.M34 +
            ";" + m.M41 + "," + m.M42 + "," + m.M43 + "," + m.M44);
    }
    [STAThread]
    static int Main()
    {
        Console.WriteLine("assembly=" + typeof(Effect).Assembly.FullName);
        Console.WriteLine("mvid=" + typeof(Effect).Module.ModuleVersionId);
        try
        {
            using (Form window = new Form())
            {
                PresentationParameters pp = new PresentationParameters();
                pp.DeviceWindowHandle = window.Handle;
                pp.IsFullScreen = false;
                pp.BackBufferWidth = pp.BackBufferHeight = 64;
                pp.PresentationInterval = PresentInterval.Immediate;
                using (GraphicsDevice device = new GraphicsDevice(GraphicsAdapter.DefaultAdapter,
                        GraphicsProfile.HiDef, pp))
                {
                    Console.WriteLine("device=" + device.Adapter.Description + " profile=" + device.GraphicsProfile);
                    using (BasicEffect e = new BasicEffect(device)) Shape(e);
                    using (AlphaTestEffect e = new AlphaTestEffect(device)) Shape(e);
                    using (DualTextureEffect e = new DualTextureEffect(device)) Shape(e);
                    using (EnvironmentMapEffect e = new EnvironmentMapEffect(device)) Shape(e);
                    using (SkinnedEffect e = new SkinnedEffect(device))
                    {
                        Shape(e);
                        EffectParameter bones = e.Parameters["Bones"];
                        Console.WriteLine("bones=" + bones.Elements.Count + " rows=" + bones.RowCount + " columns=" + bones.ColumnCount);
                        Check(bones.Elements.Count == 72, "72 bone element views");
                        Console.WriteLine("child rows=" + bones.Elements[0].RowCount + " columns=" + bones.Elements[0].ColumnCount);
                        PrintMatrix("raw-default", bones.GetValueMatrixArray(1)[0]);
                        Matrix transform = Matrix.CreateScale(2, 3, 4) * Matrix.CreateTranslation(5, 6, 7);
                        e.SetBoneTransforms(new Matrix[] { transform, Matrix.Identity });
                        PrintMatrix("raw-parent-write", bones.GetValueMatrixArray(1)[0]);
                        PrintMatrix("child-parent-write", bones.Elements[0].GetValueMatrix());
                        Console.WriteLine("raw-floats=" + string.Join(",", Array.ConvertAll(
                            bones.GetValueSingleArray(16), delegate(float value) { return value.ToString(); })));
                        PrintMatrix("raw-transpose", bones.Elements[0].GetValueMatrixTranspose());
                        Matrix asymmetric = new Matrix(1, 2, 3, 4, 5, 6, 7, 8,
                                                       9, 10, 11, 12, 13, 14, 15, 16);
                        bones.Elements[0].SetValueTranspose(asymmetric);
                        PrintMatrix("raw-transpose-write", bones.Elements[0].GetValueMatrix());
                        bones.SetValue(new float[] { 1, 2, 3, 4, 5, 6, 7, 8,
                                                     9, 10, 11, 12, 13, 14, 15, 16 });
                        PrintMatrix("flat-write-first", bones.GetValueMatrixArray(2)[0]);
                        PrintMatrix("flat-write-second", bones.GetValueMatrixArray(2)[1]);
                        e.SetBoneTransforms(new Matrix[] { transform, Matrix.Identity });
                        Check(e.GetBoneTransforms(72).Length == 72, "partial write retains array capacity");
                        Check(e.GetBoneTransforms(72)[71] == Matrix.Identity, "partial write retains untouched tail");
                        bones.Elements[0].SetValue(7.0f);
                        PrintMatrix("scalar-broadcast", bones.Elements[0].GetValueMatrix());
                        Check(e.GetBoneTransforms(2)[1] == Matrix.Identity, "scalar write preserves adjacent bone");
                        Refuses(delegate { bones.SetValue(1.0f); }, "array parent refuses scalar write");
                        foreach (float special in new float[] { float.NaN, float.PositiveInfinity, float.NegativeInfinity })
                        {
                            bones.Elements[0].SetValue(special);
                            float[] components = bones.Elements[0].GetValueSingleArray(12);
                            bool matches = components.Length == 12;
                            foreach (float component in components)
                                matches = matches && (float.IsNaN(special) ? float.IsNaN(component) : component == special);
                            Check(matches && bones.Elements[0].GetValueMatrix().M44 == 0,
                                  "nonfinite scalar broadcast " + (float.IsNaN(special) ? "NaN" :
                                      special > 0 ? "+Infinity" : "-Infinity"));
                        }
                        Matrix edited = Matrix.CreateTranslation(9, 8, 7);
                        bones.Elements[0].SetValue(edited);
                        Check(e.GetBoneTransforms(1)[0] == edited, "element write reaches parent");
                        using (SkinnedEffect clone = (SkinnedEffect)e.Clone())
                        {
                            clone.Parameters["Bones"].Elements[1].SetValue(transform);
                            Check(clone.GetBoneTransforms(2)[1] == transform, "clone element reaches clone parent");
                            Check(e.GetBoneTransforms(2)[1] == Matrix.Identity, "clone element does not mutate original");
                        }
                    }
                }
            }
        }
        catch (Exception e) { Console.WriteLine("UNAVAILABLE " + e); return 2; }
        Console.WriteLine("checks=" + checks + " failed=" + failed);
        return failed == 0 ? 0 : 1;
    }
}
