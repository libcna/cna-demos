# Changelog

All notable changes to cna-demos. The format follows [Keep a Changelog](https://keepachangelog.com/);
versions follow Semantic Versioning (pre-1.0: a minor version may change the CLI or metadata).

## [Unreleased]

### Validation
- Extended the PBR specular-material scene with dual-UV selection and independent texture
  transforms for both specular strength and colour maps.
- Extended the PBR texture-map scene with exact normal-scale and occlusion-strength behavior at
  one and zero.
- Added exact PBR base-color, emissive and specular-color sRGB decode checks plus final output
  encoding with unchanged alpha.
- Added a focused dual-UV PBR scene with independent base-color/emissive coordinate selection and
  full scale-rotate-translate texture transforms, verified through exact sampled pixels.
- Added a focused PBR dielectric-specular scene covering IOR, scalar/color factors and the
  `KHR_materials_specular` strength/color maps with exact controlled-environment pixels.
- Extended the imported skinned-PBR model with a complete image-based-light bundle and verified
  sampled green irradiance on the unchanged animated pose.
- Added a focused PbrEffect image-based-lighting scene with actual diffuse irradiance, intensity,
  BRDF and roughness-mip pixel evidence plus invalid-bundle ambient fallback.
- Classified all 127 remaining public C ABI routes by semantic disposition: fixed reflection,
  explicit test plumbing, manual/platform/service observation, or the unsupported content-resource
  placeholder; all 3205 routes now have a decision without inflating demonstrated coverage.
- Completed the C gamepad, keyboard and touch value/property route audits by adding fixed state
  text, mouse-to-touch emulation and opaque touch-window binding to existing input workflows.
- Added a standalone C renderer-selection workflow that performs a real absent-backend fallback,
  inspects selected versus active identity, latch state and exact diagnostic history.
- Added a renderer-independent C metadata workflow covering all public renderer categories,
  maturities and their stable names plus the compiled-in renderer identity/name contract.
- Added a C DebugDraw workflow covering all 15 remaining `graphics_ext.h` routes, including depth
  list routing, exact copied geometry, shape segment clamps and real end-of-frame drawing.
- Made the coverage report state `none` when every public C header has a demonstrated route.
- Added one C CRT/Depth/ASCII workflow covering 24 more `graphics_ext.h` routes, including scalar
  clamps/refusals, every depth/dither mode and two real ASCII quantize/draw grid sizes.
- Added a C image-based-light value workflow covering the first three `graphics_ext.h` routes and
  exposing wrong-kind live handles being accepted as textures (CNA-DEMOS-ISSUE-091).
- Classified the remaining XACT RPC/variation observation limits and all seven generated Audio
  member gaps, preserving a dedicated RPC-release timing fixture as the meaningful follow-up.
- Added a deterministic category ReplaceLowestPriority fixture and proved priority-based eviction
  independently of playback order through public Cue states.
- Added a deterministic XACT cue-level ReplaceOldest fixture demonstrating FACT's definition-local
  count and bank-wide oldest-victim selection through public Cue states.
- Added an authored read-only XACT global and demonstrated that SetGlobalVariable silently preserves
  its initial value, with all generated XGS variants and XACT regressions updated together.
- Extended XACT Cue lifecycle coverage with repeated Play behavior in playing, paused and stopped
  states plus asymmetric transport behavior after disposal.
- Extended XACT global variables with lower-bound clamping and symmetric empty/unknown-name
  contracts for GetGlobalVariable and SetGlobalVariable.
- Extended XACT cue variables with lower-bound clamping, bidirectional namespace/name refusals and
  disposed-cue GetVariable/SetVariable behavior.
- Added an XACT category timing control showing that Pause affects cues active at the call and does
  not persist onto cues started later.
- Demonstrated that WaveBank's streaming offset and packet-size inputs are compatibility arguments
  in CNA/FNA's standard file path, including unusual values on a valid bank.
- Added XACT WaveBank/SoundBank null, empty and missing-input boundaries plus CNA's corrupt-bank
  and missing-stream stub behavior to the existing bank lifecycle workflow.
- Extended the XACT bank/engine lifetime workflow with AudioEngine post-dispose renderer state,
  category/global-variable refusals and the safe Update no-op.
- Extended the XACT renderer workflow with AudioEngine construction boundaries for empty, missing
  and existing invalid settings files plus CNA's arbitrary-renderer single-backend fallback.
- Added SoundEffect parent move construction/assignment semantics, including active-child registry
  transfer, moved-to disposal cascades and non-disposing replacement of the old wrapper resource.
- Completed authored SoundEffect loop-coordinate validation for an accepted exact-end region plus
  past-end, negative-start and negative-length refusals in the existing playback workflow.
- Extended SoundEffect FromStream with current-position consumption, decoded lifetime beyond the
  source stream, and uniform empty/non-audio/truncated NotSupportedException handling.
- Deepened raw SoundEffect PCM construction with a nonzero aligned window, XNA helper rounding and
  whole-frame sizing, invalid-format zeros, and distinct range versus format exceptions.
- Distinguished SoundEffect fire-and-forget behavior for clamped pitch, rejected pan and disposed
  false returns, including the disposed CreateInstance exception contrast.
- Completed SoundEffect MasterVolume scalar contracts with exact endpoints, finite/NaN refusals,
  atomic preservation and explicit restoration of the process-global value.
- Completed AudioEmitter DopplerScale state boundaries with positive and zero acceptance, negative
  refusal with preservation, and restoration before the existing spatial sweep.
- Exercised Apply3D collection and pointer/count overloads with two listeners plus distinct empty,
  null, zero and negative input refusals that preserve public instance state.
- Added SoundEffect spatial-global range and restoration behavior for DistanceScale, DopplerScale
  and SpeedOfSound without claiming an acoustic result from dummy audio.
- Extended SoundEffectInstance disposal coverage across Volume, Pitch, Pan and the one-way
  IsLooped mutation lock, including preservation of every refused property value.
- Completed SoundEffectInstance Volume/Pitch/Pan scalar boundaries with exact inclusive endpoints,
  NaN refusals and preservation of all three values after finite and nonfinite failures.
- Verified DynamicSoundEffectInstance BufferNeeded subscriber independence: a one-shot listener
  removes itself during dispatch without suppressing three callbacks to the refill listener.
- Added an isolated DynamicSoundEffectInstance starvation-recovery control: an empty playing
  stream emits exactly three canonical BufferNeeded requests and synchronously reaches three
  pending buffers while remaining Playing.
- Extended XACT bank disposal with valid-name GetCue and PlayCue refusals after Dispose while
  retaining exactly-once disposal events and idempotent repeated cleanup.
- Added XACT SoundBank input boundaries for empty and unknown names through both GetCue and PlayCue,
  followed by successful valid playback to prove retry safety.
- Completed SoundEffectInstance lifetime edges: disposed Resume refusal with safe Pause/Stop, and
  move assignment that correctly transfers live playback and parent-disposal registration.
- Extended SoundEffectInstance transport behavior before first playback and after Stop, and used a
  long track to prove repeated Play while Playing preserves progress instead of restarting.
- Added disabled MediaPlayer visualization behavior: nonzero caller buffers are actively cleared
  before the existing enabled-capture spectrum path runs.
- Verified repeated Pause, Resume and Stop calls in their matching states are quiet no-ops that do
  not add deferred MediaPlayer events or disturb the established position semantics.
- Added MediaPlayer Volume boundary clamping and corrected the mute assertion to claim only the
  stored state observed under dummy audio, while retaining MasterVolume isolation.
- Completed the MediaQueue mutation workflow on an isolated queue: empty construction, ordered
  ownership through Add, explicit active-index selection and Clear restoring the empty state.
- Added bounded automatic MediaPlayer queue progression: completion advances from the first Song
  to the second, then the final nonrepeating entry stops without wrapping or changing its index.
- Deepened MediaPlayer event coverage with exact deferred active-song/state ordering, pre-pump
  state coalescing, canonical sender/arguments and verified listener removal.
- Extended the deferred MediaPlayer transport workflow with measured paused-position stability,
  resumed progress and exact Stop reset before replaying through the existing natural-end path.
- Exercised actual shuffled MediaPlayer traversal with a bounded sequence that verifies every
  selected index, active Song, queue membership and Playing state without assuming random order.
- Extended the MediaPlayer queue workflow across both collection overloads: the default overload
  starts at zero, while a replacement collection starts at a requested nonzero entry and remains
  queue-owned after its caller objects are destroyed.
- Closed the custom XNB writer policy audit by proving the default writer accepts requested LZX
  compression and that the runtime restores its compressed payload; the empty writer attribute is
  now explicitly dispositioned as a registration marker with no independent behavior.
- Added XNA ContentCompiler inventory and post-compile freeze checks, and dispositioned its
  implementation-facing worker/active-writer helpers plus ContentWriter's no-op façade members.
- Added direct custom XNB compiler refusals for raw null references, unregistered dynamic types and
  explicit nested writers owned by another compiler.
- Added custom XNB null-reference coverage for ordinary objects, shared resources and external
  references, including the absence of deferred fixups.
- Extended the real custom XNB graph with derived reference dispatch, an independent inline copy,
  deduplicated shared-resource fixups and a normalized external-reference path.
- Extended the custom XML-to-XNB writer with identity/lifecycle, requested target context,
  compression refusal and explicit typed/raw nested writer paths, exposing CNA-DEMOS-ISSUE-090:
  `ContentTypeWriter::Initialize` deadlocks when it retrieves a nested writer as documented.
- Closed the custom intermediate serializer identity audit by tying its registered instance to the
  exact `TargetType` and `XmlTypeName` used by lookup and emitted XML.
- Added custom intermediate serializer contracts for `ObjectIsEmpty` optional omission and
  `CanDeserializeIntoExistingObject` identity preservation through a read-only property.
- Exercised custom intermediate serializer child scanning with a nested custom type and exposed
  CNA-DEMOS-ISSUE-089: registered serializers never receive their documented Initialize callback.
- Extended the custom intermediate serializer with a typed nested envelope that writes and reads
  dynamic type metadata through the public reader/writer context, including missing, unknown and
  unrelated-type refusals.
- Added the public intermediate-writer depth boundary: a non-shared self-cycle is refused at
  `MaxNestingDepth` with an actionable diagnostic instead of recursing without a bound.
- Extended shared-resource intermediate XML coverage with XNA's root self-cycle behavior: the
  root is emitted again as one resource, deserializes as a distinct copy, and closes the cycle on
  that resource identity with explicit C++ ownership cleanup.
- Extended custom intermediate serializer registration coverage with same-target first-wins
  behavior and CNA-DEMOS-ISSUE-088: a conflicting XML shortcut is accepted into a partial,
  non-round-trippable registry mapping.
- Extended intermediate XML serialization with reusable format-name fallback and optional empty
  collection omission while preserving the reader object's nonempty constructor default.
- Extended the XNA component-adapter workflow with target-platform and build-configuration
  propagation plus real intermediate-directory scratch use across importer, processor and writer.
- Extended the native pipeline example with deterministic importer, processor and writer
  ambiguity diagnostics, explicit incompatible-route refusals, and name-only component selection.
- Extended CNB external-reference coverage with a valid child file of the wrong registered type,
  parent nonpublication, and same-manager repair/retry of both child and parent.
- Extended the custom CNB loader with authoring-time canonical-name requirements, direct
  `RequireAsset` type refusal and a real 31-bit hash collision rejected before factory dispatch.
- Completed the CNB read-limit audit with aligned-chunk publication and symmetric writer/reader
  checks for chunk-count, chunk-alignment, CMET string and XREF row-count budgets.
- Extended the XNB shared-resource graph with supplied ReadAsset recorder coverage, exposing
  CNA-DEMOS-ISSUE-087: disposable shared-table values bypass disposal recording.
- Extended ReadAsset ownership coverage with repeated-identity callback delivery and caller
  deduplication, partial-read resource retention, and callback-exception unwinding.
- Completed the custom CNJ registration audit with empty-name/factory refusals and fail-fast
  rejection when a built-in content reader already owns the requested C++ asset type.
- Extended the custom CNJ loader with unknown-type and future-version envelope refusals that occur
  before factory invocation, plus same-manager repair/retry proof that failed loads are not cached.
- Extended custom CNB loader registration checks with empty-factory, built-in/reserved identifier
  and canonical-name/hash mismatch refusals that preserve the installed loader.
- Extended the custom CNB loader with executed first-registration-wins precedence, ignored-factory
  capture release and retained-factory lifetime across ContentManager replacement.
- Extended ContentManager lifetime coverage with protected Dispose-hook dispatch, IDisposable and
  repeated-disposal semantics, disposed ReadAsset refusal, and runtime evidence for ISSUE-010's
  wrong disposed-Load exception type.
- Extended the RenderTarget2D lifetime checks with bound C++ destruction, stale-binding removal,
  explicit back-buffer restoration and same-address replacement rendering.
- Extended the RenderTargetCube workflow with bound-face disposal refusal, exact binding identity
  retention and post-refusal face read-back.
- Extended the RenderTarget2D workflow with atomic bound-target Dispose/move-assignment refusal,
  active-binding transfer through move construction and assignment, and post-move pixel read-back.
- Extended Texture2D ownership checks with cross-device copy assignment, shared partial edits,
  full-upload copy-on-write detachment and copy assignment that revives a disposed target wrapper.
- Closed the TextureCube wrapper-lifecycle audit with cross-device copy assignment, shared edits,
  active-binding move transfer and device-owned disposal; the new framebuffer check exposed
  CNA-DEMOS-ISSUE-086, where destroying a secondary EasyGL device blacks out the primary device.
- Closed the Texture3D behavior audit with cross-device move assignment, transferred active
  bindings, DXT volume refusal, the HiDef extent ceiling and the Reach-profile prohibition.
- Extended Texture3D transfers with raw slice-major caller windows, atomic element-width refusal,
  bound-resource upload rejection and an active binding that follows move construction.
- Extended the Texture3D sampler with exact U/V orientation and authored explicit-LOD checks that
  distinguish point mip selection from linear cross-level interpolation.
- Completed the 636-demo full rebuild/runtime checkpoint against clean CNA `d23e139de`: all 649
  enabled CTests and both renderer smokes pass with every Complete735 status and failure unchanged.
- Serialized four CTests that mutate shared pointer, clipboard or screen-saver state so parallel
  runs cannot race between mutation, observation and restoration.
- Made the C API error-handling callback probe variable-step so each explicit RunOneFrame call
  deterministically reaches one planned Update callback after its diagnostic setup.
- Extended the cube ShaderEffect scene with authored Lime/Blue mip levels and exact LOD 1.25
  checks distinguishing point mip selection from linear cross-level interpolation.
- Added direct cube-face orientation checks: four authored +X corner texels are sampled at their
  exact GLSL direction-vector centers while the existing center and rebinding controls remain.
- Extended the ShaderEffect cube-sampler workflow with an observable same-unit texture replacement:
  retained red and rebound white regions prove the second TextureCube replaces the first binding.
- Added a stock-effect SpriteBatch example whose pixel checks prove AlphaTestEffect Projection and
  BasicEffect World matrices replace ordinary sprite pixel-space placement on both renderers.
- Classified compiled/multi-pass SpriteBatch effects as build- and renderer-specific: the current
  SOFTWARE and EasyGL compiled-effect options are disabled, while the valid `.fxb` refusal path is
  already exercised by the C API effect lifecycle demo.
- Extended SpriteBatch sorting with XNA's observable unstable equal-depth ordering and paired
  depth-disabled/enabled checks proving layer depth reaches the depth buffer as vertex Z.
- Added compiled SpriteFont framebuffer checks for exact CRLF/LF LineSpacing placement and visible
  glyph ink under all four basic text tints.
- Reworked transformed SpriteFont examples into framebuffer checks for per-axis layout scale,
  centre-origin rotation, and exact whole-string horizontal/vertical/combined mirroring.
- Extended SpriteBatch layer-depth sorting with mixed DrawString/Draw framebuffer checks proving
  that nonzero text depth enters the same sorted queue independent of submission order.
- Completed the 635-demo full rebuild/runtime checkpoint: all 648 enabled CTests and both full
  renderer smokes pass, every Complete724 result remains exact, and the new cross-device workflow
  passes on both renderers.
- Added framebuffer checks to `graphics.sprite_batch.sprite_effects` for all four marker corners,
  horizontal arrow direction and invariant destination bounds under combined flips.
- Added framebuffer checks to `graphics.sprite_batch.scale` that correlate authored marker spacing
  with scalar half/native/triple sizes and independent horizontal/vertical Vector2 stretches.
- Reworked `graphics.sprite_batch.rotation` into deterministic cardinal-angle/origin examples with
  framebuffer checks for fixed pivots, clockwise orientation and top-left versus centre rotation.
- Added `graphics.sprite_batch.cross_device_texture`, a two-device ownership workflow proving that
  foreign textures are rejected at Draw without poisoning the receiving SpriteBatch.
- Extended `graphics.sprite_batch.begin_end_rules` with disposed-texture Draw rejection and
  proof that the same deferred batch remains usable after the refusal.
- Added exact-bound and normalized-pattern framebuffer checks to
  `graphics.sprite_batch.destination_rectangle` for uniform and nonuniform scaling.
- Added representative framebuffer checks to `graphics.sprite_batch.tint_and_transparency` for
  opaque RGB multiplication and premultiplied opacity over a known background.
- Added exact framebuffer checks to `graphics.sprite_batch.source_rectangle` for four atlas frame
  selections, transparent texels and the whole-strip `std::nullopt` path.
- Extended `graphics.texture2d.from_stream` with exact transparent palette-GIF decoding and
  deterministic truncated-image refusal, completing the classic PNG/JPEG/GIF container set.
- Extended `graphics.texture2d.set_data_rectangle` with atomic invalid-rectangle, short logical
  transfer and disposed-resource rejection for both upload and readback.
- Completed the 634-demo full rebuild/runtime checkpoint: all 647 enabled CTests and both full
  renderer smokes pass with only 36 declared CNA failures and no prior status drift.
- Added `graphics.texture2d.normalized_formats`, a ten-check packed texture workflow covering exact
  signed/unsigned normalized storage, channel expansion, two-bit alpha and alpha-only sampling.
- Added `graphics.texture2d.floating_formats`, a 15-check binary32/binary16 texture workflow
  covering exact one-, two- and four-channel storage, element-width refusal and sampled channel
  expansion/clamping.
- Extended the owned graphics-device lifecycle with the three-argument `Present` contract:
  equivalence, source clipping, invalid-region refusal and renderer-specific destination/window
  support are now checked on SOFTWARE and OPENGLES3.
- Completed the 632-demo full rebuild/runtime checkpoint against clean CNA `0c83dcc9dc`: all 645
  enabled CTests and both full renderer smokes pass with only36 declared CNA failures.
- Credited the existing `GraphicsDeviceStatus` behavior in the owned-device lifecycle demo and
  dispositioned all27 remaining generated type gaps without synthetic demo inflation.
- Added `c_api.graphics.sprite_font_state`, a26-check caller-built SpriteFont workflow covering
  copied glyph/character tables, atomic short reads, asymmetric layout measurement, mutable
  fallback/spacing state and atlas retention; this completes all nine `sprite_font.h` routes.
- Completed the524-demo texture checkpoint: metadata524/0, all537 enabled CTests pass, all three
  new texture IDs pass under both full smokes, and every complete521 status/known-issue pair remains
  exact.
- Added `c_api.graphics.texture_volume_data`, a19-check C workflow covering Texture3D box and raw
  byte transfers, all six TextureCube face payloads, atomic short reads and in-memory cube DDS
  decoding.
- Added `c_api.graphics.texture2d_io`, a27-check C workflow covering standalone and device-backed
  Texture2D construction, storage identity, exact full/rectangle transfers, atomic PNG/JPEG output,
  file round-tripping and encoded-memory resizing.
- Added `c_api.graphics.texture_formats`, a deterministic six-check C audit of every public
  SurfaceFormat's compression footprint, storage-unit size, pixel alignment, compatible transfer
  widths and renderer-independent base Texture support policy.
- Extended `c_api.media.media_player` to demonstrate `cna_media_player_update_ext` as an explicit
  post-loop maintenance pump: a real short track finishes while public state remains Playing, and
  the pump consumes the native end notification and transitions to Stopped.
- Completed the521-demo full checkpoint: metadata521/0, all534 enabled CTests pass, both renderer
  smokes contain every demo, and all61 failures are declared CNA issues with no unexpected pass.
- Extended `c_api.content.content_manager` with an authored BasicEffect CNJ, exact material-state
  checks, ordinary Effect application and owned usability after both manager unload and destruction;
  this closes the final `effects.h` C route.
- Added `c_api.graphics.shader_effect_textures`, a20-check C workflow covering 2D/cube/3D sampler
  family validation, same-unit replacement and ownership retained through a surviving pass view.
- Added `c_api.graphics.shader_effect_uniforms`, an18-check C workflow covering all loose typed
  uniform setters, tightly packed float/vec2/vec3/mat4 arrays, malformed inputs and portable std140
  block declarations against active shader uniforms.
- Added `c_api.graphics.shader_effect_state`, a17-check C workflow covering source construction,
  canonical validity/renderer ownership, empty successful diagnostics and independent World/View/
  Projection matrix state across executing and non-executing renderer contracts.
- Added `c_api.graphics.skinned_pbr_effect_state`, a32-check C workflow covering SkinnedPbrEffect's
  shared PBR/interface compatibility, weight modes, copied 72-bone palette and texture/material/
  palette survival through cloning and source destruction.
- Added `c_api.graphics.pbr_texture_slots`, a34-check C workflow covering all seven PBR texture
  slots, UV selectors, transforms, meaningful sRGB flags and retained ownership through cloning.
- Recorded CNA-DEMOS-ISSUE-071: the public C texture getter says five slot identities although the
  same header and implementation expose seven.
- Added `c_api.graphics.pbr_effect_material_state`, a33-check C workflow covering every non-texture
  PbrEffect material route, shared effect interfaces and clone lifetime; four retained failures
  establish CNA-DEMOS-ISSUE-070 for lost vertex-color and alpha-coverage clone state.
- Recorded CNA-DEMOS-ISSUE-070: PbrEffect and SkinnedPbrEffect copy constructors omit four public
  draw-affecting properties that their other material state preserves.
- Added `c_api.graphics.color_matrix_effect_state`, a22-check C workflow covering every
  ColorMatrixEffect route, finite matrix/offset state, atomic non-finite refusal, the exact Rec. 709
  grayscale preset, reset and type-preserving clone survival after source destruction.
- Added `c_api.graphics.skinned_effect_state`, a35-check C workflow covering every SkinnedEffect
  route, required lighting, the one/two/four-weight contract, atomic 72-bone palette boundaries and
  texture/palette ownership across cloning and source destruction.
- Added `c_api.graphics.environment_map_effect_state`, a30-check C workflow covering every
  EnvironmentMapEffect route, required lighting/default lights, shared matrix/fog state and
  separate Texture2D/TextureCube ownership across cloning and source destruction.
- Added `c_api.graphics.unlit_stock_effects`, a33-check C workflow covering every AlphaTestEffect
  and DualTextureEffect route, their shared matrix/fog interfaces and retained texture ownership
  through independent slots, clones, source destruction and final release.
- Added `c_api.graphics.basic_effect_state`, a40-check C workflow covering BasicEffect material,
  matrices, fog, lighting, same-device texture retention and standalone/nested DirectionalLight
  ownership, including stable member views that outlive their root Effect handle.
- Added `c_api.graphics.effect_concrete_types`, a22-check C workflow for EffectMaterial texture
  ownership and inherited base cloning plus SpriteEffect stock identity and type-preserving cloning.
- Recorded CNA-DEMOS-ISSUE-069: the generic C Effect clone comment promises concrete-type
  preservation even though canonical `EffectMaterial.Clone()` deliberately returns a base Effect.
- Added `c_api.graphics.effect_core`, a27-check C workflow covering the18 core Effect routes:
  owned reflection and pass application, current-technique rules, clone/disposal isolation,
  retained child views, constructor-format refusals and capability-gated compiled bytecode.
- Added `c_api.graphics.effect_passes_techniques`, a27-check C workflow covering all31 ownerless
  pass/technique value and collection routes, including reflected indices and nested lifetimes.
- Added `c_api.graphics.effect_parameter_collections`, a19-check C workflow covering stable shared
  parameter views, name/semantic lookup and descendant lifetimes across nested collections.
- Added `c_api.graphics.effect_parameter_values`, a30-check C workflow covering mutable parameter
  metadata, all tagged scalar and array families, string-buffer contracts and typed texture
  retention/release without creating a graphics device.
- Recorded CNA-DEMOS-ISSUE-068: the C parameter-collection header calls semantic lookup exact even
  though its documented C++ target deliberately matches ordinally without case.
- Added `c_api.graphics.effect_annotations`, a21-check C workflow covering all22 immutable
  annotation and collection routes, including copied UTF-8/raw values, typed views, exact lookup,
  collection value copies and independent owned lifetimes.
- Complete504-C-graphics-device passes all517 active CTests and both full smoke suites; the two new
  IDs have their expected outcomes, every complete502 status and known issue remains exact, and
  only SOFTWARE's documented ISSUE-066 assertion is added.
- Serialized `c_api.input.keyboard_mouse` in CTest because its global pointer warp/read/restore
  sequence shares desktop state with every concurrent SDL process; the targeted schedule and full
  checkpoint now pass.
- Extended `c_api.graphics.owned_device` from20to23 controls with resource-free renderer
  reconstruction, applied 4x/single-sample state, capability stability and no reset-event leakage.
- Extended `c_api.graphics.device_capabilities` from20to24 controls with exact render-target-format
  verdict agreement, effect-source/IBL execution truth and restored display-color-space probing.
- Recorded CNA-DEMOS-ISSUE-067: the public C display-color-space routes expose bare integer
  ordinals without a C identity type, named constants or an ordinal mapping.
- Extended `c_api.graphics.render_target_bindings` from22to25 controls with explicit device-wide
  ContentLost delivery, exact per-resource lost flags and selective restoration by rebinding.
- Added `c_api.graphics.occlusion_query`, a23-check C workflow for the complete eight-route owned
  query lifecycle, invalid state ordering, bounded completion and exact-versus-boolean visible
  sample results.
- Added `c_api.graphics.draw_routes`, a24-check pixel-readback workflow for bound, instanced,
  caller-vertex and caller-indexed primitives plus explicit effect, blend, depth-test and depth-write
  control.
- Recorded CNA-DEMOS-ISSUE-066: SOFTWARE's direct blend-enable route is a no-op, so repeated
  half-alpha geometry overwrites instead of accumulating while OPENGLES3 blends it correctly.
- Complete502-C-device-policy passes all515 enabled CTests and both full smoke suites; both new
  graphics IDs pass and every complete500 status, known issue and failed assertion remains exact.
- Extended `c_api.graphics.device_states` from22to25 controls with graphics-profile and
  unsupported-3D-policy round trips, atomic invalid-enum refusal and exact restoration.
- Added `c_api.graphics.owned_device`, a20-check two-device lifecycle workflow for caller ownership,
  presentation, both reset forms, ordered callbacks, independent destruction and stale handles.
- Recorded CNA-DEMOS-ISSUE-065: the public graphics-device header separates the reset declaration
  from its documentation with the complete owned-device create/destroy declarations.
- Added `c_api.graphics.device_clears`, a24-check pixel-readback workflow for normalized RGBA,
  Color/depth and selected color/depth/stencil clears plus exact offset-window, refusal and
  empty-mask preservation.
- Complete500-C-buffer-bindings passes all513 enabled CTests and both full smoke suites; all three
  new graphics IDs pass and every complete497 status, known issue and failed assertion remains exact.
- Added `c_api.graphics.buffer_bindings`, a20-check C workflow for single and ordered multi-stream
  vertex bindings, atomic copy/preflight, index binding and destruction-driven device cleanup.
- Added `c_api.graphics.texture_slots`, a16-check C workflow for exact pixel-stage slot identity,
  selective and whole-device unbinding, idempotence and destruction cleanup.
- Extended `c_api.graphics.resource_lifecycle` from25to30 controls with synchronous
  graphics-device resource creation/destruction payloads, tracked-count transitions and
  registration unsubscription behavior.
- Extended `c_api.graphics.device_states` from19to22 controls with live device identity and
  ownership checks plus exact scissor-rectangle, blend-factor, multisample-mask and
  reference-stencil save/apply/read/restore behavior.
- Added `c_api.graphics.viewport`, a19-check C workflow for all11 Viewport value routes plus exact
  live-device set/get/restoration, projection round trips and string-buffer atomicity.
- Recorded CNA-DEMOS-ISSUE-064: `cna_viewport_init` correctly produces the all-zero XNA value,
  while its public C comment incorrectly promises a maximum depth of one.
- Complete497-C-sprite-states passes all510 enabled CTests and both full smoke suites; the five new
  graphics IDs pass and every complete492 status and failed assertion remains exact.
- Extended `c_api.graphics.sprite_batch` from43to61 controls with pixel-proven explicit-state and
  transformed default-effect Begin routes, completing all14 `graphics_state.h` routes.
- Added `c_api.graphics.device_states`, a19-check audit of every graphics-state preset plus exact
  live-device blend, depth/stencil, rasterizer and independent pixel/vertex sampler round trips.
- Added `c_api.graphics.index_buffer`, an18-check dynamic 16-bit transfer, read-back, partial-update
  and ContentLost-lifetime workflow that gives every `index_resources.h` route a semantic disposition.
- Corrected the C adapter snapshot's widescreen cross-check to use XNA's strict `> 1.6` boundary,
  including an exact 16:10 mode.
- Added `c_api.graphics.resource_lifecycle`, a25-check C workflow covering all12 generic
  GraphicsResource ownership, name/string, tag, disposal and event-lifetime routes.
- Extended `c_api.graphics.sprite_batch` from30to43 controls with position/source/origin/nonuniform-
  scale geometry, whole-array preflight and texture retention for `cna_sprite_batch_submit_scaled_many`.
- Added `c_api.graphics.device_capabilities`, a20-check C audit of all ten remaining
  graphics-device identity, capability-profile, report, shader-dialect and backbuffer routes.
- Added `c_api.graphics.render_target_bindings`, a22-check C workflow covering real2D/cube MRT
  state, atomic binding read-back and the silent ContentLost lifecycle, completing `render_target.h`.
- Complete492-C-device-presentation passes505 enabled CTests and both full smoke suites; the two
  new display demos pass and every complete490 status and failed assertion remains exact.
- Added `c_api.graphics.device_presentation`, a12-check live-device C workflow that changes,
  verifies and exactly restores presentation state, completing all22 `display.h` routes.
- Added `c_api.graphics.adapter_capabilities`, a16-check C workflow covering all six adapter
  capability, format, preference and explicit-limitation routes with exact preference restoration.
- Complete490-C-adapter-snapshot clears the sixteen-batch checkpoint debt:503 enabled CTests and
  both full smoke suites preserve every prior outcome, with only the two documented ISSUE-062
  assertions added by the new sensor-lifecycle demo.
- Aligned cna-demos capability metadata and the capability-listing demo with the17 capability
  values currently exposed by CNA, removing two stale unused placeholders that blocked a full
  rebuild.
- Added `c_api.graphics.adapter_snapshot`, a15-check C workflow covering all seven read-only
  adapter enumeration, exact UTF-8 metadata and filtered display-mode routes before the first
  frame, including atomic short-buffer and upper-index refusal.
- Added `c_api.graphics.display_values`, an eight-check renderer-independent C workflow covering
  all five `DisplayMode` and `PresentationParameters` value routes, including native clone
  treatment of CNA's non-XNA headless extension and refusal before output mutation.
- Extended `c_api.input.clipboard` so the device-layer write reports actual platform acceptance
  before the existing exact read-back and desktop-clipboard restoration workflow.
- Added `c_api.devices.camera_frames`, a23-check C workflow covering14 camera descriptor,
  enumeration, synthetic-state, exact RGBA transfer and ownership routes without opening hardware.
- Recorded CNA-DEMOS-ISSUE-063: the C camera test-backend comments promise `Closed` after creation
  and frame clearing, while the implementation returns `Opening` and refuses the defined `Closed`
  state under the current platform-opened camera model.
- Added `c_api.devices.system_tray`, a16-check C workflow covering11 in-memory tray routes for
  owned lifetime, menu state, upper bounds and callbacks; real host-icon creation remains manual.
- Added `c_api.devices.dialogs`, a20-check C workflow covering all10 message-box and file-dialog
  routes through immediate test services, including callback-owned copies, cancellation, request
  validation and platform-service restoration without opening real UI.
- Added `c_api.devices.vibration`, an18-check C workflow covering all nine vibration routes through
  exact in-memory request logs, bounds, clamping, device-name buffers and backend restoration
  without actuating or claiming a physical motor.
- Added `c_api.devices.host_info`, a12-check read-only C workflow covering all14 environment,
  extension-availability, power, CPU/RAM, preferred-locale and display-information routes in
  `devices.h`; host values and windowless/windowed display answers are reported without mutation.
- Added `c_api.devices.sensor_lifecycle`, a40-check C workflow for four sensor acquisition/event
  lifecycles;66 additional `sensors.h` routes now have semantic coverage through deterministic
  synthetic backends, with no physical-hardware claim.
- Recorded CNA-DEMOS-ISSUE-062: disposed Accelerometer and Gyroscope handles still expose cached
  CurrentValue even though the C contract requires an invalid-state refusal.
- Added `c_api.devices.sensor_values`, a deterministic34-check C workflow covering all50 pure
  reading/event-description routes in `sensors.h`, including nested/timestamp equality, hashing,
  canonical text, short-buffer preservation and exact type names without requiring sensor hardware.
- Added `c_api.input.haptics`, a24-check C workflow for direction/effect/capability values and the
  complete safe closed-device lifecycle;33of36 `input_haptics.h` routes now have semantic coverage
  while enumerated hardware names and physical feedback remain manual.
- Added `c_api.input.joystick`, a23-check C workflow for descriptor values, disconnected owned
  state snapshots and deterministic hot-plug lifetime;27of30 `input_joystick.h` routes now have
  semantic demo coverage and the three populated enumeration routes remain hardware-manual.
- Added `c_api.input.text_input`, a deterministic20-check C workflow for committed UTF-16,
  UTF-8 composition/candidates, registration/reset lifetime and bound/unbound activation; all17
  public `input_text.h` routes now have semantic demo coverage.
- Complete476-C-input-touch-scaling:489enabled CTests and both full smoke suites preserve every
  parent outcome and unrelated child trace; only the gesture demo grows31to64 controls.
- Extended the C gesture workflow from31to64checks with display/orientation round trips, zero-size
  event dropping, exact normalized FreeDrag scaling and gesture/snapshot source separation.
- Recorded CNA-DEMOS-ISSUE-060: the raised C touch-event comment contradicts its gesture-only
  implementation by also claiming the event-driven touch map is populated.
- Complete476-C-input-gesture-queue:489enabled CTests and both full smoke suites preserve every
  prior result and child trace; the new C standalone adds31 exact gesture-queue controls.
- Added `c_api.input.gesture_queue`, a deterministic31-check C FIFO workflow for complete authored
  gesture values, enabled masks, availability transitions and empty-read preservation.
- Complete475-C-input-touch-values:488enabled CTests and both full smoke suites preserve all
  unrelated evidence; only the existing C touch demo grows from30to60 exact child controls.
- Extended the C touch workflow from30to60checks with pressure-independent identity, atomic
  insertion-copy and ordered collection mutation through15 additional C ABI routes.
- Complete475-C-input-touch-state:488enabled CTests and both full smoke suites preserve all old
  evidence; the new C touch demo adds30 exact lifecycle controls and eight C ABI routes.
- Added `c_api.input.touch_state`, a deterministic 30-check C press/move/release workflow covering
  live capabilities, snapshots, id lookup and previous-location reconstruction without hardware.
- Complete474-C-input-keyboard-state:487enabled CTests and both full smoke suites preserve all old
  evidence; the C keyboard demo adds three exact snapshot controls and two C ABI routes.
- Extended the existing C keyboard/mouse workflow from12to15checks with complementary key-up
  queries, atomic pressed-key capacity reporting and ascending exact-buffer copies.
- Complete474-C-media-sources:487enabled CTests and both full smoke suites preserve all old
  evidence; the C media demo adds three exact source-enumeration checks and four C ABI routes.
- Extended the existing C media workflow from89to92checks with desktop MediaSource enumeration,
  exact type/name reporting and upper-bound output preservation.
- Complete474-C-player-program-exit:487enabled CTests and both full smoke suites preserve all old
  evidence; the C media demo adds one exact early-release/restart check and one C ABI route.
- Extended the existing C media-player workflow from88to89checks with early ProgramExit during
  playback and immediate reinitialization from the independent caller-owned Song.
- Complete474-C-player-single-song:487enabled CTests and both full smoke suites preserve all old
  evidence; the C media demo adds six exact ownership checks and one C ABI route.
- Extended the existing C media-player workflow from82to88checks with caller-owned single-Song
  Play, queue-copy ownership and independent release while preserving the ISSUE-058 boundary.
- Complete474-C-player-visualization:487enabled CTests and both full smoke suites preserve all old
  evidence; the C media demo adds four exact visualization checks and four C ABI routes.
- Extended the existing C media-player workflow from78to82checks with versioned visualization
  initialization, live spectrum/waveform data and disabled zero-buffer behavior.
- Complete474-C-player-transport:487enabled CTests and both full smoke suites preserve all old
  evidence; the C media demo adds three exact indexed-transport checks and two C ABI routes.
- Extended the existing C media-player workflow from75to78checks with collection start-index,
  previous navigation and safe stopped-state restoration.
- Complete474-C-player-settings:487enabled CTests and both full smoke suites preserve all old
  evidence; the C media demo adds four exact settings checks and eight C ABI routes.
- Extended the existing C media-player workflow from71to75checks with stopped-state mute, repeat,
  shuffle and clamped-volume round trips.
- Complete474-C-song-metadata:487enabled CTests and both full smoke suites preserve all old
  evidence; the C media demo adds two exact Song metadata checks and six C ABI routes.
- Extended the existing C media-player workflow from69to71checks with plain-path Song metadata
  defaults and mutable play-count round-trip behavior.
- Complete474-C-song-identity:487enabled CTests and both full smoke suites preserve all old
  evidence; the C media demo adds eight exact Song identity checks and five C ABI routes.
- Extended the existing C media-player workflow from61to69checks with plain-path Song creation,
  exact handle text, path-based equality/hash semantics and literal empty-name storage.
- Complete474-C-song-duration:487enabled CTests and both full smoke suites preserve all old
  evidence; the C media demo adds six exact Song duration/disposal checks and six C ABI routes.
- Extended the existing C media-player workflow from55to61checks with explicit Song duration,
  elapsed-time completion boundaries and idempotent Song disposal.
- Complete474-C-media-events:487enabled CTests and both full smoke suites preserve all old
  evidence; the C media demo adds two exact event lifecycle checks and four credited C ABI routes.
- Extended the existing C media-player workflow from53to55checks with synchronous manual event
  raises and post-unsubscribe controls; credited four meaningfully exercised C ABI routes.
- Complete474-C-media-lifecycle-bounds:487enabled CTests and both full smoke suites preserve all
  old evidence; the C media demo adds seven exact checks and four actual C ABI routes.
- Extended the existing C media-player workflow from46to53checks with SongCollection Dispose,
  empty-queue bounds and unchecked active-index behavior; documented CNA-DEMOS-ISSUE-059.
- Complete474-C-media-ownership:487enabled CTests and both full smoke suites preserve all old
  evidence; the existing C media demo adds14exact ownership controls and10actual C ABI routes.
- Extended the existing C media-player workflow from32to46checks with collection retention,
  queue-owned copies, view replacement, queue_add copying and safe stopped-queue Clear.
- Complete474-album-art:487enabled CTests and both full smoke suites preserve all old
  evidence; the new standalone adds eight exact album stream/thumbnail controls.
- Added an isolated album-art workflow with folder/no-art branches, original and thumbnail
  streams, public 128x64 decoding, and ownership checks after library destruction.
- Complete473-duration:486enabled CTests and both full smoke suites preserve all old
  evidence; music import adds three positive-duration checks without playback.
- Tagged music import now checks positive Song lengths and album/playlist member sums
  with the optional FFmpeg probe;18child checks preserve all15original relationships.
- Recorded CNA-DEMOS-ISSUE-058: an ASan public-API probe shows `MediaPlayer::Play`
  frees an active-queue Song before reading it; the independent caller-owned control passes.
- Complete473-hierarchy:486enabled CTests and both full smoke suites preserve all old
  failures/child/network evidence; categories grows from five to ten checks.
- XACT categories now contrast independent groups with authored parent/child pause, resume
  and stop: ten checks pass while all five original predicates remain.
- Complete473-music:486enabled CTests and both full smoke suites retain all previous
  failure/child/network evidence; music import adds 15 explicitly reviewed child checks.
- Added isolated tagged music import with populated albums, artists, genres and an M3U
  playlist; 15 exact child checks pass in both environments with borrowed object identity preserved.
- Complete472-cuelimits:485enabled CTests and both full smoke suites retain all previous
  failure/child/network evidence; instance_limits adds five per-cue capacity controls.
- XACT instance limits now distinguish per-cue definition capacity from category limits:
 12checks pass in both requested environments, retaining all seven original category checks.
- Complete472-banks:485enabled CTests and both full smoke suites retain all previous
  failure/child/network evidence; the bank ownership demo adds12public state/event checks.
- Added XACT bank/engine ownership demonstration:12public disposal/state/event checks
  pass in both requested environments; prepared and consuming cues have distinct dependencies.
- Complete471-cue3d:484enabled CTests and both full smoke suites retain all previous
  failure/child/network evidence; owned Cue spatial variables add six public checks.
- Positional XACT now directly demonstrates owned Cue::Apply3D spatial variables and
  paused movement; six checks pass in both requested environments, preserving the audible sweep.
- Complete471-punctual:484enabled CTests and both full smoke suites retain all previous
  failure/child/network evidence; new point/spot receiver passes8GLpixel controls.
- A focused point/spot receiver adds8GLpixel controls for authored distance maps, bias,
  range and cone exclusion;27targeted CTests pass. No shadow producer or other-backend claim.
- Complete470-authored: all483enabled CTests and both full smoke suites preserve previous
  failure/child/network evidence; selected artifacts and normalized diagnostics stay stable.
- XACT cue states now distinguish no release, authored fade completion and Immediate
  interruption. Six15check runs and37targeted CTests pass; original predicates/assets remain.
- Complete470-fourth: all483enabled CTests and both full smoke suites preserve previous
  failure/child/network evidence; selected artifacts and normalized diagnostics stay stable.
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
