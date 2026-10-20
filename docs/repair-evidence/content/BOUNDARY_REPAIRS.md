# Public content boundary repair candidates

2026-10-20. Four small, separate root causes in the content subsystem. No issue
is closed here. Canonical native/external regression remains pending. Before
sources/results are preserved in `PUBLIC_BOUNDARY_TRIAGE.md`; these CNA extension
contracts are documented by CNA itself, not fabricated genuine-XNA observations.

## ISSUE-110: direct uncompressed chunk validation

The common declared-size ceiling now runs before the None branch, which also
checks exact stored/declaration equality before copying. Compressed codec paths
and container validation are unchanged. Small inputs cover zero/nonzero mismatch,
short/long/UINT64_MAX declarations, exact ceilings and the original4096/4097/limit1
case; no giant allocation is requested by the None path.

Actual `CnbChunkCompression.cpp` plus strict public/native fixtures pass11/11 public
controls and15/15 native compression tests (12 previous plus three new), both
normally and with ASan/UBSan/leak detection. Four frozen012 native hashes match.
Reports: `repair-content-chunk-limits-candidate{,-sanitizer}-*`.

## ISSUE-105: named-only importer and processor resolution

The common route resolver accepts an explicit default-eligibility predicate.
Importer/processor callers exclude named-only candidates before choosing a sole
default or constructing no-route/ambiguity diagnostics. Explicit name validation
and writer resolution remain unchanged; HasImporterForSource remains discovery,
including named-only registrations. This repairs both paths once, not audio-only.

Actual `ContentPipeline.cpp` passes25/25 strict public controls and23/23 native core
pipeline tests (21 previous plus two new), normally and with ASan/UBSan/leak checks.
First attempts passed the public25 but failed compilation of the reused legacy
native suite under stronger-than-project Werror flags: pre-existing missing-field
initializers and a range-loop temporary. Original failed logs remain. The native
suite is compiled with its exact canonical compile_commands flags, while the new
public witness retains Wall/Wextra/Werror; no diagnostic suppression or source
cleanup was introduced. Four frozen012 native hashes match.
Reports: `repair-content-named-routing-candidate{,-sanitizer}-project-policy-*`;
original no-suffix failures are retained separately.

## ISSUE-108: direct font-region validation

ExpandCharacterRegions now rejects an inverted region before iterating it, using
the already documented runtime_error contract. Empty direct input, deduplication,
authored-input preservation, fallback inclusion, inclusive U+FFFF and exact8192
glyph limits remain protected. XML parser behavior and rasterization are unchanged.

Actual `SpriteFontContentPipeline.cpp` passes12/12 strict public controls and8/8
native FontDescription tests (six previous plus two new), normally and with
ASan/UBSan/leak checks. Four frozen012 native hashes match.
Reports: `repair-content-font-regions-candidate{,-sanitizer}-*`.

## ISSUE-106: registry-affecting option diagnostics

The pre-built compiler overload's refusal now names all seven registry-affecting
option families, retaining status2 and the factory guidance. Public equality and
overload comments describe services, font directories and build configuration;
coordinator-owned container options remain supported. Neither refusal timing nor
option comparison changes. The existing positive FX case and a new seven-option
native case protect no-freeze/no-publication behavior.

Actual `tools/content/content.cpp` passes21/21 public controls and three selected
native pre-built/factory tests normally and with ASan/UBSan/leak checks. The reused
native fixture uses its canonical flags; the new public probe is Wall/Wextra/Werror.
Before21 controls gave16PASS/five missing-option-nameFAIL. Reports:
`repair-content-compiler-options-candidate{,-sanitizer}-project-policy-*`.

## Validation limits and next gates

Only the named implementation TU and its fixture are instrumented in each focused
sanitizer executable; linked dependency libraries are not all instrumented. These
candidate runs did not rebuild either canonical tree. The independent012 full
external regression uses319 frozen artifacts and must finish before any external
rebuild. No SystemLink suites may overlap. The subsequent canonical native build
now completes84 printed steps, zero warnings, exact nine source hashes; four new
native artifacts are frozen. Compare1990 content cases (1985+five),562
pipeline cases (559+three), consumers/runtime/XML and applicable link checks.
Commit each root cause separately after validation. Preserve the unrelated C API
RootDirectory conflict and all existing expected failures; no markers for this
batch have been removed yet.
