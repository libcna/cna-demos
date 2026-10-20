# Color checkpoint — 2026-10-20

Closes132 as FIXED_VALIDATED. All155 historical IDs remain:58 final/97 pending
(26 runtime findings,one verified duplicate,23 documentation corrections,seven
pre-campaign fixes,one demonstrated demo-expectation correction).

Runtime `1ad0a3916`; demo focused/marker commit `e7eafac`. The subsequent component
and launch repairs `031693a9f`/`b086814f9` are NOT in these frozen external binaries.
Neither are the newly prepared storage/Sharp Runtime regressions.

## Contract and native validation

Genuine XNA and repaired native probes pass1,782/1,782. The unchanged implementation
had162 mismatches. Lerp quantizes its factor to65536 with ties-to-even and NaN-to-zero;
Multiply bounds its scaled factor before unsigned channel products/shifts. This
preserves finite quantization as well as the original nonfinite witnesses. Zero
channels stay zero even at positive infinity. Constructors/packing/API/layout stay
unchanged; the old native NaN-Lerp alpha expectation was corrected exactly from
0 to255 based on genuine evidence, not relaxed.

Native math966PASS/two unchanged fixture skips; registered Color/packing70/70
(11.07s); exact broad consumers1,408PASS/four unchanged skips(1,412total,282.277s).
The earlier wrong-cwd fixture failures remain recorded; the unchanged suite passed
from the required CNA cwd. Actual Color.cpp and both Color test translation units
pass64/64 ASan/UBSan/float-cast-overflow/leak checks. Native/external builds are
warning-free. No lifetime sanitizer result from another subsystem is substituted.

Actual rebuilt C ABI ColorSmoke passes all six stages, including four original
controls. Six focused demos per renderer pass; the repaired two retain their exact
7/9 checks. Only then were their two knownIssue/restriction markers removed.

## Complete external checkpoint

Both95-demo math sweeps:93PASS/two unchanged reserved131 failures. Full719-ID sweeps:

| Renderer | PASS | Expected FAIL | SKIP | Manual/not run |
|---|---:|---:|---:|---:|
| SOFTWARE |605|63|48|3|
| OPENGLES3 |644|65|7|3|

Both comparisons accept exactly48 cumulative intended changes against the campaign
current-HEAD baseline, with `differences: []` for statuses, knownIssue, assertion
counts and failed-assertion multisets. Compared with Complete1128,50 expected demo
failures now pass; two improvements predate this campaign. These are demo deltas,
not issue totals. Owner-reserved131 remains open.

Metadata719/0; tool suites7+4+5 pass. All six SOFTWARE/OPENGLES3 visual references
are pixel-exact (zero differing pixels and channel delta0). The representative GLES
texture capture was also inspected. No additional renderer/platform claim.

After both full smokes completed, isolated full CTest accepted all732 enabled tests:
zero failures,48 unchanged skips/three disabled,983.90s. Network discovery passes;
no overlapping campaign SystemLink run, retry, timeout increase, failure whitelist
or assertion suppression was used.

SHA256 verification confirms the frozen executable and actual C++/C libraries
remained unchanged after the complete checkpoint. Raw evidence is retained under
`build/linux-full/reports/repair-color-*`, including comparisons, original failures,
`repair-color-demo-ctest.log` and `repair-color-frozen-final.log`. The external
freeze is released for the separate lifecycle build. No push or merge.
