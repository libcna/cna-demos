# Media path reference triage

2026-10-20. Read-only follow-up while the separate root-batch builds/tests run.
Neither015 nor028 is closed or repaired by this entry.

Follow-up: genuine WaveBank path oracle now14/14PASS; current native streaming
probe6/6PASS, strict warning-free build and four native root-artifact hashes exact.
The initial reference compile refused direct access to internal TitleLocation;
the next refused GameTime without the Game assembly. Both setup logs remain. The
final fixture uses reflection through the already referenced Framework Curve type,
without changing any path/exception expectation. Reports
`repair-wave-bank-path-reflection-framework-xna-*` and `repair-wave-bank-path-native-*`.
No native audio engine is created in the genuine probe: inspected AudioEngine's
static constructor only initializes its managed dictionary/lock. The native probe
uses dummy audio and valid bank metadata without playing a cue. These are limited
path witnesses, not full audio or physical-hardware validation. Earlier preparation
notes below are historical; the unmeasured lazy-path concern still requires triage.

## Reference provenance

The installed genuine XNA4 Framework assembly has SHA-256
`8b7aa4c4939f243b21432747281cc8aacdcda56191a16d9eaa036b4136cf0da4`,
MVID `34D977AE-C3EC-4D07-AA6D-6FED6D8E3864`. Xact has SHA-256
`6824bb0b7b42626d1ed5b7ab7e4dab4a380fa010175d4de0fadb1c3904e491d1`,
MVID `8D274BE4-2C96-4001-8E0A-77D4B131C309`. Both are the4.0 GAC assemblies
in the private `.wine-cna-xna40` reference environment.

An older `/tmp/xna-framework-4.il` has a different MVID and was not silently treated
as this installed revision. Fresh dumps are `build-probe/repair-media-reference/`.
Framework monodis exits134 on a later mixed-mode data shape; that failure and its
partial output are retained. The complete targeted MediaSong_FromUri method and
matching module identity are present before the failure. Xact disassembly exits0.
Reports `repair-media-reference-{framework,xact}-disassembly.log`.

## ISSUE-015

The installed framework's MediaSong_FromUri checks Uri.IsAbsoluteUri. For a relative
URI, its complete managed method obtains TitleLocation.Path and combines it with the
URI text before passing the path to native media creation. Current CNA Song.cpp's
scheme-less path branch still passes the path directly to the constructor and has
a stale comment denying a title-root concept for Song. This supports the historical
finding; the dedicated current native decoy reproduction and actual media reference
execution remain to be performed. No audio/playback/hardware pass is claimed.

## ISSUE-028: historical expectation needs correction evidence

Both genuine Xact WaveBank constructors first call Path.GetFullPath on the supplied
filename, then the same managed CheckWaveBankHeader. The streaming overload then
opens that full path with File.OpenRead before native handle creation. There is no
TitleLocation combination in either constructor. Thus the historical assertion
that XNA requires a streaming bank under the title directory is unsupported; source
evidence instead points to cwd resolution, unlike Song.FromUri.

`WaveBankPathOracle.cs` is prepared to measure this at the genuine constructors'
managed pre-native boundary. It uses a non-null uninitialized engine token with
finalization suppressed and only missing/invalid-header files, so no native engine
or playback is needed. It tests relative cwd/title decoys, absolute paths, exact
exceptions/filenames and argument ordering for both overloads. Not run yet. Do not
claim healthy audio-engine initialization, valid bank parsing, streaming or offset
behavior from this deliberately limited reference seam.

`WaveBankPathProbe.cpp` is also prepared, not run: six current-CNA streaming
controls stage an existing valid bank under separate title/cwd roots, inspect
prepared state and exercise absolute paths/title overrides without playback.
It must use dummy audio and the existing XACT assets. Missing-file exception timing
is deliberately not claimed equal: CNA currently logs an unavailable streaming
bank while genuine constructors throw before native resource creation.

One additional source-only concern needs separate minimization before any conclusion:
CNA's case-insensitive path resolver preserves an already-existing relative spelling,
and the streaming decoder reopens its saved source path on lazy wave-data access.
Genuine XNA obtains an absolute path and retains an opened FileStream. A later cwd
change may therefore affect CNA's lazy read even though initial cwd resolution is
correct. Do not confuse this unmeasured lifetime/path concern with the incorrect
historical title-root expectation, and do not close028 from static evidence alone.
