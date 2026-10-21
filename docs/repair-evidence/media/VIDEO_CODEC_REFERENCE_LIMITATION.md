# ISSUE-101 — native decoder reference limitation

2026-10-21, after the accepted PBR native72714 gate. This is a **partial,
platform-limited observation, not a passing reference or a product repair**.

VideoCodecFailureOracle.cs compiled with the installed genuine .NET4 compiler
without diagnostics and ran against the pinned XNA Framework/Video assemblies.
It normally constructed/disposed VideoPlayer objects and invoked the actual
UnsafeNativeMethods.VideoDecoder_Play proxy, not an invalid/fabricated native
handle. No GraphicsDevice, public Play call, texture or pixel observation was used.

Immediate returns captured under Wine:

| Input | HRESULT | Failure translator |
| --- | --- | --- |
| Missing file | 0x80070057 | System.ArgumentException |
| Existing C# text source | 0x00000000 | none |
| Existing empty WMV | 0x00000000 | none |
| Existing truncated WMV | 0x00000000 | none |
| Valid silent64x48 WMV control | no completed observation | Wine abort |

The valid, provenance-documented control hit Wine's unimplemented
`mfplat.dll.MFCreateVideoMediaType` and started WineDbg. Therefore these partial
returns do **not** establish successful native playback, Windows codec behavior,
asynchronous error timing or an exact exception contract for all invalid files.
In particular, do not register the provisional FNA-based
VideoDeferredOpenFailureTests.cpp as a genuine-XNA oracle. Do not change CNA to
throw merely to satisfy that unverified assumption. The separate completed32
managed guard/translator checks still establish the missing-file ArgumentException.

Only the owned `cna-repair-video-codec-reference.scope` was stopped after
inspection. The observer exited130 from deliberate termination, not PASS. One
remaining winedevice.exe ignored SIGTERM; it was killed within that exact scope.
No global Wine kill, installation, codec replacement or other agent process was
targeted. Scope limit1GiB/no swap; observed pre-stop peak475,926,528 bytes. A final
memory-events snapshot and normal driver audit were not produced, so none is
invented. The earlier native gate had already ended; PBR sanitizer35 then passed.

Retained logs in build/linux-full/reports:

- repair-video-codec-xna-build.log, SHA256
  `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` (empty, no diagnostics).
- repair-video-codec-xna-run.log, SHA256
  `b85a6a566677abfb4b1758f78f1ba532d9b1632e76bb3f1978d2a2e097d10534`.
- Observer source SHA256
  `6bf0747f44994f61b4b12672100eaa04c865617347d8a04d1a4b0ea2eb25da03`.

Framework MVID34d977ae-c3ec-4d07-aa6d-6fed6d8e3864;
Video MVID6bc5f328-0528-401f-828a-1d9c05e99bfd. A functional native Windows media
reference is still required before claiming complete codec/error-timing parity.
No issue is closed and no C API contract decision is inferred by this experiment.
