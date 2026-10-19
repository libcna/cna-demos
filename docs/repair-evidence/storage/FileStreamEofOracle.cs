// .NET Framework 4 boundary used by XNA File.Open; no modern-runtime inference.
using System;
using System.IO;

static class FileStreamEofOracle
{
    static int checks, failed;
    static void Check(bool ok, string name)
    {
        ++checks;
        if (!ok) { ++failed; Console.WriteLine("FAIL " + name); }
    }
    static int Main(string[] args)
    {
        if (args.Length != 1) return 2;
        bool created = false;
        try {
            using (var seed = new FileStream(args[0], FileMode.CreateNew, FileAccess.Write)) {
                created = true;
                seed.Write(new byte[]{1,2,3,4},0,4);
            }
            using (var file = new FileStream(args[0],FileMode.Open,FileAccess.ReadWrite,FileShare.ReadWrite)) {
                byte[] buffer = {77,77,77,77,77,77,77,77};
                Check(file.Read(buffer,1,6)==4,"short read count");
                Check(buffer[0]==77 && buffer[1]==1 && buffer[2]==2 && buffer[3]==3 &&
                      buffer[4]==4 && buffer[5]==77 && buffer[6]==77 && buffer[7]==77,"short read data and guards");
                Check(file.Position==4,"position after short read");
                Check(file.Read(buffer,1,2)==0,"repeated EOF count");
                Check(file.Position==4,"position after repeated EOF");
                try { Check(file.Seek(-1,SeekOrigin.Current)==3,"relative seek after EOF"); }
                catch (Exception e) { Check(false,"relative seek threw "+e.GetType().Name); }
                Check(file.Seek(0,SeekOrigin.Begin)==0,"explicit rewind control");
                Check(file.Read(buffer,1,4)==4 && buffer[4]==4,"full reread control");
                Check(file.Read(buffer,1,1)==0,"EOF before write");
                file.WriteByte(9);
                Check(file.Position==5,"write after EOF advances");
                Check(file.Length==5,"write after EOF extends");
                file.Position=0;
                Check(file.Read(buffer,1,6)==5 && buffer[5]==9,"write after EOF persists");
                Check(file.Position==5,"final short read position");
            }
            Console.WriteLine("checks="+checks+" failed="+failed);
            return failed==0?0:1;
        }
        finally { if (created) File.Delete(args[0]); }
    }
}
