# ISSUE-099: current reference preparation

Prepared during37318, NOT compiled/run: `VideoPlayerValidationTests.cpp` covers
nine native guard scenarios with real default-constructed players and no decoder,
device or playback. It checks exact exceptions/parameter/object names, refusal
before mutation, NaN/boundary controls, readable disposed configuration getters,
and existing transport/idempotence guards. The expected pre-fix split is five
failures/four controls; this is only a hypothesis until execution. It deliberately
does not substitute a fake Video token for actual native lifetime validation.
The existing real-video demo and future sanitizer lifetime cases remain required.
Prepared driver `build-probe/repair-video-player-validation-before.mjs` requires
the completed native-v4 gate and pins the four Video/VideoPlayer source files to
the validated622d0bd1a code. Run serially after the active038/073 external/final/
CTest gates, before any099 source change; do not treat the hypothesized split as
already reproduced.

## Genuine execution, 2026-10-21

56438 completed0 after the native full suite ended: all41 managed guard/retention
checks PASS on the exact pinned current Video/Graphics/Framework assemblies.
Strict C# build2246ms/run445ms, source hashes unchanged;peak295788544/all memory
events0 under1GiB/no swap. Reports `repair-video-player-guards-xna-*`.
This confirms the managed ordering, exact exception/object/parameter names,
retained identity and NaN boundary for the initialized-field/invalid-decoder seam.
It does NOT validate actual decoder creation/destruction, playback, texture pixels
or audio output. No099 production fix or closure is claimed. Earlier notes below
are preparation history, superseded only by this explicitly limited execution.

2026-10-21, read-only investigation while038/073 native full regression runs.
`VideoPlayerGuardsOracle.cs` is prepared, NOT compiled/run. Its41 managed checks
use explicitly populated uninitialized players with invalid decoder handles and
finalization suppressed; it never creates or plays a native decoder. Stop/Dispose
only traverse their no-decoder paths, so this does not validate real decode,
rendered textures, hardware, sound, or native resource destruction.

The current installed Video assembly is in GAC_MSIL, not GAC_32. SHA-256
`63c7673c936747abd9ebe779e8837c8b8add2c078a31216684fbf8c6bcab2745`,
MVID `6bc5f328-0528-401f-828a-1d9c05e99bfd`. This differs from the historical
ISSUE-099 recorded SHA; do not silently call it that older reference revision.
The Graphics reference assembly SHA is
`39aed7fd8e308ccace5ce9390256f1c829ca72c2eccd97d4a0f629f24015d3a5`.
The prepared fixture must pin these and the existing Framework identity when run.

Local decompiled VideoPlayer shows a narrower NaN rule than audio Pan: Volume
checks `value < 0 || value > 1`, so NaN reaches native assignment. The prepared
invalid-decoder control measures this managed boundary only, not what the actual
native decoder accepts. Do not generalize audio's explicit NaN rejection to video.

The original demo already covers real CNA playback/retained identity. A future
repair must inspect Video/VideoPlayer's C++ back-reference lifetime as well as
exception guards: keeping a raw Video pointer after Stop/Dispose must not create
a dangling pointer if its C++ owner later destroys it. No production099 change
or new current reproduction is claimed here. Do not run a parallel Wine oracle
while the ongoing full native suite owns its own reference Wine subprocesses.
