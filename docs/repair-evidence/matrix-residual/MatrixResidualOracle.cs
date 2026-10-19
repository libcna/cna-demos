// SPDX-License-Identifier: MS-PL
using System;
using Microsoft.Xna.Framework;
class MatrixResidualOracle
{
    static int checks,failures;
    static void Check(bool ok,string label){++checks;if(!ok){++failures;Console.WriteLine("FAIL "+label);}}
    static float Float(uint bits){return BitConverter.ToSingle(BitConverter.GetBytes(bits),0);}
    static uint Bits(float value){return BitConverter.ToUInt32(BitConverter.GetBytes(value),0);}
    static float[] Fields(Matrix m){return new[]{m.M11,m.M12,m.M13,m.M14,m.M21,m.M22,m.M23,m.M24,m.M31,m.M32,m.M33,m.M34,m.M41,m.M42,m.M43,m.M44};}
    static Matrix MatrixOf(float[] a){return new Matrix(a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7],a[8],a[9],a[10],a[11],a[12],a[13],a[14],a[15]);}
    static bool SameBits(Matrix a,Matrix b){float[] x=Fields(a),y=Fields(b);for(int i=0;i<16;++i)if(Bits(x[i])!=Bits(y[i]))return false;return true;}
    static string PlaneBits(Plane p){return Bits(p.Normal.X).ToString("X8")+","+Bits(p.Normal.Y).ToString("X8")+","+Bits(p.Normal.Z).ToString("X8")+","+Bits(p.D).ToString("X8");}
    static int Main()
    {
        Console.WriteLine(typeof(Matrix).Assembly.FullName);
        Matrix[] matrices={new Matrix(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16),
            new Matrix(0,Float(0x80000000),Float(0x7fc00001),Float(0xffc12345),float.PositiveInfinity,float.NegativeInfinity,7,8,9,10,11,12,13,14,15,16),Matrix.Identity,new Matrix()};
        foreach(Matrix original in matrices)
        {
            float[] fields=Fields(original),target=new float[16];for(int row=0;row<4;++row)for(int col=0;col<4;++col)target[row*4+col]=fields[col*4+row];
            Matrix expected=MatrixOf(target),input=original,result;
            Check(SameBits(Matrix.Transpose(input),expected),"transpose value bits");
            Matrix.Transpose(ref input,out result);Check(SameBits(result,expected),"transpose out bits");
            Check(SameBits(input,original),"transpose source unchanged");
            Matrix.Transpose(ref input,out input);Check(SameBits(input,expected),"transpose alias bits");
            Matrix.Transpose(ref input,out input);Check(SameBits(input,original),"transpose alias involution");
        }
        Matrix translation=Matrix.CreateTranslation(0,5,0);
        Plane plane=new Plane(Vector3.Up,-2),planeCopy=plane,expectedPlane=new Plane(Vector3.Up,-7),transformed;
        Check(Plane.Transform(plane,translation)==expectedPlane,"nonzero-offset plane translation");
        Plane.Transform(ref plane,ref translation,out transformed);Check(transformed==expectedPlane&&plane==planeCopy,"plane out and source preservation");
        Plane.Transform(ref plane,ref translation,out plane);Check(plane==expectedPlane,"plane alias translation");
        foreach(Vector3 normal in new[]{Vector3.UnitX,Vector3.UnitY,Vector3.UnitZ})
        foreach(float d in new[]{-2f,0f})foreach(Vector3 direction in new[]{new Vector3(-1,-1,0),new Vector3(2,3,4),Vector3.Zero})
        foreach(float scale in new[]{0.25f,2f,8f})
        {
            Plane unit=new Plane(normal,d),scaled=new Plane(normal*scale,d*scale),before=scaled;
            Vector3 light=direction;Matrix expected=Matrix.CreateShadow(light,unit),result;
            Check(SameBits(Matrix.CreateShadow(light,scaled),expected),"scaled shadow value bits");
            Matrix.CreateShadow(ref light,ref scaled,out result);Check(SameBits(result,expected),"scaled shadow out bits");
            Check(light==direction&&scaled==before,"shadow inputs unchanged");
        }
        Plane almostUnit=new Plane(0.8414709568023682f,0.5403022766113281f,0,-2),almostCopy=almostUnit,normalizedOut;
        Check(PlaneBits(Plane.Normalize(almostUnit))==PlaneBits(almostUnit),"near-unit value normalization shortcut");
        Plane.Normalize(ref almostUnit,out normalizedOut);Check(PlaneBits(normalizedOut)==PlaneBits(almostCopy),"near-unit out normalization shortcut");
        Plane.Normalize(ref almostUnit,out almostUnit);Check(PlaneBits(almostUnit)==PlaneBits(almostCopy),"near-unit alias normalization shortcut");
        almostUnit.Normalize();Check(PlaneBits(almostUnit)==PlaneBits(almostCopy),"near-unit member normalization shortcut");
        Vector3 nearLight=new Vector3(2,3,4);Matrix nearShadow=Matrix.CreateShadow(nearLight,almostUnit);
        Check(nearShadow.M41==4&&nearShadow.M42==6&&nearShadow.M43==8,"near-unit shadow keeps D unchanged");
        Matrix.CreateShadow(ref nearLight,ref almostUnit,out nearShadow);
        Check(nearShadow.M41==4&&nearShadow.M42==6&&nearShadow.M43==8,"near-unit shadow out keeps D unchanged");
        Matrix nearReflection=Matrix.CreateReflection(almostUnit);
        Check(nearReflection.M41==4*almostUnit.Normal.X&&nearReflection.M42==4*almostUnit.Normal.Y,"near-unit reflection keeps coefficients");
        foreach(Plane near in new[]{almostUnit,new Plane(0.3f,0.9539392f,0,-2),new Plane(0.6f,0.8f,0,-2),new Plane(0.1f,0.9949874f,0,-2),new Plane(1e20f,0,0,-2),new Plane(1e-30f,0,0,-2)})
        {
            Plane normalized=Plane.Normalize(near);
            Console.WriteLine("near-unit squared="+Bits(near.Normal.LengthSquared()).ToString("X8")+" before="+PlaneBits(near)+" normalized="+PlaneBits(normalized));
        }
        Console.WriteLine("checks="+checks+" failures="+failures);return failures==0?0:1;
    }
}
