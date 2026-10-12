# Changelog

All notable changes to cna-demos. The format follows [Keep a Changelog](https://keepachangelog.com/);
versions follow Semantic Versioning (pre-1.0: a minor version may change the CLI or metadata).

## [Unreleased]

### Validation
- Cascade sampling now demonstrates fourth-slot depth and diagnostic tint. Ten GL pixel
  checks and26targeted CTests pass; original eight panels stay unchanged outside the new cell.
- Complete470-clear: 483 enabled CTests and both full smoke suites retain all prior
  failure, child and network evidence; selected artifacts and normalized diagnostics stay stable.
- Stereo streaming now distinguishes paused queue clearing from Stop. Both dummy-audio
  runs pass23checks and37enabled targeted CTests pass; prior20assertion calls remain unchanged.
- Complete470-bounds: 483 enabled CTests and both full smoke suites retain all prior
  failure, child and network evidence; selected artifacts and normalized diagnostics stay stable.
- Stereo buffer example adds four float-range refusal controls. All20checks pass in both
  dummy-audio selections; original16assertion calls and37enabled targeted CTests are preserved.
- Complete470-spatial: 483 enabled CTests and both full smoke suites retain all prior
  failure, child and network evidence; selected artifacts and normalized diagnostics stay stable.
- Positional audio adds six public Pan/3D mode controls while retaining its original sweep.
  Seven checks pass in both dummy-audio selections and 37 enabled targeted CTests pass.
- Complete470-release: 483 enabled CTests and both full smoke suites retain all prior
  failure, child and network evidence; selected artifacts and normalized diagnostics stay stable.
- Looping audio adds paused Stop(false)/Resume/natural-completion controls. All six checks
  pass in ten dummy-audio repetitions; 37 enabled targeted CTests pass. No listening claim.
- Complete470-specular: 483 enabled CTests and both full smoke suites retain all prior
  failure, child and network evidence; selected artifacts and normalized diagnostics stay stable.
- Skinned shadow receiver adds isolated specular attenuation controls: nine GL checks,
  26 targeted CTests and all 15 pixels in the inspected image pass. Original controls remain.
- Complete470-border:483enabled CTests and both smoke suites preserve all prior failure,
  child and network evidence; selected artifacts and normalized diagnostics stay stable.
- Cascade atlas demo adds two shared-boundary isolation controls; all eight GL checks and
  26 relevant CTests pass. The actual image and all eight sample colors were reviewed.
- Complete470-query: the fresh full run passes483enabled CTests and both smoke suites with
  exact prior failure/child/network evidence and stable selected artifacts. The original
  staging-warning refusal and independently reproduced open ISSUE-057 remain preserved.
- Recorded CNA-DEMOS-ISSUE-057 after a pre-CTest content staging warning. Three isolated
  concurrent probes reproduce the diagnostic; a sequential control is clean. Original failure
  evidence is retained, and the full validation warning gate remains enabled.
- Complete470-cascades: all483enabled CTests and both full smoke suites preserve every old
  failure/child/network control. Six new cascade checks pass on GL; SOFTWARE declares its
  stock-path skip. Selected artifacts stay stable through runtime after consistency rebuilding.
- Complete 469-pcf: all 482 enabled CTests and both full smoke suites preserve the exact
  old failures, child/network controls and normalized diagnostics. The receiver adds three
  passing kernel controls; selected artifacts remain stable through runtime.
- Complete 469-shadow: all 482 enabled CTests and both smoke suites pass their expected
  outcomes. All old failure/child/network evidence is preserved; the only added diagnostic
  is the declared SOFTWARE shadow-path skip.
- Complete468-formats: a fresh full run after the preserved XACT timing failure passes all481
  enabled CTests and both smoke suites. Every old failure/child/network control remains exact;
  selected artifacts and normalized diagnostics remain stable.
- Complete467-readers: all480enabled CTests and both full smoke suites preserve every old
  failure and exact child/network controls. Selected binary/asset hashes, including the new
  primitive fixture, stay stable; normalized application diagnostics are unchanged.
- Complete467-move: all480enabled CTests and both full smoke suites preserve all38/42old
  failures and exact child/network controls. Selected artifacts stay unchanged through runtime,
  and normalized application diagnostics remain identical.
- Complete467-field: all480enabled CTests and both smoke suites retain every38/42old failure
  and child/network control after external CNA relinking. Selected hashes remain stable
  through runtime; normalized application diagnostics are unchanged.
- Complete467-light: all480enabled CTests and both full smoke suites retain all38/42old failures
  and exact child/network controls. Selected hashes and normalized application stderr remain
  stable; semantic coverage grows without adding IDs or API-count-only variants.
- Complete467-scene: all480enabled CTests and both full smokes retain all38/42old failures,
  exact child/network controls and stable selected hashes. Normalized application stderr
  is unchanged; generated coverage now records174C routes.
- Complete467:480enabled CTests and both full smoke suites preserve every old failed assertion
  and add only the independently confirmed EOF issue. Exact child controls and selected
  hashes remain stable; generated coverage now includes467demos and170C routes.
- Complete checkpoint 466-transfer:479 enabled CTests and both full smoke suites retain all
  37/41 old failures and child controls. Selected hashes stay stable; an exact reduction from
  84 to42 existing GL X protocol messages is reviewed separately, without a repair claim.
- Complete checkpoint 466-timing: all 479 enabled CTests and both full smoke suites pass
  exact baseline comparison, preserving all 37/41 old failures and network evaluation checks.
  The initial aggregate network deadline is a recorded demo bug; a per-phase watchdog keeps
  every packet predicate. A controlled delayed exchange lasts 37.41 seconds and still runs
  all five host checks, while the original sender times out at 22.88 seconds.
- Checkpoint 464: all 477 enabled CTests and both complete smoke suites verified, preserving
  all prior 37/41 failed assertions and C/custom child evidence. A SIGTERM-interrupted smoke
  attempt is archived; both suites were resumed without rebuilding and selected hashes stayed
  stable. Semantic auditing now distinguishes Texture3D transfer, sampling and lifetime gaps.

### Added
- The existing occlusion-query scene now checks fresh-result access, Begin/End ordering,
  observation before reuse and explicit disposal. Eleven controls pass on both renderers;
  the original visibility scene and its three checks are preserved.
- Authored two-slice shadow cascades: inclusive depth selection, blend-band visibility,
  a separate fitting camera and Count0 fallback. Six GL pixel checks pass in the inspected
  scene; unsupported SOFTWARE rendering is explicitly skipped.
- Public OcclusionQuery member audit: existing visibility/polling evidence, precision semantics,
  and explicit decisions for ordering, disposal, unsupported profiles and backend lifecycle gaps.
- The existing shadow receiver now compares one-tap, 3x3 and 5x5 visibility averages over
  an authored depth edge. All seven GL controls pass; the inspected image marks the measured
  positions and retains the original reception/bias/detachment controls.
- Authored-map skinned shadow reception: four GL pixel controls compare disabled/enabled
  reception, depth bias and detachment while preserving ambient/emission. The capture was
  inspected; unsupported SOFTWARE rendering is explicitly skipped.
- Stereo dynamic-audio buffer formats: byte/sample ranges, partial-frame rejection, preserved
  paused streams and stopped integer/float transitions. Sixteen public-state controls pass
  with dummy audio; the existing event-refill demo remains unchanged.
- Existing XNB reader policies now include built-in factory withdrawal, cache independence and
  re-declaration by a subsequent Game. All23checks pass in an isolated console process;
  a deterministic signed primitive fixture supplements the unchanged schema fixtures.
- Existing volume sampling now survives move construction, replacement of an occupied destination
  and disposal/destruction of both source wrappers. Eight GL checks pass with an unchanged
  inspected image; SOFTWARE retains its declared CustomEffects skip.
- Existing volume transfers now demonstrate signed Vector4 fields, vector/scalar element
  counts, compact two-slice edits and guarded refusal preservation; sixteen checks pass.
- C SpriteBatch now proves an invalid second texture refuses the entire command array before
  its valid first command is queued; the existing scene and all prior checks remain.
- Existing skinned lighting now isolates two diffuse lights and combines them with its retained
  specular controls; eight checks pass per renderer and labeled captures were inspected.
- C SpriteBatch now proves successful End releases retained textures while flushed pixels
  survive texture and batch destruction. Ten additions preserve every original child control.
- Shader palettes now compose a shared scalar matrix and premultiplied scene tint over both
  array entries; six GL checks preserve the old controls and verify transformed pixels.
- C storage now demonstrates explicit read-only permissions, refused truncation/writing and
  preserved file bytes, position and length. Seven new controls pass; only EOF056 remains.
- Three-tap shader image filter: five GL checks and inspected pixels demonstrate scalar weight
  arrays, vec2 offsets and non-square texel size with same-program updates.
- Existing C save example adds parent/child release guards, guarded short EOF reads, rewind
  and handle expiry. It preserves all23original controls and exposes ISSUE-056: FileStream
  returns position-1after EOF. Direct and C probes isolate the dependency defect.
- Existing Texture3D transfers now include guarded boxed reads, authored non-cubic mip data
  and refusal preservation; ten checks pass on both renderers with unchanged inspected images.
- Existing C content now demonstrates stable game-owned manager borrowing, configuration
  copying and handle expiry. Eight new controls preserve all original checks and scene pixels.
- Typed cube payloads distinguish packed storage words from C++ object sizes and preserve
  unclamped float-vector channels; eight checks pass on both renderers.
- Actual volume sampling compares slice centers, point/linear filtering and W-only wrapping;
  six GL checks pass and the labeled image was inspected. SOFTWARE declares its capability skip.
- The existing C SpriteBatch demo now checks active-batch texture retention and cancellation
  without flushing a queued corner command. All original checks plus eight additions pass.

- DXT1 cube block transfers: guarded byte windows, padded small mips and refused-transfer
  preservation pass eight checks on both renderers.
- Custom shader matrix/vec3 palettes: four GL checks verify explicit element counts/layout,
  caller-buffer overwrite and same-program replacement; the rendered comparison was inspected.

- Cube sampler ownership demo: shared full-face edits survive source disposal and all six
  faces are sampled through ShaderEffect. Five GL checks pass; the capture was inspected.
- Automatic C dispatch now isolates Draw service with a BeginDraw worker and first-Draw
  observation. All fifteen Update controls remain; six new controls pass on both renderers.

- Shader compiler diagnostics demo: seven GL checks distinguish compiler failure from package
  selection refusal, retain owned diagnostics and render with the original valid program.
  The deliberate compiler stderr and inspected cyan-card capture are retained.
- Shader clone state/lifetime demo: independently compiled programs, matrix-copy comparison,
  mutation isolation and rendering after original disposal. Nine GL checks expose the three
  ISSUE-055 assertions; both images inspected. SOFTWARE declares its capability boundary.

### Fixed
- XACT play_cue now waits for actual bank-idle state with a finite three-second limit instead
  of assuming completion after700ms. All three original checks remain; the first failed full
  checkpoint and inconclusive reproduction evidence are preserved.
- Network simulation now gives each finite startup/burst/recovery phase its own 20-second
  watchdog. A real full-smoke failure exhausted the previous aggregate budget before packet
  evaluation; its raw evidence is retained. Both focused renderers preserve all five original
  controls and the same ISSUE-030 failure, with packet counts and latency/recovery checks intact.
- Content manifest checks now verify current reader availability across CNA's registration changes. The demo validates
  per-file reader counts, inventory stability across loading and the loaded font reader family,
  replacing its blanket assumption that every built-in reader exists at Game startup.
  Later query-triggered registration is compatible with these checks; no lazy transition is inferred.
- Four shader/MRT demos now include CNA/ShaderDialectEXT.hpp and use the public CNA namespace
  after CNA promoted the dialect query type out of its renderer-internal interface. Shader text,
  stable IDs and existing assertions are retained.
- Guide notification placement now observes the first rendered toast within a bounded real-time
  window. Waiting eleven frames plus 500 ms could miss CNA's four-second notification under load;
  the reproduced empty-frame failure and all four original assertions are preserved in evidence.
  Either expected or misplaced notification pixels end observation, so wrong placement still fails.
- Audio natural-completion checks now poll state with a steady-clock deadline. A full 438 CTest
  caught the stream demo still playing after its fixed 600 ms sleep; the original failure is
  archived. Both stream and playback-state demos retain six checks and leave the voice to finish.
- The process launcher flushes C++ and C standard streams before running a child. A 421-demo
  checkpoint exposed child XML output splitting a buffered parent assertion even with stderr
  separated; raw evidence and an exact manually reviewed comparison copy are preserved.
  A small buffered-pipe CTest reproduces the old ordering and passes with the fix.
- Full validation preserves smoke stdout and stderr in separate logs. A real 418 checkpoint
  spliced native logger output into a buffered assertion; raw evidence and a narrowly reviewed
  comparison copy are retained. A regression fixture rejects the old merged-stream script.
- JPEG round-trip comparison now checks decoded dimensions before indexing the pixel array,
  so an incorrect decoder shape cannot turn a failed demo expectation into out-of-bounds access.
- Standalone C++ tools now use the browser's shared-library-first link ordering. The new XNA
  adapter tool exposed existing ISSUE-016 again: duplicate CNA TouchPanel statics caused an
  exit-time double free despite passing pipeline checks.
- Network simulation now uses coordinated finite bursts for baseline, latency, complete outage
  and recovery. Exact sent counts replace endpoint-span estimates and flaky percentage gates on
  unreliable transport. ISSUE-030 remains visible after loss is disabled and recovery is allowed.
- Complete-checkpoint assertion comparisons include prefixed child-process failures. This found
  a transient baseline network assertion hidden by the whole-demo expected-failure marker.
- Full validation removes stale current smoke JSON outputs before starting, while preserving
  named baseline archives; an interrupted process cannot leave an old report looking current.
- Four XACT demos now own GetCue results with unique_ptr: Dispose releases playback but does not
  delete the returned C++ object. Borrowed video/leaderboard pointers retain their existing ownership.
- Manifest paths are shortened to fit their on-screen columns; full names remain in console output.
- Curve loop demo now checks the last outgoing tangent for Linear extrapolation; the former
  expectation matched a CNA defect, now recorded as CNA-DEMOS-ISSUE-036.
- Catalog validation extracts each demo object once before checking its fields and known issue;
  this avoids repeatedly parsing the complete catalog and timing out at 401 demos.
- `tools/validate.sh` now stops on build and CTest failures even when their output contains no
  recognized diagnostic; complete logs are kept in the reports directory. `JOBS` controls
  parallelism. `tools/test_validate.py` checks failure propagation without building CNA.
- Coverage tables distinguish explicit NOT RUN results from demos absent in an older report,
  listing missing IDs so stale smoke evidence cannot silently appear current.

### Added
- Portable shader packages now demonstrate owned stage/language pairs, live selection independent
  of declaration order, incomplete/ambiguous refusals, owned diagnostics and checked shader pixels.
- Cube DDS loading now checks every authored face/mip and stream boundaries. ISSUE-054 records
  ignored short reads and acceptance of truncated payloads behind a prefix; both failures remain visible.
- The existing C dispatch demo now verifies final owner destruction releases an actually queued
  property call, preserves the worker error record and checks the invalidated game handle after joining.
- Cube face transfers now demonstrate guarded source/destination windows, a rectangular face edit,
  independently authored mips, refused-transfer preservation and a readback atlas (eight checks).
- Existing animation demos now distinguish local/world/skin transforms and verify nonidentity
  untracked state, wrong-space refusal with preserved transforms and disturbed-pose time clamps.
- C launch-parameter map example with owned UTF-8 spans, ordinal key enumeration, guarded
  count/copy buffers, duplicate precedence and preserved state on refused replacement.
  All fifteen child checks pass on SOFTWARE/OPENGLES3, and the 35 related C/CLI/launch CTests pass.
  The transient external CNA build failure and its repair are recorded as ISSUE-053.
- Skinned material scenes isolate textured emission/ambient response and premultiplied global
  alpha, then colored specular lights 1/2, their sum and the material shininess exponent.
  Both four-check scenes pass on SOFTWARE/OPENGLES3; all four captures were inspected.
- Focused C automatic dispatch demo proves ordinary Update service with every Draw suppressed,
  plus callback-scoped device borrowing and expiry. Fifteen checks pass on both selections.
- Existing thread-local C diagnostics retain all eight earlier controls and add nine checks for
  canonical owner exception details versus a worker's C validation, without foreign Game calls.
- Skinned vertex-color example verifies the actual CNA extension with a plain packed-color vertex,
  disabled control and interpolated material/texture/tint multiplication; both captures inspected.
- Existing C host and error demos now exercise larger compatible version-one structures: callback
  tables are copied and used after caller storage is wiped; diagnostic fields are filled without
  changing unknown tail fields or the original error. Both renderers retain all older controls.
- Two focused skinned-lighting scenes now verify rotated/blended normals and nonuniform World
  inverse transpose, then distinguish actual vertex/pixel diffuse lighting. Five and three checks
  pass on both renderers; all four captures inspected. LightingEnabled=false refusal is included.
- Skinned fog now has a focused bone-depth scene: a shared bind mesh moves only through its
  palette to the fog start, middle and end. Four rendered checks include a disabled-fog control;
  both renderer captures were inspected and agree within the original one-channel-level tolerance.
- Existing skinning-channel demo now renders through the final MaxBones palette slot, retaining
  its original short-palette controls. Seven checks pass on both renderers; image comparison
  confirms only the added cyan mesh changes, with all three earlier shapes intact.
- Active skinning-channel demo verifies that one/two influences ignore nonzero unused lanes,
  and four weights blend the explicitly indexed palette. Six rendered checks pass on SOFTWARE
  and OPENGLES3; both inspected captures are pixel-identical. Other skinning scenes remain intact.
- Focused C host-failure demo checks second-Update failure, terminal state without replay,
  preserved diagnostics, exactly-once unload and released-handle refusal. Its nine passing checks
  accompany one visible ISSUE-051 contract failure; normal host and RunOneFrame demos remain intact.
- Existing C dispatch demo now cancels a permitted queued property write, verifies no mutation
  or later replay, and resumes service for a fresh worker. All thirty prior assertions remain;
  ten new controls pass on both renderers, with only the existing two ISSUE-050 failures.
- A small host-frame reproducer confirms CNA-DEMOS-ISSUE-051: terminal state and unload cleanup
  work after Update fails, but C exiting/end_run callbacks promised by the public comment are
  suppressed. The focused host-failure demo now also checks this contract discrepancy.
- Existing C thread-dispatch demo now retains all original controls and exposes ISSUE-050 through
  two no-pump owner-only probes. Bounded cancellation leaves the game usable and avoids running
  forbidden work; only the two documented assertions fail on SOFTWARE and OPENGLES3.
- Existing C lifecycle demo rejects recursive frame driving and game destruction from Update,
  checks the callback-specific diagnostic and preserves all original timing, device-borrow and
  cleanup checks. This owner-thread boundary is independent of foreign-thread ISSUE-050.
- Bounded C owner-thread diagnostic reproduces CNA-DEMOS-ISSUE-050: enabled foreign-thread
  dispatch incorrectly queues RunFrameEXT and Destroy. Cancellation proves queuing without
  executing either forbidden operation; the same defect is now checked by the existing dispatch demo.
- Existing two-bone skinning demo now checks the blended joint silhouette, refused palette-read
  counts and an accepted maximum-sized palette. All eight older checks remain; the thirteen-check
  result preserves the original rendered images on both renderers.
- Skinned-effect clone ownership demo renders independent copied bone palettes after the source
  effect and caller texture reference are released, and checks shared texture lifetime through the
  last clone. A non-white texel makes the rendered checks distinguish real data from a fallback.
- Three existing C demos deepen callback contracts without adding IDs: retained frame failure and
  exactly-once unload cleanup; structured canonical diagnostics forwarded from a served worker
  failure; incompatible frame-hook layouts refused without replacing the active table. All older
  child assertions remain, and the host callback trace and rendered image stay identical.
- Existing texture resize demo now contrasts actual Reach/HiDef mipmap processing with native
  CNB output, compares freshly resized pixels with its loaded texture and retains the original scene.
  Seven checks pass; the public build-environment overview mismatch is documented as ISSUE-049.
- Explicit shared-art source-root capabilities through the built-in CNJ texture pipeline: actual
  decoded pixels, dependency aliases, relocated/local sources, refused paths/grants and repair.
  Console examples now show captured build messages and contextual source-root refusals.
- Native pipeline diagnostics compile an opacity strip with actual 8-bit precision loss and verify
  downstream logger forwarding, retained result messages, exact/no-warning inputs and failure/retry.
- Existing C host-frame demo removes and replaces hooks between frames with a separate caller
  context, retains all seventeen prior checks and captures the final replacement-hook frame.
- Budget dependency graph contrasts strong build edges with runtime-only references, verifies
  effective fingerprint propagation, real runtime loads, cycle/missing-node refusal and repair.
  Sixteen checks include actual two-worker importer overlap, deterministic outputs and skipping.
- The native patrol's fifteenth check documents its deliberately absent XNB writer and verifies
  the game-authored reason through a real Selection error while retaining its supported CNB path.
- Multi-artifact encounter compilation: two CNB schemas, a deployed briefing and real manifest
  ownership. Fourteen checks include partial publication/retry, graph destination conflicts,
  incremental skips and clean with or without its ownership record; no whole-node rollback claim.
- Existing patrol pipeline example distinguishes stable output declarations from actual C++
  payload types at Import, Process and Write, and verifies copied ContentValue ownership (14 checks).
- Existing XNA adapter contrasts strict refusal with warning/default parameter handling and retained
  valid overrides. Correct compiled bounds are verified separately from returned effective metadata,
  whose retained dropped entries expose ISSUE-048; the older ISSUE-047 remains visible.
- The validation script's Xvfb leg now explicitly selects SDL's x11 driver and removes inherited
  WAYLAND_DISPLAY. A real native-handle comparison exposed Wayland selection despite Xvfb; a
  regression test checks this isolation alongside existing failure-propagation controls.
- Existing-instance intermediate XML now demonstrates ordinary/raw object reads, retained
  reference identity and nonserialized state, Null/type gates and an explicit per-field compact
  serializer hint (10 checks; no global serializer replacement or runtime-XNB claim).
- Custom XML-to-XNB source integration: actual compiler publication through a game type writer,
  versioned runtime reader dispatch, incremental skip/edit and ContentManager cache/reload behavior.
  Ten checks include explicit refusal to invent a CNB schema for the arbitrary XML object.
- The embedded score compiler now exercises typed per-asset configuration fingerprints,
  semantic no-op configuration rewrites, obsolete-output collection after source removal and
  source reintroduction (20 checks, retaining all fifteen previous assertions).
- The incremental score compiler now verifies registry freezing, writer-version invalidation,
  digest-based tamper detection, corrupt-manifest recovery and refusal to clean changed owned files
  without deleting their valid peers (15 checks, retaining all ten original checks).
- Embedded incremental score compiler uses the actual RunContentCompiler coordinator: component
  counters and real manifests show unchanged skips, primary/shared dependency rebuilds, preserved
  valid output on import failure, repair, missing output recovery and ownership-aware clean.
- Nested builds now recognize separately scheduled sibling items without compiling duplicate
  output, resolve them after the host's own build/publication, and reject parameter mismatches
  for reuse (20 child checks total, retaining all fifteen prior checks).
- A nested stage/profile build demonstrates processed values versus separately compiled child
  assets, dependency/reference metadata, duplicate requests and name collisions. Caller publication
  now connects actual parent/child bytes to runtime loading, missing-child retry, shared caches
  and scoped cleanup (15 checks).
- Extended the packed ownership example with the absent-recorder path: fifteen checks pass,
  while default tracking fails to dispose its child on Unload, adding direct ISSUE-011 evidence.
- Packed XNB ReadAsset ownership with actual nested disposable callbacks, independent reads,
  source-stream lifetime and caller cleanup across manager Unload/Dispose (14 checks).
- Intermediate XML external-reference relocation and identity controls. The original emitted
  TargetType alias cannot be read back by CNA; ISSUE-046 preserves that failure beside nine
  passing checks without substituting the separately labeled canonical-name control.
- A polymorphic XML spawn list restores concrete enemy/treasure schemas through an abstract
  base and rejects unknown, unrelated or abstract type declarations (eight child checks).
- Existing intermediate XML tools now round-trip a cyclic resource graph with verified C++
  ownership cleanup, and enforce a non-null objective on both serialization and deserialization.
  These extend semantic coverage without adding demo IDs.
- Same-name compiled/source content precedence and cache lifetime checks; a truncated XNB
  exposes an unwrapped EndOfStreamException at the public Load boundary (ISSUE-045).
- Existing CNJ loading now distinguishes cache entries by requested C++ type, and the
  multiple-batch sprite demo checks device-state retention and rendered world/HUD positions.
- Versioned XNB reader policies with actual legacy/current records, explicit compatibility,
  duplicate registration, scoped withdrawal and verified factory-capture ownership (17 checks).
- Direct implicit ReadRawObject existing-instance reproduction of ISSUE-043, alongside the
  original GetTypeReader failure; eleven independent schema checks remain passing.
- Imported glTF loop/strip/fan topology checks and a DDS stream workflow with authored mipmaps,
  short reads, fit/crop conversion and malformed-chain refusal. Both demos check rendered pixels;
  their SOFTWARE/OPENGLES3 captures were inspected and match exactly.
- Recorded a transient external C API generator variable-shadowing build defect (ISSUE-044),
  corrected by the external author before the resumed build; no CNA edits were made here.
- Existing-object XNB loadout reads and external-reference cards: preserve runtime slot identity,
  distinguish indexed/raw/type-erased payloads, resolve sibling PNG/XNB assets and render shared
  cache edits. Distinct target/reader names expose CNA GetTypeReader failure (ISSUE-043), retained
  as an explicit known-failing check with a documented named-factory fallback.
- A runtime XNB shared-resource graph demo: deferred callbacks rebuild aliases and forward/weak
  backward edges, distinguish inline objects, preserve cache isolation, reject incompatible
  indices/types and release ownership (15 console checks, no graphics requirement).
- XNB model example builds an original `.x` asset through the existing content target and checks
  restored hierarchy, shared buffer fixups, material ranges and actual rendered placements.
- Multiple-skin example checks separate palettes despite shared authored geometry/material,
  scene ancestry, selective bind-pose restoration and live bounds before the final visible pose.
  CNA-DEMOS-ISSUE-042 records outdated root-prefix comments; the runtime path passes.
- Hierarchical palette example checks parent composition, untracked bind pose, interpolated
  translation/scale/rotation, inverse binds, loop/clamp time and rejected invalid skeleton requests.
- Loose skinned manifest example loads original relative sidecars, rejects truncated skeleton data
  and verifies the imported animation with actual pixels on SOFTWARE and OPENGLES3.
- Existing skinned animation now checks borrowed player state, relative versus absolute time,
  looping, clamping and restart; before/after first-frame images remain identical per renderer.
- Isolated picture-library example verifies saves, albums, original image bytes, generated thumbnail
  and reindexing; valid short reads expose CNA-DEMOS-ISSUE-041. Stale thumbnail comments are ISSUE-040.
- Existing sampler example now loads an authored glTF to verify NEAREST/CLAMP import, preserved
  out-of-range UVs and explicit application to actual rendered pixels.
- SkinnedModelEXT example transfers and replaces owned named parts, refuses a different bone count,
  removes a temporary part, then verifies sampled inverse-bind skinning after donor destruction.
- Existing mesh_parts now mixes a shared-buffer line part with triangles; existing two_bones checks
  palette readback and rejected invalid changes before verifying the preserved rendered pose.
- Model part sampler example distinguishes stored metadata from explicit device selection, with
  wrap/clamp pixel checks on both renderers.
- C host-frame example verifies copied lifecycle hooks, draw suppression, completion ordering
  and repeated terminal calls without restarting the run.
- Shared-buffer ModelMeshPart example checks independent index/base-vertex ranges and effects,
  material collection deduplication/removal, and both selected triangle halves with actual pixels.
- Focused C11 foreign-thread dispatch example checks default/restored handle affinity and explicit
  owner-thread pumping of worker reads/writes, with bounded completion and cleanup paths.
- Existing C vector/matrix example distinguishes directions from positions and checks array
  subranges, untouched guards and refusal of bad source/destination ranges before writes.
- Focused audio instance lifetime example checks retained sound data after C++ wrapper destruction,
  moving a playing voice, idempotent disposal and explicit parent disposal of moved/idle children.
- Existing CNB loader distinguishes valid containers from invalid game schemas: missing/duplicate
  RULE chunks, a truncated integer and trailing data are rejected with file/chunk diagnostics.
- Existing C lifecycle example checks expired callback-scoped device handles, fresh borrowing
  in the next callback and rejection of outside-callback borrowing after Run.
- Existing C content example checks copied root strings, preserved cache entries across root
  changes, uncached resolution under the new root and cache invalidation after Unload.
- Existing PNG/JPEG export example verifies requested 64x32 output dimensions and an unchanged
  128x128 source, with labeled decoded thumbnails beside the full-size round trips.
- Texture value-copy example distinguishes shared rectangle edits from full-upload detachment,
  copied-but-independent Name/Tag and sibling survival after Dispose. Both renderers pass pixel
  checks and produce identical inspected screenshots.
- Existing CNB limits example checks optional Zstd round trips and decoded-size budgets even
  when the compressed file fits its stored-size budget; disabling compression restores the original image.
- Existing C diagnostics example rejects short/unsupported ErrorInfo layouts while preserving
  the original thread-local diagnostic.
- Existing C audio example checks refused SoundEffect/game destruction while an instance lives,
  preserved sound duration/playback and INVALID_HANDLE after releasing the instance.
- Composite CNB encounter example resolves shared enemy profiles through XREF and the owning
  ContentManager, including reference failures and retry after a missing child is supplied.
- Bounded CNB tile-layer example distinguishes file/chunk/aggregate budgets from schema
  array/string limits, inspects actual chunk sizes and rejects a corrupted recognizable container.
- XNA pipeline adapter tool runs typed importer/processor registration through the real build
  coordinator, tracks an original scale sidecar, converts imported route units through another
  processor and checks compiled bounds, typed parameter isolation and contextual errors. Its
  processor-generated summary adds actual label/deployment dependencies, explicit changed-input
  rebuild and cleanup; a separate output-root refusal remains visible as ISSUE-047 (12/13 pass).
- XML configuration example combines validated properties, inherited-member ordering, get-only
  collections that append to constructor defaults, and flattened nested spawn coordinates.
- Focused C thread-local diagnostic demo interleaves two worker errors and preserves each worker's
  and the caller's result/category/message. This target alone uses C11 threads/atomics after an
  availability probe; the other C examples retain C99.
- Existing shared XML example checks null tables and rejects missing/duplicate resource IDs and
  incompatible resource types before returning the loaded graph.
- Existing C diagnostics example distinguishes native C validation from canonical exceptions via
  structured type/parameter fields, and checks a later native failure clears those fields.
- Checkpoint comparison tool combines ID/status checks with raw failed-assertion multiplicities,
  including child-process prefixes; regression fixtures and the archived masked failure verify it.
- Custom intermediate XML type serializer: compact NdM dice values at the root and inside a
  described attack, registry dispatch, round trip and malformed/domain validation on both paths.
- Existing custom XNB reader now counts actual Read calls to establish cache reuse and verifies
  ContentReader's logical asset name and owning ContentManager.
- Intermediate XML format example now distinguishes absent/present/explicit-null optional fields
  and checks that a missing required field is rejected even when its constructor has a default.
- C content example rejects destroying an atlas retained by a live font and checks that text
  still renders after the refused release and manager destruction.
- Shared intermediate XML resources: forward fixups, identity-preserving aliases, equal but
  independent loot tables and inline copies, with mutation showing the actual ownership graph.
- Existing C error demo verifies failed diagnostic queries preserve the original error and
  exact-sized string copies honor the payload boundary without adding a terminator.
- Intermediate XML tool demonstrates public field descriptors and ContentSerializerAttribute:
  renamed/optional/ignored fields, collection item names, round trip and ignored-field rejection.
- Existing C content demo checks texture pixels and font metrics after manager Unload and
  destruction, then checks the rendered checker in the combined sprite/text batch.
- Existing SpriteBatch layer-depth example checks overlap and unobscured colors; texture-group
  sorting now rejects a uniformly blank row as well as incorrectly grouped sprites.
- Existing content cache demo now switches roots after Unload and exposes CNA accepting a root
  change while assets remain loaded (ISSUE-039); the visual shows the observed result.
- Custom content pipeline demo: registered importer/processor/writer, build-time patrol-length
  computation, deterministic CNB output, runtime loading and contextual parameter diagnostics.
- Custom CNJ enemy demo with two named factories, actual JSON payload parsing, manager-local
  registrations, Unload/cache behavior and malformed-payload diagnostics; original fixtures
  are reproducible through the asset generator.
- Custom CNB loader now distinguishes optional and mandatory unknown chunks and retries an
  asset after its failed file is repaired.
- Dynamic streaming now checks PCM size/duration, unsupported loop/release operations and
  stop/disposal behavior. Two missing CNA disposal guards remain visible as ISSUE-038.
- Semantic coverage audits now include content loading/binary formats, texture ownership,
  audio/XACT, sprite/text behavior and C ABI diagnostics/ownership, separating exercised checks
  from visual-only evidence and preserving the differences between C and C++ contracts.
- Existing XACT cue-state demo observes Disposing, repeated Dispose and deleting a disposed
  object, with callback state kept alive through scope unwinding.
- XACT category instance-limit demo: Fail and ReplaceOldest policies, paused voices retaining
  slots, category isolation and slot reuse, with generated settings and public state checks.
- Texture rectangle demo now transfers between nonzero CPU array offsets and verifies untouched
  destination padding alongside the adjacent texture pixel.
- Manifest demo shows files arriving/disappearing between explicit refreshes and the independent
  asset cache; private temporary content is removed after the checks.
- Existing curve demos now show selective/asymmetric tangent computation, negative-time loop
  boundaries, and empty/single-key evaluation without adding duplicate demos.
- Custom CNB loader demo: a game-defined rules schema, public writer, shared loader registration,
  per-manager cache behavior and unsupported-schema rejection.
- Quaternion double-cover/relative-turn demo and typed effect parameter round trips; confirmed
  stock-parameter getter and Bones reflection gaps recorded as CNA-DEMOS-ISSUE-034/035.
- Started semantic member/behavior audit tables for Quaternion, EffectParameter and Curve/CurveKeyCollection in
  `docs/COVERAGE_AUDIT.md`, with explicit outstanding work and environment limitations.
- Project foundation: CMake integration with a CNA source checkout (`CNA_ROOT_DIR`, presets
  `default`, `windowless`, `linux-multi`, `linux-full`), Ms-PL licence, documentation set.
- Demo framework: self-registering registry, `DemoInfo` metadata, console/Game/standalone demo
  kinds, `DemoGame` base class (need checks, automated frame limit, Escape to exit, screenshots),
  `DemoContext` (output, checks, seed), runner with PASSED/FAILED/SKIPPED/NOT RUN results.
- `cna-demos` executable: interactive terminal browser and CLI (`--list`, `--categories`,
  `--search`, `--info`, `--run-category`, `--run-tag`, `--run-all`, `--smoke`, `--catalog-json`,
  `--list-ids`, `--validate`, `--renderers`, `--renderer`, `--frames`, `--seed`, `--screenshot`,
  `--report`).
- CTest integration: CLI tests and per-demo tests discovered at ctest time.
- Coverage report generator (`tools/coverage_report.py`) and deterministic asset generator
  (`tools/generate_assets.py`).
- Pilot demos in seven categories (math, core, graphics, content, audio, input, platform),
  including one standalone demo.
- Math demos: Vector2/Vector3/Vector4 (constants, arithmetic, dot, cross, normalize, distance,
  lerp, smooth step, Catmull-Rom, Hermite, barycentric, clamp/min/max, reflect, transforms,
  by-reference overloads, equality/ToString, homogeneous coordinates), Matrix (identity, SRT,
  axis rotations, axis-angle, yaw/pitch/roll, look-at, perspective, orthographic, invert,
  transpose, determinant, decompose, world/direction properties, billboards, reflection/shadow,
  element-wise operations, quaternion transform), Quaternion (axis-angle, matrix conversion,
  concatenate, slerp/lerp, inverse/conjugate, normalize), MathHelper (angles, scalar helpers,
  curves).
- `--log-level` option (CNA's logger defaults to `warn` in cna-demos).
- `tools/validate.sh`: build, validate, ctest, smoke reports and coverage in one command.
- `CnaDemos/MathChecks.hpp`: tolerance helpers for matrix/quaternion checks.
- Math demos: Point, Rectangle (edges, contains, intersect/union, inflate/offset), BoundingBox,
  BoundingSphere, BoundingFrustum culling, Plane, Ray, Curve (evaluate, tangents, loop types, step
  continuity, key collection), Color (construction, premultiplied alpha, lerp/vectors, packed
  value); packed vector formats (compact colour, half precision, signed normalized, integer,
  wide unorm, IPackedVector).
- `knownIssue` metadata: demos that fail because of a recorded CNA defect are expected failures in
  batch runs and CTest (`WILL_FAIL`), and an unexpected pass is reported.
- Game lifecycle demos (25): method order, Exit/Exiting, RunOneFrame, host-driven RunFrameEXT,
  fixed/variable time step, IsRunningSlowly, SuppressDraw, GameComponent, DrawableGameComponent,
  UpdateOrder/DrawOrder, component collection events, GameServiceContainer, back-buffer size,
  PreparingDeviceSettings, device events, Reach/HiDef profile, vertical sync, full screen (manual),
  window title/client bounds/resizing, IsMouseVisible/InactiveSleepTime, LaunchParameters,
  TitleContainer.
- SpriteBatch demos (14): destination/source rectangles, sprite-sheet animation, tint and fade,
  rotation/origin, scale, SpriteEffects flips, layer depth and sort modes, blend states, straight-alpha
  loose PNGs (NonPremultiplied), point/linear sampling, wrap tiling, transform-matrix camera.
- Graphics device demos (12): ClearOptions (and the error for a missing plane), split screen with
  viewports (XNA 4.0's Clear ignores the viewport), Viewport properties and Project/Unproject
  picking, scissor rectangle, PresentationParameters, GetBackBufferData (whole/region),
  SupportsCapability and the capability report (CNAEXT), GraphicsAdapter display modes and
  format queries, GraphicsResource lifetime events (ResourceCreated/Destroyed, Name, Tag).
- `CnaDemos/GraphicsNames.hpp`: names of graphics enums for demo output.
- Texture2D demos (7): procedural SetData, rectangle SetData on a canvas (and XNA's rule that a
  texture set on the device cannot be written), per-pixel collision with GetData, mipmap levels,
  16-bit surface formats, FromStream (as stored, fitted, zoomed), SaveAsPng/SaveAsJpeg round trips.
- SpriteBatch demos (5): SpriteSortMode::Texture, Immediate mode with device states changed between
  Draws (blend state; sampler state is CNA-DEMOS-ISSUE-004), several batches per frame, Begin/End
  rules.
- Compiled content: `content/` is built at build time by CNA's `cna-content` (`cna_add_content`)
  into `bin/assets`; `CNA_DEMOS_CONTENT_EXECUTABLE` names a host tool for cross builds.
- Original pixel font (`tools/pixel_font.py`), generated as an XNA font-texture sheet and compiled
  with `CNA.FontTextureProcessor` into `fonts/pixel_font`.
- SpriteFont demos (7): DrawString, MeasureString (centring, alignment, line rules), rotated/scaled/
  mirrored text, Spacing/LineSpacing, Characters/DefaultCharacter, StringBuilder text, and a font
  built in code with the CNAEXT SpriteFont constructor.
- Render-state demos (9): custom blend functions (multiply, screen, subtract, min, max), colour
  write channels, blend factor, bound states are immutable, texture address modes, wireframe
  sprites, back-face culling, depth testing, 2D stencil masking.
- 3D primitive and buffer demos (8): hello triangle with BasicEffect, primitive types, indexed
  triangles, VertexBuffer + DrawPrimitives, DynamicVertexBuffer with Discard, indexed cube,
  32-bit indices (90,000-vertex grid), custom vertex declaration.
- `CnaDemos/PixelChecks.hpp`: back-buffer pixel checks for Game demos.
- Render-target demos (5): Discard/Preserve usage, reading a target back, a target with its own
  depth buffer, mipmapped target, multisampled target.
- More render-state demos (7): DepthRead for transparency, depth-function X-ray silhouette, stencil
  overlap counting, depth bias for decals, MaxMipLevel, LOD bias (skipped on GL ES), anisotropic
  filtering.
- Effect demos (14): BasicEffect texture, default lighting, three coloured directional lights,
  per-pixel lighting, material colours, fog, alpha; AlphaTestEffect cut-outs; DualTextureEffect
  light map (two texture-coordinate sets); EnvironmentMapEffect with a TextureCube and Fresnel;
  SkinnedEffect with two bones; Effect parameters/techniques and Clone; CNA's PbrEffect.
- Audio demos (7): SoundEffect from PCM, Play with volume/pitch/pan and MasterVolume, instance
  playback state, looping, volume/pitch/pan ranges, Apply3D panning (smoke-only),
  DynamicSoundEffectInstance streaming, microphone input level (manual).
- Input demos (14): key pressed/released by state comparison, KeyboardState values and replay,
  key names and layouts (CNAEXT), modifier and lock keys (CNAEXT); mouse state, clicks and
  dragging, SetPosition and mouse look, relative mode, cursor shapes and a cursor from a texture,
  the ClickedEXT event; typing with TextInputEXT, input-method composition; clipboard text and
  MIME data formats (CNAEXT).
- Gamepad and touch demos (12): gamepad state, capabilities, GamePadState values and virtual
  buttons, dead-zone modes, vibration, keyboard emulation and device information (CNAEXT); touch
  state with mouse emulation, capabilities and coordinate space, gestures, TouchLocation and
  TouchCollection values, the gesture queue with the CNAEXT EnqueueGesture.
- CNAEXT device demos (12): raw joystick state, hot-plug events, device lists, force-feedback
  rumble and effects, device motion sensors, battery state; controller light bar, motion sensors,
  touchpad, trigger vibration, paddles and the share button.
- CNA-DEMOS-ISSUE-009 recorded: `HapticDevice`'s inline default constructor needs a complete
  platform type, so user code cannot create a closed device.
- Storage demos (7): StorageDevice selector (async pattern, free/total space), save and load a game
  with BinaryWriter/BinaryReader, FileMode/FileAccess, directories and search patterns, container
  lifetime (Disposing), where saves are stored and the per-player layout (CNAEXT), and CNA's rule
  that container paths stay inside the container.
- CNA-DEMOS-ISSUE-010 recorded: storage reports argument and filesystem errors with
  `std::invalid_argument`/`std::filesystem::filesystem_error` instead of XNA's exception types.
- Content demos (15): TextureProcessor parameters (colour key, premultiplied alpha against a loose
  PNG, mipmaps, resize to a power of two), XNB textures with DXT compression, a Curve from CNJ, a
  compiled WAV, ContentManager cache and Unload, one manager per level, missing assets, the CNAEXT
  manifest, a custom LooseFileContentTypeReader, an OpenStream override and Unload disposal (the last
  two expected failures).
- `content_xnb/` and the `cna_demos_content_xnb` target: content compiled into XNA's XNB format.
- CNA-DEMOS-ISSUE-011 (Unload does not dispose loaded assets), CNA-DEMOS-ISSUE-012 (Load<T> never
  calls an OpenStream override) and CNA-DEMOS-ISSUE-013 (the manifest lists the pipeline's
  bookkeeping files) and CNA-DEMOS-ISSUE-014 (before CNA's platform exists, the content root is the
  working directory) recorded; ISSUE-010 extended to ContentManager::Load after Dispose.
- Media demos (7): play a song and follow PlayPosition to its end, MediaPlayer events raised by
  FrameworkDispatcher::Update without a Game, playlists (Queue, MoveNext/MovePrevious, IsRepeating,
  IsShuffled), volume and mute, a spectrum/waveform visualizer, Song properties, MediaLibrary
  collections; two short generated tunes in `assets/music/`.
- `--screenshot` captures the last frame drawn even when a demo ends itself with `Exit()` before
  its frame limit (the frame is kept each frame and saved in `EndRun`).
- CNA-DEMOS-ISSUE-015 recorded: `Song::FromUri` resolves relative paths against the working
  directory, not the title location (FNA's behaviour).
- Model demos (7): Model::Draw, bones and their transforms, bone animation with the per-mesh drawing
  loop, frustum culling with mesh bounding spheres, per-mesh effect settings, skinned animation with
  AnimationPlayer and SkinnedEffect, and the CNAEXT glTF import report of a glTF loaded at run time.
- `tools/gltf_models.py`: original glTF scenes (a robot arm with baked vertex colours, a skinned
  strip with a bend clip) compiled by the content pipeline; one loose copy for run-time loading.
- XACT demos (5): AudioEngine/WaveBank/SoundBank with a cue played by name, Cue states, categories
  (volume, pause, stop), global variables, positioned cues; `tools/xact_files.py` writes the
  `.xgs`/`.xwb`/`.xsb` files (original tones) in the layout of CNA's own XACT example.
- Gamer services demos (10): signed-in gamers and the SignedIn event, Guide message box and keyboard
  input (password mode, cancel), trial mode with SimulateTrialMode, the Guide's sign-in screen for a
  second player, achievements from an offline title catalog, GamerProfile, rich presence,
  GameDefaults and GamerPrivileges -- offline, with local profiles.
- Graphics, model and content demos (7): occlusion queries, VertexBuffer::GetData,
  DynamicIndexBuffer with Discard, Texture3D sub-box data, glTF node-animation clips
  (ModelAnimationsEXT, ApplyClipToBonesEXT) on a generated turntable model, and the same font, sound
  and curve compiled to CNB and to XNB.
- Audit demos (3): `graphics.vertex_declaration.stock_vertex_types` (IVertexType, the four stock
  vertex structs, a VertexPositionColorTexture quad), `audio.xact.renderer_details`
  (AudioEngine::RendererDetails), `math.formatting.to_string` (ToString of the math types; expected
  failure, CNA-DEMOS-ISSUE-024: BoundingBox and Ray print doubled braces); the type-converter demo
  also lists Vector4, Plane and Ray.
- C API demos (3): `c_api.graphics.basic_effect_3d` (a lit cube from a vertex and an index
  buffer and a BasicEffect), `c_api.graphics.render_target` (render to texture, read back, drawn
  as a sprite) and `c_api.storage.save_game` (device, container, write/read/delete a save file).
- C API demos (3 more): `c_api.input.gamepad` (one button mask with the derived stick and trigger
  bits, the three dead-zone modes, live polling of player one), `c_api.game.components` (callback
  table components: draw order, visibility, enabled, collection events) and
  `c_api.media.media_player` (a song collection played as a queue: next, pause, resume, stop).
- `graphics.depth_stencil_state.two_sided_stencil` (a shadow volume counted in one draw with
  TwoSidedStencilMode), `net.network_session.private_slots` (PrivateGamerSlots) and
  `net.network_session.join_refused` (standalone, two processes: NetworkSessionJoinException for a
  game in progress, then AllowJoinInProgress; expected failure, CNA-DEMOS-ISSUE-032: the refusal
  reads SessionNotFound, and ISSUE-033: the late joiner's session stays in Lobby).
- `graphics.pbr_effect.texture_maps` (NormalMap with a Vector4 vertex tangent, EmissiveMap,
  OcclusionMap darkening ambient light only; GL renderers), `graphics.model.skinned_pbr` (a skinned
  glTF with a metallic-roughness material imports as SkinnedPbrEffect; tools/gltf_models.py writes
  painted_column.gltf), `content.cnj.sprite_font` (a seven-segment bitmap font authored as CNJ: atlas
  plus per-glyph rectangles, cropping and kerning; tools/generate_assets.py writes both),
  `game.window.screen_device_name` (ScreenDeviceName, ScreenDeviceNameChanged, the adapter's
  DeviceName), `platform.window.native_handle` (GameWindow::Handle and the typed
  NativeWindowHandle accessors).
- `c_api.game.components` takes one step a second when a person watches.
- `content.content_project.xna_content_project`: an XNA Game Studio `.contentproj`
  (`content_project/`) built as it is by `cna_add_content(... CONTENT_PROJECT ...)` into XNB:
  item names, the default colour key, a per-item processor parameter, a copied None item.
- Coverage: `Texture::Format` and `Texture::LevelCount` are named on XNA's base class `Texture`
  (they were credited to nothing as `Texture2D::...`).
- `math.color.named_colors` (chart of XNA's 141 named colours), `math.vector2.toolbox_2d`
  (Vector2's static helpers on 2D game tasks) and `math.vector4.color_math` (Vector4 helpers as
  RGBA colour maths).
- Coverage audit pass 2: 146 XNA member names that 66 existing demos call and teach were added to
  their `.apis` (effect World/View/Projection, Cue::Play, AvatarRenderer transforms, ...).
- `graphics.model.gltf_lights` (two KHR_lights_punctual lights become the PbrEffect's
  DirectionalLight0/1) and `graphics.model.clip_interpolation` (STEP, LINEAR and CUBICSPLINE
  samplers; expected failure CNA-DEMOS-ISSUE-031: CNA plays all three as LINEAR) and
  `graphics.model.material_variants` (KHR_materials_variants: Model::MaterialVariantEXT repaints a
  toy car); `tools/gltf_models.py` writes `lit_sphere.gltf`, `slide_rig.gltf` and
  `loose_paint_toy.gltf`.
- `net.network_session.simulated_conditions` (standalone: SimulatedLatency and SimulatedPacketLoss
  measured on packets between two processes; expected failure CNA-DEMOS-ISSUE-030: the simulated
  loss drops reliable packets too).
- `net.network_session.add_local_gamer` (AddLocalGamer in the lobby; AllowJoinInProgress during
  play), `net.network_machine.remove_from_session` (standalone: the host removes a player's machine,
  RemovedByHost) and the manual `gamer_services.signed_in_gamer.signed_out` (SignedOut through the
  Guide). `CnaDemos/StandaloneProgram.hpp` holds what the C++ standalone programs share; the two
  earlier net programs use it.
- `audio.sound_effect.from_stream` (a loose WAV loaded at run time; CNA-DEMOS-ISSUE-029:
  `SoundEffect::FromStream` takes a `std::istream`, not a `System::IO::Stream`),
  `game.components.change_events` (EnabledChanged, UpdateOrderChanged, VisibleChanged,
  DrawOrderChanged, the OnEnabledChanged hook) and `math.bounding_box.intersects` (box against
  boxes, spheres, rays and planes); `media.media_player.play_song` checks
  `MediaPlayer::GameHasControl` before starting its music.
- `game.window.orientation` (SupportedOrientations, CurrentOrientation, OrientationChanged, CNA's
  keyboard orientation emulation enabled before the device exists) and
  `graphics.shader_effect.vertex_displacement` (a heightmap sampled in a GLSL vertex shader through
  ShaderEffect::SetTexture); `game.graphics_device_manager.device_events` also follows the
  GraphicsDevice's own DeviceResetting/DeviceReset.
- `graphics.render_target.hdr_blendable` (additive light past 1.0 in an HdrBlendable target, read
  back as HalfVector4, shown at an exposure; new `Need::HalfFloatRenderTargets`),
  `audio.xact.streaming_wave_bank` (streaming WaveBank constructor, IsPrepared, IsInUse;
  `tools/xact_files.py` writes a streaming `music.xwb`) and
  `devices.display_info.scale_and_safe_area` (CNA::Devices::DisplayInfo), `audio.xact.cue_variables`
  (per-cue values, clamping, cue vs global variables; `demo.xgs` gains the cue variable
  Intensity). Recorded
  CNA-DEMOS-ISSUE-027 (SOFTWARE does not clamp source values above 1 before blending into an 8-bit
  target) and CNA-DEMOS-ISSUE-028 (a streaming WaveBank opens a relative path from the working
  directory).
- Two-process net demos (standalone): `net.network_session.find_and_join` (a second process finds
  the System Link session -- host gamertag, open slots, QualityOfService -- joins and leaves) and
  `net.network_session.host_leaves` (the host leaves twice: SessionEnded with HostEndedSession,
  then HostChanged with AllowHostMigration); CNA-DEMOS-ISSUE-026 (on Linux, a process that
  searched before a host started never receives its answers). CTest runs demos that need the
  network one at a time (`RESOURCE_LOCK`). `CnaDemos::UseOfflineDemoProfiles` shares the gamer
  services set-up with the standalone programs.
- Audit demos (4): `gamer_services.avatar.custom_animation` (a game's own IAvatarAnimation drawn
  by AvatarRenderer), `gamer_services.guide.notification_position` (Guide notifications at
  TopRight), `gamer_services.leaderboard.paging` (PageDown/PageUp, LeaderboardOutcome and TimeSpan
  columns; CNA-DEMOS-ISSUE-025: offline, columns written after the rating are lost) and
  `graphics.render_target.multiple_render_targets` (SetRenderTargets with two
  RenderTargetBindings and a two-output GLSL ShaderEffect).
- `game.services.graphics_device_service` (a component reaches the device through
  IGraphicsDeviceService) and `net.network_session.online_session_types` (Local vs PlayerMatch and
  Ranked without the online service).
- docs/COVERAGE_AUDIT.md: a decision for every XNA type without a demo (demo, shown in, environment,
  service, not meaningful); audit pass 2 names more types the demos already used (collections,
  converters, event arguments, MediaSourceType, VideoSoundtrackType).
- `graphics.effect.effect_interfaces`: IEffectMatrices, IEffectLights and IEffectFog on five stock
  effects; scene settings applied through the interfaces alone.
- Coverage audit, first pass: demos that already show an enumeration or type now name it in
  `.apis` (Keys, KeyState, EffectParameterClass/Type, DisplayOrientation, GamerZone, GameDefaults'
  enumerations, MicrophoneState, AvatarBodyType).
- `content.content_manager.manifest` requires XNA's own XNB readers only (a game's own reader is
  available once the game registers it).
- Content, audio and media demos (4): a custom ContentTypeReader<T> reading the game's own type
  from an .xnb (`tools/generate_assets.py` writes it), a ResourceContentManager over content
  embedded in the program (expected failure, ISSUE-012), a SoundEffect loop region, VideoPlayer
  Pause/Resume/IsLooped/Volume/IsMuted.
- Model demos (3): glTF morph targets driven by the file's weights animation, the cameras of a
  glTF scene in split screen, a glTF with an embedded texture. CNA-DEMOS-ISSUE-022 (a compiled glTF
  refers to an embedded texture file the build never writes) and ISSUE-023 (compiled models lose
  their cameras) recorded; both demos check the compiled model and then use the run-time import.
- Graphics demos (4): hardware instancing (DrawInstancedPrimitives with a per-instance world-matrix
  stream), several vertex streams (SetVertexBuffers), rendering into a RenderTargetCube (dynamic
  environment map), BasicEffect vertex colours with lighting.
- `knownIssueRenderers` metadata: a recorded defect can be limited to some renderers (expected
  failure there, a normal pass elsewhere; `--list-ids` prints `<issue>@R1,R2`, CTest follows it).
- CNA-DEMOS-ISSUE-021 recorded: the EasyGL renderers (OPENGLES3, OPENGL33) sample a rendered
  RenderTargetCube face upside down.
- C API demos (7, Phase 14): standalone C99 programs against CNA's C ABI (`CNA::CApi`) --
  lifecycle callbacks and the loop, error results and diagnostics, sprites from computed pixels,
  a ContentManager with texture and SpriteFont, keyboard/mouse state, a SoundEffect from PCM,
  vectors and matrices. `cna_demos_add_standalone(... C_API)`, `Need::CApi` (SKIPPED without the C
  API), the `CnaDemos/CApiDemo.h` scaffolding (checks, diagnostics, PNG capture), and the
  `linux-full` preset now builds CNA with `CNA_BUILD_C_API=ON`. Standalone demos receive
  `--screenshot`. docs/COVERAGE.md counts C ABI routes against CNA's C headers.
- `gamer_services.avatar.animation_presets`: 12 frames starting mid-clip (CI timed it out at 90
  frames on SOFTWARE under parallel load) and the avatars now face the camera.
- Media demos (2): `media.video_player.play_video` plays a generated WMV and
  `media.song.compiled_song` a generated Ogg Vorbis tune, both compiled by the content pipeline
  (`content_media/` into `assets/media`, compiled only when CNA has its FFmpeg backend;
  `tools/media_files.py` draws the frames and computes the audio, ffmpeg encodes them).
  `Need::Video` now skips without CNA's video backend.
- CNA-DEMOS-ISSUE-020 recorded: a compiled Song/Video stream is resolved from the ContentManager's
  root, not beside the asset.
- Coverage report: a bare XNA type name in `.apis` (an enumeration, an exception type) now credits
  that type.
- Phase 13 demos (17): devices (sensor availability, keyboard-emulated accelerometer,
  VibrateController, CNA::Devices machine information), phone (PhoneApplicationService lifecycle),
  design (TypeConverter text, cultures, property descriptors, InstanceDescriptor), diagnostics
  (modes, counters and gauges), core logging (sink, levels), cnaext (ASCII post-process, CRT effect,
  colour depth reduction, DebugDraw).
- Game demos (4): ResetElapsedTime after a load, GameTime constructors, CNAEXT file/text drop, focus
  and pausing (IsActive, OnActivated/OnDeactivated).
- CNA-DEMOS-ISSUE-019 recorded: `Game::ResetElapsedTime` is ignored with a fixed time step.
- Net demos (6): a Local session of two players (GamerJoined replayed on subscription, host, ids,
  machine), the lobby/ready/StartGame/EndGame flow, NetworkSessionProperties, BeginCreate/EndCreate,
  game objects in Gamer::Tag, and a PacketWriter -> SendData -> ReceiveData -> PacketReader round
  trip in a System Link session (`Need::Network`, the only demo that opens sockets).
- CNA-DEMOS-ISSUE-018 recorded: System Link socket failures surface as `std::runtime_error`
  instead of `NetworkNotAvailableException`.
- Avatar demos (4): AvatarRenderer with the Wave preset and its loading state, a custom pose from
  AvatarBone transforms with an AvatarExpression and the skeleton (ParentBones, BindPose),
  AvatarDescription bytes round trip and invalid descriptions, six animation presets side by side.
- Leaderboard demo: two local players write scores during a LocalWithLeaderboards session and
  LeaderboardReader reads the board and a page around one player; the `gamer_services` category
  links CNA::Net for it. `GamerServicesDemo` takes the names of several local players.
- `ownProcess` metadata and `--batch-child`: demos that set up once-per-process CNA state (gamer
  services) run as `cna-demos <id>` in a process of their own from batch runs and the browser.
- `CnaDemos/GamerServicesDemo.hpp` (offline configuration, the demos' own profile store, "Demo
  Player" signed in, the `GamerServicesComponent`) and `CnaDemos/GamerServicesNames.hpp`.
- `title/` and the `cna_demos_title_files` target: files CNA reads by a fixed title-relative path,
  starting with the offline achievement catalog `GamerServices/Achievements.json`.
- `cna-demos` names `libcna.so` first on its link line, so CNA's static gamer services library
  cannot pull a second copy of CNA into the executable (CNA-DEMOS-ISSUE-016, double free at exit).
- CNA-DEMOS-ISSUE-016 (linking `CNA_GamerServices` after another library duplicates CNA) and
  CNA-DEMOS-ISSUE-017 (`Achievement::EarnedOnline` is always true) recorded.
- `CnaDemos/InputNames.hpp`: names of input enums (keys, buttons, gestures) for demo output.
- CNA-DEMOS-ISSUE-008 recorded: sharp-runtime's `Prop.hpp`, reached from CNA's public headers,
  defines macros named `YES`, `NO`, `nothing` and similar.
- Custom shader demos (2): toon shading with a GLSL `ShaderEffect`, a grayscale sprite shader
  through `SpriteBatch::Begin`; `ColorMatrixEffect` sepia/negative on SOFTWARE.
- CNA-DEMOS-ISSUE-007 recorded: GPU renderers silently ignore `ColorMatrixEffect`.
- `CnaDemos/Geometry.hpp`: sphere and cube meshes with normals and UVs for 3D demos.
- Screenshots are saved opaque (what the window shows): XNA's SaveAsPng, which CNA matches,
  stores fully transparent pixels as (0, 0, 0, 0).
- CNA-DEMOS-ISSUE-006 recorded: no capability query tells a program that GL ES renderers ignore
  `MipMapLevelOfDetailBias` (documented CNA/FNA3D limitation).
- CNA-DEMOS-ISSUE-005 recorded: CNA's `Color` is polymorphic (16 bytes), so XNA-style custom
  vertex structs containing a `Color` silently draw garbage.
- CNA-DEMOS-ISSUE-003 (SpriteBatch leaves Textures[0] unset) and CNA-DEMOS-ISSUE-004 (Immediate
  SpriteBatch ignores sampler states set between Draws) recorded.
- Screenshots of a demo running under Reach are reported as unavailable instead of failing the run.
- `tools/contact_sheet.py`: captures Game demos on a renderer into one labelled image for review.
- `deterministic` now means "identical capture on every machine"; GameTime-driven animation is
  excluded (catch-up Updates make game time at frame N machine-dependent).
- The runner calls `Dispose()` after `Run()` (XNA's `using` pattern), so demos' `UnloadContent`
  runs while the demo object is alive.
- CNA-DEMOS-ISSUE-002 recorded: `Curve::Evaluate` with Step keys compares the absolute position
  with 1.0.
