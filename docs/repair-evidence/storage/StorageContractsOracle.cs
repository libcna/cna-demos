// Genuine XNA Storage methods with an isolated root injected before first use.
// This deliberately does NOT validate device selection or container construction.
using System;
using System.IO;
using System.Reflection;
using System.Runtime.Serialization;
using Microsoft.Xna.Framework.Storage;

static class StorageContractsOracle
{
    static int checks, failed;
    static void Check(bool ok, string name)
    {
        ++checks;
        if (!ok) { ++failed; Console.WriteLine("FAIL " + name); }
    }
    static void Disposed(Action action, string name)
    {
        Type caught = null;
        try { action(); } catch (Exception e) { caught=e.GetType(); }
        Check(caught==typeof(ObjectDisposedException), name);
    }
    static bool Refuses(Func<Stream> open)
    {
        try { using(var stream=open()) {} return false; }
        catch(IOException) { return true; }
    }
    static int Main(string[] args)
    {
        if(args.Length!=1 || Directory.Exists(args[0])) return 2;
        Directory.CreateDirectory(args[0]);
        try {
            File.WriteAllBytes(Path.Combine(args[0],"seed.bin"),new byte[]{1,2,3,4});
            Directory.CreateDirectory(Path.Combine(args[0],"child"));
            var c=(StorageContainer)FormatterServices.GetUninitializedObject(typeof(StorageContainer));
            typeof(StorageContainer).GetField("_rootPath",BindingFlags.Instance|BindingFlags.NonPublic).SetValue(c,Path.GetFullPath(args[0]));
            typeof(StorageContainer).GetField("_displayName",BindingFlags.Instance|BindingFlags.NonPublic).SetValue(c,"isolated probe");
            Check(c.DisplayName=="isolated probe" && !c.IsDisposed,"live properties");
            Check(c.FileExists("seed.bin") && c.DirectoryExists("child"),"live filesystem");
            FileAccess[] accesses={FileAccess.Read,FileAccess.Write,FileAccess.ReadWrite};
            FileShare[] shares={FileShare.None,FileShare.Read,FileShare.Write,FileShare.ReadWrite};
            foreach(var access1 in accesses) foreach(var share1 in shares)
            foreach(var access2 in accesses) foreach(var share2 in shares) {
                bool expected=((int)share1&(int)access2)==(int)access2 && ((int)share2&(int)access1)==(int)access1;
                using(var first=c.OpenFile("seed.bin",FileMode.Open,access1,share1))
                    Check(Refuses(()=>c.OpenFile("seed.bin",FileMode.Open,access2,share2))==!expected,
                          "share matrix "+access1+"/"+share1+" -> "+access2+"/"+share2);
            }
            using(var first=c.OpenFile("seed.bin",FileMode.Open))
                Check(Refuses(()=>c.OpenFile("seed.bin",FileMode.Open,FileAccess.Read,FileShare.ReadWrite)),"two-argument container open is exclusive");
            using(var first=c.OpenFile("seed.bin",FileMode.Open,FileAccess.Read))
                Check(Refuses(()=>c.OpenFile("seed.bin",FileMode.Open,FileAccess.Read,FileShare.ReadWrite)),"three-argument container open is exclusive");
            using(var append=c.OpenFile("seed.bin",FileMode.Append))
                Check(!append.CanRead && append.CanWrite && append.Position==4,"two-argument Append defaults to write access");
            using(var first=new FileStream(Path.Combine(args[0],"seed.bin"),FileMode.Open,FileAccess.Read)) {
                Check(!Refuses(()=>new FileStream(Path.Combine(args[0],"seed.bin"),FileMode.Open,FileAccess.Read,FileShare.Read)),"three-argument FileStream permits another reader");
                Check(Refuses(()=>new FileStream(Path.Combine(args[0],"seed.bin"),FileMode.Open,FileAccess.Write,FileShare.ReadWrite)),"three-argument FileStream refuses a writer");
            }
            int events=0;
            c.Disposing+=(s,e)=>{Check(s==c && e==EventArgs.Empty && c.IsDisposed,"disposing event identity and state");++events;};
            c.Dispose(); c.Dispose();
            Check(c.IsDisposed && events==1,"idempotent disposal");
            Disposed(()=>{var ignored=c.DisplayName;},"DisplayName");
            Disposed(()=>{var ignored=c.StorageDevice;},"StorageDevice");
            Disposed(()=>c.CreateDirectory("new"),"CreateDirectory");
            Disposed(()=>c.DeleteDirectory("child"),"DeleteDirectory");
            Disposed(()=>c.DirectoryExists("child"),"DirectoryExists");
            Disposed(()=>c.GetDirectoryNames(),"GetDirectoryNames");
            Disposed(()=>c.GetDirectoryNames("*"),"GetDirectoryNames pattern");
            Disposed(()=>{using(var stream=c.CreateFile("new.bin")){}},"CreateFile");
            Disposed(()=>c.DeleteFile("seed.bin"),"DeleteFile");
            Disposed(()=>c.FileExists("seed.bin"),"FileExists");
            Disposed(()=>c.GetFileNames(),"GetFileNames");
            Disposed(()=>c.GetFileNames("*"),"GetFileNames pattern");
            Disposed(()=>{using(var stream=c.OpenFile("seed.bin",FileMode.Open)){}},"OpenFile mode");
            Disposed(()=>{using(var stream=c.OpenFile("seed.bin",FileMode.Open,FileAccess.Read)){}},"OpenFile access");
            Disposed(()=>{using(var stream=c.OpenFile("seed.bin",FileMode.Open,FileAccess.Read,FileShare.Read)){}},"OpenFile share");
            Disposed(()=>c.FileExists(""),"disposed before empty path");
            Disposed(()=>c.CreateDirectory("../outside"),"disposed before path validation");
            Disposed(()=>c.GetFileNames(""),"disposed before pattern validation");
            Check(File.Exists(Path.Combine(args[0],"seed.bin")) && Directory.Exists(Path.Combine(args[0],"child")) &&
                  !File.Exists(Path.Combine(args[0],"new.bin")) && !Directory.Exists(Path.Combine(args[0],"new")),"rejected operations preserve files");
            Console.WriteLine("checks="+checks+" failed="+failed);
            return failed==0?0:1;
        }
        finally { Directory.Delete(args[0],true); } // Only the new isolated probe root.
    }
}
