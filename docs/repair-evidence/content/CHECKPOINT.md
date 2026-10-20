# Content checkpoint — 2026-10-20

Status: FIXED_VALIDATED for146,147 and149. All155 historical IDs are retained:
68 final,87 pending (36 runtime findings, one verified duplicate,23 documentation
corrections, seven pre-campaign fixes, one demonstrated demo-expectation defect).
The149 runtime finding also contains a corrected historical direct-read expectation;
it is not counted as a separate historical issue.

## Revisions and contracts

CNA `7c4431fdf` repairs NamedValueDictionary mutation validation and setter hooks;
`9dd0826bf` repairs intermediate null reads and exception propagation. Sharp Runtime
`af1b0bea` repairs XML writer/LINQ NUL character-data exception categories;
`99c2f79c` is its final comment correction. Demo `9bb033e` corrects the direct
null-root read oracle; `71e6992` updates only markers validated by focused execution.
Subsequent090 compiler and088 registry work is NOT in these frozen external binaries.

Genuine XNA dictionary14/14 and protected-hook7/7 references pass. XML writer21/21
and direct LINQ74/74 pass on both Framework4 and .NET8. Direct intermediate16/16
confirms null roots deserialize successfully; only subsequent serialization raises
ArgumentNullException(value). Value-type null members raise NullReferenceException.
The original combined-read/write golden corpus is preserved. Optional
CheckCharacters=false NUL behavior is explicitly not claimed implemented.

## Native and dependency validation

The pre-fix504-test content baseline has497PASS/one existing XACT fixture SKIP/
six new FAIL. Post-fix504 has503PASS/the same SKIP,999.450s: exactly those six
failures become PASS, with identical test identities and no other change.
Focused19/19 passes. Exact broader consumers1408PASS/four same SKIP of1412;
runtime238PASS/three same SKIP of241. XML consumers pass6/6 with original ShipGame
fixtures; the first4PASS/two missing-fixture SKIP invocation remains recorded.

Actual OpaqueDataDictionary.cpp, IntermediateReader.cpp and IntermediateSerializer.cpp
plus nine selected native regression cases pass ASan/UBSan/leak detection. This
is not whole-library or whole-corpus instrumentation. Initial harness quoting and
static-library-link failures are retained; the final link uses CnaTests' actual
static content-pipeline closure.

Sharp's full gate passes18,146/18,146 across41 executables, zero failures/skips,
with the original sample fixtures and isolated Yacht service. Its separate watcher
test barrier correction is `e0601cd2`; two unrelated stress-only rename failures
remain documented and are not claimed fixed. The Sharp build and both external
content builds are warning-free. The initial native build retains two pre-existing
material-test nodiscard warnings; its final small rebuild is warning-free.

## External regression

All28 custom-content demos per renderer give exactly23PASS/five unchanged FAIL:
56 focused runs with exact parent metadata, child assertion counts and failure
multisets checked. Dictionary15/15 and text12/12 now pass. Intermediate serializer
keeps36 checks: exactly two149 failures disappear, four148 failures remain.
Other expected failures and all behavior assertions are preserved.

| Renderer | PASS | Expected FAIL | SKIP | Manual/not run |
|---|---:|---:|---:|---:|
| SOFTWARE |614|54|48|3|
| OPENGLES3 |653|56|7|3|

Both full719 strict comparisons have57 intended demo-status improvements versus
campaign HEAD,59 versus Complete1128, plus the two149 assertion improvements.
Both have `differences: []`: no whitelist, tolerance expansion or retry hides any
delta. Both95-demo math sweeps retain93PASS/two owner-reserved131FAIL and exact48
math-related improvements. Metadata719/zero errors, tool suites7+4+5 and all six
pixel-exact visual references pass. The GLES font image was also inspected.

After both smokes, isolated serial CTest passes all732 enabled tests with zero
unexpected failures, the same48 skips and three disabled tests,747.99s. There was
no concurrent campaign SystemLink suite.146/147 tests no longer have WILL_FAIL;
the149 parent correctly retains its148 expected failure. All319 executable/DSO/tool
hashes pass both before and after CTest, including statically linked children.

Reports: `build/linux-full/reports/repair-content-*`; the authoritative319-entry
manifest is `repair-content-binaries.sha256`, not the empty failed initial manifest.
The external freeze is released after this audit. No additional renderer/platform,
unavailable hardware or online-service validation is claimed. No push or merge.
See [chronological evidence and retained failures](README.md).
