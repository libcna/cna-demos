# ISSUE-070 — PBR material clone preparation

## Current result (supersedes preparation below)

**Resolved 2026-10-21:** CNA `1c8bbb6b83bab2fc3b15bf2370b426226fd87204`.
Eight assignments in two copy constructors preserve all four omitted fields;
the same eight regression bodies are registered natively. No C adapter, public
API, renderer or assertion workaround. Native/full and sanitizer gates below
are accepted. External38861 completed48/48 six-renderer witnesses and112 exact
preceding audio/video outcomes; errors0/frozen unchanged. Existing unrelated
C video failures remain unaccepted, not converted to expected failures.

Stock48995:54PASS/12unchanged rendererSKIP over66 cases; only the two original
C070 scenarios improve. All four formerly failing checks now pass, every
assertion label/count and unrelated outcome remains. Visual14686: four original
shadow images800x480 have zero differing pixels against the accepted102 baseline;
OPENGLES3 images visually inspected, OPENGL33 counterparts byte-identical.
This is shadow regression coverage, not a clone/BRDF image claim.

Marker gate44634: only070 metadata restriction/knownIssue removed after accepted
repair. Incremental build6,929ms/zero diagnostics, original C070 SOFTWARE188ms
and OPENGLES3358ms PASS, four focused/catalog CTests PASS13,767ms, audit41ms.
Exact catalog comparison proves all other entries unchanged; source/main.c and
all non-catalog library/binary hashes retained. No assertion removed or relaxed.
Peaks: external4,301,479,936/high812; stock192,016,384; visual194,105,344;
marker3,006,173,184. All hard-limit/OOM/kill events zero, other gates' events zero.
Reports: repair-pbr-clone-{external,demos-after,visual-after,final}-audit.json
under build/linux-full/reports. All historical reproduction logs retained.

Sanitizer12049 completed35/35PASS (original8 clone+20 boundary+7 existing shadow
receiver cases), actual PBR/runtime instrumentation, no findings/suppressions.
14,068ms libraries/9,239ms strict fixture/172ms run/22ms audit;
peak816,594,944/events0, frozen hashes unchanged. External38861 build completed
74,357ms/zero diagnostics; all8 clone witnesses PASS on each actual SOFTWARE,
OPENGLES3,OPENGL33,SDL_GPU,HEADLESS,STUB.112 original audio/video parity still
running; stock66/visual/marker gates pending. No070 closure or product commit.

Native72714 completed0: build22,793ms/zero diagnostics, focused8PASS320ms,
graphics2898PASS/73SKIP44,459ms; full10875PASS/101sameSKIP1,245,153ms. Every
previous10968 outcome unchanged, exactly8newPASS, comparison127ms. Frozen
hashes unchanged;peak4,296,978,432/high1602/max0/OOM0. Sanitizer/external/stock/
visual/marker gates still pending; no product commit or070 closure yet.

Baseline67425 reproduced6FAIL/2PASS across both effect families exactly as
predicted: missing authored state/draw parameters, all alpha modes/dormant
cutoffs and independent clone values fail; default controls pass. Strict
build2,836ms, run363ms, audit23ms;peak447,148,032 bytes/events0 and frozen hashes
unchanged. CNA cbe9b9388, demos03ed687, Runtime c26614e85.

Minimal candidate copies the missing four fields in each constructor (eight
assignments), without header/API/C-adapter/renderer changes. Original eight
witness bodies now registered natively; only the stale preparation comment
was corrected, with original hash reconstructed and checked by the native
driver. Native72714 graphics/full10968+8 parity now runs under the established
single-worker/private GPU/network/6GiB/no-swap boundary. Sources frozen.
Sanitizer, external six-renderer clone witnesses, C070 improvement, other
stock/video parity and original shadow visuals remain pending. No closure,
marker removal or product commit yet. Earlier unrun statements are historical.

The original C reproduction has now been reverified by the ISSUE-102 stock66
BEFORE gate on SOFTWARE and OPENGLES3: exactly four assertions fail (vertex
color, alpha mode, cutoff, double-sided), while the metallic/dielectric and
output-encoding controls and clone Apply pass. Sources/assertions unchanged;
reports repair-shadow-demos-before-*-c_api.graphics.pbr_effect_material_state.*.
The eight-case C++ fixture remains UNCOMPILED/UNRUN. Its prepared standalone
driver repair-pbr-clone-before.mjs requires all shadow external/stock/visual
gates accepted first and pins their source/library hashes before compilation.
No070 product edits yet.

2026-10-21, read-only source review while ISSUE-102 full native gate45760 runs.
This is **not a reproduction or repair yet**. PbrCloneMaterialStateTests.cpp is
UNCOMPILED/UNRUN, outside the registered native source tree. No070 production
source, C route, demo assertion or marker changed.

Both current PbrEffect/SkinnedPbrEffect copy constructors still explicitly copy
core material fields and encodeOutputToSrgb_, but omit VertexColorEnabledEXT,
alphaMode_, alphaCutoff_ and doubleSided_. The existing native Clone tests set
other fields but do not set/assert these four. Both Clone implementations call
these private copy constructors, so the regression fixture uses the public
Clone API; it does not expose or directly call private constructors.

The public C route GetPbrEffect accepts either effect family. The existing
c_api.graphics.pbr_effect_material_state demo destroys its source and checks
exactly these four properties on the clone; retain those assertions. Its
current actual outcome will be captured by the independent102 stock-effect66
regression gate before any070 implementation change.

Prepared typed fixture8 cases (four per family): nondefault state and CPU
draw parameters after source destruction; all three alpha modes with two exact
cutoffs including dormant cutoffs; independent source/clone mutation; unchanged
defaults. Core metallic/output-encoding values serve as already-working
controls. Expected6FAIL/2PASS is only a hypothesis pending execution. No image,
native GPU shader or sanitizer result is claimed by preparation.

This is a CNA extension, not an XNA PBR contract. The narrow proposed repair is
copying the omitted four values in each constructor, not redesigning cloning or
adding new C adapters. Review any separately observed clone behavior against its
own contract rather than opportunistically changing other fields in this batch.
After102 acceptance: reproduce070, preserve the C baseline, add native coverage,
make the local copy repair, run affected/full/native/external/sanitizer gates,
then update the permanent issue status and marker only after acceptance.
