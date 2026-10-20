# ISSUE-048 minimized reproduction — 2026-10-20

Status: native repair built and focused validated; complete regression pending.

The public ContentBuildResult contract promises effective processor parameters.
RunImportAndProcess correctly drops unknown/unconvertible parameters in
XnaCompatible mode before calling Process, but Build copies the original request
into result.parameters. This is a CNA extension contract, not a claim about an
XNA-managed build-result API.

Two native regression tests are prepared in ContentPipelineCoreTests.cpp:

- LenientResultMatchesTheParametersActuallyGivenToTheProcessor fails on the
  existing library for both an entirely rejected parameter set and a mixed set
  retaining a valid multiplier. Actual Process parameters, exact output bytes,
  warning counts and preservation of the caller's request all pass; only returned
  metadata is wrong.
- LenientManifestRetainsTheRequestedCacheIdentity passes: two different rejected
  requested values yield identical output bytes but distinct direct fingerprints,
  and each manifest retains its corresponding requested configuration.

The minimized run gives onePASS/oneFAIL,4ms. Report prefix:
`build/linux-full/reports/repair-effective-parameters-project-native-before`.
It links the actual frozen CNA dependency closure without rebuilding it.

The first exploratory Wall/Wextra/Werror build failed on pre-existing aggregate
initializers and a range-loop warning elsewhere in the reused core test file;
that log remains under `repair-effective-parameters-native-before-build.log`.
The successful reproduction compilation uses the project's actual warning policy
plus Werror. It is not claimed as a successful stronger-warning build.

Repair constraint: simply changing result.parameters would also change
MakeContentBuildManifestEntry, which currently copies that field. The CLI's
IsCurrentRoute compares persisted parameters with the requested item parameters.
Preserve requested configuration separately for cache identity while returning
the actual effective set. Existing/manual result construction must remain usable;
do not silently alter persisted manifest schema or cache fingerprints.

## Candidate repair and validation in progress

The coordinator now retains the validated effective parameters in its private
staged result and returns those in `ContentBuildResult::parameters`. An appended
optional `requestedParameters` preserves the original request for manifest/cache
identity; manually constructed results with it unset retain the previous fallback
to `parameters`. The persisted manifest schema and fingerprint algorithm do not
change. Additional native cases cover strict/default builds and manual results.

The existing whole native configuration rebuilt successfully (1164 steps), with
only the two previously recorded material-test nodiscard warnings. Native artifacts
are frozen in `repair-effective-parameters-native-frozen.sha256`. The 251-case
canonical pipeline gate passes250/one unchanged SKIP in60.971s, exit0, with
`differences: []`. The separate 552-case XNA pipeline gate is running.
The completed gate compares246 existing cases against the original11161-case CTest log
(245 PASS, one opt-in large-file SKIP) and requires all five subsequent additions
to pass: the four new effective-parameter tests and the earlier digest regression.
The original `repair-baseline-native-LastTest.log` is empty and is not used as
evidence. No external rebuild or issue closure is claimed yet. Reports:
`repair-effective-parameters-pipeline-core{.log,.xml,-audit.json}`.

The full native XNA pipeline gate now completes551PASS/one unchanged XACT SKIP
of552 in1008.149s, with every previous outcome unchanged and `differences: []`.
All four native hashes match afterward. Broader consumers/runtime/XML and their
final hash audit are running; the native binary remains frozen meanwhile.

External rebuild5276 completes346 steps without warnings. All56 focused executions
pass the strict comparison:25PASS/three remaining FAIL per renderer. Exactly the048
failed assertion disappears; all36 child checks remain in xna_pipeline_components,
which still fails only047. No marker is removed or assertion changed. All319
external artifacts are frozen in `repair-effective-parameters-binaries.sha256`.
Both95 math sweeps retain exact48 earlier improvements, metadata/tools and all six
pixel-exact references pass. Full719 renderer sweeps are running; final CTest and
hash audit remain pending. Reference to later039/045 source work must not be read
as inclusion in this frozen048 checkpoint.

Broader native checkpoint54092 now completes0:1412 consumers give1408PASS/four
unchanged SKIP203.003s, runtime241 gives238PASS/three unchanged SKIP11.121s, default
XML gives fourPASS/two same fixture SKIP. Original XML fixtures pass6/6 in14ms.
Exact comparisons have no differences and all four final hashes match. Only then
is the native freeze released for candidate039/045. External048 artifacts remain
frozen while full renderer95510 and subsequent final CTest/audit are pending.

Full renderer95510 now completes0: SOFTWARE617PASS/51FAIL/48SKIP/three manual,
OPENGLES3 656PASS/53FAIL/seven SKIP/three manual. Both strict719-demo comparisons
retain exactly60 improvements against the initial HEAD (62 against Complete1128),
with no unexpected differences. Only the048 child failure additionally disappears;
the parent retains047. The GLES font screenshot was also visually inspected.
Final isolated serial CTest59450 is running after all319 pre-test hashes passed.
No closure or external freeze release until that gate and post-test audit finish.
