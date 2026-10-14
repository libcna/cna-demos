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
  `graphics.render_target.render_target_cube` (dynamic environment map; rendered cube faces are
  upright on the GL renderers since CNA `a6e8104d2`, resolving ISSUE-021), ✅
  `graphics.render_target.multiple_render_targets` (two targets, a two-output GLSL
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

## Phase 12 -- networking, gamer services, avatars 🟨 (19 gamer services + 22 net demos)
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

## Focused509-C-effect-core

New `c_api.graphics.effect_core` demonstrates the18 core Effect lifecycle, reflection and state
routes. Twenty-seven controls cover the borrowed graphics device; exact type and empty source
bytes; compiled/renderer/stock identity; the default parameter, technique and P0 graph; owned-pass
application; null/foreign current-technique refusal without mutation; selection of an added owned
technique; a fresh canonical clone; independent disposal; and descendant survival after root-handle
destruction. The compiled constructor atomically distinguishes empty and malformed buffers, the
foreign MGFX container and a committed genuine5000-byte XNA Effect Framework binary. SOFTWARE and
OPENGLES3 both have their compiled-effect opt-ins disabled in this build, report the capability
false and explicitly refuse the valid fixture; compiled reflection and execution are therefore not
claimed. Direct execution passes, both runner outputs are byte-identical and all three stderr files
are empty. Metadata is509/0, all12 focused C/API, C++ Effect and CLI tests pass in4.16s, and the
incremental build is clean. Coverage rises877->895/3205 and `effects.h`88->106/290. Clean sibling
CNA is `a6bb279d47`; production library hashes remain unchanged. No drawing, pixel result or visual
inspection is claimed.
Evidence:checkpoint-509-c-effect-core-focused-*.

## Focused510-C-effect-concrete-types

New `c_api.graphics.effect_concrete_types` demonstrates all four EffectMaterial and SpriteEffect
construction/ownership routes. Twenty-two controls prove exact concrete type and borrowed device;
EffectMaterial-only rejection on a base Effect; null no-op and invalid-family preservation; two
owned list entries for one CPU Texture2D surviving caller-handle destruction; the canonical
EffectMaterial inherited clone returning an independent base Effect; source destruction survival;
exact stock SpriteEffect identity; type-preserving SpriteEffect clone; and independent disposal.
ISSUE-069 records that the generic C clone comment promises same-concrete-type output despite the
deliberate XNA EffectMaterial exception. Direct execution passes, SOFTWARE and OPENGLES3 runner
output is byte-identical and all three stderr files are empty. Metadata is510/0, all14 focused
C/API, C++ Effect/SpriteBatch and CLI tests pass in3.70s, and the incremental build is clean.
Coverage rises895->899/3205 and `effects.h`106->110/290. Clean sibling CNA is `a6bb279d47`;
production library hashes remain unchanged. No draw, pixel result or visual inspection is claimed.
Evidence:checkpoint-510-c-effect-concrete-types-focused-*.

New `c_api.graphics.basic_effect_state` demonstrates the remaining BasicEffect state routes, all
shared IEffectMatrices/IEffectFog/IEffectLights accessors and DirectionalLight ownership. Forty
controls establish exact defaults and mutations, stable nested views, the canonical three-light
preset, plain-Effect interface refusal, root-handle survival and a real retained same-device
Texture2D. Direct and both runner selections pass with byte-identical output and empty stderr;
all24 focused C/API, C++ BasicEffect/Effect/SpriteEffect and CLI tests pass in15.82s. Metadata is
511/0 and the incremental build is clean. Forty-two routes raise total C coverage899->941/3205 and
`effects.h`110->152/290. Clean sibling CNA is `a6bb279d47`; production library hashes remain
unchanged. Apply is state-only; no draw, pixel result or visual inspection is claimed.
Evidence:checkpoint-511-c-basic-effect-state-focused-*.

New `c_api.graphics.unlit_stock_effects` demonstrates all AlphaTestEffect and DualTextureEffect C
routes as one coherent unlit-material and ownership workflow. Thirty-three controls cover defaults,
unclamped alpha/reference state, comparison and vertex-color selection, concrete-type refusal,
independent shared matrix/fog state, invalid layer atomicity and two same-device textures retained
across type-preserving clones and source destruction. Direct and both runner selections pass with
byte-identical output and empty stderr; all14 focused C effect, C++ alpha-test/dual-texture and CLI
tests pass in9.09s. Metadata is512/0 and the incremental build is clean. All22 concrete routes raise
total C coverage941->963/3205 and `effects.h`152->174/290. Clean sibling CNA is `a6bb279d47`;
production library hashes remain unchanged. Apply is state-only; no draw, alpha-test pixel,
sampling or visual inspection is claimed. Evidence:checkpoint-512-c-unlit-stock-effects-focused-*.

New `c_api.graphics.environment_map_effect_state` demonstrates every EnvironmentMapEffect C route,
its required-lighting contract and distinct 2D/cube texture ownership. Thirty controls cover exact
defaults and material/map mutations, shared matrix/fog state, refused lighting disable, the
canonical three-light preset, retained same-device handles and type-preserving clone survival after
source destruction. Direct and both runner selections pass with byte-identical output and empty
stderr; all14 focused C effect, C++ EnvironmentMapEffect and CLI tests pass in7.57s. Metadata is
513/0 and the incremental build is clean. Seventeen effect routes plus TextureCube create/destroy
raise total C coverage963->982/3205, `effects.h`174->191/290 and `texture_volume.h`0->2/16. Clean
sibling CNA is `a6bb279d47`; production library hashes remain unchanged. Apply is state-only; no
draw, cube sampling, pixel result or visual inspection is claimed.
Evidence:checkpoint-513-c-environment-map-effect-focused-*.

New `c_api.graphics.skinned_effect_state` demonstrates every SkinnedEffect C route in one material,
palette and ownership workflow. Thirty-five controls cover defaults and mutations, required
lighting, the exact one/two/four weight contract, the copied 72-bone maximum, atomic short-buffer
and invalid-count refusal, same-device texture validation and type-preserving clone survival after
source destruction. Direct and both runner selections pass with byte-identical output and empty
stderr; all26 focused C effect, C++ SkinnedEffect and CLI tests satisfy their contracts in15.47s
(23 passed, three declared SOFTWARE capability skips). Metadata is514/0 and the incremental build
is clean. All21 routes raise total C coverage982->1003/3205 and `effects.h`191->212/290. Clean
sibling CNA is `a6bb279d47`; production library hashes remain unchanged. Apply is state-only; no
skinned draw, vertex-weight, pixel result or visual inspection is claimed.
Evidence:checkpoint-514-c-skinned-effect-focused-*.

New `c_api.graphics.color_matrix_effect_state` demonstrates every ColorMatrixEffect C route in one
finite-state and clone workflow. Twenty-two controls cover exact identity/zero defaults, a complete
authored row-major transform and unclamped offset, atomic NaN/infinity refusal, the Rec. 709
grayscale preset, reset and clone survival after source destruction. Direct and both runner
selections pass, runner output is byte-identical and all three stderr files are empty; all17 focused
C/C++ ColorMatrixEffect and CLI tests pass in6.73s. Metadata is515/0 and the incremental build is
clean. All7 routes raise total C coverage1003->1010/3205 and `effects.h`212->219/290. Clean sibling
CNA is `a6bb279d47`; production library hashes remain unchanged. The existing C++ SOFTWARE demo
supplies pixel validation; this C workflow makes no draw, transformed-pixel or visual-inspection
claim. Evidence:checkpoint-515-c-color-matrix-effect-focused-*.

New `c_api.graphics.pbr_effect_material_state` demonstrates every non-texture PbrEffect material
route together with required lighting, matrices/fog and clone lifetime. Thirty-three controls cover
exact core/glTF defaults, unclamped mutations, all alpha modes, invalid-value preservation and
shared interfaces. Twenty-nine pass identically under SOFTWARE and OPENGLES3; four retained
failures establish CNA-DEMOS-ISSUE-070 because clone resets vertex color, alpha mode, alpha cutoff
and double-sided state while preserving the other material values and output encoding. Runner
output is byte-identical and all three stderr files are empty; all19 focused PBR and CLI tests
satisfy their contracts in5.94s (16 passed, three declared SOFTWARE capability skips). Metadata is
516/0 and the incremental build is clean. Thirty-one routes raise total C coverage1010->1041/3205
and `effects.h`219->250/290. Clean sibling CNA is `a6bb279d47`; production library hashes remain
unchanged. Apply is state-only; no PBR draw, BRDF pixel or visual inspection is claimed.
Evidence:checkpoint-516-c-pbr-material-focused-*.

New `c_api.graphics.pbr_texture_slots` demonstrates the complete seven-slot PBR texture surface.
Thirty-four controls cover transform initialization/equality, all default and authored per-slot UV/
transform/sRGB values, invalid-value preservation, same-device validation and two real textures
retained by five core plus two specular slots through cloning and source destruction. Each resource
releases only when its final clone slot clears. Direct and both runner selections pass with byte-
identical runner output and empty stderr; all20 focused PBR and CLI tests satisfy their contracts in
5.44s (17 passed, three declared SOFTWARE capability skips). Metadata is517/0 and the incremental
build is clean. Ten routes raise total C coverage1041->1051/3205 and `effects.h`250->260/290.
CNA-DEMOS-ISSUE-071 records the passing C-header documentation mismatch that calls these seven
identities five. Clean sibling CNA is `a6bb279d47`; production library hashes remain unchanged. No
sampling, PBR draw, pixel result or visual inspection is claimed.
Evidence:checkpoint-517-c-pbr-textures-focused-*.

New `c_api.graphics.skinned_pbr_effect_state` demonstrates every SkinnedPbrEffect-specific C route
and representative shared PBR/interface compatibility. Thirty-two controls cover the full default
and authored 72-bone palettes, copied input, atomic short output, invalid-count preservation,
one/two/four weights, material/matrix/fog/required-light state and a retained same-device base
texture. A type-preserving clone keeps material, palette and texture after source destruction, then
releases the texture independently. Direct and both runner selections pass with byte-identical
runner output and empty stderr; all21 focused PBR and CLI tests satisfy their contracts in6.07s
(18 passed, three declared SOFTWARE capability skips). Metadata is518/0 and the final incremental
build is clean. Five routes raise total C coverage1051->1056/3205 and `effects.h`260->265/290. The
initial configure build repeated the known transient CNA-DEMOS-ISSUE-057 staging warning but
completed; the clean gate does not repeat it. Clean sibling CNA is `a6bb279d47`; production library
hashes remain unchanged. Apply is state-only; no skinned PBR draw, deformation, BRDF pixel or visual
inspection is claimed. Evidence:checkpoint-518-c-skinned-pbr-focused-*.

New `c_api.graphics.shader_effect_state` demonstrates source construction, canonical validity and
renderer ownership, empty successful compiler diagnostics and independent World/View/Projection
state. Seventeen controls pass directly and under both renderer selections with byte-identical
runner output and empty stderr. The supplied dialect-correct shader must compile and apply whenever
the renderer advertises source execution; non-executing renderers retain their documented object
contract. All24 focused ShaderEffect and CLI tests satisfy their contracts in19.54s (16 passed,
eight declared SOFTWARE capability skips). Metadata is519/0 and the incremental build is clean.
Ten routes raise total C coverage1056->1066/3205 and `effects.h`265->275/290. Clean sibling CNA is
`a6bb279d47`; the production library baseline is unchanged. No draw, pixel or visual inspection is
claimed. Evidence:checkpoint-519-c-shader-state-focused-*.

New `c_api.graphics.shader_effect_uniforms` demonstrates all eleven loose typed and uniform-block
ShaderEffect routes. Eighteen controls exercise active matrix/vector/scalar/integer uniforms,
float/vec2 arrays, tightly packed vec3/mat4 arrays, empty-null forms, malformed arrays, an embedded-
NUL name and a two-member std140 declaration/clear after overwriting caller storage. Direct and
both runner selections pass with byte-identical output and empty stderr; all25 focused ShaderEffect
and CLI tests satisfy their contracts in5.64s (17 passed, eight declared SOFTWARE capability
skips). Metadata is520/0 and the final incremental build is clean. Eleven routes raise total C
coverage1066->1077/3205 and `effects.h`275->286/290. The configure build emitted only the known
third-party Draco CMP0148 developer warning. Clean sibling CNA is `a6bb279d47`; the production
library baseline is unchanged. Existing pixel demos cover rendered uniform behavior; this workflow
draws nothing and claims no pixel or visual inspection.
Evidence:checkpoint-520-c-shader-uniforms-focused-*.

New `c_api.graphics.shader_effect_textures` demonstrates all three ShaderEffect texture setters and
the first C Texture3D resource lifetime. Twenty controls create real 2D, cube and 1x1x1 volume
textures under both requested renderers; refuse negative units and wrong-family handles; replace
2D with cube and cube with volume on one sampler unit; and retain the volume through Effect
destruction until the last pass view releases. Direct and both runner selections pass with byte-
identical output and empty stderr; all26 focused ShaderEffect and CLI tests satisfy their contracts
in6.57s (18 passed, eight declared SOFTWARE capability skips). Metadata is521/0 and the final
incremental build is clean. The three bindings plus Texture3D create/destroy raise total C coverage
1077->1082/3205, `effects.h`286->289/290 and `texture_volume.h`2->4/16. The configure build emitted
only the known third-party Draco CMP0148 developer warning. Clean sibling CNA is `a6bb279d47`; the
production library baseline is unchanged. No sampling, draw, pixel or visual inspection is claimed.
Evidence:checkpoint-521-c-shader-textures-focused-*.

The existing `c_api.content.content_manager` now loads a deterministically generated authored
BasicEffect CNJ through `cna_content_manager_load_effect`. Its53-control workflow proves exact
diffuse/emissive colors, specular power, alpha and vertex-color state, ordinary Effect application
and independent usability after both manager unload and manager destruction while retaining every
previous texture/font/cache/root/draw check. Direct and both runner selections pass with byte-
identical output and empty stderr; all33 focused ContentManager, BasicEffect and CLI tests pass in
14.38s. Metadata remains521/0 and the incremental build is clean. This final route raises total C
coverage1082->1083/3205 and completes `effects.h`289->290/290. Clean sibling CNA is `a6bb279d47`;
the production library baseline is unchanged. Existing texture/font back-buffer pixels were read
programmatically; no screenshot or visual inspection is claimed.
Evidence:checkpoint-521-c-effect-load-focused-*.

## Complete521-C-effects

Complete checkpoint **521-C-effects**, source `811d2f4`, absorbs the seventeen focused effect demos
and ContentManager Effect-load extension since complete504. Full `tools/validate.sh` passes metadata
521/0 and all534 enabled CTests in233.86s; three declared tests remain disabled. SOFTWARE reports
453pass/30known-fail/35skip/3not-run and OPENGLES3 reports484/31/3/3, with zero unexpected passes.
Every one of the504 earlier status/knownIssue pairs is unchanged; the17 new IDs contribute16 passes
and the expected ISSUE-070 failure on each renderer. Fresh runtime coverage is297/329 types,
1405/2326 XNA members and1083/3205 C routes, including complete `effects.h`290/290. The build log is
diagnostic-free. Clean sibling CNA is `a6bb279d47`; production library hashes remain unchanged and
the exercised `cna-demos` SHA-256 is
`15adb468549aec96966f9a9f2a5bd2947ca0a3e9cdb4d3bb57e1023043965735`. No screenshot or visual
inspection was performed. Evidence:checkpoint-521-complete-*.

The existing `c_api.media.media_player` now closes the final meaningful `media_player.h` route
without adding an ID. After the ordinary game loop has ended, it plays a real 250 ms track and
waits long enough for native playback to finish without pumping MediaPlayer state. The public state
is still Playing immediately before `cna_media_player_update_ext`; that explicit host maintenance
call consumes the pending end notification and transitions the one-song, non-repeating queue to
Stopped. Direct execution and both renderer selections pass all94 controls; filtering the two new
labels restores all92 earlier controls in exact order, runner output is byte-identical and all
stderr files are empty. All23 focused MediaPlayer/Song/CLI CTests pass in44.43s, metadata remains
521/0 and the final incremental build is diagnostic-free. Coverage rises1083->1084/3205 and
`media_player.h`38->39/41. Its remaining two routes only size/copy the fixed MediaQueue reflection
name already verified by CNA's pure-C smoke, so they remain intentionally trivial and uncredited.
No audible quality, callback-latency or visual claim is made. Clean sibling CNA is `a6bb279d47` and
the production library hashes remain unchanged. Evidence:checkpoint-521-c-player-update-focused-*.

New `c_api.graphics.texture_formats` begins the deeper `texture.h` audit with the five
renderer-independent format routes. Six deterministic controls cover all27 public SurfaceFormat
identities: exact compressed-block footprint, texel/block byte size, pixel-store alignment,
compatible transfer element widths and the base Texture policy that accepts Color while returning
NOT_SUPPORTED for every other valid format. Invalid identities and a nonpositive width are refused
without changing getter output. Direct execution and both renderer selections pass with byte-
identical output and empty stderr. All32 focused Texture/CLI CTests satisfy their contracts in
34.25s, including two declared SOFTWARE capability skips. Metadata is522/0 and coverage rises
1084->1089/3205, with `texture.h`2->7/21. No texture is allocated and no renderer format-support,
pixel or visual claim is made. Final observation sees clean sibling CNA `b2ded99da9`; its only
change since the media batch is documentation and the production library hashes remain unchanged.
Evidence:checkpoint-522-c-texture-formats-focused-*.

New `c_api.graphics.texture2d_io` closes every meaningful `texture.h` resource route in one
27-control workflow. It contrasts an empty standalone texture with a PNG-loaded CPU-backed one;
performs exact full RGBA and offset-window rectangle transfers with atomic short readback; sizes and
copies PNG/JPEG output; saves, reloads and removes a private PNG; and uses one live device callback
for file, authored-RGBA and encoded-memory creation, exact raw replacement, resize/crop behavior and
complete child cleanup. Direct execution and both renderer selections pass with byte-identical
output and empty stderr. All34 focused C Texture/Texture/CLI tests satisfy their contracts in18.83s,
including two declared SOFTWARE capability skips. Metadata is523/0 and coverage rises1089->
1101/3205, with `texture.h`7->19/21. Its remaining pair only size/copies the fixed Texture2D type
name already covered by CNA's generic C smoke, so the header's semantic audit is complete. No draw,
screenshot or visual inspection is claimed. Clean sibling CNA is `b2ded99da9` and production
library hashes are unchanged. Evidence:checkpoint-523-c-texture2d-io-focused-*.

New `c_api.graphics.texture_volume_data` closes every meaningful `texture_volume.h` route in one
19-control HiDef workflow. It verifies specific/common metadata for a2x2x2 Color volume, exact
offset-window box transfers, atomic short reads and raw32-byte replacement; then round-trips
distinct payloads through all six2x2 cube faces and checks another atomic short read. A generated
176-byte DXT1 cube DDS decodes from memory into a4x4 one-level public Color cube on both selected
renderers, while documented NOT_SUPPORTED remains a valid outcome for other backends. Direct and
runner output is byte-identical with empty stderr. All35 focused C Texture/Texture/CLI tests satisfy
their contracts in23.95s, including two declared SOFTWARE capability skips. Metadata is524/0 and
coverage rises1101->1109/3205, with `texture_volume.h`4->12/16. Its remaining four routes are fixed
Texture3D/TextureCube type-name pairs already covered by CNA's generic C smoke, so the header's
semantic audit is complete. Nothing is sampled or drawn and no visual inspection is claimed.
Clean sibling CNA is `b2ded99da9` and production library hashes are unchanged.
Evidence:checkpoint-524-c-texture-volume-focused-*.

## Complete524-C-textures

Complete checkpoint **524-C-textures**, source `b5b4ec2`, absorbs the explicit MediaPlayer pump and
all three texture batches since complete521. Full `tools/validate.sh` passes metadata524/0 and all
537 enabled CTests in271.80s; three declared tests remain disabled. SOFTWARE reports456pass/
30known-fail/35skip/3not-run and OPENGLES3 reports487/31/3/3, with zero unexpected passes. Every
one of the521 earlier status/knownIssue pairs is unchanged; the three new texture IDs pass on both
renderers, while the MediaPlayer extension preserves its existing ID's passing status. Fresh
runtime coverage is297/329 types,1405/2326 XNA members and1109/3205 C routes. The build log is
diagnostic-free. Clean sibling CNA is `b2ded99da9`; production library hashes remain unchanged and
the exercised `cna-demos` SHA-256 is
`3cc772719b3e44c5c57dea583c0d611ed3319f3fc100d1b520bf8e465aca03bd`. No screenshot or visual
inspection was performed. Evidence:checkpoint-524-complete-*.

## Focused525-C-sprite-font

New `c_api.graphics.sprite_font_state` closes every remaining `sprite_font.h` route in one
26-control caller-built workflow. It creates a real atlas and three asymmetric glyph records,
proves constructor copy ownership, atomically reads the parallel character/glyph tables, verifies
canonical output metadata, measures exact multiline layout before and after fallback/spacing
changes, preserves state across rejected setters, clears fallback and observes missing-glyph
refusal, and establishes atlas retention through font destruction. Direct execution plus SOFTWARE
and OPENGLES3 runner selections pass with byte-identical output and empty stderr. All24 focused C
SpriteFont, C++ SpriteFont/CNJ and CLI tests pass in12.78s. Metadata is525/0 and C coverage rises
1109->1116/3205, completing `sprite_font.h`2->9/9. The incremental build is diagnostic-free. Clean
sibling CNA is `b2ded99da9` and production library hashes remain unchanged. Nothing is drawn and no
screenshot or visual inspection is claimed. Evidence:checkpoint-525-c-sprite-font-focused-*.

## Staged526-551-C-member-audit

Nine new standalone IDs and two existing-demo extensions continue the deeper C member/behavior
audit. They are source-complete and have strict direct builds plus focused execution against the
last completed CNA libraries. The ordinary project build, catalog, CTests and generated coverage
remain pending because the sibling CNA checkout is in the middle of a large Vulkan-retirement
refactor; none of the following is claimed as project-built runner evidence yet.

`c_api.graphics.vertex_values` passes seven controls for public declaration/value records.
`c_api.graphics.vertex_buffer_data` passes36 controls for copied declarations, retained buffer
metadata, typed and raw caller windows, dynamic options, a binding descriptor and ContentLost
registration ownership. Both selected environments are byte-identical with empty stderr and the
latter covers all15 meaningful previously uncredited `vertex_resources.h` routes; its four fixed
reflection-name routes remain trivial. Evidence:checkpoint-526-c-vertex-values-focused-*-direct.*
and checkpoint-527-c-vertex-buffer-data-focused-*-direct.*.

`c_api.audio.sound_effect_sources` passes22 controls for exact sample conversion, PCM/encoded/file
source ownership, atomic name copies, disposal, master-volume restoration and one-shot Play.
`c_api.audio.sound_effect_instance_state` passes19 controls, including five repeated SOFTWARE
runs, for mixing state and the complete ordinary transport lifecycle. ISSUE-072 records the public
header's incorrect pitch-clamping wording; actual CNA consistently rejects out-of-range pitch.
Evidence:checkpoint-528-c-sound-effect-sources-focused-*-direct.* and
checkpoint-529-c-sound-instance-state-focused-*-direct.*.

`c_api.audio.spatial_state` passes21 of22 controls identically on both selections and preserves the
single ISSUE-073 failure: a refused live-3D pan write changes the public property snapshot.
`c_api.audio.dynamic_streaming` passes21 controls for conversions, copied PCM/float submission,
latched-format refusal, queue/update behavior, three BufferNeeded requests and unsubscribe.
`c_api.audio.microphone_availability` passes20 controls against SDL dummy's capture device for
enumeration, exact names, duration/state, real BufferReady/GetData, conversions, pump, unsubscribe
and upper-bound refusals. Current-source metadata reaches66/74 `audio.h` routes; the eight remaining
routes are four fixed reflection-name pairs. No audible quality, physical microphone or callback-
latency claim is made. Evidence:checkpoint-530-c-spatial-state-focused-*-direct.*,
checkpoint-532-c-dynamic-streaming-focused-*-direct.* and
checkpoint-533-c-microphone-availability-focused-*-direct.*.

The existing `c_api.media.media_player` adds four controls for the final MediaQueue reflection
buffer pair: exact size, zero-capacity sizing, atomic short refusal and exact unterminated copy.
Both direct selections pass98 controls with empty stderr; check outcomes match and only the existing
live-position diagnostic varies. Current-source metadata completes `media_player.h` at41/41.
Evidence:checkpoint-531-c-media-queue-type-name-focused-*-direct.*.

`c_api.graphics.packed_vectors` passes nine byte-identical controls without creating a game or
device. It covers all17 formats, exact bits/canonical expansion, width-aware equality, atomic
invalid-bit and nonfinite handling, unused-component behavior and binary16 precision/special
values. All7 `packed_vectors.h` routes are credited. Evidence:
checkpoint-534-c-packed-vectors-focused-*-direct.*.

The existing `c_api.input.touch_state` adds pure gesture, capability, location and state
construction, exact TouchLocation text and the live false/true touch-device getter around its
existing fixture. The final direct workflow passes72 byte-identical controls with empty stderr and
credits43/47 `input_touch.h` routes. The two mouse-emulation routes remain manual and the two opaque
window-handle routes remain platform integration. Evidence:
checkpoint-535-c-touch-values-focused-*-direct.* and
checkpoint-536-c-touch-device-exists-focused-*-direct.*.

`c_api.math.color_values` closes all24 `color.h` routes in one15-control value workflow. Every
constructor family establishes channel clamping and rounding; packed, vector, debug-text and
canonical-text views are exact and short text writes are atomic; equality and hashing observe the
same four channels; named constants establish packed ordering and aliases; interpolation, scaling,
both premultiplication families and native NaN/infinity packing retain XNA behavior. Both direct
selections are byte-identical with empty stderr, and strict C99 plus metadata compilation pass.
No game, device, image, colour-space or display-quality claim is made. Evidence:
checkpoint-537-c-color-values-focused-*-direct.*.

`c_api.math.quaternion_values` closes all28 `quaternion.h` routes in one15-control value workflow.
It distinguishes C's zero value from rotation identity; relates construction, equality/hash,
length, returned/in-place conjugation and normalization; performs exact and atomic text copies;
separates component arithmetic from multiplication and XNA concatenation order; and compares
axis-angle, matrix, yaw-pitch-roll, inverse/division, dot and distinct Lerp/Slerp paths. Both direct
selections are byte-identical with empty stderr, and strict C99 plus metadata compilation pass.
No game, graphics device, transformed geometry or renderer claim is made. Evidence:
checkpoint-538-c-quaternion-values-focused-*-direct.*.

`c_api.math.scalar_helpers` demonstrates all15 scalar-helper routes from `math.h` in11 controls.
It covers exact constants, affine and curved interpolation, SmoothStep endpoints, float/integer
clamping and NaN propagation, distance/min/max, angle conversion and wrapping, the strict epsilon
boundary, and bounded MSAA power selection with atomic negative-input refusal. Both direct
selections are byte-identical with empty stderr, and strict C99 plus metadata compilation pass.
An initial overly tight degree round-trip assertion was corrected from `1e-5` to the established
single-precision `1e-3`; it was a demo defect. No device MSAA support is claimed. Evidence:
checkpoint-539-c-scalar-helpers-focused-*-direct.*.

`c_api.math.point_values` demonstrates all12 Point routes from `math.h` in seven controls. It
relates default initialization to Zero, checks authored identity/hash and exact/atomic text, then
contrasts ordinary component arithmetic with defined two's-complement wraparound and atomic
zero-divisor/overflow refusal. Both direct selections are byte-identical with empty stderr, and
strict C99 plus metadata compilation pass. Evidence:
checkpoint-540-c-point-values-focused-*-direct.*.

`c_api.math.rectangle_values` demonstrates all25 Rectangle routes in12 controls. It covers
empty/authored construction, derived edges/location/center, defined edge wraparound, half-open
coordinate/Point/rectangle containment, location/offset/inflate mutation, identity/hash,
exact/atomic text, strict positive-area overlap, exact intersection and union. Both direct
selections are byte-identical with empty stderr, and strict C99 plus metadata compilation pass.
The scalar, Point and Rectangle batches together close `math.h` at52/52. No clipping,
rasterization or pixel-coverage claim is made. Evidence:
checkpoint-541-c-rectangle-values-focused-*-direct.*.

`c_api.math.vector2_values` demonstrates all41 Vector2 routes in15 controls. It covers constructors
and constants, equality/hash, length and both normalization forms, exact/atomic text,
component/scalar arithmetic and IEEE zero division, metrics, curve/bound interpolation,
reflection, distinct matrix point/normal and quaternion transforms, all three bounded array forms,
and atomic invalid-window refusal. Both direct selections are byte-identical with empty stderr,
and strict C99 plus metadata compilation pass. No rendered geometry or SIMD-performance claim is
made. Evidence:checkpoint-542-c-vector2-values-focused-*-direct.*.

`c_api.math.vector3_values` demonstrates all50 Vector3 routes in17 controls. It adds all three unit
axes and six named XNA directions with negative-Z Forward, zero normalization, cross product,
3D point/normal/quaternion transforms and all three bounded array routes to the shared value,
arithmetic, metric, interpolation, reflection and atomic-window contracts. Both direct selections
are byte-identical with empty stderr, and strict C99 plus metadata compilation pass. No rendered
geometry or SIMD-performance claim is made. Evidence:
checkpoint-543-c-vector3-values-focused-*-direct.*.

`c_api.math.vector4_values` demonstrates all46 Vector4 routes in17 controls. Constructors extend
Vector2/3 values and define four unit axes; value, arithmetic, metric, interpolation and atomic-text
contracts include zero normalization. Homogeneous matrix transforms show implicit W one for
Vector2/3 and translation scaled by authored Vector4 W, while quaternion transforms preserve W.
Both bounded array forms and atomic invalid-window refusal pass. Both direct selections are byte-
identical with empty stderr, and strict C99 plus metadata compilation pass. Checkpoints542-544
together close `vectors.h` at137/137. No projection, clipping, rendered geometry or SIMD claim is
made. Evidence:checkpoint-544-c-vector4-values-focused-*-direct.*.

`c_api.math.matrix_values` demonstrates all35 non-factory `matrix.h` routes in18 controls. It
distinguishes zero initialization from Identity, maps all16 row-major fields, establishes the six
signed directional properties and translation row, then covers determinant, identity/hash,
successful and singular decomposition, exact/atomic text, component arithmetic, row-vector
composition order, transpose, inverse and quaternion transform. Both direct selections are byte-
identical with empty stderr, and strict C99 plus metadata compilation pass. Evidence:
checkpoint-545-c-matrix-values-focused-*-direct.*.

`c_api.math.matrix_factories` demonstrates all22 `cna_matrix_create_*` routes in13 controls.
Ordinary and coincident/fallback billboards, constrained orientation, four equivalent Z-rotation
paths, direct X/Y rotation, paired world/look-at frames, centered and off-center projections,
atomic invalid perspective ranges, three scale forms, offset-plane shadow/reflection and both
translation forms are explicit. Both direct selections are byte-identical with empty stderr, and
strict C99 plus metadata compilation pass. Checkpoints545-546 close `matrix.h` at57/57. No game,
graphics device, clipping, rendered geometry or visual result is claimed. Evidence:
checkpoint-546-c-matrix-factories-focused-*-direct.*.

`c_api.math.plane_values` demonstrates all20 Plane routes from `geometry.h` in nine controls. Five
construction forms converge on exact public fields; the three dot products separate homogeneous,
coordinate and direction behavior; identity/hash, returned/in-place normalization, exact/atomic
text, box/sphere/frustum classification, matrix translation and quaternion rotation are explicit.
Both direct selections are byte-identical with empty stderr, and strict C99 plus metadata
compilation pass. No game, device, culling pipeline or rendered geometry is claimed. Evidence:
checkpoint-547-c-plane-values-focused-*-direct.*.

`c_api.math.ray_values` demonstrates all11 Ray routes from `geometry.h` in seven controls. It
covers zero/authored construction, identity/hash, equal nearest-hit distances for a unit box and
sphere, forward-hit and away-miss plane behavior, inside and forward-miss frustum behavior, and
exact/atomic legacy double-brace text. Both direct selections are byte-identical with empty stderr,
and strict C99 plus metadata compilation pass. No physics, acceleration structure or rendering is
claimed. Evidence:checkpoint-548-c-ray-values-focused-*-direct.*.

`c_api.math.bounding_box_values` demonstrates all20 BoundingBox routes from `geometry.h` in11
controls. It covers zero/min-max construction; Contains, Intersects and Disjoint across boxes,
spheres, a point and a frustum; atomic copying in XNA's eight-corner order; hit/miss ray distance;
frustum/sphere/box/plane intersections; identity/hash; exact/atomic legacy text; point, sphere and
merge factories; and atomic empty-point refusal. Both direct selections are byte-identical with
empty stderr, and strict C99 plus metadata compilation pass. No collision response, broad phase or
rendering is claimed. Evidence:checkpoint-549-c-bounding-box-values-focused-*-direct.*.

`c_api.math.bounding_sphere_values` demonstrates all21 BoundingSphere routes from `geometry.h` in
12 controls. It covers construction, largest-axis transform scaling, all containment classes,
identity/hash, enclosing box/frustum/point/merge factories, atomic empty-point refusal,
box/frustum/tangent-sphere/ray/plane intersections and exact/atomic text. An initial demo defect
assumed `CreateFromFrustum` promised the exact centered minimum; the API promises only enclosure,
and the corrected check proves all eight source corners lie inside CNA's Ritter-style result. Both
direct selections are byte-identical with empty stderr, and strict C99 plus metadata compilation
pass. Evidence:checkpoint-550-c-bounding-sphere-values-focused-*-direct.*.

`c_api.math.bounding_frustum_values` demonstrates all22 BoundingFrustum routes from `geometry.h`
in13 controls. It covers the retained matrix, six exact identity clipping planes, all containment
classes, atomic XNA-ordered corners, frustum/box/sphere/plane intersections, ray inside/miss
semantics, the documented atomic `NOT_SUPPORTED` boundary-origin case, identity/hash and exact/
atomic six-plane text. Both direct selections are byte-identical with empty stderr, and strict C99
plus metadata compilation pass. Checkpoints547-551 close `geometry.h` at94/94. No graphics device,
rendered culling or visibility-performance claim is made. Evidence:
checkpoint-551-c-bounding-frustum-values-focused-*-direct.*.

`c_api.math.curve_key_values` demonstrates all17 CurveKey routes from `curve.h` in nine controls.
It establishes the fixed five-field layout with a strict-C99 alignment probe; constructor defaults
and full authored values; every getter and mutable field; atomic invalid-continuity refusal; clone
independence; position-only ordering; and equality/hash behavior across all fields. Both direct
selections are byte-identical with empty stderr, and strict C99 plus metadata compilation pass. An
initial use of C11 `_Alignof` was corrected as a demo portability defect. Evidence:
checkpoint-552-c-curve-key-values-focused-*-direct.*.

`c_api.math.curve_key_collection` demonstrates all14 owned CurveKeyCollection routes in11
controls: empty mutable creation, sorted insertion and access, full-key membership/indexing,
replacement reordering, atomic short and exact offset copies, invalid-key preservation, deep clone,
clear, value/index removal, invalid-index preservation and handle destruction. Both direct
selections are byte-identical with empty stderr, and strict C99 plus metadata compilation pass.
Checkpoints552-553 establish31/45 `curve.h` routes. Evidence:
checkpoint-553-c-curve-key-collection-focused-*-direct.*.

`c_api.math.curve_evaluation` demonstrates all14 Curve routes in15 controls. It uses the retained
mutable key view to move from empty through one and three keys; evaluates interpolation; computes
matching/separate whole-curve and per-key tangents; reads, sets and evaluates every loop mode;
rejects invalid modes atomically; deep-clones keys and loop state; and proves key views retain the
curve after its public handle is destroyed. Equal endpoint slopes establish the ordinary Linear
path without duplicating the distinct-slope ISSUE-036 failure; the existing C++ demos continue to
expose that and ISSUE-002's Step defect. Both direct selections are byte-identical with empty
stderr, and strict C99 plus metadata compilation pass. Checkpoints552-554 close `curve.h` at45/45.
Evidence:checkpoint-554-c-curve-evaluation-focused-*-direct.*.

`c_api.core.logger` demonstrates all19 logger routes from `core_ext.h` in10 controls. A caller sink
captures exact formatted borrowed text and distinct severity/category identities from all seven
direct and all seven conditional helpers plus the general route. The workflow proves false
suppression, category validation, minimum-level filtering, atomic refusal of malformed identities,
booleans and views, then restores both process-global settings. Both direct selections are byte-
identical with empty stderr, and strict C99 plus metadata compilation pass. This establishes
`core_ext.h` at19/57. Evidence:checkpoint-555-c-core-logger-focused-*-direct.*.

`c_api.platform.host_identity` demonstrates the eight stable platform, desktop-OS and assembly-
title routes from `core_ext.h` in10 controls. Portable target/Apple/mobile/desktop facts agree;
the platform name and process title use exact unterminated/atomic-short copy contracts; the prior
title is restored after valid, short and malformed input checks. Embedded NUL returns the shared
text contract's `ENCODING`, while the route-specific header only documents `INVALID_ARGUMENT`;
passing documentation ISSUE-074 records that mismatch. Both direct selections are byte-identical
with empty stderr, and strict C99 plus metadata compilation pass. Stable `core_ext.h` coverage is
27/57; its renderer-selection half awaits a coherent external renderer-retirement revision.
Evidence:checkpoint-556-c-platform-host-identity-focused-*-direct.*.

`c_api.game.title_content` demonstrates the remaining three TitleLocation/TitleContainer routes
with the previously credited path copy in11 controls. It atomically copies the executable root,
moves it to deployed assets, sizes and reads the complete256-byte checker PNG into caller storage,
maps a missing file to IO, and restores the process root. Both Xvfb direct selections are byte-
identical with empty stderr, and strict C99 plus metadata compilation pass. Evidence:
checkpoint-557-c-title-content-focused-*-direct.*.

`c_api.game.runtime_controls` demonstrates the final18 `runtime.h` routes in14 controls, plus two
`runtime_window.h` title accessors. It verifies exact/atomic title state; mouse, timing, sleep and
run-loop properties; FPS/millisecond derivation; idle reset/dispatcher requests; one-draw
suppression across two explicit ticks; and independent Disposed registration ownership. The first
draft incorrectly treated game destruction as registration-handle release; runtime and source show
detachment followed by one caller release, so this was a demo defect. Both Xvfb direct selections
are byte-identical with empty stderr, and strict C99 plus metadata compilation pass. Checkpoints557-
558 close `runtime.h` at45/45 and establish `runtime_window.h` at2/20. Evidence:
checkpoint-558-c-runtime-controls-focused-*-direct.*.

`c_api.game.window_state` demonstrates the remaining18 `runtime_window.h` routes in19 controls.
It round-trips and restores resizing/border policies; reads client bounds/orientation; copies exact
and atomic screen/type names; accepts documented platform refusals for state requests; brackets a
real client resize and synchronous event; and validates both legacy token and versioned borrowed
native-window fields without dereferencing them. Both Xvfb direct selections are byte-identical
with empty stderr, and strict C99 plus metadata compilation pass. Together with checkpoint558's
title accessors this closes `runtime_window.h` at20/20. Evidence:
checkpoint-559-c-window-state-focused-*-direct.*.

`c_api.game.component_members` demonstrates14 previously uncredited component-member routes in23
controls. Owner identity, property defaults/readback, change-only events, the canonical inverted
update-order comparison, explicit lifecycle calls despite disabled/invisible state, idempotent
disposal and one-shot registration ownership all pass. Its first comparison draft mistakenly used
the changed DrawOrder when the route compares UpdateOrder; the identical +/-4 runtime result under
both renderers identified a demo defect, which was corrected. `c_api.game.component_collection`
demonstrates the remaining eight meaningful collection/service routes in19 controls: indexed
mutation and bounds, absence semantics, reusable ownership, per-item clear events, named-service
lookup and idempotent removal. The existing components demo now credits its already-visible
drawable classification and graphics-device borrow. Both new Xvfb selections are byte-identical
with empty stderr; strict C99 plus metadata compilation pass. `runtime_components.h` is35/39
demonstrated, and its four fixed reflection-name size/copy helpers remain intentionally trivial and
uncredited because CNA's pure-C contract test already checks the generic buffer behavior. Evidence:
checkpoint-560-c-component-members-focused-*-direct.* and
checkpoint-561-c-component-collection-focused-*-direct.*.

`c_api.game.graphics_preferences` demonstrates24 manager/value routes in21 controls: validated
device-information cloning, single-manager ownership, callback-scoped device borrowing and all ten
preference getter/setter pairs with meaningful round-trip or refusal behavior.
`c_api.game.graphics_manager_lifecycle` demonstrates nine more routes in24 controls through all
manager event shapes, mutable prepared settings observed on the active device, ignored invalid
mutations, apply/recreate, an explicit draw bracket, idempotent disposal and one-shot destruction.
Its first draft treated a per-preparation mutable event write as a replacement for the stored manager
preference; the identical failure under both renderers exposed a demo defect, and the corrected
checks distinguish those states. Both corrected Xvfb selections are byte-identical with empty
stderr; strict C99 and metadata compilation pass. `runtime_graphics_manager.h` has semantic
dispositions for every route at33/38 demonstrated: four fixed reflection-name helpers remain
trivial/uncredited, and `toggle_full_screen` remains platform/manual because Xvfb accepted the call
but SDL asynchronously timed out and reverted the mode switch. Evidence:
checkpoint-562-c-graphics-preferences-focused-*-direct.* and
checkpoint-563-c-graphics-manager-lifecycle-focused-*-direct.*.

`c_api.content.manager_inspection` demonstrates17 content routes in18 controls: exact and atomic
resolved paths, normalized cache keys, built-in registration, service/device association, unordered
manifest entries with native/XNB details and the sorted XNB-reader usage summary.
`c_api.content.additional_loaders` demonstrates four further routes in15 controls using a real WAV,
a deterministic generated six-face DDS and a caller-defined CNJ object with bounded JSON, caching,
missing-type failure and post-Unload reload. The first cube assertion expected DXT1 after the content
loader had decoded it to Color; this was a demo defect. A removed ResourceContentManager assertion
showed only that the linked checkpoint library predates current CNA's explicit no-resource-family
placeholder and still loaded loose files. It is not current-source evidence or a new CNA issue, and
the route keeps its existing intentionally unsupported/deferred disposition. Both corrected
Xvfb/dummy-audio selections are byte-identical with empty stderr; strict C99 and metadata compilation
pass, and the cube asset regenerates byte-for-byte. `content.h` now has dispositions for every route
at31/32 demonstrated. Evidence:checkpoint-564-c-content-manager-inspection-focused-*-direct.* and
checkpoint-565-c-content-additional-loaders-focused-*-direct.*.

`c_api.storage.members` demonstrates25 storage routes in35 controls without a game. Exact/atomic
root and display names, all selector forms and synchronous completions, capacity, registration
ownership, container identity, unordered directory/file listings, explicit share selection, flush,
truncate/zero-extension and idempotent disposal/event delivery all pass. Actual device-set change
delivery is not induced, and no share enforcement is claimed because the canonical implementation
currently ignores those bits. The sandboxed attempts could not create the user storage root and are
environment-only non-evidence; direct runs in both renderer-labelled environments are byte-identical
with empty stderr. Strict C99 plus metadata compilation pass. `storage.h` now has dispositions for
all49 routes at47 demonstrated; the fixed container reflection-name pair remains trivial/uncredited.
Evidence:checkpoint-566-c-storage-members-focused-*-direct.*.

`c_api.content.reader_values` starts the public C content-reader audit with20 routes in33 controls.
A deterministic synthetic storage stream covers standalone identity, exact/atomic asset-name
copying, version/platform state, empty type-reader and shared-resource tables, a null object tag,
all seven value readers, collection/decoded-size limits, exact-byte capacity and truncation, the
exclusive stream borrow and one-shot reader/stream releases. Direct runs in both renderer-labelled
environments are byte-identical with empty stderr; no renderer is initialized. Strict C99 and
metadata compilation pass. `content_readers.h` is20/62 demonstrated; registry, placeholder,
caller-supplied, reflective and object-dictionary batches remain. Evidence:
checkpoint-567-c-content-reader-values-focused-*-direct.*.

`c_api.content.type_reader_registry` covers the next15 reader routes in37 controls. The isolated
process registry proves clearing, the idempotent currently empty unsupported-reader hook, missing
resolution, explicit placeholder identity/refusal, exact/atomic target naming, versions and
initialization. A caller-owned factory adds malformed/duplicate refusal, fresh instances,
callback-scoped reader validity and immediate invalidation, propagated read failure, survival of an
existing instance after unregistration, one-shot cleanup and canonical-name reuse. Both renderer-
labelled direct runs are byte-identical with empty stderr and initialize no renderer; strict C99 and
metadata compilation pass. `content_readers.h` is35/62 demonstrated; reflective builders and the
object-dictionary family remain. Evidence:
checkpoint-568-c-content-type-reader-registry-focused-*-direct.*.

`c_api.content.reflective_reader` covers the next11 reader routes in40 controls. It publishes exact
and atomic reflective/enum canonical names, rejects malformed declarations, snapshots the first
idempotent registration and maps every one of the14 public inline field kinds plus an enum and a
custom callback into caller-owned C storage. The callback proves scoped reader borrowing, builder
destruction leaves registrations live, and a second builder proves the shared/reference-shaped
form. Both renderer-labelled direct runs are byte-identical with empty stderr and initialize no
renderer; strict C99 and metadata compilation pass. `content_readers.h` reaches46/62 demonstrated;
the object-dictionary extension family remains. Evidence:
checkpoint-569-c-reflective-reader-focused-*-direct.*.

`c_api.content.object_dictionary` closes the final16 reader routes in47 controls. A deterministic
generated XNB provides16 sorted entries: every public fixed scalar kind, a string, packed Vector3
array, shared caller-created object and deliberately unknown UInt32 with a readable diagnostic type.
The demo covers runtime naming, indexed keys, entry descriptors, exact/atomic copies, wrong-kind
refusal, cached reader reuse, independent handles and stale-handle rejection. The initial run only
showed that the staged asset was absent from the old build tree; after exact deployment both Xvfb/
dummy-audio runs are byte-identical with empty stderr. This is a demo deployment prerequisite, not
a CNA issue. Strict C99, metadata compilation and byte-identical asset regeneration pass.
`content_readers.h` is complete at62/62. Evidence:
checkpoint-570-c-object-dictionary-focused-*-direct.*.

`c_api.media.video_asset` starts the public C video audit with19 routes in26 controls. The generated
160x120/15fps WMV is probed for correlated individual/aggregate metadata, positive duration and
device association; path and reflection text are exact/atomic and the versioned info structure
rejects malformed use. Metadata-only construction preserves declared values without opening its
missing path, accepts duration correction and defers track validation. The URI extension's literal-
path behavior and independent owned handles are explicit. Both Xvfb/dummy-audio runs are byte-
identical with empty stderr; strict C99 and metadata compilation pass. `video.h` is19/42
demonstrated; the VideoPlayer batch remains. Evidence:checkpoint-571-c-video-asset-focused-*-direct.*.

`c_api.media.video_player` closes the other23 `video.h` routes in31 controls against the real
generated WMV. It covers stopped defaults, configuration and clamping, exact/atomic type text,
active Video identity, decoded-frame descriptors and invalidation of each borrowed texture on the
next player call. A pause holds its clock across updates, Resume continues it and the paced run
observes the authored end wrap while state remains Playing. Stop resets state without resetting the
monotonic frame generation, replay advances it, and disposal makes playback routes reject access.
Both Xvfb/dummy-audio selections are byte-identical with empty stderr; strict C99 and metadata
compilation pass. `video.h` is42/42 demonstrated. Evidence:
checkpoint-572-c-video-player-focused-*-direct.*. Pixels, audible quality and visual inspection are
not claimed.

`c_api.audio.xact_engine` starts the public C XACT audit with17 routes in17 controls. It opens the
generated version46 settings through the ordinary and explicit look-ahead/renderer constructors,
then inspects CNA's single renderer through nonempty friendly/id text, the friendly ToString value,
atomic short-buffer refusal, bounds, equality and a stable process hash. The authored SpeedOfSound
global changes and clamps while cue-scoped Intensity remains inaccessible; Update pumps bookkeeping
and a live registration observes the one engine disposal before releasing independently. Both
Xvfb/dummy-audio selections are byte-identical with empty stderr; strict C99 and metadata
compilation pass. `xact.h` reaches17/62 demonstrated. Evidence:
checkpoint-573-c-xact-engine-focused-*-direct.*. No speaker output or scheduling effect is claimed.

`c_api.audio.xact_banks` adds28 XACT routes in35 controls. It loads the generated resident tone bank,
an actual prepared streaming music bank and a sound bank, with atomic missing resident paths and
initial ownership/state. Two independent category lookups expose exact/atomic Music text, value
equality and hashes. Live Music/SFX cues make category volume validation, selective Pause/Resume/
Stop and paused bank use observable; fire-and-forget and positioned fire-and-forget cues are then
retired through category Stop and Update. Engine/sound-bank parent release is refused while child
handles remain, and both bank disposal subscriptions fire once before separate release. A first
passing draft's redundant missing-streaming check emitted the canonical error log, so the corrected
workflow keeps the real streaming success path and empty stderr. Both Xvfb/dummy-audio selections
are byte-identical; strict C99 and metadata compilation pass. `xact.h` reaches45/62 demonstrated.
Evidence:checkpoint-574-c-xact-banks-focused-*-direct.*. Gain and speaker output are not measured.

`c_api.audio.xact_cue` closes the nine meaningful Cue routes in33 controls. Exact/atomic authored
names accompany two independent Intensity values whose mutation and clamping do not cross Cue
instances. Apply3D publishes exact Distance, OrientationAngle and neutral DopplerPitchScalar before
Play and updates Distance while paused. Versioned snapshots expose Pause/Resume/Immediate Stop, and
a generated release Cue distinguishes AsAuthored's live Stopping phase from Immediate interruption.
Its disposal subscription fires once and outlives the sender until separately released. Both Xvfb/
dummy-audio selections are byte-identical with empty stderr; strict C99 and metadata compilation
pass. `xact.h` has semantic dispositions for all62 routes at54 demonstrated; the eight fixed type-
name size/copy helpers for engine, wave bank, sound bank and Cue remain trivial/uncredited. Evidence:
checkpoint-575-c-xact-cue-focused-*-direct.*. No acoustic position, fade shape or audio quality is
claimed.

`c_api.net.session_properties` starts the public C networking audit with22 `net.h` routes in20
renderer-free controls. It initializes unavailable and one-sample QualityOfService structures with
version refusal, then observes that an ordinary handle failure produces no per-thread join error.
The canonical eight-slot session-property list demonstrates absent/present values, indexed writes
and bounds, IndexOf/Contains, every structural refusal, atomic shifted CopyTo, ordered live
enumeration, reset/end states and child-before-parent destruction. Both renderer-labelled direct
runs are byte-identical with empty stderr; strict C99 and metadata compilation pass. `net.h` reaches
22/51 demonstrated, leaving the coherent29-route packet buffer family. Evidence:
checkpoint-576-c-net-properties-focused-*-direct.*. No session, socket, join or throughput is
claimed.

`c_api.net.packet_buffers` closes all29 remaining `net.h` routes in20 renderer-free controls. One
ordered132-byte PacketWriter payload contains every public Color, vector, quaternion, matrix and
scalar value; exact and short atomic copies, seeking, overwrite without truncation and length/
position state are explicit. A separately owned PacketReader copies and restores every value,
supports position-independent copies and seeking, reports end-of-input, accepts an empty null reset
and rejects a nonempty null view. Both handles have one-shot ownership. The first run expected an
undersized compound Color read to leave the reader position unchanged, although the header promises
only I/O exhaustion; removing that assertion was a demo correction. Both renderer-labelled direct
runs pass byte-identically with empty stderr; strict C99 and metadata compilation pass. `net.h` is
51/51 demonstrated. Evidence:checkpoint-577-c-net-packets-focused-*-direct.*. No socket, wire
transport, peer interoperability, network byte order or project-built runner is claimed.

`c_api.net.gamer_machine_values` covers all26 non-event `net_gamers.h` routes in17 renderer-free
controls. A detached named gamer exposes all neutral role, voice, ready, talking, identifier,
round-trip and session defaults before exact transport-owned departure, host, identifier and timing
mutations. Noncanonical booleans and remote ready publication are atomic refusals. A separately
owned empty machine proves count/bounds/removal refusal, copy-on-set, independent copy-out and
one-shot ownership. Both renderer-labelled direct runs pass byte-identically with empty stderr;
strict C99 and metadata compilation pass. `net_gamers.h` reaches26/33 demonstrated. Evidence:
checkpoint-578-c-net-gamer-values-focused-*-direct.*. Populated machine borrowing, local-gamer
ready publication, successful removal and a project-built runner remain unclaimed.

`c_api.net.session_event_values` closes the final seven `net_gamers.h` routes in15 renderer-free
controls. Canonical and malformed game-transition prefixes, exact joined/left/old/new gamer
identities, atomic stale-handle refusal, all four session-end reasons, zero reserved storage and
leaderboard gamer/leaving validation are explicit. Both renderer-labelled direct runs pass byte-
identically with empty stderr; strict C99 and metadata compilation pass. `net_gamers.h` is33/33
demonstrated. Evidence:checkpoint-579-c-net-event-values-focused-*-direct.*. The demo constructs
event payloads and does not claim live dispatch or a project-built runner.

`c_api.net.available_sessions` starts `net_sessions.h` with all21 synthetic discovery-value and
collection routes in23 renderer-free controls. Exact roster/slot/type/port values, atomic host and
address text, measured QoS, independent properties and value equality are visible across three
descriptions. A two-entry collection snapshots released source handles, rejects invalid indexing,
remains readable across idempotent disposal and returns an owned entry that survives collection
release. Both renderer-labelled direct runs pass byte-identically with empty stderr; strict C99 and
metadata compilation pass. `net_sessions.h` reaches21/104 demonstrated. Evidence:
checkpoint-580-c-net-available-focused-*-direct.*. No discovery, join, socket, address reachability
or project-built runner is claimed.

`c_api.net.local_session_state` adds37 `net_sessions.h` routes in30 renderer-free controls. An
explicit signed-in gamer and copied properties create a single Local session whose four rosters,
host/local/find-by-id views and owned count agree, while borrowed views prevent early destruction.
Host/join policy, slot limits, simulated latency/loss, independent property replacement, exact type
text, zero traffic and process counts are visible. Ready publication/reset and queued StartGame/
EndGame transitions become observable through Update; idempotent disposal refuses later updates and
all ownership returns to zero instances. Both renderer-labelled direct runs pass byte-identically
with empty stderr; strict C99 and metadata compilation pass. Two supporting signed-gamer routes are
also newly demonstrated, and `net_sessions.h` reaches58/104. Evidence:
checkpoint-581-c-net-local-state-focused-*-direct.*. No socket, remote machine, measured throughput,
cross-machine propagation or project-built runner is claimed.

`c_api.net.session_events` adds14 `net_sessions.h` routes in19 renderer-free controls. All nine
instance event families and the global invite event obtain owned registrations. Existing/late
GamerJoined, remote GamerLeft, queued GameStarted/GameEnded and injected HostChanged/SessionEnded
callbacks carry exact versioned state; each callback gamer is usable only during the call. Every
registration releases once. Leaderboard-write and invite registrations remain silent under this
Local workflow, so their delivery is not claimed. Both renderer-labelled direct runs pass byte-
identically with empty stderr; strict C99 and metadata compilation pass. `net_sessions.h` reaches
72/104. Evidence:checkpoint-582-c-net-events-focused-*-direct.*. No socket, service event, remote
machine or project-built runner is claimed.

`c_api.net.local_gamer_packets` adds all16 LocalNetworkGamer routes from `net_sessions.h` in28
renderer-free controls. Standalone/session local identity, borrowed signed-in text, voice/party
validation, injected queue availability, plain/offset/PacketReader receives, scoped sender handles,
queue clearing and all six byte/PacketWriter send forms are explicit. Both renderer-labelled runs
are byte-identical with empty stderr:27 controls pass and one retains CNA-DEMOS-ISSUE-075, where
offset receive zeroes destination bytes outside the requested range. Strict C99 and metadata
compilation pass; the known-issue marker is present. Two supporting signed-gamer text routes are
newly demonstrated, and `net_sessions.h` reaches88/104. Evidence:
checkpoint-583-c-local-packets-focused-*-direct.*. No socket/wire delivery, voice audio,
party-service interaction or project-built runner is claimed.

## Focused584-C-network-session-construction

`c_api.net.session_construction` adds the six remaining Local constructor/admission routes from
`net_sessions.h` in28 renderer-free controls. One published signed-in gamer feeds the basic and
property overloads; configured limits, private slots and copied property values survive, and a
second signed-in gamer is admitted exactly once. The basic, properties and explicit-gamer async
adapters each call their completion once before returning an owned session, with the last carrying
both requested local gamers. All sessions are independently destroyed and process instance/action
counts return to zero. Both renderer-labelled direct runs are byte-identical with empty stderr;
strict C99 and metadata compilation pass. Two supporting signed-in collection routes are newly
demonstrated, staged metadata reaches2148/3205 and `net_sessions.h` reaches94/104. Evidence:
`checkpoint-584-c-session-construction-focused-*-direct.*`. No socket, service, concurrency,
remote peer or project-built runner is claimed.

## Focused585-C-network-session-discovery-join

`c_api.net.session_discovery_join` closes the final ten `net_sessions.h` routes in32 renderer-free
controls. Implicit and explicit LocalWithLeaderboards searches return empty owned collections in
both synchronous and asynchronous forms; each successful async call completes once before return.
A synthetic PlayerMatch description is insufficient without an authenticated live search, while a
synthetic Local description exercises both generic Join adapters and preserves its Local type and
joining-peer role. All four invitation overloads truthfully return NotSupported without a service
and never invoke completion. Every collection, description and session is released and process
instance/action counts return to zero. Both renderer-labelled direct runs are byte-identical with
empty stderr; strict C99 and metadata compilation pass. Staged metadata reaches2158/3205 and
`net_sessions.h` is104/104. Evidence:
`checkpoint-585-c-session-discovery-join-focused-*-direct.*`. No socket, authenticated directory,
accepted invite, address reachability, remote peer or project-built runner is claimed.

## Focused586-C-gamer-profile-identity

`c_api.gamer.profile_identity` starts `gamer_services.h` with25 routes in19 renderer-free controls.
An explicit offline SignedInGamer demonstrates exact/atomic gamertag buffers, malformed UTF-8
display-name refusal followed by copied mutation, the display-based text form, live state and a
full 64-bit caller tag. Synchronous and asynchronous owned profiles expose a canonical numeric
snapshot, motto/region count-copy contracts and an absent picture before independent release. The
runtime's unsupported global gamertag lookup and partner-token size/copy/async forms return
NotSupported without completion. Both renderer-labelled direct runs are byte-identical with empty
stderr; strict C99 and metadata compilation pass. Staged metadata reaches2183/3205 and
`gamer_services.h` reaches31/239. Evidence:`checkpoint-586-c-gamer-profile-focused-*-direct.*`.
No online account lookup, token, profile picture, game, renderer or project-built runner is claimed.

## Focused587-C-signed-in-gamer-state

`c_api.gamer.signed_in_state` adds19 `gamer_services.h` routes in29 renderer-free controls. Explicit
online-guest and offline-outside fixtures distinguish guest/service/player identities, mutable
party size, exact structured presence, the non-mutating free-text extension and bounded canonical
privileges. Offline friendship returns false and an owned empty collection. Publishing only player
two makes positional and player-index lookups visibly distinct, while IndexOf/Contains, absent
slots, upper bounds, independent borrowed handles and published retention are exact. Both
renderer-labelled direct runs are byte-identical with empty stderr; strict C99 and metadata
compilation pass. Staged metadata reaches2202/3205 and `gamer_services.h` reaches50/239. The passing
demo exposes CNA-DEMOS-ISSUE-076: the header promises an absent player-index lookup preserves
`out_gamer`, but the wrapper clears it. Evidence:`checkpoint-587-c-signed-in-state-focused-*-direct.*`.
No service friendship, presence publication, microphone, game, renderer or project-built runner is
claimed.

## Focused588-C-friend-gamer-collection

`c_api.gamer.friend_collection` adds19 `gamer_services.h` routes in20 renderer-free controls. Two
synthetic friend records preserve independent request, online, playing, away and busy predicates;
free-text presence uses exact atomic buffers. A retained two-entry collection demonstrates ordered
indexing, both bounds, IndexOf/Contains, short atomic and sufficient shifted copies, complete/reset
cursor traversal, removal including an absent no-op, re-addition and Clear. The cursor, collection
and both sources release independently and exactly once. Both renderer-labelled direct runs are
byte-identical with empty stderr; strict C99 and metadata compilation pass. Staged metadata reaches
2221/3205 and `gamer_services.h` reaches69/239. Evidence:
`checkpoint-588-c-friend-collection-focused-*-direct.*`. No live account friend graph, service
state, game, renderer or project-built runner is claimed.

## Focused589-C-achievement-values-and-collection

`c_api.gamer.achievement_collection` adds all29 achievement value and collection routes from
`gamer_services.h` in30 renderer-free controls. Synthetic values expose their versioned numeric
state, exact/atomic key/name/description buffers, empty how-to text, unavailable artwork and
complete value equality. A collection copies its inputs and returns owned index/key/copy results;
membership, both bounds, unknown-key output clearing, short atomic shifted CopyTo and every public
mutation are explicit. Both renderer-labelled direct runs are byte-identical with empty stderr;
29 controls pass and one retains CNA-DEMOS-ISSUE-077 because successful mutators coexist with a
true read-only property. The passing unknown-key control records CNA-DEMOS-ISSUE-078 because its
documented invalid argument is translated to Internal. Strict C99 and metadata compilation pass.
Staged metadata reaches2250/3205 and `gamer_services.h` reaches98/239. One-line build registrations
missing from the five preceding staged directories were restored before this checkpoint. Evidence:
`checkpoint-589-c-achievement-collection-focused-*-direct.*`. No catalog-backed picture/how-to
text, gamer service, game, renderer or project-built runner is claimed.

## Focused590-C-signed-in-achievements

`c_api.gamer.signed_in_achievements` adds the final four signed-in achievement routes in28
renderer-free controls. It selects a dedicated persistent application store, reads the deployed
three-entry offline catalog, verifies catalog score/name/how-to data, awards one key synchronously
and another through the exactly-once callback adapter, then reads through both synchronous and
callback routes. Re-award preserves both count and first-earned timestamp, and a fresh gamer object
for the same tag reloads both exact timestamps. Fresh SOFTWARE and OPENGLES3-labelled stores pass
byte-identically with empty stderr; a repeated SOFTWARE run over its populated store also produces
the same passing output. Strict C99 and metadata compilation pass. Staged metadata reaches
2254/3205 and `gamer_services.h` reaches102/239. The demo deliberately leaves EarnedOnline to the
existing C++ CNA-DEMOS-ISSUE-017 reproduction. Evidence:
`checkpoint-590-c-signed-in-achievements-focused-*-direct.*`. No online service, game, renderer,
notification appearance or project-built runner is claimed.

## Focused591-C-headless-guide-dialogs

`c_api.gamer.guide_dialogs` adds all19 non-rendering keyboard-input and message-box Guide routes in
23 renderer-free controls. Password input preserves exact title/description text, masks the
default display text, atomically refuses a short copy and remains genuinely pending until a
simulated cancellation invokes completion exactly once. Its empty canceled answer is repeatable
until reset. A three-button box starts on the requested focus, survives an invalid click, then
returns the exact simulated choice repeatedly. Active reset discards either dialog without a
callback; completed reset removes the C adapter's cached End result. Both renderer-labelled direct
runs pass byte-identically with empty stderr; strict C99 and metadata compilation pass. Staged
metadata reaches2273/3205 and `gamer_services.h` reaches121/239. Evidence:
`checkpoint-591-c-guide-dialogs-focused-*-direct.*`. The explicit headless simulation API is
validated; no dialog rendering, real keyboard/controller/mouse input, game, visible appearance or
project-built runner is claimed.

## Focused592-C-guide-dispatcher-lifecycle

`c_api.gamer.guide_dispatcher` adds20 Guide, dispatcher, canonical component and event-registration
routes in22 Xvfb controls. Before initialization both dispatcher pumps are harmless, visibility is
unavailable, an opaque 64-bit window value round-trips, notification identities validate
atomically and simulated trial state is stored without prematurely changing the trial result.
Adding the canonical GamerServicesComponent to a real game and running one frame installs the
dispatcher/Guide with the actual window handle. Subsequent updates latch trial state, async update
reports initialized work, duplicate initialization is refused and retired identity count remains
zero. The screen-saver flag toggles/restores through the platform. Three registrations attach,
remain silent with a dedicated empty local-profile store and release exactly once through the
shared route. Both SOFTWARE and OPENGLES3 Xvfb/dummy-audio runs pass byte-identically with empty
stderr; strict C99 and metadata compilation pass. Staged metadata reaches2293/3205 and
`gamer_services.h` reaches141/239. Evidence:
`checkpoint-592-c-guide-dispatcher-focused-*-direct.*`. No event delivery, visible Guide panel,
user interaction, pixel appearance or project-built runner is claimed.

## Focused593-C-guide-dialog-rendering

`c_api.gamer.guide_rendering` adds both remaining dialog-render routes to a real HiDef C game. Its
first frame opens and draws a password keyboard; the second cancels that request exactly once,
resets it and opens and draws a focused three-button alert. Direct back-buffer inspection finds
69,500 changed pixels over a464x151 region for the keyboard and100,004 changed pixels over a414x243
region for the message box under both SOFTWARE and OPENGLES3. Separate one- and two-frame Xvfb
captures give four800x480 screenshots; all were inspected and show readable labels, six masked
password glyphs, the alert icon and the requested middle-button focus. All17 keyboard-capture and
24 full-workflow controls pass with empty stderr; strict C99 and metadata compilation pass. Staged
metadata reaches2295/3205 and `gamer_services.h` reaches143/239. Evidence:
`checkpoint-593-c-guide-rendering-focused-*-direct.*`. Actual rendering and visible appearance are
validated; real keyboard, mouse and controller interaction and project-built execution remain
separate.

## Focused594-C-guide-sign-in-and-offline-social

`c_api.gamer.guide_social_offline` adds all14 remaining Guide screen routes in a real dispatcher
game with an isolated empty profile store. Invalid pane count and unavailable online-only mode
leave the Guide closed; ordinary offline sign-in opens and automatically draws the local-profile
picker. After20 frames, exact back-buffer samples prove its dimmed background, orange header and
panel; the SOFTWARE and OPENGLES3 screenshots are byte-identical and were inspected. Escape
simulation closes it. Valid text, recipient and target handles then reach every service-backed
social route: compose, friend request, friends, both invitations, gamer card, messages, party,
party sessions, review, players and achievements give their documented not-supported results,
while marketplace gives its distinct missing-profile/privilege invalid-state result. None leaves a
partial overlay. Both26-control Xvfb runs pass with empty stderr; strict C99 and metadata
compilation pass. The first one-frame OPENGLES3 probe had captured only the valid black opening
transition; using the complete20-frame interval corrected that demo timing defect. Staged metadata
reaches2309/3205 and `gamer_services.h` reaches157/239. Evidence:
`checkpoint-594-c-guide-social-offline-focused-*-direct.*`. Real service screens and user input
remain service/manual. `cna_guide_delay_notifications` remains intentionally uncredited because
its effect requires a delivered service invitation and exposes no public C readback.

## Focused595-C-game-defaults-and-properties

`c_api.gamer.game_defaults_properties` adds all30 GameDefaults and PropertyDictionary routes in24
renderer-free controls. The canonical initializer and an explicit offline gamer agree on every
default enum, flag, reserved byte and absent color, while a version-2 output is rejected atomically.
The empty dictionary exposes the canonical read-only/writable contradiction, then holds and
identifies all eight kinds C can write. Every typed getter round-trips its exact value; copied input
text, atomic output text, ascending key enumeration, kind replacement, invalid outcome and typed
getter refusals, removal, clearing and one-shot ownership are explicit. The stream-size extension
resets its outputs before refusing a non-stream; a positive stream remains service-only because C
has no stream setter. SOFTWARE- and OPENGLES3-labelled runs pass byte-identically with empty stderr;
strict C99 and metadata compilation pass. Staged metadata reaches2339/3205 and
`gamer_services.h` reaches187/239. Evidence:
`checkpoint-595-c-game-defaults-properties-focused-*-direct.*`. No game, renderer, service, stream
payload or project-built execution is claimed.

## Focused596-C-leaderboard-entry-values

`c_api.gamer.leaderboard_entries` adds all eight leaderboard-entry routes in23 renderer-free
controls. Gamer-backed, independently equal and anonymous values expose exact versioned
ranking/rating/presence state. The borrowed gamer's exact identity, the entry's live mutable columns
dictionary, columns-after-entry retention, equality across gamer/rating/ranking but not columns, a
single replaceable/detachable rating hook and stale ownership are all explicit. Both
renderer-labelled runs pass byte-identically with empty stderr; strict C99 and metadata compilation
pass. Staged metadata reaches2347/3205 and `gamer_services.h` reaches195/239. The anonymous-gamer
result also records CNA-DEMOS-ISSUE-079: the header promises to preserve the absent handle output,
while the generated wrapper deterministically clears it. Evidence:
`checkpoint-596-c-leaderboard-entries-focused-*-direct.*`. No game, renderer, service, persisted
leaderboard or project-built execution is claimed.

## Focused597-C-offline-leaderboard-readers

`c_api.gamer.leaderboard_readers` adds the remaining15 leaderboard identity/reader routes in30
renderer-free controls over separate isolated empty stores. All four canonical identities retain
their exact NUL-padded key and requested mode, with atomic invalid-key and structure-version
refusals. Explicit-page, pivot and gamer-restricted reads each return an owned empty reader through
both synchronous and exactly-once callback forms. Exact info/identity output, the empty entry bound,
synchronous and callback page-up toward zero, final-page page-down refusal and stale ownership are
explicit. SOFTWARE- and OPENGLES3-labelled runs pass byte-identically with empty stderr; strict C99
and metadata compilation pass. Staged metadata reaches2362/3205 and `gamer_services.h` reaches
210/239. Evidence:`checkpoint-597-c-leaderboard-readers-focused-*-direct.*`. Positive entry
lookup/page-down remains a persisted writer/service scenario because public C exposes no writer or
reader fixture; no game, renderer, service, persisted row or project-built execution is claimed.

## Focused598-C-avatar-descriptions

`c_api.gamer.avatar_descriptions` adds expression initialization and all eight avatar-description
routes in26 renderer-free controls. A foreign-format exact1021-byte value proves copy-on-create,
valid/unmeasured defaults, version refusal and atomic copy-out. Unconstrained and explicit
female/male random avatars are valid and measured, and the public bytes round-trip body type,
height and validity. An offline gamer read completes exactly once with its invalid default
description; its per-instance Changed registration attaches, stays correctly silent without a
change source and releases once. Both renderer-labelled runs pass byte-identically with empty
stderr; strict C99 and metadata compilation pass. Staged metadata reaches2371/3205 and
`gamer_services.h` reaches219/239. Evidence:
`checkpoint-598-c-avatar-descriptions-focused-*-direct.*`. Service-driven Changed delivery, game,
renderer, drawing and project-built execution remain separate.

## Focused599-C-avatar-animation-clips

`c_api.gamer.avatar_animations` adds all seven avatar-animation routes in17 renderer-free controls.
All31 defined presets expose71 local transforms and real lengths from2.5 to10 seconds. The Wave
right elbow changes at1.2 seconds; position clamps, non-looping advance, looping remainder,
canonical-boolean refusal, Celebration's keyed happy-to-laughing face, version/bone bounds and
one-shot ownership are explicit. Both renderer-labelled runs pass byte-identically with empty
stderr; strict C99 and metadata compilation pass. Staged metadata reaches2378/3205 and
`gamer_services.h` reaches226/239. CNA-DEMOS-ISSUE-080 records that the C info comment still says
built-in presets have no timeline and zero length, contrary to the runtime and canonical C++ API.
Evidence:`checkpoint-599-c-avatar-animations-focused-*-direct.*`. No game, renderer, composed
skeleton, pixels or project-built execution is claimed.

## Focused600-C-avatar-renderer-and-skeleton

`c_api.gamer.avatar_renderer` adds all11 avatar-renderer routes in a real C game with an installed
`GamerServicesComponent`. Its43 controls cover canonical-boolean creation refusal, description
retention, versioned Loading/Ready state, default and authored world/view/projection, optional
outputs, exact lighting and atomic non-finite refusal, all71 canonical parent links, finite bind
poses, composed human-scale head height, bounds and one-shot ownership. A Wave clip is drawn once
through `draw_animation`, then through caller-supplied71 bones and an explicit smile/wink expression;
both paths pass substantial two-dimensional back-buffer checks. SOFTWARE and OPENGLES3 runs pass
with byte-identical normalized output and empty stderr. Their inspected800x480 screenshots show a
complete waving avatar on the clear background; strict C99 and metadata compilation pass. Staged
metadata reaches2389/3205 and `gamer_services.h` reaches237/239. Its remaining Guide-delay and
physical-headset routes are intentionally service/manual, so all239 routes now have semantic
dispositions. Evidence:`checkpoint-600-c-avatar-renderer-focused-*-direct.*`. No service avatar,
physical microphone or project-built execution is claimed.

## Focused601-C-media-library-collection-lifetime

`c_api.media.library_collections` adds33 `media_library.h` routes in34 controls against separate
isolated empty Music/Pictures roots. Default and explicit Local Device construction report exact
source identity and atomic UTF-8 copy semantics. All five music collections publish live handles,
nonnegative counts and atomic upper-bound refusals. Library disposal is idempotent without erasing
the catalog; after its public handle is released, every child still retains it. Album, artist,
genre, playlist and song collection disposal then empties each collection, records disposal and
remains idempotent, with stale ownership checked explicitly. SOFTWARE- and OPENGLES3-labelled
Xvfb runs pass byte-identically with empty stderr; strict C99 and metadata compilation pass. Staged
metadata reaches2422/3205 and `media_library.h` reaches33/148. Evidence:
`checkpoint-601-c-media-library-collections-focused-*-direct.*`. Positive entity relationships,
album art, pictures and save/import behavior remain for controlled fixtures; no host content,
window, audio, image or project-built execution is claimed.

## Focused602-C-media-library-music-values

`c_api.media.library_music_values` adds42 `media_library.h` routes in33 controls. Before any CNA
platform call it creates private Linux/XDG Music/Pictures roots, copies three original tagged FLAC
tones through the public title reader and writes a two-entry M3U. The resulting exact catalog has
three songs, two same-title albums kept distinct by artist, two artists, one shared genre and one
playlist. Atomic entity strings, track tags, equality/hash, every album/artist/genre/playlist
collection relationship, Song back-references, reversed playlist order, exact duration sums,
missing-art state and shared idempotent disposal all pass after top-level collection and library
handles are released. Every retained handle then releases successfully. SOFTWARE- and
OPENGLES3-labelled Xvfb runs pass byte-identically with empty stderr; strict C99 and metadata
compilation pass. Staged metadata reaches2464/3205 and `media_library.h` reaches75/148. Evidence:
`checkpoint-602-c-media-library-values-focused-*-direct.*`. Album image bytes and the picture/save
subsystem remain; no playback, audible quality, host media or project-built execution is claimed.

## Focused603-C-media-library-album-art

`c_api.media.library_album_art` adds the four album image-transfer routes in15 controls. A private
Linux/XDG fixture holds two tagged albums and gives exactly one an original1666-byte PNG cover.
The covered album returns the exact encoded bytes, both copy routes preserve caller storage on a
short buffer, and the bare album refuses both size queries atomically with `INVALID_STATE`. The
thumbnail is a distinct programmatically parsed128x64 PNG with the source's2:1 aspect ratio.
SOFTWARE- and OPENGLES3-labelled Xvfb runs pass byte-identically with empty stderr; strict C99 and
metadata compilation pass. Staged metadata reaches2468/3205 and `media_library.h` reaches79/148.
Evidence:`checkpoint-603-c-media-library-album-art-focused-*-direct.*`. The picture hierarchy,
tokens, collections and both save/import routes remain; no rendering, screenshot, visual
inspection or project-built execution is claimed.

## Focused604-C-media-library-pictures-and-imports

`c_api.media.library_pictures` adds all43 meaningful picture, picture-album, collection, token and
save routes in57 controls. A private Linux/XDG tree contains a640x320 panorama under Family and a
64x64 checker under Vacation/Day 2. Exact atomic names/tokens, dimensions, timestamps, hierarchy,
ordered membership, bounds, equality/hashes, byte-exact images, a parsed128x64 thumbnail and token
lookup all pass. Saving that thumbnail from memory and the checker from a rewound public storage
stream grows all live views and preserves both encoded payloads. Shared disposal, collection
emptying, child-retained lifetime and complete cleanup are explicit. SOFTWARE- and OPENGLES3-
labelled Xvfb runs pass byte-identically with empty stderr; strict C99 and metadata compilation
pass. Staged metadata reaches2511/3205 and `media_library.h` reaches122/148. Its other26 routes are
fixed reflection type-name pairs and intentionally trivial/uncredited, completing semantic
dispositions for all148 routes. Evidence:
`checkpoint-604-c-media-library-pictures-focused-*-direct.*`. No rendering, screenshot, visual
inspection, host media or project-built execution is claimed.

## Focused605-C-CNB-foundations

`c_api.content.cnb_foundations` opens the previously untouched `cnb.h` audit with26 pure routes in
25 controls. It fixes the CNB v1 layout and magic, little-endian and sanitized chunk identities,
mandatory flags, exact built-in/reserved/custom type names, deterministic custom IDs, raw logical-
name verdicts, versioned allocation ceilings and atomic checked arithmetic. One-shot, split and
portable CRC-32C agree with the published iSCSI check vector. Stored compression round-trips exact
bytes, unimplemented codecs return `NOT_SUPPORTED`, and the available Zstandard implementation
clamps effort, genuinely shrinks and restores a repetitive payload. Both renderer-labelled direct
runs are byte-identical with empty stderr; strict C99 and metadata compilation pass. Staged
metadata reaches2537/3205 and `cnb.h` reaches26/272. Evidence:
`checkpoint-605-c-cnb-foundations-focused-*-direct.*`. No game, renderer, filesystem, compiled
asset or project-built execution is claimed. Bounded reader and byte-writer serialization remain
the next foundation batch.

## Focused606-C-CNB-bounded-byte-I/O

`c_api.content.cnb_byte_io` adds38 `cnb.h` routes in27 renderer-free controls. Its writer builds an
exact52-byte little-endian payload with every integer and floating-point primitive, a length-
prefixed UTF-8 string, a count, raw bytes and zero padding. Initial-byte copying, exact size/copy
queries and ownership-transferring take behavior are explicit, including atomic short-buffer
refusal. A bounded reader owns its input and context copies, reports exact cursor state, restores
all values, retains the last decoded string, preserves its cursor on a short raw destination and
distinguishes trailing data, exact exhaustion, EOF and explicit schema failure. A released handle
refuses stale access, and the UTF-8 predicate distinguishes valid multibyte text from malformed
bytes. Both renderer-labelled direct runs pass byte-identically with empty stderr; strict C99,
metadata compilation, diff checks and validation self-tests pass. Staged metadata reaches2575/3205
and `cnb.h` reaches64/272. Evidence:
`checkpoint-606-c-cnb-byte-io-focused-*-direct.*`. No game, renderer, filesystem, complete CNB
document or project-built execution is claimed. Document/writer round-trip behavior is next.

## Focused607-C-CNB-document-and-writer

`c_api.content.cnb_document` adds all40 container document/writer routes in51 renderer-free
controls. Its deterministic Curve writer owns four copied schema chunks, canonical metadata and a
cleared/replaced external reference; it enforces parse-compatible limits, selects Zstandard when
available, proves exact repeated builds, preserves a short destination and writes one temporary
file. Memory and file parsers recover six table entries with exact origin, versions, asset
identity, chunk records and logical bytes. Multi/single/required lookup, mandatory-type rules,
metadata and reference records/strings, asset requirements, limits, stale handles and borrowed-
reader lifetime all pass. Truncated, garbage and absent-file inputs publish no partial documents.
Both renderer-labelled direct runs pass byte-identically with empty stderr; strict C99, metadata
compilation, diff checks and validation self-tests pass. Staged metadata reaches2615/3205 and
`cnb.h` reaches104/272. Evidence:`checkpoint-607-c-cnb-document-focused-*-direct.*`. No game,
renderer, decoded asset or project-built execution is claimed. Texture format, representation and
codec families are next.

## Focused608-C-CNB-texture-data

`c_api.content.cnb_texture_data` adds20 pure texture-format and neutral texture-data routes in26
controls. Frozen wire identities, known and block-compressed ranges, exact diagnostic names, unit
sizes, volume and block-rounded level sizes and all27 two-way runtime SurfaceFormat mappings pass.
A copied2x2 Rgba8 value plus a general8x4x2 three-mip volume exercise versioned shape, clamped mip
dimensions, ordered Rgba8/BC7 representations, copied level payloads, atomic short copies, nested
bounds, synchronous preference callbacks, ordinary no-match results and stale ownership. Both
renderer-labelled direct runs pass byte-identically with empty stderr; strict C99, metadata
compilation, diff checks and validation self-tests pass. Staged metadata reaches2635/3205 and
`cnb.h` reaches124/272. Evidence:`checkpoint-608-c-cnb-texture-data-focused-*-direct.*`. No
document, graphics device, GPU texture, renderer, pixels or project-built execution is claimed.
The eight standalone and embedded texture codec routes are next.

## Focused609-C-CNB-texture-codecs

`c_api.content.cnb_texture_codecs` adds the final eight texture routes in22 renderer-free controls.
A three-mip2D texture, a six-face/two-mip cube and a depth-aware two-mip volume round-trip through
complete encoded and parsed CNB documents with exact shape, ordering, formats and payload bytes.
The 2D sizing/copy path is atomic, wrong asset decoders publish no handles, cube shape rules are
enforced and schema version one refuses a valid BC7 value it cannot encode. A SpriteFont document
then embeds the canonical 2D texture chunks and reads its atlas back exactly while the standalone
decoder still refuses the non-texture asset identity. Both renderer-labelled direct runs pass
byte-identically with empty stderr; strict C99, metadata compilation, diff checks and validation
self-tests pass. Staged metadata reaches2643/3205 and `cnb.h` reaches132/272. Evidence:
`checkpoint-609-c-cnb-texture-codecs-focused-*-direct.*`. No graphics device, GPU texture,
renderer, filesystem, displayed pixels or project-built execution is claimed. Model data is next.

## Focused610-C-CNB-model-core

`c_api.content.cnb_model_core` adds30 model-data routes in26 renderer-free controls. A single
neutral model owns two parented bones, a triangle part with copied vertex/index bytes, an external
effect name and complete versioned part/material values. Exact atomic strings and byte transfers,
numeric replacement, live aggregate counts, canonical booleans, nested bounds and stale ownership
all pass. Eight independently named material texture slots are explicitly kept separate from the
seven importer-state slots, where every coordinate set, UV transform and sampler round-trips.
Both renderer-labelled direct runs pass byte-identically with empty stderr; strict C99, metadata
compilation, diff checks and validation self-tests pass. Staged metadata reaches2673/3205 and
`cnb.h` reaches162/272. Evidence:`checkpoint-610-c-cnb-model-core-focused-*-direct.*`. No document
codec, ContentManager, graphics device, renderer or draw call is claimed. Morph targets and their
weight timeline are next.

## Focused611-C-CNB-model-morph-data

`c_api.content.cnb_model_morph` adds all12 morph routes in20 renderer-free controls. One two-vertex
part owns two indexed targets with distinct copied position/normal delta streams, two default
weights and a cubic timeline key with separate weight, in-tangent and out-tangent streams. Atomic
short transfers, empty streams, selector bounds, live derived counts and caller-buffer ownership
pass. Replacing the morph flags preserves every independently built stream, while idempotent clear
removes the entire nested graph and makes subsequent stream access fail. Both renderer-labelled
direct runs pass byte-identically with empty stderr; strict C99, metadata compilation, diff checks
and validation self-tests pass. Staged metadata reaches2685/3205 and `cnb.h` reaches174/272.
Evidence:`checkpoint-611-c-cnb-model-morph-focused-*-direct.*`. No document codec, mesh
deformation, animation player, graphics device, renderer or draw call is claimed. Mesh, skeleton,
animation, lights and the model codec are next.

## Focused612-C-CNB-model-scene-and-codec

`c_api.content.cnb_model_scene` adds the remaining20 neutral model and codec routes in28 renderer-
free controls. Its graph owns a child-attached mesh and ordered part list, a two-joint skeleton with
distinct bind/inverse/root-prefix matrices, a copied two-key scene-node animation and one exact
directional light. Atomic names, hierarchy/keyframe transfers, derived shape and idempotent
skeleton removal pass. A complete Model document sizing/build/parse/decode round-trip preserves
every nested relationship and byte, while a short encoder destination remains untouched. Both
renderer-labelled direct runs pass byte-identically with empty stderr; strict C99, metadata
compilation, diff checks and validation self-tests pass. Staged metadata reaches2705/3205 and
`cnb.h` reaches194/272, completing all62 neutral model-data/model-codec routes. Evidence:
`checkpoint-612-c-cnb-model-scene-focused-*-direct.*`. No CNJ file, ContentManager, graphics
device, renderer, animation playback or draw call is claimed. The CNJ compiler result is next.

## Focused613-C-CNB-CNJ-model-compiler-result

`c_api.content.cnb_model_from_cnj` adds all nine filesystem compiler-result routes in18 renderer-
free controls. A private CNJ Model and two binary sidecars establish exact authored absorbed-file
order, sorted normalized external references, atomic text transfer and both Count boundaries. The
compiled graph derives four vertices, six indices, two triangles, one synthetic mesh bone and both
external material names. Model transfer succeeds once, keeps both dependency lists readable and
outlives destruction of the result; repeated take, stale access and a missing document are exact
refusals without partial handles. Both renderer-labelled direct runs are byte-identical with empty
stderr; strict C99, metadata compilation, diff checks and validation self-tests pass. Staged
metadata reaches2714/3205 and `cnb.h` reaches203/272. Evidence:
`checkpoint-613-c-cnb-model-from-cnj-focused-*-direct.*`. No ContentManager, graphics device,
renderer, draw call or project-built execution is claimed. Neutral SpriteFont data and its codec
are next.

## Focused614-C-CNB-SpriteFont-data-and-codec

`c_api.content.cnb_sprite_font_data` adds all11 neutral SpriteFont routes in24 renderer-free
controls. An empty description receives exact metrics, two complete glyph rows and a copied8x4
Rgba8 atlas. Derived glyph counts, canonical booleans, invalid-row rollback, Count bounds and
independent atlas copies pass. The encoder rejects the intentionally descending character map;
replacing those same rows in strict order enables atomic sizing, short-buffer refusal and two
byte-identical complete images. Parse/decode restores every metric, glyph field and atlas byte in
an independently owned font. Both renderer-labelled direct runs are byte-identical with empty
stderr; strict C99, metadata compilation, diff checks and validation self-tests pass. Staged
metadata reaches2725/3205 and `cnb.h` reaches214/272. Evidence:
`checkpoint-614-c-cnb-sprite-font-data-focused-*-direct.*`. No ContentManager, graphics device,
renderer, text layout, draw call or project-built execution is claimed. Audio formats and neutral
SoundEffect data are next.

## Focused615-C-CNB-SoundEffect-data-and-codec

`c_api.content.cnb_sound_effect_data` adds all nine audio-format and neutral SoundEffect routes in
17 renderer-free controls. Frozen format values, fixed frame sizes and exact atomic names cover
all six identities plus an unknown value. A copied stereo PCM8 payload with an interior loop keeps
its bytes after caller mutation, encodes twice into an identical schema-two document and decodes
with exact shape and samples after the original is released. Sample/document short buffers are
atomic, stale access fails and the encoder catches both a stored byte-count mismatch and a named
packet format without a codec. Both renderer-labelled direct runs are byte-identical with empty
stderr; strict C99, metadata compilation, diff checks and validation self-tests pass. Staged
metadata reaches2734/3205 and `cnb.h` reaches223/272. Evidence:
`checkpoint-615-c-cnb-sound-effect-data-focused-*-direct.*`. No SoundEffect runtime object, audio
device, playback, audible output or project-built execution is claimed. Song and Video metadata
documents are next.

## Focused616-C-CNB-Song-and-Video-metadata

`c_api.content.cnb_media_metadata` adds all ten Song/Video routes in20 renderer-free controls. A
named195-second Song and42-second1280x720 Video produce complete schema-one metadata documents,
while their media payloads remain one external logical reference each. Specialized stream reads
match the exact XREF table, text/document short outputs are atomic, repeated Song encoding is
identical and an empty display name with unknown duration stays valid. Empty Song references,
oversized video, zero frame rate and unknown soundtrack identity fail. Both schema decoders reject
the other asset type and stale document access preserves output. Both renderer-labelled direct
runs are byte-identical with empty stderr; strict C99, metadata compilation, diff checks and
validation self-tests pass. Staged metadata reaches2744/3205 and `cnb.h` reaches233/272. Evidence:
`checkpoint-616-c-cnb-media-metadata-focused-*-direct.*`. No media file, ContentManager, decoder,
audio/video device, playback, rendering or project-built execution is claimed. Curve and
AnimationClip shared-runtime codecs are next.

## Focused617-C-CNB-Curve-AnimationClip-and-keyframes

`c_api.content.cnb_curve_animation` adds all11 Curve, AnimationClip and shared byte-format routes
in29 renderer-free controls. A two-key Curve with distinct loops, tangents and continuities encodes
and decodes into the ordinary runtime handle. A two-track scene-node clip owns copied borrowed
keys, reports exact shape/target space, rejects Count, copies rows atomically and remains live after
its document is released. The common keyframe writer emits exactly the published48 bytes, the
reader restores every component and rejects truncation, and both write/read sides guard times no
`TimeSpan` can represent without changing prior state or output. Both renderer-labelled direct
runs are byte-identical with empty stderr; strict C99, metadata compilation, diff checks and
validation self-tests pass. Staged metadata reaches2755/3205 and `cnb.h` reaches244/272. Evidence:
`checkpoint-617-c-cnb-curve-animation-focused-*-direct.*`. No model, animation player, graphics
device, renderer, drawing or project-built execution is claimed. The loader registry is next.

## Focused618-C-CNB-loader-registry

`c_api.content.cnb_loader_registry` adds all11 process-wide registry and owned-loader routes in21
controls. An actual31-bit FNV collision pair distinguishes idempotent duplicate registration from
collision refusal. A custom document resolves and invokes its loader with exact asset text and
live callback-scoped document/manager handles; both borrowed handles expire at return. Numeric and
document lookup produce independent copies which remain callable after the registration is
removed. Built-in registration is idempotent, and invoking its Curve loader returns
`NOT_SUPPORTED` rather than leaking a C++ object through C. Both Xvfb direct runs pass byte-
identically with empty stderr; strict C99, metadata compilation, diff checks and validation self-
tests pass. Staged metadata reaches2766/3205 and `cnb.h` reaches255/272. Evidence:
`checkpoint-618-c-cnb-loader-registry-focused-*-direct.*`. A caller-owned graphics device exists
only to construct the ContentManager required by the loader signature; no pixels, drawing or
project-built execution is claimed. Image, DDS and WAV import front ends are next.

## Focused619-C-CNB-image-DDS-and-WAV-importers

`c_api.content.cnb_importers` adds all five source import/decode routes in23 renderer-free
controls. A private one-pixel BMP proves exact RGBA8 decoding and both sides of explicit color-key
matching. Memory and file front ends produce identical six-face RGBA8 data from a complete DXT1
cube and identical copied PCM8 samples, shape and inclusive `smpl` loop from a controlled WAV.
Caller-buffer mutation proves ownership; unknown image options and missing, malformed, non-cube,
compressed or truncated sources fail without partial handles. Both renderer-labelled direct runs
are byte-identical with empty stderr; strict C99, metadata compilation, diff checks and validation
self-tests pass. Staged metadata reaches2771/3205 and `cnb.h` reaches260/272. Evidence:
`checkpoint-619-c-cnb-importers-focused-*-direct.*`. No ContentManager, graphics/audio device,
rendering, playback, audible/visual quality or project-built execution is claimed. The generic
CNJ-to-CNB result family is next.

## Focused620-C-generic-CNJ-to-CNB-result

`c_api.content.cnb_compile_cnj` adds the final12 generic compiler-result routes in22 renderer-free
controls. A private Model CNJ and two binary sidecars produce deterministic complete CNB bytes,
the exact canonical asset identity/name, a CNJ-first authored absorbed-file list and two sorted
external dependencies. Text and byte transfers are atomic, Count bounds preserve output, copied
bytes outlive the result and parse/decode into the expected derived quad graph. Stale access plus
missing and unsupported documents fail without partial result handles. Both renderer-labelled
direct runs are byte-identical with empty stderr; strict C99, metadata compilation, diff checks and
validation self-tests pass. Staged metadata reaches2783/3205 and `cnb.h` reaches272/272, completing
the header's semantic route audit. Evidence:`checkpoint-620-c-cnb-compile-cnj-focused-*-direct.*`.
No ContentManager, graphics device, renderer, animation playback, draw call, visual result or
project-built execution is claimed. The entirely uncovered `models.h` audit begins with its
renderer-independent ModelBone graph and collection routes.

## Focused621-C-ModelBone-graph-and-collections

`c_api.graphics.model_bones` adds the first17 `models.h` routes in25 renderer-free controls.
Default and named bones preserve empty/copied UTF-8 text, signed indices and exact local matrices.
A child collection acquired before mutation stays live across a two-level retained hierarchy;
indexed and named lookup, membership and parent views identify the same nodes, enforce Count
bounds and retain them beyond their original handles. Self-parenting, ancestor cycles, malformed
UTF-8 and stale access are refused atomically. The first draft expected malformed UTF-8 to return
`INVALID_ARGUMENT`; the string boundary's correct `ENCODING` result made that a demo bug. Both
renderer-labelled direct runs are byte-identical with empty stderr; strict C99, metadata
compilation, diff checks and validation self-tests pass. Staged metadata reaches2800/3205 and
`models.h` reaches17/216. Evidence:`checkpoint-621-c-model-bones-focused-*-direct.*`. No Model,
mesh, ContentManager, graphics device, renderer, animation player, draw call, visual result or
project-built execution is claimed. Standalone ModelMeshPart values and collections are next.

## Focused622-C-ModelMeshPart-values-samplers-and-collections

`c_api.graphics.model_mesh_parts` adds21 more `models.h` routes in23 renderer-free controls.
Default and explicit parts retain all four signed scalar fields across their complete ranges, the
opaque64-bit tag, every primitive topology and all seven independent material/specular sampler
slots. Invalid topology, sampler version/slot and collection inputs are atomic. An ordered
collection snapshot owns the exact live parts independently of its caller array and original
handles; stable aliases share mutation and survive collection release. Both renderer-labelled
direct runs are byte-identical with empty stderr; strict C99, metadata compilation, diff checks and
validation self-tests pass. Staged metadata reaches2821/3205 and `models.h` reaches38/216.
Evidence:`checkpoint-622-c-model-mesh-parts-focused-*-direct.*`. No effect, buffer, Model, mesh,
graphics device, renderer, draw call, visual result or project-built execution is claimed.
ModelMeshPart resources plus ModelMesh drawing and collections are next.

## Focused623-C-ModelMesh-resources-collections-and-drawing

`c_api.graphics.model_meshes` adds32 more `models.h` routes in33 controls. A real drawable part
retains its exact BasicEffect, VertexBuffer and IndexBuffer and refuses premature resource
destruction. Named and empty meshes preserve bounds, tags, parents and text; their live part/effect
views, duplicate effect mutations and an ordered retained mesh collection establish exact identity,
lookup, Count bounds and stable alias lifetime. A cull-free indexed draw produces the exact green
center pixel, while an empty mesh leaves the black clear pixel unchanged. The audit also exposes
CNA-DEMOS-ISSUE-081: an effect attached before mesh construction is missing from `Effects` until
the public setter detaches and reattaches it. Both SOFTWARE and OPENGLES3 Xvfb direct runs pass
33/33 with byte-identical stdout and empty stderr; strict C99 and metadata compilation, diff checks
and validation self-tests pass. Staged metadata reaches2853/3205 and `models.h` reaches70/216.
Evidence:`checkpoint-623-c-model-meshes-focused-*-direct.*`. No ContentManager, imported asset,
top-level Model, screenshot, visual-quality assessment or project-built execution is claimed.
Top-level Model construction, transforms, owned resources, diagnostics and drawing are next.

## Focused624-C-Model-ownership-transforms-diagnostics-and-drawing

`c_api.graphics.models` adds27 more `models.h` routes in39 controls. Default, simple and explicit-
parent constructors expose empty or retained bone/mesh collections, explicit root and mesh-parent
identity, full-width tags and one-shot owned-resource replacement/destruction callbacks. Two local
bone transforms copy atomically and compose exactly; a caller-built glTF report plus warning owns
all scalars, code, subject, message and indexed details and derives its loss totals. A procedural
triangle then proves `Model.Draw` composes the child's absolute transform with world: its exact
green center moves to(700,120) and the old(400,240) center stays black. The first probe expected
world alone; CNA's documented/source-confirmed parent composition made that a demo error. Both
SOFTWARE and OPENGLES3 Xvfb direct runs pass39/39 with byte-identical stdout and empty stderr;
strict C99 and metadata compilation, diff checks and validation self-tests pass. Staged metadata
reaches2880/3205 and `models.h` reaches97/216. Evidence:
`checkpoint-624-c-models-focused-*-direct.*`. No ContentManager, imported glTF, animation player,
screenshot, visual-quality assessment or project-built execution is claimed. Morph-target data,
weight tracks, CPU blending and ModelMeshPart attachment are next.

## Focused625-C-morph-target-data-tracks-and-part-upload

`c_api.graphics.morph_targets` adds all26 morph-related `models.h` routes in29 controls. One
deep-copied32-byte base vertex, two position targets, an optional normal stream, tangent deltas,
current weights and complete track keys survive caller-buffer overwrite. Linear, step and cubic
evaluation are distinguished numerically; CPU blend produces the exact combined position,
renormalized normal and untouched UV bytes. Flat-normal flags and triangle indices enforce their
atomic contracts. A ModelMeshPart retains the data beyond its original handle, then weights
(0.5,0.25) upload the independently predicted position(0.5,0,0.5), confirmed by direct
VertexBuffer readback. Both SOFTWARE and OPENGLES3 Xvfb direct runs pass29/29 with byte-identical
stdout and empty stderr; strict C99 and metadata compilation, diff checks and validation self-tests
pass. Staged metadata reaches2906/3205 and `models.h` reaches123/216. Evidence:
`checkpoint-625-c-morph-targets-focused-*-direct.*`. No mesh, Model, ContentManager, animation
player, draw call, screenshot, visual result or project-built execution is claimed. The
SkinnedModelEXT skeleton, clips, transforms, part ownership and move routes are next.

## Focused626-C-SkinnedModel-skeleton-clips-moves-and-parts

`c_api.graphics.skinned_models` adds26 `models.h` routes in28 controls. A copied two-bone skeleton,
bind/inverse-bind arrays and named clip survive source mutation; sorted discovery and atomic track
copies preserve exact data. Half-time, looped and clamped transform evaluation distinguish their
results, while malformed hierarchy replacement is atomic. Move construction/assignment empties
sources, and part transfer requires a matching skeleton. One same-device part then retains real
vertex/index buffers and an optional Texture2D; exact-name donor replacement releases the first
set, owns the new set and permits complete removal. Both SOFTWARE and OPENGLES3 Xvfb direct runs
pass28/28 with byte-identical stdout and empty stderr; strict C99 and metadata compilation, diff
checks and validation self-tests pass. Staged metadata reaches2932/3205 and `models.h` reaches
149/216. Evidence:`checkpoint-626-c-skinned-models-focused-*-direct.*`. No skinning shader,
texture sampling, draw call, screenshot, visual result or project-built execution is claimed.
SkinningData values plus AnimationPlayer local/world/skin transform playback are next.

## Focused627-C-SkinningData-and-AnimationPlayer

`c_api.graphics.skinning_data_player` adds32 `models.h` routes in28 controls. Deep-copied hierarchy,
bind/inverse-bind/root-prefix arrays and sorted clips survive caller mutation and keep exact atomic
copy contracts. Clip target space and declared rig-root metadata remain explicit. AnimationPlayer
starts in bind pose, selects a retained named clip, composes half-time local/world/skin transforms,
distinguishes absolute clamp from loop, rejects invalid updates without changing position and keeps
playing after the source SkinningData handle is released. Both SOFTWARE and OPENGLES3 renderer-
labelled direct runs pass28/28 with byte-identical stdout and empty stderr; strict C99 and metadata
compilation, diff checks and validation self-tests pass. Staged metadata reaches2964/3205 and
`models.h` reaches181/216. Evidence:
`checkpoint-627-c-skinning-data-player-focused-*-direct.*`. No Model, ContentManager, graphics
device, renderer, draw call, screenshot, visual result or project-built execution is claimed. The
renderer-independent ModelAnimationsEXT collection and direct Model pose helpers are next.

## Focused628-C-ModelAnimations-and-direct-posing

`c_api.graphics.model_animations` adds11 `models.h` routes in29 controls. Deep-copied named clips
preserve sorted exact metadata, reject duplicates and keep target index space explicit. A real
two-bone Model refuses joint-palette posing, applies the same clip in scene-node space at half time,
keeps the untracked child local transform intact, composes the hierarchy, clamps at duration and
rejects non-finite time without mutation. A separate two-bone SkinningData bind pose uploads to one
real retained SkinnedEffect and reports exactly one posed effect. Both SOFTWARE and OPENGLES3 Xvfb
direct runs pass29/29 with byte-identical stdout and empty stderr; strict C99 and metadata
compilation, diff checks and validation self-tests pass. Staged metadata reaches2975/3205 and
`models.h` reaches192/216. Evidence:`checkpoint-628-c-model-animations-focused-*-direct.*`. No
vertex/index buffer, texture, draw call, screenshot, visual result or project-built execution is
claimed. Ancillary Model cameras, skins, bounds, material variants and infinite projection are next.

## Focused629-C-Model-camera-skin-bounds-and-infinite-projection

`c_api.graphics.model_scene_metadata` adds16 `models.h` routes in28 controls. An authored camera
deep-copies every scene-node, projection, placement, scalar and text value. A one-bone skeleton maps
to a real Model mesh index, survives source release through a new owned handle and distinguishes a
legal unrigged skin. The aggregate sphere places local center(1,2,3), radius2 through root
translation x=10. The first check expected zero aspect ratio to be refused; source review showed
both native XNA perspective factories accept it, so the expectation was corrected as a demo error.
Both SOFTWARE and OPENGLES3 Xvfb direct runs pass28/28 with byte-identical stdout and empty stderr;
strict C99 and metadata compilation, diff checks and validation self-tests pass. Staged metadata
reaches2991/3205 and `models.h` reaches208/216. Evidence:
`checkpoint-629-c-model-scene-metadata-focused-*-direct.*`. No material, buffer, texture, draw call,
screenshot, visual result or project-built execution is claimed. A real content-loaded material-
variant model and its Tag boundary are next.

## Focused630-C-content-loaded-Model-variants-and-tags

`c_api.content.model_loading` adds the final eight `models.h` routes in30 controls. A real glTF
load exposes exact ordered Ocean/Gold names, atomic short copies and valid/invalid selection state.
Programmatic back-buffer probes distinguish the red default, blue Ocean and gold body while the
unmapped wheels remain black. Two private hand-written XNB Models cover both processor-tag shapes:
an independently owned dictionary retains its exact BoundingSphere after Model destruction, and a
shared reflective reader returns the caller's exact foreign object without transferring ownership.
SOFTWARE and OPENGLES3 Xvfb direct runs pass30/30 with byte-identical stdout and empty stderr;
strict C99 and metadata compilation, diff checks and validation self-tests pass. Staged metadata
reaches2999/3205 and `models.h` reaches216/216, completing its semantic route audit. Evidence:
`checkpoint-630-c-model-loading-focused-*-direct.*`. No screenshot or visual inspection was
performed, and project-built execution still awaits a coherent sibling CNA checkout. Continue with
the deeper generated XNA public member/behavior gaps rather than adding demos for the intentionally
uncredited fixed-name, hardware, service, manual or renderer-specific C routes.

## Focused631-Model-pose-arrays-and-collection-contracts

The existing `graphics.model.bones` retains its four original controls and adds eight for the
deeper XNA member audit. Exact bone, child and mesh lookup/membership return the same objects as
indexed access and distinguish absent or detached objects. `CopyBoneTransformsFrom` atomically
refuses a short source, applies a complete pose while ignoring one safe extra value, composes a
root +X change through the Forearm absolute transform and restores the authored locals. Short local
and absolute destinations preserve all sentinels. An isolated relink of the strictly compiled
updated object against the last complete build passes12/12 under SOFTWARE and OPENGLES3 with byte-
identical stdout and empty stderr. A temporary coverage report moves the generated XNA member
projection1405->1408/2326 without adding a demo ID. Evidence:
`checkpoint-631-model-bones-members-focused-*`. No screenshot or visual inspection was performed;
the sibling CNA checkout is still unsuitable for a full project reconfigure. The null-effect part
skip identified in the ModelMesh audit is the next focused model behavior.

## Focused632-ModelMeshPart-omission-rules

The existing `graphics.model.mesh_parts` retains all10 original controls and adds two exact
omission paths. Clearing the right part's Effect removes green from the mesh Effects collection,
keeps the left triangle red and leaves the right sample at the clear color. Restoring green and
setting the left PrimitiveCount to zero instead leaves red in Effects while omitting only its
geometry and drawing the green sibling. The updated object compiles cleanly; an isolated cumulative
relink passes12/12 under SOFTWARE and OPENGLES3 with byte-identical stdout and empty stderr.
Evidence:`checkpoint-632-model-mesh-parts-skip-focused-*`. Pixels were read programmatically; no
screenshot or visual inspection was performed. No demo ID, metadata member or C-route count
changes. Continue the top-level Model/ModelMesh behavior audit before leaving the model family.

## Focused633-VertexElement-layout-values-and-drawing

The remaining core Model-family inventory was dispositioned without another demo: only CLR-style
collection enumerators and the arbitrary `ModelMesh::Tag` carrier remain uncredited, while actual
C++ range iteration and equivalent tag workflows already exist. The deeper member audit therefore
extends `graphics.vertex_declaration.custom_vertex` in place. It retains its original stride
control and adds exact checks for all three elements' `offsetof` values, formats, usages and usage
indices; independently authored equality, inequality, typed Equals and hash behavior; a four-
property mutable value and exact diagnostic string; and a framebuffer check proving the audited
Position/Color layout draws a white fan center on a black exterior. Its updated object compiles
cleanly. A cumulative isolated relink against the last complete libraries passes7/7 under SOFTWARE
and OPENGLES3 with byte-identical stdout and empty stderr. Temporary generated coverage moves the
post-checkpoint631 XNA-member projection1408->1417/2326 with no new demo ID or C route. Evidence:
`checkpoint-633-custom-vertex-focused-*` and `/tmp/checkpoint-633-coverage.md`. Pixels were read
programmatically; no screenshot or visual inspection was performed. The current sibling CNA is
clean at `cc547f5224`, while this checkpoint deliberately retains the stable library hash
`730ac2631f65b402d5d016ae85649067441b52c973e49f786b49b3253d75100a`; assess the clean checkout
before the next complete rebuild.

## Complete634-retired-renderers-and-full-staged-baseline

The first full build since complete524 absorbs every focused batch through checkpoint633. The
catalog now has627 demos and validates without metadata problems; all640 enabled CTests pass, with
three declared tests disabled. Complete SOFTWARE smoke is556 pass/33 expected fail/35 skip/3
manual, and OPENGLES3 is588/33/3/3; neither has an unexpected pass. Generated coverage is297/329
XNA types,1417/2326 XNA members and2999/3205 C routes.

The integration removed CNA's retired SDL_RENDERER identity from presets, documentation and dead
PBR restriction branches. It also corrected two C Guide demo assertions, isolated the cold-start
ResourceContentManager workflow so global reader registration cannot hide ISSUE-012, and retired
the ISSUE-021 marker after CNA `a6e8104d2` fixed rendered-cube sampling on both EasyGL renderers.
Clean CNA `53aba6ef15` and all final binary hashes are recorded in HANDOFF.md. Continue Phase15 with
the existing stock-vertex declaration/value workflow, then select the next high-value public member
behavior instead of increasing demo count mechanically.

## Focused635-stock-vertex-declarations-and-values

The existing `graphics.vertex_declaration.stock_vertex_types` retains its original stride and
rendered checker/tint checks and adds six coherent controls. All four IVertexType properties return
their stable static declarations; the exact packed Position/Color/Normal/TextureCoordinate elements
produce strides16/24/32/20. Independently authored constructors expose every public field. Equality,
inequality, typed and boxed Equals, wrong-type rejection, equal hashes and exact field-labelled
ToString values cover the shared stock-value contract without adding redundant demos. The project
build passes8/8 under SOFTWARE and OPENGLES3 with byte-identical stdout and empty stderr; the target
SOFTWARE CTest also passes. Temporary generated coverage moves XNA members1417->1437/2326 with no
demo, type or C-route count change. Evidence:`checkpoint-635-stock-*-direct.*` and
`/tmp/checkpoint-635-coverage.md`. Pixel readback was programmatic; no screenshot or visual
inspection was performed. The four explicit CLR interface aliases remain intentionally uncredited
because the same C++ IVertexType property is executed. Audit the existing SpriteFont workflows next.

## Focused636-SpriteFont-layout-fallback-measurement-and-copying

Four existing SpriteFont IDs close the pending coherent behavior rows without adding demos.
`glyph_constructor` now has eight controls for exact retained atlas/tables, all four public layout
inspection extensions, asymmetric cropping, signed bearings, unusual measurement and actual
framebuffer placement. `default_character` has six controls including atomic rejection of an absent
fallback and reset-to-refusal. `measure_string` has seven controls including empty and CR/CRLF
rules. `spacing` has four controls for copy independence and negative gap spacing. The complete
seven-ID SpriteFont category passes under SOFTWARE and OPENGLES3; the four changed SOFTWARE CTests
pass. SOFTWARE batch stderr is empty; OPENGLES3 emits six existing GTK initialization-order warnings
while all results pass. Focused pixel checks were programmatic; no screenshot or visual inspection
was performed. Generated inventory remains1437/2326 because all five XNA SpriteFont members were
already credited and the four newly direct CNAEXT inspection methods are outside that inventory.
Evidence:`/tmp/checkpoint-636-sprite-font-*`. Audit SpriteBatch lifecycle and queued-resource
ownership next.

## Focused637-SpriteBatch-disposal-and-queued-resource-lifetime

`graphics.sprite_batch.begin_end_rules` retains its five sequencing controls and adds seven
lifecycle/ownership controls. Explicit Dispose raises Disposing once before IsDisposed, is
idempotent, and makes Begin/End/Draw throw ObjectDisposedException. A deferred command retains a
temporary Texture2D renderer after its C++ wrapper leaves scope and draws exact orange/black
framebuffer samples on SOFTWARE and OPENGLES3. The final control exposes new CNA-DEMOS-ISSUE-082:
disposing an open deferred batch clears its local state but leaves the GraphicsDevice active-batch
counter set, permanently refusing an unrelated Immediate batch. Both renderers produce eleven
passes plus that one expected failure with byte-identical stdout and empty stderr; the known-issue
CTest passes. Temporary coverage moves XNA members1437->1438/2326 through public SpriteBatch.Dispose
without changing the627-demo/type/C-route counts. Evidence:`/tmp/checkpoint-637-sprite-batch-*` and
`/tmp/checkpoint-637-coverage.md`. Pixel readback was programmatic; no screenshot or visual
inspection was performed. Audit GamePadState value identity/diagnostics and the two omitted stick-
button capability members next.

## Focused638-GamePad-value-identity-and-disconnected-capabilities

`input.gamepad.state_values` retains all11 construction, clamping, virtual-button and operator
controls and adds seven coherent value-contract checks. GamePadState plus Buttons, DPad,
ThumbSticks and Triggers now exercise typed/boxed Equals, wrong-type rejection, equality/inequality,
equal hashes and exact diagnostic strings; the renderer-free run passes18/18 with empty stderr.

`input.gamepad.capabilities` now includes the omitted left/right stick-button properties. Its
GamePad need was removed because the source already tested disconnected controllers but preflight
made that path unreachable without hardware. SOFTWARE and OPENGLES3 each pass eight checks over
four absent player slots with byte-identical stdout and empty stderr. This proves false/disconnected
behavior only; no physical controller claim is made. Both focused CTests pass. Temporary coverage
moves XNA members1438->1460/2326 with no ID/type/C-route change; the next full smoke should replace
one GamePad skip with one pass under each renderer. Evidence:`/tmp/checkpoint-638-gamepad-*` and
`/tmp/checkpoint-638-coverage.md`. No screenshot or visual inspection was performed. Audit the
existing VideoPlayer lifecycle next, including the already-called but uncredited Dispose/IsDisposed
contract and post-dispose behavior.

## Focused639-VideoPlayer-active-disposal-and-refusal

`media.video_player.pause_and_loop` retains its real decode, pause, resume, frame acquisition and
end-to-start loop behavior and adds six lifecycle controls. The automated run disposes the player
while its loop is active, observes IsDisposed together with Stopped/null-Video/zero-position state,
repeats Dispose safely, and verifies ObjectDisposedException from Play, Stop and GetTexture.
SOFTWARE and OPENGLES3 each pass12/12 with empty stderr; playback timings differ naturally, so the
outputs are not claimed byte-identical. The focused SOFTWARE CTest passes. Temporary coverage moves
the post-focused638 XNA-member projection1460->1462/2326 through VideoPlayer.Dispose and IsDisposed,
without changing the627-demo/type/C-route counts. Evidence:`/tmp/checkpoint-639-video-lifecycle-*`
and `/tmp/checkpoint-639-coverage.md`. No screenshot, frame-pixel inspection, visual-quality or
audible-quality claim was made. Audit the existing spatial-audio workflow next for
AudioListener.Velocity and AudioEmitter.Up, with dummy-audio evidence limited to public state and
accepted Apply3D calls.

## Focused640-spatial-audio-state-and-accepted-input

`audio.sound_effect_instance.apply_3d` retains its original moving-emitter sweep, public Pan/3D
mode transitions and SpeedOfSound control and adds three coherent state checks. The listener and
emitter expose their exact zero/default-basis records; a nonzero AudioListener.Velocity and
nondefault AudioEmitter.Up round-trip exactly; and Apply3D accepts that authored input while the
voice remains stopped. Neutral values are restored before the existing playback sequence.
SOFTWARE and OPENGLES3 pass10/10 with byte-identical stdout and empty stderr, and the focused
SOFTWARE CTest passes. Temporary coverage moves the focused XNA-member projection1462->1464/2326
without changing the627-demo/type/C-route counts. This establishes public state and accepted input
under dummy audio, not mixed Doppler, panning, orientation or listening output. Evidence:
`/tmp/checkpoint-640-audio-spatial-*` and `/tmp/checkpoint-640-coverage.md`. No screenshot or visual
inspection was performed. Audit AudioCategory and RendererDetail value identity/diagnostic
contracts next while preserving their real XACT workflows.

## Focused641-XACT-value-identity-and-record-lifetime

Two existing XACT workflows close the remaining useful Audio value-contract gaps.
`audio.xact.categories` now passes12 controls: typed/boxed equality, wrong-type rejection, equal
hashes and exact name text complement its real Music/SFX and hierarchy controls, including equal
SFX names from two different engines whose operations remain attached to their respective engines.
`audio.xact.renderer_details` now passes six controls: a copy of CNA's actual SDL3_mixer record has
typed/boxed equality, equal operators/hash and FriendlyName text, rejects empty/wrong boxes, and
retains both strings after the discovery engine is disposed before its id selects the playing
engine. No second record was invented and no unequal-device or actual output-selection claim is
made. SOFTWARE and OPENGLES3 pass both IDs with byte-identical per-ID stdout and empty stderr; both
focused SOFTWARE CTests pass. Temporary coverage moves the XNA-member projection1464->1472/2326
without changing the627-demo/type/C-route counts. Evidence:`/tmp/checkpoint-641-*` and
`/tmp/checkpoint-641-coverage.md`. No listening, screenshot or visual inspection was performed.
Audit Song equality/hash/operators/text next using caller-created same-file/different-name and
different-file/same-name records, while preserving the existing property/disposal workflow.

## Focused642-Song-file-identity-and-display-text

`media.song.properties` retains its original caller-created metadata and Dispose/IsDisposed
workflow and adds two value-contract controls. Three owned Songs distinguish two names for the
same file from one name shared by two files: typed/System::Object Equals, both operators and equal
hashes establish file-handle identity, null object comparison is false, and ToString returns each
record's display name independently. SOFTWARE and OPENGLES3 pass7/7 with byte-identical stdout and
empty stderr; the focused SOFTWARE CTest passes. Temporary coverage moves the XNA-member projection
1472->1477/2326 through Song.Equals/GetHashCode/operators/ToString without changing the627-demo/
type/C-route counts. Neither file is played, so no decoded or audible identity is inferred.
Evidence:`/tmp/checkpoint-642-song-values-*` and `/tmp/checkpoint-642-coverage.md`. No listening,
screenshot or visual inspection was performed. Audit Album, Artist, Genre and Playlist value
contracts next inside the existing real indexed-library workflow; leave borrowed lifetime and
disposal behavior intact.

## Focused643-indexed-media-value-contracts

`media.media_library.music_index` retains all18 relationship, ordering and duration controls and
adds five. A second independent MediaLibrary indexes the same isolated fixture and supplies
distinct but equal objects: Album identity combines title and artist, while Artist, Genre and
Playlist use their names. Typed/System::Object Equals, operators, equal hashes, exact text and
cross-type refusal run on these real values. The five collection Item members are also credited
because both the original and mirror workflows directly index Song/Album/Artist/Genre/Playlist
collections to establish order and shared identity. Both runner selections pass23 child checks
with byte-identical stdout and the same six expected TitleContainer INFO stderr lines; parent JSON
remains zero checks and the focused SOFTWARE CTest passes. Temporary coverage moves the XNA-member
projection1477->1501/2326 without changing the627-demo/type/C-route counts. Different hashes for
unequal values, reflection names, borrowed disposal and post-library pointer use are not claimed.
Evidence:`/tmp/checkpoint-643-media-values-*` and `/tmp/checkpoint-643-coverage.md`. No playback,
listening, screenshot or visual inspection was performed. Audit MediaQueue.Item and queue-owned
copies next without invoking the known unsafe Queue.ActiveSong replay path from ISSUE-058.

## Focused644-MediaQueue-owned-items-and-range-refusal

`media.media_player.queue_and_repeat` retains its six transport/repeat/shuffle controls and adds
three ownership/indexer controls. MediaPlayer.Play copies the two ordered caller Songs into distinct
queue-owned records; those same entries and ActiveSong remain readable after the caller objects are
destroyed. MediaQueue.Item rejects negative and count indices with ArgumentOutOfRangeException and
leaves both entries unchanged. The known unsafe ISSUE-058 Queue.ActiveSong replay path is never
called. SOFTWARE and OPENGLES3 pass9/9 with byte-identical stdout and empty stderr; the focused
SOFTWARE CTest passes. Temporary coverage moves the XNA-member projection1501->1502/2326 without
changing the627-demo/type/C-route counts. This is transport state and C++ ownership evidence, not
playback quality or listening evidence. Evidence:`/tmp/checkpoint-644-media-queue-*` and
`/tmp/checkpoint-644-coverage.md`. No screenshot or visual inspection was performed. Audit the
MediaSource records next: own every pointer returned by GetAvailableMediaSources and verify
ToString against the actual names without reading a real user library during validation.

## Focused645-MediaSource-record-ownership-and-library-copy

`media.media_library.collections` now adopts every raw pointer returned by
GetAvailableMediaSources instead of leaking the records. CNA's actual Local Device record has a
nonempty Name equal to ToString; the MediaLibrary source constructor publishes a distinct copied
record with the same name, type and text; and the existing IsDisposed=false assertion is now
credited. SOFTWARE and OPENGLES3 pass3/3 with byte-identical stdout and empty stderr; the focused
SOFTWARE CTest passes. Both direct runs and the CTest use fresh empty XDG music/picture roots,
report zero items and leave the roots empty, so no real user library was read. Temporary coverage
moves the XNA-member projection1502->1505/2326 through MediaSource.ToString,
MediaLibrary.MediaSource and MediaLibrary.IsDisposed without changing the627-demo/type/C-route
counts. This establishes CNA's one returned desktop source and source-copy ownership, not wider
device enumeration. Evidence:`/tmp/checkpoint-645-media-source-*` and
`/tmp/checkpoint-645-coverage.md`. No playback, listening, screenshot or visual inspection was
performed. Audit Picture token/date/value identity and PictureAlbum identity next in the existing
isolated save_picture workflow while preserving visible ISSUE-041.

## Focused646-picture-token-date-and-value-identity

`media.media_library.save_picture` retains all14 prior controls and the visible ISSUE-041 failure
and adds six. The immediate Picture Date is bounded by its SavePicture call. PictureCollection and
PictureAlbumCollection Item expose the two reindexed files and their shared Saved Pictures album.
The original path token resolves an independent reopened Picture through public
GetPictureFromToken, while an unknown token returns null and the indexed Date is populated.
Original/reopened Picture and PictureAlbum pairs exercise path-based typed/System::Object equality,
operators, equal hashes, exact name text and wrong-type rejection; Preview remains an unequal
Picture. SOFTWARE and OPENGLES3 each report19 passes plus the one expected ISSUE-041 failure with
byte-identical stdout and identical two-line TitleContainer INFO stderr; the focused known-issue
SOFTWARE CTest passes. Temporary coverage moves the XNA-member projection1505->1519/2326 without
changing the627-demo/type/C-route counts. Borrowed Dispose flags, post-library pointer use and
visual quality remain unclaimed. Evidence:`/tmp/checkpoint-646-picture-values-*` and
`/tmp/checkpoint-646-coverage.md`. No screenshot or visual inspection was performed. Run the full
627-demo integration checkpoint next to absorb focused635 through646, then resume deeper member
auditing.

## Complete647-member-audit-integration

The full outer-Xvfb validation absorbs focused635 through646 into the actual project runner.
Metadata is627/0, all640 enabled CTests pass, and three declared tests remain disabled. SOFTWARE is
555 pass/34 expected fail/35 skip/3 manual; OPENGLES3 is587/34/3/3. Both report zero unexpected
passes. Against complete634, the sole status/knownIssue change on either renderer is the intended
`graphics.sprite_batch.begin_end_rules` pass-to-expected-failure transition for ISSUE-082. Every
other extended ID preserves its status and only increases its control count. Generated coverage is
297/329 XNA types,1519/2326 XNA members and2999/3205 C routes.

The build is diagnostic-free. The run began after observing clean sibling CNA `09c3108e58`; final
capture found that external checkout independently at dirty `af798b73a4`, while both production
library hashes remained identical to complete634. Exact binary hashes and source-state details are
in HANDOFF.md; runtime evidence is archived as `checkpoint-647-complete-*`. No screenshot,
listening test or manual visual inspection was performed. Continue the public-member audit with
the ContentReader scalar/math read family in the existing custom reader workflow.

## Focused648-ContentReader-scalar-and-math-values

`content.xnb.custom_type_reader` retains its original custom Level/cache workflow and extends the
generated Level XNB with one authored scalar/color/math state block. LevelReader now directly calls
ReadSingle, ReadDouble, ReadColor, ReadMatrix, ReadQuaternion and ReadVector2/3/4. Five new controls
check exact scalars, RGBA byte order, all16 row-major matrix elements, quaternion XYZW and every
vector component. SOFTWARE and OPENGLES3 each pass10/10 with byte-identical stdout and empty
stderr; the focused SOFTWARE CTest passes. The asset regenerates deterministically and the build is
diagnostic-free. Temporary coverage moves1519->1527/2326 XNA members without changing the627-demo,
type or C-route counts. Evidence:`/tmp/checkpoint-648-content-reader-*` and
`/tmp/checkpoint-648-coverage.md`. No screenshot or visual inspection was performed. Audit the
MouseState constructor/value contract and adjacent KeyboardState hash behavior next in the existing
input state workflows.

## Focused649-keyboard-and-mouse-value-contracts

New console demo `input.mouse.state_values` uses eight controls to cover default and fully authored
MouseState records, all XNA fields, typed/boxed equality, wrong and empty boxes, both operators,
equal hashes and exact stable text. Its CNA horizontal-wheel constructor also demonstrates that the
extension value remains deliberately outside XNA equality/hash/text. The existing
`input.keyboard.state_values` grows from eight to nine controls and proves equal key sets hash
equally regardless of insertion order while the empty set hashes to zero. Both IDs pass under
SOFTWARE and OPENGLES3 with byte-identical per-ID stdout and empty stderr; both SOFTWARE CTests
pass. The final incremental build is diagnostic-free after the new source's automatic CMake
regeneration. The catalog reaches628 demos and temporary coverage moves1527->1534/2326;
`Microsoft.Xna.Framework.Input` is112/113, with only platform-specific Mouse.WindowHandle
uncredited by disposition. Evidence:`/tmp/checkpoint-649-*` and `/tmp/checkpoint-649-coverage.md`.
No physical input, screenshot or visual inspection was performed. Audit TouchLocation value
identity plus the read-only TouchCollection copy/mutation contract next in `input.touch.touch_values`.
