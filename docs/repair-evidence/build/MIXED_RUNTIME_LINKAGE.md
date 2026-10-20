# ISSUE-016: one runtime in mixed shared/static consumers

## Repair checkpoint (2026-10-20)

CNA repair: `470b12a5d4b50dbaa01038ba994b8524786b219b`.
ISSUE-016 is **FIXED_VALIDATED**. The external full regression has completed.
No behavioral assertion or known-issue marker has been changed.

## Reproduction and minimization

Removing only the first explicit `libcna.so` file from the real 720-object browser
link command reproduces a shutdown double-free (`SIGABRT`). The resulting
executable defines 6,461 strong symbols also present in the shared runtime,
including 1,725 Graphics symbols; the workaround control defines zero duplicates
and exits successfully. `--version` reproduces without asset lookup, a Game,
graphics device or online service. The earlier relocated `--validate` invocation
also reported missing assets and is not used as the clean shutdown witness.

The permanent single-translation-unit [probe](MixedRuntimeLifetimeProbe.cpp)
retains an uncalled GamerServices factory and observes `SamplerState::PointWrap`.
It constructs no Game, device or service. Without the first-DSO workaround it
aborts and defines 1,213 strong Graphics symbols; the control exits zero and
defines none. The smaller generic fixtures under `mixed-runtime/` did **not**
reproduce, including reversed order, a cyclic closure, old CMake policy and an
auxiliary predecessor. They are retained negative evidence, not proof of safety.

Reports under `build/linux-full/reports/`:
`repair-mixed-runtime-actual-before-*`,
`repair-mixed-runtime-version-audit.json`,
`repair-mixed-runtime-minimal-before-*`. An initial bounded `nm` capture failed;
the retained file-backed symbol audit supersedes it, without hiding that failure.

## Root cause and repair

Static sidecars (GamerServices, Net and ContentPipelineBuild) must still expose
module archives to module-only consumers. CMake can place that closure before
the shared umbrella's transitive DSO, extracting a second runtime. Symbol
interposition then causes its global objects to be finalized twice.

The shared target now injects its DSO file into consumers' direct dependencies
and excludes direct references to the module archives already contained in it.
The ordinary shared-target dependency remains, preserving build dependencies and
usage requirements. Static-only consumers never see this interface.

An early candidate passed the default Mold linker but failed GNU ld.bfd with an
opaque STATIC framework: `--as-needed` discarded the DSO before the first CNA
reference arrived. That candidate still aborted and defined 1,200 strong Graphics
symbols. The failure is retained in
`repair-mixed-runtime-linkers-initial-bfd-opaque_shared_last-*`.

The final fix wraps only CNA's injected DSO in a custom CMake link feature using
`--push-state,--no-as-needed` / `--pop-state`. It restores the consumer's state for
unrelated dependencies. The feature is visible in cache scope because consumers
may be declared outside CNA's `add_subdirectory`. No global linker-policy change,
sidecar merger, public API change or renderer-specific workaround is involved.

Relevant primary documentation:
[direct dependency injection](https://cmake.org/cmake/help/v3.30/prop_tgt/INTERFACE_LINK_LIBRARIES_DIRECT.html),
[custom link features](https://cmake.org/cmake/help/v3.30/variable/CMAKE_LANG_LINK_LIBRARY_USING_FEATURE.html),
[link-library expressions](https://cmake.org/cmake/help/v3.30/manual/cmake-generator-expressions.7.html).

## Completed validation

- Native focused CTests: **31/31**. Twelve mixed-consumer variants each have
  runtime and symbol checks (24), plus six static-footprint tests and the existing
  shared-mode policy fixture. Opaque cases also require `libcna.so` in `DT_NEEDED`
  while rejecting an unrelated unused DSO. A permanent explicit ld.bfd case
  protects the failed first draft.
- Eleven actual consumer topologies linked and executed under each of GNU ld.bfd,
  LLD and Mold: **33/33**, zero strong Graphics definitions. Orders, target aliases,
  opaque frameworks and the old explicit workaround are covered.
- All 21 generated module/footprint link closures remain static; none gains CNA's
  DSO. The six existing footprint tests pass.
- Four real CMake versions (3.28.6, 3.29.9, 3.30.9, 3.31.6), four modes each:
  **16 expected outcomes**, including deliberate unsupported shared-mode refusals
  on older versions. Clang adds four successful current-version scenarios.
- Canonical native build succeeds. The only configure warning is the existing
  Draco CMP0148 developer warning; no new compiler warnings.
- `CnaTests`, `CnaMathTests` and `libcna.so` are byte-identical to the immediately
  preceding publication validation: full native **10,690 PASS / 101 SKIP / 0 FAIL**,
  math **967 PASS / 2 SKIP / 0 FAIL**. This is exact-artifact reuse, **not** a second
  execution of those full suites.
- External build succeeds after removing both demo-local first-DSO workarounds.
  Of 319 artifacts, 318 are byte-identical to the preceding roots checkpoint.
  The sole changed FBX reader tool now retains its requested `libcna.so` dependency.
  Relinking a separate control with only the scoped needed-state wrapper removed
  produces the exact previous SHA-256. `readelf` shows only CNA added to
  `DT_NEEDED`; the repaired tool exits zero. See
  `repair-mixed-runtime-fbx-delta-audit.json`.
- Fresh 95-case math checks on both SOFTWARE and OPENGLES3, metadata/catalog
  validation, three Python test suites and all six exact visual references pass.
- ASan/UBSan-instrumented minimal consumer with normal CNA archives/DSO:
  the old link graph fails LeakSanitizer with **2,070 bytes in 34 allocations**
  originating in interposed BlendState, SamplerState and TouchPanel initialization;
  the repaired graph exits zero with leak detection enabled and no diagnostics.
  The first audit incorrectly predicted the same double-free manifestation under
  the instrumented allocator and therefore exits 1; it is retained. A separate
  read-only observed-evidence audit records the actual leak result. No diagnostic
  was suppressed. This is allocator-intercepted lifetime evidence, **not** a fully
  instrumented CNA build or a claim that ASan diagnosed the original double-free.
  See `repair-mixed-runtime-sanitizer-{before,after}.log` and
  `repair-mixed-runtime-sanitizer-observed-audit.json`.

Full 719-demo sweeps completed on both renderers: SOFTWARE **625 PASS / 43 known
FAIL / 48 SKIP / 3 manual**, OPENGLES3 **664 PASS / 45 known FAIL / 7 SKIP /
3 manual**. Every status, known-issue marker, check count and failed-assertion
multiset matches the preceding roots checkpoint exactly. The original C
RootDirectory contract residual remains visible: both original-baseline audits
and the enclosing driver exit 1, solely for that previously recorded residual.
This is a separate owner-approved follow-up, not an ISSUE-016 suppression or a
claim that the entire campaign is green.

Serial CTest completed **684 PASS / 48 unchanged SKIP / 3 disabled** in 990.96s:
735 registered, 732 enabled, zero failed tests. All 319 frozen artifact hashes
were verified before and after both sweep/CTest stages. The final gates summary
and `repair-mixed-runtime-ctest-final-audit.json` retain every outcome. No
unexplained regression delta remains for this repair. Hardware/service coverage
is not inferred from these link-only probes; runtime coverage here is Linux
SOFTWARE and OPENGLES3, not all platforms or renderers.
