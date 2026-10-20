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
