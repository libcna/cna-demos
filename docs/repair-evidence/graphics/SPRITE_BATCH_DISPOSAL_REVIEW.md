# SpriteBatch disposal and GraphicsResource events — inherited review, 2026-10-23

The owner explicitly approved matching XNA/C++ for C disposal-event timing and
matching XNA SpriteBatch post-Dispose behavior. These decisions supersede the
pending questions in `SPRITE_BATCH_TRIAGE.md`; historical failures remain there.

## Genuine XNA observations

`SpriteBatchDisposedGpuObservation.cs` uses the shipped XNA 4.0 assemblies and a
real HiDef device under Wine/DXVK on a private Linux display. Each mode runs in a
fresh process. Graphics assembly SHA-256:
`39aed7fd8e308ccace5ce9390256f1c829ca72c2eccd97d4a0f629f24015d3a5`.
Framework assembly SHA-256:
`8b7aa4c4939f243b21432747281cc8aacdcda56191a16d9eaa036b4136cf0da4`.
This is not native Windows validation.

- Closed disposed batch: End and a non-null Draw reject the missing pair with
  InvalidOperationException; null Draw gives ArgumentNullException. Deferred
  Begin and Draw succeed. End then crashes in EffectParameter.SetValue(Matrix).
- Closed disposed batch, Immediate Begin: the same native effect crash.
- Open deferred batch, empty or queued: Dispose leaves the pair, queued count
  and device counter unchanged. End reaches the same native effect crash.
- Open Immediate batch: Dispose leaves its pair and both device counters intact.
  Draw throws ObjectDisposedException naming DynamicVertexBuffer; End succeeds
  and clears the pair and both counters. A later deferred Begin succeeds.
  Repeated Dispose does not clear that newly opened pair.

The four crash processes exit 5. The two Immediate cases exit 0 and print
OBSERVATION_COMPLETE. These are measured reference outcomes, not six passing
conformance tests. The first driver timed out because Wine opened its debugger;
the retained v2 run disables winedbg exactly as CNA's existing XNA harness does.
The v2 run has no source drift, a 343,957,504-byte peak and zero memory events.
Reports: `repair-inherited-sprite-disposed-gpu-v2-xna-*`.

## CNA repair boundary

Dispose releases native SpriteBatch storage but preserves the pair, pending
queue and device accounting. Managed operations retain their sequencing rules;
deferred Draw retains its texture until queue destruction. Immediate End can
still balance its counters. An Immediate Draw refuses the disposed vertex buffer.
At state application, where the measured XNA reference dereferences its destroyed
effect, CNA throws ObjectDisposedException naming SpriteEffect. This is a deliberate
safe native-boundary limitation, not a claim to reproduce the reference crash.
It does not restore the former blanket rejection of all post-Dispose operations.

The separately reproduced GraphicsResource event defect is fixed by publishing
IsDisposed before Disposing. Reentry and exceptions cannot repeat the event;
device registration is retired even if a callback throws. C documentation and
C regression expectations are explicitly corrected with the owner's approval.

## Validation status

Native compilation passes. All52 selected SpriteBatch/event regressions pass on
SOFTWARE, OPENGLES3, OPENGL33, WEBGPU, SDL_GPU and FNA3D (its default driver).
Full configured OPENGLES3 graphics:3,051 PASS/115 unchanged SKIP (3,166 total).
Only two disproved post-disposal tests are renamed/corrected; three new accounting/
lifetime tests are added. Other deltas are the separately committed113 counter fix.
The C GraphicsResource smoke passes with the owner-approved published-state timing.
Reports: `repair-inherited-ownership-sprite-native-*`, including exact parity JSON.

The renewed SpriteBatch/event sanitizer closure passes 106/106 under ASan,
UBSan, float-cast-overflow and leak detection. The actual SpriteBatch and
GraphicsResource implementations are linked; all 145 existing diagnostics
match the prior multiset. Peak memory was 940,290,048 bytes with no memory-limit
event or source drift. The first driver incorrectly required every repeated
warning string to have uniform multiplicity; its failed audit is retained, and
the corrected v2 audit verifies the exact old/new diagnostic multiset.
Reports: `repair-inherited-sprite-event-asan-v2-*`.

The final external rebuild completes 191 PASS / one unchanged capability SKIP.
The four SpriteBatch and four C resource-lifecycle rows are the intended
FAIL-to-PASS deltas; all other prior outcomes remain unchanged. Six exact visual
references pass, peak memory is 4,301,119,488 bytes and OOM counters remain zero.
Accepted CNA commits: `39ef8ea2f` for the event lifetime and `65e69cc5e` for
SpriteBatch sequencing. Final unmarked demo/catalog closure is recorded with
the issue disposition.
