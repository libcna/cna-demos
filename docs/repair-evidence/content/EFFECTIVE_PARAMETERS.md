# ISSUE-048 minimized reproduction — 2026-10-20

Status: confirmed open; no runtime repair yet.

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
