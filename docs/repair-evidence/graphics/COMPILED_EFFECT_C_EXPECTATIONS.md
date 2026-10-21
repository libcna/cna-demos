# C compiled-effect expectations exposed by ISSUE-003 validation

## Validated correction (2026-10-22)

The complete full-smoke logs retain the original32PASS/2FAIL on both SOFTWARE and
OPENGLES3. The demo now expects false for the source-program query on a bytecode
Effect and INVALID_STATE for a non-current pass. The latter additionally verifies
the STATE category and exact System.InvalidOperationException diagnostic before
any later API call replaces it. All34 original checks and other controls remain.
No native or C API runtime behavior changed. The C header and EFFECTS.md now make
the source-program/bytecode-runtime distinction and pass-refusal result explicit.
Validation25303 completed: actual C demo target build224911ms/no diagnostics;
SOFTWARE34PASS71ms, OPENGLES334PASS257ms, compiled-off SDL_GPU27PASS249ms,
and11 existing native EffectApply checks PASS1103ms. Both compiled-on logs
contain the exact System.InvalidOperationException / INVALID_STATE / STATE
diagnostic. No frozen-input drift; peak1394274304 bytes, all memory events zero.
Raw logs and audit: build/linux-full/reports/repair-compiled-c-expectations-*.
The original full-smoke observations remain unchanged; this is focused validation,
not a new full-smoke result. ISSUE-011/082 failures are not waived.

The matching CNA documentation commit is `89cd0b202`
(`Clarify C Effect source-program and pass-state contracts`); no CNA runtime fix
was necessary for these two expectation corrections.

## Historical observation (2026-10-22)

The ongoing full SOFTWARE smoke at CNA `e582aa93d` / demos `4162e76`, including
the uncommitted ISSUE-003 candidate and preserved ISSUE-011/082 candidates,
records two failing assertions in `c_api.graphics.effect_core`:

- `the compiled Effect reports its device, runtime, live renderer and non-stock concrete identity`
- `switching current technique rejects the old owned pass and enables the new P1 pass`

The child completes34 checks (32 pass, two fail); the preceding compiled-off
checkpoint completed27 passing checks. Parent standalone reports do not expose
that child check-count difference, so inspect the raw child logs as well.

Raw evidence is `build/linux-full/reports/repair-sprite-binding-full-parts/`
`SOFTWARE/c_api.log`. The preceding full staging-lifecycle checkpoint passed the
demo with compiled effects disabled. Its log explicitly checks refusal of the
valid fixture, not compiled reflection or pass dispatch. The current configuration
enables SOFTWARE/EasyGL compiled effects for the SpriteBatch regression tests.
Therefore this status delta must be investigated; it is not proof that the
SpriteBatch candidate regressed an already-executed compiled-effect path.

## Source-backed contract review

1. `cna/modules/graphics/include/Microsoft/Xna/Framework/Graphics/Effect.hpp`
   documents `GetEffectRendererPtr()` as the source-based effect program query,
   overridden by `ShaderEffect`; it may return null. The base `Effect.cpp`
   implementation returns null. The bytecode constructor separately owns
   `compiledRuntime_` and exposes it through `GetCompiledRuntimePtr()`.
   `modules/c-api/spec/Effects.toml` maps `cna_effect_has_renderer` directly to
   `GetEffectRendererPtr() != nullptr`, while `cna_effect_get_is_compiled_ext`
   reports the distinct compiled runtime. The C header's description omits the
   source-based qualification. An ordinary compiled bytecode `Effect` must thus
   report compiled=true and source-program-present=false, not both true.
   Do not manufacture an `IEffectRenderer` merely to satisfy the demo.

2. Genuine local XNA `EffectPass.cs`, from
   `xna4-decomp/reference/xna4/decompiled/windows/Microsoft.Xna.Framework.Graphics/`
   `Microsoft.Xna.Framework.Graphics/`, checks disposal, then compares the owning
   technique with `CurrentTechnique`, and throws `InvalidOperationException`
   before `OnApply` on a mismatch. CNA's `EffectPass.cpp` does the same.
   `EffectTests.cpp` already protects this exact exception and the absence of
   `OnApply` effects. `modules/c-api/src/CnaCApiDetail.hpp` translates that type
   to `CNA_RESULT_INVALID_STATE`; the generated `cna_effect_pass_apply` route
   uses this exception barrier. The demo incorrectly expects INVALID_ARGUMENT.
   This is genuine-XNA source evidence, not a new managed GPU execution.

## Original validation requirements (fulfilled above)

Wait for the frozen full-smoke run to terminate. Preserve its original failures.
Then correct only the two proven demo expectations and clarify the C header's
source-program wording / non-current-pass result. Keep the compiled construction,
reflection counts, clone independence, current-pass success and restored-technique
controls. Add exact exception-type diagnostics for the refused old pass so the
new result expectation cannot hide an unrelated refusal. Run the unchanged-before
and corrected-after demo on enabled SOFTWARE and OPENGLES3, and on compiled-off
SDL_GPU as the refusal control. Verify the complete child check counts and labels,
not only the parent standalone status. No correction or post-correction runtime
pass is claimed by this note.

Separate failures remain independent: the C content-manager Effect lifetime
failure is the previously recorded ISSUE-011 owner-contract question; the C
resource-disposal callback failure is ISSUE-082. Neither is waived here.
