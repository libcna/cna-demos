import fs from 'node:fs';
import path from 'node:path';
import {spawnSync} from 'node:child_process';
// Run from the cna-demos root inside a 6 GiB no-swap systemd scope; it writes reports and the
// overlay under build/ and build-probe/ (copied from build-probe/repair-storage-ctor-reference.mjs).
// Genuine XNA StorageContainer construction (CNA-DEMOS-ISSUE-010). The shared prefix maps Documents
// to the owner's real ~/Documents, so this builds an overlay prefix: copied registry, symlinks to
// the shared drive_c content, and a real empty Documents fixture. Nothing is written to the shared
// registry or to ~/Documents. The overlay is left in place for a guarded manual removal.
const root=process.cwd(),prefix=root+'/build/linux-full/reports/repair-storage-ctor-reference-v1';
const scope='/sys/fs/cgroup'+fs.readFileSync('/proc/self/cgroup','utf8').match(/^0::(.+)$/m)[1];
if(fs.readFileSync(scope+'/memory.max','utf8').trim()!=='6442450944')throw Error('6GiB cap');
const shared='/home/robertvokac/.wine-cna-xna40',overlay=root+'/build-probe/repair-storage-ctor-wineprefix';
const user='robertvokac',documents=overlay+'/drive_c/users/'+user+'/Documents';
fs.mkdirSync(overlay);
for(const f of ['system.reg','user.reg','userdef.reg','.update-timestamp'])fs.copyFileSync(shared+'/'+f,overlay+'/'+f,fs.constants.COPYFILE_EXCL);
fs.mkdirSync(overlay+'/dosdevices');fs.symlinkSync('../drive_c',overlay+'/dosdevices/c:');fs.symlinkSync('/',overlay+'/dosdevices/z:');
fs.mkdirSync(overlay+'/drive_c/users/'+user,{recursive:true});
for(const d of ['csdev','ProgramData','Program Files','windows'])fs.symlinkSync(shared+'/drive_c/'+d,overlay+'/drive_c/'+d);
fs.symlinkSync(shared+'/drive_c/users/Public',overlay+'/drive_c/users/Public');
for(const d of ['AppData','Contacts','Favorites','Links','Saved Games','Searches'])fs.symlinkSync(shared+'/drive_c/users/'+user+'/'+d,overlay+'/drive_c/users/'+user+'/'+d);
for(const d of ['Desktop','Downloads','Music','Pictures','Videos','Documents'])fs.mkdirSync(overlay+'/drive_c/users/'+user+'/'+d);
const win=p=>'Z:'+p.replaceAll('/','\\');
const source=root+'/docs/repair-evidence/storage/StorageContainerCreationOracle.cs';
const refs=path.resolve('../xna4-decomp/reference/xna4/original/windows');
const env={...process.env,WINEPREFIX:overlay,WINEDEBUG:'-all',DISPLAY:'',WAYLAND_DISPLAY:''};
const stages=[];let error=null,folders=null;
function run(label,args){const log=prefix+'-'+label+'.log',fd=fs.openSync(log,'wx'),start=Date.now();const r=spawnSync(args[0],args.slice(1),{env,stdio:['ignore',fd,fd],timeout:180000,killSignal:'SIGKILL'});fs.closeSync(fd);const s={label,exit:r.status,signal:r.signal,error:r.error?.message,elapsedMs:Date.now()-start};stages.push(s);console.log(JSON.stringify(s));if(r.status!==0)throw Error(label);return fs.readFileSync(log,'utf8');}
try{
  run('compile',['wine','C:\\windows\\Microsoft.NET\\Framework\\v4.0.30319\\csc.exe','/nologo','/warnaserror+','/platform:x86','/target:exe','/out:'+win(prefix+'.exe'),'/reference:'+win(refs+'/Microsoft.Xna.Framework.dll'),'/reference:'+win(refs+'/Microsoft.Xna.Framework.Storage.dll'),win(source)]);
  const created=run('create',['wine',win(prefix+'.exe'),'C:\\users\\'+user+'\\Documents','create']);
  folders=created.match(/^folders saved=(.+) title=(.+) all=(.+) one=(.+)\r?$/m);
  if(!folders)throw Error('folder names');
  const denied=documents+'/'+folders[1]+'/'+folders[2]+'/denied';
  fs.mkdirSync(denied,{recursive:true});fs.chmodSync(denied,0o555);
  try{run('denied',['wine',win(prefix+'.exe'),'C:\\users\\'+user+'\\Documents','denied']);}
  finally{fs.chmodSync(denied,0o755);}
}catch(e){error=e.stack;}
spawnSync('wineserver',['-w'],{env,timeout:60000});
const memory={peak:fs.readFileSync(scope+'/memory.peak','utf8').trim(),events:fs.readFileSync(scope+'/memory.events','utf8')};
const sharedUnchanged=fs.readlinkSync(shared+'/drive_c/users/'+user+'/Documents');
fs.writeFileSync(prefix+'-audit.json',JSON.stringify({error,stages,folders:folders?.slice(1),overlay,sharedDocumentsLink:sharedUnchanged,memory},null,2)+'\n',{flag:'wx'});
console.log(JSON.stringify({error,memory}));process.exitCode=error?1:0;
