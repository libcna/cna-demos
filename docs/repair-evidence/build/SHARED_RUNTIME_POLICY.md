# ISSUE-001: cyclic whole-archive configuration contract

## Closed checkpoint (2026-10-20,14:38 UTC)

ISSUE-001 is FIXED_VALIDATED. External57606 completes0:50 printed tasks (three
regeneration/prelude steps and47 build tasks), only the existing Draco CMP0148
developer warning, no compiler warning. All319 artifacts are byte-identical to
the completed roots/boundaries checkpoint; all11 repair-source hashes match.
Fresh content146 focus2930 gives68PASS/five unchangedFAIL per renderer, exact
audit errors[]. Fresh three catalog CTests53901 pass6.60s. The complete earlier
both719/math95/six exact visuals/735 CTest outcomes apply to these identical
runtime artifacts; a second full runtime sweep is not claimed. The unchanged
C RootDirectory conflict still prevents global acceptance. All155 issue bodies
are preserved; this closure brings the campaign to84 final/71 pending.

The following notes retain the repair's earlier pending state chronologically.

Repair committed CNA `b35fbc933`. Final four-version audit93677 passes all16
scenarios with the final fixture source, plus actual top-level shared-ON configure
refusals under3.28.6 and3.29.9. Eleven committed files are frozen in
`repair-shared-runtime-source.sha256`. External reconfiguration/build awaits the
independent content-roots full CTest freeze; ISSUE-001 remains open until that gate.

2026-10-20. The original bug is reproducible, not a claim that newer CNA's successful
default build proves compatibility with every admitted CMake version. The unchanged
production shared-runtime helper with two cyclic archives fails with duplicate
symbols under checksum-verified official CMake3.28.6 and3.29.9. The same fixture
builds/runs under3.30.9 and system3.31.6. CMP0156 OLD still passes on newer versions;
explicitly disabling feature deduplication on3.31 reproduces duplicate symbols.
Original reports remain `repair-baseline-shared-runtime-*`.

Kitware documents the link-feature attributes added in [CMake3.30 release notes](https://cmake.org/cmake/help/v3.30/release/3.30.html#generator-expressions).
The measured boundary is3.30, not the historical suggested3.29 workaround.

## Narrow repair, not a new legacy linker implementation

The actual shared-mode policy is moved intact into `cmake/SharedRuntimePolicy.cmake`
with its minimum corrected3.27->3.30. The top-level configuration includes that
policy before defining dependency/module targets, preserving PIC and link-dependency
settings. Older CMake defaults to the existing static build; an explicit or cached
shared request fails during configuration with the required version and OFF/upgrade
remedies. The shared helper also refuses unsupported direct use. CNA's overall
minimum and static linking are unchanged. Current supported versions still default
to shared mode. This does **not** claim shared linking now works on3.28/3.29.

The permanent CNA fixture uses real mutually dependent archives and a public
consumer through the unchanged shared-link construction. It tests DEFAULT, ON,
OFF and direct-helper invocation with the actual executing CMake; it does not
simulate CMAKE_VERSION. Current-toolchain CTest is `CnaSharedRuntimeLinkage`.
All fixture code compiles Wall/Wextra/Werror through the shared compiler cache.

## Validation to date

- 3.28.6 and3.29.9: default/static builds and consumers pass; explicit ON and direct
 helper each fail at configure time with the exact actionable contract diagnostic.
- 3.30.9 and3.31.6: all four configure/build/consumer scenarios pass, including the
 cyclic shared library with CMP0156 OLD. Twelve successful build/run scenarios and
 four deliberate configuration refusals, sixteen in total.
- Initial older-version audit failures are retained: CMake wrapped the diagnostic
 between `>=` and `3.30`; normalization now collapses whitespace before checking
 the same exact version/remedy. No link failure was reclassified as a pass.
- Canonical CNA build27369 succeeds, with three regeneration/prelude steps plus
 six build tasks. The sole CMake developer warning is existing Draco CMP0148;
 no C++ compiler warnings. Four previous native artifact hashes are identical
 after the shared-library regeneration/relink. All319 external frozen artifacts
 remain unchanged while the independent content-roots full regression runs.
- Registered native CTest5779 passes (test2.38s,total4.35s). Repeated-cache33118 also
 passes (test0.87s,total1.22s); the fixture explicitly clears its own option cache
 keys to retest DEFAULT when reusing a work directory. Source hash audit, commit,
 external configuration/build and issue closure remain
 pending. No ISSUE-001 final-state credit yet.

Logs: `repair-shared-runtime-policy-{wrapped-3.28.6,wrapped-3.29.9,3.30.9,3.31.6}.log`,
`repair-shared-runtime-{native-build,registered-ctest,pre-native-hashes,post-native-hashes,external-unchanged-hashes}.log`.
The original failing audit logs lack the `wrapped-` tag and are not overwritten.
