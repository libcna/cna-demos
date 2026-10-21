# ISSUE-073: failed pan assignment and XNA validation order

2026-10-21. No production fix yet. The completed058 external baseline still has
the original one failing C assertion on both SOFTWARE/OPENGLES3: refusing direct
pan on a live3D instance changes its public pan. The implementation remains the
same during015's ongoing external build.

Genuine Framework SHA-256
`8b7aa4c4939f243b21432747281cc8aacdcda56191a16d9eaa036b4136cf0da4`,
MVID `34d977ae-c3ec-4d07-aa6d-6fed6d8e3864`. The complete SoundEffectInstance.set_Pan
IL at RVA0x1ac10 establishes this order:

1. Refuse a disposed instance.
2. If no packet has been submitted, clear the3D latch.
3. Refuse a still-latched3D instance with InvalidOperationException.
4. Validate the requested range (including NaN).
5. Perform the native assignment, then store the public pan.

The initial new `SpatialPanRefusalOracle.cs` incorrectly assumed that range checks
precede the live3D refusal. Session69388 is retained as a FAILED reference draft:
strict C# build2693ms PASS, genuine run518ms exits1,10PASS/4FAIL (both wrong
exception-family and parameter expectations for invalid range/NaN while live3D).
All five prior-pan checks passed; the historical CNA defect remains legitimate.
No source or report from that first measurement is overwritten. This is not an
existing demo-expectation correction or a CNA regression.

`SpatialPanRefusalOrderOracle.cs` corrects that assumption using the actual IL and
adds non3D invalid-range controls so parameter-name checks remain covered. It also
checks the prepacket latch reset even when range validation subsequently fails.
67026 TERMINAL0: all33 genuine checks PASS; strict C# build4003ms/run837ms,
peak262078464/all memory events0. Source/hash/MVID remain exact. Reports:
`repair-spatial-pan-refusal-order-xna-*`; first reports omit `-order`.

Both runs use explicitly populated uninitialized instances with finalization
suppressed, and only refusal branches before native voice calls. They validate
actual managed property/exception ordering, not live playback or native audio
initialization. The separate original C demo supplies real CNA playback evidence.

Repair constraints: move public Pan mutation behind the refusal gates; do not
retain CNA's current range-before-mode ordering merely because it seems cleaner.
Add native coverage through both C++ setter overloads and preserve the existing
C assertion and its original label. Verify live3D, non3D, prepacket and disposed
ordering without introducing sleeps or altering the original playback oracle.
