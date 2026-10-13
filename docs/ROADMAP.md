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

## Phase 6 -- effects and shaders 🟨 (38 demos: stock effects, effect API, PbrEffect)
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

## Phase 9 -- audio and media 🟨 (audio: 23 demos incl. 10 XACT, media: 12 incl. video)
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
- ✅ `audio.dynamic_sound_effect_instance.buffer_formats`: stereo byte/sample ranges, refused
  partial integer frames, float range refusals, paused format guards/queue clearing and stopped
  integer/float transitions. Twenty-three controls pass with dummy audio; no audible output or concurrent-producer claim.
- ✅ `audio.microphone.input_level` (manual).
- ✅ `audio.xact.*` (5): play a cue, cue states, categories, global variables, positioned cues; XACT
  files generated by `tools/xact_files.py` in the layout CNA's own example writes;
  ✅ `audio.xact.renderer_details`, ✅ `audio.xact.streaming_wave_bank` (ISSUE-028),
  ✅ `audio.xact.cue_variables`. ✅ Four controlled-cue demos now own GetCue results correctly.
  ✅ `cue_states` adds an authored500ms release, bounded Stopping/Stopped transition and
  Immediate interruption;15checks pass in six dummy-audio runs, with no audible-envelope claim.
  ✅ `positional_cue` retains its audible loop and adds6owned-Cue Apply3D variable/state
  controls beforePlay and while paused; SmokeOnly remains and mixed output is not measured.
  ✅ `bank_lifetime` distinguishes prepared and paused consuming cues during bank/engine
  disposal, with12public ownership/event checks and caller-owned wrapper cleanup.
  ✅ `instance_limits` retains its category scenario and adds five per-cue definition
  capacity controls;12checks pass using one extra original sound bank and existing PCM.
  ✅ `audio.xact.instance_limits`: authored category Fail/ReplaceOldest limits, paused slots,
  isolation and freed-slot reuse checked through public states. Remaining: RPC curves (a variable driving
  volume/pitch), complex variation selection and cue-level limits.
- ✅ `media.*` (12): play a song (State, PlayPosition, end), events through FrameworkDispatcher
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

## Phase 14 -- bindings 🟨 (26 C API demos)
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
✅ `c_api.input.devices` (descriptor values, host enumeration, absent motion sensors, deterministic
mouse/keyboard hot-plug events and one power snapshot),
✅ `c_api.input.clipboard` (UTF-8 size/copy/has-text, atomic short-buffer refusal and exact
desktop-state restoration),
✅ `c_api.input.text_input` (deterministic committed UTF-16, UTF-8 composition/candidates,
registration/reset lifecycle and bound/unbound activation),
✅ `c_api.input.joystick` (descriptor values, disconnected owned raw-state snapshots and
deterministic connected/disconnected event lifecycle),
✅ `c_api.input.haptics` (direction/effect/capability values and complete safe closed-device
rumble/effect/control/ownership behavior),
✅ `c_api.input.touch_state` (deterministic press/move/release snapshots, id lookup and previous locations),
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

Complete checkpoint **467-move**, source `1721771`, preserves all 467 IDs. Focused validation
retains six original GL sampler controls plus two ownership controls, with inspected identical
before/after RGBA and fifteen verified centers. SOFTWARE retains its declared CustomEffects skip.
The broad focused build and a consistency build each observed unchanged hashes for 3,906 watched
CNA module/header/CMake files. Source commits advanced while prior dirty contents were committed;
this does not itself imply changing build inputs. The known vendored Draco CMake developer warning
is preserved. The full validation build scheduled seven operations; metadata467/0 and all480enabled
CTests pass in207.06s (448passed incl27WILL_FAIL,32skips; three additional manual tests disabled).
SOFTWARE405/27/32/3 and OPENGLES3 432/29/3/3. All38/42previous failed assertions, every C/custom child
list/trace and all five network evaluation controls remain exact, with no additions or filtering.
The planned JSON change is GL sampler6->8; positive window.title Update variance, if present,
is recorded separately in the renderer reviews. All35selected binary/fourasset hashes captured
after the full build remain stable. Normalized application stderr is unchanged, including the42
known GL X protocol records and intentional shader error. Separate outerXvfb diagnostics contain
15exact amdgpu pairs (previously15). Both independently journaled inner/outer exits are0.
Capture source observation: CNA0ae1c41b18 (dirty);
final observation: CNA5a85d5ed24 (clean).
Selected artifact verification is not an atomic source or full transitive dependency snapshot.
All capture/archive/finish helpers pass; immutable evidence: checkpoint-467-move-*.
Generated coverage remains293/329types,1373/2326XNA members,174/3210C routes. Semantic ownership
coverage grew without adding IDs; comprehensive coverage is not established.

Focused checkpoint 467-readers extends the existing reader_versions demo without adding an ID.
All17original custom-policy controls remain unchanged, with six built-in lifetime controls:
Game declaration, explicit withdrawal, cached survival, uncached refusal without resurrection,
refusal after Unload, and a fresh load after a subsequent Game declaration. Both requested
renderer environments pass23checks with empty stderr. Console ownProcess isolates registry
mutation; neither Game runs and no rendering/visual credit is claimed. The deterministic90byte
signed Int32 fixture is staged identically; all three old fixtures remain byte-identical.
The nine-operation build (eight numbered completions) exits0 with no diagnostics and no changed
hashes among3,906watched CNA source/header/CMake inputs. Metadata467/0; all23targeted CLI/XNB/
manifest CTests pass in3.16s, including one expected known-issue failure. Full validation is pending.

Complete checkpoint **467-readers**, source `583ee6f`, preserves all 467 IDs. The existing
reader_versions demo retains every17custom-policy control and adds six built-in lifetime controls:
withdrawal, cache independence, refusal without resurrection and a subsequent Game declaration.
Both requested environments pass23checks; neither Game runs and no renderer/visual credit is claimed.
Console ownProcess keeps the mutation out of the parent/browser registry. Original three fixtures
remain byte-identical; the new90byte signed primitive is generated and staged identically.
Focused build9operations/8numbered completions, metadata467/0 and23targeted CTests3.16s passed.
The full build schedules7operations; metadata467/0 and all480enabled CTests pass in169.66s
(448passed incl27WILL_FAIL,32skips; three additional manual tests disabled).
SOFTWARE405/27/32/3 and OPENGLES3 432/29/3/3. All38/42previous failed assertions, every C/custom
child list/trace and all five network controls remain exact. Only planned JSON changes are
reader17->23both; positive window.title Update variance, if present, is recorded in the reviews.
All35selected binary/fourexistingasset hashes and both copies of the newfixture remain stable
through runtime. Normalized application stderr is unchanged, including42known GL X protocol
records and the deliberate shader compiler error. Separate outerXvfb diagnostics contain11
exact amdgpu pairs (previously15); independently journaled inner/outer exits are0.
Capture source observation: CNAe842e74733 (clean);
final observation: CNAe842e74733 (clean).
The e842e7473 commit before capture changed only external link-architecture documentation.
These source observations and selected runtime hashes are not an atomic/full dependency snapshot.
All capture/archive/finish helpers pass; immutable evidence: checkpoint-467-readers-*.
Generated coverage now records293/329types,1374/2326XNA members,174/3210C routes; explicit
Game construction adds one named member. Semantic reader
lifetime coverage grew without a new ID; comprehensive coverage is not established.

Focused checkpoint468-formats adds one stereo buffer-format scenario (468catalog demos).
Sixteen checks pass in both requested environments with dummy audio and empty stderr, covering
integer byte versus float sample ranges, partial-frame refusal with preserved queue, pause/resume,
live-paused symmetric format guards and stopped transitions in both directions. XNA duration
helpers retain16-bit PCM semantics after float submission. The old event-refill source and its
known disposed-getter failures are unchanged. No Game, renderer/image, sample-readback or audible
quality credit is claimed. The build regenerated CMake and scheduled33operations (30numbered
completions); all3,906watched CNA input hashes stayed unchanged. The existing vendored Draco
CMP0148 developer warning remains. Metadata468/0; all37enabled audio/CLI CTests pass in7.47s
(one additional manual microphone test disabled). Full checkpoint validation is pending.

Initial full468-formats attempt stopped at CTest: audio.xact.play_cue failed its bank-idle
assertion after a fixed700ms pump (one failure among481enabled tests,182.28s); no smoke phase
started. Original logs, both exit1journals, source d70f2ca and captured unchanged35runtime hashes
are retained as checkpoint-468-formats-initial-ctest-refusal-*. An independent public XACT probe
ran five default and five requested-large-buffer trials without reproducing the delay; actual
buffer-size acceptance and the transient cause were not measured/isolated. The demo now pumps
until actual idle state or a3s steady-clock deadline, retaining all three original assertions.
No issue waiver or CNA fix claim. Nine-operation rebuild, metadata468/0, ten native repetitions
(three checks each, empty stderr) and all24XACT/CLI CTests3.18s pass. Wait observations0.345-0.394s
start after PlayCue/firstUpdate and do not measure audible duration. A fresh full attempt is next.

Complete checkpoint **468-formats**, final source `aad1917`, adds one coherent
stereo buffer-format console example. All16new controls pass in both requested environments:
byte/sample ranges, integer frame-alignment refusals, preserved paused streams and stopped
format transitions. Original event-refill source/ISSUE038 are unchanged. No renderer/image,
output-sample, audible-quality or concurrent-producer credit is claimed.
The initial full attempt (source d70f2ca, session6704) stopped at CTest after play_cue's fixed700ms
idle assertion failed; one failure among481tests in182.28s. No smoke ran. Original logs, exit1
journals and runtime hashes are preserved as checkpoint-468-formats-initial-ctest-refusal-*.
Ten independent public XACT probe trials did not reproduce the transient; actual buffer-hint
acceptance and the original cause remain unisolated. The demo now polls actual bank state while
pumping with a3s steady-clock limit, retaining all three original predicates. Ten corrected
native repetitions and24XACT/CLI CTests pass, empty stderr; this is no CNA or audible-timing fix.
A fresh full attempt (session98882) builds in7scheduled operations, validates468metadata/0problems
and passes all481enabled CTests in211.17s (449passed incl27WILL_FAIL,32skips; three additional
manual tests disabled). SOFTWARE406/27/32/3 and OPENGLES3 433/29/3/3. Every old38/42failed assertion,
C/custom child list/trace and all five network controls remain exact. The new audio example
passes16; old count differences are limited to any separately reviewed positive title Update
variance. All35selected binary/fourexistingasset hashes plus readerfixture copies stay stable.
Normalized application stderr is identical, including42known GL X protocol records and the
intentional shader error. The fresh outerXvfb log contains16exact amdgpu pairs (previously11);
independently journaled inner/outer exits are0. Capture source observation CNAe842e74733
(clean), final CNAe842e74733 (clean).
These are selected artifact/source observations, not an atomic/full transitive input snapshot.
All capture/archive/finish helpers pass; immutable complete evidence: checkpoint-468-formats-*.
Generated coverage: 293/329types, 1374/2326XNA members, 174/3210C routes.
Semantic format coverage and the completion-check correction do not establish comprehensive CNA coverage.

Focused checkpoint469-shadow adds one authored-map SkinnedEffect receiver (469catalog demos).
Four paired-pixel GL controls pass: disabled/enabled reception, excessive positive depth bias
and map detachment. A post-skin light-space lookup removes direct diffuse while retaining the
predicted ambient/emissive floor. The actual800x480OPENGLES3image was inspected; all eight
selected pixels match within tolerance1, stderr empty. SOFTWARE explicitly skips its unsupported
stock shadow path with the declared diagnostic. OPENGL33 is in the declared EasyGL scope but
was not run. No shadow-map generation, PCF kernels, cascades, punctual/specular-shadow or other
renderer credit. CMake regenerated;33scheduled/30numbered build operations, no changed hashes
among3,906watched CNA inputs. The known vendored Draco developer warning remains. Metadata469/0;
all25targeted CLI/skinned CTests pass in4.04s (one declared shadow skip). Full validation pending.

## Complete 469-shadow

Complete checkpoint **469-shadow**, source `a4a2a31`, adds authored-map shadow
reception after skinning. Four paired-pixel controls pass on OPENGLES3: disabled reception,
shadowed direct diffuse with retained ambient/emission, excessive bias and detached map.
The actual image was inspected; eight selected pixels match predictions within tolerance 1.
SOFTWARE explicitly skips the unsupported stock path. OPENGL33 is declared but untested.
No shadow-map generation, PCF kernel, cascade, punctual/specular-shadow or other-renderer credit.
Full session 13123 builds in 7 scheduled operations, validates 469 metadata entries / 0 problems,
and passes all 482 enabled CTests in 220.45s (449 passed including 27 WILL_FAIL, 33 skips;
three additional manual tests disabled). SOFTWARE 406/27/33/3, OPENGLES3 434/29/3/3.
Every old 38/42 failed assertion, C/custom child list/trace and all five network controls remain
exact. Old JSON count differences: {"OPENGLES3": {}, "SOFTWARE": {}}.
All 35 selected binaries / four existing assets and reader fixture copies remain unchanged.
Normalized stderr differs only by the expected SOFTWARE shadow skip diagnostic; GL retains
42 known X protocol records and its intentional shader error. The outer Xvfb log contains
22 exact amdgpu pairs (previously 16); independently journaled inner/outer exits are zero.
Capture CNA e842e74733 (clean), final CNA e842e74733 (clean).
These are selected artifact/source observations, not an atomic/full transitive input snapshot.
All capture/archive/finish helpers pass; immutable evidence: checkpoint-469-shadow-*.
Generated coverage: 293/329 types, 1374/2326 XNA members, 174/3210 C routes.
Semantic coverage remains incomplete; named-member counts do not establish all behaviors.

Focused 469-pcf extends the existing shadow_receiver without adding an ID. Four original
paired-pixel controls remain; a lower row compares one-tap, 3x3 and 5x5 visibility (0, 1/3,
2/5). All seven GL checks pass; SOFTWARE retains its exact declared skip. The actual image
was inspected and eleven selected pixels match within tolerance 1. No whole-image identity
is claimed after rearranging the panels. Build44646 closed0, 9 scheduled / 8 numbered
operations; all 3,906 watched CNA inputs unchanged at clean e842e7473. Metadata469/0.
Focused/targeted24236 closed0; all25CLI/skinned CTests pass in3.67s, including the declared
skip. Full469-pcf validation is pending. Evidence: checkpoint-469-pcf-focused-*.

The public-member audit now classifies OcclusionQuery separately: existing depth/visibility,
precision and polling evidence versus constructor refusals, ordering and disposal gaps.
A source-only preflight proposal is queued after PCF and cascades; no new runtime credit.

## Complete 469-pcf

Complete checkpoint **469-pcf**, source `b8f3d0d`, extends shadow_receiver
without adding an ID. Four original paired-pixel controls remain; three lower-row controls
compare one-tap, 3x3 and 5x5 kernels with visibility 0, 1/3 and 2/5. All seven GL checks pass.
The actual image was inspected; eight original and three new selected pixels match within
tolerance 1. Markers locate the kernel samples. No image-identity claim after the layout change.
SOFTWARE retains its declared skip; OPENGL33 remains untested. No map generation, cascades,
other edge orientations, out-of-range radius clamping or punctual/specular-shadow credit.
Full session 18517 builds in 7 scheduled operations, validates 469 metadata entries / 0 problems,
and passes all 482 enabled CTests in 223.34s (449 passed including 27 WILL_FAIL, 33 skips;
three additional manual tests disabled). SOFTWARE 406/27/33/3, OPENGLES3 434/29/3/3.
Every old 38/42 failed assertion, C/custom child list/trace and all five network controls remain
exact. Reviewed JSON count differences: {"OPENGLES3": {"graphics.skinned_effect.shadow_receiver": {"after": 7, "before": 4, "reason": "Three authored-map kernel controls added; original four paired-pixel predicates retained."}}, "SOFTWARE": {}}.
All 35 selected binaries / four existing assets and reader fixture copies remain unchanged.
Normalized application stderr is identical, retaining 42 known GL X protocol records and the
intentional shader error. The outer Xvfb log contains 18 exact amdgpu pairs (previously 22);
independently journaled inner/outer exits are zero. Capture CNA e842e74733
(clean), final CNA 123c36c34b (clean).
These are selected artifact/source observations, not an atomic/full transitive input snapshot.
All capture/archive/finish helpers pass; immutable evidence: checkpoint-469-pcf-*.
Generated coverage remains 293/329 types, 1374/2326 XNA members, 174/3210 C routes.
Semantic coverage grew without increasing named-member or demo counts; it remains incomplete.

External CNA advanced during the frozen checkpoint from e842e7473 to clean123c36c34b:
LINK-041 changes declared CNB loader installation/removal locking, LINK-042 refines static
linker-flag matching. Selected runtime hashes remained unchanged; these newer source changes
are not credited as exercised by469-pcf. The next watched build must incorporate them.
Evidence: checkpoint-469-pcf-external-source-delta.json. No sibling files were edited.

Focused470-cascades adds one coherent authored-atlas receiver. Six GL pixel controls pass:
near and inclusive-split selection, far selection, half visibility in the blend band,
independent fitting CameraView, and Count0 restoring the separate single-map transform.
The actual 800x480 image was inspected; all six selected colors match within tolerance1.
SOFTWARE explicitly skips the unsupported stock path; GL stderr is empty. OPENGL33 remains
untested. No map generation, atlas-border filtering, four-cascade maximum or debug-tint proof.
The watched build incorporates external LINK-041/LINK-042 at CNA123c36c34b;
Initial build observed 6 changed watched CNA inputs during external work; the consistency rebuild then completed with unchanged inputs. No native checks ran between.
The final watch covers 3,909 CNA source/header/CMake inputs; it is not
an atomic or full transitive dependency snapshot.
Metadata470/0; all26CLI/skinned CTests pass in4.36s, including two declared shadow skips.
Full470-cascades validation is pending. Evidence: checkpoint-470-cascades-focused-*.

## Complete 470-cascades

Complete checkpoint **470-cascades**, source `370bf37`, adds a focused
six-control authored-atlas scene: near/inclusive-split/far selection, half visibility in a
blend band, independent fitting CameraView and Count0 restoring the single-map transform.
All six GL checks pass; the actual image was inspected and six selected colors match within
tolerance1. SOFTWARE explicitly skips the unsupported stock path; OPENGL33 remains untested.
No shadow-map generation, atlas-border filtering, four-cascade maximum or DebugTint credit.
Initial build33371 incorporates external CNA LINK-041/LINK-042 and completes584scheduled /
581numbered operations, but detects six watched CMake input changes during external work.
No native checks ran until consistency90087 completed32scheduled/29numbered operations with
all3,909watched inputs unchanged. Focused/targeted16253 passes26CTests in4.36s, two shadow skips.
Full21772 builds in7scheduled operations, validates470metadata/0problems and passes all483
enabled CTests in221.58s (449passed including27WILL_FAIL,34skips; three additional manual
tests disabled). SOFTWARE406/27/34/3, OPENGLES3 435/29/3/3. Every old38/42failed assertion,
C/custom child list/trace and all five network controls remain exact. Old JSON count differences:
{"OPENGLES3": {}, "SOFTWARE": {}}.
All35selected binaries/four existing assets and readerfixture copies remain unchanged.
Normalized application stderr adds only the declared SOFTWARE cascade skip; GL retains42known
X protocol records and the intentional shader error. The outer Xvfb log contains12exact
amdgpu pairs (previously18); independently journaled inner/outer exits are zero.
Capture CNA123c36c34b (dirty), final CNAdc9c34f62f (clean).
These are selected artifact/source observations, not an atomic/full transitive input snapshot.
All capture/archive/finish helpers pass; immutable evidence: checkpoint-470-cascades-*.
Generated coverage: 293/329types, 1374/2326XNA members, 174/3210C routes.
Semantic coverage remains incomplete; authored cascade sampling does not establish all shadows.

Focused 470-query extends the existing visible_pixels demo with eight scratch-query
ordering/disposal controls, retaining its three original visibility controls. Both SOFTWARE
and OPENGLES3 pass11checks with empty stderr. All four actual before/after images were
inspected; the complete Update/Draw/EndRun source remains unchanged. Moving-light positions
vary with time, so no image identity or GPU completion-latency claim. Original fixed counts
remain0/4422 (SOFTWARE, precise) and0/1 (OPENGLES3, boolean). Build81666 closed0:9scheduled /
8numbered operations, all3,909watched inputs unchanged at clean CNAdc9c34f62. Metadata470/0.
Before70538 and after/targeted95414 closed0; all16CLI/query CTests pass in3.46s. Focused reviewer
passed before the source commit, preserving the original runtime code against HEAD.
Full470-query validation is pending. Evidence: checkpoint-470-query-focused-*.

The initial full470-query attempt stopped before CTest on a content-tool staging warning.
A private native probe using unchanged CNA staging code reproduces the exact diagnostic in
all three concurrent trials (1,000 owners each), while a1,000-owner sequential control reports
zero diagnostics. No owner errors/removals, empty final private parents, unchanged35selected
binaries/four assets. This is CNA-DEMOS-ISSUE-057, a build-tool diagnostic race; the original
build interleaving remains unobserved and no content corruption is claimed. Original failure,
probe sources/executables/logs and review are archived under checkpoint-470-query-initial-staging-refusal-*
and checkpoint-470-query-staging-probe-*. The warning gate remains unchanged; a fresh full retry
is pending. No runtime demo knownIssue marker or extra demo ID is added.

## Complete 470-query

Complete checkpoint **470-query**, source `68980d0` (query code a699a2d),
extends the existing occlusion scene with eight ordering/disposal controls. All eleven checks
pass on SOFTWARE and OPENGLES3; the three original visibility controls and complete
Update/Draw/EndRun source remain unchanged. Four actual before/after images were inspected.
Fixed counts remain 0/4422 (SOFTWARE, precise) and 0/1 (OPENGLES3, boolean). Moving-light
positions are time-dependent; no image-identity or GPU completion-latency claim.
Initial watched build81666 passes9scheduled/8numbered operations, all3,909watched inputs
unchanged at clean dc9c34f62. Before70538 and after/targeted95414 pass;16CTests in3.46s.
The initial full attempt (sourcea699a2d) stopped before metadata/CTest/smoke after a staging
warning, inner/outer exits1. Original evidence: checkpoint-470-query-initial-staging-refusal-*.
Private unchanged-CNA-header concurrency probes reproduce the missing-metadata diagnostic in
all three1,000-owner trials, while a1,000-owner sequential control has no diagnostics. Zero
owner errors/scavenger removals and empty final private parents; selected35binaries/4assets
stay unchanged through probes. This is open CNA-DEMOS-ISSUE-057, a tool diagnostic race, not
an occlusion failure or demonstrated content corruption. Original exact interleaving remains
unobserved. Probe evidence: checkpoint-470-query-staging-probe-*. A clean retry does not fix it.
Fresh full16616 builds in7scheduled operations without warning, validates470metadata/0problems
and passes all483enabled CTests in165.69s (449passed including27WILL_FAIL,34skips; three
additional manual tests disabled). SOFTWARE406/27/34/3, OPENGLES3 435/29/3/3. Every old38/42
failed assertion, C/custom child list/trace and all five network controls remain exact.
Reviewed JSON count differences: {"OPENGLES3": {"graphics.occlusion_query.visible_pixels": {"after": 11, "before": 3, "reason": "Eight scratch-query lifecycle controls added; original three visibility predicates retained."}}, "SOFTWARE": {"graphics.occlusion_query.visible_pixels": {"after": 11, "before": 3, "reason": "Eight scratch-query lifecycle controls added; original three visibility predicates retained."}}}.
All35selected binaries/four existing assets and readerfixture copies remain unchanged through
this fresh runtime attempt. Normalized application stderr is identical, including42known GL
X protocol records and the intentional shader error. The fresh outer Xvfb log contains13
exact amdgpu pairs (previous complete12, initial refusal1); independent inner/outer exits0.
Capture CNAeff3131017 (clean), final CNAeff3131017 (clean).
These are selected artifact/source observations, not an atomic/full transitive input snapshot.
All fresh capture/archive/finish helpers pass; immutable complete evidence: checkpoint-470-query-*.
Generated coverage: 293/329types, 1374/2326XNA members, 174/3210C routes.
Semantic coverage remains incomplete; no extra demo or runtime knownIssue marker was added.

## Focused470-border

Focused **470-border** extends the existing cascade demo from six to eight GL checks.
Both slices receive identical atlas UV(.5,.5), depth.5 and requested radius2; near/far
selection retains the independent occluded/lit colors at the shared boundary. Original
six assertion labels remain exact. The actual800x480 three-row image was inspected and
all eight selected pixels match within tolerance1; no image-identity/all25tap-weight claim.
SOFTWARE retains its exact declared skip, GL stderr is empty. Watched build36795 passes
9scheduled/8numbered operations with all3,909watched inputs unchanged. Metadata470/0,
focused/targeted19479 passes all26CTests in2.69s (two declared shadow skips). Focused
review passes before commit; evidence checkpoint-470-border-focused-*. Full validation is
next; last complete baseline470-query. No new demo ID or genuine CNA failure found.

## Complete470-border

Complete checkpoint **470-border**, source `54dfda7`,
extends the existing cascade demo with two shared-boundary isolation controls. All eight
GL checks pass; SOFTWARE retains its declared stock-path skip. The actual three-row image
was inspected and six relocated original plus two new selected pixels match within 1.
Both new controls use atlas UV (.5,.5), receiver depth .5 and requested radius 2. Selected
near/far slices remain independently occluded/lit. This establishes slice isolation, not
all 25 filter weights inside a cascade, arbitrary atlas layouts or other renderers.
Watched build 36795 passes 9 scheduled / 8 numbered operations with all 3,909 watched inputs
unchanged. Focused/targeted session 19479 passes 26 CTests in 2.69 s; original six labels retained.
Full session 88833 builds in 7 scheduled operations without warning, validates 470 metadata entries with 0 problems
and passes all 483 enabled CTests in 150.80 s (449 passed including 27 WILL_FAIL, 34 skips;
three additional manual tests disabled). SOFTWARE 406/27/34/3, OPENGLES3 435/29/3/3.
Every old 38/42 failed assertion, C/custom child list/trace and all five network controls
remain exact. Reviewed JSON count differences:
{"OPENGLES3": {"graphics.skinned_effect.shadow_cascades": {"after": 8, "before": 6, "reason": "Two shared-edge slice-isolation controls added; original six cascade predicates retained."}}, "SOFTWARE": {}}.
All 35 selected binaries/four existing assets and reader fixture copies remain unchanged
through runtime. Normalized application stderr is identical, including 42 known GL X protocol
records and the intentional shader error. Outer Xvfb has 18 exact amdgpu pairs (previous 13);
independent inner/outer exits 0. Capture CNA eff3131017 (clean),
final CNA eff3131017 (clean).
These are selected artifact/source observations, not an atomic/full transitive input snapshot.
All capture/archive/finish helpers pass; immutable evidence: checkpoint-470-border-*.
Generated coverage: 293/329 types, 1374/2326 XNA members, 174/3210 C routes.
The additional source-only spatial-audio audit classifies listener/emitter properties,
Doppler, orientation, multi-listener approximation and Pan/3D mode changes. It corrects
an old range-error gap: volume_pitch_pan already checks three refused ranges. No listening
or runtime credit comes from that source review.
No new ID or issue marker. Semantic coverage remains incomplete; build-only ISSUE-057 stays
open with its original refusal and independent private staging probe preserved.

## Focused470-specular

Focused **470-specular** extends shadow_receiver from seven to nine GL checks while
preserving all original assertion labels. Two additional panels remove diffuse/ambient/
emission and compare disabled/enabled reception of directional specular light. Both
highlights remain RGB(76,38,13) with reception disabled; enabling it makes only the
occluded half black. Actual three-row800x480 image inspected; all15 selected pixels,
including all original controls at new coordinates, match within tolerance1. GL stderr
is empty; SOFTWARE retains its exact declared skip. No whole-image identity, per-vertex,
alpha, cascade or punctual-specular claim. Watched build10959 passes9scheduled/8numbered
operations with all3,909inputs unchanged. Metadata470/0; focused/targeted75346 passes
26CTests in2.64s (two declared shadow skips). Focused reviewer passes BEFORE source commit;
evidence checkpoint-470-specular-focused-*. Last full baseline470-border; full run is next.

## Complete470-specular

Complete checkpoint **470-specular**, source `c9660d8`,
extends the existing shadow receiver from seven to nine GL controls. All nine pass with
empty focused stderr; SOFTWARE retains its exact declared stock-path skip. Seven original
assertion labels remain unchanged. The actual three-row 800x480 image was inspected and
all 15 selected pixels, including relocated original controls, match within tolerance 1.
The new specular-only panels have zero diffuse, ambient and emission. Disabled reception
preserves both RGB(76,38,13) highlights; enabled reception makes only the occluded half
black. This is single-map per-pixel directional specular evidence, not per-vertex, alpha,
cascade or punctual-specular behavior; no whole-image identity after the layout change.
Watched build 10959 passes 9 scheduled / 8 numbered operations with all 3,909 watched
inputs unchanged. Focused/targeted session 75346 passes 26 CTests in 2.64 s (two skips).
Full session 80513 builds in 7 scheduled operations without warning, validates 470 metadata
entries with 0 problems and passes all 483 enabled CTests in 150.15 s (449 passed,
including 27 WILL_FAIL; 34 skips; three additional manual tests disabled).
SOFTWARE 406/27/34/3, OPENGLES3 435/29/3/3. Every old 38/42 failed assertion, C/custom child
list/trace and all five network controls remain exact. Reviewed JSON count differences:
{"OPENGLES3": {"graphics.skinned_effect.shadow_receiver": {"after": 9, "before": 7, "reason": "Two isolated specular shadow controls added; original seven receiver predicates retained."}}, "SOFTWARE": {}}.
All 35 selected binaries/four existing assets and reader fixture copies remain unchanged
through runtime. Normalized application stderr is identical, including 42 known GL X
protocol records and the intentional shader error. Outer Xvfb has 17 exact amdgpu
pairs (previous 18); independent inner/outer exits 0. Capture CNA eff3131017
(clean), final CNA eff3131017 (clean).
These are selected artifact/source observations, not an atomic/full transitive input snapshot.
All capture/archive/finish helpers pass; immutable evidence: checkpoint-470-specular-*.
Generated coverage: 293/329 types, 1374/2326 XNA members, 174/3210 C routes.
No new ID or issue marker. Semantic coverage remains incomplete; build-only ISSUE-057 stays
open with its original refusal and independent private staging probe preserved.

## Focused470-release

Focused **470-release** extends the existing looping demo from three to six checks.
All ten dummy-audio repetitions (five per requested renderer environment) pass with empty
stderr. Stop(false) leaves a paused voice paused and its configured IsLooped true; Resume
starts it, then it naturally completes within a3s deadline. Original function source through
all three old checks is unchanged. Observed waits after Resume range from
0.239 to 0.261s; these are polling observations, not audible duration/latency or sample evidence.
Watched build64542 passes9scheduled/8numbered operations, all3,909inputs unchanged.
Metadata470/0; focused/targeted78295 passes37enabled CTests in5.65s; one manual microphone
test disabled. Focused reviewer passes BEFORE commit; evidence checkpoint-470-release-focused-*.
Last complete baseline470-specular. No extra ID or genuine CNA problem found; full run next.

## Complete470-release

Complete checkpoint **470-release**, source `f08554f`,
extends the existing looping demo from three to six checks. Ten dummy-audio repetitions
(five per requested renderer environment) pass with empty stderr. The complete original
function prefix through its three old controls remains unchanged. Stop(false) leaves a
paused voice paused with configured IsLooped true; Resume starts it, and it then naturally
stops within a 3 s steady-clock deadline. Observed post-Resume waits in focused repetitions
are 0.239-0.261 s; these are polling observations, not audible duration/latency or sample
content evidence. Pausing before release avoids a racy immediate-Playing assertion.
Watched build 64542 passes 9 scheduled / 8 numbered operations with all 3,909 watched
inputs unchanged. Focused/targeted session 78295 passes 37 enabled CTests in 5.65 s;
one additional manual microphone test is disabled. Metadata has 470 entries / 0 problems.
Full session 59103 builds in 7 scheduled operations without warning and passes all 483
enabled CTests in 157.07 s (449 passed including 27 WILL_FAIL; 34 skips; three additional
manual tests disabled). SOFTWARE 406/27/34/3, OPENGLES3 435/29/3/3. Every old 38/42 failed
assertion, C/custom child list/trace and all five network controls remain exact.
Reviewed JSON count differences:
{"OPENGLES3": {"audio.sound_effect_instance.looping": {"after": 6, "before": 3, "reason": "Three paused-loop release controls added; original three looping predicates retained."}}, "SOFTWARE": {"audio.sound_effect_instance.looping": {"after": 6, "before": 3, "reason": "Three paused-loop release controls added; original three looping predicates retained."}}}.
All 35 selected binaries/four existing assets and reader fixture copies remain unchanged
through runtime. Normalized application stderr is identical, including 42 known GL X
protocol records and the intentional shader error. Outer Xvfb has 18 exact amdgpu
pairs (previous 17); independent inner/outer exits 0. Capture CNA eff3131017
(clean), final CNA eff3131017 (clean).
These are selected artifact/source observations, not an atomic/full transitive input snapshot.
All capture/archive/finish helpers pass; immutable evidence: checkpoint-470-release-*.
Generated coverage: 293/329 types, 1374/2326 XNA members, 174/3210 C routes.
No extra ID or issue marker. Semantic coverage remains incomplete; build-only ISSUE-057
stays open with its original refusal and independent private staging probe preserved.

## Focused470-spatial

Focused **470-spatial** extends apply_3d from one to seven checks. Two dummy-audio runs
(one per requested renderer environment) pass with empty stderr. Six public controls cover
paused Pan/3D mode refusals, re-aiming after Stop, the Pan field assignment before its
refusal, and switching back to pan. The complete original audible sweep/default-speed
check source remains verbatim. No mixer-output panning, attenuation, Doppler or new
listening evidence; SmokeOnly remains. Watched build53385 passes9scheduled/8numbered
operations with all3,909inputs unchanged and diagnostic gate0. Metadata470/0;
focused/targeted84713 passes37enabled CTests in5.60s (one manual microphone disabled).
Focused reviewer passes BEFORE commit; evidence checkpoint-470-spatial-focused-*.
Last full baseline470-release; full validation next. No extra ID or genuine CNA defect found.

## Complete470-spatial

Complete checkpoint **470-spatial**, source `9a6234b`,
extends the existing apply_3d demo from one to seven checks. Two dummy-audio runs (one per
requested renderer environment) pass with empty stderr. The complete original audible
sweep/default-speed check source remains verbatim. Six added controls demonstrate paused
Pan/3D refusals, selecting the other mode after Stop, and re-aiming a paused3D voice.
The refused Pan setter exposes its newly assigned .75 public value while State remains
Paused. This is observed CNA property/state behavior, not by itself a CNA defect or a
measurement of output panning. No attenuation, Doppler or new listening credit; SmokeOnly
remains appropriate for the original audible sweep.
Watched build 53385 passes 9 scheduled / 8 numbered operations, all 3,909 watched inputs
unchanged and diagnostic gate 0. Focused/targeted session 84713 passes 37 enabled CTests
in 5.60 s; one additional manual microphone test disabled. Metadata 470 / 0 problems.
Full session 17556 builds in 7 scheduled operations without warning and passes all 483
enabled CTests in 148.64 s (449 passed including 27 WILL_FAIL; 34 skips; three additional
manual tests disabled). SOFTWARE 406/27/34/3, OPENGLES3 435/29/3/3. Every old 38/42 failed
assertion, C/custom child list/trace and all five network controls remain exact.
Reviewed JSON count differences:
{"OPENGLES3": {"audio.sound_effect_instance.apply_3d": {"after": 7, "before": 1, "reason": "Six public Pan/3D mode controls added; original sweep and speed-default predicate retained."}}, "SOFTWARE": {"audio.sound_effect_instance.apply_3d": {"after": 7, "before": 1, "reason": "Six public Pan/3D mode controls added; original sweep and speed-default predicate retained."}}}.
All 35 selected binaries/four existing assets and reader fixture copies remain unchanged
through runtime. Normalized application stderr is identical, including 42 known GL X
protocol records and the intentional shader error. Outer Xvfb has 18 exact amdgpu
pairs (previous 18); independent inner/outer exits 0. Capture CNA eff3131017
(clean), final CNA eff3131017 (clean).
These are selected artifact/source observations, not an atomic/full transitive input snapshot.
All capture/archive/finish helpers pass; immutable evidence: checkpoint-470-spatial-*.
Generated coverage: 293/329 types, 1374/2326 XNA members, 174/3210 C routes.
No extra ID or issue marker. Semantic coverage remains incomplete; build-only ISSUE-057
stays open with its original refusal and independent private staging probe preserved.

## Focused470-bounds

Focused **470-bounds** extends buffer_formats from16to20checks. Two dummy-audio runs
(one per requested renderer environment) pass with empty stderr. Negative float offset
and max-int count preserve the staged integer queue; negative count and end-offset with
positive count preserve the staged float queue. Each refusal is ArgumentOutOfRangeException
with Stopped/Pending1 retained. All16original assertion calls remain verbatim/in order,
including original integer/float mode guards and playback transitions. No partial-float
output, live-playing/concurrent-producer, sample-readback or listening claim.
Watched build1507 passes9scheduled/8numbered operations with all3,909inputs unchanged and
diagnostic gate0. Metadata470/0; focused/targeted14824 passes37enabled CTests in5.60s,
one additional manual microphone disabled. Focused reviewer passes BEFORE source commit;
evidence checkpoint-470-bounds-focused-*. Last full baseline470-spatial; full validation next.
No new ID or genuine CNA problem found. Float alignment acceptance remains a source-only
classification and must not be converted into an invented alignment exception.

## Complete470-bounds

Complete checkpoint **470-bounds**, source `41af406`,
extends buffer_formats from 16 to 20 checks. Two dummy-audio runs (one per requested
renderer environment) pass with empty stderr. Negative float offset and max-int count
preserve the staged integer queue; negative count and a nonempty range at the vector end
preserve the staged float queue. Each refusal is ArgumentOutOfRangeException with
Stopped/Pending1 retained. All 16 original assertion calls remain verbatim/in order,
including original mode guards and integer/float playback transitions. No sample readback,
partial-float output, live-playing/concurrent-producer or listening evidence. Float range
bounds differ from integer frame alignment; no invented alignment exception is expected.
Watched build 1507 passes 9 scheduled / 8 numbered operations, all 3,909 watched inputs
unchanged and diagnostic gate 0. Focused/targeted session 14824 passes 37 enabled CTests
in 5.60 s; one additional manual microphone test disabled. Metadata 470 / 0 problems.
Full session 24481 builds in 7 scheduled operations without warning and passes all 483
enabled CTests in 149.72 s (449 passed including 27 WILL_FAIL; 34 skips; three additional
manual tests disabled). SOFTWARE 406/27/34/3, OPENGLES3 435/29/3/3. Every old 38/42 failed
assertion, C/custom child list/trace and all five network controls remain exact.
Reviewed JSON count differences:
{"OPENGLES3": {"audio.dynamic_sound_effect_instance.buffer_formats": {"after": 20, "before": 16, "reason": "Four float-range refusal controls added; original sixteen buffer-format predicates retained."}}, "SOFTWARE": {"audio.dynamic_sound_effect_instance.buffer_formats": {"after": 20, "before": 16, "reason": "Four float-range refusal controls added; original sixteen buffer-format predicates retained."}}}.
All 35 selected binaries/four existing assets and reader fixture copies remain unchanged
through runtime. Normalized application stderr is identical, including 42 known GL X
protocol records and the intentional shader error. Outer Xvfb has 11 exact amdgpu
pairs (previous 18); independent inner/outer exits 0. Capture CNA eff3131017
(clean), final CNA eff3131017 (clean).
These are selected artifact/source observations, not an atomic/full transitive input snapshot.
All capture/archive/finish helpers pass; immutable evidence: checkpoint-470-bounds-*.
Generated coverage: 293/329 types, 1374/2326 XNA members, 174/3210 C routes.
No extra ID or issue marker. Semantic coverage remains incomplete; build-only ISSUE-057
stays open with its original refusal and independent private staging probe preserved.

## Focused470-clear

Focused **470-clear** extends buffer_formats from20to23checks. Guarded application verified
the exact validated41af406 source base before copying. Two dummy-audio runs (one per
requested renderer environment) pass with empty stderr. ClearBuffers retainsPaused/Pending0;
a valid float submission still refuses with that same state; same-format refill and visible
QueueInitialBuffers retainPaused/Pending1 before originalResume/Stop. All20original assertion
calls remain verbatim/in order. Pending combines staged/submitted data and does not alone
prove native transfer or sample output. No live-playing/concurrent-producer or listening claim.
Watched build47439 passes9scheduled/8numbered operations with all3,909inputs unchanged and
diagnostic gate0. Metadata470/0; focused/targeted51461 passes37enabled CTests in5.61s,
one additional manual microphone disabled. Focused reviewer passes BEFORE commit;
evidence checkpoint-470-clear-focused-*. Last full baseline470-bounds; full validation next.
Original mono streaming/ISSUE-038 untouched; no new ID or genuine CNA defect found.

## Complete470-clear

Complete checkpoint **470-clear**, source `10a9593`,
extends buffer_formats from 20 to 23 checks. Guarded application verified the unchanged
validated41af406 base. Two dummy-audio runs (one per requested renderer environment) pass
with empty stderr. ClearBuffers retainsPaused/Pending0; a valid float submission still
refuses with that same state; same-format refill and visible QueueInitialBuffers retain
Paused/Pending1 before originalResume/Stop. All 20 original assertion calls remain verbatim
and in order. Counts combine staged/submitted data and do not independently prove native
transfer or output samples. No live-playing/concurrent-producer or listening claim.
Watched build 47439 passes 9 scheduled / 8 numbered operations, all 3,909 watched inputs
unchanged and diagnostic gate 0. Focused/targeted session 51461 passes 37 enabled CTests
in 5.61 s; one additional manual microphone test disabled. Metadata 470 / 0 problems.
Full session 79728 builds in 7 scheduled operations without warning and passes all 483
enabled CTests in 149.94 s (449 passed including 27 WILL_FAIL; 34 skips; three additional
manual tests disabled). SOFTWARE 406/27/34/3, OPENGLES3 435/29/3/3. Every old 38/42 failed
assertion, C/custom child list/trace and all five network controls remain exact.
Reviewed JSON count differences:
{"OPENGLES3": {"audio.dynamic_sound_effect_instance.buffer_formats": {"after": 23, "before": 20, "reason": "Three paused-queue clearing and format-retention controls added; original twenty predicates retained."}}, "SOFTWARE": {"audio.dynamic_sound_effect_instance.buffer_formats": {"after": 23, "before": 20, "reason": "Three paused-queue clearing and format-retention controls added; original twenty predicates retained."}}}.
All 35 selected binaries/four existing assets and reader fixture copies remain unchanged
through runtime. Normalized application stderr is identical, including 42 known GL X
protocol records and the intentional shader error. Outer Xvfb has 14 exact amdgpu
pairs (previous 11); independent inner/outer exits 0. Capture CNA eff3131017
(clean), final CNA eff3131017 (clean).
These are selected artifact/source observations, not an atomic/full transitive input snapshot.
All capture/archive/finish helpers pass; immutable evidence: checkpoint-470-clear-*.
Generated coverage: 293/329 types, 1374/2326 XNA members, 174/3210 C routes.
Original mono streaming/ISSUE-038 is untouched. No extra ID or issue marker; semantic
coverage remains incomplete. Build-only ISSUE-057 stays open with its original refusal
and independent private staging probe preserved.

## Focused470-fourth

Focused **470-fourth** extends shadow_cascades from eight to ten GL controls.
Count4 with fitting depth7 selects slot3. Its comparison depth .5 yields lit RGB(173,130,77);
.9 with DebugTint yields shadow-floor RGB(20,28,15). Both actual800x480 images were viewed;
all original eight pixels remain correct and regions outside the replaced ninth cell are
RGBA-identical. Original eight drawing/check loops remain verbatim. SOFTWARE retains the
exact stock-path skip and GL stderr is empty. No every-index/debug-color, invalid-count,
arbitrary-atlas, shadow-producer or other-renderer credit, and no whole-image identity.
Watched build18210 passes9scheduled/8numbered operations, all3909inputs unchanged and
warning/error gate0. Metadata470/0. Focused/targeted21966 passes26CTests in2.71s, including
two declared shadow skips. Focused reviewer passes BEFORE source commit. Evidence:
checkpoint-470-fourth-focused-*. Full validation against complete470-clear is pending.

## Complete470-fourth

Complete checkpoint **470-fourth**, source `6f27142`,
extends the existing shadow_cascades from eight to ten GL pixel checks. At Count4,
fitting depth7 selects slot3: its comparison depth .5 is lit RGB(173,130,77), while
.9 with DebugTint produces the shadow floor RGB(20,28,15). CNA uses four horizontal
atlas strips. Both actual800x480 GL images were inspected; all eight original pixels
remain, two new pixels match within tolerance1, and regions outside the replaced ninth
cell are RGBA-identical. The original eight drawing/check loops remain verbatim.
SOFTWARE retains its exact declared skip; GL stderr is empty. This does not demonstrate
every four-cascade index/debug color, arbitrary atlases, invalid counts, a producer or
other renderers. No whole-image identity or additional demo ID is claimed.
The watched build and focused review passed before source commit; original evidence is
under checkpoint-470-fourth-focused-*. Full validation passes all483enabled CTests in
148.94s, with three additional manual tests disabled. SOFTWARE406/27/34/3 and
OPENGLES3 435/29/3/3 retain all38/42prior failed assertions, C/custom child evidence and
five network controls. Reviewed JSON count changes:
{"OPENGLES3": {"graphics.skinned_effect.shadow_cascades": {"after": 10, "before": 8, "reason": "Two fourth-slot depth/debug-tint controls added; original eight cascade predicates retained."}}, "SOFTWARE": {}}.
All35selected binaries, four existing assets and reader fixture copies stay unchanged
through runtime. Normalized application stderr is identical, including42known GL X
protocol records and the intentional shader error. Outer Xvfb has17exact amdgpu
pairs (previous14); independent inner/outer exits are0. Capture CNA
eff3131017 (clean), final CNA
eff3131017 (clean). These are selected
artifact/source observations, not an atomic or full transitive input snapshot.
All capture/archive/finish helpers pass; immutable evidence: checkpoint-470-fourth-*.
Semantic coverage remains incomplete, and build-only ISSUE-057 stays open with its
original staging refusal and independent private reproduction preserved.

## Focused470-authored

Focused **470-authored** extends cue_states from nine to fifteen public state checks.
Six dummy-audio runs (three per requested renderer environment) pass with empty stderr.
All nine original predicates remain in order; one misleading Immediate label is corrected.
The original no-fade bank stops immediately with AsAuthored. A new original three-second
PCM bank and complex single-sound cue with500ms fadeOutMS demonstrate Stopping to bounded
Stopped, then Immediate interruption of a fresh tail. Observed state waits are
500–510ms including caller scheduling; no audible-envelope or output-sample timing claim.
All six old XACT assets remain byte-identical and all eight source/staged banks match.
Watched build62470 passes9scheduled/8numbered operations,3909inputs unchanged and diagnostic
gate0. Metadata470/0. Focused/targeted8158 passes37enabled CTests in5.72s, with one additional
manual microphone test disabled. Focused reviewer passes BEFORE source commit; original
reports/streams/hashes: checkpoint-470-authored-focused-*. Full validation is pending.

## Complete470-authored

Complete checkpoint **470-authored**, source `f44b514`,
extends cue_states from nine to fifteen checks. Six dummy-audio runs pass with empty
stderr. All nine original predicates remain in order; the misleading Immediate label
is corrected. Public controls now compare immediate no-fade AsAuthored stop, an authored
cue entering Stopping and reaching Stopped, and Immediate interruption of a fresh tail.
The generator adds an original three-second PCM bank and a complex single-sound cue with
500ms fade-out. All six existing XACT assets remain byte-identical; all eight source/staged
banks are checked. Observed state waits are 500–510ms, including caller
scheduling and polling; no acoustic envelope, audible quality or output-sample timing claim.
The watched build and focused review passed before source commit; original evidence is
under checkpoint-470-authored-focused-*. Full validation passes all483enabled CTests in
149.50s, with three additional manual tests disabled. SOFTWARE406/27/34/3 and
OPENGLES3 435/29/3/3 retain all38/42prior failed assertions, C/custom child evidence and
five network controls. Reviewed JSON count changes:
{"OPENGLES3": {"audio.xact.cue_states": {"after": 15, "before": 9, "reason": "Six authored-release controls added; original nine predicates retained, one misleading label corrected."}}, "SOFTWARE": {"audio.xact.cue_states": {"after": 15, "before": 9, "reason": "Six authored-release controls added; original nine predicates retained, one misleading label corrected."}}}.
All35selected binaries, four existing assets and18fixture hashes (source/staged builtinXNB
and all eight XACT banks) stay unchanged through runtime. Normalized application stderr
is identical, including42known GL X protocol records and the intentional shader error.
Outer Xvfb has9exact amdgpu pairs (previous17); independent inner/outer exits0.
Capture CNA eff3131017 (clean), final CNA
eff3131017 (clean). These are selected
artifact/source observations, not an atomic or full transitive input snapshot.
All capture/archive/finish helpers pass; immutable evidence: checkpoint-470-authored-*.
Semantic coverage remains incomplete. Original mono streaming/ISSUE-038 is untouched;
build-only ISSUE-057 retains its staging refusal and private reproduction evidence.

## Focused471-punctual

Focused **471-punctual** adds one coherent point/spot receiver scene with eight GL
pixel controls. Both authored maps store normalized distance64/255; the receiver's post-
skin distance3/Range6 is.5. The spot matrix deliberately gives projected depth.1, so the
shadow control distinguishes distance comparison from projected depth. No map preserves
direct light, attaching the map removes it, and bias.3 restores it. Separate range/cone
controls remove direct light. Linear intensity4 gives lit RGB(74,64,43), versus floor
RGB(20,28,26). The actual800x480 image was inspected and all eight centers match within1.
GL stderr is empty; SOFTWARE retains the exact declared stock-path skip. No producer,
all cube faces/seams, partial spot-kernel weights, cone-edge interpolation, punctual
specular or other-renderer proof. All27targeted CLI/skinned CTests pass in3.19s,
including three declared shadow skips. Initial watched build22225 compiled33scheduled operations but gate1 correctly refused
ISSUE-057 missing-owner diagnostics. Original evidence is retained under
checkpoint-471-punctual-initial-staging-refusal-*. Unchanged watched consistency retry
passes7scheduled operations with3909inputs unchanged and diagnostic gate0. Focused/targeted
8468 and the pixel reviewer pass BEFORE source commit. Metadata471/0; full validation against470-authored is pending.
Evidence: checkpoint-471-punctual-focused-*.

## Complete471-punctual

Complete checkpoint **471-punctual**, source `ed7daab`,
adds one coherent SkinnedEffect point/spot receiver demo with eight GL pixel checks.
Authored constant-distance cube/2D maps store64/255; post-skin distance3/Range6 compares
at.5. No map preserves direct diffuse, the map removes it, and normalized bias.3 restores
it. The spot matrix gives projecteddepth.1, deliberately different from distance.5,
so its shadow control distinguishes the comparison quantity. Separate controls exclude
the receiver by range or cone. Linear light intensity4
produces RGB(74,64,43), versus the ambient/emissive floor RGB(20,28,26). The actual800x480
GL image was inspected and all eight selected pixels match within1. GL stderr is empty;
SOFTWARE has the exact declared unsupported-stock-path skip. No producer, all cube
faces/seams, partial spot-kernel weights, cone-edge interpolation or other-renderer proof.
PunctualLightEXT offers diffuse only; no public punctual specular capability is invented.
Initial focused build22225 compiles33scheduled operations but its diagnostic gate
correctly refuses the recurrent ISSUE-057 missing-owner warning. Original logs/input
observations/headers and post-build hashes remain under
checkpoint-471-punctual-initial-staging-refusal-*. The unchanged consistency retry passes
7scheduled operations with3909watched inputs stable and gate0; no CNA edit, warning
suppression or staging-directory deletion. Native/27CTest/pixel review then pass before
source commit. A clean retry/full run does not resolve the independently reproduced race. Evidence:
checkpoint-471-punctual-focused-*. Full validation passes all484enabled CTests in159.93s,
with three additional manual tests disabled. SOFTWARE406/27/35/3 and OPENGLES3 436/29/3/3
retain all38/42prior failed assertions, C/custom child evidence and five network controls.
The new ID is graphics.skinned_effect.punctual_shadows. Reviewed old JSON count changes:
{"OPENGLES3": {}, "SOFTWARE": {}}.
All35selected binaries, four existing assets and18fixture hashes (source/staged builtinXNB
and all eight XACT banks) stay unchanged through runtime. Normalized stderr differs only
by the expected new SOFTWARE skip; GL retains42known X protocol records and the intentional
shader error. Outer Xvfb has14exact amdgpu pairs (previous9); independent
inner/outer exits0. Capture CNA eff3131017 (clean),
final CNA eff3131017 (clean). These are selected
artifact/source observations, not an atomic or full transitive input snapshot.
All capture/archive/finish helpers pass; immutable evidence: checkpoint-471-punctual-*.
Generated coverage:293/329types,1374/2326XNA members.
Semantic coverage remains incomplete; original mono streaming/ISSUE-038 and build-only
ISSUE-057 remain intact, with the latter's staging refusal/private reproduction retained.

## Focused471-cue3d

Focused **471-cue3d** extends positional_cue from zero to six public checks while
retaining its complete original audible eight-position sweep and SmokeOnly classification.
Two dummy-audio runs pass with empty stderr and still print all eight original positions.
An owned Cue::Apply3D publishes Distance5, OrientationAngle0 and neutral Doppler beforePlay;
turning the emitter sideways changes its public angle to90degrees. A paused cue accepts
Distance10 and approaching velocity at one tenth of public SpeedOfSound, giving raw
DopplerPitchScalar10/9. These are public variable/state observations, not acoustic panning,
mixed pitch, RPC curve application or listening evidence. All37enabled audio/CLI CTests
pass in5.78s; one additional manual microphone test is disabled. Watched build,
diagnostic gate and focused reviewer pass BEFORE sourcecommit. Metadata471/0; full
validation against complete471-punctual is pending. Evidence: checkpoint-471-cue3d-focused-*.

## Complete471-cue3d

Complete checkpoint **471-cue3d**, source `057d783`,
extends positional_cue from zero to six checks while retaining its entire original
8-position audible sweep and SmokeOnly classification. Two dummy-audio runs pass with
empty stderr. Explicit owned Cue::Apply3D publishes Distance5, OrientationAngle0 and
neutral Doppler beforePlay; reorientation yields90degrees. While paused, a new position
and velocity produce Distance10 and raw Doppler10/9. These are public variable/state
observations, not acoustic panning, mixed pitch, RPC curve application or listening.
The watched build/diagnostic gate and37targeted CTests pass before source commit;
original loop preservation and actual observations are checkpoint-471-cue3d-focused-*.
Full validation passes all484enabled CTests in150.01s, with three additional manual
tests disabled. SOFTWARE406/27/35/3 and OPENGLES3 436/29/3/3 retain all38/42prior failed
assertions, C/custom child evidence and five network controls. Reviewed count changes:
{"OPENGLES3": {"audio.xact.positional_cue": {"after": 6, "before": 0, "reason": "Six public Cue spatial-variable/state controls added; entire original audible loop retained."}}, "SOFTWARE": {"audio.xact.positional_cue": {"after": 6, "before": 0, "reason": "Six public Cue spatial-variable/state controls added; entire original audible loop retained."}}}.
All35selected binaries, four existing assets and18fixture hashes stay unchanged through
runtime. Normalized stderr is identical, including42known GL X protocol records and the
intentional shader error. Outer Xvfb has13exact amdgpu pairs (previous14);
independent inner/outer exits0. Capture CNA eff3131017
(clean), final CNA eff3131017
(clean). These are selected artifact/source
observations, not an atomic or full transitive input snapshot. All capture/archive/finish
helpers pass; immutable evidence: checkpoint-471-cue3d-*. Generated coverage:
293/329types and1378/2326XNA members. No extra demo ID.
Semantic coverage remains incomplete. Original mono streaming/ISSUE-038 is unchanged;
ISSUE-057 retains its original staging refusal, private reproduction and471-punctual
recurrence. Subsequent clean runs do not establish a repair of that race.

## Focused472-banks

Focused **472-banks** adds audio.xact.bank_lifetime with12 public ownership checks.
Two dummy-audio runs pass with empty stderr. Prepared cues leave both banks idle; a paused
consuming cue holds both banks in use. WaveBank disposal disposes that consumer while the
prepared cue and sound bank survive. SoundBank disposal also disposes prepared cues.
Public event senders/counts establish exactly-once notifications across repeated disposal
and destruction of the disposed wrappers. Fresh banks register with the still-live engine;
engine disposal cascades to both banks and both cue states. Caller-owned allocations remain
unique_ptr-owned. No global callback-order, concurrent teardown or acoustic-latency claim.
Watched build/diagnostic gate, metadata472/0, two focused runs and all38enabled audio/CLI
CTests pass in6.23s before source commit; one additional manual microphone test is
disabled. Original assets are reused. Full validation against complete471-cue3d is pending.
Evidence: checkpoint-472-banks-focused-*.

## Complete472-banks

Complete checkpoint **472-banks**, source `5fe4921`,
adds audio.xact.bank_lifetime with12 public ownership checks in each requested environment.
A paused consuming cue holds both banks in use; WaveBank disposal disposes it while a
prepared never-played cue and SoundBank survive. SoundBank disposal releases prepared
cues too. Sender/exactly-once counts persist across repeated disposal and deletion of
disposed wrappers. A fresh graph of banks/cues is released by AudioEngine disposal.
Caller-owned allocations remain unique_ptr-owned. No global callback-order, concurrent
teardown or acoustic-latency claim. Two focused runs have empty stderr;38enabled audio/CLI
CTests, watched build/diagnostic gate and focused reviewer pass before source commit.
Full validation passes all485enabled CTests in165.34s, with three additional manual
tests disabled. SOFTWARE407/27/35/3 and OPENGLES3 437/29/3/3 retain all38/42prior failed
assertions, C/custom child evidence and five network controls. One newID passes12checks;
reviewed old count changes:
{"OPENGLES3": {}, "SOFTWARE": {}}.
All35selected binaries, four existing assets and18fixture hashes stay unchanged through
runtime. Normalized stderr is identical, including42known GL X protocol records and the
intentional shader error. Outer Xvfb has18exact amdgpu pairs (previous13);
independent inner/outer exits0. Capture CNA eff3131017
(clean), final CNA eff3131017
(clean). These are selected artifact/source
observations, not an atomic or full transitive input snapshot. All capture/archive/finish
helpers pass; immutable evidence: checkpoint-472-banks-*. Generated coverage:
293/329types and1386/2326XNA members.
Semantic coverage remains incomplete. Original mono streaming/ISSUE-038 is unchanged;
ISSUE-057 retains its original staging refusal, private reproduction and471-punctual
recurrence. Subsequent clean runs do not establish a repair of that race.

## Focused472-cuelimits

Focused **472-cuelimits** extends audio.xact.instance_limits from7to12 checks.
Its complete original category-limit scenario remains verbatim. Five additional controls
use unlimited categories and LowTone's authored1/Fail cue limit: a paused first instance
occupies the slot, a duplicate is refused, another Music cue and another SoundBank loaded
from the same file retain independent capacity, and Stop frees only the relevant slot.
Two dummy-audio runs pass all12checks with empty stderr. Caller-owned cues remain
unique_ptr-owned. No replacement-policy, fade, mixed-output or concurrency claim.
The original generator gains optional per-cue limits in complex single-sound records;
new cue_limits.xsb is364bytes and reuses original PCM. All8old assets remain byte-identical;
all9source/staged bank pairs match. Watched build/diagnostic gate, metadata472/0,
38enabled audio/CLI CTests in6.97s and focused reviewer pass before source commit.
One additional manual microphone CTest is disabled. Full validation against complete472-banks
is pending. Evidence: checkpoint-472-cuelimits-focused-*. No extra demo ID.

## Complete472-cuelimits

Complete checkpoint **472-cuelimits**, source `edc32f7`,
extends instance_limits from 7 to 12 public checks, preserving its complete original
category-limit scenario. With spare category capacity, one paused LowTone occupies its
cue-definition slot and a duplicate is refused. A different same-category definition and
a separate SoundBank loaded from the same file have independent capacity. Stop frees the
relevant slot while those other voices remain paused. Two focused runs pass with empty
stderr; 38 enabled audio/CLI CTests, watched build/gate and reviewer pass before source
commit. No replacement-policy, fade, mixed-output or concurrency claim. The new 364-byte
cue_limits.xsb reuses original PCM; all 8 old bank assets remain byte-identical and all
9 source/staged bank pairs match. No extra demo ID.
Full validation passes all 485 enabled CTests in 195.64s; three additional manual tests
are disabled. SOFTWARE 407/27/35/3 and OPENGLES3 437/29/3/3 retain all 38/42 prior failed
assertions, C/custom child evidence and five network controls. Reviewed count changes:
{"OPENGLES3": {"audio.xact.instance_limits": {"after": 12, "before": 7, "reason": "Five per-cue definition capacity controls added; original seven category checks retained."}}, "SOFTWARE": {"audio.xact.instance_limits": {"after": 12, "before": 7, "reason": "Five per-cue definition capacity controls added; original seven category checks retained."}}}.
All 35 selected binaries, four existing assets and 20 fixture hashes stay unchanged through
runtime. Normalized stderr is identical, including 42 known GL X protocol records and the
intentional shader error. Outer Xvfb has 14 exact amdgpu pairs (previous 18);
independent inner/outer exits are zero. Capture CNA eff3131017
(clean), final CNA eff3131017
(clean). These are selected artifact/source
observations, not an atomic or full transitive input snapshot. All capture/archive/finish
helpers pass; immutable evidence: checkpoint-472-cuelimits-*. Generated coverage:
293/329 types and 1386/2326 XNA members.
Semantic coverage remains incomplete. Original mono streaming/ISSUE-038 is unchanged;
ISSUE-057 retains its original staging refusal, private reproduction and 471-punctual
recurrence. Subsequent clean runs do not establish a repair of that race.

## Focused473-music

Focused **473-music** adds media.media_library.music_index, a standalone Linux workflow
with 15 exact child checks in each requested environment. Each stderr has exactly six reviewed TitleContainer INFO
lines for opening the three original assets. Parent JSON
reports zero checks; the actual child lists are archived and compared. Both public XDG
music/picture roots are verified private before library access. Three original tagged FLAC
tracks yield two artists, two same-name albums distinguished by artist and unequal by Album::Equals, one genre and one
M3U playlist. Public names/track numbers, group membership, shared Song back-references
and reversed playlist order agree. All returned objects/views remain library-owned.
No duration, playback, art, other-tag-format or other-platform claim. The fixture encoder
produced identical bytes twice; ffprobe confirms tags, and all source/staged copies match.
The initial reviewer refused its incorrect empty-stderr expectation. Original evidence and
reviewer remain under checkpoint-473-music-focused-initial-stderr-refusal-*. Re-review accepts
only the six exact INFO lines; no source change, logging suppression or native rerun.
Watched build/gate, metadata473/0, two native runs, 28 enabled media/CLI CTests in 10.12s
and focused reviewer pass before source commit. Full validation against complete472-cuelimits
is pending. Evidence: checkpoint-473-music-focused-*.

## Complete473-music

Complete checkpoint **473-music**, source `979a99b`,
adds an isolated tagged music-library workflow with 15 identical child checks in each
requested environment. Focused stderr retains six exact TitleContainer requested/resolved
INFO lines; the initial empty-stderr review refusal is preserved without changing code. Parent JSON has zero checks; the child
lists are explicitly preserved. Both public XDG roots are verified private before library
access. Three original tagged FLAC tracks create two artists, two same-name albums scoped
by artist and unequal by Album::Equals, one genre and an ordered M3U playlist. Public metadata, group membership and
Song back-references share library-owned objects. No duration, playback, album-art,
other-tag-format or other-platform claim. Two private encodings produced identical bytes
and ffprobe verified tags; all three source/staged fixtures match. Watched build/gate,
28 enabled media/CLI CTests and focused reviewer pass before source commit.
Full validation passes all 486 enabled CTests in 149.86s; three additional manual tests
are disabled. SOFTWARE 408/27/35/3 and OPENGLES3 438/29/3/3 retain all 38/42 prior failed
assertions, C/custom child evidence and five network controls. The new standalone's 15
child checks match the focused run exactly. Reviewed old count changes:
{"OPENGLES3": {}, "SOFTWARE": {}}.
All 36 selected binaries, four existing assets and 26 fixture hashes stay unchanged through
runtime. Normalized stderr adds only those six exact INFO lines, retaining 42 known GL X protocol records and the
intentional shader error. Outer Xvfb has 20 exact amdgpu pairs (previous 14);
independent inner/outer exits are zero. Capture CNA eff3131017
(clean), final CNA eff3131017
(clean). These are selected artifact/source
observations, not an atomic or full transitive input snapshot. All capture/archive/finish
helpers pass; immutable evidence: checkpoint-473-music-*. Generated coverage:
297/329 types and 1400/2326 XNA members.
Semantic coverage remains incomplete. Original mono streaming/ISSUE-038 is unchanged;
ISSUE-057 retains its original staging refusal, private reproduction and 471-punctual
recurrence. Subsequent clean runs do not establish a repair of that race.

## Focused473-hierarchy

Focused **473-hierarchy** extends audio.xact.categories from five to ten checks.
All five original predicates and the independent Music/SFX workflow remain, with only the
incorrect identity label corrected to name equality. A separate original160byte hierarchy.xgs
makes Master the parent of SFX and reuses the existing PCM/sound bank. Five added checks
establish live paused direct/descendant voices, parent Resume/Pause, child-only Resume and
parent Stop from a verified both-paused state. Two dummy-audio runs pass ten checks with
empty stderr;38enabled audio/CLI CTests pass in 6.80s, plus one disabled manual microphone
test. Watched build/gate, metadata473/0 and focused reviewer pass before source commit.
All nine old XACT files remain byte-identical; all ten source/staged pairs match.
No future-voice pause inheritance, audible volume, deep hierarchy or category-fade claim.
Full473-hierarchy is pending against complete473-music. Evidence:checkpoint-473-hierarchy-focused-*.

## Complete473-hierarchy

Complete checkpoint **473-hierarchy**, source `be5ac32`,
extends categories from5to10checks while preserving all original predicates/independent
Music/SFX behavior and correcting only the equality label to names. A separate160byteXGS
makes Master parent of SFX: live paused direct/descendant voices, parent Resume/Pause,
child-only Resume and parent Stop from verified both-paused state pass in two dummy-audio
runs with empty stderr. All9oldXACTassets are unchanged; all10source/staged pairs match.
Focused38enabled audio/CLI CTests and reviewer passed before source commit. No new ID,
future-voice pause inheritance, audible volume, deep hierarchy or category-fade credit.

Full tools/validate.sh passes metadata473/0 and486enabled CTests in 166.07s (three further
manual tests disabled). SOFTWARE408/27/35/3 and OPENGLES3 438/29/3/3 preserve all38/42old
failed assertions, C/custom child lists/traces and five network controls. Music import retains
15exact child checks; parentJSON0. Count changes:
{"OPENGLES3": {"audio.xact.categories": {"after": 10, "before": 5, "reason": "Five authored hierarchy controls added; original five predicates retained, name-equality label corrected."}}, "SOFTWARE": {"audio.xact.categories": {"after": 10, "before": 5, "reason": "Five authored hierarchy controls added; original five predicates retained, name-equality label corrected."}}}.
All36selected binaries/four existing assets/28fixture hashes stay unchanged through runtime.
Normalized stderr is identical, including six music TitleContainer INFO lines,42known GL
X protocol records and the intentional shader error. Outer Xvfb has14exact amdgpu pairs
(previous20); independent inner/outer exits are zero. Capture CNAeff3131017
(clean), final CNAeff3131017
(clean). These are selected artifact/source observations,
not an atomic or full transitive input snapshot. All evidence helpers pass; immutable
checkpoint-473-hierarchy-*. Generated coverage:297/329types,
1400/2326XNA members. Semantic coverage remains incomplete.
Original mono streaming/ISSUE-038 and build-only ISSUE-057 remain unchanged; clean runs do
not establish a repair of the staging race. Original refusals and reproduction remain archived.

## Queue replay lifetime investigation

A public-API ASan probe after complete473-hierarchy confirms CNA-DEMOS-ISSUE-058. Passing an
independently caller-owned Song to `MediaPlayer::Play` succeeds, while passing the valid current
`MediaPlayer::Queue.ActiveSong` aborts: Play clears the owning queue and then reads the freed Song
in LoadSong. Exact allocation/free/read stacks, original streams and exits are archived under
`checkpoint-473-hierarchy-*queue-replay*`; all36selected project binaries remained unchanged.
No catalog knownIssue marker deliberately executes this undefined behavior. No CNA source was
edited and no playback-quality conclusion is drawn. The next semantic work remains positive
imported duration aggregation, followed by the separate Album art owned-stream workflow.

## Focused473-duration

Focused **473-duration** extends music_index from15to18child checks, preserving all
original tagged-library relationships and object identity checks. With CNA_FFMPEG_AVAILABLE,
Song durations match the original250/500/375msFLAC sample counts; artist-specific Album sums
are750/375ms and the two-track Playlist totals750ms while excluding the third song.
Two native runs pass18identical child checks; parentJSON remains PASSED/0. Both stderr streams
retain the same six exact TitleContainer INFO lines.28enabled media/CLI CTests pass in
7.58s. Watched build/gate, metadata473/0, unchanged fixtures and focused reviewer pass
before source commit. No zero-sum, playback, acoustic-duration or album-art claim.
The optional non-FFmpeg branch was syntax-checked only (1074), not natively exercised here;
it retains the15relationship checks and explicitly prints the unexercised duration boundary.
Full473-duration against complete473-hierarchy is pending. Evidence:checkpoint-473-duration-focused-*.

## Complete473-duration

Complete checkpoint **473-duration**, source `65011a3`,
extends music_index from 15 to 18 child checks. All original relationship and borrowed-object
controls remain; three added checks establish Song durations of 250/500/375 ms, artist-specific
Album sums of 750/375 ms and the two-track Playlist total of 750 ms excluding the third song.
Two focused runs pass 18 identical child checks with the same six TitleContainer INFO lines;
parent JSON remains PASSED/0. The current build has CNA_FFMPEG_AVAILABLE. The optional
non-FFmpeg branch has syntax-only evidence, preserves the original relationship workflow and
prints the unexercised duration boundary. No zero-sum, playback, audible timing or art claim.
Focused 28 enabled media/CLI CTests, watched build/gate and reviewer passed before source commit.
No new ID or fixture changes.

Full tools/validate.sh passes metadata 473/0 and 486 enabled CTests in 159.22s (three further
manual tests disabled). SOFTWARE 408/27/35/3 and OPENGLES3 438/29/3/3 preserve all 38/42 prior
failed assertions, C/custom child lists/traces and five network controls. Music alone gains
three exact child checks; all first 15 remain. Parent count changes:
{"OPENGLES3": {}, "SOFTWARE": {}}.
All 36 selected binaries, four existing assets and 28 fixture hashes stay unchanged through
runtime. Normalized stderr is identical, retaining the six music INFO lines, 42 known GL
X protocol records and intentional shader error. Outer Xvfb has 16 exact amdgpu pairs
(previous 14); independent inner/outer exits are zero. Capture CNA eff3131017
(clean), final CNA eff3131017
(clean). Selected artifact/source observations are not
an atomic or full transitive input snapshot. All evidence helpers pass; immutable archives:
checkpoint-473-duration-*. Generated coverage: 297/329 types,
1402/2326 XNA members. Semantic coverage remains incomplete.
Original mono streaming/ISSUE-038 and build-only ISSUE-057 remain unchanged; clean runs do
not establish a staging-race repair. Preserve all original refusals and reproduction evidence.

## Focused474-album-art

The new `media.media_library.album_art` standalone reuses two original tagged FLACs and the
existing640x320panorama inside verified private Linux XDG roots. Two native runs pass eight
identical child checks: two artist-specific albums, folder-art/no-art HasArt values, both
missing-art InvalidOperationException branches, caller-owned nonnull streams, exact original
bytes, a distinct public-decodable128x64thumbnail, and both streams readable after MediaLibrary
destruction. Parent JSON remains PASSED/0; each stderr has six exact TitleContainer INFO lines.
29enabled media/CLI CTests pass in7.69s. The first glob-triggered build attempt was refused only
because sandbox permissions blocked CNA's CMake lock; its log is preserved. A separate watched
consistency build passes with3909unchanged CNA inputs and an empty diagnostic gate. Metadata is
474/0; original source/staged assets and audited CNA source hashes remain exact. ISSUE-040 now
records native file-based Album evidence. No embedded-art, filename-priority, fallback,
visual-quality, playback or other-platform claim. Full474 checkpoint is pending.
Evidence:checkpoint-474-art-focused-* and checkpoint-474-art-*-build-*.

## Complete474-album-art

Complete checkpoint **474-album-art**, source `2524511`, adds one coherent standalone over
verified private Linux XDG roots. Two focused and two full-smoke runs pass eight identical child
checks: two artist-specific albums, folder-art/no-art HasArt, both missing-art exceptions,
nonnull caller-owned streams, exact original bytes, a distinct public-decodable128x64thumbnail,
and both streams readable after MediaLibrary destruction. ParentJSON remains PASSED/0. No
embedded art, filename priority, fallback, visual quality, playback or other-platform claim.
Focused29enabled media/CLI CTests pass. The initial sandbox-only CMake-lock refusal is archived;
the separate watched consistency build passed with3909unchanged inputs and no diagnostics.

Full tools/validate.sh passes metadata474/0 and487enabled CTests in162.75s. SOFTWARE
409/27/35/3 and OPENGLES3439/29/3/3 preserve all38/42old failed assertions, every C/custom
child list/trace, five network controls and all18music checks. The sole new ID adds eight exact
checks.37selected binaries, four general assets and30source/staged fixture paths remain exact.
Normalized stderr adds only six expected album-art INFO lines per renderer. Outer Xvfb has13
exact amdgpu pairs (previous16); inner/outer exits are zero. Capture/final CNAeff3131017 are
clean. Coverage is297/329types and1405/2326members. Semantic coverage remains incomplete.
Immutable evidence:checkpoint-474-art-*.

## Focused474-C-media-ownership

The existing `c_api.media.media_player` grows from32to46child checks without a new ID or fixture.
Both renderers pass identical lists with empty stderr; removing the14new controls yields every
old label in its original order. A collection-shared Rising handle survives collection destruction;
queue accessors return independent Rising/Falling copies; queue_add retains Appended after its
caller handle is destroyed; a replacement view sees the same three-entry global queue; stopped
queue_clear empties/resets it while every retained copy remains readable and releases independently.
29enabled media/CLI CTests pass in9.96s. Metadata remains474/0. The initial build refusal is
ccache sandbox-only and archived; the watched consistency build passes with3909unchanged CNA
inputs and no diagnostics. ISSUE-058's unsafe active-queue replay is never invoked. No audible,
empty/out-of-range, Dispose, failed-add, live-clear or callback edge claim. Full474 checkpoint
against complete474-album-art is pending. Evidence:checkpoint-474-c-media-focused-*.

## Complete474-C-media-ownership

Complete checkpoint **474-C-media-ownership**, source `7d23c6f`, extends the existing C media
demo from32to46child checks without another ID or fixture. Both focused and full runs retain all
32old labels in order and add14exact ownership controls with empty demo stderr. Collection sharing,
queue-owned copies, queue_add copying, view replacement and safe stopped-queue Clear all pass;
ISSUE-058 replay is never invoked. Focused29media/CLI CTests pass.

Full validation passes metadata474/0 and487enabled CTests in154.33s. SOFTWARE409/27/35/3 and
OPENGLES3439/29/3/3 preserve all38/42old failures and protected child/network evidence. C media
alone grows; parentJSON stays0.37binaries, four assets and30fixture paths remain exact. Normalized
stderr is identical. Outer Xvfb has15exact amdgpu pairs (previous13); exits are zero and CNA
eff3131017 remains clean. XNA coverage remains297/329types and1405/2326members; C ABI routes rise
174->184/3210, media.h2->7/39 and media_player.h12->17/41. Evidence:checkpoint-474-c-media-*.

## Focused474-C-media-lifecycle-bounds

The same `c_api.media.media_player` now grows46->53child checks without a new ID or fixture.
Both renderers pass the same exact list with empty stderr; removing seven lifecycle/bounds checks
restores every prior label in order. The collection starts count2/not disposed, Dispose clears and
marks it, a second Dispose succeeds, and the shared Rising handle stays live. After stopped Clear,
empty active/get_at outputs initialize to invalid, get_at(0) returns INVALID_ARGUMENT, index7 is
stored without an active Song and -1 is restored. The active-output result contradicts the header's
"untouched otherwise" wording and is documented as ISSUE-059; no failure marker is appropriate.
ISSUE-058 replay remains uncalled. Focused29media/CLI CTests pass. The initial ccache sandbox
refusal is preserved; the watched consistency build passes with3909unchanged CNA inputs and no
diagnostics. Metadata remains474/0 and actual C routes rise184->188/3210. No audible, failed-add,
live-clear or callback-edge claim. Full474 checkpoint is pending.
Evidence:checkpoint-474-c-media-bounds-focused-*.

## Complete474-C-media-lifecycle-bounds

Complete checkpoint **474-C-media-lifecycle-bounds**, source `5141458`, extends the same C media
demo46->53checks without another ID or fixture. Focused and full runs preserve all46old labels and
add seven exact collection lifecycle and empty-queue bounds controls. ISSUE-059 records the active
output documentation mismatch; ISSUE-058 replay remains uncalled. Focused29media/CLI CTests pass.

Full validation passes metadata474/0 and487enabled CTests in171.80s. SOFTWARE409/27/35/3 and
OPENGLES3439/29/3/3 preserve all38/42old failures and protected child/network evidence. C media
alone grows; parentJSON stays0.37binaries, four assets and30fixture paths remain exact. Normalized
stderr is identical. Outer Xvfb has14exact amdgpu pairs (previous15); exits are zero and CNA
eff3131017 remains clean. XNA coverage stays297/329types and1405/2326members; C ABI routes rise
184->188/3210, media.h7->10/39 and media_player.h17->18/41.
Evidence:checkpoint-474-c-media-bounds-*.

## Focused474-C-media-events

The same `c_api.media.media_player` now grows53->55child checks without a new ID or fixture. Both
renderers pass identical lists with empty stderr; removing the two new labels restores all53prior
checks in order. Explicit active-song/media-state raises synchronously increment their subscribed
counters exactly once. After both existing registration handles release, the same raises succeed
without invoking either handler. Focused29media/CLI CTests pass. The initial ccache sandbox refusal
is preserved; the watched consistency build passes with3909unchanged CNA inputs/no diagnostics.
Metadata remains474/0. Adding both raises and crediting the already material queue-count/unsubscribe
uses raises C routes188->192/3210 and media_player.h18->22/41. No callback order, reentrancy,
foreign-thread delivery or game-destruction cleanup claim. Full474 checkpoint is pending.
Evidence:checkpoint-474-c-media-events-focused-*.

## Complete474-C-media-events

Complete checkpoint **474-C-media-events**, source `02c81db`, extends the same C media demo
53->55checks without another ID or fixture. Focused and full runs preserve all53old labels and add
two exact controls: both process-wide manual raises invoke live handlers synchronously, then invoke
neither after both registrations release. Focused29media/CLI CTests pass. No callback ordering,
reentrancy, foreign-thread delivery or game-destruction cleanup claim.

Full validation passes metadata474/0 and487enabled CTests in151.27s. SOFTWARE409/27/35/3 and
OPENGLES3439/29/3/3 preserve all38/42old failures and protected child/network evidence. C media
alone grows; parentJSON stays0.37binaries, four assets and30fixture paths remain exact. Normalized
stderr is identical. Outer Xvfb has16exact amdgpu pairs (previous14); exits are zero and CNA
eff3131017 remains clean. XNA coverage stays297/329types and1405/2326members; C ABI routes rise
188->192/3210 and media_player.h18->22/41. Evidence:checkpoint-474-c-media-events-*.

## Focused474-C-song-duration

The same `c_api.media.media_player` now grows55->61child checks without a new ID or fixture. Both
renderers pass identical lists with empty stderr; removing six new labels restores all55prior checks
in order. A caller-owned Song over rising.wav stores explicit750ms as7,500,000ticks; fallback
completion is false one tick before and true at the boundary. After duration is set to zero, ten
elapsed seconds remain not-ended. IsDisposed starts false; Dispose succeeds twice, sets it true and
leaves Duration readable before the handle is destroyed. The helper is never played or queued.
Focused29media/CLI CTests pass. The initial ccache refusal is sandbox-only and preserved; the
watched consistency build passes with3909unchanged CNA inputs/no diagnostics. Metadata remains
474/0. Six actual routes raise coverage192->198/3210, media.h10->15/39 and media_player.h22->23/41.
No file-probe, audible timing, collection propagation or invalid-handle output claim. Full474 is
pending. Evidence:checkpoint-474-c-song-duration-focused-*.

## Complete474-C-song-duration

Complete checkpoint **474-C-song-duration**, source `e3a566d`, extends the same C media demo
55->61checks without another ID or fixture. Focused and full runs preserve all55old labels and add
six exact controls for explicit duration conversion, the elapsed-time boundary, zero-duration
fallback and idempotent disposal. The helper Song is never played or queued. Focused29media/CLI
CTests pass.

Full validation passes metadata474/0 and487enabled CTests in152.03s. SOFTWARE409/27/35/3 and
OPENGLES3439/29/3/3 preserve all38/42old failures and protected child/network evidence. C media
alone grows; parentJSON stays0.37binaries, four assets and30fixture paths remain exact. Normalized
stderr is identical. Outer Xvfb has12exact amdgpu pairs (previous16); exits are zero and CNA
eff3131017 remains clean. XNA coverage stays297/329types and1405/2326members; actual C ABI routes
rise192->198/3210, media.h10->15/39 and media_player.h22->23/41.
Evidence:checkpoint-474-c-song-duration-*.

## Focused474-C-song-identity

The same `c_api.media.media_player` now grows61->69child checks without a new ID or fixture. Both
renderers pass identical lists with empty stderr; removing eight additions restores all61prior
labels in order. Three caller-owned plain-path Songs establish exact stored handle text, equality
by path rather than handle or display name, an unequal different-path control, equal hashes for the
equal pair and literal empty-name storage. All three release independently and are never played or
queued. Focused29media/CLI CTests pass in11.05s. Two watched builds pass with3909unchanged CNA
inputs/no diagnostics; metadata remains474/0. Five actual routes raise C coverage198->203/3210 and
media.h15->20/39. No URI canonicalization, case/symlink equivalence, hash stability across processes
or playback identity claim. Full474 is pending. Evidence:checkpoint-474-c-song-identity-focused-*.

## Complete474-C-song-identity

Complete checkpoint **474-C-song-identity**, source `52b4163`, extends the same C media demo
61->69checks without another ID or fixture. Focused and full runs preserve all61old labels and add
eight exact controls for plain-path construction, stored path text, equality/hash semantics,
different-path inequality, empty-name storage and independent release. The helper Songs are never
played or queued. Focused29media/CLI CTests pass.

Full validation passes metadata474/0 and487enabled CTests in195.21s. SOFTWARE409/27/35/3 and
OPENGLES3439/29/3/3 preserve all38/42old failures and protected child/network evidence. C media
alone grows; parentJSON stays0.37binaries, four assets and30fixture paths remain exact. Normalized
stderr is identical. Outer Xvfb has8exact amdgpu pairs (previous12); exits are zero and CNA
eff3131017 remains clean. XNA coverage stays297/329types and1405/2326members; actual C ABI routes
rise198->203/3210 and media.h15->20/39. Evidence:checkpoint-474-c-song-identity-*.

## Focused474-C-song-metadata

The same `c_api.media.media_player` now grows69->71child checks without a new ID or fixture. Both
renderers pass identical lists with empty stderr; removing two additions restores all69prior labels
in order. The existing unplayed caller-owned plain-path Song reports unprotected/unrated with
rating0, track number0 and play count0, then stores and reads back play count42. Focused29media/CLI
CTests pass in8.52s. Two watched builds pass with3909unchanged CNA inputs/no diagnostics; metadata
remains474/0. Six actual routes raise C coverage203->209/3210 and media.h20->26/39. No tagged-song
mutation, persistence or automatic playback-count claim; trivial type-name routes do not justify
separate controls. Full474 is pending. Evidence:checkpoint-474-c-song-metadata-focused-*.

## Complete474-C-song-metadata

Complete checkpoint **474-C-song-metadata**, source `85260eb`, extends the same C media demo
69->71checks without another ID or fixture. Focused and full runs preserve all69old labels and add
two exact controls for unprotected/unrated plain-path defaults, zero rating/track/play count and a
mutable play-count round trip. The helper Song remains unplayed. Focused29media/CLI CTests pass.

Full validation passes metadata474/0 and487enabled CTests in172.79s. SOFTWARE409/27/35/3 and
OPENGLES3439/29/3/3 preserve all38/42old failures and protected child/network evidence. C media
alone grows; parentJSON stays0.37binaries, four assets and30fixture paths remain exact. Normalized
stderr is identical. Outer Xvfb has17exact amdgpu pairs (previous8); exits are zero and CNA
eff3131017 remains clean. XNA coverage stays297/329types and1405/2326members; actual C ABI routes
rise203->209/3210 and media.h20->26/39. Evidence:checkpoint-474-c-song-metadata-*.

## Focused474-C-player-settings

The same `c_api.media.media_player` now grows71->75child checks without a new ID or fixture. Both
renderers pass identical lists with empty stderr; removing four additions restores all71prior labels
in order. After playback is stopped, mute/repeat/shuffle each round-trip true then restored false;
volume clamps -0.25to0 and1.25to1 before restoring1. Focused29media/CLI CTests pass in8.56s. Two
watched builds pass with3909unchanged CNA inputs/no diagnostics; metadata remains474/0. Eight
actual routes raise C coverage209->217/3210 and media_player.h23->31/41. No audible-volume,
queue-order, repeated-playback or persistence claim. Full474 is pending.
Evidence:checkpoint-474-c-player-settings-focused-*.

## Complete474-C-player-settings

Complete checkpoint **474-C-player-settings**, source `eac3fa7`, extends the same C media demo
71->75checks without another ID or fixture. Focused and full runs preserve all71old labels and add
four exact stopped-state controls for mute, repeat, shuffle and clamped volume, restoring ordinary
settings before teardown. Focused29media/CLI CTests pass.

Full validation passes metadata474/0 and487enabled CTests in169.00s. SOFTWARE409/27/35/3 and
OPENGLES3439/29/3/3 preserve all38/42old failures and protected child/network evidence. C media
alone grows; parentJSON stays0.37binaries, four assets and30fixture paths remain exact. Normalized
stderr is identical. Outer Xvfb has8exact amdgpu pairs (previous17); exits are zero and CNA
eff3131017 remains clean. XNA coverage stays297/329types and1405/2326members; actual C ABI routes
rise209->217/3210 and media_player.h23->31/41. Evidence:checkpoint-474-c-player-settings-*.

## Focused474-C-player-transport

The same `c_api.media.media_player` now grows75->78child checks without a new ID or fixture. Both
renderers pass identical lists with empty stderr; removing three additions restores all75prior
labels in order. With the live collection and stopped player, play_songs_from starts its second
Song at index1, move_previous returns to index0, and existing move_next plus Stop restore the
previous index1/stopped state before ownership checks. No queue-owned Song reaches Play, preserving
ISSUE-058. Focused29media/CLI CTests pass in8.58s. Two watched builds pass with3909unchanged CNA
inputs/no diagnostics; metadata remains474/0. Two actual routes raise C coverage217->219/3210 and
media_player.h31->33/41. No audible or immediate event-delivery claim. Full474 is pending.
Evidence:checkpoint-474-c-player-transport-focused-*.

## Complete474-C-player-transport

Complete checkpoint **474-C-player-transport**, source `91dba1a`, extends the same C media demo
75->78checks without another ID or fixture. Focused and full runs preserve all75old labels and add
three exact controls for a nonzero collection start index, previous navigation and stopped-state
restoration. No queue-owned Song reaches Play. Focused29media/CLI CTests pass.

Full validation passes metadata474/0 and487enabled CTests in150.82s. SOFTWARE409/27/35/3 and
OPENGLES3439/29/3/3 preserve all38/42old failures and protected child/network evidence. C media
alone grows; parentJSON stays0.37binaries, four assets and30fixture paths remain exact. Normalized
stderr is identical. Outer Xvfb has19exact amdgpu pairs (previous8); exits are zero and CNA
eff3131017 remains clean. XNA coverage stays297/329types and1405/2326members; actual C ABI routes
rise217->219/3210 and media_player.h31->33/41. Evidence:checkpoint-474-c-player-transport-*.

## Focused474-C-player-visualization

The same `c_api.media.media_player` now grows78->82child checks without a new ID or fixture. Both
renderers pass identical lists with empty stderr; removing four additions restores all78prior
labels in order. The C value initializes with version1 and two zeroed256-float arrays; capture
enables and reads back true; the existing live music produces nonzero frequency and sample data;
disabling reads false and zeros deliberately poisoned prior data. Focused29media/CLI CTests pass
in8.60s. Two watched builds pass with3909unchanged CNA inputs/no diagnostics; metadata remains
474/0. Four actual routes raise C coverage219->223/3210, media.h26->27/39 and
media_player.h33->36/41. No audible, FFT-accuracy, exact-amplitude, paused-data or rendered-pixel
claim. Full474 is pending. Evidence:checkpoint-474-c-player-visualization-focused-*.

## Complete474-C-player-visualization

Complete checkpoint **474-C-player-visualization**, source `7f0ce57`, extends the same C media demo
78->82checks without another ID or fixture. Focused and full runs preserve all78old labels and add
four exact controls for versioned zero initialization, enabled readback, live frequency/sample data
and disabled zero-buffer behavior. Focused29media/CLI CTests pass.

Full validation passes metadata474/0 and487enabled CTests in158.33s. SOFTWARE409/27/35/3 and
OPENGLES3439/29/3/3 preserve all38/42old failures and protected child/network evidence. C media
alone grows; parentJSON stays0.37binaries, four assets and30fixture paths remain exact. Normalized
stderr is identical. Outer Xvfb has14exact amdgpu pairs (previous19); exits are zero and CNA
eff3131017 remains clean. XNA coverage stays297/329types and1405/2326members; actual C ABI routes
rise219->223/3210, media.h26->27/39 and media_player.h33->36/41.
Evidence:checkpoint-474-c-player-visualization-*.

## Focused474-C-player-single-song

The same `c_api.media.media_player` now grows82->88child checks without a new ID or fixture. Both
renderers pass identical lists with empty stderr; removing six additions restores all82prior labels
in order. After the old queue is cleared, an independent caller-owned Song starts playback into one
active queue entry. Destroying the input leaves playback and the published queue copy live; Stop
leaves the copy readable, and copy/view release independently. No queue-owned Song reaches Play,
preserving ISSUE-058. Focused29media/CLI CTests pass in8.33s. Two watched builds pass with
3909unchanged CNA inputs/no diagnostics; metadata remains474/0. One actual route raises C coverage
223->224/3210 and media_player.h36->37/41. No audible or event-timing claim. Full474 is pending.
Evidence:checkpoint-474-c-player-single-song-focused-*.

## Complete474-C-player-single-song

Complete checkpoint **474-C-player-single-song**, source `a9aa6ce`, extends the same C media demo
82->88checks without another ID or fixture. Focused and full runs preserve all82old labels and add
six exact controls for caller-owned Play, one active queue copy, input destruction during playback,
Stop/readability and independent release. No queue-owned Song reaches Play. Focused29media/CLI
CTests pass.

Full validation passes metadata474/0 and487enabled CTests in152.63s. SOFTWARE409/27/35/3 and
OPENGLES3439/29/3/3 preserve all38/42old failures and protected child/network evidence. C media
alone grows; parentJSON stays0.37binaries, four assets and30fixture paths remain exact. Normalized
stderr is identical. Outer Xvfb has16exact amdgpu pairs (previous14); exits are zero and CNA
eff3131017 remains clean. XNA coverage stays297/329types and1405/2326members; actual C ABI routes
rise223->224/3210 and media_player.h36->37/41. Evidence:checkpoint-474-c-player-single-song-*.

## Focused474-C-player-program-exit

The same `c_api.media.media_player` now grows88->89child checks without a new ID or fixture. Both
renderers pass identical lists with empty stderr; removing the one addition restores all88prior
labels in order. During live playback, ProgramExit succeeds and the same independent caller-owned
Song immediately restarts into one active queue entry; the earlier published owned copy stays
readable. Source confirms the initialized media resources are released and the next Play
reinitializes them. No queue-owned Song reaches Play. Focused29media/CLI CTests pass in8.66s. Two
watched builds pass with3909unchanged CNA inputs/no diagnostics; metadata remains474/0. One actual
route raises C coverage224->225/3210 and media_player.h37->38/41. The remaining update route is
deferred to an observable host-driven playback pump; the two generated queue type-name routes are
intentionally trivial. No audible, continuity, event-timing or ProgramExit state-transition claim.
Full474 is pending. Evidence:checkpoint-474-c-player-program-exit-focused-*.

## Complete474-C-player-program-exit

Complete checkpoint **474-C-player-program-exit**, source `aebc766`, extends the same C media demo
88->89checks without another ID or fixture. Focused and full runs preserve all88old labels and add
one exact control for early ProgramExit and immediate Play reinitialization from an independent
caller-owned Song. No queue-owned Song reaches Play. Focused29media/CLI CTests pass.

Full validation passes metadata474/0 and487enabled CTests in157.51s. SOFTWARE409/27/35/3 and
OPENGLES3439/29/3/3 preserve all38/42old failures and protected child/network evidence. C media
alone grows; parentJSON stays0.37binaries, four assets and30fixture paths remain exact. Normalized
stderr is identical. Outer Xvfb has15exact amdgpu pairs (previous16); exits are zero and CNA
eff3131017 remains clean. XNA coverage stays297/329types and1405/2326members; actual C ABI routes
rise224->225/3210 and media_player.h37->38/41. Evidence:checkpoint-474-c-player-program-exit-*.

## Focused474-C-media-sources

The same `c_api.media.media_player` now grows89->92child checks without a new ID or fixture. Both
renderers pass identical lists with empty stderr; removing three additions restores all89prior
labels in order. Desktop enumeration reports exactly one `LOCAL_DEVICE` named `Local Device`; the
index equal to Count is refused before changing four preseeded outputs. Focused29media/CLI CTests
pass in11.23s. Two watched builds pass with3909unchanged CNA inputs/no diagnostics; metadata
remains474/0. Four actual routes raise C coverage225->229/3210 and media.h27->31/39. The eight
remaining media.h routes are generated fixed type-name size/copy pairs and are intentionally
trivial. This is CNA's synthetic desktop source, not hardware discovery, hot-plug or Windows Media
Connect evidence. Full474 is pending. Evidence:checkpoint-474-c-media-sources-focused-*.

## Complete474-C-media-sources

Complete checkpoint **474-C-media-sources**, source `48c9f0e`, extends the same C media demo
89->92checks without another ID or fixture. Focused and full runs preserve all89old labels and add
three exact controls for the one desktop Local Device, its type/name and upper-bound output
preservation. Focused29media/CLI CTests pass.

Full validation passes metadata474/0 and487enabled CTests in158.85s. SOFTWARE409/27/35/3 and
OPENGLES3439/29/3/3 preserve all38/42old failures and protected child/network evidence. C media
alone grows; parentJSON stays0.37binaries, four assets and30fixture paths remain exact. Normalized
stderr is identical. Outer Xvfb has15exact amdgpu pairs (previous15); exits are zero. Clean watched
builds and focused reruns used CNAaa3f4e0. The full post-build capture observed concurrent
CMake-only CNA edits at aa3f4e0; final checkout74194f4 is clean and all selected runtime artifacts
stayed fixed. This is immutable runtime evidence, not an atomic external-source snapshot. XNA
coverage stays297/329types and1405/2326members; actual C ABI routes rise225->229/3210 and
media.h27->31/39. Evidence:checkpoint-474-c-media-sources-*; the earlier drifted full attempt is
archived under checkpoint-474-c-media-sources-refused-cna-drift-*.

## Focused474-C-input-keyboard-state

The existing `c_api.input.keyboard_mouse` now grows12->15child checks without a new ID or fixture.
Both renderers pass identical lists with empty stderr; removing three additions restores all12prior
labels in order. A known two-key POD snapshot proves key-up/down complements, reports the exact
required count without partial writes to a short buffer, then copies Space and Left in ascending
numeric order to an exact buffer. All56input/CLI CTests pass, with six declared environment skips.
Two watched builds pass with3909unchanged CNA inputs/no diagnostics at clean74194f4; metadata
remains474/0. Two actual routes raise C coverage229->231/3210 and input.h8->10/15. This does not
claim live key transitions, keyboard-layout mapping or manual input validation. Full474 is pending.
Evidence:checkpoint-474-c-input-keyboard-state-focused-*.

## Complete474-C-input-keyboard-state

Complete checkpoint **474-C-input-keyboard-state**, source `a0ecf81`, extends the existing C
keyboard/mouse demo12->15checks without another ID or fixture. Focused and full runs preserve all12
old labels and add three exact controls for complementary key-up queries, atomic insufficient-buffer
reporting and ascending exact-buffer enumeration. All56input/CLI CTests pass with six declared
environment skips.

Full validation passes metadata474/0 and487enabled CTests in162.80s. SOFTWARE409/27/35/3 and
OPENGLES3439/29/3/3 preserve all38/42old failures and protected child/network evidence. C keyboard
alone grows; parentJSON stays0.37binaries, four assets and30fixture paths remain exact. Normalized
stderr is identical. Outer Xvfb has12exact amdgpu pairs (previous15); exits are zero. Focused builds
and the full post-build capture used clean CNA74194f4. External LINK-050 later changed only CMake
tests; final CNA f44a8ff is clean and all selected runtime artifacts stayed fixed. This is immutable
binary/runtime evidence rather than an atomic external-source snapshot. XNA coverage stays
297/329types and1405/2326members; actual C ABI routes rise229->231/3210 and input.h8->10/15.
Evidence:checkpoint-474-c-input-keyboard-state-*.

## Complete475-C-input-touch-state

`c_api.input.touch_state` adds one coherent deterministic standalone C program. Both renderers pass
the same30child checks with empty stderr. CNA's public fixture routes publish connected four-touch
capabilities and advance a finger through empty, Pressed, Moved and Released snapshots. ID lookup
distinguishes the exact finger from a canonical invalid miss; previous-location queries write the
invalid first predecessor and reconstruct the Pressed and Moved positions on later frames. The
process-wide fixture resets before game destruction. All57input/CLI CTests pass in95.60s with six
declared environment skips. Two watched builds pass with3910unchanged CNA inputs/no diagnostics at
cleanf44a8ff; metadata is475/0. Eight actual routes raise C coverage231->239/3210, input.h10->14/15
and input_touch.h0->4/47. The remaining explicit live gamepad dead-zone capture is manual-deferred
because disconnected modes are indistinguishable and the authored-value helper already shows the
algorithms. No physical touch, gesture, pressure or visual claim. Focused evidence:
checkpoint-475-c-input-touch-lifecycle-focused-*.

Full validation passes metadata475/0 and488enabled CTests in152.96s. SOFTWARE410/27/35/3 and
OPENGLES3440/29/3/3 preserve all38/42old failures, every old C/custom child list and protected
trace, five network controls,18music checks and8album-art checks. The new ID contributes exactly
the focused30 checks and parentJSON0. All38selected binaries, four assets and30fixture paths remain
hash-identical; normalized stderr matches complete474-C-input-keyboard-state. Outer Xvfb emits
8exact amdgpu pairs (previous12), and both wrapper exits are zero. Post-build and final CNA state is
cleanf44a8ff. This is selected immutable runtime evidence rather than an atomic dependency-source
snapshot. XNA coverage stays297/329types and1405/2326members; C coverage remains239/3210 after the
focused increase. Evidence:checkpoint-475-c-input-touch-state-*.

## Complete475-C-input-touch-values

The existing `c_api.input.touch_state` workflow grows30->60checks without another demo ID or
fixture. Both renderers produce the same output with empty stderr, and removing the30 additions
restores every complete lifecycle label in order. Two authored locations differing only in pressure
compare equal, hash equally and match Contains/IndexOf/Remove. A disconnected two-touch snapshot
reports advisory IsReadOnly=true while ordered Insert/RemoveAt/Add/Clear operations still work. A
short insertion-copy returns required count4 without touching its destination; exact capacity
inserts both entries and shifts the old tail. All57input/CLI CTests pass in95.90s with six declared
environment skips. Two watched builds pass with3910unchanged CNA inputs/no diagnostics at clean
CNAf44a8ff; metadata remains475/0. Fifteen actual routes raise C coverage239->254/3210 and
input_touch.h4->19/47. This is authored POD and fixed-capacity collection evidence, not physical
pressure sensing, concurrency or unbounded growth. Focused evidence:
checkpoint-475-c-input-touch-values-focused-*.

Full validation passes metadata475/0 and488enabled CTests in189.40s. SOFTWARE410/27/35/3 and
OPENGLES3440/29/3/3 preserve all38/42old failures and every unrelated C/custom child list,
protected trace, network control, music check and album-art check. Only the touch child list grows
30->60 while parentJSON remains0. The selected38 runtime paths are fixed through both smokes;
against complete475-C-input-touch-state only the metadata-bearing catalog binary and touch
standalone differ, while the other36 binaries, four assets and30fixtures remain exact. Normalized
stderr is identical. Outer Xvfb emits7exact amdgpu pairs (previous8), both exits are zero, and CNA
is cleanf44a8ff at post-build and final observation. XNA coverage stays297/329types and
1405/2326members; C coverage remains254/3210 after the focused increase. Evidence:
checkpoint-475-c-input-touch-values-*.

## Focused476-C-input-gesture-queue

`c_api.input.gesture_queue` adds one distinct deterministic C standalone. Both renderers pass the
same31 child checks with empty stderr. Reset proves the default None mask and empty queue;
Tap|FreeDrag round-trips exactly. Two explicit samples retain timestamps, both finger ids and all
four vectors. Disabling detection after enqueue preserves the pending entries, reads return exact
FIFO Tap then FreeDrag values, availability follows the queue, and an empty read returns
INVALID_STATE without changing its output. All58input/CLI CTests pass in123.46s with six declared
environment skips. Two watched builds pass with3910unchanged CNA inputs/no diagnostics at clean
CNAf44a8ff; metadata is476/0. Six actual routes raise C coverage254->260/3210 and
input_touch.h19->25/47. No physical gesture detection, recognizer timing/threshold, callback-thread
or renderer-pixel claim. Full476 is pending. Evidence:checkpoint-476-c-input-gesture-queue-focused-*.

## Complete476-C-input-gesture-queue

Complete checkpoint **476-C-input-gesture-queue**, source `972dc87`, adds the focused standalone
unchanged. Full validation passes metadata476/0 and489enabled CTests in152.64s. SOFTWARE
411/27/35/3 and OPENGLES3441/29/3/3 preserve all38/42old failures, every prior C/custom child list
and protected trace, five network controls,18music checks and8album-art checks. The new ID
contributes exactly its focused31 child controls and parentJSON0. All39selected runtime paths stay
fixed through both smoke suites. Against complete475-C-input-touch-values, the catalog binary is
the only changed old binary, the gesture executable is the only addition, and the other37 old
binaries, four assets and30fixtures remain exact. Normalized stderr is identical. Outer Xvfb emits
7exact amdgpu pairs (previous7), and both exits are zero. The post-build capture saw clean CNA
f44a8ff; unrelated concurrent CNA work dirtied that sibling during smoke while every selected
runtime artifact stayed fixed. This is immutable binary/runtime evidence for the clean captured
source, not validation of the later dirty external tree or an atomic dependency snapshot. XNA
coverage stays297/329types and1405/2326members; actual C ABI coverage is260/3210 and
input_touch.h25/47. No physical detector, recognizer timing/threshold, callback-thread or visual
claim. Evidence:checkpoint-476-c-input-gesture-queue-*.

## Focused476-C-input-touch-scaling

The same `c_api.input.gesture_queue` grows31->64checks without a new ID or fixture. Both renderers
pass identical lists with empty stderr; removing33 additions restores all31complete-476 labels in
order. Reset exposes width0, height0 and Default orientation. Normalized events before a published
size succeed as drops; a later FreeDrag on640x480 yields exact position(480,240), delta(320,120)
and finger23 while the touch snapshot stays empty. Portrait round-trips, an undefined state is
refused, release adds no disabled DragComplete, and reset clears all display properties. The exact
58input/C-API/CLI CTests pass in43.67s with six declared environment skips. Two watched builds pass
with3911unchanged CNA inputs/no diagnostics; external unrelated CNA work remains dirty at f44a8ff,
so no clean-source claim is made. Metadata stays476/0. Seven actual routes raise C coverage
260->267/3210 and input_touch.h25->32/47. ISSUE-060 records the header's contradictory claim that
the gesture-only raise route also fills the event-driven touch map. No physical touch,
orientation-driven rotation, threshold boundary, snapshot injection or visual claim. Full476 is
pending. Evidence:checkpoint-476-c-input-touch-scaling-focused-*.

## Complete476-C-input-touch-scaling

Complete checkpoint **476-C-input-touch-scaling**, source `87cc277`, extends the same gesture demo
31->64checks without another ID or fixture. Full validation passes metadata476/0 and489enabled
CTests in161.14s. SOFTWARE411/27/35/3 and OPENGLES3441/29/3/3 preserve all38/42old failures,
every unrelated C/custom child list and protected trace, five network controls,18music checks and
8album-art checks. Only the gesture child grows to the exact focused64 controls; parentJSON stays0.
All39selected paths match the focused preflight and stay fixed through both smoke suites. Against
complete476-C-input-gesture-queue, the catalog and gesture standalone carry this batch; two CNA
libraries already carried unrelated dependency drift, while the other35 binaries, four assets and
30fixtures remain exact. Normalized stderr is identical. Outer Xvfb emits6exact amdgpu pairs
(previous7), and both exits are zero. Focused preflight saw the external bytes as dirty CNAf44a8ff;
the full post-build capture and final observation see their clean commit bb6820f with identical
runtime hashes. This is immutable selected-runtime evidence rather than an atomic dependency-source
snapshot. XNA coverage stays297/329types and1405/2326members; actual C ABI coverage is267/3210 and
input_touch.h32/47. ISSUE-060 remains a passing documentation mismatch; no physical touch,
orientation-driven rotation, threshold boundary, snapshot injection or visual claim. Evidence:
checkpoint-476-c-input-touch-scaling-*.

## Focused476-C-input-mouse-cursor

The existing `c_api.input.keyboard_mouse` grows15->32checks without a new ID or fixture. Both
renderers pass the same child-label list with empty stderr, and removing17 additions restores every
complete476 label in order. Six value controls cover default/ordinary/extended construction,
equality, the position/vertical-wheel-only hash and field-bearing text. Both renderers round-trip
their window token, observe SetPosition and an exact global warp before restoration, exercise
capture results, synchronously deliver/unsubscribe/reset clicked callbacks and pass owned/default,
borrowed-stock and copied-texture cursor lifetimes. OPENGLES3 exposes a nonzero window token,
toggles/restores relative mode and applies capture/release; SOFTWARE exposes token0, retains false
through its successful void relative setter, reports capture not applied and applies release.
Those outcomes are asserted rather than skipped. All58 exact input/C-API/CLI CTests pass in52.10s
with six declared environment skips. The initial watched build passes with3911unchanged CNA inputs
and no diagnostics at clean bb6820f; metadata is476/0. All20 `input_mouse.h` and all6
`input_cursor.h` routes now have coherent runtime evidence, raising actual C coverage267->293/3210.
ISSUE-061 records that the C default-cursor documentation says empty/no-op while the direct public
source path constructs Arrow. No physical mouse transition, confinement, cursor-shape visual,
other desktop or thread/reentrancy claim. Full476 is pending. Evidence:
checkpoint-476-c-input-mouse-cursor-focused-*.

## Complete476-C-input-mouse-cursor

Complete checkpoint **476-C-input-mouse-cursor**, source `76fe296`, extends the existing C
keyboard/mouse demo15->32checks without another ID or fixture. Full validation passes
metadata476/0 and all489enabled CTests in175.58s. SOFTWARE411/27/35/3 and
OPENGLES3441/29/3/3 preserve every parent outcome, the exact38/42old failures, all unrelated
C/custom child lists and protected traces, five network controls,18music checks and8album-art
checks. Only the keyboard/mouse child grows to the focused32 controls; parentJSON stays0. All39
selected paths match the focused preflight and remain fixed through both smokes. Against
complete476-C-input-touch-scaling, only the catalog and keyboard/mouse standalone change; the
other37 binaries, four assets and30fixtures remain exact. Normalized stderr is identical. Outer
Xvfb emits7exact amdgpu pairs (previous6), and both exits are zero. Post-build and final
observations see clean CNAbb6820f. XNA coverage stays297/329types and1405/2326members; actual C ABI
coverage is293/3210, `input_mouse.h`20/20 and `input_cursor.h`6/6. ISSUE-061 remains a passing
documentation mismatch. No physical mouse, cursor-shape visual or other-desktop claim. Evidence:
checkpoint-476-c-input-mouse-cursor-*.

## Focused476-C-input-keyboard

The existing `c_api.input.keyboard_mouse` grows32->40checks without a new ID or fixture. Both
renderers pass the same child-label list with empty stderr; filtering eight additions restores all
32 complete476 labels in order. Four value controls cover default initialization, the key-state
indexer, set equality/inequality and the documented A/NumPad1 32-slot XOR hash collision. Four live
controls prove player slots One and Four match the ordinary snapshot, modifiers contain only public
bits, physical A's copied scancode name round-trips, and the active layout's copied key name resolves
to the key reported for that scancode. The exact58 input/C-API/CLI CTests pass in51.33s with six
declared skips. Metadata is476/0. A stable watched build passes with3912 unchanged CNA inputs and no
diagnostics after two successful builds were refused for concurrent unrelated content-source drift;
the accepted CNA tree is dirty at bb6820f. Fourteen actual routes raise C coverage293->307/3210 and
`input_keyboard.h`1->15/17. Its two fixed type-name size/copy routes remain intentionally uncredited
as trivial reflection-buffer mechanics already covered by generated tests. No physical input,
alternate layout, lock toggle, scancode mode, other-desktop or visual claim. Full476 is pending.
Evidence:checkpoint-476-c-input-keyboard-focused-*.

## Complete476-C-input-keyboard

Complete checkpoint **476-C-input-keyboard**, source `920a614`, extends the existing C
keyboard/mouse child32->40checks without another ID or fixture. Full validation passes
metadata476/0 and all489enabled CTests in289.65s. SOFTWARE411/27/35/3 and
OPENGLES3441/29/3/3 preserve every parent outcome, the exact38/42 old failures, all unrelated
C/custom child lists and protected traces, five network controls,18music checks and8album-art
checks. Only the keyboard/mouse child grows to the exact focused40 controls; parentJSON remains0.
All39 selected paths match the focused preflight and remain fixed through both smoke suites.
Against complete476-C-input-mouse-cursor, the catalog and keyboard/mouse standalone carry this
batch;15 other selected binaries carry the separately identified external CNA content work, while
four assets and30fixtures remain exact. Normalized stderr is identical. Outer Xvfb emits6exact
amdgpu pairs, and both exits are zero. Post-build capture sees the external bytes dirty at bb6820f;
they become clean LINK-053 commit9edbf314 during smoke without changing any relevant keyboard
source hash or selected runtime artifact. Coverage stays297/329types and1405/2326XNA members;
actual C ABI coverage is307/3210 and `input_keyboard.h`15/17. The two omitted routes remain trivial
fixed type-name reflection. No physical input, alternate layout, lock toggle, scancode mode,
other-desktop or visual claim. Evidence:checkpoint-476-c-input-keyboard-*.

## Focused476-C-input-gamepad-values

The existing `c_api.input.gamepad` grows28->47checks without a new ID or fixture. Both renderers
pass identical child-label lists with empty stderr; filtering19 additions restores all28
complete476 labels in order. Three button controls cover empty/mask/array construction, combined
queries, equality and raw-mask hashing. Three D-pad controls prove non-directional array members are
ignored, comparisons use only directional bits and Up+Left hashes to canonical weight10. Seven
analog controls cover stick square clamping/exact equality and trigger0..1 clamping/epsilon
equality plus both hashes. Four state controls reproduce the existing snapshot from components,
round-trip its components, mutate packet7 and prove the canonical hash ignores an analog change
that equality observes. The capability default checks every feature flag; scalar dead-zone
exclusion checks interior zero and endpoint rescaling. All58 exact input/C-API/CLI CTests pass in
30.48s with six declared skips. Two watched builds pass with3912 unchanged CNA inputs/no
diagnostics at clean9edbf314; metadata is476/0. Thirty-four actual routes raise C coverage
307->341/3210 and `input_gamepad.h`4->38/63. Two fixed type-name routes remain trivial/uncredited;
23 live device routes remain for disconnected and hardware-success dispositions. Full476 is
pending. Evidence:checkpoint-476-c-input-gamepad-values-focused-*.

## Complete476-C-input-gamepad-values

Complete checkpoint **476-C-input-gamepad-values**, source `6ac3d08`, extends the existing C
gamepad child28->47checks without another ID or fixture. Full validation passes metadata476/0 and
all489enabled CTests in153.30s. SOFTWARE411/27/35/3 and OPENGLES3441/29/3/3 preserve every parent
outcome, the exact38/42 old failures, all unrelated C/custom child lists and protected traces, five
network controls,18music checks and8album-art checks. Only the C gamepad child grows to the exact
focused47 controls; parentJSON remains0. All39 selected paths match focused preflight and remain
fixed through both smoke suites. Against complete476-C-input-keyboard, only the catalog and C
gamepad executable change; four assets and30fixtures remain exact. Normalized stderr is identical.
Outer Xvfb emits6exact amdgpu pairs, and both exits are zero. Post-build capture sees clean
CNA9edbf314; unrelated DirectX11 renderer/documentation retirement work appears during smoke at
the same head without changing any relevant gamepad source hash or selected Linux runtime artifact.
Coverage stays297/329types and1405/2326XNA members; actual C ABI coverage is341/3210 and
`input_gamepad.h`38/63. No physical controller, device actuation/sensor, other-platform or new
visual claim. Evidence:checkpoint-476-c-input-gamepad-values-*.

## Focused476-C-input-gamepad-device

The same C gamepad demo grows47->55checks and credits all23 remaining live device routes in
`input_gamepad.h` as one disconnected-slot contract. It selects an empty slot from the four player
slots, then proves both vibration requests are refused while the void light-bar request succeeds;
motion sensors report unavailable and preserve their output; player index, power, face-label and
connection return their exact absent sentinels. GUID/name/path/serial share one zero-byte
size/copy check rather than eight getter-shaped checks. Firmware and Steam identities are zero;
touchpad counts are zero and an unavailable finger preserves all fields. Both renderer selections
pass the same55 labels with empty stderr, and removing the eight additions restores all47 previous
labels. The exact58 input/C-API/CLI CTests pass in33.63s with six declared skips. Metadata is476/0;
coverage rises341->364/3210 and `input_gamepad.h`38->61/63. The remaining two routes only expose a
fixed GamePadState type name. Physical actuation, sensor values, device identities and touch input
remain manual.

The ordinary watched build is temporarily blocked by unrelated concurrent sibling-CNA work:
dirty9edbf314 has incompatible in-progress Vulkan source/header reductions. Both changed demo
translation units compile cleanly, and the focused executables were manually linked against the
last built CNA libraries for the recorded runtime. This evidence is intentionally focused; the
complete476 checkpoint remains pending. Evidence:checkpoint-476-c-input-gamepad-device-focused-*.

## Focused477-C-input-devices

One new standalone C demo groups the host-device facilities that form a coherent runtime snapshot:
descriptor values, mouse/keyboard/touch/sensor enumeration, optional machine motion readings,
mouse/keyboard hot-plug events and power. Both renderer selections pass the same29 labels and
produce the same host values with empty stderr. Under Xvfb, three mice and three keyboards execute
the successful descriptor/name size/copy paths; touch and sensor counts are zero. Their index-at-
count boundaries are checked, but the six metadata success routes remain uncredited until suitable
hardware actually executes them. Both absent motion reads preserve their poisoned vectors.

Four raised events synchronously deliver exact ids to separate handlers, selective unsubscribe
leaves the keyboard handler attached, reset detaches everything, and the remaining owned handles
release safely afterwards. The power snapshot writes Charged,79percent and unknown seconds in this
environment. Direct compilation is diagnostic-free; metadata is477/0 and all59 exact input/C-API/
CLI CTests pass in39.49s with six declared skips. Twenty-seven routes establish
`input_devices.h`27/37 and raise total C coverage364->391/3210. The four clipboard routes remain a
separate state-restoration concern. The same unrelated dirty CNA Vulkan source/header mismatch
blocks a standard build and complete checkpoint; focused runtime is linked against the last built
CNA libraries. Evidence:checkpoint-477-c-input-devices-focused-*.

## Focused478-C-input-clipboard

One small standalone keeps desktop-global clipboard mutation separate from host-device
enumeration. Both renderer selections pass identical15-label children with empty stderr. The demo
captures the original text with a bounded retry around the public count/copy race, requests an
explicit multibyte UTF-8 value, checks has-text and the exact byte count/copy, and proves a
one-byte-short copy reports the full requirement without changing any of128 guard bytes. Empty
text clears has-text, reports zero and accepts a null zero-capacity copy. The captured bytes are
then restored and reread exactly.

Direct C and metadata compilation is diagnostic-free; metadata is478/0 and the expanded exact60
input/C-API/CLI CTests pass in41.81s with six declared skips. Four routes raise actual C coverage
391->395/3210 and `input_devices.h`27->31/37. The six remaining routes are successful sensor/touch
metadata enumeration paths; Xvfb reports zero devices, so these stay hardware-manual even though
their index-at-count refusals run. The unrelated in-progress CNA Vulkan mismatch still blocks the
standard build and complete478 checkpoint. Evidence:checkpoint-478-c-input-clipboard-focused-*.

## Focused479-C-input-text

One new C standalone keeps deterministic event delivery and the platform activation lifecycle in
one coherent text-input workflow. Both renderer selections pass the same20 labels with empty
stderr. Public raises synchronously deliver exact UTF-16 code units including a surrogate pair,
copy a multibyte UTF-8 composition while preserving deliberately out-of-range byte offsets, and
copy two candidate views plus orientation and an unchecked selected index. Selective unsubscribe
removes only committed text; reset removes every remaining handler and all registration handles
then release safely.

The demo captures the actual backend window token, forces zero to establish false activation and
screen-keyboard answers plus successful no-op start/placement/stop, and accepts all nine text-input
hints while refusing the first undefined identity. It restores the backend token for one real-path
start/placement/visibility/stop cycle without assuming that a keyboard is visible, then proves
reset clears the binding and restores the original token. Direct C and metadata compilation is
diagnostic-free; metadata is479/0 and all61 exact input/C-API/CLI CTests pass in31.40s with six
declared skips. All17 `input_text.h` routes gain semantic credit and total C coverage rises
395->412/3210. Physical typing and platform IME/keyboard presentation remain manual. The
unrelated in-progress CNA Vulkan work still prevents a standard build and complete479 checkpoint;
focused runtime uses the last built CNA libraries. Evidence:checkpoint-479-c-input-text-focused-*.

## Focused480-C-input-joystick

One C standalone separates the raw-joystick C contract from the visual/manual C++ state display.
Both renderer selections pass the same23 labels with empty stderr, and Xvfb enumerates zero raw
joysticks. Descriptor defaults/equality cover id, physical type and UTF-8 name. Capability defaults
represent a disconnected device with unknown charge, while equality observes hardware fields,
name and GUID. The absent-id live query returns that exact value and zero-byte name/GUID size/copy
answers without changing guard bytes.

Two absent ids create independent owned snapshots. All four axis/button/hat/trackball count and
copy pairs report empty arrays without changing destinations; the snapshots compare equal and
release independently. Public connected/disconnected raises deliver exact ids synchronously,
selective unsubscribe preserves the other handler, and reset removes it before its registration
releases safely. Direct C and metadata compilation is diagnostic-free; metadata is480/0 and all62
exact input/C-API/CLI CTests pass in30.94s with six declared skips. Twenty-seven routes raise C
coverage412->439/3210 and establish `input_joystick.h`27/30. The three successful enumerated
info/name routes remain uncredited until hardware executes them; physical axes, buttons, hats,
trackballs and feedback are likewise manual. The watched standard build sees3898 unchanged CNA
inputs at dirty d71308d3 but fails in unrelated `SdlGpuRenderer.cpp` definitions whose concurrently
reduced header removed compute/storage/indirect members. Focused runtime therefore uses the last
built libraries and is not a complete-build claim. Evidence:checkpoint-480-c-input-joystick-focused-*.

## Focused481-C-input-haptics

One C standalone separates deterministic haptic values and the safe closed-device contract from
hardware actuation. Both renderer selections pass identical24-label children with empty stderr;
Xvfb reports zero haptic devices. Direction equality observes even unused polar components. Effect
equality observes a field unused by the constant family and caller-owned custom samples.
Capabilities retain -1 for unknown closed-device limits, and equality observes feature bits and
the external UTF-8 name.

An absent id opens as an owned closed handle. Its name is empty without a destination write and
its capabilities are the canonical closed value. Effect support, all three rumble operations,
effect create/update/run/status/stop/destroy, global stop, gain, autocenter, pause and resume all
return their documented false/no-op outcomes. Dispose is idempotent and destroy releases the
handle. Mouse and absent-joystick query/open paths produce coherent owned results. Direct C and
metadata compilation is diagnostic-free; metadata is481/0 and all63 exact input/C-API/CLI CTests
pass in43.85s with six declared skips. Thirty-three routes raise C coverage439->472/3210 and
establish `input_haptics.h`33/36. The three successful enumerated id/name routes, effect upload and
physical force feedback remain hardware-manual. The unchanged external SDL_GPU source/header
mismatch still blocks the standard/full checkpoint, so focused runtime uses the last built CNA
libraries. Evidence:checkpoint-481-c-input-haptics-focused-*.

## Focused482-C-sensor-values

One deterministic C standalone separates pure sensor-reading values from acquisition and physical
hardware. Both renderer selections pass identical34-label children with empty stderr. Five
reading families preserve their versioned defaults and authored timestamp/data fields. Equal
values hash equally; acceleration, angular velocity, rotation matrices, true heading, nested
attitude and timestamps independently affect equality. Canonical strings expose only their
documented subsets, including fused motion's acceleration/gravity but not attitude, and a short
attitude buffer is refused without a write. Legacy accelerometer event values keep their distinct
double-component constructor order and timestamp semantics; calibration's empty payload is
represented by its exact type name.

Direct C and metadata compilation is diagnostic-free; metadata is482/0 and all64 exact
input/device/C-API/CLI CTests pass in44.98s with six declared skips. All50 value/event-description
routes establish `sensors.h`50/144 and current-inventory C coverage472->522/3205. Concurrent
sibling-CNA graphics retirement reduced the total declaration count by five; it did not remove any
demonstrated route. No game, live acquisition or physical measurement is claimed. The unchanged
SDL_GPU source/header timestamp still blocks the standard/full checkpoint, so focused runtime uses
the last built CNA libraries. Evidence:checkpoint-482-c-sensor-values-focused-*.

## Focused483-C-sensor-lifecycle

A second C standalone keeps device acquisition and event ownership separate from the pure reading
values. It reports the real availability of Accelerometer, Gyroscope, Compass and Motion, then uses
the public supported/started switches or supplied C test backends to execute the otherwise
unreachable supported paths without hardware. Synthetic acceleration is converted from metres per
second squared to g and reaches CurrentValueChanged before the legacy ReadingChanged callback;
selective unsubscribe leaves the current event attached. Gyroscope preserves radians/second.
Authored Compass and fused-Motion values become valid current readings, and their context-only
calibration events stop after individual unsubscribe. Each family round-trips its interval,
observes Stop/backend retention and releases its registration, disposed object and handle.

Both renderer selections produce identical40-label children with empty stderr:38 pass and exactly
two fail. CNA-DEMOS-ISSUE-062 records that disposed Accelerometer and Gyroscope handles still return
SUCCESS plus cached CurrentValue even though `sensors.h` promises INVALID_STATE; source inspection
finds the shared getter checks support but not disposal. Compass/Motion refuse in this workflow
after backend removal. Metadata validates483/0 and all65 exact input/device/C-API/CLI CTests pass
in45.45s with six declared skips and one known-issue expected failure. Sixty-six routes raise
current C coverage522->588/3205 and `sensors.h`50->116/144. The remaining eight fixed handle
type-name helpers and twenty low-level Accelerometer/Gyroscope dispatcher/failure-injection test
diagnostics are intentionally uncredited: their generated buffer mechanics or CNA-internal test
plumbing do not justify app-facing demo controls. No physical measurement or callback-thread claim
is made. The standard/full checkpoint remains blocked by the unchanged external SDL_GPU mismatch.
Evidence:checkpoint-483-c-sensor-lifecycle-focused-*.

## Focused484-C-host-info

A new read-only C standalone starts the `devices.h` audit with the fourteen routes that describe
the executing environment. It distinguishes Device/Emulator identity and extension availability,
reads a coherent power state, battery percentage and remaining-time sentinel, requires positive
logical CPU and RAM capacities, and enumerates every preferred locale through exact language and
country size/copy pairs. The upper-bound locale index is refused by all four routes without
touching a guarded destination.

Both renderer selections pass the same12 controls with empty stderr. Their only textual difference
is meaningful host state: SOFTWARE has no window and reports scale0 with a0x0 safe area, while
OPENGLES3 reports scale1 and800x480 under Xvfb. Direct C and metadata compilation is
diagnostic-free and metadata validates484/0. Fourteen routes establish `devices.h`14/62 and raise
current C coverage588->602/3205. All66 exact input/device/C-API/CLI CTests pass in182.31s with six
declared skips and one known-issue expected failure. No power transition, locale mutation, drawing,
screenshot or visual inspection is claimed. The standard/full checkpoint remains blocked by the
unchanged external SDL_GPU mismatch.
Evidence:checkpoint-484-c-host-info-focused-*.

## Focused485-C-vibration

A new C standalone completes the nine vibration routes in `devices.h`. It reads the real supported
flag and exact device-name bytes but does not send a request to that platform backend. CNA's public
in-memory test backend then makes delivery observable: a versioned log records full-intensity and
explicit-intensity starts, independent large/small motors, stops and exact durations. The demo
checks the inclusive zero/five-second duration limits, refusal immediately outside them without a
log change, below/above-range clamps and NaN-to-zero normalization. Exact name copying and
short-buffer preservation, idle Stop, backend replacement with a reset log and final platform
restoration are included.

Both renderer selections pass identical18-control output with empty stderr. Direct C and metadata
compilation is diagnostic-free and metadata validates485/0. The nine routes raise current C
coverage602->611/3205 and `devices.h`14->23/62. All67 exact input/device/C-API/CLI CTests pass in
115.45s with six declared skips and one known-issue expected failure. No physical actuation,
haptic sensation, request timing or platform motor success is claimed. The standard/full
checkpoint remains blocked by the unchanged external SDL_GPU mismatch.
Evidence:checkpoint-485-c-vibration-focused-*.

## Focused486-C-dialogs

A new C standalone completes all ten message-box and file-dialog routes in `devices.h`. It reads
the two real availability flags without displaying UI, then installs CNA's public immediate dialog
services. Message boxes demonstrate versioned request-log reset, dismiss-only severity, two-button
choice, closure without a choice, request validation before dispatch and platform restoration.
File dialogs demonstrate a versioned filter, two-path open selection, one-path save and folder
answers, copying callback-borrowed strings during delivery, canonical empty/null cancellation,
invalid callbacks/filter arrays/filter versions and platform restoration.

Both renderer selections pass identical20-control output with empty stderr. Direct C and metadata
compilation is diagnostic-free and metadata validates486/0. The ten routes raise current C
coverage611->621/3205 and `devices.h`23->33/62. All68 exact input/device/C-API/CLI CTests pass in
117.99s with six declared skips and one known-issue expected failure. No real window, manual
response, external filesystem access or callback-thread behavior is claimed. The standard/full
checkpoint remains blocked by the unchanged external SDL_GPU mismatch.
Evidence:checkpoint-486-c-dialogs-focused-*.

## Focused487-C-system-tray

A new C standalone covers eleven of the twelve system-tray routes in `devices.h` without creating
external host UI. It reads real availability, then creates CNA's per-handle in-memory tray and
demonstrates owned lifetime, tooltip mutation, contiguous menu indices, initial and mutable
checked/enabled state, label mutation, canonical ignored upper-bound mutations/false reads, exact
callback context selection, silent no-handler activation, invalid click bounds and stale handles.

Both renderer selections pass identical16-control output with empty stderr. Direct C and metadata
compilation is diagnostic-free and metadata validates487/0. Eleven routes raise current C
coverage621->632/3205 and `devices.h`33->44/62. All69 exact input/device/C-API/CLI CTests pass in
198.53s with six declared skips and one known-issue expected failure. The real
`cna_system_tray_create` route remains manual because successful execution creates a host icon; no
icon appearance, native click delivery or callback thread is claimed. The standard/full checkpoint
remains blocked by the unchanged external SDL_GPU mismatch.
Evidence:checkpoint-487-c-system-tray-focused-*.

## Focused488-C-camera-frames

A new C standalone covers fourteen of fifteen camera routes in `devices.h` without opening camera
hardware. It initializes the descriptor, reads driver support/count, enumerates every current
facing/name with exact buffers and validates the upper bound. CNA's synthetic camera then
round-trips permission-pending, denied and lost states, refuses unreachable/undefined states and
invalid frames, publishes exact2x2 RGBA data and copies every channel into a caller-owned Color
texture. Wrong-size and absent frames return ordinary false; clearing restores zero-size Opening;
the acquired texture remains readable after the camera handle is released.

Both renderer selections pass identical23-control output with empty stderr. Direct C and metadata
compilation is diagnostic-free and metadata validates488/0. Fourteen routes raise current C
coverage632->646/3205 and `devices.h`44->58/62. All70 exact input/device/C-API/CLI CTests pass in
221.19s with six declared skips and one known-issue expected failure. ISSUE-063 records that the
public C comments still promise Closed after synthetic creation/clear even though implementation,
CNA smoke and this runtime use Opening and refuse Closed. Real `cna_camera_create` remains manual;
no permission prompt, capture, preview, display or visual inspection is claimed. The standard/full
checkpoint remains blocked by the unchanged
external SDL_GPU mismatch. Evidence:checkpoint-488-c-camera-frames-focused-*.

## Focused488-C-clipboard-acceptance

The existing `c_api.input.clipboard` now uses the device extension for its explicit multibyte probe
write and requires the returned platform-acceptance flag before the unchanged exact read-back. Its
other14 controls retain size/copy/has-text, atomic short-buffer, empty-text and captured-value
restoration behavior; the later writes still exercise the plain clipboard setter.

Both renderer selections pass identical15-control output with empty stderr, metadata remains488/0
and the exact focused CTest passes. The one added route raises current C coverage646->647/3205 and
`devices.h`58->59/62 without adding a demo. Another process can still race the shared clipboard.
The three remaining header routes are deliberately manual: a real URL launch hands control to
another application, real tray creation publishes host UI, and real camera creation may activate
hardware or trigger permission. Their trivial refusal paths do not justify coverage credit, while
the public test paths already demonstrate CNA-owned behavior. The standard/full checkpoint remains
blocked by the unchanged external SDL_GPU mismatch.
Evidence:checkpoint-488-c-clipboard-acceptance-focused-*.

## Focused489-C-display-values

New `c_api.graphics.display_values` covers the five renderer-independent value routes that begin
`display.h`. Eight controls demonstrate complete versioned DisplayMode initialization, computed
16:9 and zero-height aspect ratios, equality over width/height/format rather than the cached ratio,
the complete PresentationParameters defaults, native cloning, derived zero-origin bounds and
refusal before output mutation for an invalid reserved byte. The clone deliberately follows CNA's
native XNA implementation: every XNA field survives while the CNA-only `headless_ext` returns to
false. Source, C API documentation and runtime agree, so this is not recorded as a CNA issue.

Both renderer selections pass identical output with empty stderr. Direct C and metadata compilation
is diagnostic-free, metadata validates489/0 and the exact focused CTest passes. A57-test C/API,
graphics-adapter, graphics-device-manager and CLI regression selection passes all56 enabled tests;
the declared manual fullscreen test is disabled. Five routes raise current C coverage647->652/3205
and establish `display.h`5/22. No game, device, window, adapter query, rendering or visual inspection
is claimed. The standard/full checkpoint remains blocked by the external sibling-CNA renderer
source/header mismatch. Evidence:checkpoint-489-c-display-values-focused-*.

## Focused490-C-adapter-snapshot

New `c_api.graphics.adapter_snapshot` covers all seven read-only adapter routes in `display.h`
through the active game's own handle before its first frame. Fifteen controls enumerate the one
Xvfb adapter; validate the versioned default-adapter metadata and nonempty names; copy exact
unterminated description and device-name bytes; prove both short buffers are unchanged; read a
coherent current mode; count/copy complete and Color-filtered mode arrays with guard preservation;
and refuse the index equal to adapter count through all six index-bearing routes before output
mutation. This host reports `screen`, `\\.\DISPLAY1` and one1280x1024 Color mode.

Both renderer selections pass identical output with empty stderr. Direct C and metadata compilation
is diagnostic-free, metadata validates490/0 and the exact focused CTest passes. A58-test C/API,
graphics-adapter, graphics-device-manager and CLI regression selection passes all57 enabled tests;
the declared manual fullscreen test is disabled. Seven routes raise current C coverage652->659/3205
and `display.h`5->12/22. Host values are observations; no rendering, alternative monitor topology
or visual inspection is claimed. Evidence:checkpoint-490-c-adapter-snapshot-focused-*.

## Complete490-C-adapter-snapshot

The sixteen focused C batches after complete476 now have a complete standard checkpoint. Current
CNA `440af2a` removed two capability enum values which cna-demos still named in unused framework
requirements and in the capability listing; aligning those lists with CNA's17 public capabilities
restored two clean standard builds. Full validation reports490/0 metadata and503/503 enabled CTests
in224.48s, with three declared tests disabled. SOFTWARE is424/28/35/3 and OPENGLES3 is454/30/3/3;
all failures are expected known issues. Against complete476, all prior statuses and known-issue
metadata remain exact. Fourteen new C IDs comprise13passes and the documented sensor-lifecycle
ISSUE-062 failure; only its two disposed-value assertions extend the failure multisets from38to40
and42to44. Coverage is297/329 types,1405/2326 XNA members and659/3205 C routes. No visual
inspection is claimed. Evidence:checkpoint-490-c-adapter-snapshot-*.

## Focused491-C-adapter-capabilities

New `c_api.graphics.adapter_capabilities` covers the six remaining adapter capability, format,
preference and explicit-limitation routes in `display.h`. Sixteen controls establish Reach support;
exact Color/None/0 render-target and backbuffer selections with complete versioned outputs; a
one-flag preference change followed by exact two-flag restoration before device creation; and the
stable ABI's zero-output NOT_SUPPORTED monitor-handle and callback-scoped refresh contracts. Both
renderer selections pass identical output with empty stderr. Metadata validates491/0, the exact
CTest passes, and all58 enabled tests in an Xvfb-backed59-test C/API, graphics-adapter,
graphics-device-manager and CLI selection pass with the declared fullscreen test disabled. A
deliberate no-display diagnostic run fails only the existing desktop clipboard and mouse/cursor
checks, while the Xvfb rerun passes them; this is an environment requirement rather than a new CNA
issue. Six routes raise C coverage659->665/3205 and `display.h`12->18/22. No monitor handle, cache
refresh, rendering or visual inspection is claimed. Evidence:
checkpoint-491-c-adapter-capabilities-focused-*.

## Focused492-C-device-presentation

New `c_api.graphics.device_presentation` completes the last four `display.h` routes through one
callback-scoped graphics-device loan. Twelve controls read and validate every public field of the
applied presentation parameters, change the presentation interval and observe it with every other
field preserved, restore and re-read the exact original value, obtain a coherent device display
mode, and establish that the device-level native-window route returns NOT_SUPPORTED with zero
output. SOFTWARE and OPENGLES3 pass the same checks with empty stderr; their device-mode
observations are respectively800x480 and1280x1024. Metadata validates492/0, the exact CTest passes,
and all59 enabled tests in the Xvfb-backed60-test C/API, graphics-adapter,
graphics-device-manager and CLI selection pass with the declared fullscreen test disabled. Four
routes raise C coverage665->669/3205 and `display.h`18->22/22. No rendering, pixels, native handle
or visual inspection is claimed. Evidence:checkpoint-492-c-device-presentation-focused-*.

## Complete492-C-device-presentation

The two focused `display.h` batches now have a complete standard checkpoint. Full validation
reports492/0 metadata and505/505 enabled CTests in258.17s, with three declared tests disabled.
SOFTWARE is426/28/35/3 and OPENGLES3 is456/30/3/3; all failures are expected known issues.
Comparison with complete490 finds only the two new passing display IDs, while every older status,
known-issue marker and failed-assertion message/multiplicity remains exact at40/44. Coverage is
297/329 types,1405/2326 XNA members and669/3205 C routes, with `display.h` complete at22/22. The
sibling CNA source remains at `440af2a`; its unrelated uncommitted handoff documentation was not
modified by cna-demos. No visual inspection is claimed. Evidence:
checkpoint-492-c-device-presentation-*.

## Focused493-C-render-target-bindings

New `c_api.graphics.render_target_bindings` completes the nine previously uncovered
`render_target.h` routes. Twenty-two controls classify Discard/Preserve/Platform usage; create
matching bindable2D and cube targets; read their complete applied state; bind and copy a real
two-entry2D/cube MRT array in order; prove atomic short-buffer refusal; replace it with one selected
cube face; and restore/copy the empty backbuffer state. ContentLost registration is valid, remains
silent during ordinary binding on both current non-device-losing renderers, releases once and is
then invalid. Both renderer selections pass identical output with empty stderr. Metadata is493/0,
the exact CTest passes, and an Xvfb-backed68-test C/API, render-target, device-manager and CLI
selection has no failures:66pass, the SOFTWARE custom-effect MRT scene declares a skip, and the
fullscreen test remains disabled. Nine routes raise C coverage669->678/3205 and complete
`render_target.h` at12/12. No device-loss notification, pixels or visual inspection is claimed.
Evidence:checkpoint-493-c-render-target-bindings-focused-*.

## Focused494-C-device-capabilities

New `c_api.graphics.device_capabilities` demonstrates the ten remaining meaningful graphics-device
introspection routes in `graphics.h`. Twenty controls copy renderer identity through the versioned
summary and atomic UTF-8 buffer pair; prove all17 individual legacy capability answers equal their
summary bits; enumerate all live detailed features and limits while refusing the retired gaps;
validate the known/supported masks of all27 surface formats; atomically copy the complete English
capability report; and relate the renderer-specific shader dialect and logical backbuffer to the
applied presentation state. SOFTWARE reports15 supported detailed features and dialect Unknown;
OPENGLES3 reports20 and GLSL ES. Both pass with empty stderr. Metadata is494/0, the exact CTest
passes, and an Xvfb-backed68-test C/API, graphics-device, device-manager and CLI selection passes
all67 active tests with fullscreen disabled. Ten routes raise C coverage678->688/3205 and
`graphics.h`14->24/27. Its remaining meaningful route is the scaled SpriteBatch array submission;
the two fixed type-name helpers are trivial/uncredited. The externally advanced clean CNA source
was `3b5755944`; no pixels or visual inspection is claimed.
Evidence:checkpoint-494-c-device-capabilities-focused-*.

## Focused494-C-sprite-scaled

The existing `c_api.graphics.sprite_batch` workflow grows from30to43 controls without a new demo
ID. A rejected two-command scaled array proves whole-array preflight before its valid first command
can draw. The accepted array selects separate cyan and magenta source texels, retains their texture
until End and then releases it while emitted pixels remain. Exact backbuffer points establish that
the cyan command's one-texel origin is multiplied by its12x8 nonuniform scale before offsetting its
position, while the magenta command begins at its own position and spans6x14. Both renderers produce
identical43-control output with empty stderr. Metadata remains494/0, the exact CTest and all73 tests
in the C/API, SpriteBatch and CLI regression selection pass, and the clean follow-up build passes.
One route raises C coverage688->689/3205 and `graphics.h`24->25/27. The only two uncredited routes
are fixed SpriteBatch type-name helpers, so every route in this header now has a semantic
disposition. No screenshot or visual inspection is claimed.
Evidence:checkpoint-494-c-sprite-scaled-focused-*.

## Focused495-C-resource-lifecycle

New `c_api.graphics.resource_lifecycle` completes all12 routes in `graphics_resource.h` through one
real device-owned Texture2D. Twenty-five controls recover its callback-scoped owner; distinguish an
empty Name from the unnamed `Texture2D` string; verify atomic short and exact copies; round-trip a
non-ASCII UTF-8 name and opaque64-bit tag; and refuse an embedded NUL without replacing the name.
An owned Disposing registration receives exactly one synchronous callback while the resource still
reports live. The retained C handle then sees the disposed state, repeated disposal is a successful
no-op, subscription release is one-shot and typed Texture2D destruction finally invalidates the
handle. Both renderers produce identical output with empty stderr. Metadata is495/0, the exact
CTest and all56 focused C/API, GraphicsResource and CLI tests pass, and the clean follow-up build
passes. Twelve routes raise C coverage689->701/3205. Sibling CNA advanced externally to clean
test-only `b45024c79` during validation while the production library hash remained unchanged. No
rendering or visual inspection is claimed.
Evidence:checkpoint-495-c-resource-lifecycle-focused-*.

## Focused496-C-index-buffer

New `c_api.graphics.index_buffer` demonstrates the five remaining meaningful routes in
`index_resources.h`. Eighteen controls create a dynamic four-entry16-bit buffer, validate every
field of its applied state, upload from a nonzero caller-array window, prove atomic short-buffer
read-back and copy into an exact shifted destination. A second caller window patches two entries at
byte offset four; complete read-back proves the two untouched indices survive. The owned
ContentLost registration stays silent during ordinary transfers on SOFTWARE and OPENGLES3,
releases once and refuses the stale handle. Both selections produce identical output with empty
stderr. Metadata is496/0, all59 focused C/API, C++ IndexBuffer and CLI tests pass in documented
isolated X11, and a clean follow-up build passes. Five routes raise C coverage701->706/3205 and
`index_resources.h`3->8/10. Its fixed type-name size/copy pair remains trivial/uncredited, giving
all ten routes a semantic disposition. An inherited-Wayland diagnostic also exposed and led to a
separate fix for the adapter demo's strict XNA `>1.6` widescreen boundary; its original mouse-warp
check passes unchanged under the required X11 isolation. Sibling CNA later advanced to clean
docs-only `e67c54936` without changing the exercised production library. No drawing, device-loss
notification or visual inspection is claimed.
Evidence:checkpoint-496-c-index-buffer-focused-*.

## Focused497-C-device-states

New `c_api.graphics.device_states` demonstrates the ten uncovered preset and live-device routes in
`graphics_state.h`. Nineteen controls validate every field of all5 blend,3 depth/stencil,4
rasterizer and7 sampler presets. The callback saves the active blend, depth, rasterizer and
pixel/vertex sampler-slot3 values; applies distinctive complete descriptors; reads each back field
for field; and proves the two sampler collections stay independent. It then attempts all five
restore operations independently and re-reads the exact original values before returning. SOFTWARE
and OPENGLES3 produce identical output with empty stderr. Metadata is497/0, all74 focused C/API,
C++ graphics-state and CLI tests pass, and a clean follow-up build passes. Ten routes raise C
coverage706->716/3205 and `graphics_state.h`2->12/14. The remaining state/effect SpriteBatch Begin
routes are assigned to the existing pixel-validated C SpriteBatch demo. Clean sibling CNA is
docs-only `e67c54936`; the production library hash remains unchanged. No drawing or visual
inspection is claimed.
Evidence:checkpoint-497-c-device-states-focused-*.

## Focused497-C-sprite-states

The existing `c_api.graphics.sprite_batch` workflow grows from43to61 controls without another demo
ID. Its explicit-state Begin uses Opaque, PointClamp, depth/stencil None and CullNone; exact
backbuffer read-back proves the half-alpha red source replaces the clear colour as raw RGB128. Its
effect-bearing Begin selects the default effect with `CNA_INVALID_HANDLE` and supplies a
nonidentity translation; the original destination stays clear and the opaque green command appears
exactly sixty pixels to the right. Both intervals share one owned texture and batch and retain
successful End/release checks. SOFTWARE and OPENGLES3 produce identical output with empty stderr.
Metadata is497/0, all76 focused C/API, C++ SpriteBatch and CLI tests pass, and a clean follow-up
build passes. These final two routes raise C coverage716->718/3205 and complete
`graphics_state.h` at14/14. Clean sibling CNA is docs-only `e67c54936`; production `libcna.so`
remains unchanged. No screenshot or visual inspection is claimed.
Evidence:checkpoint-497-c-sprite-states-focused-*.

## Complete497-C-sprite-states

Complete checkpoint source `cfed648` absorbs all seven focused graphics batches after complete492.
`tools/validate.sh` reports metadata497/0 and passes all510 enabled CTests in308.90s; three declared
tests remain disabled. SOFTWARE is431pass/28known-fail/35skip/3not-run and OPENGLES3 is
461pass/30known-fail/3skip/3not-run. Comparison with complete492 shows all five new IDs passing,
every existing status and known-issue value unchanged, and identical failed-assertion multisets of
40 and44. The existing C SpriteBatch workflow alone grows43to61 passing child checks. Fresh runtime
coverage is297/329 types,1405/2326 XNA members and718/3205 C routes. CNA advanced during the run
from clean `e67c54936` to clean `d7eff66ee` through two documentation-only commits; both production
library hashes stayed unchanged. No visual inspection is claimed.
Evidence:checkpoint-497-c-sprite-states-*.

## Focused498-C-viewport

New `c_api.graphics.viewport` demonstrates all11 Viewport value routes plus the live device get/set
pair in `graphics_device.h`. Nineteen controls distinguish the all-zero XNA value default from the
parameterized constructors'0..1 depth range; derive aspect, bounds and desktop safe area; preserve
depth while replacing bounds; round-trip identity and perspective projections; and prove exact,
atomic canonical text copies. The callback saves a complete live viewport, applies and reads back
a distinctive finite value, refuses infinity without mutation, then restores and verifies every
original field. SOFTWARE and OPENGLES3 produce identical output with empty stderr. Metadata is
498/0, all61 focused C/API, C++ Viewport, split-screen and CLI tests pass in35.43s, and a clean
follow-up build passes. Thirteen routes establish `graphics_device.h`16/84; the newly named matrix
identity route raises total C coverage718->732/3205. ISSUE-064 records the public C default-value
comment that incorrectly promises MaxDepth1 although XNA, CNA, its pure-C test and both demo runs
produce zero. Clean sibling CNA is `d7eff66ee`; the production library hash remains unchanged.
Nothing is drawn and no visual inspection is claimed.
Evidence:checkpoint-498-c-viewport-focused-*.

## Focused498-C-device-values

The existing `c_api.graphics.device_states` workflow grows from19to22 controls without another
demo ID. Its callback proves the game-owned device is undisposed, Normal, HiDef and tied to a
currently enumerated adapter; a direct disposal request returns NOT_SUPPORTED and leaves it live.
The existing complete blend, depth/stencil, rasterizer and independent sampler round trips now save,
apply and exactly read a distinctive scissor rectangle, blend factor, multisample mask and
reference stencil too. All original object and scalar/geometric values are restored independently,
then one final aggregate read verifies every field before callback return. SOFTWARE and OPENGLES3
produce identical output with empty stderr. Metadata remains498/0, all83 focused C/API, C++
graphics-device/state and CLI tests pass in49.86s, and a clean follow-up build passes. Thirteen
routes raise C coverage732->745/3205 and `graphics_device.h`16->29/84. Clean sibling CNA is
`d7eff66ee`; production `libcna.so` remains byte-identical. Nothing is drawn and no visual
inspection is claimed.
Evidence:checkpoint-498-c-device-values-focused-*.

## Focused498-C-resource-events

The existing `c_api.graphics.resource_lifecycle` workflow grows from25to30 controls without
another demo ID. Before creating its real Texture2D, the callback records the tracked-resource
baseline and subscribes to the graphics device's resource-created and resource-destroyed events.
Creation synchronously reports a complete versioned presence payload after tracking grows by one.
Explicit disposal first raises the resource's Disposing callback and then reports the exact
non-ASCII UTF-8 name while the resource is still tracked; native tag presence remains false because
the demonstrated full-width opaque C tag token is owned by the C handle table. The count then
returns to baseline. Both registrations release once, refuse stale handles and silence a second
create/destroy cycle. SOFTWARE and OPENGLES3 produce identical30-check output with empty stderr.
Metadata remains498/0, all67 focused C/API, C++ graphics-device/resource and CLI tests pass in
39.99s, and a clean1693-step full build follows. Four routes raise C coverage745->749/3205 and
`graphics_device.h`29->33/84. Clean sibling CNA is `d7eff66ee`; production `libcna.so` remains
byte-identical. Nothing is drawn and no visual inspection is claimed.
Evidence:checkpoint-498-c-resource-events-focused-*.

## Focused499-C-texture-slots

New `c_api.graphics.texture_slots` demonstrates the three graphics-device texture-collection routes.
Sixteen controls start from two empty high pixel slots on a fresh HiDef device; bind one owned Color
Texture2D and read its exact native occupancy and C-handle identity; clear only one slot with the
invalid handle; bind the same texture into both slots; clear both through whole-device unbinding;
and prove repeated unbinding is an idempotent no-op. A final rebind followed by typed destruction
clears the slot automatically and invalidates the stale handle. Every returned slot structure has
exact size/version and zero reserved bytes. SOFTWARE and OPENGLES3 produce identical output with
empty stderr. Metadata is499/0, all68 focused C/API, C++ graphics-device/resource and CLI tests
pass in40.15s, and a clean1696-step full build follows. Three routes raise C coverage749->752/3205
and `graphics_device.h`33->36/84. Clean sibling CNA is `d7eff66ee`; production `libcna.so` remains
byte-identical. No vertex-stage format support, drawing or visual inspection is claimed.
Evidence:checkpoint-499-c-texture-slots-focused-*.

## Focused500-C-buffer-bindings

New `c_api.graphics.buffer_bindings` demonstrates all eight graphics-device vertex/index binding
routes. Twenty controls begin with empty device state and create two real vertex buffers sharing one
declaration plus one index buffer. The single and offset vertex overloads round-trip exact handles
and offsets. An ordered two-stream array preserves both offsets and a nonzero instance frequency;
a short destination reports the complete count without mutation, and a negative second offset is
refused before either applied binding changes. The index binding round-trips its exact owned handle.
Empty bindings clear the device explicitly; after rebinding, destroying one vertex owner detaches
the whole multi-stream set and destroying the index owner clears its slot. SOFTWARE and OPENGLES3
produce identical output with empty stderr. Metadata is500/0, all76 focused C/API, C++ graphics
device/resource/buffer and CLI tests pass in59.02s, and a clean1699-step full build follows. The two
simple setters were already used by the pixel-validated BasicEffect cube; six newly named routes
raise C coverage752->758/3205 and `graphics_device.h`36->42/84. Clean sibling CNA is `d7eff66ee`;
production `libcna.so` remains byte-identical. No buffer data, drawing or visual inspection is
claimed.
Evidence:checkpoint-500-c-buffer-bindings-focused-*.

## Complete500-C-buffer-bindings

Complete checkpoint source `dee3837` absorbs the five focused graphics batches after complete497.
`tools/validate.sh` reports metadata500/0 and passes all513 enabled CTests in170.48s; three declared
tests remain disabled. SOFTWARE is434pass/28known-fail/35skip/3not-run and OPENGLES3 is
464pass/30known-fail/3skip/3not-run. Comparison with complete497 shows all three new IDs passing,
every existing status and known-issue value unchanged, and identical failed-assertion multisets of
40 and44. The extended C device-state and resource-lifecycle children pass22 and30 checks on both
renderers. Fresh runtime coverage is297/329 types,1405/2326 XNA members and758/3205 C routes. CNA
remained clean and unchanged at `d7eff66ee`, and the production library hash stayed unchanged. No
visual inspection is claimed.
Evidence:checkpoint-500-c-buffer-bindings-*.

## Focused501-C-device-clears

New `c_api.graphics.device_clears` demonstrates all three graphics-device clear routes and windowed
readback against an explicit HiDef Color/Depth24Stencil8 back buffer. Twenty-four controls clear
normalized RGBA red, Color-plus-depth green and all three selected planes blue; complete RGBA8
readback samples both pairs of corners and the center exactly after each. Non-finite channels/depth
and an unknown option bit are refused without changing the established color, while an empty mask
succeeds as a no-op. A2x2 source window writes exact pixels at destination offset2; short and
malformed requests preserve all caller bytes. SOFTWARE and OPENGLES3 produce identical output with
empty stderr. Metadata is501/0, the exact demo, four related C++ clear/readback/depth demos and three
CLI tests pass8/8, and the incremental full build is clean. Four routes raise C coverage
758->762/3205 and `graphics_device.h`42->46/84. Clean sibling
CNA is `d7eff66ee`; production `libcna.so` remains byte-identical. Pixel values were read
programmatically; no screenshot or visual inspection is claimed.
Evidence:checkpoint-501-c-device-clears-focused-*.

## Focused502-C-owned-device

New `c_api.graphics.owned_device` demonstrates the six caller-owned lifecycle routes. Twenty
controls create two distinct HiDef devices outside any Game, query the first, install all four
no-payload event subscriptions, clear and present one frame, then reset with current and changed
parameters. Both successful resets synchronously raise the exact resetting-then-reset order; the
second applies an explicit adapter plus400x300 dimensions and interval one, while a null parameter
set raises nothing. Reset/lost registrations release once. Destroying the second leaves the first
live; destroying the first raises one exact disposing callback which releases its registration
while the event source is still alive, and all stale handles are refused. SOFTWARE and OPENGLES3
produce byte-identical output with20 passing checks and empty stderr. Metadata is502/0; the exact
demo plus presentation/state/manager-event and CLI tests pass8/8. Six routes raise C coverage
762->768/3205 and `graphics_device.h`46->52/84. The header placement defect found during source
audit is recorded as ISSUE-065. Clean sibling CNA is `d7eff66ee`; production `libcna.so` remains
byte-identical. No displayed pixels, screenshot or visual inspection is claimed.
Evidence:checkpoint-502-c-owned-device-focused-*.

## Focused502-C-device-policy

The existing `c_api.graphics.device_states` workflow grows from22to25 controls. Its live HiDef
device begins with strict unsupported-3D handling. The callback changes the graphics profile to
Reach and the policy to warn-and-stub, reads each back exactly and proves the first undefined enum
identity is refused without mutation. Both original values are restored and re-read before the
existing object, sampler, scalar and geometric state workflow continues. SOFTWARE and OPENGLES3
produce byte-identical output with25 passing checks and empty stderr. Metadata remains502/0; seven
focused C/API, profile, capability and CLI regressions pass in5.07s, followed by a clean incremental
full build. Three routes raise C coverage768->771/3205 and `graphics_device.h`52->55/84. Clean
sibling CNA is `d7eff66ee`; production `libcna.so` remains byte-identical. Nothing is drawn and no
visual inspection is claimed. Evidence:checkpoint-502-c-device-policy-focused-*.

## Complete502-C-device-policy

Complete checkpoint source `1398d9d` absorbs device clears, caller-owned device lifecycle and the
profile/policy state extension after complete500. `tools/validate.sh` reports metadata502/0 and
passes all515 enabled CTests in208.32s; three declared tests remain disabled. SOFTWARE is436 pass,
28 known-fail,35 skip and3 not-run; OPENGLES3 is466/30/3/3. Comparison with complete500 shows both
new IDs passing, every existing status and known-issue value unchanged, and identical failed-
assertion multisets of40 and44. The extended C device-state child passes25 focused controls under
both renderers. Fresh runtime coverage is297/329 types,1405/2326 XNA members and771/3205 C routes.
CNA was clean at `d7eff66ee` for the checkpoint build, then advanced externally during smoke to
clean `f37f8d184` through changes confined to one test and three tooling files. The production CNA
and C ABI library hashes remained unchanged. No visual inspection was performed.
Evidence:checkpoint-502-c-device-policy-*.

## Focused503-C-draw-routes

New `c_api.graphics.draw_routes` demonstrates all five remaining draw-family routes, the primitive-
count helper, explicit current-effect selection and all three direct pipeline toggles. Twenty-four
controls pixel-prove all five primitive topologies; bound, caller-vertex, caller-indexed and repeated-
instance geometry; effect refusal/selection; blend enable/disable; and depth-test/depth-write
behavior. OPENGLES3 passes24/24. SOFTWARE passes23 and retains one expected ISSUE-066 failure:
its empty direct blend-enable implementation leaves two half-alpha instances at `(255,0,0,128)`
instead of accumulating near `(192,0,0,159)`. Both stderr files are empty and both batch wrappers
accept exactly their declared outcomes. Metadata is503/0, all19 focused C/API, C++ primitive/state
and CLI tests pass in23.69s, and the final incremental full build is clean. Ten routes raise total C
coverage771->781/3205, `graphics_device.h`55->64/84 and `effects.h`8->9/290. Clean sibling CNA is
`f37f8d184`; both production library hashes remain unchanged. Pixel values were read
programmatically; no screenshot or visual inspection is claimed.
Evidence:checkpoint-503-c-draw-routes-focused-*.

## Focused504-C-occlusion-query

New `c_api.graphics.occlusion_query` demonstrates all eight public C query routes through a real
owned HiDef query and full-screen visible draw. Twenty-three controls cover renderer presence,
precision, fresh-result refusal, Begin/End pairing, the required completion observation, bounded
polling, result interpretation, destruction and stale-handle preservation. A center-pixel readback
proves the measured quad is exact red and synchronizes pending work. SOFTWARE reports a precise
count of384000 for the800x480 buffer; OPENGLES3 reports its documented boolean mode and count one.
Both direct and runner executions pass with empty stderr. Metadata is504/0, all19 focused query,
capability and CLI tests pass in9.96s, and the final incremental build is clean. Eight routes raise
coverage781->789/3205 and `graphics_device.h`64->72/84. Clean sibling CNA is `f37f8d184`; both
production library hashes remain unchanged. No screenshot or visual inspection is claimed.
Evidence:checkpoint-504-c-occlusion-query-focused-*.

## Focused504-C-content-loss

The existing `c_api.graphics.render_target_bindings` workflow grows from22to25 controls. After its
binding audit restores the backbuffer, explicit device-wide notification marks both tracked targets
lost and raises the cube's subscribed ContentLost event exactly once. Rebinding only the2D target
clears only its flag; the cube remains lost until its own successful face bind, which clears it
without another event. SOFTWARE and OPENGLES3 produce byte-identical output with25 passing checks
and empty stderr. Metadata remains504/0; all28 focused C/API, render-target, dynamic-buffer and CLI
tests pass in30.37s with one declared SOFTWARE MRT skip, followed by a clean incremental build. One
route raises coverage789->790/3205 and `graphics_device.h`72->73/84. Clean sibling CNA is
`f37f8d184`; both production library hashes remain unchanged. This deterministic route does not
claim an actual platform context loss or visual inspection.
Evidence:checkpoint-504-c-content-loss-focused-*.

## Focused504-C-device-capabilities

The existing `c_api.graphics.device_capabilities` workflow grows from20to24 controls. All27 direct
render-target-format verdicts agree with the detailed supported-usage masks. SOFTWARE reports that
it executes neither shader source nor IBL, while OPENGLES3 reports both. All three color-space
support probes preserve the active state; set results agree with support, successful alternate
states would be restored, and undefined ordinal3 is refused atomically. Both renderers report sRGB0
with support `1/0/0` for sRGB/scRGB/HDR10. Both direct and runner executions pass with empty stderr.
Metadata remains504/0; all37 focused capability, presentation, render-target, PBR/shader and CLI
tests pass in17.95s with11 declared SOFTWARE skips, followed by a clean incremental build. Six
routes raise coverage790->796/3205 and `graphics_device.h`73->79/84. ISSUE-067 records the missing
C type/names for the color-space ordinals. Clean sibling CNA is `f37f8d184`; both production library
hashes remain unchanged. No rendering or visual inspection is claimed.
Evidence:checkpoint-504-c-device-capabilities-focused-*.

## Focused504-C-renderer-recreation

The existing `c_api.graphics.owned_device` workflow grows from20to23 controls while retaining every
prior label. With zero tracked GPU resources, a negative multisample reconstruction is refused
without presentation or event changes. A4-sample request rebuilds the same renderer family,
preserves its capability bits, applies4 samples on SOFTWARE and OPENGLES3 and raises no lifecycle
event; clear/present remain operational. A second reconstruction restores exact single-sample state
before the existing reset workflow. Both renderers produce byte-identical output with23 passing
checks and empty stderr. Metadata remains504/0; all27 enabled focused lifecycle, multisampling,
selection and CLI tests pass in11.94s, with one manual fullscreen test disabled, followed by a clean
incremental build. One route raises coverage796->797/3205 and `graphics_device.h`79->80/84. Clean
sibling CNA is `f37f8d184`; both production library hashes remain unchanged. No displayed pixels or
visual inspection is claimed.
Evidence:checkpoint-504-c-renderer-recreation-focused-*.

## Complete504-C-graphics-device

Complete checkpoint source `cf105b2` (functional graphics source through `2746fcc`) absorbs all
five focused graphics-device batches after complete502. `tools/validate.sh` reports metadata504/0
and passes all517 active CTests in413.76s; three declared tests remain disabled. SOFTWARE is
437pass/29known-fail/35skip/3not-run and OPENGLES3 is468/30/3/3. Against complete502, both new IDs
have their expected outcomes, every existing status and known-issue value is unchanged, SOFTWARE
adds only the ISSUE-066 failed assertion (40->41), and OPENGLES3 retains the exact44-assertion
multiset. Fresh runtime coverage is297/329 types,1405/2326 XNA members and797/3205 C routes.

The first complete attempt exposed an intermittent global-pointer observation failure in
`c_api.input.keyboard_mouse` while the other516 active tests passed. Ten isolated repetitions
passed. Because the demo's global warp/read/restore sequence shares one X-server pointer with every
SDL process, CTest now runs that demo serially. A targeted two-test schedule proves it starts only
after the other process exits, and the fresh complete suite passes. This is a test-orchestration
fix, not a CNA defect. CNA was clean `f37f8d184` for the18:09 build and advanced externally during
smoke to clean `fbd8e64a3` through two CI/CMake-test/docs/tooling-only commits. Production CNA
library hashes remain unchanged. No visual inspection was performed.
Evidence:checkpoint-504-c-graphics-device-*.

## Focused505-C-effect-annotations

New `c_api.graphics.effect_annotations` demonstrates all22 public annotation and annotation-
collection routes. Twenty-one controls prove copied non-ASCII UTF-8 name, semantic, cached string
and raw payload ownership; exact and atomic short-buffer text copies; complete versioned shape/type
metadata; Boolean, integer-bit, float, Vector2/3/4 and Matrix views; a separate signed-integer
value; insertion order and exact case-sensitive lookup; collection value copying; owned result
copies surviving their source handles and collection; bounds refusal and one-shot destruction.
Direct and runner executions produce identical output with empty stderr on SOFTWARE and
OPENGLES3. Metadata is505/0, all eight focused C/API, C++ effect-reflection and CLI tests pass in
9.04s, and the final incremental build is clean. Coverage rises797->819/3205 and `effects.h`
9->31/290. Clean sibling CNA is `fbd8e64a3`; production library hashes remain unchanged. No game,
graphics device, drawing or visual inspection is claimed.
Evidence:checkpoint-505-c-effect-annotations-focused-*.

## Focused506-C-effect-parameter-values

New `c_api.graphics.effect_parameter_values` demonstrates the16 public mutable-parameter metadata
and value routes plus the CPU-only Texture2D constructor needed for a real retained resource.
Thirty controls cover copied and versioned shape/type metadata; atomic short and exact
unterminated name, semantic and string copies; all nine tagged scalar and array families;
ordinary and transposed non-symmetric matrices; bounded array retrieval; and independent typed
Texture2D/base slots with exact retention and release. Direct execution passes. SOFTWARE and
OPENGLES3 runner output is byte-identical and every stderr file is empty. Metadata is506/0, all
eight focused C/API, C++ effect and CLI tests pass in4.57s, and the incremental build is clean.
Coverage rises819->836/3205, `effects.h`31->47/290 and `texture.h`1->2/21. Clean sibling CNA is
`fbd8e64a3`; production library hashes remain unchanged. No game, graphics device, shader
execution, drawing or visual inspection is claimed.
Evidence:checkpoint-506-c-effect-parameter-values-focused-*.

## Focused507-C-effect-parameter-collections

New `c_api.graphics.effect_parameter_collections` demonstrates all ten remaining mutable-parameter
and collection routes. Nineteen controls prove copied add-create metadata; stable mutable element
views after substantial collection growth; shared identity across index, exact-name and semantic
lookups; case-sensitive names, case-insensitive first semantic matching and upper-bound refusal;
distinct element, structure-member and annotation collections; and owned descendant lifetimes after
both root handles and nested collection views are destroyed. ISSUE-068 records that the public C
comment calls the deliberately case-insensitive semantic match exact. Direct execution passes.
SOFTWARE and OPENGLES3 runner output is byte-identical and every stderr file is empty. Metadata is
507/0, all nine focused C/API, C++ effect and CLI tests pass in4.66s, and the incremental build is
clean. Coverage rises836->846/3205 and `effects.h`47->57/290. Clean sibling CNA is `fbd8e64a3`;
production library hashes remain unchanged. No game, graphics device, shader execution, drawing or
visual inspection is claimed.
Evidence:checkpoint-507-c-effect-parameter-collections-focused-*.

## Focused508-C-effect-passes-techniques

New `c_api.graphics.effect_passes_techniques` demonstrates all31 ownerless pass, technique and
collection routes. Twenty-seven controls prove copied names with atomic short/exact copies;
distinct nonzero technique identities; ordinary and reflected runtime indices; empty versus
canonical-P0 construction; independent pass/technique annotation views; documented ownerless
no-op Apply; stable element views after substantial collection growth; exact case-sensitive
lookup and upper-bound refusal; and nested P0 survival after root collections, technique handles
and nested collection views are destroyed. Direct execution passes. SOFTWARE and OPENGLES3
runner output is byte-identical and every stderr file is empty. Metadata is508/0, all ten focused
C/API, C++ effect and CLI tests pass in6.55s, and the incremental build is clean. Coverage rises
846->877/3205 and `effects.h`57->88/290. CNA advanced externally from clean `fbd8e64a3` to clean
`a6bb279d47` through two docs-only commits; production library hashes remain unchanged. No effect-
owned pass, game, graphics device, shader execution, drawing or visual inspection is claimed.
Evidence:checkpoint-508-c-effect-passes-techniques-focused-*.
