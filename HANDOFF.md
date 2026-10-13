# HANDOFF

**Read this first in every new session.** It is the single place that says what state the
project is in and what to do next. Update it after every batch. Last updated: 2026-10-13.

## 1. Purpose (one paragraph)

cna-demos is a growing collection of small executable demonstrations of CNA (C++ XNA 4.0
reimplementation): one concept per demo, stable textual IDs, metadata, a terminal browser, a CLI,
unattended runs, CTest tests and a coverage report. Long-term goal: every meaningful CNA
capability has a demo. See docs/PROJECT.md.

## 2. Current phase

Systematic demo batches, one CNA area at a time (status per phase in docs/ROADMAP.md):

* Complete: Phase 0-1 (research, infrastructure), 2 (math, 78 + 6 packed vectors), 3 (game lifecycle, 33), 4 (2D
  graphics), 5 (render states, buffers, primitives, render targets), 10 (input, 39), 11 (storage, 7).
* Mostly complete: 6 (effects and shaders, 38).
* In progress: 7 (models/glTF, 23), 8 (content, 46), 9 (audio 23 incl. 10 XACT, media 13 incl. a
  compiled video and song), **12 (gamer services 19 incl. avatars and leaderboards; net 14; remainders in ROADMAP)**.
* In progress too: 13 (devices 5, phone 1, design 4, diagnostics 2 + logger 2, cnaext 4).
* In progress: 14 (C API, 66 standalone C demos; `linux-full` builds CNA with `CNA_BUILD_C_API=ON`).
* In progress: 15 (type decisions and deeper public-member/behavior tables in docs/COVERAGE_AUDIT.md;
  comprehensive semantic coverage is not yet established).

## 3. Architecture in 10 lines

* One executable `cna-demos` (src/app: CLI `Cli.cpp`, browser `Browser.cpp`).
* Framework (src/framework, headers src/CnaDemos): `Registry`, `DemoInfo` metadata, `DemoContext`
  (output/checks/seed), `DemoGame : Microsoft::Xna::Framework::Game`, `Runner`, catalog JSON,
  validation, process helpers.
* Demos: `demos/<category>/.../<name>.cpp`, ID = path, registered by `CNA_DEMOS_CONSOLE`/
  `CNA_DEMOS_GAME`; each category is an OBJECT library found by `GLOB_RECURSE CONFIGURE_DEPENDS`.
* Standalone demos: `standalone/<id>/{metadata.cpp,main.cpp,CMakeLists.txt}` → `cna-demo-<id-with-dashes>`.
* `ownProcess` Game/Console demos isolate once-per-process services or deliberate registry
  mutation; batch runs and the browser start them as `cna-demos <id> --batch-child --report <tmp>`.
* CNA consumed via `add_subdirectory(${CNA_ROOT_DIR} EXCLUDE_FROM_ALL SYSTEM)`, linking `CNA`.
* Renderer: CNA's `CNA_GRAPHICS_RENDERER(S)`; runtime `--renderer` (one renderer per process).
* Project graphics profile set to HiDef in `main` (Reach demos opt in).
* CTest: `cna_demos.cli.*` + per-demo tests discovered at ctest time (`--list-ids`).
* Coverage: `tools/coverage_report.py` → docs/COVERAGE.md (matches `.apis` to CNA's XNA inventory).
* Full details: docs/ARCHITECTURE.md.

## 4. What is complete

* CMake (presets `default`, `windowless`, `linux-multi`, `linux-full`, `linux-full-release`),
  CNA integration, warnings, packaging helpers, asset copying.
* Registry, metadata, validation (`--validate`), catalog (`--catalog-json`), results (`--report`).
* Browser (tree navigation, search, info, run in-process, back) and CLI (see `--help`).
* DemoGame: need checks → SKIPPED, automated frame limit, Escape/Back exit, `--screenshot`.
* CTest integration and CI workflow (`.github/workflows/ci.yml`, windowless preset).
* Docs: README, PLAN, CLAUDE, CONTRIBUTING, CHANGELOG, THIRD_PARTY_NOTICES, docs/*.
* Pilot demos (9) and math batches (vectors, Matrix, Quaternion, MathHelper): see section 6.

## 5. Partially complete / known gaps

* Visual regression: screenshots exist, reference comparison not implemented (AUTOMATION.md §5).
* CI on GitHub (ubuntu-24.04, `windowless` preset): green since run 4 (the CTest discovery fix;
  checked through run 40); runs 1-3 failed only at the ctest step (`if(IN_LIST)` in ctest script
  mode on CMake 3.x).
  Check the latest run with the GitHub Actions list for `libcna/cna-demos`.
* `windowless` preset: see build status below.

## 6. Demo counts

521 catalog demos (latest complete checkpoint504-C-graphics-device plus focused505-521 effect batches): math 78, game 33, graphics 162 (19 SpriteBatch, 8 device, 9 texture2d, 5 texture_cube, 2 texture3d, 7 sprite font,
4 blend state, 4 sampler state, 3 rasterizer state, 6 depth-stencil state, 4 primitives,
4 vertex buffer, 3 index buffer, 2 vertex declaration, 9 render target, 1 occlusion query, 8 BasicEffect, 30 other
effects and shaders, 2 viewport, 2 adapter, 1 resource, 6 packed vector, 23 model), audio 23 (10 XACT), core 3 (2 logger),
content 46 (incl. an XNA .contentproj), input 39 (5 keyboard, 6 mouse, 2 text input, 2 clipboard, 12 gamepad, 5 touch, 7 CNAEXT devices:
joystick, hot-plug, device lists, haptics, sensors, power), storage 7, media 13 (compiled video and song, tagged library import and album art), gamer services 19
(own process each; 5 avatar, 2 leaderboard), net 14 (own process each; 5 standalone: two processes), devices 5, phone 1, design 4,
diagnostics 2, cnaext 4, platform 2 (1 standalone), c_api 66 (standalone C programs). Every declared category has demos.
Coverage: see docs/COVERAGE.md (current catalog, complete504 runtime reports and focused505-521 source).
Actual C ABI coverage is1082/3205: `input_gamepad.h`61/63,
`input_devices.h`31/37, `input_text.h`17/17, `input_joystick.h`27/30 and
`input_haptics.h`33/36, `sensors.h`116/144, `devices.h`59/62, `display.h`22/22 and
`render_target.h`12/12, `graphics.h`25/27, `graphics_resource.h`12/12 and
`index_resources.h`8/10, `graphics_state.h`14/14, `graphics_device.h`80/84, `effects.h`289/290,
`texture.h`2/21 and `texture_volume.h`4/16.
Complete504 exercises the first797 credited routes in both full smoke suites; focused505 exercises
the next22, focused506 the next17, focused507 the next10, focused508 the next31, focused509 the
next18, focused510 the next4, focused511 the next42, focused512 the next22, focused513 the next19,
focused514 the next21, focused515 the next7, focused516 the next31, focused517 the next10,
focused518 the next5, focused519 the next10, focused520 the next11 and focused521 the newest5
routes under both renderer selections.
Build-only CNA-DEMOS-ISSUE-057 is tracked separately; it adds no runtime knownIssue marker.
Known-issue demos: 32 (`c_api.graphics.pbr_effect_material_state` CNA-DEMOS-ISSUE-070,
`c_api.graphics.draw_routes` CNA-DEMOS-ISSUE-066,
`c_api.devices.sensor_lifecycle` CNA-DEMOS-ISSUE-062,
`c_api.storage.save_game` CNA-DEMOS-ISSUE-056,
`graphics.shader_effect.clone_state` CNA-DEMOS-ISSUE-055,
`graphics.texture_cube.dds_streams` CNA-DEMOS-ISSUE-054,
`c_api.game.host_failure` CNA-DEMOS-ISSUE-051,
`c_api.game.foreign_thread_dispatch` CNA-DEMOS-ISSUE-050,
`content.custom.xna_pipeline_components` CNA-DEMOS-ISSUE-047 and -048,
`content.custom.intermediate_external_references` CNA-DEMOS-ISSUE-046,
`content.content_manager.asset_precedence` CNA-DEMOS-ISSUE-045,
`content.xnb.existing_objects` CNA-DEMOS-ISSUE-043,
`media.media_library.save_picture` CNA-DEMOS-ISSUE-041,
`content.content_manager.cache_and_unload` CNA-DEMOS-ISSUE-039,
`audio.dynamic_sound_effect_instance.streaming` CNA-DEMOS-ISSUE-038,
`math.curve.step_continuity` CNA-DEMOS-ISSUE-002,
`math.curve.loop_types` CNA-DEMOS-ISSUE-036,
`graphics.sprite_batch.immediate_mode.sampler_state` CNA-DEMOS-ISSUE-004,
`content.content_manager.unload_disposes` and `content.content_manager.read_asset_ownership` CNA-DEMOS-ISSUE-011,
`content.content_manager.open_stream_override` and `content.content_manager.resource_content_manager`
CNA-DEMOS-ISSUE-012,
`gamer_services.achievement.award` CNA-DEMOS-ISSUE-017,
`game.time_step.reset_elapsed_time` CNA-DEMOS-ISSUE-019; and on OPENGLES3/OPENGL33 only,
`graphics.render_target.render_target_cube` CNA-DEMOS-ISSUE-021; `graphics.model.textured_gltf`
CNA-DEMOS-ISSUE-022, `graphics.model.gltf_cameras` CNA-DEMOS-ISSUE-023, `math.formatting.to_string`
CNA-DEMOS-ISSUE-024, `net.network_session.simulated_conditions` CNA-DEMOS-ISSUE-030,
`graphics.model.clip_interpolation` CNA-DEMOS-ISSUE-031, `net.network_session.join_refused`
CNA-DEMOS-ISSUE-032 and -033; `graphics.effect.parameter_values` CNA-DEMOS-ISSUE-034 and -035).
Passing documentation issues ISSUE-068, ISSUE-069 and ISSUE-071 do not add runtime knownIssue markers.
Live numbers: `cna-demos --categories`, docs/COVERAGE.md.

## 7. Build and test status

Focused **521-C-shader-textures** adds `c_api.graphics.shader_effect_textures` for every ShaderEffect
texture-binding route. Twenty controls create real 2D, cube and 3D resources under both requested
renderers; prove negative-unit and wrong-family refusal; replace 2D with cube and cube with volume
on one unit; and keep the final volume alive through Effect destruction until the last retained pass
view releases. Direct execution and both runner selections pass with byte-identical runner output
and empty stderr. All26 focused ShaderEffect/CLI tests satisfy their contracts in6.57s (18 passed,
eight declared SOFTWARE capability skips). Metadata is521/0 and the final incremental build is
clean. The three binding routes plus Texture3D create/destroy raise coverage1077->1082/3205,
`effects.h`286->289/290 and `texture_volume.h`2->4/16. The configure build emitted only the known
third-party Draco CMP0148 developer warning. Clean sibling CNA is `a6bb279d47`; the production
library baseline is unchanged. No sampling, draw, pixel or visual inspection is claimed.
Evidence:`checkpoint-521-c-shader-textures-focused-*`.

Focused **520-C-shader-uniforms** adds `c_api.graphics.shader_effect_uniforms` for every loose typed
and uniform-block ShaderEffect route. Eighteen controls exercise active matrix/vector/scalar/integer
uniforms, typed float/vec2 arrays, tightly packed vec3/mat4 arrays, all empty-null forms, invalid
array shapes, embedded-NUL name refusal and a two-member std140 declaration/clear with overwritten
caller storage. Direct execution and both runner selections pass with byte-identical runner output
and empty stderr. All25 focused ShaderEffect/CLI tests satisfy their contracts in5.64s (17 passed,
eight declared SOFTWARE capability skips). Metadata is520/0 and the final incremental build is
clean. Eleven routes raise coverage1066->1077/3205 and `effects.h`275->286/290. The configure build
emitted only the known third-party Draco CMP0148 developer warning. Clean sibling CNA is
`a6bb279d47`; the production library baseline is unchanged. Existing pixel demos cover rendered
uniform behavior; this workflow draws nothing and claims no pixel or visual inspection.
Evidence:`checkpoint-520-c-shader-uniforms-focused-*`.

Focused **519-C-shader-state** adds `c_api.graphics.shader_effect_state` for source construction,
canonical validity/renderer ownership, empty successful compiler diagnostics and independent World,
View and Projection state. Seventeen controls pass directly and through both runner selections with
byte-identical runner output and empty stderr. A source-executing renderer must compile and apply
the selected dialect; a non-executing renderer still proves the documented object contract. The
focused ShaderEffect/CLI suite satisfies all24 contracts in19.54s (16 passed, eight declared
SOFTWARE capability skips). Metadata is519/0 and the incremental build is clean. Ten routes raise
coverage1056->1066/3205 and `effects.h`265->275/290. Clean sibling CNA is `a6bb279d47` and the
production library baseline is unchanged. No draw, pixel or visual inspection is claimed.
Evidence:`checkpoint-519-c-shader-state-focused-*`.

Focused **518-C-skinned-pbr** adds `c_api.graphics.skinned_pbr_effect_state` for every
SkinnedPbrEffect-specific C route and representative shared PBR/interface compatibility.
Thirty-two controls prove the complete identity and authored 72-bone palettes, copied input,
atomic short output, invalid-count preservation, one/two/four weights, core material mutation,
required lighting, matrices/fog, same-device base texture retention and material/palette/texture
survival through a type-preserving clone and source destruction. Direct execution and both runner
selections pass with byte-identical runner output and empty stderr. Metadata is518/0; all21 focused
PBR and CLI tests satisfy their contracts in6.07s (18 passed, three declared SOFTWARE capability
skips); and the final incremental build is clean. Five routes raise coverage1051->1056/3205 and
`effects.h`260->265/290. The first configure build repeated the already documented transient
CNA-DEMOS-ISSUE-057 staging warning and still completed; the clean incremental gate did not repeat
it. Clean sibling CNA is `a6bb279d47`; production library hashes remain unchanged. Apply is state-
only; no skinned PBR draw, vertex deformation, BRDF pixel or visual inspection is claimed.
Evidence:`checkpoint-518-c-skinned-pbr-focused-*`.

Focused **517-C-pbr-textures** adds `c_api.graphics.pbr_texture_slots` for the complete seven-slot
PBR texture surface. Thirty-four controls prove transform value initialization/equality; empty,
UV0, identity-transform and sRGB defaults; distinct UV/transform mutations; invalid slot/channel/
Boolean/structure preservation; same-device validation; and two actual Texture2D handles retained
by five core and two specular slots through cloning and source destruction, then released only
after each final clone slot clears. Direct execution and both runners pass, runner output is byte-
identical and all three stderr files are empty. Metadata is517/0; all20 focused PBR and CLI tests
satisfy their contracts in5.44s (17 passed, three declared SOFTWARE capability skips); and the
incremental build is clean. Ten routes raise coverage1041->1051/3205 and `effects.h`250->260/290.
CNA-DEMOS-ISSUE-071 records the passing public comment's stale claim of five rather than seven
texture-slot identities. Clean sibling CNA is `a6bb279d47`; production library hashes remain
unchanged. No sampling, PBR draw, pixel result or visual inspection is claimed.
Evidence:`checkpoint-517-c-pbr-textures-focused-*`.

Focused **516-C-pbr-material** adds `c_api.graphics.pbr_effect_material_state` for every non-texture
PbrEffect material route. Thirty-three controls prove exact core/glTF defaults, deliberately
unclamped factors, all three alpha-mode identities and invalid preservation, Boolean validation,
required lighting, shared matrices/fog and clone survival. Twenty-nine pass identically under
SOFTWARE and OPENGLES3; four retained failures establish CNA-DEMOS-ISSUE-070: the clone resets
vertex color, alpha mode, alpha cutoff and double-sided state while preserving the other material
values and output encoding. Runner output is byte-identical and all three stderr files are empty.
Metadata is516/0; all19 focused PBR and CLI tests satisfy their contracts in5.94s (16 passed, three
declared SOFTWARE capability skips); and the incremental build is clean. Thirty-one routes raise
coverage1010->1041/3205 and `effects.h`219->250/290. Clean sibling CNA is `a6bb279d47`; production
library hashes remain unchanged. Apply is state-only; no PBR draw, BRDF pixel or visual inspection
is claimed. Evidence:`checkpoint-516-c-pbr-material-focused-*`.

Focused **515-C-color-matrix-effect** adds `c_api.graphics.color_matrix_effect_state` for every
ColorMatrixEffect C route. Twenty-two controls prove identity/zero defaults; exact finite
row-major matrix and unclamped offset round trips; atomic NaN/infinity refusal; the exact Rec. 709
grayscale matrix with cleared offset; reset; and authored state survival through a type-preserving
clone and source destruction. Direct execution and both runner selections pass, the runner output
is byte-identical, and all three stderr files are empty. Metadata is515/0; all17 focused C/C++
ColorMatrixEffect and CLI tests pass in6.73s; and the incremental build is clean. Coverage rises
1003->1010/3205 and `effects.h`212->219/290. Clean sibling CNA is `a6bb279d47`; production library
hashes remain unchanged. The existing C++ SOFTWARE demo supplies pixel validation; this C workflow
is state-only and makes no draw, transformed-pixel or visual-inspection claim.
Evidence:`checkpoint-515-c-color-matrix-effect-focused-*`.

Focused **514-C-skinned-effect** adds `c_api.graphics.skinned_effect_state` for every SkinnedEffect
C route. Thirty-five controls prove material/switch/interface defaults and mutations; required
lighting; accepted one/two/four weights; a complete identity and authored 72-bone palette; copied
input, atomic short output and invalid-count preservation; ownerless-texture refusal; and retained
same-device texture plus palette survival through a type-preserving clone and source destruction.
Direct execution passes, SOFTWARE and OPENGLES3 runner output is byte-identical, and all three
stderr files are empty. Metadata is514/0; all26 focused C effect, C++ SkinnedEffect and CLI tests
satisfy their contracts in15.47s (23 passed, three declared SOFTWARE capability skips); and the
incremental build is clean. Coverage rises982->1003/3205 and `effects.h`191->212/290. Clean sibling
CNA is `a6bb279d47`; production library hashes remain unchanged. Apply is state-only; no skinned
draw, vertex-weight, pixel result or visual inspection is claimed.
Evidence:`checkpoint-514-c-skinned-effect-focused-*`.

Focused **513-C-environment-map-effect** adds `c_api.graphics.environment_map_effect_state` for
every EnvironmentMapEffect C route. Thirty controls prove exact material/map defaults and
unclamped authored values; shared matrix/fog state; mutable ambient color with permanently enabled
lighting; the canonical three-light preset; and distinct same-device Texture2D/TextureCube slots
retained through cloning and source destruction, then independently released. Direct execution
passes, SOFTWARE and OPENGLES3 runner output is byte-identical, and all three stderr files are
empty. Metadata is513/0, all14 focused C effect, C++ EnvironmentMapEffect and CLI tests pass in
7.57s, and the incremental build is clean. Seventeen effect routes plus TextureCube create/destroy
raise coverage963->982/3205, `effects.h`174->191/290 and `texture_volume.h`0->2/16. Clean sibling
CNA is `a6bb279d47`; production library hashes remain unchanged. Apply is state-only; no draw,
cube sampling, pixel result or visual inspection is claimed.
Evidence:`checkpoint-513-c-environment-map-effect-focused-*`.

Focused **512-C-unlit-stock-effects** adds `c_api.graphics.unlit_stock_effects` for every
AlphaTestEffect and DualTextureEffect C route. Thirty-three controls prove coherent defaults;
unclamped alpha/reference state; comparison and vertex-color selection; independent shared
matrix/fog state; concrete-type rejection; invalid texture-index atomicity; and two same-device
textures retained independently across type-preserving clones and source destruction. Each texture
releases only after its last clone-owned slot clears. Direct execution passes, SOFTWARE and
OPENGLES3 runner output is byte-identical, and all three stderr files are empty. Metadata is512/0,
all14 focused C effect, C++ alpha-test/dual-texture and CLI tests pass in9.09s, and the incremental
build is clean. Coverage rises941->963/3205 and `effects.h`152->174/290. Clean sibling CNA is
`a6bb279d47`; production library hashes remain unchanged. Apply is state-only; no draw, alpha-test
pixel, texture-sampling or visual inspection is claimed.
Evidence:`checkpoint-512-c-unlit-stock-effects-focused-*`.

Focused **511-C-basic-effect-state** adds `c_api.graphics.basic_effect_state` for the remaining
BasicEffect state routes, all shared IEffectMatrices/IEffectFog/IEffectLights accessors and the
DirectionalLight API. Forty controls prove coherent XNA defaults; exact matrix, material, switch,
fog and lighting round trips; stable aliases for nested lights; the canonical three-light preset;
standalone and nested-view lifetimes; plain-Effect interface refusal; and same-device Texture2D
retention, release and destruction. A nested light stays mutable after its root Effect handle and
callback end. Direct execution passes, SOFTWARE and OPENGLES3 runner output is byte-identical, and
all three stderr files are empty. Metadata is511/0, all24 focused C/API, C++ BasicEffect/Effect/
SpriteBatch and CLI tests pass in15.82s, and the incremental build is clean. Coverage rises
899->941/3205 and `effects.h`110->152/290. Clean sibling CNA is `a6bb279d47`; production library
hashes remain unchanged. Apply is state-only; no draw, pixel result or visual inspection is
claimed. Evidence:`checkpoint-511-c-basic-effect-state-focused-*`.

Focused **510-C-effect-concrete-types** adds `c_api.graphics.effect_concrete_types` for all four
EffectMaterial and SpriteEffect construction/ownership routes. Twenty-two controls prove
EffectMaterial concrete identity, owner and two retained entries for one caller-released CPU
Texture2D; null/invalid/wrong-type behavior; its canonical inherited clone returning an independent
base Effect; exact stock SpriteEffect identity; type-preserving SpriteEffect clone; and independent
disposal. ISSUE-069 records that the generic C clone comment overpromises concrete-type
preservation despite EffectMaterial's deliberate XNA exception. Direct execution passes, SOFTWARE
and OPENGLES3 runner output is byte-identical, and all three stderr files are empty. Metadata is
510/0, all14 focused C/API, C++ Effect/SpriteBatch and CLI tests pass in3.70s, and the incremental
build is clean. Coverage rises895->899/3205 and `effects.h`106->110/290. Clean sibling CNA is
`a6bb279d47`; production library hashes remain unchanged. No draw, pixel result or visual
inspection is claimed. Evidence:`checkpoint-510-c-effect-concrete-types-focused-*`.

Focused **509-C-effect-core** adds `c_api.graphics.effect_core` for the18 core Effect routes.
Twenty-seven controls cover exact owner/type/source/runtime facts; the empty parameter, technique
and effect-owned P0 graph; current-technique ownership and preservation; pass application; clone
and disposal isolation; and child-view survival after root destruction. Constructor checks
distinguish empty/malformed input, MGFX and a committed genuine5000-byte XNA Effect Framework
fixture. Both configured renderers have their compiled-effect build options off, report the
capability false and explicitly refuse the valid binary, so no compiled reflection or shader
execution is claimed. Direct execution passes, SOFTWARE and OPENGLES3 runner output is byte-
identical, and all three stderr files are empty. Metadata is509/0, all12 focused C/API, C++ Effect
and CLI tests pass in4.16s, and the incremental build is clean. Coverage rises877->895/3205 and
`effects.h`88->106/290. Clean sibling CNA is `a6bb279d47`; production library hashes remain
unchanged. No drawing, pixel result or visual inspection is claimed.
Evidence:`checkpoint-509-c-effect-core-focused-*`.

Focused **508-C-effect-passes-techniques** adds
`c_api.graphics.effect_passes_techniques` for all31 ownerless pass, technique and collection routes.
Twenty-seven controls prove copied names with atomic short/exact copies; nonzero distinct technique
identities; ordinary and reflected runtime indices; empty versus canonical-P0 construction;
separate pass/technique annotation views; documented ownerless no-op Apply; stable element views
after32 insertions; exact case-sensitive lookup and bounds; and nested P0 survival after root
collections, technique handles and nested collection views are destroyed. Direct execution passes,
SOFTWARE and OPENGLES3 runner output is byte-identical, and all three stderr files are empty.
Metadata is508/0, all ten focused C/API, C++ effect and CLI tests pass in6.55s, and the final
incremental build is clean. Coverage rises846->877/3205 and `effects.h`57->88/290. CNA advanced
externally from clean `fbd8e64a3` to clean `a6bb279d47` through two docs-only commits; production
library hashes remain unchanged. No effect-owned pass, game, graphics device, shader execution,
drawing or visual inspection is claimed.
Evidence:`checkpoint-508-c-effect-passes-techniques-focused-*`.

Focused **507-C-effect-parameter-collections** adds
`c_api.graphics.effect_parameter_collections` for all ten remaining parameter and parameter-
collection routes. Nineteen controls prove copied add-create metadata; a stable first element after
32 further insertions; shared mutable identity across index, name and semantic views; case-sensitive
name absence, case-insensitive first semantic matching and upper-bound refusal; distinct mutable
element, structure-member and annotation collections; and descendant survival after root handles
and then collection views are destroyed. ISSUE-068 records that the C header calls the deliberately
case-insensitive semantic match exact. Direct execution passes, SOFTWARE and OPENGLES3 runner
outputs are byte-identical, and all three stderr files are empty. Metadata is507/0, all nine focused
C/API, C++ effect and CLI tests pass in4.66s, and the final incremental build is clean. Coverage
rises836->846/3205 and `effects.h`47->57/290. Clean sibling CNA is `fbd8e64a3`; production library
hashes remain unchanged. No game, graphics device, shader execution, drawing or visual inspection
is claimed. Evidence:`checkpoint-507-c-effect-parameter-collections-focused-*`.

Focused **506-C-effect-parameter-values** adds `c_api.graphics.effect_parameter_values`. Thirty
controls cover copied and versioned parameter metadata; atomic short and exact unterminated text
copies; all nine tagged scalar and array value families; ordinary versus transposed non-symmetric
matrices; bounded array retrieval; owned string storage; and typed Texture2D/base-slot retention and
release using a CPU-only texture. Direct execution passes, runner output is byte-identical under
SOFTWARE and OPENGLES3, and all three stderr files are empty. Metadata is506/0, all eight focused
C/API, C++ effect and CLI tests pass in4.57s, and the final incremental build is clean. Coverage
rises819->836/3205, `effects.h`31->47/290 and `texture.h`1->2/21. Clean sibling CNA is
`fbd8e64a3`; production library hashes remain unchanged. Nothing creates a game or graphics device,
and no drawing or visual inspection is claimed. Evidence:
`checkpoint-506-c-effect-parameter-values-focused-*`.

Focused **505-C-effect-annotations** adds `c_api.graphics.effect_annotations` for all22 public
annotation and annotation-collection routes. Twenty-one controls prove copied non-ASCII UTF-8 name,
semantic, cached string and raw payload ownership; exact and atomic short text copies; complete
versioned shape/type metadata; Boolean, integer-bit, float, Vector2/3/4 and Matrix views; a separate
signed-integer value; insertion-order and exact case-sensitive lookup; collection value copying;
owned result copies surviving both source and collection destruction; bounds refusal and one-shot
release. Direct and runner executions produce identical output with empty stderr on SOFTWARE and
OPENGLES3. Metadata is505/0, all eight focused C/API, C++ reflection and CLI tests pass in9.04s,
and the final incremental build is clean. Coverage rises797->819/3205 and `effects.h`9->31/290.
Clean sibling CNA is `fbd8e64a3`; production library hashes remain unchanged. Nothing creates a
game or graphics device, and no drawing or visual inspection is claimed. Evidence:
`checkpoint-505-c-effect-annotations-focused-*`.

Complete **504-C-graphics-device**, source `cf105b2` (graphics source through `2746fcc`), absorbs
all five focused graphics batches after complete502. `tools/validate.sh` reports metadata504/0 and
passes all517 active CTests in413.76s; three declared tests remain disabled. SOFTWARE is437pass/
29known-fail/35skip/3not-run and OPENGLES3 is468/30/3/3. Comparison with complete502 shows both
new IDs at their expected outcomes, every existing status and known-issue value unchanged,
SOFTWARE adding exactly the one ISSUE-066 assertion (40->41), and OPENGLES3 retaining its exact44
failed-assertion multiset. Fresh runtime coverage is297/329 types,1405/2326 XNA members and
797/3205 C routes. The first full attempt exposed an intermittent global-desktop-pointer race in
`c_api.input.keyboard_mouse`; ten isolated repeats passed, and `RUN_SERIAL` now protects its
warp/read/restore interval. A targeted two-test run proves serial ordering, and the fresh complete
suite passes. CNA was clean `f37f8d184` for the18:09 build, then advanced externally during smoke
to clean `fbd8e64a3` through two CI/CMake-test/docs/tooling-only commits. Production `libcna.so`
and `libcna_c_api.so` remain SHA-256
`3b533283162bbcc5a0fe9a9f6c767f52f91eefe557dda2ee04d5f1cbdcc604e0` and
`730ac2631f65b402d5d016ae85649067441b52c973e49f786b49b3253d75100a`; `cna-demos` remains
`8ac27e51e2a40a9d12263883b0b6d9d6fdd93ac08a242601a2c282a55daa1c57`. No visual inspection was
performed. Evidence:`checkpoint-504-c-graphics-device-*`.

Focused **504-C-renderer-recreation** extends `c_api.graphics.owned_device` from20to23 controls
while retaining every prior label. With zero tracked GPU resources, a negative multisample request
is refused without presentation or event changes. A request for4 rebuilds the same renderer family,
preserves its capability bits, applies4 samples under both tested backends and raises no reset,
lost or disposing event; the old clear/present controls remain green. A second reconstruction
restores exact single-sample state before the existing reset workflow. SOFTWARE and OPENGLES3
produce byte-identical output with23 passing checks and empty stderr. Metadata remains504/0, and
all27 enabled focused lifecycle, multisampling, renderer-selection and CLI tests pass in11.94s;
one manual fullscreen test is disabled. One route raises coverage796->797/3205 and
`graphics_device.h`79->80/84. Clean sibling CNA is `f37f8d184`; production `libcna.so` and
`libcna_c_api.so` remain SHA-256
`3b533283162bbcc5a0fe9a9f6c767f52f91eefe557dda2ee04d5f1cbdcc604e0` and
`730ac2631f65b402d5d016ae85649067441b52c973e49f786b49b3253d75100a`. `cna-demos` is
`8ac27e51e2a40a9d12263883b0b6d9d6fdd93ac08a242601a2c282a55daa1c57` and the standalone is
`8a49e13217635efbcca4b232f6cace128eae724aac87e8f4ec2adb3084277447`. No displayed pixels or
visual inspection is claimed. Evidence:`checkpoint-504-c-renderer-recreation-focused-*`.

Focused **504-C-device-capabilities** extends `c_api.graphics.device_capabilities` from20to24
controls. All27 direct render-target-format verdicts agree with the detailed supported-usage masks.
SOFTWARE reports no shader-source execution and no IBL; OPENGLES3 reports both. All three display-
color-space probes preserve the original state; set results agree with support, any successful
alternate would be restored, and undefined ordinal3 is refused without changing output or state.
Both report current sRGB0 and support `1/0/0` for sRGB/scRGB/HDR10. Direct and runner executions
pass with empty stderr. Metadata remains504/0, and all37 focused capability, presentation, render-
target, PBR/shader and CLI tests pass in17.95s with11 declared SOFTWARE skips. Six routes raise
coverage790->796/3205 and `graphics_device.h`73->79/84. ISSUE-067 records the missing public C type
and names for those accepted color-space ordinals. Clean sibling CNA is `f37f8d184`; production
`libcna.so` and `libcna_c_api.so` remain SHA-256
`3b533283162bbcc5a0fe9a9f6c767f52f91eefe557dda2ee04d5f1cbdcc604e0` and
`730ac2631f65b402d5d016ae85649067441b52c973e49f786b49b3253d75100a`. `cna-demos` is
`dc5069f58016d555e831129813238f7ec6a3b63660c0ea4ccba86c623d8a3f7e` and the standalone is
`b756c44fdfad6b813cdd0a2b07a43cdfe1dec390216b6b8d6826245c339b18ce`. No rendering or visual
inspection is claimed. Evidence:`checkpoint-504-c-device-capabilities-focused-*`.

Focused **504-C-content-loss** extends `c_api.graphics.render_target_bindings` from22to25 controls.
After restoring the backbuffer, explicit device notification marks both tracked2D/cube targets lost
and synchronously raises the cube's subscribed ContentLost event once. Rebinding only the2D target
clears only its flag; the cube remains lost until its own successful face bind, which clears it
without another event. The delivered registration still releases once. SOFTWARE and OPENGLES3
produce byte-identical output with25 passing checks and empty stderr. Metadata remains504/0, and
all28 focused C/API, render-target, dynamic-buffer and CLI tests pass in30.37s with one declared
SOFTWARE MRT skip. One route raises coverage789->790/3205 and `graphics_device.h`72->73/84. Clean
sibling CNA is `f37f8d184`; production `libcna.so` and `libcna_c_api.so` remain SHA-256
`3b533283162bbcc5a0fe9a9f6c767f52f91eefe557dda2ee04d5f1cbdcc604e0` and
`730ac2631f65b402d5d016ae85649067441b52c973e49f786b49b3253d75100a`. `cna-demos` is
`ad02769b94813ab72fcdf002249f9bacbb01892af510fed586ed21fbf51dec84` and the standalone is
`a1466b2c9606ae6cf3fbea538e39bb9d5693475e150c27a69beb71770fc9228f`. This deterministic
route does not claim an actual platform context loss or visual inspection. Evidence:
`checkpoint-504-c-content-loss-focused-*`.

Focused **504-C-occlusion-query** adds `c_api.graphics.occlusion_query` for all eight public query
routes. Twenty-three controls prove a fresh owned HiDef query's renderer, precision and unavailable-
result state; Begin/End pairing and the observation-before-reuse rule; a real full-screen red draw;
bounded completion; exact-versus-boolean result interpretation; and one-shot destruction with
stale-output preservation. The center-pixel readback is exact red. SOFTWARE reports a precise
384000 samples for the800x480 buffer; OPENGLES3 reports its documented imprecise mode and one.
Both direct and runner executions pass with empty stderr. Metadata is504/0, all19 focused query,
capability and CLI tests pass in9.96s, and the final incremental build is clean. Eight routes raise
coverage781->789/3205 and `graphics_device.h`64->72/84. Clean sibling CNA is `f37f8d184`;
production `libcna.so` and `libcna_c_api.so` remain SHA-256
`3b533283162bbcc5a0fe9a9f6c767f52f91eefe557dda2ee04d5f1cbdcc604e0` and
`730ac2631f65b402d5d016ae85649067441b52c973e49f786b49b3253d75100a`. `cna-demos` is
`8352a8a9708b8d224e4800213679f5be55f8be5caf43c53434c860ddccf857f7` and the standalone is
`476eda8e5d56d4412d5a87020ec85f881ca979a33da96ecbe86bbc23f09ab60c`. Pixels were read
programmatically; no screenshot or visual inspection is claimed. Evidence:
`checkpoint-504-c-occlusion-query-focused-*`.

Focused **503-C-draw-routes** adds `c_api.graphics.draw_routes` for all five remaining draw-family
routes, the primitive-count helper, current-effect selection and all three direct pipeline toggles.
Twenty-four controls pixel-prove all five primitive topologies; bound, caller-vertex, caller-indexed
and repeated-instance geometry; explicit current-effect refusal/selection; blend enable/disable;
and depth-test/depth-write behavior. OPENGLES3 passes24/24. SOFTWARE passes23 controls and retains
one expected ISSUE-066 failure: two half-alpha instances overwrite to `(255,0,0,128)` because its
direct blend toggle is a no-op, while OPENGLES3 accumulates near `(192,0,0,159)`. Both stderr files
are empty, each batch wrapper accepts exactly its declared outcome, and all19 focused C/API, C++
primitive/state and CLI tests pass in23.69s. Metadata is503/0 and the final incremental build is
clean. Ten routes raise coverage771->781/3205, `graphics_device.h`55->64/84 and `effects.h`8->9/290.
Clean sibling CNA is `f37f8d184`; production `libcna.so` and `libcna_c_api.so` remain SHA-256
`3b533283162bbcc5a0fe9a9f6c767f52f91eefe557dda2ee04d5f1cbdcc604e0` and
`730ac2631f65b402d5d016ae85649067441b52c973e49f786b49b3253d75100a`. `cna-demos` is
`567f8d18e29a483c70892ad8393d97d8791fa0d62c022640bcd41d8107cff19f` and the standalone is
`669d00dcceb47a49a318d44aa8788e34d5ba044289068b237ca3d69bd04cf6d0`. Pixels were read
programmatically; no screenshot or visual inspection is claimed. Evidence:
`checkpoint-503-c-draw-routes-focused-*`.

Complete **502-C-device-policy**, source `1398d9d`, clears the device-clears, owned-device and policy
batches after complete500. Full `tools/validate.sh` passes metadata502/0 and all515 enabled CTests
in208.32s; three declared tests remain disabled. SOFTWARE is436/28/35/3 and OPENGLES3 is
466/30/3/3, with every failure expected. Against complete500, both new IDs pass, every older ID
retains status and known-issue metadata, and the failed-assertion multisets remain exactly40/44;
stderr changes only in generated temporary-directory identities. The extended `device_states`
child passes25 focused controls under both renderers. Coverage is297/329 types,1405/2326 XNA
members and771/3205 C routes. CNA was clean at `d7eff66ee` for the17:07 build, then advanced
externally during smoke through test/tooling-only commits to clean `f37f8d184`; no CNA production
binary was rebuilt and `libcna.so` remains SHA-256
`3b533283162bbcc5a0fe9a9f6c767f52f91eefe557dda2ee04d5f1cbdcc604e0`.
`cna-demos` is `c30cde5e514948d8b40b6265dc26aed6728f1261fbe5a52d389a4c3f42ff5a7b` and
`libcna_c_api.so` remains `730ac2631f65b402d5d016ae85649067441b52c973e49f786b49b3253d75100a`.
No visual inspection was performed. Evidence:`checkpoint-502-c-device-policy-*`.

Focused **502-C-device-policy** extends `c_api.graphics.device_states` from22to25 controls. Its live
HiDef device begins with the strict unsupported-3D policy; the demo changes the profile to Reach and
the policy to warn-and-stub, reads both back, and proves undefined enum identities are rejected
without mutation. It restores and re-reads the exact borrowed profile and policy before continuing
the existing complete state workflow. SOFTWARE and OPENGLES3 produce byte-identical child output
with25 passing checks and empty stderr. Metadata remains502/0, seven focused C/API, profile,
capability and CLI regressions pass in5.07s, and the incremental full build is clean. Three routes
raise coverage768->771/3205 and `graphics_device.h`52->55/84. Clean sibling CNA is `d7eff66ee`;
`cna-demos` is SHA-256 `c30cde5e514948d8b40b6265dc26aed6728f1261fbe5a52d389a4c3f42ff5a7b`,
the standalone is `870246f7c0ad0b3883f168fa4824ad4a6d864d27510cb875869ca4a468089625`, and
production `libcna.so` remains byte-identical. Nothing is drawn and no visual inspection is
claimed. Evidence:`checkpoint-502-c-device-policy-focused-*`.

Focused **502-C-owned-device** adds `c_api.graphics.owned_device` for caller-owned graphics-device
creation/destruction, manual presentation, both reset forms and no-payload lifecycle subscriptions.
Twenty controls create two distinct HiDef devices outside any Game; query the first; subscribe its
resetting/reset/lost/disposing events; clear and present it; and prove both ordinary and changed-
parameter resets synchronously raise the exact resetting-then-reset order. The changed400x300
presentation is read back field for field, while a null reset is refused before any callback. Reset
and lost registrations release once; destroying the second device leaves the first live; destroying
the first raises one exact disposing callback that safely releases its own registration before the
native event source dies. Both selections produce byte-identical child output with empty stderr.
Metadata is502/0 and the exact demo plus presentation/state/manager-event and CLI regressions pass
8/8. Six routes raise coverage762->768/3205 and `graphics_device.h`46->52/84. The source audit also
records passing documentation defect CNA-DEMOS-ISSUE-065. Clean sibling CNA is `d7eff66ee`;
`cna-demos` is SHA-256 `4b38fbea8cd380e7ee0b572c5abb4a90f9c59f8ce3cffa4485596a1b87e1e926`,
the standalone is `f7cf7fed096255010f98fe890de3e527f8ebd4b633b02484ef29e9755bf154a0`, and
production `libcna.so` remains byte-identical. No displayed pixels, screenshot or visual inspection
is claimed. Evidence:`checkpoint-502-c-owned-device-focused-*`.

Focused **501-C-device-clears** adds `c_api.graphics.device_clears` for all three graphics-device
clear routes and windowed backbuffer readback. Its explicit HiDef Color/Depth24Stencil8 back buffer
lets24 controls clear normalized RGBA red, Color-plus-depth green and all three selected planes
blue. Full RGBA8 readback after each accepted operation samples both pairs of corners and the center
exactly. Non-finite channels/depth, an unknown option bit and an empty accepted mask also prove
refusal/no-op preservation of the last complete color result. A2x2 source window writes four blue
pixels at destination offset2 while preserving both guards; one-pixel-short and malformed requests
preserve every caller byte. SOFTWARE and OPENGLES3 produce identical child output with empty
stderr. Metadata is501/0, the exact demo and four related C++ clear/readback/depth demos plus three
CLI tests all pass (8/8), and the incremental full build is clean. Four routes raise coverage
758->762/3205 and `graphics_device.h`42->46/84. Clean sibling CNA is `d7eff66ee`; `cna-demos` is
SHA-256 `2d654a671d23426eb9a64fe61c67bc1ba522c5c37852717a77e4d07e429e2477`, the standalone is
`3f528b55b4c6c608fda8fe3669f6c64e5fcd5861b68c5959df661e61d6c0677e`, and production
`libcna.so` remains `3b533283162bbcc5a0fe9a9f6c767f52f91eefe557dda2ee04d5f1cbdcc604e0`.
No screenshot or visual inspection is claimed. Evidence:`checkpoint-501-c-device-clears-focused-*`.

Complete **500-C-buffer-bindings**, source `dee3837`, clears all five focused graphics batches
after complete497. Full `tools/validate.sh` passes metadata500/0 and all513 enabled CTests in
170.48s; three declared tests remain disabled. SOFTWARE is434/28/35/3 and OPENGLES3 is
464/30/3/3, with every failure expected. Against complete497, the three new IDs
`c_api.graphics.viewport`, `c_api.graphics.texture_slots` and
`c_api.graphics.buffer_bindings` all pass, every older ID retains status and known-issue metadata,
and the failed-assertion multisets remain exactly40/44. The extended device-state and
resource-lifecycle children pass22 and30 controls on both renderers. Coverage is297/329 types,
1405/2326 XNA members and758/3205 C routes. Sibling CNA stayed clean and unchanged at `d7eff66ee`;
production `libcna.so` remains SHA-256
`3b533283162bbcc5a0fe9a9f6c767f52f91eefe557dda2ee04d5f1cbdcc604e0`. No visual inspection is
claimed. Evidence:`checkpoint-500-c-buffer-bindings-*`.

Focused **500-C-buffer-bindings** adds `c_api.graphics.buffer_bindings` for all eight device buffer-
binding routes. Twenty controls start with empty vertex/index bindings; create two real vertex
buffers sharing one declaration plus one index buffer; prove the single and offset overloads;
round-trip an ordered two-stream array with distinct offsets and instance frequency; prove atomic
short-copy and whole-array validation; apply/read/clear the exact index handle; and show that
destroying a bound vertex owner detaches the whole multi-stream set while destroying the bound index
owner clears its slot. SOFTWARE and OPENGLES3 produce identical output with empty stderr. Metadata
is500/0, all76 focused C/API, C++ graphics device/resource/buffer and CLI tests pass in59.02s, and a
clean1699-step full build follows. The simple vertex/index setters were already used by the pixel-
validated BasicEffect cube; six newly named routes raise C coverage752->758/3205 and
`graphics_device.h`36->42/84. Clean sibling CNA is `d7eff66ee`; `cna-demos` is SHA-256
`1d9c8b68182c4061f7cac667483e956946121265ba6f139e7daf3501850bc708`, the standalone is
`8f876e91eb123e7dd6a7f589a4810132d21d64eed841ecf6e85d547109946d7f`, and production
`libcna.so` remains `3b533283162bbcc5a0fe9a9f6c767f52f91eefe557dda2ee04d5f1cbdcc604e0`.
Nothing is drawn and no visual inspection is claimed. Evidence:
`checkpoint-500-c-buffer-bindings-focused-*`.

Focused **499-C-texture-slots** adds `c_api.graphics.texture_slots` for the three device texture-
collection routes. Sixteen controls start from two empty high pixel slots on a fresh HiDef device;
bind one owned Color Texture2D and read its exact occupancy/handle; clear one slot with the invalid
handle; bind the same texture in both slots; clear both through whole-device unbinding; prove that
repeat unbinding is an idempotent no-op; then rebind and prove typed destruction clears the slot and
invalidates the stale handle. Every versioned slot result has exact size/version and zero reserved
bytes. SOFTWARE and OPENGLES3 produce identical output with empty stderr. Metadata is499/0, all68
focused C/API, C++ graphics-device/resource and CLI tests pass in40.15s, and a clean1696-step full
build follows. Three routes raise C coverage749->752/3205 and `graphics_device.h`33->36/84. Clean
sibling CNA is `d7eff66ee`; `cna-demos` is SHA-256
`bc84e274a19d5359ebd7f1748ec8bcfaac15d27bb91e2d4894c93079b9268857`, the standalone is
`25f0d8e79fe668d8beb5f8711eda24296fba148d7ad3e0c7cb28a2ebe7b433d6`, and production
`libcna.so` remains `3b533283162bbcc5a0fe9a9f6c767f52f91eefe557dda2ee04d5f1cbdcc604e0`.
Nothing is drawn and no visual inspection is claimed. Evidence:
`checkpoint-499-c-texture-slots-focused-*`.

Focused **498-C-resource-events** extends `c_api.graphics.resource_lifecycle` from25to30 controls
without adding an ID. Before creating its real Texture2D, the callback records the tracked-resource
baseline and subscribes to both device resource events. Creation synchronously supplies a complete
versioned presence payload after the tracker grows by one. Explicit disposal first raises the
resource's existing Disposing callback, then supplies the exact non-ASCII UTF-8 name while the
resource is still tracked; the native-tag flag stays false because the demonstrated full-width C
tag token belongs to the C handle table. The count then returns to baseline. Both registrations
release once, refuse stale handles and silence a second create/destroy cycle. SOFTWARE and
OPENGLES3 produce identical30-check output with empty stderr. Metadata is498/0, all67 focused
C/API, C++ graphics-device/resource and CLI tests pass in39.99s, and a clean1693-step full build
follows. Four routes raise C coverage745->749/3205 and `graphics_device.h`29->33/84. Clean sibling
CNA is `d7eff66ee`; `cna-demos` is SHA-256
`6107234b854365a68f0da105e20c30dad0f98a6c471e5d1b2e54574e11c63af4`, the extended standalone is
`299010d12d27963d6528b7534f3caae2956437cf3200a784e873670e7e853c82`, and production
`libcna.so` remains `3b533283162bbcc5a0fe9a9f6c767f52f91eefe557dda2ee04d5f1cbdcc604e0`.
Nothing is drawn and no visual inspection is claimed. Evidence:
`checkpoint-498-c-resource-events-focused-*`.

Focused **498-C-device-values** extends `c_api.graphics.device_states` from19to22 controls without
adding an ID. The callback proves the live game device is undisposed, Normal, HiDef and owned by a
currently enumerated adapter; its borrowed handle refuses direct disposal and remains live. It
saves, applies and exactly reads a distinctive scissor rectangle, blend factor, multisample mask
and reference stencil alongside the existing complete object and sampler states, then restores and
re-reads every original value. SOFTWARE and OPENGLES3 produce identical output with empty stderr.
Metadata is498/0, all83 focused C/API, C++ graphics-device/state and CLI tests pass in49.86s, and a
clean build follows. Thirteen routes raise total C coverage732->745/3205 and
`graphics_device.h`16->29/84. Clean sibling CNA is `d7eff66ee`; `cna-demos` is SHA-256
`3fedc1edb7fa5d6a5141d207d27db937414f11a225240905dfe9a9ffeb9b767e`, the extended standalone is
`a485768ec03ccd4362ceeeef53a3ef197a8214c35735ec9dcbc13d67cc38f60e`, and production
`libcna.so` remains `3b533283162bbcc5a0fe9a9f6c767f52f91eefe557dda2ee04d5f1cbdcc604e0`.
Nothing is drawn and no visual inspection is claimed. Evidence:
`checkpoint-498-c-device-values-focused-*`.

Focused **498-C-viewport** adds `c_api.graphics.viewport` for all11 Viewport value routes and the
live device get/set pair in `graphics_device.h`. Nineteen controls distinguish the all-zero XNA
default value from the parameterized constructors'0..1 depth range; derive aspect, bounds and
desktop safe area; preserve depth while changing bounds; round-trip identity and perspective
projection; and prove canonical text's exact/atomic count-copy contract. The callback reads the
live viewport, applies and exactly observes a distinctive finite value, refuses infinity without
mutation, then restores and re-reads every original field. SOFTWARE and OPENGLES3 produce
identical output with empty stderr. Metadata is498/0, all61 focused C/API, C++ Viewport,
split-screen and CLI tests pass in35.43s, and a clean build follows. Thirteen routes raise
`graphics_device.h`3->16/84; one newly named matrix identity route makes total C coverage
718->732/3205. The default-value evidence also exposes passing documentation mismatch
CNA-DEMOS-ISSUE-064. Clean sibling CNA is `d7eff66ee`; production `libcna.so` remains SHA-256
`3b533283162bbcc5a0fe9a9f6c767f52f91eefe557dda2ee04d5f1cbdcc604e0`. Nothing is drawn and no
visual inspection is claimed. Evidence:`checkpoint-498-c-viewport-focused-*`.

The post-complete497 `media_player.h` audit confirms exactly three uncredited routes and the
existing semantic decisions. `cna_media_player_update_ext` is still deferred until a host-driven
example can observe a native track-end transition outside the automatic game loop; calling it
while stopped would demonstrate no behavior. The queue type-name size/copy pair is fixed generic
reflection text and remains intentionally trivial/uncredited. Every route in that header now has
a recorded disposition; no demo or coverage count was added for this audit.

Complete **497-C-sprite-states**, source `cfed648`, clears all seven focused graphics batches after
complete492. Full `tools/validate.sh` passes metadata497/0 and all510 enabled CTests in308.90s;
three declared tests remain disabled. SOFTWARE is431/28/35/3 and OPENGLES3 is461/30/3/3, with
every failure expected. Against complete492, all five new IDs pass, every older ID retains status
and known-issue metadata, and the failed-assertion multisets remain exactly40/44. The existing C
SpriteBatch child output grows43->61 with all new checks passing. Coverage is297/329 types,
1405/2326 XNA members and718/3205 C routes. The run started at clean CNA `e67c54936`; two concurrent
documentation-only commits advanced it to clean `d7eff66ee` without changing the production CNA or
C API library hashes. No visual inspection is claimed.
Evidence:`checkpoint-497-c-sprite-states-*`.

Focused **497-C-sprite-states** extends the existing `c_api.graphics.sprite_batch` workflow from43
to61 controls without adding an ID. One explicit-state interval uses Opaque, PointClamp, None and
CullNone; exact backbuffer read-back proves a half-alpha red source replaces the clear colour
without blending. A second interval selects the default effect and a nonidentity matrix; its green
command leaves the original position clear and appears exactly sixty pixels to the right. Both
routes share one owned texture and batch, whose successful End and final destruction remain
checked. SOFTWARE and OPENGLES3 produce identical output with empty stderr. Metadata is497/0,
all76 focused C/API, C++ SpriteBatch and CLI tests pass, and a clean follow-up build passes. These
last two routes raise C coverage716->718/3205 and complete `graphics_state.h` at14/14. The build
used clean docs-only CNA `e67c54936`, `cna-demos` SHA-256
`42329561d73f16d7091a26452a9fdb820f3beed355a2089ebca06054e8664d29` and production
`libcna.so` SHA-256 `3b533283162bbcc5a0fe9a9f6c767f52f91eefe557dda2ee04d5f1cbdcc604e0`.
No screenshot or visual inspection is claimed. Evidence:`checkpoint-497-c-sprite-states-focused-*`.

Focused **497-C-device-states** adds `c_api.graphics.device_states` for the ten uncovered preset and
live-device routes in `graphics_state.h`. Nineteen controls compare every field produced by all5
blend,3 depth/stencil,4 rasterizer and7 sampler presets; save the live blend, depth, rasterizer and
pixel/vertex sampler-slot3 states; apply and exactly read back distinctive complete values; prove
the two sampler stages remain independent; and restore/re-read every original field before leaving
the callback. SOFTWARE and OPENGLES3 produce identical output with empty stderr. Metadata is497/0,
the exact CTests and all74 focused C/API, C++ graphics-state and CLI tests pass, and a clean build
follows. Ten routes raise C coverage706->716/3205 and `graphics_state.h`2->12/14. The remaining two
state- and effect-bearing SpriteBatch Begin routes are now pixel-validated by the focused
497-C-sprite-states extension. The build used clean docs-only CNA `e67c54936` and
production `libcna.so` SHA-256
`3b533283162bbcc5a0fe9a9f6c767f52f91eefe557dda2ee04d5f1cbdcc604e0`. No drawing or visual
inspection is claimed. Evidence:`checkpoint-497-c-device-states-focused-*`.

Focused **496-C-index-buffer** adds `c_api.graphics.index_buffer` for the five remaining meaningful
routes in `index_resources.h`. Eighteen controls create a dynamic four-entry16-bit buffer; validate
its complete applied state; upload from a nonzero caller-array window; prove atomic short-buffer
read-back and an exact shifted destination; and patch two indices at a four-byte buffer offset while
preserving both untouched entries. A ContentLost registration remains silent during ordinary
transfers on the current non-device-losing renderers, releases once and then refuses its stale
handle. SOFTWARE and OPENGLES3 produce identical output with empty stderr. Metadata is496/0, the
exact CTests and all59 focused C/API, C++ IndexBuffer and CLI tests pass in the documented isolated
X11 environment, and a clean build follows. Five routes raise C coverage701->706/3205 and
`index_resources.h`3->8/10; its fixed type-name size/copy pair is trivial/uncredited, so every route
has a semantic disposition. The first diagnostic accidentally retained host Wayland despite
`xvfb-run`; it exposed the adapter demo's stale 4:3 widescreen cross-check at exact16:10, corrected
separately in `46c327d`. The restored X11 mouse path passes unchanged. Sibling CNA later advanced
externally to clean docs-only `e67c54936`; the exercised production `libcna.so` remains SHA-256
`3b533283162bbcc5a0fe9a9f6c767f52f91eefe557dda2ee04d5f1cbdcc604e0`. No drawing, device-loss
notification or visual inspection is claimed. Evidence:`checkpoint-496-c-index-buffer-focused-*`.

Focused **495-C-resource-lifecycle** adds `c_api.graphics.resource_lifecycle` and demonstrates all12
routes in `graphics_resource.h` through a real device-owned Texture2D. Twenty-five controls match
the callback-scoped owning device; distinguish empty Name from the unnamed `Texture2D` string;
prove atomic size/copy behavior; round-trip a non-ASCII UTF-8 name and a full-width opaque tag;
refuse embedded NUL without mutation; and establish synchronous one-shot Disposing delivery.
The callback observes the resource live, the still-valid C handle then observes it disposed,
repeated disposal is a no-op, the registration releases once, and typed destruction finally
invalidates the handle. SOFTWARE and OPENGLES3 produce identical output with empty stderr.
Metadata is495/0, the exact CTest and all56 C/API, GraphicsResource and CLI regression tests pass,
and a clean build follows. Twelve routes raise C coverage689->701/3205 and complete
`graphics_resource.h` at12/12. During validation sibling CNA advanced externally to clean
test-only `b45024c79`; production `libcna.so` stayed byte-identical at SHA-256
`3b533283162bbcc5a0fe9a9f6c767f52f91eefe557dda2ee04d5f1cbdcc604e0`. No rendering or visual
inspection is claimed. Evidence:`checkpoint-495-c-resource-lifecycle-focused-*`.

Focused **494-C-sprite-scaled** extends the existing `c_api.graphics.sprite_batch` workflow from30
to43 controls without adding a demo ID. A two-command position/scale array selects cyan and magenta
source texels. Exact readback proves the first command's nonzero origin moves its 12x8 result before
the named position and the second starts at its position with a6x14 nonuniform scale. An invalid
second handle rejects the whole earlier array before its valid first command draws; the accepted
array retains its texture until successful End, then releases it while the pixels remain. SOFTWARE
and OPENGLES3 produce identical43-control output with empty stderr. Metadata remains494/0, the
exact CTest passes, all73 C/API, SpriteBatch and CLI regression tests pass, and a clean build follows.
The route raises C coverage688->689/3205 and `graphics.h`24->25/27. Its two remaining fixed
SpriteBatch type-name helpers are trivial/uncredited, giving every route in the header a semantic
disposition. No screenshot or visual inspection is claimed. Evidence:
`checkpoint-494-c-sprite-scaled-focused-*`.

Focused **494-C-device-capabilities** adds `c_api.graphics.device_capabilities` for the ten
remaining meaningful graphics-device introspection routes in `graphics.h`. Twenty controls copy
the exact renderer name atomically; cross-check all17 legacy capability queries against the
summary bit set; enumerate every live and reserved detailed feature and limit identity; validate
all27 surface-format masks; copy the English diagnostic report atomically; and relate the shader
dialect and complete logical backbuffer information to the applied presentation state. SOFTWARE
reports15 supported detailed features, dialect Unknown and a6427-byte report; OPENGLES3 reports20,
GLSL ES and6423 bytes. Both pass every control with empty stderr. Metadata is494/0, the exact CTest
passes, and all67 active tests in the Xvfb-backed68-test C/API/device/device-manager/CLI selection
pass; fullscreen remains disabled. Ten routes raise C coverage678->688/3205 and `graphics.h`
14->24/27. The two fixed SpriteBatch type-name helpers remain trivial/uncredited. The sibling CNA
advanced externally from the complete baseline's `440af2a` to clean docs-only `3b5755944`; this
focused build used `libcna.so` SHA-256 `3b533283162bbcc5a0fe9a9f6c767f52f91eefe557dda2ee04d5f1cbdcc604e0`.
No pixels or visual inspection is claimed. Evidence:`checkpoint-494-c-device-capabilities-focused-*`.

Focused **493-C-render-target-bindings** adds `c_api.graphics.render_target_bindings` and completes
all12 routes in `render_target.h`. It classifies all three content-usage values, creates matching
bindable2D and cube targets, reads their complete applied state, binds them as a real two-entry MRT
array and round-trips the normalized order and cube face. A one-entry destination reports the full
count without mutation; a single-cube bind replaces the array and an empty array restores the
backbuffer. ContentLost subscription/unsubscription is demonstrated as a silent lifecycle on these
non-device-losing renderers, without claiming event delivery. SOFTWARE and OPENGLES3 pass
identical22-control output with empty stderr; metadata is493/0 and the exact CTest passes. The
Xvfb-backed68-test C/API/render-target/device-manager/CLI selection has no failures:66pass, the C++
custom-effect MRT scene declares a SOFTWARE skip, and fullscreen remains disabled. Nine routes
raise C coverage669->678/3205 and `render_target.h`3->12/12. No pixels or visual inspection is
claimed. Evidence:`checkpoint-493-c-render-target-bindings-focused-*`.

Complete **492-C-device-presentation**, source `808b274`, clears both focused display batches. Full
`tools/validate.sh` passes metadata492/0 and all505 enabled CTests in258.17s; three declared tests
remain disabled. SOFTWARE is426/28/35/3 and OPENGLES3 is456/30/3/3, with every failure expected.
Against complete490, only `c_api.graphics.adapter_capabilities` and
`c_api.graphics.device_presentation` are new, both pass, every older ID retains status and
known-issue metadata, and the failed-assertion multisets remain exactly40/44. Coverage is297/329
types,1405/2326 XNA members and669/3205 C routes, including `display.h`22/22. The exercised sibling
CNA is still source commit `440af2a`; its unrelated uncommitted `handoff.md` documentation addition
was preserved. No visual inspection is claimed. Evidence:`checkpoint-492-c-device-presentation-*`.

Focused **492-C-device-presentation** adds `c_api.graphics.device_presentation` and completes all22
routes in `display.h`. A callback-scoped device returns a complete applied presentation value; the
demo changes its presentation interval, reads the exact change with every other public field
preserved, then restores and verifies every original public field before leaving the callback. It
also reads a coherent current device mode and establishes the device-level zero-output
NOT_SUPPORTED native-window contract. SOFTWARE and OPENGLES3 pass the same12 checks with empty
stderr; their expected renderer-specific mode observations are800x480 and1280x1024. Metadata is492/0,
the exact CTest passes, and the Xvfb-backed60-test C/API/adapter/device-manager/CLI selection passes
all59 enabled tests with one declared fullscreen test disabled. Four routes raise C
coverage665->669/3205 and `display.h`18->22/22. No pixels, native handle or visual inspection is
claimed. Evidence:`checkpoint-492-c-device-presentation-focused-*`.

Focused **491-C-adapter-capabilities** adds `c_api.graphics.adapter_capabilities` for the six
remaining adapter capability, format, preference and explicit-limitation routes in `display.h`.
The default adapter reports Reach support and exact baseline Color/None/0 render-target and
backbuffer selections with complete versioned output. The demo changes one device preference,
observes the pair, restores both original flags and confirms the restoration before creating a
device. It also establishes zero-output NOT_SUPPORTED monitor handles and callback-scoped adapter
refresh. SOFTWARE and OPENGLES3 pass identical16-control output with empty stderr; metadata is491/0,
the exact CTest passes, and the Xvfb-backed59-test C/API/adapter/device-manager/CLI selection passes
all58 enabled tests with one declared fullscreen test disabled. A no-display diagnostic run failed
only the existing desktop clipboard and mouse/cursor checks; the Xvfb rerun distinguishes that
environment requirement from a demo or CNA regression. Six routes raise coverage659->665/3205 and
`display.h`12->18/22. No monitor handle, adapter refresh, rendering or visual result is claimed.
After the runtime evidence, the sibling CNA acquired an unrelated uncommitted `handoff.md`
documentation addition at the same `440af2a` HEAD; cna-demos did not edit or revert it, and the
exercised CNA libraries remain the ones hashed by the checkpoint. Evidence:
`checkpoint-491-c-adapter-capabilities-focused-*`.

Complete **490-C-adapter-snapshot**, source `ed04492`, clears the sixteen-batch checkpoint debt.
The current clean sibling CNA at `440af2a` required one cna-demos compatibility correction: two
unused capability placeholders removed upstream were still referenced by framework metadata and
the capability listing demo. Removing those stale names leaves every current public capability in
the demo and restores two diagnostic-free standard builds. Full `tools/validate.sh` passes
metadata490/0 and all503 enabled CTests in224.48s; three declared tests remain disabled. SOFTWARE
is424/28/35/3 and OPENGLES3 is454/30/3/3, with every failure expected. Comparison with complete476
keeps all476 prior ID statuses and known-issue metadata, adds13 passing C API IDs and the documented
`c_api.devices.sensor_lifecycle` ISSUE-062 failure, and changes the failed-assertion totals only by
its two disposed Accelerometer/Gyroscope checks:38->40 and42->44. Coverage is297/329 types,
1405/2326 XNA members and659/3205 C routes. No visual inspection is claimed. Evidence:
`checkpoint-490-c-adapter-snapshot-*`.

Focused **490-C-adapter-snapshot** adds `c_api.graphics.adapter_snapshot` for all seven read-only
adapter routes in `display.h`. A game handle before its first frame enumerates the one Xvfb adapter,
copies exact nonempty description/device-name bytes, proves atomic short-buffer refusal, reads a
coherent current mode, copies the complete and Color-filtered mode collections and refuses the
index-at-count through every snapshot route before changing output. Both renderer selections pass
identical15-control output with empty stderr; this environment reports `screen`, `\\.\DISPLAY1`
and one1280x1024 Color mode. Direct C/C++ compilation is clean, metadata is490/0 and the focused
CTest passes. A58-test C/API, adapter, device-manager and CLI regression selection passes all57
enabled tests with the declared fullscreen test disabled. Seven routes raise current C coverage
652->659/3205 and `display.h`5->12/22. Host names/modes are observations, and no rendering or visual
validation is claimed. Evidence:checkpoint-490-c-adapter-snapshot-focused-*.

Focused **489-C-display-values** adds `c_api.graphics.display_values`, a renderer-independent C
program for the five pure value routes in `display.h`. Eight controls establish full versioned
`DisplayMode` initialization, 16:9 and zero-height aspect ratios, equality over the defining fields,
complete presentation defaults, native clone behavior, derived bounds and refusal before output
mutation. Source inspection and runtime agree that the native XNA clone preserves its XNA fields
but resets CNA's non-XNA `headless_ext`; this is the documented contract, not a CNA bug. Both
renderer selections pass identical output with empty stderr, direct C/C++ compilation is clean,
metadata is489/0 and the exact focused CTest passes. A57-test C/API, adapter, device-manager and CLI
regression selection passes all56 enabled tests with the one declared fullscreen test disabled.
Five routes raise current C coverage647->652/3205 and establish `display.h`5/22. No graphics device,
window, adapter query or visual validation is claimed. Evidence:
checkpoint-489-c-display-values-focused-*.

Focused **488-C-clipboard-acceptance** extends `c_api.input.clipboard` without adding an ID. Its
explicit multibyte write now uses the device-layer setter and requires `accepted=true`; the other14
controls retain exact read-back, atomic short-buffer refusal, empty text and captured-value
restoration, with later plain-setter calls preserved. Both renderer selections pass identical
15-control output with empty stderr, metadata remains488/0 and the exact focused CTest passes. One
route raises current C coverage646->647/3205 and `devices.h`58->59/62. The header audit is now
semantically complete: real URL launch, real tray creation and real camera creation remain manual
because they cross into another application, external host UI or physical hardware/permission.
Refusal-only controls would not demonstrate those behaviors. Evidence:
checkpoint-488-c-clipboard-acceptance-focused-*.

Focused **488-C-camera-frames** adds `c_api.devices.camera_frames` and covers fourteen of fifteen
camera routes without opening hardware. It enumerates the actual descriptor/name snapshot and
upper bound, then uses CNA's synthetic camera for Opening/Denied/Lost states, input refusal, exact
2x2 RGBA transfer into a caller-owned texture, size mismatch/no-frame false, clear and independent
texture lifetime. Both renderer selections pass identical23-control output with empty stderr.
Direct C/C++ compilation is diagnostic-free and metadata is488/0. Fourteen routes raise current C
coverage632->646/3205 and `devices.h`44->58/62. All70 exact input/device/C-API/CLI CTests pass in
221.19s with six declared skips and one known-issue expected failure. ISSUE-063 records that the C
comments promise Closed after synthetic creation and clear while implementation, CNA smoke and
runtime use Opening and refuse Closed. Real
`cna_camera_create` remains manual; no permission prompt, capture, preview, display or visual
inspection is claimed. The unchanged external SDL_GPU mismatch still blocks the standard/full
checkpoint. Evidence:checkpoint-488-c-camera-frames-focused-*.

Focused **487-C-system-tray** adds `c_api.devices.system_tray` and covers eleven of twelve tray
routes in `devices.h` without creating host UI. It reads real availability, then uses CNA's
per-handle in-memory tray for owned lifetime, tooltip/menu indices, checked/enabled state, label
mutation, canonical ignored upper-bound changes/false reads, callback selection, silent no-handler
activation, invalid click bounds and stale handles. Both renderer selections pass identical
16-control output with empty stderr. Direct C/C++ compilation is diagnostic-free and metadata
is487/0. Eleven routes raise current C coverage621->632/3205 and `devices.h`33->44/62. All69 exact
input/device/C-API/CLI CTests pass in198.53s with six declared skips and one known-issue expected
failure. Real `cna_system_tray_create` remains manual because successful execution creates an
external icon. No icon appearance, native click delivery or callback thread is claimed. The
unchanged external SDL_GPU mismatch still blocks the
standard/full checkpoint. Evidence:checkpoint-487-c-system-tray-focused-*.

Focused **486-C-dialogs** adds `c_api.devices.dialogs` and completes all ten message-box and
file-dialog routes in `devices.h`. It reads real availability without opening UI, then uses CNA's
immediate test services for severity/choice logs, close-without-choice, versioned filters, multi- and
single-path callback results, callback-owned copies, canonical cancellation, invalid requests and
platform restoration. Both renderer selections pass identical20-control output with empty stderr.
Direct C/C++ compilation is diagnostic-free and metadata is486/0. Ten routes raise current C
coverage611->621/3205 and `devices.h`23->33/62. All68 exact input/device/C-API/CLI CTests pass in
117.99s with six declared skips and one known-issue expected failure. No real window, manual
response, filesystem access or callback-thread behavior is claimed. The unchanged external SDL_GPU
mismatch still blocks the standard/full checkpoint.
Evidence:checkpoint-486-c-dialogs-focused-*.

Focused **485-C-vibration** adds `c_api.devices.vibration` and completes all nine vibration routes
in `devices.h`. It reads the real support/name without actuation, then uses CNA's public test backend
to prove exact delivery, inclusive zero/five-second duration bounds, adjacent refusals without log
mutation, scalar and independent dual-motor clamping including NaN-to-zero, idle Stop, exact and
short-buffer-safe name copies, backend replacement/log reset and platform restoration. Both
renderer selections pass identical18-control output with empty stderr. Direct C/C++ compilation is
diagnostic-free and metadata is485/0. Nine routes raise current C coverage602->611/3205 and
`devices.h`14->23/62. All67 exact input/device/C-API/CLI CTests pass in115.45s with six declared
skips and one known-issue expected failure. No physical actuation, request timing or platform motor
success is claimed. The unchanged external SDL_GPU mismatch still blocks the standard/full
checkpoint. Evidence:
checkpoint-485-c-vibration-focused-*.

Focused **484-C-host-info** adds `c_api.devices.host_info`, a read-only C game covering all fourteen
environment/device-extension, power, CPU/RAM, preferred-locale and display-information routes at
the start of the `devices.h` audit. Both renderer selections pass the same12 controls with empty
stderr. Exact host values agree; the expected display answer differs because SOFTWARE is
windowless (scale0, safe area0x0) while OPENGLES3 owns the800x480 Xvfb window (scale1). Every
reported locale round-trips through exact language/country byte counts, and all four index-at-count
routes refuse before changing a guard. Direct C/C++ compilation is diagnostic-free and metadata
is484/0. Fourteen routes raise current C coverage588->602/3205 and establish `devices.h`14/62.
All66 exact input/device/C-API/CLI CTests pass in182.31s with six declared skips and one
known-issue expected failure.
No power transition, locale mutation, rendering or visual inspection is claimed. The unchanged
external SDL_GPU mismatch still blocks the standard/full checkpoint. Evidence:
checkpoint-484-c-host-info-focused-*.

Focused **483-C-sensor-lifecycle** adds `c_api.devices.sensor_lifecycle`, a deterministic C game
covering the four device classes after reporting real host availability. CNA's public synthetic
switches/backends drive converted accelerometer g, exact gyroscope radians/second, compass and
fused-motion readings through current-value and calibration callbacks; the demo verifies legacy
event order, exact current values, validity, intervals, selective unsubscribe, backend retention,
Stop, repeated Dispose refusal and owned-handle release. Both renderer selections produce the same
40 controls and empty stderr:38 pass and exactly the two disposed Accelerometer/Gyroscope
CurrentValue checks fail under declared CNA-DEMOS-ISSUE-062. `SensorBase<T>` checks support but not
disposal before returning cached values, contrary to `sensors.h`; Compass/Motion refuse in this
workflow after their test backend removal. Direct C/C++ compilation is diagnostic-free; metadata
is483/0 and all65 exact input/device/C-API/CLI CTests pass in45.45s with six declared skips and one
known-issue expected failure. Sixty-six routes raise current C coverage522->588/3205 and establish
`sensors.h`116/144. Its remaining28 routes are eight fixed handle type-name helpers plus twenty
low-level Accelerometer/Gyroscope test diagnostics for dispatcher registration/failure injection;
they are intentionally uncredited. No physical sensor or callback-thread claim is made. The
unchanged external SDL_GPU mismatch still blocks the standard/full checkpoint. Evidence:
checkpoint-483-c-sensor-lifecycle-focused-*.

Focused **482-C-sensor-values** adds deterministic `c_api.devices.sensor_values`, separating five
pure reading families and the legacy event payload from device acquisition. Both renderer
selections pass the same34 controls with empty stderr. Constructors preserve version fields,
timestamps, vectors, Euler/quaternion/matrix orientation and nested fused-motion values. Equality
and hashing observe timestamps and nested fields that the canonical text deliberately omits; one
short-buffer check proves refusal before write. Every reading and both event-argument type names
are exact. Direct C/C++ compilation is diagnostic-free; metadata is482/0 and all64 exact
input/device/C-API/CLI CTests pass in44.98s with six declared skips. Fifty routes establish
`sensors.h`50/144 and raise current-inventory coverage472->522/3205. The denominator changed from
3210 because concurrent sibling-CNA graphics work removed five declarations; no demonstrated
route regressed. This demo needs no game or hardware and claims no live measurement. The SDL_GPU
source/header files retain their blocking12:00:56 versions even though unrelated CNA work advanced
to a037dc1b6, so the standard/full checkpoint was not retried. Focused runtime uses the last built
libraries. Evidence:checkpoint-482-c-sensor-values-focused-*.

Focused **481-C-input-haptics** adds one `c_api.input.haptics` standalone with24 checks. Both
renderer selections pass identical labels/output with empty stderr, and this Xvfb host reports zero
haptic devices. Direction equality observes all components; effect equality observes fields unused
by its selected family plus caller-owned custom samples; capability equality observes feature bits
and the external UTF-8 name while closed limits remain -1. Opening an absent id yields an owned
closed device with empty name and canonical capabilities. Every effect-support, rumble,
create/update/run/status/stop/destroy, global stop, gain, autocenter, pause and resume route returns
its documented safe negative answer. Dispose is idempotent and destroy releases the handle. Mouse
and absent-joystick query/open paths also return coherent owned results. Direct C/C++ compilation
is diagnostic-free; metadata is481/0 and all63 exact input/C-API/CLI CTests pass in43.85s with six
declared skips. Thirty-three routes raise coverage439->472/3210 and establish
`input_haptics.h`33/36. The three successful enumerated id/name routes and actual effect upload,
actuation and physical feedback remain hardware-manual. The external SDL_GPU mismatch still blocks
the standard/full checkpoint; focused runtime uses the last built libraries. Evidence:
checkpoint-481-c-input-haptics-focused-*.

Focused **480-C-input-joystick** adds one `c_api.input.joystick` standalone with23 checks. Both
renderer selections pass identical labels and output with empty stderr; this Xvfb host reports zero
raw joysticks. Default descriptor equality observes id, type and UTF-8 name; default capability
equality observes the hardware fields, name and GUID while distinguishing unknown charge from
zero. An absent id returns exact disconnected capabilities and empty name/GUID size/copy results
without changing guard bytes. Two absent ids produce independent owned snapshots whose axis,
button, hat and trackball counts/copies are empty, preserve destinations and compare equal before
independent release. Connected/disconnected raises synchronously reach only their handlers;
selective unsubscribe leaves the other event attached, reset removes it and the registration then
releases safely. Direct C/C++ translation-unit compilation is diagnostic-free; metadata is480/0
and all62 exact input/C-API/CLI CTests pass in30.94s with six declared skips. Twenty-seven routes
raise coverage412->439/3210 and establish `input_joystick.h`27/30. The three successful
enumerated-info/name routes plus physical axis/button/hat/trackball values remain hardware-manual.
The standard/full checkpoint remains blocked by the external SDL_GPU mismatch described below;
focused runtime uses the last built libraries. Evidence:checkpoint-480-c-input-joystick-focused-*.

Focused **479-C-input-text** adds one deterministic `c_api.input.text_input` standalone with20
checks. Both renderer selections pass identical labels and output with empty stderr. The demo
captures the backend's real text-input window token, forces the unbound case, proves false
activation/keyboard queries and successful start/placement/stop no-ops, accepts all nine public
input hints and refuses the first undefined value. It restores the real backend token for one
start/placement/visibility/stop cycle without assuming that an on-screen keyboard appears.
Three process-wide subscriptions receive authored committed UTF-16 (including a surrogate pair),
multibyte UTF-8 composition with deliberately out-of-range byte offsets, and copied candidate
views with an unchecked selected index. Selective unsubscribe leaves both IME families attached;
reset removes all handlers, clears the bound window and makes post-reset releases safe. The
original token is restored. Direct C/C++ translation-unit compilation is diagnostic-free;
metadata is479/0 and all61 exact input/C-API/CLI CTests pass in31.40s with six declared skips.
Seventeen routes raise coverage395->412/3210 and establish `input_text.h`17/17. Physical typing,
platform IME behavior and visible screen-keyboard behavior remain manual. The standard/full
checkpoint still depends on unrelated external CNA renderer work; focused runtime uses the last
built libraries and is not a complete-build claim. Evidence:checkpoint-479-c-input-text-focused-*.

The post-commit watched standard build reached CNA compilation at dirty `d71308d3` with3898
source/header/CMake inputs unchanged across the run, then failed in `SdlGpuRenderer.cpp`: it still
defines retired compute/storage/indirect members absent from the concurrently reduced header. The
earlier Vulkan mismatch has moved on; this SDL_GPU source/header mismatch is the current external
blocker. The first sandboxed configure attempt only failed to create CNA's vendored-SDL lock and
changed no watched input; the repeated unsandboxed build establishes the real compiler failure.
Evidence:checkpoint-479-c-input-text-initial-build-* and `/tmp/cna-479-initial-build.log`.

Focused **478-C-input-clipboard** adds the small `c_api.input.clipboard` standalone rather than
mixing desktop-global state into host-device enumeration. Both renderers pass the same15 checks
with empty stderr. The demo captures the original clipboard through its race-aware size/copy
protocol, requests explicit multibyte UTF-8, observes has-text=true and the exact byte count/copy,
then proves a one-byte-short copy returns BUFFER_TOO_SMALL without changing any of128 destination
bytes. Empty text produces false/zero and a successful null zero-capacity copy. The original byte
sequence is restored and reread exactly. Direct C/C++ translation-unit compilation is clean,
metadata is478/0 and all60 exact input/C-API/CLI CTests pass in41.81s with six declared skips. Four
routes raise coverage391->395/3210 and `input_devices.h`27->31/37. Its last six routes are populated
sensor/touch metadata branches and remain hardware-manual. The standard/full checkpoint has the
same unrelated external CNA Vulkan blocker and focused runtime uses the last built libraries.
Evidence:checkpoint-478-c-input-clipboard-focused-*.

Focused **477-C-input-devices** adds one coherent `c_api.input.devices` standalone with29 checks.
Both renderers pass identical labels and outputs with empty stderr. Xvfb enumerates three mice and
three keyboards; every descriptor and copied UTF-8 name is checked, while zero touch devices and
zero host sensors are ordinary counts whose metadata-success branches remain manual/uncredited.
Default sensor/device descriptors and equality cover id, kind/name and a64-bit id. Accelerometer
and gyroscope return unavailable without changing poisoned vectors. Four public raised hot-plug
events synchronously reach only their handler/id; selective unsubscribe leaves another attached,
reset removes the rest and owned registrations then release safely. One power snapshot reports a
known state,79percent and unknown seconds in this environment. Direct C/C++ translation-unit
compilation is diagnostic-free; metadata is477/0 and the exact59 input/C-API/CLI CTests pass in
39.49s with six declared skips. Twenty-seven routes raise coverage364->391/3210 and establish
`input_devices.h`27/37. The three sensor metadata and three touch metadata routes need populated
hardware; the four clipboard routes are the next separate state-restoration batch. The standard
build/full checkpoint has the same unrelated external CNA Vulkan blocker as the prior focused
batch, so runtime uses the last built CNA libraries and is not a complete-build claim. Evidence:
checkpoint-477-c-input-devices-focused-*.

Focused **476-C-input-gamepad-device** extends `c_api.input.gamepad` from47to55checks without a
new ID or fixture. Both renderers pass the same55 child labels with empty stderr; filtering the
eight additions restores all47 complete-gamepad-values labels in order. One empty slot is selected
from the four live player slots. Rumble and trigger rumble return applied=false while the light-bar
request is a successful no-op; gyro and accelerometer return unavailable without changing poisoned
vectors. Player index, power, button label and connection return -1, Error/-1, Unknown and Unknown.
All four identity strings use the zero-byte size/copy protocol; firmware and Steam identities are
zero; touchpad counts are zero and an unavailable finger preserves every output field. Physical
success branches are explicitly manual. Direct compilation of both changed translation units is
clean, metadata is476/0, and the exact58 input/C-API/CLI CTests pass in33.63s with six declared
skips. Twenty-three actual routes raise C coverage341->364/3210 and `input_gamepad.h`38->61/63;
the two fixed state type-name routes remain trivial/uncredited. The documented watched full build
is externally blocked before this demo: dirty CNA9edbf314 has a concurrently reduced
`VulkanRenderer.hpp` incompatible with `VulkanRenderer.cpp`. A manual link against the last built
CNA libraries supplied the focused immutable runtime only; this is not a complete build claim.
Evidence:checkpoint-476-c-input-gamepad-device-focused-*.

Focused **476-C-input-gamepad-values** extends `c_api.input.gamepad` from28to47checks without a
new ID or fixture. Both renderers pass identical child-label lists with empty stderr; filtering19
additions restores all28 complete-476 labels. Button, D-pad, thumbstick and trigger value families
cover construction, queries, equality/inequality and hashing; composed-state controls cover
component construction/access, packet mutation and the button/packet-only hash. Default
capabilities check every flag, and scalar dead-zone exclusion complements the existing full analog
transforms. The exact58 input/C-API/CLI CTests pass in30.48s with six declared skips. Two watched
builds pass with3912 unchanged CNA inputs/no diagnostics at clean9edbf314; metadata is476/0.
Thirty-four actual routes raise C coverage307->341/3210 and `input_gamepad.h`4->38/63. Two fixed
type-name routes remain trivial/uncredited;23 live device routes remain separately classified.
Full checkpoint pending. Evidence:checkpoint-476-c-input-gamepad-values-focused-*.

Focused **476-C-input-keyboard** extends `c_api.input.keyboard_mouse` from32to40checks without a
new ID or fixture. Both renderers pass identical child-label lists with empty stderr; filtering the
eight additions restores all32complete-476 labels. Four value checks cover default initialization,
the key-state indexer, equality/inequality and the documented A/NumPad1 hash collision. Four live
checks cover player-slot equivalence, modifier bits, physical scancode names and active-layout key
names. The exact58 input/C-API/CLI CTests pass in51.33s with six declared skips; metadata is476/0.
After two successful builds were refused for concurrent unrelated CNA content drift, a stable
watched build passes with3912 unchanged inputs/no diagnostics at dirty bb6820f. Actual C coverage
rises293->307/3210 and `input_keyboard.h`1->15/17. The two remaining routes only size/copy a fixed
type name and are intentionally uncredited. No physical input, alternate layout, lock toggle,
scancode mode, other-desktop or visual claim. Full checkpoint pending.
Evidence:checkpoint-476-c-input-keyboard-focused-*.

Focused **476-C-input-mouse-cursor** extends `c_api.input.keyboard_mouse` from15to32checks without
a new ID or fixture. Both renderers pass identical child-label lists with empty stderr; filtering17
additions restores all15complete-476 labels. Six value checks cover constructors, equality/hash and
field-bearing text. Both selections round-trip their window token, observe SetPosition and an exact
restored global warp, exercise capture, clicked callback/unsubscribe/reset and owned/default,
borrowed-stock and texture cursor lifetimes. OPENGLES3 has a nonzero token, toggles/restores
relative mode and applies capture/release. SOFTWARE has token0, its void relative setter succeeds
without changing false, capture reports not applied and release reports applied. The exact58
input/C-API/CLI CTests pass in52.10s with six declared skips. Two watched builds pass with3911
unchanged CNA inputs/no diagnostics at clean bb6820f; metadata476/0. Actual C coverage rises
267->293/3210: `input_mouse.h`20/20 and `input_cursor.h`6/6. ISSUE-061 records the C header's
empty/default-cursor claim versus the direct Arrow constructor path. No physical transition,
confinement, pointer-shape visual, other-desktop or callback-thread claim. The complete checkpoint
is recorded below.
Evidence:checkpoint-476-c-input-mouse-cursor-focused-*.

Focused **476-C-input-touch-scaling** extends `c_api.input.gesture_queue` from31to64checks without
a new ID or fixture. Both renderers pass identical lists with empty stderr; filtering33 additions
restores all31complete-476 labels in order. Reset reports width0, height0 and Default orientation;
zero-size normalized events are successful drops. With640x480 published, a raised FreeDrag returns
finger23, position(480,240) and delta(320,120), while `CNA_TouchState` stays empty. Portrait
round-trips, an invalid state is refused, and reset clears all three display properties. The exact
58input/C-API/CLI CTests pass in43.67s with six declared environment skips. Two watched builds pass
with3911unchanged CNA inputs/no diagnostics; unrelated external CNA work remains dirty at f44a8ff,
so this is not a clean-source claim. Metadata stays476/0. Seven actual routes raise C ABI coverage
260->267/3210 and input_touch.h25->32/47. ISSUE-060 records the public comment's event-map versus
gesture-only contradiction. No physical touch, orientation-driven rotation, threshold-boundary,
snapshot-injection or visual claim. Evidence:checkpoint-476-c-input-touch-scaling-focused-*.

Focused **476-C-input-gesture-queue**, source `972dc87`, adds one deterministic C standalone.
Both renderers pass31 identical child checks with empty stderr. Reset proves the default None mask
and empty queue; Tap|FreeDrag round-trips exactly. Explicit Tap and FreeDrag samples retain their
timestamps, both finger ids and all four vectors. Disabling detection after enqueue preserves the
pending entries; reads return exact FIFO values, availability follows queue state, and an empty read
returns INVALID_STATE without changing its output. All58input/CLI CTests pass in123.46s with six
declared environment skips. Two watched builds pass with3910unchanged CNA inputs/no diagnostics at
cleanf44a8ff; metadata476/0. Six actual routes raise C ABI coverage254->260/3210 and
input_touch.h19->25/47. No physical detector, recognizer timing/threshold, callback-thread or visual
claim. Evidence:checkpoint-476-c-input-gesture-queue-focused-*.

Last complete baseline is **490-C-adapter-snapshot**, described at the start of this section.
Complete476 remains its retained comparison baseline.
Preserve earlier refusal evidence and ISSUE-038/057/058/059/060/061.

## 8. Exact commands that work

```bash
# siblings: ../cna ../sharp-runtime ../easy-gl ../meta-gl (+ cna submodules)
export CCACHE_DIR=/rv/cnaccache CCACHE_BASEDIR=/rv
# Configure a new/changed environment; reuse the existing build otherwise.
cmake --preset linux-full -DCMAKE_CXX_COMPILER=g++-14 -DCMAKE_C_COMPILER=gcc-14 \
  -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_C_COMPILER_LAUNCHER=ccache
cmake --build --preset linux-full -j4
# Complete checkpoint: isolate both the outer CTest/SOFTWARE phase and the script's GL child.
JOBS=4 SDL_AUDIO_DRIVER=dummy SDL_VIDEO_DRIVER=x11 env -u WAYLAND_DISPLAY \
  xvfb-run -a -s "-screen 0 1280x1024x24" tools/validate.sh
# Individual diagnostic commands (not a replacement for the complete checkpoint):
export XDG_RUNTIME_DIR=/tmp/xdg-runtime; mkdir -p -m 700 $XDG_RUNTIME_DIR
build/linux-full/bin/cna-demos --validate
ctest --test-dir build/linux-full -j4
SDL_AUDIO_DRIVER=dummy build/linux-full/bin/cna-demos --smoke --renderer SOFTWARE --report /tmp/r.json
SDL_AUDIO_DRIVER=dummy SDL_VIDEO_DRIVER=x11 env -u WAYLAND_DISPLAY \
  xvfb-run -a -s "-screen 0 1280x1024x24" build/linux-full/bin/cna-demos --smoke --renderer OPENGLES3
python3 tools/coverage_report.py --exe build/linux-full/bin/cna-demos --cna-root ../cna --results SOFTWARE=/tmp/r.json
```

## 9. Renderers and platforms exercised

| Renderer | Status in this environment |
|---|---|
| SOFTWARE | full smoke suite run with declared capability skips and manual exclusions; selected screenshots inspected |
| OPENGLES3 | full smoke suite run under Xvfb + Mesa llvmpipe with declared skips/manual exclusions; selected screenshots inspected |
| HEADLESS, STUB, OPENGL33, SDL_RENDERER, VULKAN, SDL_GPU | compiled in; not yet run in a recorded session |
| WEBGL2, WEBGPU, DIRECTX9, DIRECTX11, METAL, FNA3D | not built (other platforms / extra deps) |

Platforms: only Linux x86_64 was available. Windows, macOS, web, Android: untested.

## 10. Known CNA issues

See docs/CNA_ISSUES.md. Open: CNA-DEMOS-ISSUE-068 (the C parameter-collection header calls
case-insensitive semantic lookup exact), CNA-DEMOS-ISSUE-067 (C display-color-space routes expose unnamed
integer ordinals), CNA-DEMOS-ISSUE-066 (SOFTWARE ignores the direct GraphicsDevice
blend-enable toggle), CNA-DEMOS-ISSUE-065 (the C GraphicsDevice reset declaration is
separated from its documentation), CNA-DEMOS-ISSUE-064 (the C Viewport default comment promises
MaxDepth1 while XNA/CNA produce zero), CNA-DEMOS-ISSUE-063 (synthetic-camera Closed comments contradict
Opening behavior), CNA-DEMOS-ISSUE-062 (disposed Accelerometer/Gyroscope CurrentValue
remains readable), CNA-DEMOS-ISSUE-057 (concurrent tool staging diagnostic race),
CNA-DEMOS-ISSUE-059 (empty MediaQueue active-Song output contradicts its C documentation),
CNA-DEMOS-ISSUE-056 (FileStream position after EOF),
CNA-DEMOS-ISSUE-055 (shader clone matrices/disposal),
CNA-DEMOS-ISSUE-054 (cube DDS short reads and prefixed truncation),
CNA-DEMOS-ISSUE-051 (host failure C exit-hook contract mismatch),
CNA-DEMOS-ISSUE-050 (owner-only C game operations incorrectly queued),
CNA-DEMOS-ISSUE-049 (stale build-environment overview),
CNA-DEMOS-ISSUE-048 (dropped parameters retained as effective metadata),
CNA-DEMOS-ISSUE-047 (AddOutputFile rejects the configured separate output directory),
CNA-DEMOS-ISSUE-043 (target type versus reader factory lookup),
 CNA-DEMOS-ISSUE-001 (libcna.so link with CMake 3.27/3.28),
CNA-DEMOS-ISSUE-002 (Curve Step keys evaluated against absolute position 1.0),
CNA-DEMOS-ISSUE-003 (SpriteBatch does not leave its texture in Textures[0]),
CNA-DEMOS-ISSUE-004 (Immediate SpriteBatch ignores sampler states set between Draws),
CNA-DEMOS-ISSUE-005 (polymorphic Color breaks XNA-style custom vertex structs silently),
CNA-DEMOS-ISSUE-006 (no capability query for sampler LOD bias on GL ES, nor for PbrEffect's
unlit fallback), CNA-DEMOS-ISSUE-007 (GPU renderers silently ignore ColorMatrixEffect),
CNA-DEMOS-ISSUE-008 (sharp-runtime's Prop.hpp, reached from CNA's public headers, defines macros
named `YES`, `NO`, `nothing`, ...), CNA-DEMOS-ISSUE-009 (`HapticDevice`'s inline default
constructor cannot be compiled outside CNA), CNA-DEMOS-ISSUE-010 (storage and ContentManager throw
std exceptions instead of XNA's types), CNA-DEMOS-ISSUE-011 (ContentManager::Unload does not dispose),
CNA-DEMOS-ISSUE-012 (Load<T> ignores OpenStream overrides), CNA-DEMOS-ISSUE-013 (the content manifest
lists pipeline bookkeeping files), CNA-DEMOS-ISSUE-014 (content root is the working directory until
CNA's platform exists -- matters for console demos that use a ContentManager before any Load),
CNA-DEMOS-ISSUE-015 (`Song::FromUri` resolves relative paths against the working directory),
CNA-DEMOS-ISSUE-016 (static `CNA_GamerServices` ahead of libcna.so duplicates CNA in the
executable -- worked around in our CMake by naming libcna.so first), CNA-DEMOS-ISSUE-017
(`Achievement::EarnedOnline` is always true), CNA-DEMOS-ISSUE-018 (System Link socket failures are
`std::runtime_error`, not `NetworkNotAvailableException`), CNA-DEMOS-ISSUE-019
(`Game::ResetElapsedTime` is ignored with a fixed time step, FNA's rule), CNA-DEMOS-ISSUE-020
(a compiled Song/Video stream is resolved from the ContentManager's root, not beside the asset:
the media content build in `assets/media` gets its own ContentManager rooted there),
CNA-DEMOS-ISSUE-021 (EasyGL -- OPENGLES3, OPENGL33 -- samples a rendered RenderTargetCube face
upside down; SOFTWARE, VULKAN and SDL_GPU are right), CNA-DEMOS-ISSUE-022 (a compiled glTF with an
embedded texture refers to a `<name>_tex0.png` the build never writes), CNA-DEMOS-ISSUE-023
(compiled glTF models lose their cameras; the run-time import keeps both), CNA-DEMOS-ISSUE-024
(`BoundingBox`/`Ray` ToString print doubled braces, following FNA), CNA-DEMOS-ISSUE-025 (offline,
leaderboard columns written after the rating are lost: an entry is saved when its rating is set),
CNA-DEMOS-ISSUE-026 (on Linux, a process that searched for System Link sessions before a host on the
same machine started never receives that host's answers: unicast to the shared discovery port
reaches only the newest socket), CNA-DEMOS-ISSUE-027 (SOFTWARE does not clamp float source values
above 1 before blending into an 8-bit target), CNA-DEMOS-ISSUE-028 (a streaming WaveBank opens a
relative path from the working directory, following FNA), CNA-DEMOS-ISSUE-029
(`SoundEffect::FromStream` takes a `std::istream`, not the `System::IO::Stream` that
`TitleContainer::OpenStream` returns), CNA-DEMOS-ISSUE-030 (`SimulatedPacketLoss` drops reliable packets for good), CNA-DEMOS-ISSUE-031 (glTF
STEP and CUBICSPLINE samplers play as LINEAR), CNA-DEMOS-ISSUE-032 (a System Link join refused
for a game in progress reads `SessionNotFound`, not `SessionNotJoinable`), CNA-DEMOS-ISSUE-033 (a
gamer who joins a System Link game in progress sees the session in Lobby),
CNA-DEMOS-ISSUE-034 (stock Vector4 effect parameters accept scalar getters),
CNA-DEMOS-ISSUE-035 (SkinnedEffect Bones exposes no reflected array elements),
CNA-DEMOS-ISSUE-036 (Linear curve post-loop reads the first outgoing tangent),
CNA-DEMOS-ISSUE-037 (documentation only: dynamic IsLooped setter comments contradict the
implementation and XNA reference; no separate expected-failure marker),
CNA-DEMOS-ISSUE-038 (dynamic PendingBufferCount/IsLooped omit disposal guards),
CNA-DEMOS-ISSUE-039 (ContentManager accepts root changes while assets remain loaded),
CNA-DEMOS-ISSUE-040 (documentation only: Picture/Album thumbnail comments contradict generation paths),
CNA-DEMOS-ISSUE-041 (SavePicture ignores short reads and writes zero-padded image data),
CNA-DEMOS-ISSUE-042 (documentation only: root-prefix comments describe older mesh cancellation),
CNA-DEMOS-ISSUE-058 (`MediaPlayer::Play` frees an active-queue Song before copying/playing it;
ASan probe-backed, no runtime knownIssue marker).

## 11. Dependencies (pinned for CI; same as cna-template's dependencies.lock)

| Repo | Version | Commit |
|---|---|---|
| cna | 0.1.0 | 10a4ef1cb4cd832f761536d77588000104947f8d |
| sharp-runtime | 0.1.0 | c20373bae5be1f02d1fe2fa33c4cbba8ad05e4a0 |
| easy-gl | 0.1.1 | f4d6afaf16cdc7e7e73130950bc45e6d865b41bd |
| meta-gl | 0.4.1 | dd3b30055d55fdc42240de7af248259338256992 |

System packages: README "Requirements". CMake >= 3.29 recommended; this session uses 3.31.6.

## 12. Important design decisions

PLAN.md §2 (table). The non-obvious ones: HiDef project profile; SOFTWARE as test renderer;
`SDL_AUDIO_DRIVER=dummy` in tests; SKIPPED only for declared needs; standalone only for
process-level features; static CNA on CMake < 3.29.

## 13. Unusual and generated files

* `docs/COVERAGE.md` -- generated (tools/coverage_report.py). Do not edit by hand.
* `assets/*` -- generated (tools/generate_assets.py), committed, deterministic.
* `build*/` and `*.log` at the root -- ignored.

## 14. Next task

HIGHEST PRIORITY: close the final `effects.h` route with a coherent ContentManager effect-loading
workflow, then run a complete all-demo checkpoint. Reuse the existing compiled shader and
pixel demos for actual execution claims. Do not turn its290 routes into declaration-count variants: group
related lifetimes,
metadata, typed values, collections, reflection and actual rendering behavior. The complete
`graphics_device.h` audit is
closed at80/84; keep context recovery manual/platform-specific, fixed type-name copies trivial/
uncredited and the one-way string marker intentionally uncredited unless an observable consumer is
added.
Preserve external state and do not claim a real dialog, URL launch, vibration or tray interaction
when a public test backend is the executed path. Do not credit the three joystick, three haptic or
six host sensor/touch populated-metadata routes until suitable hardware executes them.
Do not credit the separate live gamepad dead-zone overload on a disconnected neutral slot or
duplicate generated argument checks. Leave mouse-touch emulation manual-deferred, generic
touch/media reflection uncredited and `media_player_update_ext` deferred. Never edit/revert CNA;
preserve ISSUE-038/057/058/059/060/061/062/063/064/065/066/067/068/069.

## 15. Assumptions the next session must know

* Current workspace: `/rv/data/development/github.com/libcna/cna-demos`; CNA and the other
  dependencies are siblings (`../cna`, `../sharp-runtime`, `../easy-gl`, `../meta-gl`).
* Current verified toolchain: GCC 14.2 and CMake 3.31.6. Reuse `build/linux-full`; do not create
  temporary build trees. Set `CCACHE_DIR=/rv/cnaccache` and `CCACHE_BASEDIR=/rv` for every build.
* External work may change the sibling CNA checkout. Do not edit/revert it from this task;
  record the observed state and exercised binary hashes, and keep runtime checkpoints immutable.
* Running windowed renderers needs `xvfb-run`; audio needs a device or `SDL_AUDIO_DRIVER=dummy`.

Current complete baseline is504-C-graphics-device; source commit and exact results are in section7.
Focused503-C-draw-routes plus focused504-C-occlusion-query, content-loss, device-capabilities and
renderer-recreation are included in it.
Focused505-C-effect-annotations, focused506-C-effect-parameter-values, focused507-C-effect-
parameter-collections, focused508-C-effect-passes-techniques, focused509-C-effect-core,
focused510-C-effect-concrete-types, focused511-C-basic-effect-state, focused512-C-unlit-stock-
effects, focused513-C-environment-map-effect, focused514-C-skinned-effect, focused515-C-color-
matrix-effect, focused516-C-pbr-material, focused517-C-pbr-textures, focused518-C-skinned-pbr,
focused519-C-shader-state, focused520-C-shader-uniforms and focused521-C-shader-textures are
newer source/runtime evidence and are not yet in a full checkpoint.
All earlier drafts are APPLIED AND STALE. The gamepad, host-device, clipboard, text-input, joystick,
haptics, sensor-value, sensor-lifecycle, host-info, vibration, dialogs, system-tray, camera-frame,
clipboard-acceptance, display-value and adapter-snapshot batches are all included in that complete
checkpoint. Adapter-capabilities and device-presentation are included in complete492. The later
render-target-bindings, device-capabilities, sprite-scaled, resource-lifecycle, index-buffer,
device-states and sprite-states batches are all included in complete497. Viewport, the later
device-values and resource-events extensions, texture slots and buffer bindings are included in
complete500. Device clears, owned-device lifecycle and device profile/policy state are included in
complete502. The complete504 checkpoint binary was built with clean CNA `f37f8d184`; CNA advanced
externally during smoke to clean `fbd8e64a3` through CI/CMake-test/docs/tooling-only changes, while
both production library hashes stayed unchanged. Complete504 is the immutable full-runtime
baseline.
Preserve refusal evidence and source/fixture observations, including ISSUE-062. Retain
architecture/stable IDs.
