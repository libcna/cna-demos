# ISSUE-011 inherited ownership review — 2026-10-23

The owner authorized reviewing and completing the inherited ownership candidate.
The full earlier failures, genuine XNA evidence, 61-case sanitizer result and
historical C contract conflict remain in `RUNTIME_OWNERSHIP_TRIAGE.md`.

## Accepted contract and implementation

ContentManager records every materialized disposable, including nested children
and completed children from a later failed load. It disposes in recorded order;
a throwing callback still clears the records/cache. Mutating the records during
Unload is detected without retaining an invalid vector iterator. Custom caller
recorders keep their explicit ownership. Polymorphic Object/IDisposable readers
retain the internal resource ownership capability.

Value wrappers use an internal ownership group and stable representative, with
copy/move membership and callbacks outside the membership lock. The group avoids
a representative cycle. Concurrent access to the same wrapper still needs caller
synchronization; this is not a new blanket thread-safety guarantee. Models record
buffers, effects and owned textures at their materialization layer. Content-owned
texture Name/Tag metadata is shared; ordinary unrecorded copies remain independent.

The owner explicitly selected XNA/C++ lifetime for C loaded Effect/Model:
Unload/Destroy disposes their native resources. Stable C handles protect object
storage and remain usable for cleanup/metadata, but Effect Apply and mesh Draw
refuse disposed resources. Public C headers, CONTENT/EFFECTS/MODELS documentation
and the old two usability assertions are corrected explicitly, not suppressed.
Fresh C model loads test Unload and direct Destroy independently, checking effect,
vertex and index disposal and exact effect exception identity.

Transferred C texture/font/sound ownership is retained. The original exact-pixel
and audio-instance probe still protects that different C transfer contract.

## Additional defect caught during review

ResourceDestroyed used the wrapper's stale private Name/Tag instead of its current
shared content metadata. `ContentTextureMetadataTest.ResourceDestroyedReportsCurrentSharedMetadata`
failed before the one-line getter correction. The new assertion checks every
notification, not just one representative. Report `repair-inherited-metadata-before`
is retained as the red witness.

## Current validation

The inherited baseline built and ran Core119, Content2060, Media355 and Storage55
without failures. The candidate's four occlusion count failures were fixed and
committed separately under ISSUE-113. Current ownership/event/SpriteBatch rebuild
succeeded. Current native modules: Core119 PASS; Content2,057 PASS / four unchanged SKIP;
Graphics3,051 PASS / 115 unchanged SKIP; Media355 PASS; Audio775 PASS / four SKIP.
The new metadata regression passes, with every old Content outcome unchanged.
C GraphicsResourceSmoke and ContentModelEffectSmoke both pass on OPENGLES3.
The first native driver incorrectly used the build directory; Content102 and
Media234 failures were missing-relative-asset/setup failures. The corrected source
working directory matches `cmake/UnitTests.cmake`; both entire unchanged binaries
pass there. Failed logs remain `repair-inherited-ownership-sprite-native-*`;
accepted rerun is `repair-inherited-ownership-native-cwd-*`. No source drift.
Renewed integrated ASan/leak/UBSan/float-cast-overflow validation passes all62
ownership cases. Actual affected CNA implementation and129 Sharp Runtime TUs are
instrumented; third-party libraries are not. The445-task incremental library build
took1,191,450ms; fixture compile38,104ms; runtime476ms. All145 previously known
renderer-header unused-parameter diagnostics match by message/count (line positions
shifted with the separately committed113 header addition). Sixteen existing Draco
warnings remain visible. Peak4,296,978,432 bytes,1500 high-limit throttles, zero
max/OOM events and no source/artifact drift. Reports: `repair-inherited-ownership-asan-*`.
The final external driver rebuilt all consumers serially and completed 191 PASS /
one unchanged capability SKIP. It preserved every prior content/graphics outcome
except the ten explicitly accepted FAIL-to-PASS rows: C content manager on
SOFTWARE/OPENGLES3, SpriteBatch begin/end on four renderers and C resource
lifecycle on four renderers. Independent C cube/sound ownership passes on
SOFTWARE and OPENGLES3. Six exact visual references pass. No frozen source or
artifact changed; peak memory was 4,301,119,488 bytes, with zero max/OOM events.
Reports: `repair-inherited-ownership-sprite-external-*`.

Accepted CNA lifetime commit: `39ef8ea2f`. Final unmarked demo/catalog closure
is recorded with the issue disposition rather than weakening any assertion.

## Handoff boundary for ISSUE-010

The current ownership repair also fixes the disposed ContentManager Load guard.
That does not by itself close all of ISSUE-010. A source-only residual audit finds
StorageContainer's constructor still calling throwing std::filesystem::exists /
create_directories directly on its resolved path. The successor should reproduce
an unavailable/uncreatable container path, check XNA/.NET exceptions and delegate
to the correct runtime layer before auditing final010 closure. No new runtime
claim or constructor fix is made in this review.
