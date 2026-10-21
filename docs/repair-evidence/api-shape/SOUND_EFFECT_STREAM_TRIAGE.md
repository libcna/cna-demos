# ISSUE-029: stream shape and ownership constraints

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
