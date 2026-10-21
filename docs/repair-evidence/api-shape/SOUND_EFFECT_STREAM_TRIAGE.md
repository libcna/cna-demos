# ISSUE-029: stream shape and ownership constraints

## Additional native format fixture prepared (2026-10-21)

`SoundEffectXnaWaveFormatTests.cpp` adds nineteen UNCOMPILED/UNRUN native cases
corresponding to the completed71-check managed format/chunk reference: eleven
successful inputs and eight exact refusals. It preserves source ownership, loop
endpoints and exact rounded duration, using the existing read-only loop seam.
It does not claim direct access to the parsed payload array, which is internal to
the genuine parser. Together with the nine initial stream cases this prepares28
native regressions; none can pass until the real Stream overload is implemented.
No native source changed during the ongoing038/073 regression gate.

Source inspection found another implementation constraint: existing SoundEffect
Duration is derived from mixer frames via TimeSpan::FromSeconds, whereas genuine
WavFile records a millisecond-rounded duration (including zero for eight frames at
48000Hz). Preserve the measured metadata when implementing the new overload;
do not silently route it through the extension and assume identical semantics.
This is not permission to alter unrelated extension or pipeline decoding rules.

## Extended genuine execution (2026-10-21)

52326 TERMINAL0: all71 extended managed WAV checks PASS, with exact Framework
hash/MVID and unchanged source. Strict C# build2708ms, run585ms;
peak285597696/all memory events0 in a separate1GiB no-swap scope. Reports
`repair-sound-effect-wave-chunks-xna-*`. This small reference ran while the sole
C++ external015 build continued; no second heavy build or CNA source mutation.
It confirms complete-frame truncation, inclusive SMPL end, WSMP precedence even
when its loop is invalid, success-only stream closure, and millisecond-rounded
duration. As before, this is the genuine managed parser, not successful public
native sound creation or physical output. The nine native cases remain UNCOMPILED.

## Prepared native parity fixture (2026-10-21)

`SoundEffectWaveChunksOracle.cs` additionally prepares71 managed reference checks
for PCM8/16 mono/stereo, rate/alignment limits, partial frames, odd unknown chunks,
RIFF size, malformed trailing chunks, SMPL inclusive endpoints, WSMP priority and
millisecond-rounded duration. It is NOT compiled/run yet. Its separate driver is
`build-probe/repair-sound-effect-wave-chunks-reference.mjs`; it pins the actual
genuine Framework hash/MVID. Existing25-control evidence is not overwritten.

`SoundEffectXnaStreamTests.cpp` prepares nine native cases: three successful
whole/short-read inputs and six exact failure families, with source closure,
duration and the eight-frame default loop asserted. It has NOT been compiled or
run. It deliberately calls the missing System::IO::Stream overload, never an
adapter/fallback to the existing std::istream extension. Successful controls will
use the existing dummy-audio test environment, not claim physical output.

The native API should follow the existing Texture2D reference-shaped Stream&
convention. The genuine null-argument control remains reference evidence; a C++
reference has no corresponding valid null input. Do not add an unnecessary public
pointer overload just to manufacture one.

Implementation trap confirmed by source inspection: Sharp Runtime BinaryReader's
destructor closes its stream by default. A stack reader in the XNA WAV parser must
use leaveOpen=true, then explicitly close the source only after parsing succeeds;
otherwise exception unwinding would violate the measured failure ownership.
Reuse BinaryReader's short-read handling rather than duplicating System.IO in CNA.
Do not change the lenient pipeline WAV reader or the existing std::istream path.

## Genuine execution (2026-10-21)

95566 completed0: all25 oracle checks pass against the installed XNA4 Framework
assembly with MVID34d977ae-c3ec-4d07-aa6d-6fed6d8e3864. Strict C# compilation and
genuine execution are retained in `repair-sound-effect-stream-contract-xna-*`.
Peak317890560/all memory events0. Successful managed parses with reads capped at
one/three bytes retain all16 sample bytes and the eight-frame default loop, then
close the source. Failure cases retain the exact exception family/parameter and
leave the source open. This validates the managed parser and public pre-native
refusal boundaries only, not successful public sound creation or physical audio.
The original prepared source is unchanged; no production029 repair exists yet.

Read-only follow-up, 2026-10-21. The existing SoundEffectStreamProbe.cpp still
describes a real API gap: only std::istream& is declared. It has not been rerun
against today's ownership candidate; the older failure remains in README.md.

Do not implement a generic stream adapter and assume that is the entire XNA
contract. The installed genuine Framework assembly (identity/hash recorded in
../content/MEDIA_PATH_TRIAGE.md) has these complete managed methods:

- SoundEffect.FromStream(Stream), RVA0x195e4, checks null then constructs from
  the stream. The constructor, RVA0x19608, calls WavFile.Open before native Create.
- WavFile.Open, RVA0x1b824, and its constructor, RVA0x1b848, access Length,
  construct BinaryReader, parse header/chunks and call reader.Close only after
  successful parsing. They do not use an unconditional finally-close.
- Empty source names ArgumentNullException("source"); bad header and malformed
  format take distinct exception paths. Short reads are handled by BinaryReader,
  not treated as EOF merely because one read returns less than requested.
- WavFile's format validation is narrower than CNA's general internal WAV decoder.
  The existing ReadWavFormatAndData is intentionally lenient and supports more
  formats for the content pipeline. Do not silently change that shared parser's
  contract merely to provide the missing XNA overload.

`SoundEffectStreamOracle.cs` is prepared, NOT compiled/run. It invokes genuine
WavFile.Open by reflection for successful parsing and the public FromStream for
inputs that fail before native sound creation. It covers short reads, success-
only source closure, empty/unreadable/unsupported-length sources, malformed
header and non-PCM input. No physical audio or successful public native playback
validation is claimed. Measure these controls before choosing the implementation.
Preserve the current std::istream overload as an explicitly marked extension;
do not change its existing callers just to make the API-shape probe compile.
