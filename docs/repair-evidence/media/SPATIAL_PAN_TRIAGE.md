# ISSUE-073: failed pan assignment and XNA validation order

**Resolved:** CNA514a6a8c2a1d81b9dd54187f7a3a0aecb925414c. Accepted native full,
sanitizer27, original/final external112 and32native/19demo CTest gates are recorded
in [the checkpoint](AUDIO_INSTANCE_GUARDS_CHECKPOINT.md). The C assertion is unchanged;
only its knownIssue marker/obsolete restriction are removed. The separate old
Apply3D demo expectation correction is fully documented below, not suppressed.
All earlier RUNNING/not-fixed statements below are retained chronological history.

## Native reproduction, 2026-10-21

37318 original external gate TERMINAL1: build2680774ms/zero diagnostics, original
18 pan and5 disposal cases PASS against the actual external library, and both
original038/073 demos PASS on both renderers. Strict112-case comparison correctly
rejected exactly two unexpected failures: `audio.sound_effect_instance.apply_3d`
on SOFTWARE/OPENGLES3, each its same18 checks with one failure. Its older assertion
explicitly expected the refused Pan write to expose0.75 instead of retaining-0.5;
the comment acknowledged CNA mutation before its mode gate. This conflicts with
the genuine33-check oracle and original073 C oracle. Original source is preserved
in demos7486b2b (introduced by9a6234b); original logs/audit are retained. This is a
demo-expectation correction within073, not a new runtime defect or an extra closed
historical ID. Replace only that strict expected value/comment/label, retaining
the check and all other17 assertions. No tolerance, suppression or changed status.
Source hashes unchanged during the failed run;peak4303581184/high2623/max0/OOM0.
69886 external-v2 RUNNING with exact-diff verification of this one oracle change,
the same112 baseline comparisons and both knownIssue markers still present.
No closure until v2, metadata-final and registered CTests pass.

16641 native-v4 accepted: full10905=10804PASS/101unchangedSKIP, all10882 prior
outcomes retained with two explicit038 helper renames and23 newPASS; full1202840ms,
compare115ms. Audio775=771PASS/4sameSKIP, focused175PASS, hashes unchanged;
peak1140305920/events0. The v3 extra Wayland SKIP comparison failure remains
recorded; forced-X11 versus private-Wayland controls identify the launch error,
not a test defect. Same unchanged Wayland case passes in the corrected full run.
76943 integrated sanitizer27/27PASS: instrumented audio/Sharp Runtime, ASan/UBSan/
float-cast-overflow/leaks enabled, no suppressions or findings. Library build32465ms,
strict fixtures15999ms/run223ms/XML22ms, hashes unchanged;peak1258749952/events0.
Original external112 scenario validation37318 is now running with both markers
retained. No073 closure until the original C scenario and remaining gates pass.

41722 focused candidate completed0: build116708ms/no diagnostics; original18/18
pan cases PASS (strict compile3448ms/run131ms/XML32ms), original5/5 dynamic cases
PASS, hashes unchanged,peak1260371968/all memory events0. Wider21670 native gate
is running; no issue closure or external original-demo pass is claimed yet.

59404 strict compile failed on two dangling-else warnings caused by GoogleTest
macros inside unbraced if-constexpr statements. No native cases ran and no final
audit was emitted. Original source is committed in50d0c97; original compile log
is retained. Braces correct the fixture syntax without changing any expectation.
75902 (`repair-spatial-pan-refusal-before-v2-*`) then reproduces exactly10FAIL/
8PASS across18 cases. Real looping voices are confirmed Playing; strict compile
2456ms/run110ms/XML18ms, hashes unchanged,peak454631424/all memory events0.

Local candidate now orders disposal, prepacket latch reset, live3D refusal, range
validation and stored pan mutation. It adds the unchanged18-case fixture to CNA
native audio tests and corrects contradictory public/internal comments. Focused
41722 rebuild/original-fixture validation is RUNNING together with038's guards;
no repair acceptance or closure yet. Other preparation notes below are historical.

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

Prepared native witness (not yet compiled or run): `SpatialPanRefusalTests.cpp`
has nine scenarios through each C++ setter overload (18 cases). It constructs
real looping instances with the configured dummy device, retains public pan and
checks the existing read-only spatial-latch seam. A missing device fails instead
of skipping. Live spatial, nonspatial, prepacket and disposed controls mirror the
measured refusal order; no sleeps, retries or new production seam are required.
Keep the native sources frozen until the ongoing015 external/CTest gates finish.
