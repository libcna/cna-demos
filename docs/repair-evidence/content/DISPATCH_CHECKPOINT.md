# ISSUE-087 dispatch repair checkpoint

Date: 2026-10-20. CNA repair `3d254dfa7`; demos correction/marker `8b69456`.
ISSUE-087 is resolved. This is a validated subsystem checkpoint, **not a globally
green campaign checkpoint**: the pre-existing C API RootDirectory contract delta
from039 remains explicitly reported, and103's original demo still exits early.

The concrete reader preserves disposable capability across type erasure. Typed,
raw and shared dispatch now agree on callback ordering, existing-reference identity,
boxed-value rejection, null/nullable arguments and explicit-reader dispatch.
No default-manager011 disposal or by-value resource-ownership repair is claimed.

## Contract and native evidence

Genuine XNA reference dispatch53/53 and value/nullable34/34 pass. Native original28
gives10PASS/18FAIL. Expanded36 exposed four nullable-null candidate regressions;
those failures and the interrupted first whole build are retained. Final36 pass
normally and with the actual ContentReader TU under ASan/UBSan/leak checking, also
when rerun against rebuilt libraries. Linked libraries were not all instrumented.

The final whole native build passes with the same51 existing warnings. Canonical
focus36/36 and full content module1965=1961PASS/four unchangedSKIP pass; the exact
comparison preserves1929 preceding outcomes plus36 additions and one explicitly
measured value-oracle rename. Pipeline552=551PASS/one unchangedSDK-fixtureSKIP,
932.000s. Consumers1412=1408PASS/fourSKIP; runtime241=238PASS/threeSKIP; default
XML4PASS/two missing-fixtureSKIP and original XML6/6PASS. Exact comparisons and all
four native artifact hashes pass. No native failure was waived.

## External evidence and oracle correction

Whole external build1043 printed steps and both nine-step demo rebuilds pass
warning-free. The initial build-log audit incorrectly assumed a fixed Ninja
terminal gap; retained evidence and corrected exact prelude/body accounting prove
the complete1043 steps. This was a helper error, not a compiler diagnostic waiver.

Initial146 focused runs show087's unchanged17 assertions passing on both renderers,
but existing_objects stops after5/13: its old expectation that index0 returns the
offered fallback contradicts genuine XNA. That failed run remains. The corrected
demo expects null AND an untouched offered object; it retains all13 behavior
controls, including reader counts, identity, tags and payload alignment.

Oracle and marker-updated146-case runs each give64PASS/nine unchangedFAIL per
renderer, exact comparisons errors[]. The087 marker is removed only after its
original17/17 assertions pass. No fixture, tolerance or unrelated expected failure
was weakened. This additional demo-oracle correction is not a new historical ID.

Full719 sweeps:

| Renderer | PASS | Known failures | SKIP | Manual |
|---|---:|---:|---:|---:|
| SOFTWARE | 621 | 47 | 48 | 3 |
| OPENGLES3 | 660 | 49 | 7 | 3 |

Both95 math comparisons have the same48 intended changes. Metadata/tool checks
and all six visual references pass with zero differing pixels at zero tolerance.
There are64 intended status improvements versus campaign-start HEAD,66 versus
Complete1128. The strict original-baseline audits still fail on exactly the
unchanged C API root-contract assertion delta. An additional predecessor audit
compares all719 statuses, check counts, markers and failure multisets against043:
only087 improves, no other delta, and `globallyAccepted` remains explicitly false.
The predecessor audit does not replace or waive the original-baseline audit.

Final isolated serial CTest:735 inventory,732 enabled,684PASS/48 unchangedSKIP/
three disabled,670.99s. Every735 outcome matches the prior accepted048 inventory;
all319 artifacts match before/after full regression and CTest. The overall driver
therefore exits1 for the deliberately retained original-baseline C root delta,
although CTest itself and the087-specific regression gate pass. The last globally
accepted checkpoint remains048.039/045/043/103 are not closed by this document.

Reports: `build/linux-full/reports/repair-content-dispatch-*`, notably
`*-predecessor-comparison.json`, `*-full-{SOFTWARE,OPENGLES3}-comparison.json`,
`*-ctest-final-audit.json`, `*-binaries.sha256` and `*-final-gates-summary.json`.
No pushes or merges. External087 freeze can now be released for the next validated
repair build; native089 work is separately tracked in the CNA handoff.
