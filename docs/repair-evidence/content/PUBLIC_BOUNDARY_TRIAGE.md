# Remaining public content boundaries

2026-10-20. Read-only triage and minimized before witnesses while the separate
ISSUE-012 canonical build runs. No production changes for these findings yet.
Probes link the unchanged external089 `build/linux-full/CNA/libcna.so` and its
existing link closure, not the native tree being rebuilt. Strict C++23/Werror.
Later105/108 probes instead link the completed, frozen canonical012 native
artifacts: neither target implementation changed in012. All four native hashes
match before and after each probe. These results do not depend on the concurrent
external rebuild.

## ISSUE-013: runtime manifest bookkeeping

`ManifestBookkeepingProbe.cpp`:16 checks,8PASS/8FAIL. Exact reserved lock/build
manifest files at root and nested roots appear as assets. The tool-generated
`.cna-contentproj` subtree appears too. A refresh introduces more bookkeeping.
Controls preserve real assets, unrelated hidden files, ordinary `.lock` extensions,
similar filename/directory prefixes, cached snapshots and explicit refresh.

The current full demo output independently lists the same bookkeeping. Source
`ContentManager::RefreshContentManifest` enumerates every regular file and groups
extensions with no internal-path exclusion. Pipeline's public manifest constant,
`docs/content-pipeline.md` lock contract and `tools/content/content.cpp` generated
intermediate path establish which names belong to CNA. Native manifest tests cover
grouping, malformed XNB and refresh, but do not protect this boundary. Do not solve
it by dropping every hidden file, JSON file or lock extension. The original demo
is a passing diagnostic; do not turn an unrelated assertion into a known failure.

Reports: `repair-content-manifest-before-{build.log,audit.json}` and `.log`.

## ISSUE-110: direct chunk-size validation

`ChunkCompressionLimitsProbe.cpp`:11 checks,4PASS/7FAIL. The None branch copies
before checking either exact size or the configured ceiling. Small bounded inputs
prove shorter/larger declarations, zero/nonzero disagreement, UINT64_MAX declaration
and the historical4096/4097/limit1 case are accepted incorrectly. Controls preserve
empty/exact chunks, independent result storage and contextual exact ContentLoadException
for an unknown codec. No giant buffer is allocated by the probe.

The public `CnbChunkCompression.hpp` contract requires exact size and pre-allocation
ceiling enforcement for this helper. `CnbDocument` validates its own uncompressed
table entries; ordinary container parsing is not claimed unbounded. Existing native
compression tests exercise compressed size limits and container totals, not the
direct None branch. This is a runtime validation defect, not an XNA-specific API.

Reports: `repair-content-chunk-limits-before-{build.log,audit.json}` and `.log`.

## ISSUE-014: cold content-root ordering

`ColdContentRootProbe.cpp` reproduces this independently in a fresh Linux process:
12 controls,9PASS/threeFAIL, no Game. It stages different manifest-only assets under
the executable directory and an unrelated private working directory without first
calling CNA's platform/title APIs. The first relative-root scan uses the working
directory; querying TitleLocation and explicitly refreshing switches the same
manager to the executable directory. Authored RootDirectory, snapshot caching,
stable later refresh and an explicit absolute-root control remain correct.

The fixture's Linux `/proc/self/exe` lookup only stages data without warming CNA.
Uniquely owned directories are removed after restoring the working directory; no
user content is touched. Strict warning-free build and exact audit pass against
the frozen boundary-candidate native artifacts (title/root code unchanged), with
all four hashes identical before/after. Reports `repair-content-cold-root-before-*`.
No Windows/macOS/Android or genuine-XNA execution is claimed by this probe.

Current TitleRootedDirectory consults the platform base only when HasCurrentPlatform
is already true, whereas TitleLocation explicitly obtains the lazy platform's
filesystem. The recorded public title/content-root contract requires a stable title
root. Repair at the lower content/platform layer, not through a new dependency on
the upper runtime TitleLocation class; preserve explicit roots/overrides and the
separate Android packaged namespace. No014 production patch during the freeze.

## ISSUE-106: pre-built compiler diagnostics

`CompilerOptionDiagnosticProbe.cpp`:21 controls,16PASS/fiveFAIL against frozen
canonical012 (compiler implementation unchanged). All seven registry-affecting
options refuse the pre-built overload with status2, refer to the factory overload,
leave the registry mutable and create no output directory. Only the two FX options
are named by the diagnostic; XMA executable/launcher/argument, font directory and
build configuration are missing. No external compiler, encoder or font service runs.
Strict warning-free build, exact audit and four native pre/post hashes pass.
Reports `repair-content-compiler-options-before-{build.log,audit.json}` and `.log`.

The runtime compares all seven fields against defaults but uses the old FX-only
message; public equality/overload comments are correspondingly stale. Preserve
early refusal and the factory guidance, update the diagnostic and documentation,
and retain the demo's service-not-entered controls when replacing its historical
observation. This is not an unsupported external-service blocker.

## ISSUE-105: named-only routing

Current089 focused `content.sound_effect.pipeline_components` retains precisely
the named-only exclusion failure. Current ResolveImporter and ResolveProcessor
filter only when the route has more than one candidate, then fall back to the
unfiltered resolver unless exactly one ordinary candidate remains. Both sole
named-only and all-named-only sets need explicit negative tests; preserve explicit
selection, one ordinary candidate among named-only entries and ordinary ambiguity.
HasImporterForSource is a separate discovery predicate, not default-route resolution.

`NamedOnlyRoutingProbe.cpp` now independently verifies25 controls:19PASS/sixFAIL.
Both importer and processor fail the sole-named-only exclusion, report a false
ambiguity for an all-named-only set, and include ineligible names in mixed-set
ambiguity diagnostics. Positive controls preserve explicit selection, rejection of
unknown/wrong-route names, sole ordinary selection, ordinary ambiguity, stable
candidate ordering and case-insensitive discovery (including named-only discovery).
Reports `repair-content-named-routing-before-{build.log,audit.json}` and `.log`.

## ISSUE-108: programmatic font regions

Current089 standalone font pipeline output explicitly confirms an inverted
programmatic range disappears. ExpandCharacterRegions enters an unsigned ascending
loop without rejecting end<start; the XML parser already rejects that input. Public
declaration promises std::runtime_error. Preserve sorted/deduplicated output, default
character inclusion and glyph-count ceilings when validating the direct public input.
No genuine-XNA claim is needed for this CNA extension contract.

`FontRegionBoundaryProbe.cpp` verifies12 controls:8PASS/fourFAIL. Sole, leading,
trailing and full-endpoint inverted ranges are all silently accepted; an outside
fallback can mask the invalid input. Positive controls preserve empty direct input,
sorted deduplication, no input mutation, fallback inclusion/deduplication, inclusive
U+FFFF termination and the exact8192-glyph ceiling/oversize exception. No font file,
rasterizer, renderer or device is required. Reports
`repair-content-font-regions-before-{build.log,audit.json}` and `.log`.

012 compilation/source freeze remains intact. These issues remain open; no demo
markers/assertions or production code have been changed for them.
