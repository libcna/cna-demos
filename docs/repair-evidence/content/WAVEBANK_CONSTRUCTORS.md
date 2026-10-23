# ISSUE-028: WaveBank constructor paths and refusals

Continuation of [the retained-file repair](WAVEBANK_RETAINED_FILE.md), source
`3c67874ba54492d0d10258f0742824c1bb5bcec9`. The constructor baseline uses CNA
`04099802bbce32098cfd86004e265ab9c7f69ec9` and demos
`1aa8afef3c6a97dcce5906614449976306e95e03`, with the recorded pre-existing worktree
changes preserved. The lifetime checkpoint is published in both repositories.

## Contract and root cause

The genuine XNA 4.0 WaveBank constructors first validate non-null engine and
non-empty filename, resolve Path.GetFullPath, and validate a file longer than four
bytes with WBND magic. Both constructors refuse missing files. Failed native bank
creation produces InvalidOperationException. Its streaming bank retains an opened
file. See MEDIA_PATH_TRIAGE.md for reference hashes, MVIDs and the genuine14-case
WaveBankPathOracle.cs run. That oracle checks managed pre-native behavior with an
uninitialized non-null engine token; it does not claim successful native XACT audio.

CNA previously substituted title-rooted files for the eager constructor and
suppressed streaming file/header errors as an unprepared bank. FNA's unchecked
native-result behavior had also been encoded in CNA's C header and smoke test.
The historical ISSUE-028 demand for title-rooted streaming filenames was incorrect.

The repair resolves both constructors from cwd using Path.GetFullPath, preserving
CNA's existing case-insensitive file lookup. A read-only FileStream checks the first
five bytes, establishing XNA's length > 4/header boundary without narrowing a large
file length to Sharp Stream's current int32 Length property. Real open/read errors
keep their runtime exception types and full filename. Metadata parser failures now
raise InvalidOperationException with their cause. The previously fixed retained
native stream continues to use 64-bit positions and lazy per-entry reads.

The C API uses its existing exception barrier: missing file -> IO, wrong header ->
INVALID_ARGUMENT, invalid metadata -> INVALID_STATE; refusal leaves no owned handle.
Public comments explicitly correct the old FNA-based promise. No C ABI layout or
signature changed. Existing offset/packet-size compatibility behavior remains and
is described as such; no sector scheduling or offset support is claimed. No new
Windows sharing guarantee is claimed by the native retained-stream implementation.

## Tests before production edits

- Original14 reference-backed controls were copied unchanged to CNA's cross-module
  `tests/WaveBankConstructorContractTests.cpp`; TitleLocation belongs to runtime,
  so this does not add runtime as an audio module dependency.
- Full integration baseline45: **36PASS/9FAIL**,0.332 seconds.
- Six additional native boundary cases (both overloads: empty file, four-byte magic,
  and valid magic with truncated metadata) are source-backed, not represented as
  additional measured Windows oracle cases. Baseline51: **36PASS/15FAIL**,0.368 seconds.
- Correcting the native C smoke test to XNA's missing-streaming result fails before
  repair with code30. The old native corrupt-file stub assertion is replaced by an
  exact ArgumentException assertion with its XA-13/FNA provenance retained.

## Candidate validation

- Full integration: **51PASS**,0.540 seconds, all15 failures become passes and the
  previous36 passes remain passes.
- Full audio: **779 registered = 775PASS/4sameSKIP**,8.821 seconds. Exact outcome
  parity with the lifetime checkpoint after accounting for the documented corrupt
  file assertion/name correction. All four retained-file regressions remain passes.
- Targeted ASan/UBSan/leaks: **62PASS**,0.119 seconds, no findings or frozen-input
  changes. Ten translation units instrumented: XactParser, WaveBank, AudioEngine,
  SoundEffect, SoundEffectInstance, WaveBankTests, the new constructor fixture,
  TitleLocation, FileStream and Path. Other native and third-party libraries were
  not instrumented. No runtime suppression.

The C++ XACT demos now identify authored WaveBank assets with explicit title paths.
This is necessary because XNA resolves relative WaveBank paths from cwd, whereas
CTest runs from the build directory. The bank-lifetime demo retains its checks but
replaces two FNA-based empty-bank expectations with precise XNA refusals. C demos
preserve their controls and add five checks total: two explicit asset-path checks,
missing streaming-file refusal, and both invalid-header refusal routes.

The first final CTest sweep found one fixture error: the bank-lifetime missing-file
probe named a missing parent directory from CTest's cwd, so it correctly raised
DirectoryNotFoundException. The fixture now names a missing file in the existing
absolute asset directory. Its FileNotFoundException assertion was not broadened.
This failed run is retained as `repair-wavebank-constructor-ctest.*`.

The initial comparison script also assumed C++ check labels were printed with
`--quiet`; they are not. Final comparison verifies all JSON statuses/counts/markers/
messages and logged assertions where present. The two C++ assertion replacements
are directly visible in the source diff and supported by the genuine oracle.

Final native C: five audio/XACT CTestsPASS1.47 seconds. Complete consumer build
passes without compiler warnings. On both SOFTWARE and OPENGLES3, all32 audio/C
scenarios retain30PASS/1known114FAIL/1manual microphoneNOTRUN. Exact comparison
preserves statuses, parent JSON counts, markers and messages. Standalone C reports
zero checks in the parent JSON; its child logs prove exactly the five added checks
and preserve every old assertion. The first comparison incorrectly expected that
parent count to increase; the corrected audit uses the actual child assertion log.

All15 registered XACT/catalog CTestsPASS26.93 seconds from the build directory.
Catalog719 remains identical. All275 final artifacts are hash-verified after the
registered tests; freeze released. Original unchanged external probes: constructor
14PASS, lazy-path8PASS, retained-file6PASS, no frozen-input changes. Reports and
ignored drivers are under `build/linux-full/reports/repair-wavebank-constructor-*`,
`repair-wavebank-probes-constructors-after-*` and `build-probe/repair-wavebank-*`. No full campaign, physical-audio, Windows, browser, other-platform
or exact-visual rerun is claimed. The last full campaign checkpoint remains075.

Source repair: `783fe5739bbf968aae71983269115e4b29a3a016`. ISSUE-028 is now RESOLVED/FIXED_VALIDATED; historical evidence is retained. No028 knownIssue marker existed to remove.
