# ISSUE-089 initialization repair checkpoint

2026-10-20. Runtime CNA `5d32ca58b`, marker/evidence demos `4c03e74`.
ISSUE-089 is resolved. This is a local validated repair, not a globally green
campaign checkpoint: the existing C API RootDirectory contract delta remains.

## Repair and contract boundary

Registered intermediate serializers receive Initialize once before their payload
or child-scanning callbacks. Published workers share a persistent owner; nested and
concurrent documents retain separate operation state. Self/mutual lookup, workers
registered during writing and simultaneous first use have regression coverage.

CNA's explicit unique_ptr registration owns supplied instances; unlike XNA's
reflection-based constructor catalog, it cannot reconstruct fresh workers after
initialization failure. The deliberate extension boundary retains the failed owner
and exception, does not retry the same instance, and leaves raw lookup usable.
No fake factory/copy behavior or equivalence to XNA's fresh-owner recovery is claimed.
Genuine recovery evidence is retained rather than removed from the reference oracle.

## Native evidence

Genuine references21/21 and15/15 pass. Native initialization before:4PASS/17FAIL;
cycles1PASS/10FAIL, failure1PASS/5FAIL. Two original isolation cases retain ten
owner/init assertion failures with context-isolation controls otherwise passing.

Actual four production TUs pass21 initialization,11 cycle,6 failure controls and
four isolation/late/concurrent cases normally and under ASan/UBSan/leak detection.
Seven permanent native GTests and a separate fresh-process cold1/1 pass both modes.
Linked libraries are not all instrumented. Canonical79-step build passes with the
same two material-test warnings traced to their last compiled baseline. An initial
warning auditor used a build that did not compile those TUs; both audits are retained.

Canonical module1965=1961PASS/four unchangedSKIP; pipeline559=558PASS/one unchanged
SDK-fixtureSKIP (818.582s), exactly preceding552 plus seven regressions. Consumers
1412=1408PASS/fourSKIP; runtime241=238PASS/threeSKIP; default XML4PASS/two fixtureSKIP,
original XML6/6PASS. Exact case comparisons and four artifact hashes pass. Missing
freeze-manifest and oversized-filter launcher errors occurred before tests started;
logs are retained. Compact suite selection still audits every individual case.

## External checkpoint

Whole build105 printed steps and nine-step marker rebuild are warning-free. Both
146-case focused runs have65PASS/eight unchangedFAIL per renderer. The standalone
tool has20 ordered controls in both predecessor and current binaries; all20 now
pass. A helper's stale historical18 count caused an audit failure, corrected by
read-only comparison of the same logs against the preceding20-label inventory.
No behavior assertion or failure log was removed. Marker089 is removed only after
the actual tool passes; the marker-aware focus repeats the exact result.

Full719 sweeps:

| Renderer | PASS | Known failures | SKIP | Manual |
|---|---:|---:|---:|---:|
| SOFTWARE | 622 | 46 | 48 | 3 |
| OPENGLES3 | 661 | 48 | 7 | 3 |

65 intended status improvements versus campaign-start HEAD,67 versus Complete1128.
The additional predecessor comparison allows only089 to improve from087, with no
other status/check/marker/failure-multiset change. Both95 math sets, metadata/tool
checks and six exact visual references pass (zero differing pixels, zero tolerance).

Final serial CTest:735 inventory/732 enabled,684PASS/48 unchangedSKIP/three disabled,
650.59s. All735 outcomes match the preceding accepted inventory;319 artifact hashes
match before/after sweeps and CTest. The original-baseline comparison still reports
exactly the C API root-contract assertion replacing the old103 witness, and explicitly
sets globallyAccepted=false. Overall driver exits1 for that retained conflict; every
independent gate passes.048 remains the last globally accepted checkpoint; pending
039/045/043/103 are not closed by this repair.

Reports: `build/linux-full/reports/repair-intermediate-initialization-*`, particularly
`*-predecessor-comparison.json`, `*-ctest-final-audit.json`, `*-final-gates-summary.json`,
both full comparison files and frozen artifact manifests. External freeze is released
only after these completed audits. Native012 rebuild is tracked separately. No push/merge.
