# Roadmap

Coverage work is organised in **phases** of concrete, CNA-named areas. Take one area at a time,
finish it (demos built, run, validated, coverage regenerated, HANDOFF updated), then move on.
The machine-derived backlog of XNA types without a demo is in [COVERAGE.md](COVERAGE.md)
("Types without a demo yet"); this file adds order, CNA-specific areas and judgement.

Status: ✅ done · 🟨 started · ⬜ not started. Each area lists the demos it should contain; "≈"
means the number follows the API, not a quota.

## Phase 0 -- research and architecture ✅
CNA inspected (modules, build, renderers, content formats, bindings, samples); architecture and
plan written.

## Phase 1 -- infrastructure ✅
CMake integration, registry, metadata, runner, DemoGame, browser, CLI, catalog JSON, validation,
CTest discovery, screenshots, coverage report, CI, pilot demos in 7 categories.

## Phase 2 -- math (`math.*`) ✅ (78 math demos + 6 packed-vector demos)
Console demos with checks; each shows one operation family.
- `math.vector2.*`, `math.vector3.*`, `math.vector4.*`: construction and constants, arithmetic
  operators, `Dot`, `Cross` (Vector3), `Normalize`, `Distance`/`DistanceSquared`, `Lerp`,
  `SmoothStep`, `Hermite`, `CatmullRom`, `Barycentric`, `Clamp`, `Min`/`Max`, `Reflect`,
  `Transform` (matrix, quaternion), `TransformNormal`. ≈ 30
- `math.matrix.*`: `CreateTranslation`, `CreateScale`, `CreateRotationX/Y/Z`, `CreateFromAxisAngle`,
  `CreateFromYawPitchRoll`, `CreateFromQuaternion`, `CreateLookAt`, `CreatePerspectiveFieldOfView`,
  `CreateOrthographic(OffCenter)`, `CreateWorld`, `CreateBillboard`, `CreateReflection`,
  `CreateShadow`, `Invert`, `Transpose`, `Decompose`, multiplication order, `Lerp`. ≈ 18
- `math.quaternion.*`: axis-angle, yaw/pitch/roll, `Slerp`/`Lerp`, `Concatenate`, `Conjugate`,
  `Inverse`, `Normalize`, rotating a vector. ✅ `double_cover`: sign ambiguity, stable orientation
  comparisons, aligned blending and relative rotations. ≈ 8
- `math.math_helper.*`: `ToRadians`/`ToDegrees`, `Clamp`, `Lerp`, `SmoothStep`, `WrapAngle`,
  `Hermite`, `CatmullRom`, `Barycentric`, `Distance`, constants. ≈ 6
- `math.point.*`, `math.rectangle.*`: `Contains`, `Intersects`, `Intersect`, `Union`, `Inflate`,
  `Offset`, `Center`, `IsEmpty`. ≈ 8
- `math.bounding_box.*`, `math.bounding_sphere.*`, `math.bounding_frustum.*`, `math.plane.*`,
  `math.ray.*`: creation (`CreateFromPoints`, `CreateMerged`, `CreateFromSphere`), `Contains` →
  `ContainmentType`, `Intersects` (each pair that differs in meaning), `Plane.DotCoordinate`,
  `Ray.Intersects` distance, frustum culling. ≈ 18
- `math.curve.*`: keys, `Evaluate`, `CurveLoopType` (constant/cycle/oscillate/linear),
  `ComputeTangents`, `CurveContinuity`. ✅ Existing demos extended for selective/asymmetric
  tangents, negative loops, and empty/single-key curves; Linear post-loop exposes ISSUE-036. ≈ 5
- `math.color.*`: constructors (bytes, floats, Vector3/4), `FromNonPremultiplied`, `Lerp`, `* float`,
  `PackedValue` layout, `ToVector3/4`, named colours. ≈ 6
- `graphics.packed_vector.*`: pack/unpack round trip per format (Alpha8, Bgr565, Bgra4444,
  Bgra5551, Byte4, HalfSingle/Vector2/Vector4, NormalizedByte2/4, NormalizedShort2/4, Rg32,
  Rgba1010102, Rgba64, Short2/4) -- one demo per format family, not per method. ≈ 8

## Phase 3 -- game lifecycle (`game.*`) ✅ (33 demos; ScreenDeviceName: `game.window.screen_device_name`)
- Game: method order (`Initialize`→`LoadContent`→`BeginRun`→Update/Draw→`EndRun`→`UnloadContent`),
  `IsFixedTimeStep` and `TargetElapsedTime`, variable time step, `IsRunningSlowly`, `ResetElapsedTime`,
  `SuppressDraw`, `Exit`, `Exiting`/`Activated`/`Deactivated` events, `IsMouseVisible`,
  `InactiveSleepTime`, `RunOneFrame`/`Tick` (standalone where process-level), `RunFrameEXT`. ≈ 12
- GameTime: `ElapsedGameTime`, `TotalGameTime`, first-update zero elapsed. ≈ 2
- Components: `GameComponent`, `DrawableGameComponent`, `UpdateOrder`/`DrawOrder`,
  `Enabled`/`Visible`, `ComponentAdded`/`Removed` events. ≈ 5
- Services: `GameServiceContainer` `AddService`/`GetService`/`RemoveService`. ≈ 2
- GameWindow: `Title`, `ClientBounds`, `AllowUserResizing`, `ClientSizeChanged`,
  `ScreenDeviceName`, native handle (CNAEXT), file/text drop events (CNAEXT). ≈ 6
- GraphicsDeviceManager: preferred back-buffer size + `ApplyChanges`, `SynchronizeWithVerticalRetrace`,
  `PreferMultiSampling`, `PreparingDeviceSettings`, device events, `ToggleFullScreen` (manual),
  `GraphicsProfile` Reach vs HiDef. ≈ 8
- `LaunchParameters`, `TitleContainer.OpenStream`, `TitleLocation`. ≈ 3

## Phase 4 -- 2D graphics ✅ (48 demos including 7 SpriteFont demos; small remainders listed per line)
- ✅ `graphics.device.*`, `graphics.viewport.*`, `graphics.graphics_adapter.*`,
  `graphics.graphics_resource.*` (13): `Clear`/`ClearOptions`, split-screen viewports, `Viewport`
  properties and `Project`/`Unproject`, `ScissorRectangle`, `PresentationParameters`,
  `GetBackBufferData`, `SupportsCapability` and the capability report (CNAEXT), adapters and
  display modes, format queries, resource lifetime events. Remaining: `Present` with rectangles
  (CNAEXT, SOFTWARE/HEADLESS only), `Reset` with new parameters, `DeviceLost` (never raised on
  desktop).
- ✅ (19 demos) `graphics.sprite_batch.*`: every `Draw` overload family (position, destination rectangle,
  source rectangle, rotation+origin, scale float/vector, `SpriteEffects` flips, layer depth),
  tinting, `Begin` overloads: `SpriteSortMode` (Deferred, Immediate, Texture, BackToFront,
  FrontToBack), `BlendState` (AlphaBlend, Additive, NonPremultiplied, Opaque), `SamplerState`
  (PointClamp, LinearWrap...), `DepthStencilState`, `RasterizerState`, transform matrix, custom
  `Effect`; sprite sheets; many Begin/End pairs per frame. ≈ 20 -- custom `Effect` in `Begin` moves
  to Phase 6 (needs effects). The existing multiple_batches demo now checks deferred state
  application/retention after End and actual camera-world versus screen-HUD pixels (six checks
  on both renderers; captures inspected).
- ✅ (7 demos) `graphics.sprite_font.*`: build-time CNB font (generated font-texture sheet),
  `DrawString`, `MeasureString`, `LineSpacing`, `Spacing`, `DefaultCharacter`, `Characters`,
  scaled/rotated/mirrored text, `StringBuilder`, the CNAEXT glyph constructor. Remaining: loading
  an XNB font (Phase 8, content), TrueType `.spritefont` (needs FreeType and a licensed font).
- ✅ (9 demos) `graphics.texture2d.*`: `SetData`/`GetData` (whole, rectangle, mip level), mipmaps,
  `SurfaceFormat` variants, `FromStream`, `SaveAsPng`/`SaveAsJpeg` (including requested output
  dimensions and unchanged source), `LevelCount`. Remaining:
  float/HDR formats and broader direct compressed-block updates.
  ✅ `graphics.texture2d.dds_stream`: packed offset/short reads, original DXT1 mip-chain block
  readback, fit/crop Color conversions, malformed-chain refusal and actual pixels (14 checks).
  ✅ `copy_ownership` checks partial shared edits versus full-upload detachment, Name/Tag isolation,
  sibling survival after Dispose and actual rendered pixels.
  ✅ Existing rectangle demo now checks nonzero CPU array offsets and untouched destination padding.
  DXT1 content loading is already shown by `content.xnb.dxt_texture`.

The existing layer-depth and texture-sort demos now verify their rendered overlaps, including
a check that all authored colors remain visible and that a blank texture row cannot pass.

## Phase 5 -- render states, buffers, primitives, render targets ✅ (41 demos; small remainders below)
- ✅ (17) `graphics.blend_state.*` (blend functions, colour write channels, blend factor, bound
  states are immutable), `graphics.sampler_state.*` (address modes, anisotropic, MaxMipLevel, LOD
  bias), `graphics.rasterizer_state.*` (cull mode, wireframe, depth bias),
  `graphics.depth_stencil_state.*` (depth buffer, DepthRead transparency, depth function X-ray,
  stencil mask, stencil counting), ✅ `graphics.depth_stencil_state.two_sided_stencil` (a shadow
  volume counted in one draw with TwoSidedStencilMode, equal to two culled passes).
- ✅ (9) `graphics.vertex_buffer.*` (static buffer + DrawPrimitives, DynamicVertexBuffer with
  Discard and ContentLost), `graphics.index_buffer.*` (indexed cube, 32-bit indices),
  `graphics.vertex_declaration.custom_vertex`, ✅ `graphics.vertex_buffer.get_data`,
  ✅ `graphics.index_buffer.dynamic_index_buffer`, ✅ `graphics.vertex_buffer.several_streams`
  (SetVertexBuffers). Remaining: `NoOverwrite` ring buffer (CNA accepts the hint and re-uploads the
  whole buffer).
- ✅ (4) `graphics.primitives.*`: DrawUserPrimitives, DrawUserIndexedPrimitives, all four
  PrimitiveTypes, ✅ `graphics.primitives.draw_instanced_primitives` (CNA's stock effects read a
  per-instance world matrix from the instance stream).
- ✅ (9) `graphics.render_target.*`: render to texture, `RenderTargetUsage` Discard/Preserve,
  read back with GetData, own depth buffer, mipmapped target, multisampling, ✅
  `graphics.render_target.render_target_cube` (dynamic environment map; ISSUE-021 on the GL
  renderers), ✅ `graphics.render_target.multiple_render_targets` (two targets, a two-output GLSL
  ShaderEffect), ✅ `graphics.render_target.hdr_blendable` (values past 1.0 in an HdrBlendable
  target; ISSUE-027 found on SOFTWARE).
- ✅ `graphics.texture3d.volume_data`, ✅ `graphics.occlusion_query.visible_pixels`. Remaining:
  remaining typed/compressed format families, further DDS stream boundaries and resource lifetimes.
  ✅ volume_data now also passes six signedVector4/scalar boxed-transfer controls, preserving
  its ten original assertions and pixel-identical inspected Color atlases on both renderers.
  ✅ `graphics.texture_cube.face_transfers` now passes eight focused checks per renderer: guarded
  rectangle windows, neighboring faces, authored mip levels and refused-call preservation.
  Both inspected readback-atlas captures are RGBA-identical; no direct GPU-readback claim.
  Full 457 passes all 470 enabled CTests and both smokes, retaining all original 35/36 failures.
  ✅ `graphics.texture_cube.compressed_blocks`: eight checks on both renderer selections cover
  exact DXT1 blocks, guarded byte windows, padded small mips and refused-transfer preservation.
  Empty stderr; no decoded-pixel/sampling or direct GPU-readback claim.
  ✅ `graphics.texture_cube.typed_payloads`: eight checks on both selections cover packed-word
  serialization, guarded element windows, unclamped Vector4 channels, scalar views, a typed
  rectangle patch and atomic refusal. No sampling or direct GPU-readback claim.
  ✅ `graphics.texture3d.sampler_coordinates`: six GL checks and an inspected image establish
  actual slice centers, point versus 3:1 linear filtering, and Clamp versus W-only Wrap on unit1.
  SOFTWARE capability skip; mips, other formats and varying X/Y remain separate.
  ✅ `graphics.texture_cube.sampler_ownership`: shared full-face edits, independent wrapper names,
  source disposal and actual custom cube sampling of all six faces. Five GL checks pass, image
  inspected and stderr empty; SOFTWARE capability skip. Full 462 passes 475 enabled CTests and
  both smokes while retaining every old 37/41 failed assertion.
  🟨 `graphics.texture_cube.dds_streams`: full and prefixed RGB24 cubes/mips and ordinary
  truncation pass; short-read completion and prefixed truncation expose ISSUE-054 on both
  selections. Five checks execute, with three passes/two failures and empty stderr.

## Phase 6 -- effects and shaders 🟨 (35 demos: stock effects, effect API, PbrEffect)
- ✅ Skinned material and lighting controls: emission/ambient/half-alpha composition, independent
  diffuse/specular lights1/2, their sums and shininess exponent. Material has four checks;
  specular_lights now has eight on both renderers with inspected labeled comparisons.

- ✅ `graphics.skinned_effect.vertex_colors`: a plain packed-color skinned vertex contrasts the
  disabled channel with actual material/texture/tint multiplication and interpolation. Three
  rendered checks pass on both selections; inspected images agree within one blue level.

- ✅ Stock effects (12): `BasicEffect` (texture, default lighting, three directional lights,
  per-pixel lighting, material colours, fog, alpha), `AlphaTestEffect` cut-outs,
  `DualTextureEffect` light map, `EnvironmentMapEffect` with a `TextureCube` and Fresnel,
  `SkinnedEffect` with two bones, mixed-weight joint pixels and palette read/write boundaries, ✅ `graphics.basic_effect.vertex_color_lighting` (vertex colour
  and lighting together).
- 🟨 CNA effects: ✅ `PbrEffect` metallic/roughness, ✅ `ColorMatrixEffect` (SOFTWARE), ✅
  `graphics.pbr_effect.texture_maps` (normal map with a Vector4 vertex tangent, emissive map,
  occlusion map on ambient light), ✅ `SkinnedPbrEffect` (`graphics.model.skinned_pbr`). Remaining:
  image-based lighting.
- 🟨 `graphics.shader_effect.clone_state`: nine GL checks establish independently owned programs,
  mutation isolation and clone survival after original disposal; three failures expose ISSUE-055
  (lost matrices, changed pixels and disposed-clone refusal). Both captures inspected; SOFTWARE
  capability skip. Full 461 passes 474 enabled CTests and both smokes; all old failures are retained.
- ✅ `graphics.shader_effect.uniform_palettes`: four GL checks verify two matrix/vec3 entries,
  explicit payload layout/counts, caller-buffer overwrite and same-program updates. The image
  was inspected; SOFTWARE capability skip. Other array families remain separate gaps.
- ✅ `graphics.shader_effect.compiler_diagnostics`: seven GL checks retain raw/owned diagnostics
  after destroying a rejected syntactically invalid replacement; the valid program still draws
  checked cyan pixels. Image inspected, intentional compiler stderr preserved, SOFTWARE skip.
- ✅ `graphics.shader_effect.portable_package`: owned language/stage pairs, order-independent
  selection, incomplete/ambiguous refusals, owned exception diagnostics and actual RGB-to-GBR
  pixels (eight GL checks, SOFTWARE capability skip). GL capture inspected; desktop/binary
  variants and additional compiler-stage/link failures remain distinct gaps.
- 🟨 Custom shaders: ✅ `ShaderEffect` in 3D (toon shading), a heightmap read in the vertex shader
  (`graphics.shader_effect.vertex_displacement`) and in `SpriteBatch::Begin`
  (grayscale), GLSL chosen from `GetShaderDialectEXT()`. Remaining: compiled XNA `.fx` effects
  (needs a CNA build with `CNA_<RENDERER>_COMPILED_EFFECTS`), other dialects.
- ✅ `graphics.skinned_effect.normal_lighting`: joint rotation/blended-normal normalization,
  nonuniform World inverse transpose and the lighting-disable refusal (five checks).
- ✅ `graphics.skinned_effect.per_pixel_lighting`: actual vertex-lit constant color versus
  per-pixel diffuse gradient on one shared skinned mesh (three checks, RGBA-identical captures).
- ✅ `graphics.skinned_effect.fog_depth`: bone-only depth changes, disabled-fog control and actual
  FogStart/midpoint/FogEnd colors. Four checks pass on both renderers; both captures inspected,
  with only one blue-channel level of difference on the middle card.
- ✅ `graphics.skinned_effect.weight_channels`: one/two selected channels ignore deliberately
  nonzero inactive lanes, four active weights blend permuted bone indices, and near-edge pixels
  check the translated shapes. A fourth mesh now addresses the final MaxBones slot while preserving
  all original controls: seven checks pass on both renderers; inspected captures are RGBA-identical.
- ✅ `graphics.skinned_effect.clone_ownership`: copied independent palettes and shared owned
  texture lifetime after source/caller release; actual tinted pixels and final weak ownership release
  pass ten checks on both renderers. Explicit texture disposal and other skinning variants remain separate.
- ✅ Effect API (4): parameters/techniques/passes reflection (properties reach parameters on
  Apply), `Effect::Clone`, the effect interfaces (IEffectMatrices/IEffectLights/IEffectFog).
- Custom shaders: `ShaderEffect` per shader language (ask `RendererCapabilityProfile`), compiled
  XNA effect bytecode (`Effect(GraphicsDevice&, bytes)`), `EffectParameter`/`EffectTechnique`/
  `EffectPass` reflection and parameter setting. The ShaderEffect member table in COVERAGE_AUDIT.md
  separates legacy source success from portable package selection, compiler diagnostics, uniform
  arrays, cube/volume samplers, block layout and clone/lifetime behavior. These are semantic gaps,
  not extra credit for the existing GLSL scenes. ≈ 8

- ✅ `graphics.effect.parameter_values`: typed values, matrix transpose and bone-array reflection;
  ISSUE-034/035 remain explicit expected failures.

## Phase 7 -- models, animation, glTF 🟨 (23 demos)
- ✅ `graphics.model.*`: `Model::Draw`, bones (relative/absolute transforms, lookup by name,
  ParentBone), bone animation with the per-mesh drawing loop, frustum culling with
  `ModelMesh::BoundingSphere`, per-mesh effect settings, skinned animation (`SkinningData` in
  `Model.Tag`, `AnimationPlayer`, `SkinnedEffect`), the CNAEXT glTF import report of a model loaded
  as a loose `.gltf` at run time, CNAEXT node-animation clips (`ModelAnimationsEXT`,
  `ApplyClipToBonesEXT`) on a turntable. Assets: original glTF written by `tools/gltf_models.py`
  (decision: generate, do not copy CNA's corpus), compiled by `cna_add_content`.
- ✅ `graphics.model.morph_targets` (MorphTargetDataEXT, weights track, SetMorphWeightsEXT),
  ✅ `graphics.model.gltf_cameras` (ModelCameraEXT; compiled models lose them, ISSUE-023),
  ✅ `graphics.model.textured_gltf` (embedded PNG; compiled models lose it, ISSUE-022).
- ✅ `graphics.model.gltf_lights` (KHR_lights_punctual -> the PbrEffect's directional lights; GL
  only), ✅ `graphics.model.clip_interpolation` (STEP/LINEAR/CUBICSPLINE samplers; expected failure,
  CNA-DEMOS-ISSUE-031: all play as LINEAR).
- ✅ `graphics.model.material_variants` (KHR_materials_variants, run-time import: CNA's content
  pipeline refuses variants, which the compiled model format cannot carry).
- ✅ `graphics.model.skinned_pbr` (skinned + metallic-roughness imports as SkinnedPbrEffect).
- ✅ `graphics.model.mesh_parts`: procedural parts share buffers with distinct index/base-vertex
  ranges and material effects; effect collection deduplication/replacement and both selected
  triangle halves plus a shared-buffer line part are checked with actual pixels on both renderers.
- ✅ `graphics.model.part_sampler_state`: sampler metadata is selected explicitly by the application;
  point-wrap versus point-clamp is checked with pixels on both renderers. An actual unlit glTF
  fixture verifies imported NEAREST/CLAMP metadata, texture ownership and preserved UVs.
- ✅ `graphics.model.skinned_parts`: the distinct SkinnedModelEXT owns/transfers/replaces named
  parts, rejects mismatched bone counts, removes a temporary part and feeds a sampled inverse-bind
  palette to actual SkinnedEffect drawing after donor destruction.
- ✅ `graphics.model.palette_sampling`: three-bone parent composition, untracked bind pose,
  scale/rotation/translation interpolation, inverse binds, loop/clamp boundaries and invalid-data guards.
  Its 19-check extension now contrasts AnimationPlayer local/world/skin spaces through the same
  authored hierarchy. Existing node_animation now has nine checks for untracked application state,
  wrong-space refusal/preserved transforms and disturbed-pose boundary clamps. Both renderer
  selections pass; original node images are unchanged separately on each renderer.
- ✅ `graphics.model.skinned_manifest`: original nested skeleton/mesh/texture/clip sidecars load
  through ContentManager, a truncated skeleton is refused and actual animated pixels are checked.
- ✅ Existing skinned_animation now checks player state, relative versus absolute time, looping,
  clamping and restart before its original rendered pose check (10 checks).
- ✅ `graphics.model.xnb_model`: original `.x` compiled to XNB, restored hierarchy/shared-buffer
  fixups, independent part ranges/materials and actual mesh placements (11 checks per renderer).
- ✅ `graphics.model.multiple_skins`: independent palettes for shared authored geometry/material,
  retained non-joint ancestry, ignored mesh-node placement, selective bind-pose restoration and
  live bounds responding to a controlled distant palette before the final rendered pose (21 checks).
  ISSUE-042 records stale root-prefix comments only; the runtime workflow passes.
- ✅ `graphics.model.imported_topology`: line-loop closure and strip/fan normalization, exact
  public index ranges and topology-dependent counts, plus actual outline/filled pixels (18 checks).
- Remaining: points/incomplete primitive tails, specular slots, more skin weighting/material variants and other
  member decisions/evidence limits in COVERAGE_AUDIT.md. Runtime XNB custom graph fixups are covered separately
  in Phase 8; built-in model loading does not demonstrate every ContentReader member.

## Phase 8 -- content 🟨 (46 demos; remaining below)

- ✅ Existing `content.texture_processor.resize_to_power_of_two` now verifies XNB-targeted
  Reach/HiDef mipmap processing, resized payloads matching its rendered CNB and the distinct native
  CNB profile policy (seven checks). Scene captures remain RGBA-identical on both renderers.
  XNB writing/loading and actual Reach hardware remain separate; ISSUE-049 is a stale public
  environment comment, not a runtime failure.
- ✅ texture processor parameters (colour key, premultiplied alpha, mipmaps, power of two), XNB
  DXT textures (`content_xnb/`), CNJ curve, compiled WAV, ContentManager cache/Unload, several
  managers, missing assets, OpenStream override (ISSUE-012), Unload disposal (ISSUE-011), CNAEXT
  manifest, custom loose-file reader, the same SpriteFont/SoundEffect/Curve as CNB and XNB.
  ✅ `content.xnb.custom_type_reader` (ContentTypeReader<T> + AddTypeCreator for the game's own
  type; now checks actual reader invocation counts, cache reuse and reader context), ✅ `content.content_manager.resource_content_manager` (content embedded as resources;
  ISSUE-012), ✅ `content.cnj.sprite_font` (a bitmap font authored as CNJ: atlas, glyph rectangles,
  cropping, kerning), ✅ `content.content_project.xna_content_project` (an XNA `.contentproj` built
  as it is, `CONTENT_PROJECT`). A CNJ texture repeats the texture processor demos (the same
  parameters in JSON). ✅ `content.custom.cnb_loader` uses public CnbWriter and
  RegisterCnbLoaderEXT for a typed asset, caching and schema rejection. ✅ Existing manifest demo
  now demonstrates explicit snapshot refresh, its independence from the asset cache, per-file reader aggregation and lazy availability after typed loading (13 focused checks on both renderers). ✅
  `content.custom.cnj_loader` uses two named JSON factories for a game-owned Enemy type, actual
  payload decoding with nlohmann-json, manager-local registration and useful error paths. The CNB
  example also covers optional/mandatory unknown chunks, retry after a repaired file, and valid
  containers whose game schema has missing/duplicate blocks, truncated fields or trailing bytes.
  ✅ `content.custom.pipeline_components`: registered importer/processor/writer, real patrol
  precomputation, deterministic CNB output, runtime loading and typed parameter diagnostics.
  Existing example now has 15 passing checks: copied carrier lifetime and separate stable-identity
  versus C++ payload guards at Import, Process and Write, with exact error context. An explicitly
  documented absent XNB route gives the game's reason at Selection while CNB remains supported.
  ✅ `content.custom.source_root_capabilities`: built-in CNJ texture compilation reads original
  shared art through a request-local alias. Ten focused checks cover exact pixels, recorded roots,
  identical output after relocation, ordinary local references, contextual refusals and repair.
  No GPU, nested-capability propagation or CLI incremental-fingerprint claim.
  ✅ `content.custom.pipeline_diagnostics`: an original opacity strip produces real 8-bit
  quantization loss, contextual importer/processor/writer messages and a copied downstream log.
  Nine checks cover exact endpoints without warnings, null sinks, import failure and repair.
  Concurrent logger callbacks and CLI verbosity are not established by these synchronous builds.
  ✅ `content.custom.multi_artifact_publication`: a real compiler node publishes two declared CNB
  schemas plus a briefing through native AddDeploymentFile. Fourteen focused checks cover digests,
  XREF/ownership, skips, missing companion recovery, changed support input, Publish failure/retry,
  graph refusal of two nodes claiming one destination, and clean with/without its manifest.
  The failed deployment leaves newly published CNBs and the older manifest; no whole-node rollback
  or crash/concurrent-publisher guarantee is claimed.
  ✅ `content.custom.dependency_graph`: strong build edges versus runtime-only XREFs, actual
  incremental propagation of effective fingerprints with unchanged parent bytes, fresh runtime
  budgets, cycle/missing-node refusal and repaired skipping. Sixteen focused checks also verify
  actual two-worker importer overlap, byte-identical serial/parallel manifests/artifacts and
  unchanged parallel skipping. Same-output concurrent compiler processes remain untested.
  ✅ Existing cache/Unload demo now shows root switching after Unload and exposes CNA's missing
  loaded-root guard (ISSUE-039). ✅ `content.custom.intermediate_serializer`: a separate build-time
  tool with named/optional/ignored fields, collection item names, absent/present/null distinctions,
  XML round trip and rejection of missing required or injected ignored fields.
  ✅ `content.custom.intermediate_shared_resources`: shared aliases, equal-but-distinct objects,
  inline copies and mutation after forward reference fixups; null tables and rejected missing,
  duplicate or incompatible resource references.
  ✅ `content.custom.intermediate_type_serializer`: registered compact dice syntax at the root
  and within described fields, with round trip and lexical/domain rejection on read and write.
  ✅ `content.custom.intermediate_properties`: inherited-member ordering, validated setter
  dispatch, get-only collections appending constructor defaults and flattened spawn coordinates.
  ✅ `content.custom.xna_pipeline_components`: actual XNA importer/processor adapters, default
  processor selection, typed parameter conversion/isolation, source sidecar dependency and
  nested Convert producing compiled route bounds. The extension reads a processor label dependency
  and writes a bounds summary. With the strictness extension, 17 of 19 checks pass:
  AddOutputFile refuses the configured separate output root (ISSUE-047), and returned effective
  parameters retain entries dropped by XnaCompatible (ISSUE-048). Defaulted compiled bounds,
  two contextual warnings, retained valid overrides and refusal of missing inputs are verified. Explicit rebuild/input failure and cleanup are verified; no CLI cache claim.
  ✅ `content.custom.cnb_external_references`: actual XREF tables and owner-mediated child loads,
  shared/local caches, bad references, missing child retry and unsafe-name rejection.
  ✅ `content.custom.cnb_read_limits`: container inspection, independent writer/reader byte
  budgets, opaque schema count/string limits, compressed expansion when supported, and checksum
  rejection despite valid magic.
  Remaining: deeper serializer behaviors, composite asset builds and the member/behavior gaps
  in COVERAGE_AUDIT.md; this phase is not comprehensively covered.
- ✅ `content.xnb.shared_resources`: deferred binary graph fixups, shared versus inline identity,
  forward/weak backward edges, null references, cache/manager isolation, malformed index/type
  refusal and released ownership (15 console checks). Direct low-level orchestration remains
  intentionally deferred to the loader.
- ✅ `content.xnb.existing_objects`: indexed/raw reads into existing slots, retained runtime state,
  null fallback, two-pass initialization and safely unboxed tags (11 passing checks plus the
  GetTypeReader and implicit ReadRawObject target/factory-name failures, ISSUE-043).
- ✅ `content.xnb.external_references`: sibling loose image/compiled label, null and unsafe/missing
  paths, typed/erased owner caches and actual shared-image rendering (11 checks per renderer).
- ✅ Existing intermediate XML tools now cover a two-resource cycle, explicit shared_ptr unlinking
  and verified release (16 child checks), plus AllowNull(false) on both read and write with an
  authored value distinct from its constructor default (11 child checks). Both tools run without
  a display under both renderer selections.
- ✅ `content.custom.intermediate_polymorphism`: explicit concrete serializer registration, an
  abstract-base list, inherited/derived values, emitted Type attributes and nulls, plus refusal
  of unknown/unrelated/abstract declarations (eight child checks, no display needed).
- ✅ `content.content_manager.read_asset_ownership`: callback identity for nested disposable tiles,
  uncached repeat reads, short-read stream ownership, delegated lifetime across Unload/Dispose,
  early refusals and explicit caller cleanup. Fourteen original checks remain passing; an added
  absent-recorder comparison exposes default tracking ISSUE-011 (16 total, 15 passing).
- ✅ `content.custom.nested_builds`: processed child versus compiled child, typed parameter
  propagation, source dependencies, additional CNB/XREF/runtime-name agreement, repeated-build
  deduplication, writer schemas and changed-processing name collision. The caller now publishes
  actual parent/child bytes: missing-child refusal, same-manager retry, restored profile data,
  shared runtime cache and cleanup are verified. Matching sibling items now defer compilation
  to their host; mismatched parameters produce a generated child (20 child checks).
- ✅ `content.custom.intermediate_existing_instances`: ordinary and raw reads populate preallocated
  reference objects, preserve nonserialized state, distinguish Null/type attributes and use an
  explicit per-field compact serializer without replacing the default descriptor (10 child checks).
- ✅ `content.custom.xml_xnb_pipeline`: original custom XML source through the actual compiler,
  pre-registered serializer and versioned XNB writer, manifest digest, runtime reader dispatch,
  unchanged skip, authored edit, cache versus Unload/reload and explicit CNB-schema refusal
  (10 child checks). No genuine-XNA or GPU claim; external graphs remain separate work.
- ✅ `content.custom.incremental_compiler`: real RunContentCompiler registry factory and manifest,
  unchanged skips, primary/shared dependency invalidation, failed-import preservation, repair,
  missing-output rebuild, writer-version identity, same-length/timestamp output tampering, corrupt
  manifest recovery, frozen-registry refusal and ownership-aware clean. Typed per-asset Bonus
  changes rebuild only the affected node, whitespace-only configuration edits skip, and source
  removal/reintroduction collects/restores only its owned output (20 child checks). Clean
  rejects changed owned bytes without deleting an untouched peer or the manifest. Crash consistency,
  parallel publishers and graph/deployment cache cases remain separate work.
- ✅ `content.custom.intermediate_external_references`: filename versus inline data, table
  deduplication, relocation, wrapper/null identity and invalid references. Nine child checks pass;
  native emitted TargetType alias round-trip exposes ISSUE-046. File imports/build publication
  remain separate work; the canonical-name control is explicitly distinct from the failed XML.
- ✅ `content.content_manager.asset_precedence`: same-name XNB/CNB/CNJ/native values,
  cache versus fresh-manager lookup, Unload, explicit native filenames, corrupt compiled-file
  refusal and repair/retry. Thirteen checks pass; truncated XNB exposes a public exception-type
  mismatch (ISSUE-045). `custom.cnj_loader` now separately caches Enemy and Nameplate from the
  same name and CNJ type, including reload after Unload (11 passing checks total).
- ✅ `content.xnb.reader_versions`: real legacy/current schema compatibility, future-version
  refusal before payload reads, duplicate factory policy, scoped withdrawal, owned capture
  lifetime and cache independence (17 console checks per renderer selection).
- `content.content_manager.*`: `Load` caching semantics, `Unload`, `RootDirectory`, separate
  managers, `ContentLoadException`, loose files vs XNB vs CNB vs CNJ lookup order.
- `content.xnb.*`: Texture2D, SoundEffect, SpriteFont, Model, Effect, TextureCube, Curve XNBs.
- `content.cnb.*`, `content.cnj.*`: CNA's binary/JSON formats, `cna_add_content()` build step.
- `content.content_reader.*`: custom `ContentTypeReader`, `ResourceContentManager`,
  `ContentManifestEntry`. ≈ 20

## Phase 9 -- audio and media 🟨 (audio: 21 demos incl. 9 XACT, media: 10 incl. video)
- ✅ `audio.sound_effect.*`: Play, Play(volume, pitch, pan), MasterVolume, PCM buffer
  constructor, GetSampleDuration/GetSampleSizeInBytes, ✅ `audio.sound_effect.loop_region` (loop
  points), ✅ `audio.sound_effect.from_stream` (loose WAV; ISSUE-029).
- ✅ `audio.sound_effect_instance.*`: playback state, looping, volume/pitch/pan, Apply3D
  (smoke-only: hearing is the check).
  ✅ `audio.sound_effect_instance.lifetime`: temporary-wrapper data retention, moving a playing
  voice, explicit idempotent disposal, rejected replay and parent disposal of moved/idle children.
- ✅ `audio.dynamic_sound_effect_instance.streaming`: generated tone, BufferNeeded, pending buffers,
  PCM size/duration, unsupported loop/release requests and immediate-stop cleanup. Disposed
  getters expose ISSUE-038; their checks deliberately remain visible failures.
- ✅ `audio.microphone.input_level` (manual).
- ✅ `audio.xact.*` (5): play a cue, cue states, categories, global variables, positioned cues; XACT
  files generated by `tools/xact_files.py` in the layout CNA's own example writes;
  ✅ `audio.xact.renderer_details`, ✅ `audio.xact.streaming_wave_bank` (ISSUE-028),
  ✅ `audio.xact.cue_variables`. ✅ Four controlled-cue demos now own GetCue results correctly.
  ✅ `audio.xact.instance_limits`: authored category Fail/ReplaceOldest limits, paused slots,
  isolation and freed-slot reuse checked through public states. Remaining: RPC curves (a variable driving
  volume/pitch), complex cues/variations and cue-level limits.
- ✅ `media.*` (11): play a song (State, PlayPosition, end), events through FrameworkDispatcher
  without a Game, playlists with Queue/MoveNext/MovePrevious/IsRepeating/IsShuffled, volume and
  mute, visualization data, Song properties, MediaLibrary collections. Songs are generated WAVs
  played from files (`Song::FromUri`, ISSUE-015). ✅ `media.video_player.play_video` (generated WMV)
  and ✅ `media.song.compiled_song` (generated Ogg Vorbis), both through the content pipeline
  (`assets/media`, own ContentManager: ISSUE-020), ✅ `media.video_player.pause_and_loop`
  (Pause/Resume, IsLooped, Volume, IsMuted).
- ✅ `media.media_library.save_picture`: standalone Linux fixture verifies private XDG music and
  picture roots before library access, vector/stream saves, borrowed picture/album views,
  caller-owned image/thumbnail streams, smaller aspect-preserving thumbnail and reindexing.
  A valid stream delivering 13-byte chunks exposes ISSUE-041: SavePicture reads once and writes
  a zero-padded image. ISSUE-040 records stale thumbnail comments separately. Other platforms,
  media tokens, name collisions and further stream/disposal contracts remain unaudited.


## Phase 10 -- input ✅ (39 demos: keyboard, mouse, text input, clipboard, gamepad, touch, CNAEXT devices)
- Keyboard: state, edge detection (pressed this frame), modifiers, `GetPressedKeys`.
- Mouse: position, buttons, scroll wheel (vertical/horizontal CNAEXT), `SetPosition`,
  `MouseCursor`, relative mode (CNAEXT).
- GamePad: state, capabilities, `GamePadDeadZone`, thumbsticks/triggers/buttons/DPad, vibration,
  CNA extensions (rumble, light bar, gyro).
- Touch: `TouchPanel.GetState`, `TouchLocation`, gestures (`EnabledGestures`, `ReadGesture`).
- `TextInputEXT`, `CNA::Input::Joysticks`, `Haptics`, `Sensors`, `Power`, `Clipboard`. ≈ 25
  Mostly smoke-only; logic-only parts (dead-zone maths, state comparison) are automated.

## Phase 11 -- storage ✅ (7 demos)
`StorageDevice` selector and containers, create/open/delete files and directories, `FileExists`,
`GetFileNames`, `SetAppNameEXT`/`GetStorageRootEXT`, `TitleContainer.OpenStream`. ≈ 8
(Demos write under a temporary storage root and clean up.)

## Phase 12 -- networking, gamer services, avatars 🟨 (19 gamer services + 14 net demos)
Offline throughout: `CnaDemos::GamerServicesDemo` forces CNA's offline mode, uses the demos' own
profile store and signs in "Demo Player"; every such demo runs in its own process (`ownProcess`).
- ✅ `gamer_services.*` (10): signed-in gamers + SignedIn, Guide message box, keyboard input
  (password, cancel), trial mode, sign-in screen for a second player, achievements from the offline
  catalog (`title/GamerServices/Achievements.json`), GamerProfile, presence, GameDefaults,
  GamerPrivileges.
- ✅ `gamer_services.avatar.*` (5): AvatarRenderer + AvatarAnimation, custom pose + expression +
  skeleton, Description bytes, preset gallery, a game's own IAvatarAnimation.
- ✅ `gamer_services.leaderboard.*` (2): LeaderboardWriter in a LocalWithLeaderboards session,
  LeaderboardReader (page, pivot); paging with PageDown/PageUp and typed columns
  (LeaderboardOutcome, TimeSpan; CNA-DEMOS-ISSUE-025).
- ✅ `gamer_services.guide.notification_position`: Guide notifications moved to TopRight.
- ✅ `gamer_services.signed_in_gamer.signed_out` (manual: sign out through the Guide, Home key).
- Remaining gamer services: `AvatarDescription::Changed` (manual at most: offline, only the Guide's
  own avatar editor changes a profile's stored avatar); `Guide::DelayNotifications` is service-only
  (it delays the game-invitation prompts, which need the online service), like friends, messages,
  gamer cards, marketplace and the WriteLeaderboards events.
- ✅ `net.*` (6): Local session (gamers, GamerJoined replay, host, ids, machine), lobby/ready/game
  flow, NetworkSessionProperties, BeginCreate/EndCreate, Gamer::Tag, PacketWriter/PacketReader
  round trip in a System Link session (`Need::Network`). CNA's Local sessions open no sockets and
  deliver no packets, so packets need System Link.
- ✅ `net.network_session.online_session_types`: Local works offline; PlayerMatch and Ranked need
  the online service (GamerServicesNotAvailableException in CNA's offline mode).
- ✅ Two processes (standalone): `net.network_session.find_and_join` (Find, AvailableNetworkSession,
  QualityOfService, Join, GamerJoined/GamerLeft on the host) and `net.network_session.host_leaves`
  (SessionEnded with HostEndedSession; AllowHostMigration with HostChanged; CNA-DEMOS-ISSUE-026:
  on Linux a process that searched before a host started does not find it).
- ✅ `net.network_session.add_local_gamer` (AddLocalGamer in the lobby, AllowJoinInProgress during
  play), ✅ `net.network_machine.remove_from_session` (standalone: the host kicks a player,
  RemovedByHost).
- ✅ `net.network_session.simulated_conditions` (standalone: SimulatedLatency and SimulatedPacketLoss
  measured between two processes in coordinated finite bursts: baseline, delay, full outage and
  recovery. Exact sent counts replace short-sample percentage assumptions; expected failure,
  CNA-DEMOS-ISSUE-030: reliable packets are lost too).
- ✅ `net.network_session.private_slots` (PrivateGamerSlots keep slots from players who just join),
  ✅ `net.network_session.join_refused` (standalone: NetworkSessionJoinException for a game in
  progress, then AllowJoinInProgress; expected failure, CNA-DEMOS-ISSUE-032 and -033).
- Remaining net: voice (manual).

## Phase 13 -- devices, phone, design, diagnostics, platform, CNA extensions 🟨 (18 demos)
- ✅ `devices.*` (4): sensor availability (Accelerometer/Compass/Gyroscope/Motion, State,
  AccelerometerFailedException, Environment::DeviceType), CNAEXT keyboard-emulated accelerometer,
  VibrateController, CNA::Devices SystemInfo/Locale/PowerInfo, ✅ DisplayInfo (content scale, safe
  area). Remaining: Compass/Gyroscope/Motion readings (hardware), CNA devices-ext clipboard, message box, file dialog,
  URL launcher (file:// only), camera, system tray (manual).
- ✅ `phone.*` (1): PhoneApplicationService lifecycle with AttachEXT and State. Remaining:
  notification channels (need a push service: manual).
- ✅ `design.*` (4): text round trip and which types parse, cultures, property descriptors and
  CreateInstance, InstanceDescriptor.
- ✅ `diagnostics.*` (2) and `core.logger.*` (2): diagnostics build/runtime modes, counters and
  gauges (recorded only in STATS/FULL builds), Logger sinks and levels. Remaining: zones, frame
  scopes and trace recording (need a CNA build with `CNA_DIAGNOSTICS=FULL`).
- ✅ `cnaext.*` (4): AsciiPostProcessEffect (every renderer), CRTEffect and DepthEffect (shader
  renderers), DebugDraw.
- `platform.*`: ✅ renderer selection with fallback chain (standalone), ✅
  `platform.window.native_handle` (GameWindow::Handle, typed NativeWindowHandle); renderer
  capability profiles are `graphics.device.capability_report`. Remaining: platform capabilities
  (`CNA::Platform::IPlatform` serves CNA's own static API rather than games).

## Phase 14 -- bindings 🟨 (19 C API demos)
The compatible-prefix batch extends existing host/error demos without adding IDs: 26 host checks
(28 with capture) and 31 error checks pass on both renderer selections. Larger version-one input
and output prefixes retain their known behavior; unknown output tails and original diagnostics
survive. All previous assertions remain, with one host setup label updated.

The new `c_api.game.launch_parameters` source demonstrates explicit UTF-8 spans, ownership,
ordinal enumeration and count/copy boundaries. Both focused selections pass all fifteen child
checks with empty stderr after the external ISSUE-053 repair; complete 456 validation passed all 469 enabled CTests and both smokes, preserving every old failure.
The existing foreign_thread_dispatch now also tests final owner destruction cancelling a waiting
property call: six new controls pass on both selections, preserving all forty original checks
and both ISSUE-050 failures (44 passes/2 failures total). Full 459 passes all 472 enabled CTests and both smokes; exact comparisons preserve old failures
and add only the two documented DDS ISSUE-054 checks.

C API demos as standalone C programs (one C game per process), built only with
`CNA_BUILD_C_API=ON` (experimental in CNA; on in the `linux-full` preset), `Need::CApi`:
✅ `c_api.game.lifecycle_callbacks` (ABI version, callback table, cna_game_run, request_exit, expired callback device handles, refused owner-thread callback reentry),
✅ `c_api.core.error_handling` (results, categories, last message, failed-query/layout preservation, exact-sized count/copy, canonical exception type/parameter details, retained callback failure and exactly-once unload cleanup),
✅ `c_api.core.thread_local_errors` (bounded C11 interleaving, independent worker/caller diagnostics and canonical owner type/parameter isolation; 17 passing checks),
✅ `c_api.game.automatic_dispatch` (21 passing checks: all 15 Update-only controls plus 6 isolated first-Draw setter controls, retaining callback-scope device borrowing and expiry in the Update phase),
✅ `c_api.game.host_failure` (terminal CALLBACK/false, retained diagnostic, no replay, exactly-once unload and handle release; promised C exit hooks expose ISSUE-051),
✅ `c_api.game.host_frames` (copied larger compatible frame-hook prefix, explicit host stepping, draw suppression, incompatible-layout refusal, removal/replacement and terminal exit),
✅ `c_api.game.foreign_thread_dispatch` (default/restored thread affinity, explicit owner pumping, served canonical diagnostics and no-pump owner-only probes exposing ISSUE-050, permitted-write cancellation and resumed dispatch),
✅ `c_api.graphics.sprite_batch` (texture from pixels, submit_many, back-buffer read-back),
✅ `c_api.content.content_manager` (texture, SpriteFont + atlas, draw_string, surviving manager Unload/destruction, retained-atlas release guard, root-change cache behavior and visible text),
✅ `c_api.input.keyboard_mouse` (state structs, snapshots from keys),
✅ `c_api.audio.sound_effect` (PCM16 effect, instance state, refused parent destruction and stale instance handles),
✅ `c_api.math.vectors_and_matrices` (value types, world matrix, inverse, directions and bounded array ranges with refusal before writes),
✅ `c_api.graphics.basic_effect_3d` (stock declaration, vertex/index buffers, BasicEffect with the
IEffectMatrices/IEffectLights routes), ✅ `c_api.graphics.render_target` (render target, read-back,
the target as a sprite texture), ✅ `c_api.storage.save_game` (synchronous selector and container
routes, write/read/delete a save, no game object), ✅ `c_api.input.gamepad` (one button mask with
derived bits, dead-zone modes, live polling), ✅ `c_api.game.components` (a component is a callback
table; draw order, visibility, enabled, collection and component events), ✅
`c_api.media.media_player` (song collection keeps its songs, queue view, transport, process-wide
events). Remaining: the diagnostics/handle/content table in COVERAGE_AUDIT.md identifies
ABI-specific gaps (additional diagnostic families, invalid release guards, borrowed handles
and remaining dispatch/shutdown behaviors). The existing dispatch demo now reproduces ISSUE-050 on RunFrameEXT/Destroy:
owner-only operations incorrectly queue when dispatch is enabled. Its 40 child checks retain
all thirty prior assertions and expose exactly two failures on both selections; permitted queued
write cancellation and resumed dispatch add ten passing controls. The independent
bounded diagnostic remains available; neither change raises the global C route count. The focused host_failure demo confirms
terminal CALLBACK/false and cleanup but exposes suppressed C exit hooks despite the public
promise (ISSUE-051), nine passes/one failure on both selections. Other callback failure locations remain untested. Extend the existing examples where practical; do not duplicate C++ scenes
merely to increase the route count.
C#/.NET demos belong with the separate `cna-dotnet` binding; decide later whether to reference or
host them.

## Phase 15 -- coverage audit 🟨
Walk COVERAGE.md's backlog and the CNA-specific headers; for each remaining type decide: demo,
part of another demo, or not independently meaningful (record why). ✅ First passes: the
decisions for every XNA type without a demo are in [COVERAGE_AUDIT.md](COVERAGE_AUDIT.md), demos
that already showed a type now name it, and new demos cover the effect interfaces and the
graphics device service. Remaining: the candidates COVERAGE_AUDIT.md lists, the CNA-specific
headers. Member/behavior audit covers initial reviews of Quaternion, EffectParameter, Curve,
ContentManager, Texture2D, audio/XACT and CNA binary-content writer/document/cursor APIs
(including overload, ownership and output-observation limitations); this is not a comprehensive member audit. Add screenshot references for
deterministic demos and run the visual-regression comparison across renderers.

## Cross-cutting work (any time)
- Visual regression: reference images + comparison step (AUTOMATION.md section 5).
- CI: an Xvfb job for OPENGLES3; more renderers as CI capacity allows.
- A text font for on-screen labels (SpriteFont asset policy; see PLAN.md).

Complete checkpoint **466-timing** at source5e7c950 validates the typed-cube, actual volume
sampling and C deferred-batch cancellation extensions:479 enabled CTests pass (237.52s),
SOFTWARE406/26/31/3 and OPENGLES3 432/28/3/3. Inner validation and outer Xvfb both exit0.
Exact comparisons retain all37/41 old failed assertions, all five network evaluation checks
and old C/custom child evidence, permitting only eight named C sprite additions. Selected
binary/asset hashes remain stable; normalized stderr adds one SOFTWARE CustomEffects skip.

The initial466 SOFTWARE aggregate network deadline was rejected despite its known-issue
status. A phase watchdog preserves the original packet predicates. A separate controlled
native regression with three8-second transition delays makes the old sender time out at
22.88s; the repaired37.41s exchange preserves all five host checks with only ISSUE-030.
This is a demo coordination fix. Original/repaired/full evidence remains immutable.

Existing volume_data and C content_manager extensions now pass focused validation on both
renderers: ten volume checks and49 captured C child checks (47 without), preserving all old
controls. Eight inspected before/after images form four RGBA-identical pairs; stderr is empty.
Volume coverage adds guarded boxed transfers and non-cubic authored mips/refusal preservation;
C coverage adds stable game-owned borrowing, copied-root independence and expiry. No new ID.
Full466-transfer at5944070 is now complete:479enabled CTests281.68s, SOFTWARE406/26/31/3,
OPENGLES3 432/28/3/3, both inner/outer exits0. Exact37/41old failures, network controls and
C/custom evidence are retained except the planned seven/eight additions. Selected hashes
remain stable. GL has42rather than84known X protocol diagnostics; all other normalized
stderr is unchanged and the precise reduction is reviewed, without claiming a fix.
Generated inventory names168C routes; runtime columns now use the new full checkpoint.

A mid-build external Game.hpp layout change produced an initial SIGSEGV at an original check.
The archived backtrace/object timestamps identify mixed object layouts; an incremental rebuild
resolves it without editing CNA or changing the demo. This is build consistency evidence, not
a new CNA transfer defect. Next drafts remain unapplied: the shader sample kernel has syntax evidence only. The C
storage extension is now natively probed:30passes/one EOF-position failure, with its23old
checks retained. Direct FileStream independently reports-1after EOF with correct data and
rewind controls (ISSUE-056). Apply both as batch467, preserving the real failure explicitly.


Focused467 now builds/validates467/0 and passes24targeted CTests (16passed,8capability skips,
5.55s). The sample kernel adds five GL checks and an actually inspected image with27exact
pixel samples: distinct scalar weights/vec2 offsets and texel size change spread and direction
on the same program after caller buffers are overwritten. SOFTWARE declares CustomEffects
absence. C save_game retains23old checks plus8new (30pass/one ISSUE-056), empty stderr on both
requested selections. Two C routes raise the current metadata inventory to170/3210; generated
COVERAGE.md remains the complete466-transfer snapshot until full467 validation finishes.
The pending full comparison must retain every old failure and add exactly the confirmed EOF
position failure, rather than accepting the changed known-issue status alone.


Complete467 atc45349f now validates the focused batch:480enabled CTests217.01s, both full
smokes (SW405/27/32/3,GL432/29/3/3), independent inner/outer exits0 and stable selected hashes.
Exact old37/41failures remain plus only EOF056 (38/42); all old C/custom/network evidence is
retained with the eight named storage additions. GL title10->11is a positive per-Update
variance. Normalized stderr adds one SOFTWARE capability skip, GL unchanged. COVERAGE.md
now records467complete results/170C routes. Next coherent extensions reuse uniform_palettes
for a shared parent transform/tint and save_game for explicit read-only permission semantics;
their source drafts do not yet have runtime/visual credit.


Focused467-scene extends the two existing examples without changing catalog count. Palettes
retain their four original checks and add two for a shared scalar matrix/premultiplied tint;
both before/after GL images were inspected and all17selected pixels match. C storage retains
31old child checks plus7positive permission controls, with only original EOF056 still failing.
All24targeted CTests pass in4.14s, metadata467/0, current C inventory174/3210. Full467-scene
comparison remains pending; COVERAGE.md still holds the preceding complete467 snapshot.


Complete checkpoint **467-scene**, source `11dbd5b`, preserves all 467 IDs. Build: seven
scheduled operations; metadata 467/0; all 480 enabled CTests pass in 211.34 s (448 passed,
including 27 WILL_FAIL, 32 capability skips; three additional manual tests disabled).
SOFTWARE: 405 passed / 27 failed / 32 skipped / 3 not run. OPENGLES3: 432 / 29 / 3 / 3.
Both inner validation and outer Xvfb independently exit zero. All 38/42 original failed
assertions, C/custom child lists and traces, and five network evaluation checks are retained,
allowing precisely seven new positive storage permission checks (31 -> 38 with only ISSUE-056).
Palette GL checks increase 4 -> 6; SOFTWARE keeps its declared skip. The only other JSON count
change is GL window.title 11 -> 10, the reviewed positive per-Update variation.
All 35 selected binary and four asset hashes remain unchanged. Normalized application stderr
is identical, including 42 known GL X protocol records and the deliberate shader compiler
error. Separate outer Xvfb diagnostics contain 13 exact amdgpu pairs (previously 14); no cause
or hardware GPU claim. CNA 714f9f76a was observed clean at capture/final, not an atomic input
snapshot. All capture/archive/finish helpers pass. Immutable evidence: checkpoint-467-scene-*.
Coverage now records 293/329 types, 1373/2326 XNA members and 174/3210 C routes. These counts
do not establish comprehensive semantic coverage.

Next coherent extensions keep the same IDs: independent SkinnedEffect diffuse-light terms
and the successful-End counterpart to C SpriteBatch cancellation. Drafts have source review
only; neither has runtime or visual credit yet.


Focused467-light extends existing examples without adding an ID or API route. Build12scheduled/
11completed operations and metadata467/0 pass; all25targeted CLI/skinned/C-sprite CTests pass
in4.75s. Skinned lights preserve four specular controls and add four independent diffuse/additive
checks; eight pass per renderer. The C sprite example retains all19old children plus10successful
End-release controls (29without capture/31with). All eight before/after images were inspected;
selected color/clear controls and all400gold pixels are verified. Layout changes and animated
ring timing preclude general image identity. All focused stderr is empty; no new CNA issue.
Generated COVERAGE.md remains complete467-scene until full467-light closes; inventory totals
remain293/329types,1373/2326XNA members,174/3210C routes.

Complete checkpoint **467-light**, source `2588b74`, preserves all 467 IDs. Build: seven
scheduled operations; metadata 467/0; all 480 enabled CTests pass in 235.91 s (448 passed,
including 27 WILL_FAIL, 32 capability skips; three additional manual tests disabled).
SOFTWARE: 405 passed / 27 failed / 32 skipped / 3 not run. OPENGLES3: 432 / 29 / 3 / 3.
Both inner validation and outer Xvfb independently exit zero. All 38/42 original failed
assertions, C/custom child lists and traces, and five network evaluation checks are retained,
allowing precisely ten new positive C SpriteBatch End-release checks (19 -> 29).
Skinned lighting checks increase 4 -> 8 on both renderers. Exact JSON count variations are
archived in each renderer's review; only positive window.title Update variance is allowed.
All 35 selected binary and four asset hashes remain unchanged. Normalized application stderr
is identical, including 42 known GL X protocol records and the deliberate shader compiler
error. Separate outer Xvfb diagnostics contain 16 exact amdgpu pairs (previously 13); no cause
or hardware GPU claim. CNA 714f9f76a was observed clean at capture/final, not an atomic input
snapshot. All capture/archive/finish helpers pass. Immutable evidence: checkpoint-467-light-*.
Coverage remains 293/329 types, 1373/2326 XNA members and 174/3210 C routes. No new demo ID or
route was needed for these behavioral extensions; comprehensive coverage is not established.

Focused467-field keeps all467IDs and current API inventory. Build12scheduled/11completed
operations and metadata467/0 pass. All18targeted CLI/volume/C-sprite CTests pass in4.06s
(17passed/one declared SOFTWARE CustomEffects skip). Volume10->16checks covers signedVector4
versus scalar storage, compact two-slice boxes, guards and refused-count preservation on both
renderers. C SpriteBatch29->30children (31->32with capture) retains every old label/order and
adds invalid-second-handle array refusal before enqueue. All eight actual before/after images
were inspected; both volume RGBA pairs are identical, all400left-corner/400gold pixels remain
exact. The C ring is time-dependent. All stderr is empty; no new CNA issue. This does not
establish float sampling, packed formats, direct GPU readback or native-flush rollback.
Generated COVERAGE.md remains complete467-light until the new full checkpoint finishes;
inventory remains293/329types,1373/2326XNA members,174/3210C routes.

Complete checkpoint **467-field**, source `d4eee34`, preserves all 467 IDs. The external CNA
checkout advanced from714f9f76a to clean1bc9ff044 (LINK-032 mold archive extraction) at the
build boundary. CMake regenerated and scheduled53operations (50numbered completions), including
relinking CNA and its consumers. Metadata467/0 and all480enabled CTests pass in326.00s
(448passed incl27WILL_FAIL,32capability skips; three additional manual tests disabled).
The known vendored Draco CMP0148 developer warning remains; no compiler diagnostic is hidden.
Concurrent cc1plus processes/host load22.10were observed, without isolating a performance cause.
SOFTWARE405/27/32/3 and OPENGLES3 432/29/3/3; independently journaled inner/outer exits both0.
Exact comparisons retain all38/42old failed assertions, all C/custom child lists/traces and
all five network evaluation controls, allowing only one new C sprite array-refusal child
(29->30). Volume10->16is the intended JSON count change on both. Any positive window.title
Update variance is recorded explicitly in the renderer reviews.
All35selected binary/fourasset hashes captured after the rebuild remain stable. Normalized
application stderr is identical, including42known GL X protocol records and the intentional
shader compiler error. Separate outerXvfb diagnostics contain15exact amdgpu pairs (previously16);
no hardware GPU claim. After-build source capture observed cleanCNA1bc9ff044; external sources
subsequently advanced (mid-run547c3092a plus optional-service changes; final 60aedeab57 with external uncommitted changes).
Selected runtime hashes stayed fixed across that source work. Focused images used prior714f9f76a.
The full checkpoint exercises the captured binaries, not those later source edits; this is not
an atomic source/input snapshot.
All capture/archive/finish helpers pass; immutable evidence: checkpoint-467-field-*.
Generated coverage remains293/329types,1373/2326XNA members,174/3210C routes. Semantic coverage
grew without adding IDs/routes; comprehensive coverage is not established.

Focused checkpoint 467-move extends existing Texture3D sampling with move construction,
occupied-destination move assignment and explicit disposal/destruction of both source wrappers.
All six original GL controls remain, plus two ownership checks; SOFTWARE retains its declared
CustomEffects skip. The actual GL images were inspected and are RGBA-identical, with fifteen
sample centers verified. Metadata: 467 demos, zero problems. All 17 targeted CLI/volume CTests
pass (one declared capability skip), 3.50 s. GL stderr is empty. The initial broad build
scheduled 671 operations (668 numbered completions), followed by a seven-operation consistency
build; both exit zero and all 3,906 watched CNA source/header/CMake hashes stay unchanged
across each build. CNA commits advanced from 3a1d77b47 to 0ae1c41b1 while already-dirty content
was committed; this is not evidence of changed watched contents during compilation. The known
vendored Draco CMP0148 developer warning remains. These observations are not an atomic or
full transitive input snapshot. Full checkpoint validation is pending.
