# Dynamic audio stress coverage scheduling

## Current validation

**Validated and committed:** CNA
`2af199aad4d11053a7549bee01f9efff272cd9e0`,
`Fix AUD-07-003 stress coverage with deterministic format rendezvous`.
Only the audio test file was committed (51 additions/5 deletions); no production
audio code, graphics candidate or inherited011 change was staged.

Actual sanitizer session75848 completed0: instrumented library build18679ms,
strict fixture compile8665ms, both original stress tests PASS398ms, XML inspection
29ms. ASan/UBSan/float-cast/leak detection found no issue; no suppressions,
frozen inputs unchanged, peak3717730304bytes and all memory events0. Together
with20/20 focused runs, the775-case audio module and accepted11000-case full
regression below, this completes the test-scheduling repair. The old failed
coverage run remains part of the evidence. No historical CNA-DEMOS issue is
closed merely by this reliability repair, and no TSAN/vendor coverage is claimed.

Native-v2 session88430 is now ACCEPTED (exit0): full11000 completed1508686ms,
10899PASS/101unchangedSKIP; exact comparison292ms retains every old004 outcome and
adds exactly the six event regressions asPASS. Audio module outcomes are identical
to the corresponding full-suite outcomes. Frozen inputs unchanged,
peak4297629696bytes/maxOOM0. The original failed30524 evidence remains retained.
The subsequent two-test audio sanitizer was accepted as recorded above.

Original session30524 terminated with exit1: full11000 cases in1442390ms,
10898PASS/101unchangedSKIP/one failure. Exact comparison with004 shows only the
recorded audio coverage failure and six newly added passing resource-event tests.
Frozen inputs unchanged; peak4297887744bytes, max/OOM/kill0. Its evidence remains
unaltered.

After that run terminated, the native test gained16 semaphore-rendezvous cycles
(eight live integer streams, eight live float streams), each requiring an exact
InvalidOperationException, not ObjectDisposedException. Playback must remain
Playing before/after submission. Exact16 rejections are required in addition to
the existing nonzero rejection assertion. All original3000 free-running calls
and3000 Play/Stop pairs remain; no production implementation was changed.

Session88430 executed `repair-resource-event-native-v2.mjs`: one-worker
build, fixed20 focused repetitions (every iteration required to pass), audio
module, original6 resource-event tests, graphics2985, full11000 and exact004
outcome comparison. Same private GPU/network/caps0/6GiB/no-swap policy.

Intermediate results: build11796ms, all20 focused iterations PASS804ms; audio
module771PASS/4SKIP9070ms with every outcome identical to004's full XML; original6
event cases PASS218ms, graphics2912PASS/73SKIP41221ms. The revised alternating-
format stress also passed inside the accepted full11000 run (20ms).

`repair-audio-stress-sanitizer.mjs` produced the accepted sanitizer evidence above.
The dependent resource-event gates are separate graphics work and do not expand
this audio repair's coverage claim. All validation used serial one-worker6GiB
scopes. No TSAN or vendor instrumentation pass is claimed.

## Observed failure (2026-10-21)

During the ISSUE-082 native regression gate, the unchanged
`DynamicSoundEffectInstanceTest.StressSubmitFloatBufferEXTAgainstRepeatedPlayCyclesNeverCorruptsLiveStream`
failed at `DynamicSoundEffectInstanceTests.cpp:1130`: `callsThrown == 0`.
The test completed in 52 ms. This is a failed coverage assertion, not evidence
that an unexpected exception or memory corruption occurred.

Original evidence is retained in
`build/linux-full/reports/repair-resource-event-native-full.log` and its
corresponding XML and audit. Session30524 finished with the failure before any
of its frozen source inputs were changed; that failed run remains unaltered.

The previous full native checkpoints contain the same test as PASS:

- `repair-shader-clone-native-resume-full.xml`: 28 ms.
- `repair-sprite-sampler-native-full.xml`: 21 ms.

Those earlier passes do not cancel the failure or establish reliable scheduling.
Before this repair, the test file's last committed change was
`85a23569cf8df6ce048eb34fcd9bb0f30ba54f53` (ISSUE-038 disposed access guards).
The pending resource-event repair changes graphics disposal, not this test or
the dynamic audio production implementation.

## Source-backed cause

The test stops its initial live stream before launching a producer. The producer
alternates 3000 float/integer submissions; independently, the calling thread
runs 3000 Play/Stop pairs and then disposes the instance. No synchronization
requires any producer operation to occur between Play and Stop.

A valid schedule can run all game-thread pairs before the producer gets its
first timeslice. The producer then observes ObjectDisposedException and exits;
`callsAttempted > 0`, but no live opposite-format rejection occurs. Likewise,
running the producer while the instance remains stopped can avoid all such
rejections. A nonzero rejection count is therefore not guaranteed by the test's
arrangement, even when the production guard is correct.

This is independent of the previously documented AUD-07-003/AUD-15-006 UAF.
Their free-running alternating-format stress must remain: serializing the whole
test would lose the teardown/submit interleavings that originally detected it.

## Original repair and validation plan (completed above)

Add a short rendezvous phase to the same producer/game-thread test. Across
repeated real Play/Stop cycles, alternate which format is live; require the
producer's opposite-format submission to finish before Stop. Assert every one
of those submissions throws InvalidOperationException, not ObjectDisposedException,
and that playback remains Playing. Retain all original 3000 free-running
submission attempts and 3000 Play/Stop cycles, disposal checks, and nonzero
rejection requirement. Do not add sleeps, timing tolerances or retries.

After the original full run terminates, apply the test-only repair, build the
native audio/full targets with one worker and the established memory limit,
run a fixed-count scheduling validation in which every iteration must pass,
and run the audio module. Re-establish full native parity before accepting the
resource-event sanitizer prerequisite. Keep the original failed audit intact.
The accepted validation above proves the test-only repair, not an audio product
repair or a historical campaign issue closure.
