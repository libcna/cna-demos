# Content manifest and cold-root repair candidates

2026-10-20. ISSUE-013 and014 remain open until canonical and external regression.
Before evidence is preserved in `PUBLIC_BOUNDARY_TRIAGE.md`,
`ManifestBookkeepingProbe.cpp` and `ColdContentRootProbe.cpp`.

## Current validation,13:59 UTC

14:01 update: corrected146 focus completes0,68PASS/five unchangedFAIL per renderer,
exact audit errors[]; all319 frozen external hashes match. Additive demo committed
1f8ae67. Full combined gates24685 run; math95 per renderer and six exact visuals
already pass, both719 and CTest pending. No closure yet.

Runtime/tests committed as CNA bbe2bda7b. Canonical1141-step build passes with45
exactly prior warnings; module1993=1989PASS/four unchangedSKIP, pipeline562=561PASS/
one unchangedSKIP (858.214s), consumers1412=1408/4,runtime241=238/3,XML4/2 and6/6,
all six link-footprint tests pass. Four native artifacts and three sources match.

External1183-step build passes with zero warnings and all eight source hashes exact.
Strict external manifest16/16 and cold-root12/12 both pass against rebuilt libcna.so.
C API354 focus exactly matches012, including all25 original C foundations controls;
the documented stored-codec C ABI regression is gone, with no oracle relaxation.
The original unrelated C RootDirectory residual remains explicit.

Content focus's first audit incorrectly expected passing DemoContext labels in the
log; those are counted, not printed. The corrected audit compares the original
manifest source byte-for-byte after removing just the new four-check block and
comment correction, alongside runtime13->17 and exact failure/status comparisons.
An initial correction included one extra newline and refused before running tests;
both failed audit logs remain. Corrected focus78820 is running. No product failure
has been waived; full regression/closure is still pending.

## ISSUE-013

RefreshContentManifest excludes exactly the pipeline lock and build-manifest names
at every depth, and prunes exactly `.cna-contentproj` directories. The public build
manifest constant is reused. Ordinary hidden files, unrelated lock extensions,
prefix lookalikes, an ordinary file named `.cna-contentproj` and assets beneath
a directory named `.cna-content.lock` remain discoverable. Two native regressions
protect this distinction and the existing cached-snapshot/explicit-refresh contract.
The public manifest comment documents the reserved namespaces.

Actual ContentManager.cpp plus the strict original witness passes16/16; all eight
native manifest cases pass. Both normal and ASan/UBSan/leak variants pass, first
alone and again with the014 candidate. Native fixtures use canonical project flags;
the standalone witness uses Wall/Wextra/Werror. All exact audits errors[].
Reports `repair-content-manifest-bookkeeping-candidate{,-sanitizer}-project-policy-*`
and `*-combined-project-policy-*`; all four frozen boundary native hashes match.

## ISSUE-014

For a relative, nonempty root with no title override, ContentManager now obtains
the lazy platform filesystem regardless of prior ambient platform initialization.
Absolute, empty and invalid roots do not need to initialize the platform. Explicit
title overrides, the existing missing-title-directory cwd fallback and Android's
packaged-asset namespace remain unchanged. There is no content-to-runtime dependency
or new public API. This is an ordering fix, not a redesign of root resolution.

The original fresh-process Linux witness improves9PASS/threeFAIL to12/12PASS.
A new native fresh-process case independently obtains the executable directory
from an owned, non-ambient platform, stages title/cwd decoys, proves an absolute
root needs no ambient initialization, and checks the first relative scan, authored
root, subsequent refresh, explicit title override and snapshot retention.
Its SDL desktop/death-test guard states other configurations explicitly; Linux SDL
executes it, not skips it. No other-platform runtime validation is claimed.

Actual ContentManager.cpp/public witness/native case pass normally and with
ASan/UBSan/leak detection. Linked dependency libraries are not all instrumented.
Four native hashes match before/after. Reports
`repair-content-cold-root-candidate{,-sanitizer}-project-policy-*`.

## Canonical gates pending

The completed boundary-native freeze was released only after a final four-hash
check. Three compiled sources are frozen in `repair-content-roots-source.sha256`.
Canonical build12705 runs with two workers while the separate boundary external
regression92630 uses319 frozen binaries/one worker. Do not rebuild that external
tree until its full gates finish. Next native inventories: module1993 (1990+three),
pipeline562 unchanged, consumers1412/runtime241/XML/link exact. No overlapping
SystemLink suites. No013/014 demo change, marker removal, commit or closure yet.
The old private strict-stream flag comment is corrected while its header is touched;
that comment-only clarification does not change the already validated012 behavior.

Canonical build now completes1141 printed steps with45 exactly matched historical
warnings and zero new diagnostics. Three source hashes match; four native artifacts
are frozen. Module1993/pipeline562/consumers gates run independently; the three new
focused regressions already pass. The preceding boundary external sweep failed on
the documented C ABI None exception (see `CNB_C_API_COMPATIBILITY.md`), not on these
root candidates, which were not in its binaries. Adapter fix is CNAc57ecab8c.

The manifest demo adds four controls to its original thirteen: cached snapshot after
staging files, exact bookkeeping exclusion, reserved subtree exclusion, and exact
retention of legitimate hidden/prefix assets. No original control or tolerance is
changed. External content-roots validation will include the four preceding boundary
repairs and C adapter; only105 status should improve from the completed012 checkpoint,
with manifest checks13->17. The original C API RootDirectory residual stays explicit.
