# Audio instance guard repair checkpoint — 2026-10-21

ISSUE-038 and ISSUE-073 are fixed and validated for the available Linux environment.
This is a subsystem checkpoint, not completion of the campaign.

## Repairs and reference

- CNA `514a6a8c2a1d81b9dd54187f7a3a0aecb925414c` fixes073: disposal, prepacket
  spatial-latch reset, live-spatial refusal, range validation, then Pan mutation.
  Both setter overloads have18 native cases. The unchanged C assertion passes.
- CNA `85a23569cf8df6ce048eb34fcd9bb0f30ba54f53` fixes038: PendingBufferCount,
  IsLooped and both sample conversion helpers refuse disposed instances before
  argument validation/access. Five new native cases protect the contract.
  Two old FNA-based helper expectations are explicitly corrected to XNA exceptions;
  their inputs remain. Queue cleanup and the original1000-buffer stress bound
  remain asserted via a read-only seam, plus public disposal refusal.
- CNA `a52d6f0cede3592521e03f4b61404a1594c0b1ce` adds four explicit void casts
  to existing nodiscard CNJ exception assertions; neither inputs nor exceptions change.
- CNA `7af2cd42b3e60b883cbb2316fb05740e149f1300` makes the no-hardware harness
  override both SDL driver environment spellings. Inherited modern dummy audio
  must not override the harness's deliberately nonexistent legacy driver.

Genuine Framework SHA256
`8b7aa4c4939f243b21432747281cc8aacdcda56191a16d9eaa036b4136cf0da4`,
MVID `34d977ae-c3ec-4d07-aa6d-6fed6d8e3864`:073 oracle33/33 and038 oracle24/24 PASS.
These initialized-field managed guard oracles do not claim native XNA playback.
See [pan evidence](SPATIAL_PAN_TRIAGE.md) and [disposal evidence](DYNAMIC_DISPOSAL_TRIAGE.md).

## Accepted gates

Reports live in `build/linux-full/reports/repair-audio-instance-guards-*`.

| Gate | Result |
|---|---|
| Native `native-v4`, session16641 | Full10905:10804PASS/101 unchanged SKIP; prior10882 outcomes retained with two documented helper renames and23 newPASS. Full1202840ms; comparison115ms. |
| Audio / affected native focus | 775:771PASS/4 unchanged SKIP;175/175PASS. |
| Integrated `sanitizer`,76943 | 27/27PASS223ms, ASan/UBSan/float-cast-overflow and leaks enabled, no suppressions/findings; actual audio/Sharp Runtime instrumented, third-party libraries not instrumented. |
| External `external-v2`,69886 | Build7019ms/zero diagnostics; original18+5 cases PASS against external `CNA/libcna.so`;112 exact scenario comparisons. |
| Marker-only `external-final`,22724 | Build9836ms/zero diagnostics; same23 original cases and112 exact comparisons;318 non-browser artifacts byte-identical. |
| Registered `ctest`,23775 | Native32/32PASS11393ms; demos19/19PASS23532ms, including corrected Apply3D. No skips. |
| Platform static gates | Inventory, classification, renderer SDL boundary, ratchet and hot-path lint all PASS. |

Both external renderer groups contain56 scenarios:48PASS,7 unchanged historical
FAIL,1 manual NOT RUN each. The preceding015 checkpoint had46PASS/9FAIL/1NOT RUN;
only038/073 improve. Check counts and unrelated failed assertion labels remain
identical. The knownIssue markers for038/073 and073's obsolete restriction sentence
are removed only after their original demos passed. No original C assertion changed.

All accepted gates retain exact source hashes. Memory peaks: native1140305920,
sanitizer1258749952, external-v2 1996042240, final1265840128, CTest228753408 bytes;
all their cgroup memory events are zero. One compiler worker, shared ccache,
existing build trees, private display,6GiB maximum/no swap. Full native additionally
uses the private cna-test0 network namespace, owner UID and no capabilities.

## Rejected attempts and the demo correction

The original external37318 build succeeded in2680774ms with zero diagnostics;
23 original cases and both repaired demos passed. Its strict112 comparison failed
on exactly the SOFTWARE/OPENGLES3 instances of `audio.sound_effect_instance.apply_3d`.
That older demo explicitly required a refused Pan write to mutate-0.5 to0.75.
It contradicted genuine XNA and the073 C oracle. The original source is retained
in demos7486b2b, introduced9a6234b; failed logs and audit remain untouched.
Only that exact expected value, label and explanatory comment are corrected;
all18 assertions remain. This is one demo-expectation correction within073,
not another resolved historical ID. v2 verifies this exact source diff before
rerunning the unchanged baseline comparison. Original peak4303581184/high2623,
max/OOM/OOM-kill zero; it was not an OOM failure.

Earlier native build diagnostics, the no-hardware harness/stress failures and the
v3 extra Wayland SKIP are also retained. The last was a launcher error: forced X11
skips the same case that passes with private Wayland. v4 preserves the old PASS,
without changing that test. The first managed pan draft's wrong validation-order
assumption and first native fixture's dangling-else compile errors remain recorded.

## Limits and next work

Linux SOFTWARE/OPENGLES3 with dummy audio, not audible quality or physical audio
hardware validation. No new full719 demo sweep, global CTest campaign, screenshots,
other-renderer/platform runtime result or final Complete1128 closure is claimed.
The inherited011 ownership work was present during validation but remains unaccepted,
uncommitted and excluded from these repairs. Nothing was pushed or merged.
Next run the prepared028 large-offset and099 guard/lifetime baselines, then repair
the next coherent audio/media root cause; remaining ordinary defects are not blockers.
