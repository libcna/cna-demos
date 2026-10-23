# ISSUE-028: retain the streaming WaveBank input

This is a **partial repair**, not closure of CNA-DEMOS-ISSUE-028. The historical
TitleLocation requirement is disproved: genuine Windows XNA resolves both WaveBank
constructor filenames using Path.GetFullPath (the current working directory).
Its streaming bank retains its opened file until disposal. Constructor refusal
and path selection defects remain independently actionable after this lifetime fix.

Reference provenance and the unchanged genuine 14-control constructor oracle are
recorded in MEDIA_PATH_TRIAGE.md and WaveBankPathOracle.cs. The genuine oracle
covers managed checks before native bank creation, not working Windows XACT playback.
The retained-file behavior is supported by the reference implementation; the native
rename/unlink reproductions here run on Linux, not Windows.

## Current baseline, 2026-10-23

CNA next `79c56330faa5de25c813188ceee1b3482c530bf0`, demos next
`e842f7a4414d78e6f78f58ce66c3378ecb21dba3`, with the already-recorded foreign
content/audio/occlusion work preserved. No downstream repositories changed.

- Full CnaAudioTests: 775 registered, **771 PASS / 4 SKIP**, 8.638 seconds.
  The existing skips concern empty/misaligned/zero-length buffers and invalid channel
  enums; none was introduced by this batch.
- Unchanged constructor witness: **5 PASS / 9 FAIL**, 0.004 seconds. Exact same nine
  failures as the earlier reference-backed triage.
- Unchanged lazy-path witness: **6 PASS / 2 FAIL** across eight checks. Changing cwd
  loses the selected file or redirects the read to an unrelated short decoy.
- Unchanged retained-file witness: **4 PASS / 2 FAIL** across six checks. Renaming or
  replacing an absolute filename loses the already-selected bank.
- Four new native StreamingWaveBankFileTest regressions: **0 PASS / 4 FAIL**, 0.003
  seconds, before production changes. They cover cwd, cwd with a decoy, rename plus
  replacement, and unlink followed by disposal. They do not catch failures as skips.
- Consumer baseline on each of SOFTWARE and OPENGLES3: 32 audio/C-audio scenarios,
  **30 PASS / 1 existing FAIL / 1 manual NOT RUN**. The failure is ISSUE-114
  `audio.sound_effect.loop_region`; the manual scenario is microphone input.

The first standalone probe link lacked the runtime TitleLocation object; this is a
fixture build failure, not a product result. The v2 build includes that already-built
object and the canonical CnaAudioTests static dependency closure, without libcna.so
or duplicate runtime copies. All three probe sources remain unchanged. Logs and
hash audits: `build/linux-full/reports/repair-wavebank-probes-before-v2-*`.

## Implementation

WaveBank now retains the exact std::ifstream used for streaming metadata parsing.
An internal parser overload takes the existing open stream; the filename overload
remains available. Lazy reads seek the retained stream, clear previous stream state,
and retain the previous file-length bounds checks. The existing xactImplMutex_
serializes reads and disposal; destroying the pimpl closes the file. Wave audio is
still loaded per entry, not eagerly held in XwbData::fileData.

This scope deliberately preserves existing 64-bit native file positions. Sharp
Runtime Stream currently exposes int32 Position/Length; replacing the native stream
without addressing that lower-layer limitation would reduce the supported file
range. This patch does not claim .NET FileShare enforcement or Windows runtime
validation. Constructor cwd/header/exception and offset/packet-size behavior are
not changed or declared fixed.

## Validated lifetime sub-fix

Full native candidate: **779 registered = 775 PASS / 4 unchanged SKIP**, 8.669 seconds.
Every one of the 775 old test outcomes is unchanged; the four new tests pass.
Build has no compiler warnings. Native red and candidate logs/XML are retained under
`repair-wavebank-native-red*` and `repair-wavebank-lifetime-*`.

The unchanged lazy-path eight and retained-file six checks now all pass. All14
constructor outcomes remain exactly unchanged (5PASS/9FAIL). Inputs/libraries were
hashed during each probe run; no changes during execution.

Targeted ASan/UBSan with leak detection: **42PASS**,0.063 seconds. Six translation
units instrumented: XactParser, WaveBank, AudioEngine, SoundEffect, SoundEffectInstance
and WaveBankTests. Other native/third-party dependencies were not instrumented.
No findings or frozen-input changes. No suppression, retries or expanded tolerance.

Five rebuilt C audio/XACT controls pass. Full consumer build passes without compiler
warnings. SOFTWARE32 and OPENGLES3 32 each retain **30PASS/1same114FAIL/1manualNOTRUN**.
Exact comparison checks status, check counts, knownIssue, message and logged assertion
multiplicities: no deltas. Fifteen registered XACT/catalog CTests pass in13.65 seconds.
Catalog719 is identical to025; all275 final consumer artifact hashes verify after
all gates. Freeze released. All heavy builds/tests were serial, using the shared
ccache, existing trees, one compiler,6GiB cap/no swap and private GPU sessions.

No issue marker has been removed and no final disposition has been set. Source repair: `3c67874ba54492d0d10258f0742824c1bb5bcec9`. Publication is pending the evidence checkpoint.
The last full campaign regression remains checkpoint075; this document makes no
new full-suite, visual, physical-audio, browser or other-platform claim.
