# HANDOFF

**Read this first in every new session.** It is the single place that says what state the
project is in and what to do next. Update it after every batch. Last updated: 2026-10-14.

## 1. Purpose (one paragraph)

cna-demos is a growing collection of small executable demonstrations of CNA (C++ XNA 4.0
reimplementation): one concept per demo, stable textual IDs, metadata, a terminal browser, a CLI,
unattended runs, CTest tests and a coverage report. Long-term goal: every meaningful CNA
capability has a demo. See docs/PROJECT.md.

## 2. Current phase

Systematic demo batches, one CNA area at a time (status per phase in docs/ROADMAP.md):

* Complete: Phase 0-1 (research, infrastructure), 2 (math, 78 + 6 packed vectors), 3 (game lifecycle, 33), 4 (2D
  graphics), 5 (render states, buffers, primitives, render targets), 10 (input, 39), 11 (storage, 8).
* Mostly complete: 6 (effects and shaders, 41; the C `effects.h` route audit is complete).
* In progress: 7 (models/glTF, 23), 8 (content, 52), 9 (audio 26 incl. 10 C++ XACT plus a complete
  semantic C XACT route audit, media 15 incl. a compiled video and song plus C video asset/player audits),
  **12 (gamer services 33 incl. avatars, leaderboards and C Gamer/profile/friend/achievement/Guide/property workflows; net 24 incl. complete C `net.h`/`net_gamers.h`/`net_sessions.h`; remainders in ROADMAP)**.
* In progress too: 13 (devices 5, phone 1, design 4, diagnostics 2 + logger 2, cnaext 4).
* In progress: 14 (C API, 172 standalone C demos in the source tree; `linux-full` builds CNA with
  `CNA_BUILD_C_API=ON`).
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

636 project-built catalog demos in18 categories: math78, game33, graphics170, audio23, core3,
content46, input40, storage7, media13, gamer services19, net14, devices5, phone1, design4,
diagnostics2, cnaext4, platform2 and c_api172. Kinds are163 console,279 game and194 standalone;
600 are automated,33 smoke-only and3 manual. Every declared category has demos.
Coverage: generated docs/COVERAGE.md is runtime-backed by complete744 at636 demos,302/329 XNA
types,2114/2326 XNA members,169/753 fields and2999/3205 public C ABI routes. Selected semantic
totals are `input_gamepad.h`61/63,
`input_devices.h`31/37, `input_text.h`17/17, `input_joystick.h`27/30 and
`input_haptics.h`33/36, `sensors.h`116/144, `devices.h`59/62, `display.h`22/22 and
`render_target.h`12/12, `graphics.h`25/27, `graphics_resource.h`12/12 and
`index_resources.h`8/10, `graphics_state.h`14/14, `graphics_device.h`80/84, `effects.h`290/290,
`media_player.h`41/41, `net.h`51/51, `net_gamers.h`33/33, `net_sessions.h`104/104,
`gamer_services.h`237/239, `media_library.h`122/148, `cnb.h`272/272, `models.h`216/216,
`texture.h`19/21,
`texture_volume.h`12/16 and `sprite_font.h`9/9.
Complete744 exercises all2999 credited C routes and all636 catalog IDs in both full smoke suites.
It absorbs every staged C batch through complete `models.h`, the focused SpriteFont route, the
deeper XNA member audit through focused716, the Texture2D behavior batches through focused726 and
the SpriteBatch/SpriteFont behavior batches through focused740 plus the ShaderEffect cube batches
through focused743.
Build-only CNA-DEMOS-ISSUE-057 is tracked separately; it adds no runtime knownIssue marker.
Complete744 has37 known-issue demos after focused660 adds
`game.graphics_device_manager.back_buffer_size` for CNA-DEMOS-ISSUE-083; the baseline35 are
`graphics.sprite_batch.begin_end_rules` CNA-DEMOS-ISSUE-082,
`c_api.gamer.achievement_collection` CNA-DEMOS-ISSUE-077,
`c_api.net.local_gamer_packets` CNA-DEMOS-ISSUE-075,
`c_api.audio.spatial_state` CNA-DEMOS-ISSUE-073,
`c_api.graphics.pbr_effect_material_state` CNA-DEMOS-ISSUE-070,
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
`game.time_step.reset_elapsed_time` CNA-DEMOS-ISSUE-019; `graphics.model.textured_gltf`
CNA-DEMOS-ISSUE-022, `graphics.model.gltf_cameras` CNA-DEMOS-ISSUE-023, `math.formatting.to_string`
CNA-DEMOS-ISSUE-024, `net.network_session.simulated_conditions` CNA-DEMOS-ISSUE-030,
`graphics.model.clip_interpolation` CNA-DEMOS-ISSUE-031, `net.network_session.join_refused`
CNA-DEMOS-ISSUE-032 and -033; `graphics.effect.parameter_values` CNA-DEMOS-ISSUE-034 and -035.
Passing documentation/result issues ISSUE-068, ISSUE-069, ISSUE-071, ISSUE-072, ISSUE-074,
ISSUE-076, ISSUE-078 and ISSUE-081 do not add
runtime knownIssue markers.
Focused748 stages a37th runtime marker: `graphics.texture_cube.face_transfers` names
CNA-DEMOS-ISSUE-086 only for OPENGLES3. Its next primary frame is wholly black after a secondary
EasyGL device is destroyed; SOFTWARE remains a12/12 pass.
Live numbers: `cna-demos --categories`, docs/COVERAGE.md.

## 7. Build and test status

Complete **744-SpriteBatch-SpriteFont-cube-full-checkpoint** is the current full-build and
full-runtime baseline. Because the live sibling CNA changed externally to a newer dirty checkout,
the build was configured against an isolated worktree at `/tmp/cna-demos-cna-d23`, exactly commit
`d23e139de9e5c15511b6be745de290896a69ee37`. Recreate that worktree or reconfigure only after the
sibling is clean; the current build tree records the temporary path. The full rebuild used at most
three compile jobs. Metadata is636/0, all649 enabled CTests pass in187.73s and three declared manual
tests remain disabled. SOFTWARE/Xvfb reports562 pass/36 expected fail/35 skip/3 manual;
OPENGLES3/Xvfb reports594/36/3/3. Both execute all636 IDs with zero unexpected failures or
unexpected passes; all five validator self-tests and all four checkpoint-comparison tests pass.

Every Complete735 status/knownIssue pair and failed-assertion multiplicity remains exact on both
renderers. The only added ID, `graphics.sprite_batch.stock_effect`, passes on both. Intended check
growth is limited to SpriteBatch layer-depth sorting, SpriteFont draw/transformed text and the GL
cube sampler; normalized stderr is byte-for-byte equivalent to Complete735 for each renderer.
Generated coverage is302/329 types,2114/2326 non-field members,169/753 fields and2999/3205 C
routes. During validation, the C error-handling demo was corrected to use variable time so each
explicit RunOneFrame produces one planned callback, and four demos that mutate global pointer,
clipboard or screen-saver state were made `RUN_SERIAL`. These were demo/test-harness races, not CNA
defects. A sandboxed CTest attempt predictably failed storage, localhost-network and X11 mutations;
the authoritative unrestricted rerun is the passing result above.

Tested SHA-256: `cna-demos`
`fa2475c6a782f3a0f0030edffbf27a7316a99a5cfb9b0b0328f9ced2aa124cf9`, `libcna.so`
`3827917b8b1219fb659c031039f518680e76670dfb6c69bc891c942049f469ce`, and `libcna_c_api.so`
`813bf0a2095524183bbaca66f6fbcc1fbdfcfcd5ddd8148c8ec6ce3138ebee24`. Report hashes are SOFTWARE
`f3fb7bd967ddd8ca5d6794a3681fef199c1866a565206da7b555a13c79380852` and OPENGLES3
`c246872d21ece0d8cc6af3ecd50dca88dcd9158ebe4de336fb11e6b498d06785`. Evidence is archived as
`build/linux-full/reports/checkpoint-744-complete-*`. Pixel checks were programmatic; no screenshot,
listening test, physical input, external service session or manual visual-quality inspection was
performed.

Focused **745-Texture3D-coordinate-and-mip-filtering** is staged after Complete744. The existing
`graphics.texture3d.sampler_coordinates` now uses a4x4x2 mipmapped Color volume. Four exact
slice-zero U/V texel centres preserve White/Yellow/Cyan/Magenta row order, while authored uniform
Lime mip1 and Blue mip2 at explicit LOD1.25 distinguish `LinearMipPoint` exact selection from
`PointMipLinear` RGB(0,191,64) interpolation. The original shader compilation, move ownership,
slice centres, Z filtering and W addressing remain intact. OPENGLES3/Xvfb passes10/10; SOFTWARE
retains its declared CustomEffects skip and exact skipped CTest. The incremental `-j3` build is
warning-free, metadata remains636/0, both tool self-test suites pass and generated coverage remains
302/329 types,2114/2326 members,169/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-745-texture3d-*`. Pixel readback was programmatic; no screenshot or manual visual
inspection was performed. No CNA issue was found.

Focused **746-Texture3D-raw-transfer-and-active-binding** extends
`graphics.texture3d.volume_data` from16 to20 controls on both renderers. A two-slice Color volume
round-trips four arbitrary trivially-copyable four-byte words through guarded caller windows in
X-fastest/slice-major order. Eight-byte elements are refused atomically for upload and readback. A
bound volume refuses upload without mutation; after move construction the active binding follows
the new wrapper, still refuses mutation, and the original payload remains exact after explicit
unbinding. SOFTWARE and OPENGLES3/Xvfb pass20/20 with empty stderr; the exact SOFTWARE CTest passes.
The incremental `-j3` build is warning-free, metadata remains636/0, both tool self-test suites pass
and coverage remains302/329 types,2114/2326 members,169/753 fields and2999/3205 C routes. Packed and
half format conversion is intentionally not duplicated because the exhaustive Texture2D workflows
exercise the same packed-value conversion routes. Evidence:`/tmp/checkpoint-746-texture3d-*`. No
screenshot or manual visual inspection was performed. No CNA issue was found.

Focused **747-Texture3D-device-boundaries-and-cross-device-assignment** extends the same
`graphics.texture3d.volume_data` workflow from20 to25 checks. An occupied target bound on the
primary device receives a bound2x1x2 source from a secondary device; it reports the secondary
owner, retains exact four-color payload and refuses upload until that transferred binding is
cleared. Constructor controls separately reject DXT1 volume storage, width257 above the HiDef
ceiling and1x1x1 Color on Reach. SOFTWARE and OPENGLES3/Xvfb pass25/25 with empty focused stderr;
the exact SOFTWARE CTest passes. The incremental `-j3` build is warning-free, metadata is636/0,
tool self-tests pass5/5 and4/4, and coverage remains302/329 types,2114/2326 members,169/753 fields
and2999/3205 C routes. Evidence:`/tmp/checkpoint-747-texture3d-*`. Repetitive box-coordinate guards,
packed/half variants and reflection/renderer hooks are dispositioned, closing the Texture3D
behavior table without duplicate demos. No screenshot or manual visual inspection was performed;
no CNA issue was found.

Focused **720-GraphicsDevice-Present-region-contract** is staged after Complete719. The existing
owned-device lifecycle now passes11 controls on both SOFTWARE and OPENGLES3/X11: null/null/zero is
ordinary Present, a partly outside source region is clipped and accepted only by SOFTWARE among
the two primary renderers, a wholly outside region is rejected as an argument error, and valid
destination/window requests are explicitly refused rather than silently ignored. The exact
SOFTWARE CTest, metadata632/0 and all five validator self-tests pass. Generated coverage is
unchanged because `GraphicsDevice::Present` was already credited. Evidence:
`/tmp/checkpoint-720-present-regions-*`. No presented window pixels or screenshots were inspected.

Focused **721-Texture2D-binary32-formats** adds the633rd catalog ID,
`graphics.texture2d.floating_formats`. SOFTWARE and OPENGLES3/X11 pass8/8 with byte-identical
sampled colors; exact Single/Vector2/Vector4 values survive SetData/GetData, an incompatible
element width is refused without mutation, and actual sampling proves missing-channel expansion
plus final Color-buffer clamping. The exact SOFTWARE CTest, metadata633/0 and all five validator
self-tests pass. Temporary coverage is302/329 types,2114/2326 non-field members,158/753 fields and
2999/3205 C routes. Evidence:`/tmp/checkpoint-721-floating-formats-*`. Pixel readback was
programmatic; no screenshot or manual visual inspection was performed.
During focused721 the clean sibling CNA checkout advanced externally to docs-only `a37bb07ba`;
`libcna.so` and `libcna_c_api.so` retain the exact Complete719 hashes.

Focused **722-Texture2D-binary16-formats** extends the same catalog ID with HalfSingle,
HalfVector2 and HalfVector4 storage and sampling. Exact packed values survive SetData/GetData,
including channels above one and below zero, while point sampling proves missing-channel expansion
and final Color-buffer clamping. SOFTWARE and OPENGLES3/X11 pass15/15 with byte-identical sampled
colors; the exact SOFTWARE CTest, metadata633/0 and all five validator self-tests pass. Temporary
coverage remains302/329 types,2114/2326 non-field members and2999/3205 C routes; fields move
158->161/753 through the three half-float SurfaceFormat values. Evidence:
`/tmp/checkpoint-722-half-formats-*`. Pixel readback was programmatic; no screenshot or manual
visual inspection was performed.

Focused **723-Texture2D-normalized-integer-formats** adds the634th catalog ID,
`graphics.texture2d.normalized_formats`. All six remaining classic normalized layouts round-trip
exact packed values and actual sampling distinguishes signed clamping, missing-channel expansion,
Rgba1010102's two-bit alpha, wide UNorm channels and Alpha8's zero RGB. SOFTWARE and OPENGLES3/X11
pass10/10; their only byte difference is expected signed-alpha halfway rounding at129 versus128,
within tolerance2. The exact SOFTWARE CTest, metadata634/0 and all five validator self-tests pass.
Temporary coverage is302/329 types,2114/2326 non-field members,167/753 fields and2999/3205 C
routes. Evidence:`/tmp/checkpoint-723-normalized-formats-*`. Pixel readback was programmatic; no
screenshot or manual visual inspection was performed.

Focused **725-Texture2D-transfer-rejection-atomicity** extends the existing
`graphics.texture2d.set_data_rectangle` from4 to7 controls. A partly outside rectangle and a short
logical count are rejected in both transfer directions before texture or caller data changes; a
separate disposed texture rejects SetData/GetData and preserves its read sentinel. SOFTWARE and
OPENGLES3/X11 pass7/7, the exact SOFTWARE CTest passes, metadata remains634/0 and all five validator
self-tests pass. Coverage remains302/329 types,2114/2326 non-field members,167/753 fields and
2999/3205 C routes. Evidence:`/tmp/checkpoint-725-texture2d-rejections-*`. No screenshot or manual
visual inspection was performed.

Focused **726-Texture2D-classic-stream-boundaries** extends
`graphics.texture2d.from_stream` from4 to6 controls. A 2x2 palette GIF decodes to its exact
transparent and opaque RGBA texels, and a recognized but truncated GIF is refused with
InvalidOperationException. Together with the retained PNG behavior and `save_as_image` JPEG round
trip, all three classic FromStream containers now have executed evidence. SOFTWARE and
OPENGLES3/X11 pass6/6, the exact SOFTWARE CTest passes, metadata remains634/0 and all five validator
self-tests pass. Coverage remains302/329 types,2114/2326 non-field members,167/753 fields and
2999/3205 C routes. Evidence:`/tmp/checkpoint-726-from-stream-*`. Pixel validation used public
GetData; no screenshot or manual visual inspection was performed.

Focused **727-SpriteBatch-source-rectangle-pixels** adds three framebuffer controls to the existing
`graphics.sprite_batch.source_rectangle`. Four enlarged source rectangles select their exact atlas
colors, known transparent texels preserve the background and `std::nullopt` draws the full strip in
original order. An initial compile failure was caught before accepting stale zero-check reports;
after the corrected rebuild, SOFTWARE and OPENGLES3/X11 pass3/3 and the exact SOFTWARE CTest passes.
Metadata remains634/0, all five validator self-tests pass and coverage remains302/329 types,
2114/2326 non-field members,167/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-727-source-rectangle-*`. No screenshot or manual visual inspection was performed.
After validation the sibling CNA checkout acquired an unrelated external uncommitted change in
`modules/renderers/sdl-gpu/tests/CNA/Internal/Renderers/SdlGpu/SdlGpuCompiledEffectTests.cpp`; no
CNA file was edited here, and both production library hashes remain exact from Complete724.

Focused **728-SpriteBatch-tint-and-premultiplied-fade** adds two representative framebuffer
controls to `graphics.sprite_batch.tint_and_transparency`. Point-sampled opaque white texels prove
exact White, Red, Lime and half-red/half-green Blue multiplication; four opacity levels prove the
expected premultiplied white-over-DimGray blend sequence within tolerance2. SOFTWARE and
OPENGLES3/X11 pass2/2, the exact SOFTWARE CTest passes, metadata remains634/0, the five validator
self-tests and four checkpoint-comparison tests pass, and coverage remains302/329 types,
2114/2326 non-field members,167/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-728-tint-*`. An initial combined Python test invocation used an invalid root-level
import path; the supported invocations both pass. No screenshot or manual visual inspection was
performed. During this validation the sibling CNA checkout still carried only the unrelated
external SDL GPU test change; the production library hashes remained exact from Complete724.

Focused **729-SpriteBatch-destination-rectangle-pixels** adds two framebuffer controls to
`graphics.sprite_batch.destination_rectangle`. All four native/enlarged/nonuniform rectangles fill
their bounds without touching the immediately adjacent pixels, and representative samples from
the256x128 horizontal and48x400 vertical stretches preserve the full checker's normalized pattern.
SOFTWARE and OPENGLES3/X11 pass2/2, the exact SOFTWARE CTest passes, metadata remains634/0, the
five validator self-tests and four checkpoint-comparison tests pass, and coverage remains302/329
types,2114/2326 non-field members,167/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-729-destination-*`. No screenshot or manual visual inspection was performed. The
external SDL GPU test change was subsequently committed in the sibling checkout as test-only CNA
`d23e139de`; that checkout is clean and one commit ahead of its origin, while both production
library hashes remain exact from Complete724.

Focused **730-SpriteBatch-disposed-texture-refusal** extends the existing known-issue
`graphics.sprite_batch.begin_end_rules` from12 to14 controls. Draw rejects an explicitly disposed
Texture2D with ObjectDisposedException before queueing, then the same still-open deferred batch
successfully draws a valid texture and ends. SOFTWARE and OPENGLES3/X11 each retain only the exact
prior CNA-DEMOS-ISSUE-082 active-accounting failure, with thirteen passing controls and the same
failure message. The known-issue SOFTWARE CTest passes, metadata remains634/0, the five validator
self-tests and four checkpoint-comparison tests pass, and coverage remains302/329 types,
2114/2326 non-field members,167/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-730-disposed-texture-*`. No screenshot or manual visual inspection was performed.
Sibling CNA is clean at test-only `d23e139de`; production library hashes remain exact from
Complete724.

Focused **731-SpriteBatch-cross-device-texture** adds the635th catalog ID as a focused console
workflow with two explicitly owned GraphicsDevices. Drawing the second device's Texture2D through
the first device's SpriteBatch is rejected at Draw with InvalidOperationException; the still-open
batch then accepts its own texture, End and another Begin/End, proving no foreign command was
queued. SOFTWARE and OPENGLES3/X11 pass2/2, the exact SOFTWARE CTest passes, metadata is635/0,
the five validator self-tests and four checkpoint-comparison tests pass, and temporary coverage is
302/329 types,2114/2326 non-field members,167/753 fields and2999/3205 C routes.

An initial attempt inside the existing Game demo passed the ownership checks but disturbed its
later OPENGLES3 framebuffer control when the second context was disposed. That host-interference
variant was removed completely. Both renderer regressions of `begin_end_rules` retain14 controls
and only the exact ISSUE-082 failure. Evidence:`/tmp/checkpoint-731-cross-device-*` and
`/tmp/checkpoint-731-begin-end-regression-*`. No screenshot or manual visual inspection was
performed. Sibling CNA remains clean at test-only `d23e139de`; production library hashes remain
exact from Complete724.

Focused **732-SpriteBatch-rotation-origin-pixels** makes the existing rotation workflow
deterministic and adds three framebuffer controls. Four centre-origin arrows at0/90/180/270
degrees keep the authored yellow centre on each pivot while the red corner marker turns clockwise
through the predicted quadrants; a second90-degree pair distinguishes centre from top-left origin
placement. SOFTWARE and OPENGLES3/X11 pass3/3, the exact SOFTWARE CTest passes, metadata remains
635/0, the five validator self-tests and four checkpoint-comparison tests pass, and coverage
remains302/329 types,2114/2326 non-field members,167/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-732-rotation-*`. Pixel readback was programmatic; no screenshot or manual visual
inspection was performed. Sibling CNA remains clean at test-only `d23e139de`; production library
hashes remain exact from Complete724.

Focused **733-SpriteBatch-scalar-and-vector-scale-pixels** adds two framebuffer controls to the
existing five-draw scale workflow. The authored red/yellow marker spacing grows proportionally
under scalar0.5/1/3, while Vector2(5,1) and(1,5) stretch the same pair independently on X and Y.
SOFTWARE and OPENGLES3/X11 pass2/2, the exact SOFTWARE CTest passes, metadata remains635/0, the
five validator self-tests and four checkpoint-comparison tests pass, and coverage remains302/329
types,2114/2326 non-field members,167/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-733-scale-*`. Pixel readback was programmatic; no screenshot or manual visual
inspection was performed. Sibling CNA remains clean at test-only `d23e139de`; production library
hashes remain exact from Complete724.

Focused **734-SpriteBatch-effects-mirroring-pixels** adds three framebuffer controls to the
existing four-flag SpriteEffects scene. The asymmetric red marker reaches all four expected
corners, the yellow arrow tip proves that only the horizontal flag reverses direction, and adjacent
samples keep every variant inside the same128x128 destination bounds. SOFTWARE and OPENGLES3/X11
pass3/3, the exact SOFTWARE CTest passes, metadata remains635/0, the five validator self-tests and
four checkpoint-comparison tests pass, and coverage remains302/329 types,2114/2326 non-field
members,167/753 fields and2999/3205 C routes. Evidence:`/tmp/checkpoint-734-sprite-effects-*`.
Pixel readback was programmatic; no screenshot or manual visual inspection was performed. Sibling
CNA remains clean at test-only `d23e139de`; production library hashes remain exact from Complete724.

Focused **736-SpriteFont-layer-depth-in-shared-queue** extends the existing
`graphics.sprite_batch.layer_depth_sorting`. It now loads the compiled pixel font and mixes
DrawString glyphs with ordinary opaque sprites in one BackToFront batch. Text submitted first at
depth0.25 remains visible over a later depth0.5 sprite, while text submitted last at depth0.75 is
fully hidden behind an earlier depth0.5 sprite. This proves the full DrawString overload forwards
nonzero depth for every glyph into the shared sorted queue, a text-specific behavior absent from
the previous sprite-only controls.

SOFTWARE and OPENGLES3/X11 pass5/5, and the exact SOFTWARE CTest passes. Metadata remains635/0,
all five validator self-tests and all four checkpoint-comparison tests pass, and temporary coverage
remains302/329 types,2114/2326 non-field members,167/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-736-layer-depth-*` and `/tmp/checkpoint-736-coverage.md`. The first cached build was
blocked by the sandbox's read-only external ccache and was rerun successfully with cache disabled;
the final GL run used an approved out-of-sandbox Xvfb because the sandbox's shared X11 socket
directory could not accept another listener. Pixel readback was programmatic; no screenshot or
manual visual inspection was performed. No CNA issue was found.

Focused **737-SpriteFont-transformed-layout-pixels** replaces the presentation-only variants in
`graphics.sprite_font.transformed_text` with three aggregate framebuffer contracts. The same text
at scalar(2,2) and Vector2(4,2) proves the entire glyph layout doubles only in width. A0-degree and
90-degree pair proves both ink bounds and their offset around the `MeasureString` centre rotate
together. Finally, normal, horizontal, vertical and combined flags on the asymmetric multi-glyph
string `LJ` prove exact whole-layout reflections rather than independent in-place glyph flips.

SOFTWARE and OPENGLES3/X11 pass3/3, and the exact SOFTWARE CTest passes. Metadata remains635/0,
all five validator self-tests and all four checkpoint-comparison tests pass, and temporary coverage
remains302/329 types,2114/2326 non-field members,167/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-737-transformed-text-*` and `/tmp/checkpoint-737-coverage.md`. Pixel readback was
programmatic; no screenshot or manual visual inspection was performed. No CNA issue was found.

Focused **738-SpriteFont-basic-multiline-pixels** extends
`graphics.sprite_font.draw_string` from one metadata check to three controls. Its multiline input
now mixes CRLF and LF; the repeated `Line ` prefix produces an exact pixel-identical mask at each
18-pixel `LineSpacing` interval. A second aggregate framebuffer control confirms that opaque glyph
ink reaches all four displayed White/Gold/LightGreen/LightSkyBlue tint paths. This validates the
compiled-font scene without treating ordinary SpriteBatch tint multiplication as a new API gap.

SOFTWARE and OPENGLES3/X11 pass3/3, and the exact SOFTWARE CTest passes. Metadata remains635/0,
all five validator self-tests and all four checkpoint-comparison tests pass, and temporary coverage
remains302/329 types,2114/2326 non-field members,167/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-738-draw-string-*` and `/tmp/checkpoint-738-coverage.md`. Pixel readback was
programmatic; no screenshot or manual visual inspection was performed. No CNA issue was found.

Focused **739-SpriteBatch-equal-depth-and-depth-buffer** extends
`graphics.sprite_batch.layer_depth_sorting` from five to seven controls. Three equal-depth sprites
prove XNA 4.0's intentionally unstable three-item Array.Sort result: the first red submission ends
on top after reversal rather than preserving call order. Two otherwise identical FrontToBack
panels then draw near red before far blue; painter order leaves blue visible with
DepthStencilState::None, while Default writes the nearer0.1 depth and rejects the farther0.9 quad.
This proves layerDepth remains real vertex Z as well as a CPU sort key.

SOFTWARE and OPENGLES3/X11 pass7/7, and the exact SOFTWARE CTest passes. Metadata remains635/0,
all five validator self-tests and all four checkpoint-comparison tests pass, and temporary coverage
remains302/329 types,2114/2326 non-field members,167/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-739-layer-depth-*` and `/tmp/checkpoint-739-coverage.md`. The first build exposed a
range-loop copy warning; changing the loop variable to a reference produced a warning-free rebuild
before runtime validation. Pixel readback was programmatic; no screenshot or manual visual
inspection was performed. No CNA issue was found.

Focused **740-SpriteBatch-stock-effect-and-custom-effect-disposition** adds
`graphics.sprite_batch.stock_effect`. An AlphaTestEffect Projection maps a red sprite into the
top-right model-space quadrant; a BasicEffect with the same Projection and a Y-flipping World maps
a blue sprite into the bottom-right quadrant. Exact samples also keep the left half and original
pixel-space corner black. This demonstrates the portable stock-Effect argument to
SpriteBatch::Begin in both Deferred and Immediate modes without hiding the relevant calls.

The adjacent custom-effect audit found no portable compiled-effect workflow in the current
baseline: `CNA_EASYGL_COMPILED_EFFECTS` and `CNA_SOFTWARE_COMPILED_EFFECTS` are both OFF, SOFTWARE
does not advertise `custom-effects`, and `c_api.graphics.effect_core` already passes by checking
explicit refusal of the valid `CnaConformanceEffect.fxb`. The existing GLSL `sprite_shader` and
`portable_package` cover executable source effects on OPENGLES3. CNA contains renderer conformance
tests for compiled SpriteBatch, multi-pass ordering and effect alternation when compiled effects
are enabled, but those tests were inspected rather than run here. A new `.fxb` demo would only
duplicate a refusal in this build, so none was added.

SOFTWARE and OPENGLES3/X11 pass2/2; the exact SOFTWARE CTest passes. Metadata is636/0, all five
validator self-tests and all four checkpoint-comparison tests pass, and temporary coverage remains
302/329 types,2114/2326 non-field members,167/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-740-stock-effect-*` and `/tmp/checkpoint-740-coverage.md`. Pixel readback was
programmatic; no screenshot or manual visual inspection was performed. No CNA issue was found.

Focused **741-ShaderEffect-cube-binding-replacement** extends
`graphics.texture_cube.sampler_ownership` from five to six controls. After the existing copied cube
draws all six solid faces, a distinct all-white cube is rebound to the already occupied shader unit
1 and overwrites only the right part of the +X card. Exact samples retain red on the untouched left
and require white on the replacement region, so residual state from the first cube cannot pass.

OPENGLES3/X11 passes6/6; SOFTWARE retains its explicit CustomEffects capability skip and its exact
CTest is reported skipped. The incremental build is warning-free, metadata remains636/0, all five
validator self-tests and all four checkpoint-comparison tests pass, and temporary coverage remains
302/329 types,2114/2326 non-field members,167/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-741-cube-rebinding-*` and `/tmp/checkpoint-741-coverage.md`. Pixel readback was
programmatic; no screenshot or manual visual inspection was performed. No CNA issue was found.

Focused **742-ShaderEffect-cube-face-orientation** extends the same demo from six to seven controls.
The surviving cube is now4x4: its +X face keeps a red centre but authors White/Yellow/Cyan/Magenta
at top-left/top-right/bottom-left/bottom-right. Directions `(1,+.75,+.75)`, `(1,+.75,-.75)`,
`(1,-.75,+.75)` and `(1,-.75,-.75)` address the exact four texel centres under GLSL's +X mapping
`s=(-Z/X+1)/2`, `t=(-Y/X+1)/2`. Four framebuffer swatches preserve that order while the red centre
and white same-unit replacement controls remain independently visible.

OPENGLES3/X11 passes7/7; SOFTWARE retains its explicit CustomEffects skip and exact skipped CTest.
The incremental build is warning-free, metadata remains636/0, all five validator self-tests and all
four checkpoint-comparison tests pass, and temporary coverage remains302/329 types,2114/2326
non-field members,167/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-742-cube-orientation-*` and `/tmp/checkpoint-742-coverage.md`. Pixel readback was
programmatic; no screenshot or manual visual inspection was performed. No CNA issue was found.

Focused **743-ShaderEffect-cube-mip-filtering** extends the same demo from seven to eight controls.
The copied4x4 cube now owns a complete authored chain: the base faces retain all earlier selection
and orientation evidence, mip1 is Lime and mip2 Blue. The shader uses explicit LOD1.25. A unit1
`LinearMipPoint` sampler returns exact Lime, while `PointMipLinear` returns the predicted3:1
cross-level blend RGB(0,191,64). This isolates mip selection/interpolation from geometry-derived
LOD and from the already demonstrated base-level texel filtering.

OPENGLES3/X11 passes8/8; SOFTWARE retains its CustomEffects skip and exact skipped CTest. The
incremental build is warning-free, metadata remains636/0, all five validator self-tests and all four
checkpoint-comparison tests pass, and temporary coverage remains302/329 types,2114/2326 non-field
members,167/753 fields and2999/3205 C routes. Evidence:`/tmp/checkpoint-743-cube-mips-*` and
`/tmp/checkpoint-743-coverage.md`. Pixel readback was programmatic; no screenshot or manual visual
inspection was performed. No CNA issue was found.

Complete **717-member-gap-disposition-runtime** is the prior incremental runtime baseline. Exact-current
metadata validation is632/0, all five validator self-tests pass, and the full CTest run passes all
645 enabled tests in273.41s; three declared manual tests remain disabled. The complete SOFTWARE/X11
smoke is558 pass/36 expected fail/35 skip/3 manual, while OPENGLES3/X11 is590/36/3/3. Both execute
all632 catalog IDs with zero unexpected failures or unexpected passes. Generated coverage is
301/329 XNA types,2114/2326 non-field members,157/753 fields and2999/3205 public C ABI routes.

The checkpoint also reviews every one of the212 remaining generated non-field gaps in
`docs/COVERAGE_AUDIT.md`: they are already represented collection semantics, CLR aliases,
protected/finalizer machinery or truthful platform/service/hardware work, rather than212 missing
coherent demos. The first smoke attempts exposed three demo defects that are fixed and revalidated:
the C keyboard/mouse demo now skips when no desktop-global pointer/cursor exists, fixed-step
method-order accepts legitimate catch-up Updates, and the C SuppressDraw control switches to
variable-step timing before exact Tick accounting.

This is an incremental/runtime checkpoint against the existing production libraries, not a fresh
whole-project build. During the work the sibling CNA checkout carried unrelated external changes,
so only the exact changed cna-demos objects and executables were compiled/relinked. Tested SHA-256:
`cna-demos` `e1e7932acb863e40c2830f84e3066dec704768f6fdb313a4cea8e898d42e44d8`,
`libcna.so` `e2c8c9dbd6ef8fb4e611a6f0818ebec6290c34d738930c3e9f1dbb4a55ba9aeb`, and
`libcna_c_api.so` `471f0a12b6b167262d18ae8fb763f4c10b07e7bc478f72f0bbfd5a2879b193ef`.
Evidence is archived as `build/linux-full/reports/checkpoint-717-complete-*`. After capture the CNA
checkout became clean at external `0c83dcc9dc`; the next task is a full documented rebuild against
that source before further runtime claims. No screenshot, listening test, physical input, service
session or manual visual-quality inspection was performed.

Complete **647-member-audit-integration** is an older comparison baseline. The documented
outer-Xvfb `tools/validate.sh` invocation passes metadata627/0 and all640 enabled CTests; three
declared tests remain disabled. SOFTWARE is555 pass/34 known-fail/35 skip/3 not-run and OPENGLES3
is587/34/3/3, with zero unexpected passes. Fresh generated coverage is297/329 XNA types,
1519/2326 XNA members and2999/3205 C routes. Against complete634, the only status/knownIssue delta
on either renderer is `graphics.sprite_batch.begin_end_rules`: its new disposal control exposes
CNA-DEMOS-ISSUE-082, so it changes from pass to expected failure. All13 other extended IDs retain
their prior statuses and only increase their control counts. The preflight GamePad capability
focused runs passed without hardware, but the complete smoke totals remain unchanged because that
ID was already a normal pass in the archived complete634 reports; the earlier projected skip delta
was incorrect.

The incremental build log is diagnostic-free. The run began after observing clean sibling CNA
`09c3108e58`; by final capture that external checkout had independently advanced to `af798b73a4`
and contained four uncommitted FNA3D CMake/patch paths. No CNA source was edited or reverted here.
The tested binaries are immutable evidence: SHA-256 `cna-demos`
`d0d15b3d4f7a61dd3ee85795ad557f5581fb9a3911426fa5cf914404476e49e6`, `libcna.so`
`1f54d1a7f969d9ca1258ec75f76a9a75549d1bf20313a63c6734606c10abc8f9`, and
`libcna_c_api.so` `fedaef026b8c52d619905f6f2aee1553e6def843ce407f8290c45c16857b6f19`.
Evidence is archived as `build/linux-full/reports/checkpoint-647-complete-*`; no screenshot,
listening test or manual visual-quality inspection was performed.

Complete **634-retired-renderers-and-full-staged-baseline** remains the prior comparison baseline.
Its `linux-full` configure follows CNA's renderer retirement: OPENGLES3, OPENGL33, SDL_GPU,
SOFTWARE, HEADLESS and STUB are compiled, with no stale SDL_RENDERER request or dead PBR branch.

The first CTest attempt found two demo defects, not CNA defects: Guide social sampled one fixed
pixel while its card was still translated by the entrance animation, and Guide dispatcher assumed
the SOFTWARE renderer's documented zero legacy window handle was impossible. Whole-frame accent/
panel counts and sentinel replacement now pass directly on SOFTWARE and OPENGLES3; five repeated
SOFTWARE CTest runs also pass. A later smoke exposed process-order contamination in
`resource_content_manager`: CNA `c1006b03e` routes ResourceContentManager through OpenStream before
registering built-in readers, so a prior demo could hide the cold-start ISSUE-012 failure. The demo
now owns its process and both content-manager batches report zero unexpected passes. The same audit
confirmed CNA `a6e8104d2` fixes rendered RenderTargetCube orientation; ISSUE-021 is resolved and its
runtime marker is removed after isolated OPENGLES3 and OPENGL33 pixel checks.

The final build uses clean sibling CNA `53aba6ef15`. SHA-256: `cna-demos`
`17c86d6692c3b87332152c17cb8db1ca82fa4af339e50692e7805687b5b099bf`, `libcna.so`
`1f54d1a7f969d9ca1258ec75f76a9a75549d1bf20313a63c6734606c10abc8f9`, and
`libcna_c_api.so` `fedaef026b8c52d619905f6f2aee1553e6def843ce407f8290c45c16857b6f19`.
Evidence is archived as `build/linux-full/reports/checkpoint-634-complete-*`; the complete run made
no screenshot or manual visual-quality claim. A separate diagnostic Guide screenshot was actually
inspected while identifying the moving-pixel demo defect.

Focused **525-C-sprite-font** adds `c_api.graphics.sprite_font_state`, a26-control caller-built
SpriteFont workflow covering all seven previously uncredited `sprite_font.h` routes. It proves the
constructor copies caller glyph storage, both parallel collections preserve exact order and are
atomic on short output, asymmetric bounds/cropping/kerning drive exact multiline measurement,
valid mutable fallback/spacing state survives rejected changes, clearing fallback restores missing-
glyph refusal, and the font retains its atlas. Direct execution plus SOFTWARE and OPENGLES3 runner
selections pass with byte-identical output and empty stderr. All24 focused C SpriteFont, C++
SpriteFont/CNJ and CLI tests pass in12.78s. Metadata is525/0 and coverage rises1109->1116/3205,
completing `sprite_font.h`2->9/9. The incremental build is diagnostic-free. Clean sibling CNA is
`b2ded99da9`; production library hashes remain unchanged. Nothing is drawn and no screenshot or
visual inspection is claimed. Evidence:`checkpoint-525-c-sprite-font-focused-*`.

Complete **524-C-textures**, source `b5b4ec2`, absorbs the explicit MediaPlayer pump and all three
texture batches after complete521. `tools/validate.sh` passes metadata524/0 and all537 enabled
CTests in271.80s; three declared tests remain disabled. SOFTWARE is456pass/30known-fail/35skip/
3not-run and OPENGLES3 is487/31/3/3, with zero unexpected passes. Every complete521 status/
knownIssue pair remains exact; all three new texture IDs pass on both renderers and the extended
MediaPlayer keeps its passing status. Fresh runtime coverage is297/329 types,1405/2326 XNA members
and1109/3205 C routes. The build log is diagnostic-free. Clean sibling CNA is `b2ded99da9`;
production `libcna.so` and `libcna_c_api.so` remain SHA-256
`3b533283162bbcc5a0fe9a9f6c767f52f91eefe557dda2ee04d5f1cbdcc604e0` and
`730ac2631f65b402d5d016ae85649067441b52c973e49f786b49b3253d75100a`; `cna-demos` is
`3cc772719b3e44c5c57dea583c0d611ed3319f3fc100d1b520bf8e465aca03bd`. No screenshot or visual
inspection was performed. Evidence:`checkpoint-524-complete-*`.

Complete **521-C-effects**, source `811d2f4`, absorbs all seventeen effect demos and the final
ContentManager Effect-load extension added after complete504. `tools/validate.sh` passes metadata
521/0 and all534 enabled CTests in233.86s; three declared tests remain disabled. SOFTWARE is453
pass/30 known-fail/35 skip/3 not-run and OPENGLES3 is484/31/3/3, with zero unexpected passes.
Against complete504, every one of the504 existing status/knownIssue pairs is unchanged; the17 new
IDs contribute16 passes and the expected ISSUE-070 failure on each renderer. Fresh runtime coverage
is297/329 types,1405/2326 XNA members and1083/3205 C routes; `effects.h` is complete290/290.
The build log is diagnostic-free. Clean sibling CNA is `a6bb279d47`; production `libcna.so` and
`libcna_c_api.so` remain SHA-256
`3b533283162bbcc5a0fe9a9f6c767f52f91eefe557dda2ee04d5f1cbdcc604e0` and
`730ac2631f65b402d5d016ae85649067441b52c973e49f786b49b3253d75100a`; `cna-demos` is
`15adb468549aec96966f9a9f2a5bd2947ca0a3e9cdb4d3bb57e1023043965735`. No screenshot or visual
inspection was performed. Evidence:`checkpoint-521-complete-*`.

Focused **524-C-texture-volume** adds `c_api.graphics.texture_volume_data`, a19-control HiDef
workflow covering all eight meaningful remaining `texture_volume.h` routes. Exact specific/common
metadata, offset-window and raw-byte volume transfers, all six cube faces, atomic short reads and a
generated in-memory DXT1 cube DDS pass directly and under both renderer selections with byte-
identical output and empty stderr. The DDS source decodes to public Color storage; the initial demo
expectation of retained DXT1 metadata was corrected and was not a CNA defect. All35 focused C
Texture/Texture/CLI tests satisfy their contracts in23.95s with two declared SOFTWARE capability
skips. Metadata is524/0 and coverage rises1101->1109/3205, with `texture_volume.h`4->12/16. Its
four fixed type-name routes remain intentionally trivial/uncredited, completing the semantic audit.
Nothing is sampled or drawn and no visual inspection is claimed. Clean sibling CNA is
`b2ded99da9` and production library hashes are unchanged.
Evidence:`checkpoint-524-c-texture-volume-focused-*`.

Focused **523-C-texture2d-I/O** adds `c_api.graphics.texture2d_io`, a27-control standalone/device
workflow covering all12 remaining meaningful `texture.h` resource routes. It proves empty versus
file-backed CPU storage, exact full and rectangle transfers, atomic PNG/JPEG output, private file
round-trip, device-backed file/RGBA creation, raw replacement and encoded-memory resize/crop before
complete child cleanup. Direct execution and both renderer selections pass with byte-identical
output and empty stderr. All34 focused C Texture/Texture/CLI tests satisfy their contracts in
18.83s, with two declared SOFTWARE capability skips. Metadata is523/0 and coverage rises1089->
1101/3205, with `texture.h`7->19/21. Its fixed Texture2D type-name pair is intentionally trivial/
uncredited, completing the header's semantic audit. No draw, screenshot or visual inspection is
claimed. Clean sibling CNA is `b2ded99da9` and production library hashes are unchanged.
Evidence:`checkpoint-523-c-texture2d-io-focused-*`.

Focused **522-C-texture-formats** adds `c_api.graphics.texture_formats` for the five
renderer-independent format routes. Six deterministic controls cover all27 SurfaceFormat
identities: exact compressed-block footprint, texel/block byte size, pixel-store alignment,
compatible transfer widths and the base Texture policy that accepts only Color. Invalid identities
preserve getter output. Direct execution and both renderer selections pass with byte-identical
output and empty stderr. All32 focused Texture/CLI CTests satisfy their contracts in34.25s, with
two declared SOFTWARE capability skips. Metadata is522/0, the build is clean, and coverage rises
1084->1089/3205 with `texture.h`2->7/21. No resource, backend-support, pixel or visual claim is
made. Final observation sees clean sibling CNA `b2ded99da9`; the only change since the media batch
is a binding-plan documentation commit, and both production library hashes are unchanged.
Evidence:`checkpoint-522-c-texture-formats-focused-*`.

Focused **521-C-player-update** extends `c_api.media.media_player` from92to94 controls without a new
ID. After the game loop ends, a real short track finishes while the public state remains Playing;
`cna_media_player_update_ext` then consumes the pending native end notification and transitions the
one-song, non-repeating queue to Stopped. Direct execution and both runner selections pass;
filtering the two additions restores all92 older labels in order, runner output is byte-identical
and all three stderr files are empty. All23 focused MediaPlayer/Song/CLI CTests pass in44.43s,
metadata remains521/0 and the incremental build is diagnostic-free. Coverage rises1083->1084/3205
and `media_player.h`38->39/41. Its two remaining fixed MediaQueue type-name helpers are already
verified by CNA's pure-C smoke and remain intentionally trivial/uncredited, completing the header's
semantic audit. Clean sibling CNA is `a6bb279d47`; production library hashes are unchanged. No
audible quality, callback-latency or visual claim is made.
Evidence:`checkpoint-521-c-player-update-focused-*`.

Focused **521-C-effect-load** extends `c_api.content.content_manager` with a deterministically
generated, authored BasicEffect CNJ. The load preserves exact diffuse/emissive colors, specular
power, alpha and vertex-color state; applies through the ordinary Effect interface; and remains
authored and usable after both ContentManager.Unload and manager destruction before its independent
handle releases. The existing texture/font/cache/root/draw workflow remains intact. All53 controls
pass directly and through both runner selections with byte-identical runner output and empty stderr.
All33 focused ContentManager/BasicEffect/CLI tests pass in14.38s. Metadata remains521/0 and the
incremental build is clean. The final route raises coverage1082->1083/3205 and completes
`effects.h`289->290/290. Clean sibling CNA is `a6bb279d47`; the production library baseline is
unchanged. Back-buffer pixels for the existing texture/font draw were read programmatically; no
screenshot or visual inspection is claimed. Evidence:`checkpoint-521-c-effect-load-focused-*`.

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

The completed `media_player.h` audit demonstrates39/41 routes. The last behavioral route,
`cna_media_player_update_ext`, observes a native track-end transition outside the automatic game
loop. The queue type-name size/copy pair is fixed generic reflection text already covered by CNA's
pure-C smoke and remains intentionally trivial/uncredited. Every route in the header has a recorded
semantic disposition.

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
| HEADLESS, STUB, OPENGL33, SDL_GPU | compiled in; not yet run in a recorded session |
| SDL_RENDERER, VULKAN | exercised by historical checkpoints; retired by CNA and removed from current presets |
| WEBGL2, WEBGPU, DIRECTX9, FNA3D | not built (other platforms / extra deps) |
| DIRECTX11, METAL | retired by CNA; never exercised by this project |

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
CNA-DEMOS-ISSUE-021 (resolved by CNA `a6e8104d2`: EasyGL samples rendered RenderTargetCube faces
upright), CNA-DEMOS-ISSUE-022 (a compiled glTF with an
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

HIGHEST PRIORITY: Complete744 is the current full-build/runtime baseline; focused745-780 close the
texture/render-target lifetimes, ContentManager disposal, CNB/CNJ registration and ReadAsset
recorder boundaries plus component selection/environment propagation. ISSUE-087 records omitted
disposable shared-table values, ISSUE-088 records partial intermediate-serializer registration and
ISSUE-089 records the missing custom-serializer Initialize callback, and ISSUE-090 records the
custom XNB writer Initialize/GetTypeWriter deadlock.
Root-as-shared-resource identity and self-cycle behavior now match the measured XNA contract in
`content.custom.intermediate_shared_resources`, and a non-shared self-cycle now exercises the
writer's public `MaxNestingDepth` refusal. The typed envelope now covers direct reader/writer type-
name/context accessors plus the ScanChildren namespace prepass, and the registration control binds
its exact `TargetType` and `XmlTypeName` to the looked-up instance and emitted XML. The custom XNB
writer now covers identity/lifecycle, Windows Phone/HiDef context, compression refusal, explicit
typed/raw nested writer dispatch, dynamic reference dispatch, shared-resource identity and external
path normalization. Audit the remaining custom writer null/refusal contracts and ContentCompiler
inspection/freeze behavior against existing demos before adding controls. Ordinary, shared and
external nulls now round-trip too; raw-null and missing/foreign writer refusals are covered. Audit
ContentCompiler inventory/freeze behavior is now covered, and implementation-facing worker,
active-writer/output plus no-op façade members are dispositioned. The custom writer audit is now
closed: a default writer honors requested LZX and round-trips through ContentManager, while the
empty ContentTypeWriterAttribute is dispositioned as a registration marker. Continue the
MediaPlayer collection overloads now cover the default zero start, live queue replacement and a
nonzero start whose queue-owned copies outlive caller objects. Bounded shuffled traversal now proves
queue index/identity/state consistency without assuming a random order. Audit paused PlayPosition,
resume and Stop-reset behavior next without duplicating the completed C route coverage. Do not add
isolated getter demos. Build with at most three compile jobs and keep focused evidence current.

### Focused748 TextureCube cross-device lifecycle and EasyGL teardown

`graphics.texture_cube.face_transfers` grows from eight to twelve controls. An occupied primary
cube receives a copied 2x2 mipmapped source from a secondary HiDef device, changes owner/shape/data,
and detaches its old primary slot. A later source face edit is shared through the assigned wrapper.
Move construction transfers the source's active secondary slot and retains that payload; explicit
secondary-device disposal marks both registered wrappers disposed, and the refused destination
read preserves every output sentinel. All four lifecycle controls pass on both renderers.

SOFTWARE passes12/12 and the exact CTest passes. OPENGLES3 passes those ten non-framebuffer
transfer/lifecycle controls but fails the two established atlas checks because the next primary
frame is completely black. A diagnostic run of the existing Texture3D two-device workflow produces
the same all-black screenshot despite its25 logical controls passing, identifying a common EasyGL
multi-device teardown defect rather than lost cube data. CNA-DEMOS-ISSUE-086 records it and the
demo is marked only for OPENGLES3. The incremental build is warning-free at `-j3`, metadata remains
636/0, tool self-tests pass5/5 and4/4, and generated coverage remains302/329 types,2114/2326
members,169/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-748-texture-cube-final-*`, `/tmp/checkpoint-748-failed-gl.png` and
`/tmp/checkpoint-748-texture3d-gl.png`. The two black diagnostic captures were inspected; no
physical-GPU or non-OPENGLES3 runtime claim is made. No CNA source was modified.

### Focused749 Texture2D cross-device copy assignment

`graphics.texture2d.copy_ownership` grows from six to ten controls. An occupied primary target is
copy-assigned from a2x2 secondary-device source, takes its owner/bounds/payload and detaches the old
primary slot. A partial source edit remains shared. The assigned target's simple full Color upload
then detaches only that writing wrapper, leaving the source exact. After explicit target disposal,
copy assignment revives it as a live registered secondary wrapper with the shared source payload.

SOFTWARE and OPENGLES3/Xvfb pass10/10 with empty stderr, and the exact SOFTWARE CTest passes.
Metadata remains636/0, tool self-tests pass5/5 and4/4, and coverage remains302/329 types,
2114/2326 members,169/753 fields and2999/3205 C routes. The first `-j3` incremental build succeeded
but one concurrent content step emitted a transient staging-scavenger race warning; the immediate
`-j1` retry was clean, and both obeyed the three-job ceiling. Evidence:
`/tmp/checkpoint-749-texture2d-*` and `/tmp/checkpoint-749-coverage.md`. Primary framebuffer checks
were programmatic; cross-device data was read back but not drawn. No screenshot or manual visual
inspection was performed, and no CNA issue was found.

### Focused750 RenderTarget2D bound move identity

`graphics.render_target.render_target2d` grows from three to nine controls. A bound 32x16 target
refuses Dispose and move assignment without changing its live binding, either move operand's
shape/properties or an authored Gold pixel. Move construction from an active 128x128 source then
updates `GetRenderTargets` to the constructed wrapper; move assignment into the unbound destination
updates that binding again and transfers the complete target shape. An Orange clear through the
transferred binding reads back exactly after unbinding.

SOFTWARE and OPENGLES3/Xvfb pass9/9, and the exact SOFTWARE CTest passes. The warning-free
incremental build uses `-j3`; metadata remains636/0, tool self-tests pass5/5 and4/4, and coverage
remains302/329 types,2114/2326 members,169/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-750-render-target-*` and `/tmp/checkpoint-750-coverage.md`. Pixel checks were
programmatic; no screenshot or manual visual inspection was performed. No CNA issue was found.

### Focused751 RenderTargetCube bound disposal

`graphics.render_target.render_target_cube` grows from seven to nine controls. Its NegativeZ face
is bound and cleared Gold before public Dispose is attempted. The call is refused while the target
stays live and `GetRenderTargets` retains the exact cube/face pair. After unbinding, one exact Gold
texel proves that the face storage remains intact. The original dynamic environment rendering,
preservation and sampling controls continue unchanged.

SOFTWARE and OPENGLES3/Xvfb pass9/9, and the exact SOFTWARE CTest passes. The warning-free
incremental build uses `-j3`; metadata remains636/0, tool self-tests pass5/5 and4/4, and coverage
remains302/329 types,2114/2326 members,169/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-751-render-target-cube-*` and `/tmp/checkpoint-751-coverage.md`. Pixel checks were
programmatic; no screenshot or manual visual inspection was performed. No CNA issue was found.

### Focused752 RenderTarget2D bound destruction and address reuse

`graphics.render_target.render_target2d` grows from nine to thirteen controls. Destroying an
optional-backed active target removes its dangling public binding, but `Present` remains refused
until an explicit null-target transition restores the saved back-buffer viewport. Reconstructing
the target in the same optional storage yields a genuine new binding and24x12 viewport; a
CornflowerBlue clear through it reads back exactly after unbinding. The explicit Dispose and move
identity controls from focused750 remain intact.

SOFTWARE and OPENGLES3/Xvfb pass13/13, and the exact SOFTWARE CTest passes. The warning-free
incremental build uses `-j3`; metadata remains636/0, tool self-tests pass5/5 and4/4, and coverage
remains302/329 types,2114/2326 members,169/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-752-render-target-*` and `/tmp/checkpoint-752-coverage.md`. Pixel checks were
programmatic; no screenshot or manual visual inspection was performed. No CNA issue was found.

### Focused753 ContentManager disposal dispatch and refusal parity

`content.content_manager.read_asset_ownership` grows from sixteen to eighteen controls. Disposal
through `IDisposable` reaches a derived protected `Dispose(true)` override, and repeated public
disposal reaches that hook again while base cleanup stays idempotent. Its exact disposed ReadAsset
refusal remains passing; seventeen controls pass and only the unchanged default-recorder control
fails as CNA-DEMOS-ISSUE-011.

`content.content_manager.several_managers` grows from two to three controls and becomes an exact
CNA-DEMOS-ISSUE-010 reproduction: disposed `Load<T>` throws std::runtime_error instead of
ObjectDisposedException before any asset read. Both renderers reproduce the two known issues with
identical assertion texts, and both exact SOFTWARE CTests pass as expected failures. The
warning-free incremental build uses `-j3`; metadata remains636/0, known-issue demos increase38->39,
self-tests pass5/5 and4/4, and coverage remains302/329 types,2114/2326 members,169/753 fields and
2999/3205 C routes. Evidence:`/tmp/checkpoint-753-*` and `/tmp/checkpoint-753-coverage.md`. No
screenshot or manual inspection was performed, and no new CNA issue was found.

### Focused754 CNB loader registration precedence and lifetime

`content.custom.cnb_loader` grows from twelve to fourteen controls and becomes `ownProcess` to
isolate its process-global registration. Two observably different factories register one custom
ID/name: the registry retains the first factory capture, releases the ignored replacement capture
and decodes Moon rather than the replacement marker. That first factory remains live after one
manager is destroyed and another is created, while their caches remain independent. All previous
schema, repair and refusal checks stay intact.

SOFTWARE and OPENGLES3/Xvfb pass14/14, and the exact SOFTWARE CTest passes. The warning-free
incremental build uses `-j3`; metadata remains636/0, tool self-tests pass5/5 and4/4, and coverage
remains302/329 types,2114/2326 members,169/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-754-cnb-loader-*` and `/tmp/checkpoint-754-coverage.md`. No rendering, screenshot
or manual inspection was used, and no CNA issue was found.

### Focused755 CNB loader registration guards

`content.custom.cnb_loader` grows from fourteen to seventeen controls. An empty factory, CNA's
built-in Curve identifier, the first reserved identifier and a valid custom identifier paired with
a different canonical name are all refused with `std::invalid_argument`. Every refusal preserves
the installed first factory capture; the later Moon load still reaches that factory and proves the
valid process-global registration remains usable.

SOFTWARE and OPENGLES3/Xvfb pass17/17, and the exact SOFTWARE CTest passes. The warning-free
incremental build uses `-j3`; metadata remains636/0, tool self-tests pass5/5 and4/4, and coverage
remains302/329 types,2114/2326 members,169/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-755-cnb-loader-*` and `/tmp/checkpoint-755-coverage.md`. No rendering, screenshot
or manual inspection was used, and no CNA issue was found.

### Focused756 CNJ envelope dispatch and retry

`content.custom.cnj_loader` grows from eleven to fifteen controls. Private documents carrying an
unknown type name or future `cnjVersion` are refused with contextual `ContentLoadException`
messages before the registered factory runs. Rewriting each exact file with a supported envelope
and retrying it through the same manager produces the authored Enemy and increments the factory
once, proving failed loads do not publish cache entries.

SOFTWARE and OPENGLES3/Xvfb pass15/15, and the exact SOFTWARE CTest passes. The warning-free
incremental build uses `-j3`; metadata remains636/0, tool self-tests pass5/5 and4/4, and coverage
remains302/329 types,2114/2326 members,169/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-756-cnj-loader-*` and `/tmp/checkpoint-756-coverage.md`. No rendering, screenshot
or manual inspection was used, and no CNA issue was found.

### Focused757 CNJ registration guards

`content.custom.cnj_loader` grows from fifteen to eighteen controls. Empty type names and empty
factories are rejected with `std::invalid_argument` before claiming the game-owned C++ type. A
custom CNJ registration for the built-in Texture2D type is rejected with `std::logic_error` before
its unreachable callback runs. The subsequent valid Enemy registration and repaired-path loads
still succeed, proving refused calls leave manager state usable.

SOFTWARE and OPENGLES3/Xvfb pass18/18, and the exact SOFTWARE CTest passes. The warning-free
incremental build uses `-j3`; metadata remains636/0, tool self-tests pass5/5 and4/4, and coverage
remains302/329 types,2114/2326 members,169/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-757-cnj-loader-*` and `/tmp/checkpoint-757-coverage.md`. No rendering, screenshot
or manual inspection was used, and no CNA issue was found.

### Focused758 ReadAsset callback and partial-failure ownership

`content.content_manager.read_asset_ownership` grows from eighteen to twenty-one controls. Its
regenerated XNB has two typed tile fields resolved by the custom reader to one disposable identity.
The supplied callback receives both occurrences while the caller deduplicates them into one
ownership slot. Removing the final null-field byte proves a later parse failure preserves that
already delegated resource for explicit cleanup. A callback-thrown sentinel propagates while the
short-read stream and unclaimed object unwind.

SOFTWARE and OPENGLES3/Xvfb each pass20/21; only the unchanged default-recorder assertion fails as
CNA-DEMOS-ISSUE-011, and the expected-failure SOFTWARE CTest passes. The ownership asset
regenerates byte-identically. The warning-free incremental build uses `-j3`; metadata remains636/0,
known-issue demos remain39, tool self-tests pass5/5 and4/4, and coverage remains302/329 types,
2114/2326 members,169/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-758-read-asset-*` and `/tmp/checkpoint-758-coverage.md`. No rendering, screenshot
or manual inspection was used, and no new CNA issue was found.

### Focused759 XNB shared-resource disposal recording

`content.xnb.shared_resources` grows from fifteen to seventeen controls. Its existing Item now
implements `IDisposable`, and a derived manager exposes protected ReadAsset with a supplied
recorder. The callback receives the typed inline Item, proving it is installed and functional, but
omits both Items materialized from the type-erased shared-resource table. All established
alias/fixup/cache/failure/lifetime controls remain intact. CNA-DEMOS-ISSUE-087 records the defect.

SOFTWARE and OPENGLES3/Xvfb each pass16/17, and the expected-failure SOFTWARE CTest passes. The
warning-free incremental build uses `-j3`; metadata remains636/0, known-issue demos increase39->40,
tool self-tests pass5/5 and4/4, and coverage remains302/329 types,2114/2326 members,169/753 fields
and2999/3205 C routes. Evidence:`/tmp/checkpoint-759-shared-resources-*` and
`/tmp/checkpoint-759-coverage.md`. No rendering, screenshot or manual inspection was used.

### Focused760 CNB public read-limit coverage

`content.custom.cnb_read_limits` grows from eleven to sixteen controls. Its NOTE chunk now requests
64-byte alignment and the parsed entry proves both the declared alignment and physical offset.
Alongside the existing file, per-chunk and aggregate byte limits, writer and reader now
symmetrically refuse a two-entry table under a one-entry chunk limit, the aligned NOTE under a
32-byte alignment limit, the CMET canonical type name under a shorter string limit and a two-row
XREF table under a one-element array limit. These cases exercise all seven public `CnbReadLimits`
fields with real container data while retaining separate schema-level ReadString/ReadCount checks.

SOFTWARE and OPENGLES3/Xvfb pass16/16, and the exact SOFTWARE CTest passes. The warning-free
incremental build uses `-j3`; metadata remains636/0, known-issue demos remain40, tool self-tests
pass5/5 and4/4, and generated coverage remains302/329 types,2114/2326 members,169/753 fields and
2999/3205 C routes. Evidence:`/tmp/checkpoint-760-cnb-limits-*` and
`/tmp/checkpoint-760-coverage.md`. No rendering, screenshot or manual inspection was used, and no
CNA issue was found.

### Focused761 CNB custom-type identity and real hash collision

`content.custom.cnb_loader` grows from seventeen to twenty-two controls. The writer now visibly
refuses a custom file with no canonical metadata and one whose name hashes to another identifier;
a parsed valid rules document also refuses a direct `RequireAsset` request for the wrong numeric
type. A fixed pair of distinct canonical names demonstrates a real collision at `0xB412AA0B`.
The writer accepts the Beta file because its own name hashes correctly, but ContentManager compares
that full name with the registered Alpha name and refuses it before Alpha's factory runs. Rewriting
the same path with Alpha then loads marker42 through the same manager, proving the failure was not
cached.

SOFTWARE and OPENGLES3/Xvfb pass22/22, and the exact SOFTWARE CTest passes. The warning-free
incremental build uses `-j3`; metadata remains636/0, known-issue demos remain40, tool self-tests
pass5/5 and4/4, and generated coverage remains302/329 types,2114/2326 members,169/753 fields and
2999/3205 C routes. Evidence:`/tmp/checkpoint-761-cnb-loader-*` and
`/tmp/checkpoint-761-coverage.md`. No rendering, screenshot or manual inspection was used, and no
CNA issue was found. The remaining physical header/table corruption variants are intentionally
left to CNA's exhaustive container unit tests rather than repeated as demo assertions.

### Focused762 CNB external child-file type and retry

`content.custom.cnb_external_references` grows from nine to eleven controls. Its parent XREF
correctly declares the enemy-profile type, but the first artifact at that logical child path is a
complete CNB for a separately registered DecoyProfile. The decoy loader runs, then ContentManager
refuses the result as the requested SharedProfile; unchanged profile/encounter counters prove the
parent was not published. Replacing the exact child path with the expected profile lets the same
manager retry child and parent, returning count4 and health40 without another decoy read.

SOFTWARE and OPENGLES3/Xvfb pass11/11, and the exact SOFTWARE CTest passes. The warning-free
incremental build uses `-j3`; metadata remains636/0, known-issue demos remain40, tool self-tests
pass5/5 and4/4, and generated coverage remains302/329 types,2114/2326 members,169/753 fields and
2999/3205 C routes. Evidence:`/tmp/checkpoint-762-cnb-xref-*` and
`/tmp/checkpoint-762-coverage.md`. No rendering, screenshot or manual inspection was used, and no
CNA issue was found. Runtime XREF traversal is explicitly owned by the custom loader; automatic
child builds and graph-cycle checks belong to the existing nested-build/dependency-graph workflows.

### Focused763 native pipeline component selection

`content.custom.pipeline_components` adds eight controls, reaching23. Two ordinary candidates at
each stage make one real Build request fail successively with importer, processor and writer
ambiguity as stable-name overrides resolve the preceding stage. Naming all three original
components produces the established bytes. Explicit processor and writer names whose declared
input types reject the preceding value fail at Selection without fallback. Separately registered
name-only importer and processor candidates do not disturb conventional routing, but explicit
names select both and preserve the output.

SOFTWARE and OPENGLES3/Xvfb pass23/23, and the exact SOFTWARE CTest passes. The warning-free
incremental build uses `-j3`; metadata remains636/0, known-issue demos remain40, tool self-tests
pass5/5 and4/4, and generated coverage remains302/329 types,2114/2326 members,169/753 fields and
2999/3205 C routes. Evidence:`/tmp/checkpoint-763-pipeline-components-*` and
`/tmp/checkpoint-763-coverage.md`. No rendering, screenshot or manual inspection was used, and no
CNA issue was found. Registry inventory views and direct resolution queries remain ordinary
inspection. Audit meaningful `ContentBuildEnvironment` platform/configuration/intermediate-
directory propagation next before deeper serializer behavior.

### Focused764 XNA build-environment propagation

`content.custom.xna_pipeline_components` adds three passing controls, reaching20 passes and the two
established ISSUE-047/048 failures. One real Build requests WindowsPhone, configuration Profile and
a private intermediate directory. The emitted CNB proves the XNA processor and environment-aware
native writer receive the same platform and configuration. It also proves the XNA importer,
processor and native writer receive the same scratch path, while the processor creates and the
demo reads an exact environment file there. The writer's schema and component versions advance to
two because its emitted representation now carries those fields.

SOFTWARE and OPENGLES3/Xvfb each retain exactly20/22 child passes with empty child stderr; the
three new labels pass and the two old labels fail. The expected-failure SOFTWARE CTest passes. The
warning-free incremental build uses `-j3`; metadata remains636/0, known-issue demos remain40, tool
self-tests pass5/5 and4/4, and generated coverage remains302/329 types,2114/2326 members,169/753
fields and2999/3205 C routes. Evidence:`/tmp/checkpoint-764-xna-environment-*` and
`/tmp/checkpoint-764-coverage.md`. No rendering, screenshot or manual inspection was used, and no
new CNA issue was found. Direct in-memory environment propagation is covered; the compiler host's
own scratch-directory policy is not inferred. Continue `content.custom.intermediate_serializer`
with implicit format naming and optional empty-collection semantics.

### Focused765 intermediate format-name and empty Optional collection

`content.custom.intermediate_serializer` adds two controls, reaching16/16. One reusable
ContentSerializerAttribute deliberately sets Optional without ElementName and is applied to two
integer collections. The populated Patrol emits under its own described member name and round
trips, proving Format preserves each binding's implicit name. Empty Reinforcements is omitted;
deserialization leaves its nonempty constructor default `{9}` intact rather than replacing it with
an empty vector. The established scalar optional/null, required, ignored, renamed and custom item-
name controls remain passing.

SOFTWARE and OPENGLES3/Xvfb both pass all16 child labels; a direct child run has empty stderr, and
the exact SOFTWARE CTest passes. The warning-free incremental build uses `-j3`; metadata remains636/0,
known-issue demos remain40, tool self-tests pass5/5 and4/4, and generated coverage remains302/329
types,2114/2326 members,169/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-765-intermediate-serializer-*` and `/tmp/checkpoint-765-coverage.md`. No rendering,
screenshot or manual inspection was used, and no CNA issue was found. Audit collection-level
CollectionItemName inheritance/override behavior in `content.custom.intermediate_properties` next.

### Focused766 intermediate serializer registration collision

The collection-level CollectionItemName audit found no executable serializer behavior: the public
descriptor stores, returns and conditionally inherits the string, but no CNA content or content-
pipeline consumer reads `ContentTypeSerializerBase::CollectionItemName`. It is dispositioned as
metadata awaiting a real consumer rather than credited through getter assertions.

`content.custom.intermediate_type_serializer` adds three controls, reaching9/10 passes. Registering
an alternate serializer for the already-owned DiceRoll target returns the first instance and does
not install the later shortcut. A distinct CoinFlip serializer with the already-owned `dice`
shortcut is silently installed under its type/full name but not the shortcut. It emits `<Asset
Type="dice">3c</Asset>`, which resolves to DiceRoll and cannot deserialize as CoinFlip.
CNA-DEMOS-ISSUE-088 records this partial, non-round-trippable registration.

SOFTWARE and OPENGLES3/Xvfb reproduce the same single failed invariant, and the expected-failure
SOFTWARE CTest passes. A direct child run confirms nine passing labels, one failure and empty
stderr. The warning-free incremental build uses `-j3`; metadata remains636/0, known-issue demos
increase40->41, tool self-tests pass5/5 and4/4, and generated coverage remains302/329 types,
2114/2326 members,169/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-766-intermediate-registration-*` and `/tmp/checkpoint-766-coverage.md`. No
rendering, screenshot or manual inspection was used. Continue with root-as-shared-resource identity
and self-cycle behavior in `content.custom.intermediate_shared_resources`.

### Focused767 intermediate root self-cycle

`content.custom.intermediate_shared_resources` adds three passing controls, reaching19/19. A root
LootTable whose shared Fallback points back to itself follows the measured XNA contract: the XML
writes the root again as exactly one Resource and both Fallback edges target that resource. After
deserialization the root and resource are distinct copies, while the resource's edge closes on its
own identity. Explicit unlinking and weak handles prove that the authored root and both loaded
identities are released.

SOFTWARE and OPENGLES3/Xvfb pass19/19 with byte-identical stdout and empty stderr; the exact
SOFTWARE CTest passes. The warning-free incremental build uses at most three jobs; metadata remains
636/0, known-issue demos remain41, validator/checkpoint self-tests pass5/5 and4/4, and generated
coverage remains302/329 types,2114/2326 non-field members,169/753 fields and2999/3205 C routes.
Evidence:`/tmp/checkpoint-767-intermediate-shared-*` and `/tmp/checkpoint-767-coverage.md`. No
rendering, screenshot or manual inspection was used. No CNA issue was found. Continue with the
public MaxNestingDepth boundary for malformed non-shared object graphs.

### Focused768 intermediate writer depth guard

`content.custom.intermediate_shared_resources` adds one passing control, reaching20/20. A separate
InlineNode deliberately points to itself through an ordinary non-shared field. Namespace scanning
terminates by identity, then serialization reaches the public writer `MaxNestingDepth` of1024 and
throws `InvalidContentException` whose diagnostic identifies the non-shared cycle. The authored
edge is explicitly broken after the attempt.

SOFTWARE and OPENGLES3/Xvfb pass20/20 with byte-identical stdout and empty stderr; the exact
SOFTWARE CTest passes. The warning-free incremental build uses at most three jobs; metadata remains
636/0, known-issue demos remain41, validator/checkpoint self-tests pass5/5 and4/4, and generated
coverage remains302/329 types,2114/2326 non-field members,169/753 fields and2999/3205 C routes.
`MaxNestingDepth` is a CNA extension outside the XNA member inventory. Reader depth is deliberately
not duplicated: tinyxml2's roughly500-level XML ceiling is reached before the reader's1024-level
guard, and CNA's own hardening suite already checks malformed550-level refusal and400-level input.
Evidence:`/tmp/checkpoint-768-intermediate-depth-*` and `/tmp/checkpoint-768-coverage.md`. No
rendering, screenshot or manual inspection was used. No CNA issue was found. Continue with direct
type-name/context behavior in the custom intermediate serializer workflows.

### Focused769 intermediate typed-envelope context

`content.custom.intermediate_type_serializer` adds six passing controls while retaining
CNA-DEMOS-ISSUE-088's one failure, reaching15/16 passes. A custom typed envelope resolves the dice
codec through the active writer Serializer, emits a nested `Type="dice"` with WriteTypeName, then
uses MoveToElement, reader Xml/Serializer, ReadTypeName and explicit ReadObject dispatch to restore
the value. Focused malformed documents prove that an absent Payload, missing Type, unknown Type and
a registered but schema-incompatible string Type all fail at their intended boundary.

SOFTWARE and OPENGLES3/Xvfb reproduce the same15 passes and one established failure with byte-
identical stdout and empty stderr; the expected-failure SOFTWARE CTest passes. The warning-free
incremental builds use at most three jobs; metadata remains636/0, known-issue demos remain41,
validator/checkpoint self-tests pass5/5 and4/4, and generated runtime coverage remains302/329
types,2114/2326 non-field members,169/753 fields and2999/3205 C routes. The build-time pipeline APIs
are listed as CNA-specific/unmatched by that runtime inventory. Evidence:
`/tmp/checkpoint-769-intermediate-context-*` and `/tmp/checkpoint-769-coverage.md`. No rendering,
screenshot or manual inspection was used. No new CNA issue was found. Continue with the custom
serializer Initialize/ScanChildren namespace prepass.

### Focused770 intermediate Initialize and child scan

The typed envelope in `content.custom.intermediate_type_serializer` now contains a described Attack
without an XML shortcut. Its ScanChildren override reports that value through the object serializer,
so the namespace prepass declares `xmlns:CnaDemos` before WriteTypeName emits the nested
`CnaDemos:Attack`; both the dice and Attack payloads round-trip. The serializer's Initialize override
records and would cache the Attack serializer, but CNA never invokes it. CNA-DEMOS-ISSUE-089 records
the missing documented lifecycle callback; the demo uses a fallback lookup only to keep the child-
scan and round-trip controls observable.

SOFTWARE and OPENGLES3/Xvfb reproduce16 passes and two failures, ISSUE-089 plus established
ISSUE-088, with byte-identical stdout and empty stderr; the primary ISSUE-088 expected-failure
SOFTWARE CTest passes. The warning-free incremental build uses at most three jobs; metadata remains
636/0, known-issue demos remain41, validator/checkpoint self-tests pass5/5 and4/4, and generated
runtime coverage remains302/329 types,2114/2326 non-field members,169/753 fields and2999/3205 C
routes. Evidence:`/tmp/checkpoint-770-intermediate-scan-*` and `/tmp/checkpoint-770-coverage.md`. No
rendering, screenshot or manual inspection was used. Continue with ObjectIsEmpty and existing-
instance serializer contracts before closing this intermediate serializer audit.

### Focused771 intermediate empty and existing contracts

Two existing tools close the remaining observable ContentTypeSerializer value contracts.
`intermediate_type_serializer` overrides typed ObjectIsEmpty for zero-count DiceRoll values. An
optional zero-count bonus is omitted before its rejecting Serialize path, while its required peer
still emits; a valid2d12 bonus emits and both documents round-trip. The tool reaches18 passes plus
the two established ISSUE-088/089 failures. `intermediate_existing_instances` adds a custom
MeterSlot serializer whose CanDeserializeIntoExistingObject is true. Through a described read-only
property it receives the constructor-created object, preserves its pointer and runtime-only token,
updates only health and stays independent of the authored instance, reaching12/12 passes.

Both tools have byte-identical SOFTWARE/OPENGLES3 Xvfb output and empty stderr. Their two exact
SOFTWARE CTests pass, including the expected-failure contract. The warning-free incremental build
uses at most three jobs; metadata remains636/0, known-issue demos remain41,
validator/checkpoint self-tests pass5/5 and4/4, and generated runtime coverage remains302/329
types,2114/2326 non-field members,169/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-771-*` and `/tmp/checkpoint-771-coverage.md`. No rendering, screenshot or manual
inspection was used. Close the custom serializer identity-property audit, then inspect the custom
ContentTypeWriter/ContentWriter XNB path.

### Focused772 intermediate serializer identity

The existing registration control in `content.custom.intermediate_type_serializer` now binds the
same returned and looked-up serializer instance to its exact DiceRoll `TargetType` and `dice`
`XmlTypeName`. The shortcut is already consumed by the emitted root `Type` attribute and successful
round trip, so both identity properties are covered in their real registry context without a
getter-only demo or another assertion label.

SOFTWARE and OPENGLES3/Xvfb retain eighteen passes and the two established ISSUE-088/089 failures
with byte-identical stdout and empty stderr; the expected-failure SOFTWARE CTest passes. The
warning-free incremental build uses at most three jobs; metadata remains636/0, known-issue demos
remain41, validator/checkpoint self-tests pass5/5 and4/4, and generated runtime coverage remains
302/329 types,2114/2326 non-field members,169/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-772-intermediate-identity-*` and `/tmp/checkpoint-772-coverage.md`. No rendering,
screenshot or manual inspection was used. No CNA issue was found. Audit the custom
ContentTypeWriter/ContentWriter XNB path next.

### Focused773 custom XNB writer lifecycle and context

`content.custom.xml_xnb_pipeline` grows from ten passing controls to twelve passes plus one focused
failure. Its factory retains the registered ProfileWriter contract: exact authoring TargetType,
runtime type and reader, version two and the default false existing-object policy. A Windows Phone,
HiDef, LZX-requested build reaches `ShouldCompressContent` and both ContentWriter target properties;
the writer declines compression and the actual XNB header carries the requested platform/profile
without the compression flag. Initialize supplies its compiler before Write, which resolves a
registered Vital writer and emits health through explicit typed and raw writer overloads; matching
game readers restore distinct75/76 values and later120/121 values after an authored rebuild.

A separate bounded writer follows the documented Initialize contract and retrieves a nested writer
there. Its AddTypeWriter call deadlocks because CNA invokes Initialize while holding the same
non-recursive mutex GetTypeWriter acquires. CNA-DEMOS-ISSUE-090 records the defect; the one-second
worker watchdog leaves the failure visible without hanging CTest. Both renderer selections pass
12/13 with matching normalized output and diagnostics, and the expected-failure SOFTWARE CTest
passes. The warning-free incremental build uses at most three jobs; metadata remains636/0,
known-issue demos increase41->42, validator/checkpoint self-tests pass5/5 and4/4, and generated
runtime coverage remains302/329 types,2114/2326 non-field members,169/753 fields and2999/3205 C
routes. Evidence:`/tmp/checkpoint-773-xnb-writer-*` and `/tmp/checkpoint-773-coverage.md`. No
rendering, screenshot or manual inspection was used. Continue the same real XNB graph with shared
resources, external references and dynamic reference dispatch.

### Focused774 custom XNB reference graph

The same original XML profile now authors a RareBadge through a base Badge field and an absolute
portrait XNB path. ProfileWriter sends the badge once through base-typed WriteObject and twice
through WriteSharedResource, then writes the portrait through WriteExternalReference. The runtime
readers prove dynamic derived dispatch for both materializations, full Founder/level7/glow11 data,
an inline identity independent from the shared table, exact alias identity for the two deferred
fixups and the normalized extensionless `portraits/sentinel` path. The rebuilt profile retains the
shared alias while exposing its edited health through a fresh root object.

SOFTWARE and OPENGLES3/Xvfb each pass15/16 controls and fail only established ISSUE-090; stdout and
the intentional compiler/debug diagnostics match after normalizing the private temporary path. The
exact expected-failure SOFTWARE CTest passes. The warning-free incremental build uses at most three
jobs; metadata remains636/0, known-issue demos remain42, validator/checkpoint self-tests pass5/5
and4/4, and generated runtime coverage remains302/329 types,2114/2326 non-field members,169/753
fields and2999/3205 C routes. Evidence:`/tmp/checkpoint-774-xnb-graph-*` and
`/tmp/checkpoint-774-coverage.md`. No rendering, screenshot or manual inspection was used. Audit
the remaining custom writer null/refusal contracts and ContentCompiler inspection/freeze behavior
against existing demos before adding more controls.

### Focused775 custom XNB null references

The edited original profile now replaces its derived badge with an explicit XML null and its
portrait with an empty string. The same ProfileWriter sends that state through WriteObject, two
WriteSharedResource calls and WriteExternalReference. After cache-preserving publication and an
explicit Unload, the fresh runtime profile has no inline badge, neither shared fixup ran, and its
external path is empty, while the edited health plus typed/raw values remain exact.

SOFTWARE and OPENGLES3/Xvfb each pass16/17 controls and fail only established ISSUE-090; stdout and
the intentional compiler/debug diagnostics match after normalizing the private temporary path. The
exact expected-failure SOFTWARE CTest passes. The warning-free incremental build uses at most three
jobs; metadata remains636/0, known-issue demos remain42, validator/checkpoint self-tests pass5/5
and4/4, and generated runtime coverage remains302/329 types,2114/2326 non-field members,169/753
fields and2999/3205 C routes. Evidence:`/tmp/checkpoint-775-xnb-null-*` and
`/tmp/checkpoint-775-coverage.md`. No rendering, screenshot or manual inspection was used. Audit
raw-null and missing/foreign writer refusals next, then disposition ContentCompiler inspection and
freeze members against the existing registry/compiler workflows.

### Focused776 custom XNB writer refusals

Three focused public ContentCompiler::Compile probes now exercise writer failures after the main
source-pipeline workflow. WriteRawObject rejects a null reference because a raw payload has no type
marker. Base-typed WriteObject refuses a RareBadgeSource when only the base Badge writer is
registered, naming both dynamic and declared types. The explicit-writer overload refuses a valid
Vital writer obtained from another compiler before it emits the value. Each probe keeps the public
ContentWriter call visible and checks its actionable PipelineException diagnostic.

SOFTWARE and OPENGLES3/Xvfb each pass19/20 controls and fail only established ISSUE-090; stdout and
the intentional compiler/debug diagnostics match after normalizing the private temporary path. The
exact expected-failure SOFTWARE CTest passes. The warning-free incremental build uses at most three
jobs; metadata remains636/0, known-issue demos remain42, validator/checkpoint self-tests pass5/5
and4/4, and generated runtime coverage remains302/329 types,2114/2326 non-field members,169/753
fields and2999/3205 C routes. Evidence:`/tmp/checkpoint-776-xnb-refusals-*` and
`/tmp/checkpoint-776-coverage.md`. No rendering, screenshot or manual inspection was used. Audit
ContentCompiler inventory/freeze behavior next, then disposition implementation-facing registry
and active-writer helpers.

### Focused777 XNA ContentCompiler inventory and freeze

The compiler retained from the first real XML build reports a sorted KnownTypeNames inventory that
contains a built-in integer plus the Vital, Badge, RareBadge and Profile source writers. Because it
has produced the XNB, a later distinct AddTypeWriter is refused with the frozen diagnostic; its
read-only GetTypeWriter still returns the exact Vital writer, while an unknown probe type is refused.
This is the XNA-shaped ContentCompiler lifecycle and is separate from the native pipeline registry
freeze already covered by `incremental_compiler`.

The remaining public TypeWriterRegistry, FindWorker variants and ActiveWriter methods connect the
façade to CNA's canonical XNB implementation; ContentWriter::Output exposes that same bridge.
Flush/Close intentionally do nothing for the buffered writer, and repeating every primitive Write
overload belongs to CNA's byte-level tests. COVERAGE_AUDIT now dispositions these rather than adding
identity/no-op controls.

SOFTWARE and OPENGLES3/Xvfb each pass21/22 controls and fail only established ISSUE-090; stdout and
the intentional compiler/debug diagnostics match after normalizing the private temporary path. The
exact expected-failure SOFTWARE CTest passes. The warning-free incremental build uses at most three
jobs; metadata remains636/0, known-issue demos remain42, validator/checkpoint self-tests pass5/5
and4/4, and generated runtime coverage remains302/329 types,2114/2326 non-field members,169/753
fields and2999/3205 C routes. Evidence:`/tmp/checkpoint-777-xnb-compiler-*` and
`/tmp/checkpoint-777-coverage.md`. No rendering, screenshot or manual inspection was used. Close
the custom writer audit by checking any remaining meaningful ContentTypeWriter attributes or
compression/default behavior, then move to the next public-member family.

### Focused778-custom-XNB-compression-default

`content.custom.xml_xnb_pipeline` now contrasts ProfileWriter's established compression refusal
with a fresh VitalWriter that inherits the default `ShouldCompressContent` policy. Direct public
`ContentCompiler::Compile` receives `compressContent=true`; the emitted XNB carries the compression
flag and `ContentManager` restores the exact health91 payload. `ContentTypeWriterAttribute` is also
named explicitly in metadata and dispositioned as CNA's empty C++ registration marker rather than
inflated into a vacuous property check.

SOFTWARE and OPENGLES3/Xvfb each pass22/23 controls and fail only established ISSUE-090; stdout and
the intentional compiler/debug diagnostics match after normalizing the private temporary path. The
exact expected-failure SOFTWARE CTest passes. The warning-free incremental build uses at most three
jobs; metadata remains636/0, known-issue demos remain42, validator/checkpoint self-tests pass5/5
and4/4, and generated runtime coverage remains302/329 types,2114/2326 non-field members,169/753
fields and2999/3205 C routes. Evidence:`/tmp/checkpoint-778-xnb-compression-*` and
`/tmp/checkpoint-778-coverage.md`. No rendering, screenshot or manual inspection was used. The
custom writer audit is closed; extend the existing MediaPlayer queue workflow with a nonzero
collection start and collection replacement next.

### Focused779-MediaPlayer-collection-replacement

`media.media_player.queue_and_repeat` retains its nine prior controls and adds three. Its first
playlist now uses the plain `Play(SongCollection)` overload and proves the documented zero start.
While that queue is live, a second collection with distinct names uses the indexed overload at
entry one: the active queue is replaced rather than appended, playback starts on Alternate Rising,
and both ordered queue copies remain valid after the caller's collection and Songs are destroyed.

SOFTWARE and OPENGLES3/Xvfb direct runs pass12/12 controls with byte-identical stdout and empty
stderr; the focused SOFTWARE CTest passes. The warning-free incremental build uses at most three
jobs; metadata remains636/0, validator/checkpoint self-tests pass5/5 and4/4, and generated runtime
coverage remains302/329 types,2114/2326 non-field members,169/753 fields and2999/3205 C routes.
Evidence:`/tmp/checkpoint-779-media-queue-*` and `/tmp/checkpoint-779-coverage.md`. No acoustic
quality, screenshot or manual inspection is claimed. Exercise the shuffle branch with a bounded
sequence that proves queue membership/index consistency without asserting a random order next.

### Focused780-MediaPlayer-shuffled-traversal

The same `media.media_player.queue_and_repeat` retains all twelve prior controls and replaces its
shuffle flag-only claim with one behavior control. Sixteen actual shuffled `MoveNext` calls each
retain an in-range index, make `ActiveSong` equal the indexed queue member, select one of the two
known names and return to Playing. Repeating an index is accepted because the source's uniform
selection permits it; the demo does not make an unstable ordering or distribution assertion.

SOFTWARE and OPENGLES3/Xvfb direct runs pass13/13 controls with byte-identical stdout and empty
stderr; the focused SOFTWARE CTest passes. The warning-free incremental build uses at most three
jobs; metadata remains636/0, validator/checkpoint selftests pass5/5 and4/4, and generated runtime
coverage remains302/329 types,2114/2326 non-field members,169/753 fields and2999/3205 C routes.
Evidence:`/tmp/checkpoint-780-media-shuffle-*` and `/tmp/checkpoint-780-coverage.md`. No acoustic
quality, randomness quality, screenshot or manual inspection is claimed. Extend the existing
transport workflow with bounded paused-position stability, Resume progress and Stop reset next.

Historical audit detail follows. Focused711 projected the632-demo staged catalog to2102/2326 XNA
members and157/753 fields at301/329 types.
Effect, EffectParameter, EffectAnnotation, EffectPass and EffectTechnique have no remaining
generated gaps; their collection enumerators are dispositioned CLR/C# machinery. GraphicsDevice's
ordinary members are complete; DeviceLost is desktop/platform-specific and Finalize is CLR
machinery. The stock-effect families now retain only dispositioned protected OnApply hooks and one
SkinnedEffect explicit-interface inventory artifact; real pass application reaches the hooks and
focused671 proved the override dispatch contract. RenderTargetCube and VertexBufferBinding now have
no member gap. GraphicsAdapter, the complete PackedVector namespace, PacketReader, all Design
converter types and the ordinary Content surface now have no member gap. The seven remaining Content
entries are four nominal .NET attribute-wrapper types and are dispositioned rather than inflated
into inert wrapper demos. Input now has no member gap. Music-library object, collection and library
lifetime and VisualizationData construction are covered. Media's remaining14 entries are the seven
collections' generic/non-generic CLR GetEnumerator pairs, dispositioned rather than credited from
C++ range syntax. Input.Touch now likewise retains only six CLR enumerator artifacts. The ordinary
Graphics audit is complete; its remaining56 entries are already dispositioned finalizers, protected
OnApply hooks, explicit-interface aliases, collection enumerators, environment-only exception paths,
EffectMaterial construction, DeviceLost and ModelMesh.Tag. Storage's three gaps are also dispositioned
as CLR Finalize, an event with no CNA production raiser and the removed-device exception path.
AvailableNetworkSession public/private capacity and sparse advertised properties are now covered,
as is shared search-collection disposal and copied-listing refusal. Remote NetworkGamer
departure/history/session/round-trip behavior is also covered by the same real two-process leave.
The simulated-conditions packet workflow now samples positive sent/received session bandwidth, and
game_flow covers successful lobby ResetReady plus Playing-state refusal. System Link discovery/join
now executes both Begin/End pairs and discovery measures both QoS bandwidth directions.
NetworkSession disposal state, post-dispose roster safety, the fixed-shape C++
NetworkSessionProperties collection contract and the offline LocalWithLeaderboards creation path
are now covered. The
remaining event-payload constructors, voice, invites, leaderboard callbacks and CLR serialization
members are dispositioned in COVERAGE_AUDIT. The existing leaderboard paging workflow now covers
offline Begin/End read and navigation, identity and reader disposal. Continue ordinary
GamerServices members after completing PropertyDictionary's actual C++ collection behavior; its
typed values, lookup, Item, snapshot, copy and mutation surfaces are now covered, and ISSUE-085
records the broken offline stream column. Its remaining eight inventory entries are CLR aliases
and enumerators and stay intentionally uncredited. Gamer/GamerProfile identity and lifetime are now
covered. AchievementCollection's populated indexing/lifetime and actual C++ collection surface are
now covered; its remaining inventory names are CLR aliases. The same real workflow now covers
SignedInGamer Begin/End award and achievement-read pairs. Remaining friend, party/headset and
online-identity paths require service/hardware evidence. AvatarAnimation and AvatarRenderer sampled
values, lighting and lifetime are now covered. AvatarDescription's local-profile Begin/End retrieval
is covered; Changed requires a service revision or manual avatar-editor save through public APIs and
remains dispositioned. Achievement::GetPicture now covers real catalog bytes, fresh-stream ownership
and the unavailable path. GamerServicesComponent construction, Initialize/Update and dispatcher
Initialize/WindowHandle are now behaviorally covered; InstallingTitleUpdate is never raised by
CNA's production path. Guide screen-saver state is now covered through the platform; notification
delay and social overlays remain service/manual work. Cue's synchronous creation/preparation-state
audit is now complete. The remaining Microphone collection/duration/headset members are now covered
in the existing hardware-manual workflow against SDL's dummy recording device, with physical input
still manual. Consolidate the remaining Audio, Media, Net and platform-dependent
member dispositions next, then continue the next high-value behavior family after each batch,
rebuilding and running both renderers where applicable, updating coverage/audit/ROADMAP/HANDOFF and
pushing each completed batch.

The public C audit is at2999/3205 with all remaining routes dispositioned as fixed reflection names,
hardware/service/manual observations or renderer-specific work. Do not create controls merely to
raise that count. The core Model inventory is also dispositioned: CLR collection enumerators and
the arbitrary `ModelMesh::Tag` carrier do not justify new demos. Preserve external state and do not
claim real hardware, service, manual or visual behavior without executing it. Never edit or revert
the sibling CNA checkout. It is currently clean at external `0c83dcc9dc`; recheck the moving
checkout before every build and do not absorb or revert external work.

HISTORICAL FOCUSED NOTES (all built and absorbed by complete634):
`standalone/c_api.graphics.vertex_values` contains the planned seven-check
workflow and compiles cleanly with strict C99 warnings against the last completed C API binary. Its
direct, SOFTWARE-environment and OPENGLES3-environment runs are byte-identical passes with empty
stderr. `standalone/c_api.graphics.vertex_buffer_data` is also staged: its36 checks coherently cover
the15 meaningful previously uncredited `vertex_resources.h` routes through copied empty/inferred/
explicit declarations, retained buffer metadata, typed caller windows, raw buffer windows,
dynamic streaming options, a public binding descriptor and ContentLost registration ownership.
Its strict direct build and one-frame SOFTWARE and OPENGLES3 runs pass against the last completed
libraries; the four fixed declaration/buffer type-name routes remain intentionally trivial and
uncredited.

The next selected audio batch is also staged as `standalone/c_api.audio.sound_effect_sources`.
Its22 controls generate a100ms mono WAV, prove exact sample-size conversion, create owned effects
from a looped PCM subrange, encoded memory and a private file, then verify input/file independence,
exact atomic name copies, live disposal state, restored process master volume and both one-shot
Play forms. Strict compilation plus SOFTWARE/OPENGLES3 direct runs pass with byte-identical output
and empty stderr. It does not claim audible content or output quality. Continue the later audio
audit as separate instance-control, 3D, streaming and microphone-availability workflows.

`standalone/c_api.audio.sound_effect_instance_state` is the next staged workflow. Its19 controls
cover canonical defaults, valid/invalid mixing properties, Play/Pause/Resume, rejected loop
mutation after start, authored release to bounded natural completion, replay and immediate Stop.
Five repeated SOFTWARE runs and the OPENGLES3 direct run are byte-identical passes with empty
stderr. The first draft expected the C header's stated pitch clamping; source and runtime instead
establish canonical range rejection. The passing documentation mismatch is recorded as
CNA-DEMOS-ISSUE-072 and the demo follows actual CNA behavior without a runtime knownIssue marker.

`standalone/c_api.audio.spatial_state` now compiles strictly and has identical direct SOFTWARE and
OPENGLES3 output with empty stderr. Twenty-one of22 controls pass; the sole expected failure is
CNA-DEMOS-ISSUE-073: setting direct pan on a playing 3D instance returns INVALID_STATE but changes
the publicly reported pan because CNA assigns `Pan_` before validating the latched 3D state. The
demo restores all three process spatial settings and releases every child despite that assertion.
Evidence:`checkpoint-530-c-spatial-state-focused-*-direct.*`.

The existing `c_api.media.media_player` adds four controls for the last two `media_player.h`
functions: exact size, zero-capacity sizing, atomic short refusal and exact unterminated MediaQueue
type-name copy. Both direct selections pass all98 controls with empty stderr and identical check
labels; only the existing printed live position differs (0.56s SOFTWARE/0.53s OPENGLES3).
`media_player.h` is now credited41/41. Evidence:
`checkpoint-531-c-media-queue-type-name-focused-*-direct.*`.

`standalone/c_api.audio.dynamic_streaming` passes21 byte-identical controls under both direct
selections with empty stderr. It covers all11 meaningful streaming routes: exact instance-based
sample conversion, copied/aligned PCM windows, float submission, live format-transition refusal,
queue ownership/clear, explicit update, three BufferNeeded requests and one-shot unsubscribe. The
two fixed dynamic-instance type-name routes remain trivial/uncredited. Evidence:
`checkpoint-532-c-dynamic-streaming-focused-*-direct.*`.

`standalone/c_api.audio.microphone_availability` passes20 byte-identical direct controls with empty
stderr against SDL dummy's one capture device. It covers count/default, exact and atomic-short name
copies, captured/restored100ms duration, stopped/started/stopped state, negotiated sample rate,
exact duration/size conversion, a real BufferReady followed by a bounded nonempty GetData, one-shot
unsubscribe, the dispatcher pump and every index-at-Count refusal. This is backend API behavior,
not a physical-microphone, content-quality or latency claim. The microphone type-name pair remains
trivial/uncredited. Together the staged audio workflows establish66/74 `audio.h` routes; its eight
remaining routes are the four fixed type-name size/copy pairs. Evidence:
`checkpoint-533-c-microphone-availability-focused-*-direct.*`.

`standalone/c_api.graphics.packed_vectors` passes nine byte-identical controls under both direct
renderer selections with empty stderr. It covers all seven `packed_vectors.h` routes across all17
formats, exact storage bits and canonical expansion, width-aware equality, atomic above-width and
nonfinite refusals, ignored unused components, reduced precision and IEEE binary16 special values.
The workflow creates no game or graphics device. Evidence:
`checkpoint-534-c-packed-vectors-focused-*-direct.*`.

The existing `c_api.input.touch_state` now adds the ten pure value-construction/text routes and the
live touch-device existence getter without another ID. The final direct workflow passes72
byte-identical controls under SOFTWARE and OPENGLES3 with empty stderr. It checks complete gesture,
capability, location and empty-state defaults; authored combined flags and prior locations; exact
unterminated and atomic-short location text; and false/true live device state around the existing
public fixture. Current-source metadata credits43/47 `input_touch.h` routes. The remaining four are
the two manual pointer-emulation routes and two platform window-handle routes. Evidence:
`checkpoint-535-c-touch-values-focused-*-direct.*` and
`checkpoint-536-c-touch-device-exists-focused-*-direct.*`.

`standalone/c_api.math.color_values` closes all24 `color.h` routes in one renderer-independent
15-control workflow. It covers every constructor family, exact packed/vector/text representations,
value identity/hash, named constants, interpolation, scaling, both premultiplication families and
native vector packing including NaN/infinity. Both direct selections are byte-identical with empty
stderr; strict C99 and metadata compilation pass. No game or graphics device is created. Evidence:
`checkpoint-537-c-color-values-focused-*-direct.*`.

`standalone/c_api.math.quaternion_values` closes all28 `quaternion.h` routes in one renderer-
independent15-control workflow. It covers construction, identity versus the zero value, equality,
hashing, length, returned/in-place conjugation and normalization, exact/atomic text, component
arithmetic, three equivalent rotation constructors, multiplication versus concatenation order,
inverse/division, dot and distinct Lerp/Slerp paths. Both direct selections are byte-identical with
empty stderr; strict C99 and metadata compilation pass. Evidence:
`checkpoint-538-c-quaternion-values-focused-*-direct.*`.

`standalone/c_api.math.scalar_helpers` demonstrates all15 scalar-helper routes from `math.h` in an
11-control renderer-independent workflow. It covers constants, affine and curved interpolation,
clamped SmoothStep, float/integer bounds, NaN propagation, distance/min/max, angle conversion and
wrapping, the strict framework-epsilon boundary and bounded MSAA power selection with atomic
negative-input refusal. Both direct selections are byte-identical with empty stderr; strict C99
and metadata compilation pass. The first draft's `1e-5` degree round-trip tolerance was a demo bug
and was corrected to the existing public example's `1e-3`; CNA behavior was consistent. Evidence:
`checkpoint-539-c-scalar-helpers-focused-*-direct.*`.

`standalone/c_api.math.point_values` demonstrates all12 Point routes from `math.h` in seven
renderer-independent controls: default/Zero and authored construction, equality/hash, exact and
atomic text, ordinary component arithmetic, defined two's-complement wraparound, and atomic zero-
divisor/overflow refusal. Both direct selections are byte-identical with empty stderr; strict C99
and metadata compilation pass. Evidence:`checkpoint-540-c-point-values-focused-*-direct.*`.

`standalone/c_api.math.rectangle_values` demonstrates all25 Rectangle routes in12 renderer-
independent controls: empty/authored construction, derived edges/location/center, wraparound,
half-open containment, sequential mutation, identity/hash, exact/atomic text, strict positive-area
overlap, intersection and union. Both direct selections are byte-identical with empty stderr;
strict C99 and metadata compilation pass. Together checkpoints539-541 close `math.h` at52/52.
Evidence:`checkpoint-541-c-rectangle-values-focused-*-direct.*`.

`standalone/c_api.math.vector2_values` demonstrates all41 Vector2 routes in15 renderer-independent
controls. It covers construction/constants, identity/hash, length and both normalization forms,
exact/atomic text, component/scalar arithmetic and IEEE zero division, metrics, interpolation,
bounds/reflection, distinct point/normal/quaternion transforms, three bounded array forms and an
atomic invalid window. Both direct selections are byte-identical with empty stderr; strict C99 and
metadata compilation pass. Evidence:`checkpoint-542-c-vector2-values-focused-*-direct.*`.

`standalone/c_api.math.vector3_values` demonstrates all50 Vector3 routes in17 renderer-independent
controls. In addition to the Vector2 families it establishes the three unit axes, six named XNA
directions with negative-Z Forward, zero normalization, cross product, 3D translation/normal/
quaternion transforms and all three bounded array forms. Both direct selections are byte-identical
with empty stderr; strict C99 and metadata compilation pass. Evidence:
`checkpoint-543-c-vector3-values-focused-*-direct.*`.

`standalone/c_api.math.vector4_values` demonstrates all46 Vector4 routes in17 renderer-independent
controls. It adds Vector2/3 extension constructors, four unit axes, zero normalization and exact
homogeneous transform behavior: Vector2/3 inputs receive W one, authored Vector4 W scales matrix
translation, and quaternion transforms preserve W. Bounded matrix/quaternion arrays and atomic
invalid windows are explicit. Both direct selections are byte-identical with empty stderr; strict
C99 and metadata compilation pass. Checkpoints542-544 close `vectors.h` at137/137. Evidence:
`checkpoint-544-c-vector4-values-focused-*-direct.*`.

`standalone/c_api.math.matrix_values` demonstrates all35 non-factory Matrix routes in18 renderer-
independent controls: zero/Identity construction, all16 fields, six signed direction properties,
translation, determinant, equality/hash, successful and singular decomposition, exact/atomic text,
component arithmetic, row-vector composition order, transpose, inverse and quaternion transform.
Both direct selections are byte-identical with empty stderr; strict C99 and metadata compilation
pass. Evidence:`checkpoint-545-c-matrix-values-focused-*-direct.*`.

`standalone/c_api.math.matrix_factories` demonstrates all22 factory routes in13 controls. It covers
ordinary and coincident/fallback billboards, constrained orientation, equivalent and direct
rotation factories, paired world/view frames, centered/off-center projections with atomic invalid
ranges, scaling, offset-plane shadow/reflection and translations. Both direct selections are byte-
identical with empty stderr; strict C99 and metadata compilation pass. Checkpoints545-546 close
`matrix.h` at57/57. Evidence:`checkpoint-546-c-matrix-factories-focused-*-direct.*`.

`standalone/c_api.math.plane_values` demonstrates all20 Plane routes from `geometry.h` in nine
renderer-independent controls. Five constructors, the three dot forms, identity/hash, both
normalization paths, exact/atomic text, box/sphere/frustum classification, matrix translation and
quaternion rotation pass in both direct selections with byte-identical output and empty stderr;
strict C99 and metadata compilation pass. Evidence:
`checkpoint-547-c-plane-values-focused-*-direct.*`.

`standalone/c_api.math.ray_values` demonstrates all11 Ray routes from `geometry.h` in seven
renderer-independent controls. Construction, identity/hash, box/sphere/plane/frustum distances and
exact/atomic legacy text pass in both direct selections with byte-identical output and empty
stderr; strict C99 and metadata compilation pass. Evidence:
`checkpoint-548-c-ray-values-focused-*-direct.*`.

`standalone/c_api.math.bounding_box_values` demonstrates all20 BoundingBox routes from
`geometry.h` in11 renderer-independent controls. Construction, all containment classes, atomic XNA-
ordered corners, ray/frustum/sphere/box/plane intersections, identity/hash, exact/atomic text,
three enclosing factories and empty-point refusal pass in both direct selections with byte-
identical output and empty stderr; strict C99 and metadata compilation pass. Evidence:
`checkpoint-549-c-bounding-box-values-focused-*-direct.*`.

`standalone/c_api.math.bounding_sphere_values` demonstrates all21 BoundingSphere routes from
`geometry.h` in12 renderer-independent controls. Construction, transform radius, all containment
classes, identity/hash, four enclosing factories, atomic empty-input refusal, five intersection
forms and exact/atomic text pass in both direct selections with byte-identical output and empty
stderr; strict C99 and metadata compilation pass. The first draft's exact-centered frustum sphere
assumption exceeded the documented enclosure contract and was corrected; this was a demo defect.
Evidence:`checkpoint-550-c-bounding-sphere-values-focused-*-direct.*`.

`standalone/c_api.math.bounding_frustum_values` demonstrates all22 BoundingFrustum routes from
`geometry.h` in13 renderer-independent controls. The retained matrix, six derived planes, all
containment classes, atomic ordered corners, volume/plane/ray intersections including the
documented boundary-origin refusal, identity/hash and exact/atomic text pass in both direct
selections with byte-identical output and empty stderr; strict C99 and metadata compilation pass.
Checkpoints547-551 close `geometry.h` at94/94. Evidence:
`checkpoint-551-c-bounding-frustum-values-focused-*-direct.*`.

`standalone/c_api.math.curve_key_values` demonstrates all17 `cna_curve_key_*` routes in nine
renderer-independent controls. It fixes the five-field record layout with a C99-compatible
alignment probe; checks all constructor defaults and authored fields, getters/setters, atomic
continuity refusal, clone independence, position-only ordering, equality and hash consistency.
Both direct selections are byte-identical with empty stderr, and strict C99 plus metadata
compilation pass. The first draft used C11 `_Alignof`; replacing it was a demo portability fix,
not a CNA defect. Evidence:`checkpoint-552-c-curve-key-values-focused-*-direct.*`.

`standalone/c_api.math.curve_key_collection` demonstrates all14 owned collection routes in11
renderer-independent controls. It covers empty mutable construction, out-of-order insertion and
sorted access, full-key membership/indexing, reorder after replacement, atomic short and exact
indexed copies, invalid-key atomicity, deep-clone independence, clear, both removal forms, invalid
indexes and stale-handle refusal. Both direct selections are byte-identical with empty stderr, and
strict C99 plus metadata compilation pass. Together checkpoints552-553 put `curve.h` at31/45.
Evidence:`checkpoint-553-c-curve-key-collection-focused-*-direct.*`.

`standalone/c_api.math.curve_evaluation` demonstrates all14 curve/evaluation routes in15
renderer-independent controls. It covers empty/single/multi-key constant state, a mutable retained
key view, interpolation, all four whole-curve/per-key tangent forms, every loop mode, property
readback, atomic invalid requests, independent cloning and view retention after the public curve
handle is destroyed. Both direct selections are byte-identical with empty stderr, and strict C99
plus metadata compilation pass. Equal authored endpoint slopes exercise ordinary Linear looping;
the distinct-slope and Step compatibility defects remain explicitly demonstrated by the existing
ISSUE-036 and ISSUE-002 C++ demos. Checkpoints552-554 close `curve.h` at45/45. Evidence:
`checkpoint-554-c-curve-evaluation-focused-*-direct.*`.

`standalone/c_api.core.logger` demonstrates all19 logger routes from `core_ext.h` in10 controls.
It captures all seven severity-specific and conditional forms, the general condition/category
route, exact formatted borrowed text, minimum-level filtering, invalid identities/booleans/views,
and restores the process-global sink and prior minimum. Both direct selections are byte-identical
with empty stderr, and strict C99 plus metadata compilation pass. This establishes `core_ext.h`
at19/57 without relying on logger calls hidden in the common demo scaffold. Evidence:
`checkpoint-555-c-core-logger-focused-*-direct.*`.

`standalone/c_api.platform.host_identity` demonstrates the eight platform, desktop-OS and assembly-
title routes from `core_ext.h` in10 controls. It correlates portable target/Apple/mobile/desktop
facts, copies the exact unterminated platform name with atomic short refusal, owns/restores the
process title and proves exact/atomic title copies. Embedded NUL returns the shared text contract's
`ENCODING` and preserves the title; the route-specific header omits that result, recorded as passing
documentation ISSUE-074. Both direct selections are byte-identical with empty stderr, and strict
C99 plus metadata compilation pass. Stable `core_ext.h` coverage is27/57; the remaining graphics-
backend/renderer-selection routes are deferred while that CNA subsystem changes externally.
Evidence:`checkpoint-556-c-platform-host-identity-focused-*-direct.*`.

`standalone/c_api.game.title_content` demonstrates the three previously uncredited TitleLocation/
TitleContainer routes alongside the existing path-copy route in11 controls. It moves the process
root from the executable directory to deployed assets, verifies exact/atomic path copies, sizes and
reads the complete256-byte checker PNG into caller storage, maps a missing file to IO, restores the
root and destroys the owner game. Both Xvfb direct selections are byte-identical with empty stderr;
strict C99 and metadata compilation pass. The first sandboxed attempt could not open an X display
and supplied no runtime evidence. Evidence:`checkpoint-557-c-title-content-focused-*-direct.*`.

`standalone/c_api.game.runtime_controls` demonstrates the final18 `runtime.h` routes in14 controls,
plus the two window-title accessors that verify its title setter. Timing/mouse/run-loop state,
derived FPS/milliseconds, an idle framework pump, ResetElapsedTime request, observable one-draw
Suppression across two explicit Ticks and independent Disposed registrations all pass. A first
draft wrongly expected game destruction to release the registration handle; the correct ownership
is detached-then-caller-release, and this was a demo defect. Both Xvfb direct selections are byte-
identical with empty stderr; strict C99 and metadata compilation pass. Checkpoints557-558 close
`runtime.h` at45/45 and establish `runtime_window.h` at2/20. No wall-clock pacing, focus event,
pixel or visual claim is made. Evidence:`checkpoint-558-c-runtime-controls-focused-*-direct.*`.

`standalone/c_api.game.window_state` demonstrates the remaining18 `runtime_window.h` routes in19
controls. It reads/restores resizing and border policies, validates bounds/orientation, exact/atomic
screen and reflection names, brackets a synchronous client-size change and its event, accepts the
documented platform refusal for state requests, and classifies the versioned borrowed native
descriptor without dereferencing it. Both Xvfb direct selections are byte-identical with empty
stderr; strict C99 and metadata compilation pass. Together with checkpoint558's title pair this
closes `runtime_window.h` at20/20. No appearance, manual interaction or borrowed-pointer lifetime
beyond game destruction is claimed. Evidence:`checkpoint-559-c-window-state-focused-*-direct.*`.

`standalone/c_api.game.component_members` demonstrates14 previously uncredited component-member
routes in23 controls: owner identity, property defaults/readback and change-only events, the
canonical inverted update-order comparison, direct initialize/update/draw behavior despite disabled
or invisible state, idempotent disposal with drawable unload, and one-shot event detachment. The
first draft compared update order using the separately changed draw-order value; both renderers
reported the same failure, and correcting the expectation from +/-10 to +/-4 was a demo defect.
`standalone/c_api.game.component_collection` demonstrates the remaining eight meaningful collection
and service routes in19 controls: membership/index/lookup agreement, insert and boundary refusal,
remove-at reuse, clear event counts, named service lookup and idempotent removal. The existing
`c_api.game.components` metadata now credits its already-semantic drawable classification and
callback-scoped graphics-device borrow. Both new Xvfb direct selections are byte-identical with
empty stderr; strict C99 and metadata compilation pass. `runtime_components.h` is35/39 demonstrated;
the four fixed component/collection reflection-name size/copy helpers are intentionally trivial and
already covered by CNA's pure-C contract test. Evidence:
`checkpoint-560-c-component-members-focused-*-direct.*` and
`checkpoint-561-c-component-collection-focused-*-direct.*`.

`standalone/c_api.game.graphics_preferences` demonstrates24 previously uncredited manager/value
routes in21 controls: versioned/nested device-information cloning, single-manager ownership, the
callback-scoped device borrow and every manager preference with round-trip or atomic-refusal
behavior. `standalone/c_api.game.graphics_manager_lifecycle` demonstrates nine more routes in24
controls: all event shapes, mutable prepared settings that affect the active device, ignored invalid
mutations, apply/recreate, an explicit draw bracket, idempotent disposal/service removal and one-
shot handle destruction. The first lifecycle draft incorrectly expected a mutable event write to
replace the manager's stored preference; corrected checks distinguish the per-preparation candidate
from the stored default. Both corrected Xvfb direct selections are byte-identical with empty stderr;
strict C99 and metadata compilation pass. `runtime_graphics_manager.h` is33/38 demonstrated. Four
fixed reflection-name size/copy routes are trivial/uncredited. `toggle_full_screen` is intentionally
platform/manual: under Xvfb both renderers returned success but SDL later logged a mode-switch
timeout and reverted, so that attempt is not credited as deterministic behavior and is not a CNA
issue. Evidence:`checkpoint-562-c-graphics-preferences-focused-*-direct.*` and
`checkpoint-563-c-graphics-manager-lifecycle-focused-*-direct.*`.

`standalone/c_api.content.manager_inspection` demonstrates17 previously uncredited content routes
in18 controls: exact/atomic resolved paths, normalized keys, idempotent built-in registration,
service/device association, unordered manifest traversal, native/XNB entry details and the sorted
reader-usage summary. `standalone/c_api.content.additional_loaders` demonstrates four more routes
in15 controls through a decoded WAV, a deterministic generated six-face DDS and a caller-owned CNJ
object whose bounded JSON, cache reuse, missing-type failure and post-Unload re-invocation are
observable. The initial cube check incorrectly expected the retained DXT1 storage identity; CNA
decodes this content path to Color, so that was a demo defect. An initial resource-manager check
also exposed that the old linked checkpoint library predates current CNA's placeholder behavior and
still fell back to loose files. It was removed rather than treated as current-source evidence;
`create_resource` retains the existing intentional unsupported/deferred disposition. Both corrected
Xvfb/dummy-audio selections are byte-identical with empty stderr; strict C99 and metadata compilation
pass. The generated `assets/textures/cube_faces.dds` reproduces byte-for-byte. `content.h` is31/32
demonstrated with all routes dispositioned. Evidence:
`checkpoint-564-c-content-manager-inspection-focused-*-direct.*` and
`checkpoint-565-c-content-additional-loaders-focused-*-direct.*`.

`standalone/c_api.storage.members` demonstrates25 previously uncredited storage routes in35
controls without a game. It covers exact/atomic root and container names, all selector forms and
synchronous completions, capacity, device-change registration ownership, container identity,
unordered directory/file listings, explicit share selection, flush, truncation/zero extension and
idempotent disposal with one event. File-share enforcement and actual device-change delivery are
not claimed: CNA currently ignores share bits, and no host device-set change was induced. The first
sandboxed runs failed to create the user storage root and are environment-only non-evidence; the
same binary passed directly in both renderer-labelled environments with byte-identical output and
empty stderr. Strict C99 and metadata compilation pass. `storage.h` is47/49 demonstrated; the two
fixed container reflection-name helpers are intentionally trivial/uncredited. Evidence:
`checkpoint-566-c-storage-members-focused-*-direct.*`.

`standalone/c_api.content.reader_values` demonstrates the first20 `content_readers.h` routes in33
controls without a game. A synthetic storage stream proves standalone reader identity, exact/atomic
asset-name copying, version/platform preservation, the empty type/shared-resource protocol, a null
object tag, all seven public value readers, collection and decoded-size validation, exact-byte
capacity/truncation behavior, the reader's exclusive stream borrow and one-shot handle release.
Both renderer-labelled direct runs are byte-identical with empty stderr; neither renderer is
initialized. Strict C99 and metadata compilation pass. `content_readers.h` is20/62 demonstrated;
its registry, placeholder, caller-supplied, reflective and object-dictionary routes remain for the
next batches. Evidence: `checkpoint-567-c-content-reader-values-focused-*-direct.*`.

`standalone/c_api.content.type_reader_registry` demonstrates the next15 `content_readers.h` routes
in37 controls. Its isolated process registry covers clearing, the idempotent currently empty known-
unsupported hook, missing-name resolution, explicit placeholder identity/refusal, exact/atomic
target names, versions and initialization. A caller-owned factory then proves malformed and
duplicate registration refusal, fresh per-lookup instances, callback-scoped reader usability and
post-callback invalidation, read-failure propagation, instance survival after unregistration,
one-shot cleanup and immediate name reuse. Both renderer-labelled direct runs are byte-identical
with empty stderr; no renderer is initialized. Strict C99 and metadata compilation pass.
`content_readers.h` is35/62 demonstrated. Evidence:
`checkpoint-568-c-content-type-reader-registry-focused-*-direct.*`.

`standalone/c_api.content.reflective_reader` demonstrates the next11 `content_readers.h` routes in
40 controls. Exact/atomic reflective and enum canonical names feed an isolated registry. The value-
shaped builder rejects malformed declarations, snapshots its first idempotent registration and
maps all14 public `CNA_ContentFieldKind` identities, an inline enum and a caller callback into
declared C offsets. The callback's borrowed reader works only during the call. Destroying the
builder leaves its registrations alive. A distinct shared builder then produces a reference-shaped
carrier around a second caller-created object. Both renderer-labelled direct runs are byte-
identical with empty stderr and initialize no renderer. Strict C99 and metadata compilation pass.
`content_readers.h` is46/62 demonstrated. Evidence:
`checkpoint-569-c-reflective-reader-focused-*-direct.*`.

`standalone/c_api.content.object_dictionary` closes the final16 `content_readers.h` routes in47
controls. Its deterministic generated XNB has16 sorted entries covering every public fixed scalar
kind, string and foreign-object access, a packed Vector3 array and an intentionally unknown UInt32
whose diagnostic C++ name remains inspectable. The demo proves logical runtime naming, indexed
keys, entry descriptors, wrong-kind and short-buffer atomicity, cached-load reader reuse,
independent owned handles and stale-handle rejection. The initial run found only that the staged
asset had not yet been copied into the old build tree; after an exact deployment copy both Xvfb/
dummy-audio runs passed byte-identically with empty stderr. This was a demo deployment prerequisite,
not a CNA defect. Strict C99, metadata compilation and deterministic asset regeneration pass.
`content_readers.h` is62/62 demonstrated. Evidence:
`checkpoint-570-c-object-dictionary-focused-*-direct.*`.

`standalone/c_api.media.video_asset` demonstrates the first19 `video.h` routes in26 controls. It
resolves and probes the generated160x120/15fps WMV, correlates individual and aggregate metadata,
checks positive duration and device association, exact/atomic path and reflection text, and proves
versioned-structure refusal. A metadata-only missing-file Video preserves all declared values,
accepts loader duration correction and defers track-index validation. The URI extension accepts a
plain path but treats a scheme as literal path text, and all owned handles release independently.
Both Xvfb/dummy-audio runs pass byte-identically with empty stderr; no playback, frame or audible/
visual claim is made. Strict C99 and metadata compilation pass. `video.h` is19/42 demonstrated.
Evidence: `checkpoint-571-c-video-asset-focused-*-direct.*`.

`standalone/c_api.media.video_player` closes the remaining23 `video.h` routes in31 controls. A real
decode proves fresh state/configuration, volume clamping, exact/atomic reflection text, active Video
identity, the versioned frame descriptor and the one-call lifetime of borrowed texture handles.
The paced run holds a pause across multiple updates, resumes and observes the authored three-second
clip wrap while the player remains Playing. Stop clears its public state, replay advances the
lifetime generation strictly, and the caller-owned Video plus disposed player each release once.
Both Xvfb/dummy-audio selections pass byte-identically with empty stderr. Strict C99 and metadata
compilation pass. `video.h` is42/42 demonstrated. No pixel correctness, audible quality or visual
inspection is claimed. Evidence: `checkpoint-572-c-video-player-focused-*-direct.*`.

`standalone/c_api.audio.xact_engine` starts the C XACT audit with17 `xact.h` routes in17 controls.
Both engine constructors open the generated version46 settings, including the accepted renderer and
look-ahead form. The single backend's friendly name, id, text identity, bounds, equality and stable
process hash are visible through exact/atomic buffers. The authored SpeedOfSound global changes,
clamps at its maximum and remains distinct from the cue-scoped Intensity variable. Update succeeds,
and an owned subscription observes exactly one synchronous disposal before releasing separately.
Both Xvfb/dummy-audio selections pass byte-identically with empty stderr; strict C99 and metadata
compilation pass. No speaker output or look-ahead scheduling effect is claimed. Evidence:
`checkpoint-573-c-xact-engine-focused-*-direct.*`.

`standalone/c_api.audio.xact_banks` adds28 `xact.h` routes in35 controls. It contrasts missing and
resident wave/sound banks with an actual prepared streaming bank, then observes initial and live use
state. Independent Music and SFX category values prove exact/atomic names, equality/hashes, finite
volume validation and selective Pause/Resume/Stop through two playing cues; paused use remains live.
Fire-and-forget and positioned fire-and-forget playback are bank-owned and category Stop retires
them. Parent releases are refused while C children remain, and each bank disposal event fires once
before its detached registration releases. The first passing draft deliberately opened a missing
streaming path and produced the canonical audio error log; that redundant noisy check was removed
because the real streaming asset already demonstrates the constructor. Both corrected Xvfb/dummy-
audio runs pass byte-identically with empty stderr; strict C99 and metadata compilation pass.
`xact.h` is45/62 demonstrated. No category gain or spatial speaker output is measured. Evidence:
`checkpoint-574-c-xact-banks-focused-*-direct.*`.

`standalone/c_api.audio.xact_cue` closes the nine meaningful remaining `xact.h` routes in33
controls. Two independently prepared LowTone cues prove exact/atomic authored names, per-instance
Intensity state and range clamping. Apply3D publishes exact Distance, OrientationAngle and neutral
DopplerPitchScalar values before Play and updates Distance while paused. Explicit Pause/Resume and
Immediate Stop produce coherent versioned snapshots; a separate generated ReleaseTone distinguishes
AsAuthored's live Stopping phase from an immediate interruption. A caller-owned disposal
registration fires once and releases after its Cue. Both Xvfb/dummy-audio runs pass byte-identically
with empty stderr; strict C99 and metadata compilation pass. `xact.h` is54/62 demonstrated, and its
eight fixed type-name helpers are intentionally trivial/uncredited. No acoustic position, gain,
fade shape or audible-quality claim is made. Evidence:
`checkpoint-575-c-xact-cue-focused-*-direct.*`.

`standalone/c_api.net.session_properties` starts the C networking audit with22 `net.h` routes in20
controls and creates no game or socket. Default and one-sample QualityOfService structures preserve
availability, timing, zero throughput and version refusal. A non-join failure leaves the per-thread
join record absent with its canonical default identity. The mutable fixed eight-slot property list
then proves optional-value representation, bounds, IndexOf/Contains, every structural-mutation
refusal, exact shifted CopyTo with atomic short failure, complete ordered enumeration, reset/end
states and parent refusal while the enumerator child is live. Both renderer-labelled direct runs
pass byte-identically with empty stderr; strict C99 and metadata compilation pass. No real session,
join, throughput or renderer claim is made. Evidence:
`checkpoint-576-c-net-properties-focused-*-direct.*`.

`standalone/c_api.net.packet_buffers` closes the remaining29 `net.h` routes in20 controls without
creating a game, session or socket. A PacketWriter emits every public packet value into one ordered
132-byte payload, reports exact/atomic copies, then seeks and overwrites its Color without
truncating later data. An independent PacketReader copies the payload and restores the exact RGBA,
vector, quaternion, matrix and scalar values; its copies remain independent of position, seeking is
observable, exhaustion reports I/O, empty data resets it, and both owned handles reject a second
destruction. The first run incorrectly required an undersized compound Color read to preserve its
starting position; the header promises I/O exhaustion but no transactional position, so removing
that assertion corrects the demo rather than CNA. Both renderer-labelled direct runs pass
byte-identically with empty stderr; strict C99 and metadata compilation pass. `net.h` is51/51
demonstrated. No socket, wire transport, peer interoperability or network byte order is claimed.
Evidence: `checkpoint-577-c-net-packets-focused-*-direct.*`.

`standalone/c_api.net.gamer_machine_values` covers all26 non-event `net_gamers.h` routes in17
controls without a game, session or socket. A detached named NetworkGamer exposes all neutral role,
voice, ready, talking, identifier, round-trip and session defaults; the transport-owned departure,
host, identifier and timing values then mutate exactly, while noncanonical booleans and remote
ready publication are refused without changing state. A separately created empty NetworkMachine
proves count/bounds/removal refusal, copy-on-set and an independent copy-out handle with one-shot
ownership. Both renderer-labelled direct runs pass byte-identically with empty stderr; strict C99
and metadata compilation pass. Populated machine borrowing, local-gamer ready publication and
successful removal still require a live session and are not claimed. `net_gamers.h` is26/33
demonstrated. Evidence: `checkpoint-578-c-net-gamer-values-focused-*-direct.*`.

`standalone/c_api.net.session_event_values` closes the seven remaining `net_gamers.h` routes in15
controls. The payload-free game transition prefixes accept their canonical size/version and refuse
both failure shapes. Joined, left and host-changed descriptions retain exact live gamer handles
and refuse stale inputs atomically. All four session-end reasons round-trip with zero reserved
bytes; an unknown identity is refused before output mutation. Leaderboard-write payloads preserve
their optional gamer and canonical leaving flag, zero reserved storage and reject a noncanonical
boolean atomically. Both renderer-labelled direct runs pass byte-identically with empty stderr;
strict C99 and metadata compilation pass. `net_gamers.h` is33/33 demonstrated. These are payload
constructors; no live event dispatch is claimed. Evidence:
`checkpoint-579-c-net-event-values-focused-*-direct.*`.

`standalone/c_api.net.available_sessions` starts `net_sessions.h` with all21 synthetic discovery-
value and collection routes in23 controls. Three descriptions preserve exact roster/slot/type/port
metadata, exact/atomic host and address text, a measured round trip without invented throughput and
independent property copies. Equality deliberately ignores QoS/properties but includes connection
identity. A two-item collection snapshots its sources, rejects an invalid index atomically, remains
readable across idempotent disposal and yields an owned entry that outlives the collection. Both
renderer-labelled direct runs pass byte-identically with empty stderr; strict C99 and metadata
compilation pass. `net_sessions.h` is21/104 demonstrated. All descriptions are synthetic: no
discovery, join, socket or reachability is claimed. Evidence:
`checkpoint-580-c-net-available-focused-*-direct.*`.

`standalone/c_api.net.local_session_state` adds37 `net_sessions.h` routes in30 controls. An explicit
signed-in gamer and copied property list create one socket-free Local session. Its four rosters,
borrowed local/host/find-by-id views and owned count agree; borrowed children prevent early parent
release. Host/join policies, slot limits, simulated latency/loss, copy-mutate-replace properties,
exact/atomic type text, traffic defaults and process instance/action counts are explicit. Local
ready publication and host reset change aggregate readiness. StartGame remains queued until Update,
EndGame returns to Lobby, disposal is idempotent, post-dispose Update is refused and all ownership
unwinds to zero instances. Both renderer-labelled direct runs pass byte-identically with empty
stderr; strict C99 and metadata compilation pass. Two supporting signed-gamer routes are also newly
demonstrated, and `net_sessions.h` reaches58/104. No socket, remote machine, measured throughput or
cross-machine propagation is claimed. Evidence:
`checkpoint-581-c-net-local-state-focused-*-direct.*`.

`standalone/c_api.net.session_events` adds14 `net_sessions.h` routes in19 controls. Ten owned
registrations cover every instance subscription plus the process-wide invite event. GamerJoined
replays the existing local participant and later observes an injected retained remote; removal
raises GamerLeft with departure state. Start/End produce exact versioned callbacks, while injected
host and ended-state events produce exact HostChanged and disconnected SessionEnded payloads.
Every callback gamer handle works inside the call and is invalid immediately afterward. All
registrations release exactly once. The three leaderboard-write and invite subscriptions remain
silent, as expected for this Local workflow, so only their registration lifetime is claimed. Both
renderer-labelled direct runs pass byte-identically with empty stderr; strict C99 and metadata
compilation pass. `net_sessions.h` reaches72/104. Evidence:
`checkpoint-582-c-net-events-focused-*-direct.*`.

`standalone/c_api.net.local_gamer_packets` adds all16 LocalNetworkGamer routes from
`net_sessions.h` in28 controls. It proves standalone/session local identities and exact borrowed
signed-in text, same-session voice routing validation, Local party refusal, queue availability,
transport-style injection, plain/offset/PacketReader receives, sender-view ownership, queue clear
and every byte/range/recipient/PacketWriter send form. Both renderer-labelled direct runs are
byte-identical with empty stderr:27 controls pass and one deliberately fails under new
CNA-DEMOS-ISSUE-075 because successful offset receive zeroes caller bytes outside the requested
range. Strict C99 and metadata compilation pass; metadata marks the known issue. Two supporting
signed-gamer text routes are also newly demonstrated, and `net_sessions.h` reaches88/104. No wire
delivery, voice audio or party-service interaction is claimed. Evidence:
`checkpoint-583-c-local-packets-focused-*-direct.*`.

The four final MediaQueue reflection buffer controls in `c_api.media.media_player` were re-audited
before the next networking batch. Exact size, zero-capacity sizing, atomic short refusal and exact
unterminated copying all remain present. A fresh strict C99 build and both Xvfb/dummy-audio runs
pass all98 controls. This confirms the existing41/41 semantic disposition and corrects the stale
39/41 summary above; no duplicate demo or coverage credit was added.

`standalone/c_api.net.session_construction` adds the six remaining Local constructor/admission
routes from `net_sessions.h` in28 controls. The basic and property overloads consume one published
signed-in gamer, preserve limits/private slots/copied properties and admit a second local gamer
exactly once. All three canonical async adapters invoke their callback once before returning an
owned session, including the explicit two-gamer overload, and every instance/action count returns
to zero. Both renderer-labelled direct runs are byte-identical with empty stderr; strict C99 and
metadata compilation pass. The two supporting process-wide signed-in collection routes are also
newly demonstrated, the staged metadata union reaches2148/3205 and `net_sessions.h` reaches94/104.
No socket, service, concurrency, remote peer or project-built runner is claimed. Evidence:
`checkpoint-584-c-session-construction-focused-*-direct.*`.

`standalone/c_api.net.session_discovery_join` closes the final ten `net_sessions.h` routes in32
controls. All four implicit/explicit synchronous/asynchronous LocalWithLeaderboards searches
return owned empty collections, with each async callback invoked exactly once. Descriptive online
metadata cannot impersonate an authenticated search result, while a synthetic Local description
drives both generic Join adapters and marks the created process as a joining peer. All four invite
overloads truthfully return NotSupported without a configured service or invoking callbacks. Both
renderer-labelled direct runs are byte-identical with empty stderr; strict C99 and metadata
compilation pass. Staged metadata reaches2158/3205 and `net_sessions.h` is104/104. No socket,
authenticated directory, accepted invite, address reachability, remote peer or project-built
runner is claimed. Evidence:`checkpoint-585-c-session-discovery-join-focused-*-direct.*`.

`standalone/c_api.gamer.profile_identity` starts `gamer_services.h` with25 routes in19 controls.
An explicit offline SignedInGamer exposes the shared Gamer surface through exact/atomic gamertag,
mutable UTF-8 display name, matching text form, live state and a full 64-bit caller tag. Owned sync
and async profile snapshots expose versioned numeric values, motto/region buffer contracts and an
ordinary absent picture, then release independently. Global gamertag lookup and all partner-token
forms return their documented NotSupported result without async completion. Both renderer-labelled
direct runs are byte-identical with empty stderr; strict C99 and metadata compilation pass. Staged
metadata reaches2183/3205 and `gamer_services.h` reaches31/239. No online account lookup, token,
profile picture, game, renderer or project-built runner is claimed. Evidence:
`checkpoint-586-c-gamer-profile-focused-*-direct.*`.

`standalone/c_api.gamer.signed_in_state` adds19 `gamer_services.h` routes in29 controls. Two
explicit gamers distinguish guest/online/player identity, mutable party size, exact structured
presence, non-mutating free-text presence and bounded canonical privileges. The offline friend
query/collection is empty. Publishing only player two then distinguishes collection position from
player index, proves IndexOf/Contains missing results, independent borrowed handles and published
retention. Both renderer-labelled direct runs are byte-identical with empty stderr; strict C99 and
metadata compilation pass. The staged union reaches2202/3205 and `gamer_services.h` reaches50/239.
The passing workflow also records CNA-DEMOS-ISSUE-076: the public header says an absent player-index
lookup preserves `out_gamer`, while the generated wrapper clears it. No service friendship,
presence publication, microphone, game, renderer or project-built runner is claimed. Evidence:
`checkpoint-587-c-signed-in-state-focused-*-direct.*`.

`standalone/c_api.gamer.friend_collection` adds19 `gamer_services.h` routes in20 controls. Two
synthetic friends preserve independent online/playing/away/busy and request flags plus exact atomic
free-text presence. Their retained mutable collection exposes ordered indexing, both bounds,
IndexOf/Contains, atomic shifted CopyTo, complete/reset enumeration, present/repeated removal,
re-addition and Clear before independent one-shot source release. Both renderer-labelled direct
runs are byte-identical with empty stderr; strict C99 and metadata compilation pass. Staged
metadata reaches2221/3205 and `gamer_services.h` reaches69/239. No live account friend graph,
service state, game, renderer or project-built runner is claimed. Evidence:
`checkpoint-588-c-friend-collection-focused-*-direct.*`.

`standalone/c_api.gamer.achievement_collection` adds all29 achievement value/collection routes
from `gamer_services.h` in30 controls. Three factory values expose versioned numeric state, exact
atomic text, absent catalog text/artwork and value equality. Their copied collection demonstrates
owned index/key results, value membership, bounds, atomic shifted CopyTo, Add/Insert/Remove/
RemoveAt/Clear and independent lifetimes. Both renderer-labelled direct runs are byte-identical
with empty stderr;29 controls pass and one deliberately fails under CNA-DEMOS-ISSUE-077 because a
successfully mutable collection reports read-only. CNA-DEMOS-ISSUE-078 records the passing but
wrong `INTERNAL` result for an absent key whose contract promises `INVALID_ARGUMENT`. Strict C99
and metadata compilation pass. Staged metadata reaches2250/3205 and `gamer_services.h` reaches
98/239. The missing one-line `CMakeLists.txt` registrations for checkpoints584-588 were also
restored so the next coherent configure will build those five staged programs. No catalog-backed
picture/how-to text, gamer service, game, renderer or project-built runner is claimed. Evidence:
`checkpoint-589-c-achievement-collection-focused-*-direct.*`.

`standalone/c_api.gamer.signed_in_achievements` adds the final four signed-in achievement routes
in28 controls. A dedicated persistent application store and the deployed three-entry catalog
show sync read/award, exactly-once async award/read callbacks, catalog score/name/how-to text,
idempotent re-award with a stable first timestamp and both sync/async earned states surviving full
gamer-object replacement. Fresh SOFTWARE and OPENGLES3-labelled stores pass byte-identically with
empty stderr, and a second SOFTWARE run over the same store also matches exactly. Strict C99 and
metadata compilation pass. Staged metadata reaches2254/3205 and `gamer_services.h` reaches102/239.
The existing C++ achievement demo remains the sole runtime assertion for CNA-DEMOS-ISSUE-017;
this C demo does not duplicate it. No online service, game, renderer, notification appearance or
project-built runner is claimed. Evidence:
`checkpoint-590-c-signed-in-achievements-focused-*-direct.*`.

`standalone/c_api.gamer.guide_dialogs` adds all19 non-rendering Guide keyboard/message-box routes
in23 controls. Password input remains pending without completion, exposes exact/atomic title and
description text plus masked display text, refuses overlap, cancels exactly once and returns a
repeatable empty answer. A three-button box preserves initial focus and pending state across an
invalid click, then completes once with an exact repeatable choice. Both active reset paths discard
without callbacks and completed reset paths clear cached End results. Both renderer-labelled
direct runs pass byte-identically with empty stderr; strict C99 and metadata compilation pass.
Staged metadata reaches2273/3205 and `gamer_services.h` reaches121/239. The explicit headless
simulation extensions are exercised; drawing, real input and visible appearance remain separate.
Evidence:`checkpoint-591-c-guide-dialogs-focused-*-direct.*`.

`standalone/c_api.gamer.guide_dispatcher` adds20 Guide/dispatcher/component/event routes in22
controls. One real Xvfb game starts uninitialized, proves both pre-init pumps, exact 64-bit opaque
window storage, notification/trial validation and trial's update latch, then creates/adds/runs the
canonical GamerServicesComponent. Its frame installs the dispatcher and Guide with an actual
window handle; duplicate initialization is refused, async update reports initialized work and the
retired-identity count remains zero. Platform screen-saver state toggles and restores. Signed-in,
signed-out and title-update registrations attach, stay correctly silent with an isolated empty
profile store, release through the shared unsubscribe route and reject double release. Both Xvfb/
dummy-audio renderer runs pass byte-identically with empty stderr; strict C99 and metadata
compilation pass. Staged metadata reaches2293/3205 and `gamer_services.h` reaches141/239. No event
delivery, visible Guide panel, user interaction, pixel appearance or project-built runner is
claimed. Evidence:`checkpoint-592-c-guide-dispatcher-focused-*-direct.*`.

`standalone/c_api.gamer.guide_rendering` adds the two Guide dialog-render routes in a real HiDef
game. The first frame draws a pending password keyboard and the second cancels it exactly once,
resets its operation and draws a focused three-button alert. Back-buffer reads find69,500 changed
pixels over a464x151 region for the keyboard and100,004 changed pixels over a414x243 region for the
message box under both SOFTWARE and OPENGLES3. Separate one- and two-frame captures produce four
800x480 screenshots; all were inspected and show readable labels, six masked password glyphs, the
alert icon and the requested middle-button focus. All17 keyboard-capture and24 full-workflow
controls pass with empty stderr; strict C99 and metadata compilation pass. Staged metadata reaches
2295/3205 and `gamer_services.h` reaches143/239. This establishes actual rendering and visual
inspection, while real keyboard, mouse and controller interaction remain separate. Evidence:
`checkpoint-593-c-guide-rendering-focused-*-direct.*`.

`standalone/c_api.gamer.guide_social_offline` adds all14 remaining Guide screen routes in a real
dispatcher game with an isolated empty profile store. Invalid pane count and offline online-only
sign-in fail atomically; ordinary offline sign-in opens and automatically renders the one-pane
local-profile picker. After20 frames, exact samples prove the dimmed background, orange header and
panel, and the two renderer screenshots are byte-identical. Escape simulation closes the overlay.
Valid text, recipient and target handles then reach every service-backed route: compose, friend
request, friends, both invitation overloads, gamer card, messages, party, party sessions, review,
players and achievements return the documented not-supported result; marketplace preserves its
distinct missing-profile/privilege invalid-state result; none opens a partial screen. Both
26-control Xvfb runs pass with empty stderr; strict C99 and metadata compilation pass. Staged
metadata reaches2309/3205 and `gamer_services.h` reaches157/239. The first one-frame OPENGLES3
probe captured only the valid black opening transition; extending the demo to20 frames corrected
the test timing, with no CNA issue. `cna_guide_delay_notifications` remains intentionally
uncredited because its effect requires a delivered service invitation and has no public C
readback. Evidence:`checkpoint-594-c-guide-social-offline-focused-*-direct.*`.

`standalone/c_api.gamer.game_defaults_properties` adds the30 GameDefaults and PropertyDictionary
routes in24 renderer-free controls. `cna_game_defaults_init` and an explicit offline gamer's
versioned property agree on every canonical enum, flag, reserved byte and absent color; an unknown
structure version is atomic. One dictionary then holds all eight C-writable tagged kinds, reports
each kind, round-trips every typed value, proves that input strings are copied, and exercises exact
atomic string/key copies, ascending enumeration, replacement across kinds, invalid outcome and
getter refusals, remove/no-op, clear and stale-handle rejection. Its canonical read-only property
remains true throughout successful mutation. The stream-size extension is covered through its
wrong-kind/output-reset contract; a positive stream remains service-supplied because the public C
factory has no stream setter. Both SOFTWARE- and OPENGLES3-labelled runs pass the same24 controls
byte-for-byte with empty stderr; strict C99 and metadata compilation pass. Staged metadata reaches
2339/3205 and `gamer_services.h` reaches187/239. No game, renderer, service, stream payload or
project-built execution is claimed. Evidence:
`checkpoint-595-c-game-defaults-properties-focused-*-direct.*`.

`standalone/c_api.gamer.leaderboard_entries` adds all eight leaderboard-entry routes in23
renderer-free controls. Gamer-backed, independently equal and anonymous values expose exact
versioned ranking/rating/presence state. A borrowed gamer's identity round-trips, the live columns
dictionary accepts a tagged value and retains its entry storage after the entry handle is released,
and equality includes gamer/rating/ranking while excluding columns. A single rating hook runs once,
detaches exactly, and stale handles are refused. Both renderer-labelled runs pass byte-identically
with empty stderr; strict C99 and metadata compilation pass. Staged metadata reaches2347/3205 and
`gamer_services.h` reaches195/239. The anonymous-gamer result also records
CNA-DEMOS-ISSUE-079: the header promises an untouched absent output while the generated wrapper
deterministically clears it. No game, renderer, service, persisted leaderboard or project-built
execution is claimed. Evidence:`checkpoint-596-c-leaderboard-entries-focused-*-direct.*`.

`standalone/c_api.gamer.leaderboard_readers` adds the remaining15 leaderboard identity/reader
routes in30 renderer-free controls over separate isolated empty stores. All four canonical keys are
exact and NUL-padded with the requested mode; unknown identity and structure versions are atomic.
Explicit-page, pivot and gamer-restricted reads each succeed in sync and callback form with owned
empty readers, and every callback runs exactly once before return. Exact info/identity, empty
entry-bound refusal, synchronous and callback page-up toward zero, empty-final-page page-down
refusal and one-shot ownership all pass. SOFTWARE- and OPENGLES3-labelled outputs are byte-
identical with empty stderr; strict C99 and metadata compilation pass. Staged metadata reaches
2362/3205 and `gamer_services.h` reaches210/239. Positive entry lookup/page-down remains a
persisted writer/service scenario because public C exposes no writer or reader fixture. No game,
renderer, service, persisted row or project-built execution is claimed. Evidence:
`checkpoint-597-c-leaderboard-readers-focused-*-direct.*`.

`standalone/c_api.gamer.avatar_descriptions` adds `cna_avatar_expression_init` and all eight avatar-
description routes in26 renderer-free controls. A foreign-format exact1021-byte payload proves
copy-on-create, valid/unmeasured default semantics and atomic copy-out. Unconstrained plus explicit
female/male random avatars are valid and measured; their public codec round-trips exact body,
height and validity. An offline gamer read completes its callback once with an ordinary invalid
default description. Its per-instance Changed registration attaches, stays correctly silent in
the no-change workflow and releases once. Both renderer-labelled outputs are byte-identical with
empty stderr; strict C99 and metadata compilation pass. Staged metadata reaches2371/3205 and
`gamer_services.h` reaches219/239. Service-driven Changed delivery remains separate. No game,
renderer, service, avatar drawing or project-built execution is claimed. Evidence:
`checkpoint-598-c-avatar-descriptions-focused-*-direct.*`.

`standalone/c_api.gamer.avatar_animations` adds all seven avatar-animation routes in17 renderer-
free controls. Every one of31 built-in presets reports71 local bone transforms and a real2.5-to-
10-second timeline. Wave sampling changes its right-elbow transform at1.2 seconds; direct time
clamps, non-looping advance, remainder wrapping, canonical-boolean refusal and Celebration's keyed
happy-to-laughing facial expression all pass with atomic version/bounds and stale-handle checks.
Both renderer-labelled outputs are byte-identical with empty stderr; strict C99 and metadata
compilation pass. Staged metadata reaches2378/3205 and `gamer_services.h` reaches226/239.
CNA-DEMOS-ISSUE-080 records the stale C header statement that built-in presets have zero-length
timelines. No game, renderer, composed skeleton, pixels or project-built execution is claimed.
Evidence:`checkpoint-599-c-avatar-animations-focused-*-direct.*`.

`standalone/c_api.gamer.avatar_renderer` adds all11 avatar-renderer routes in43 controls inside a
real C game with a `GamerServicesComponent`. A female random description reaches Ready after its
public handle is released, and the renderer reports exact identity/authored transforms, optional
outputs, exact lighting plus atomic non-finite refusal, the canonical71-parent hierarchy, finite
bind pose, human-scale composed head height, bounds and one-shot ownership. `draw_animation` and
`draw_bones` both cover substantial two-dimensional back-buffer regions. SOFTWARE and OPENGLES3
runs pass with byte-identical normalized output and empty stderr; strict C99 and metadata
compilation pass. Both800x480 screenshots were inspected and show a complete waving avatar on the
clear background. Staged metadata reaches2389/3205 and `gamer_services.h` reaches237/239; its only
two uncredited routes are the intentionally service/manual Guide delay and physical-headset
observations above. No service avatar, microphone or project-built execution is claimed. Evidence:
`checkpoint-600-c-avatar-renderer-focused-*-direct.*`.

`standalone/c_api.media.library_collections` adds33 `media_library.h` routes in34 controls over
separate isolated empty Music/Pictures roots. Default and explicit Local Device construction,
exact/atomic source text, all five music collections, nonnegative counts, four upper bounds,
idempotent library disposal, child-retained lifetime, collection disposal-to-empty and stale
ownership all pass. SOFTWARE- and OPENGLES3-labelled Xvfb runs are byte-identical with empty
stderr; strict C99 and metadata compilation pass. Staged metadata reaches2422/3205 and
`media_library.h` reaches33/148. Positive entity values, relationships, album art, playlists,
pictures and save/import behavior remain for controlled fixtures; no host content, window, audio,
image or project-built execution is claimed. Evidence:
`checkpoint-601-c-media-library-collections-focused-*-direct.*`.

`standalone/c_api.media.library_music_values` adds42 `media_library.h` routes in33 controls over a
self-created private Linux/XDG catalog holding three original tagged FLAC tones and one M3U. Exact
and atomic entity names, track numbers, same-title/different-artist album identity, equality/hash,
album/artist/genre/playlist group membership, every Song back-reference, reversed playlist order,
duration aggregation, honest missing-art state, shared disposal and all retained releases pass
after the top-level collections and library are gone. SOFTWARE- and OPENGLES3-labelled Xvfb runs
are byte-identical with empty stderr; strict C99 and metadata compilation pass. Staged metadata
reaches2464/3205 and `media_library.h` reaches75/148. Album image bytes and the picture/save
subsystem remain next; no playback, audible quality, host media or project-built execution is
claimed. Evidence:`checkpoint-602-c-media-library-values-focused-*-direct.*`.

`standalone/c_api.media.library_album_art` adds the four album image-transfer routes in15 controls
over two isolated tagged albums. The covered album returns all1666 original encoded bytes; short
art and thumbnail buffers are atomic. The bare album refuses both size queries with
`CNA_RESULT_INVALID_STATE` without changing output. The generated thumbnail is a distinct,
programmatically parsed128x64 PNG retaining the source's2:1 aspect ratio. SOFTWARE- and
OPENGLES3-labelled Xvfb runs pass byte-identically with empty stderr; strict C99 and metadata
compilation pass. Staged metadata reaches2468/3205 and `media_library.h` reaches79/148. The
picture hierarchy, tokens, collections and both save/import routes remain next; no rendering,
screenshot, visual inspection or project-built execution is claimed. Evidence:
`checkpoint-603-c-media-library-album-art-focused-*-direct.*`.

`standalone/c_api.media.library_pictures` adds all43 meaningful picture, picture-album, collection,
token and save routes in57 controls over a two-level private Linux/XDG hierarchy. Exact names,
dimensions, timestamps, parents, ordered membership, equality/hashes, atomic tokens and image
copies, lookup absence, a generated128x64 thumbnail, buffer save and full storage-stream import all
pass. Both live save collections grow together; shared entity disposal, collection emptying,
child-retained lifetime and one-shot cleanup are explicit. SOFTWARE- and OPENGLES3-labelled Xvfb
runs are byte-identical with empty stderr; strict C99 and metadata compilation pass. Staged
metadata reaches2511/3205 and `media_library.h` reaches122/148. Its other26 routes are fixed
reflection type-name pairs and intentionally trivial/uncredited, so all148 now have semantic
dispositions. No rendering, screenshot, visual inspection, host media or project-built execution
is claimed. Evidence:`checkpoint-604-c-media-library-pictures-focused-*-direct.*`.

`standalone/c_api.content.cnb_foundations` begins the previously uncovered `cnb.h` audit with26
pure format routes in25 controls. Exact CNB v1 constants and magic, printable/sanitized chunk IDs,
mandatory flags, built-in/reserved/custom asset identities, custom-name hashing, raw logical-name
verdicts, versioned allocation limits and atomic overflow guards all pass. One-shot, split and
portable CRC-32C agree with the published iSCSI vector. Stored compression is byte-exact,
unimplemented codecs return `NOT_SUPPORTED`, and the linked Zstandard path clamps effort, shrinks
and round-trips the repetitive payload. Both renderer-labelled direct runs are byte-identical with
empty stderr; strict C99 and metadata compilation pass. Staged metadata reaches2537/3205 and
`cnb.h` reaches26/272. No game, renderer, file, compiled asset or project-built execution is
claimed. Evidence:`checkpoint-605-c-cnb-foundations-focused-*-direct.*`.

`standalone/c_api.content.cnb_byte_io` adds38 bounded-reader and primitive-byte-writer routes in27
controls. A deterministic52-byte little-endian payload covers every integer and floating-point
primitive, length-prefixed UTF-8, counts, raw bytes and zero padding. Writer copies and ownership-
transferring take operations are atomic on short destinations; the copied reader exposes exact
context, cursor and remaining state, preserves position on short raw destinations, retains its
last decoded string, distinguishes trailing bytes from exact exhaustion and rejects EOF, explicit
schema failure and stale access without corrupting outputs. The UTF-8 predicate accepts a real
multibyte sequence and rejects malformed input. Both renderer-labelled direct runs pass the same27
controls byte-for-byte with empty stderr; strict C99 and metadata compilation pass. Staged metadata
reaches2575/3205 and `cnb.h` reaches64/272. No game, renderer, filesystem, complete CNB document or
project-built execution is claimed. Evidence:
`checkpoint-606-c-cnb-byte-io-focused-*-direct.*`.

`standalone/c_api.content.cnb_document` adds all40 document/writer routes in51 controls. A
deterministic Curve container owns four copied schema chunks, canonical metadata and one replaced
external reference. Writer counts, limits, optional Zstandard selection, exact repeat builds,
atomic short-buffer refusal, file output and stale ownership all pass. Memory and file parsing
recover six table entries; exact origin/version/type/schema facts, chunk metadata/data, duplicate
lookup, required/mandatory rules, metadata strings, reference records/names, asset requirements and
parse limits are explicit. A borrowed chunk reader prevents premature document destruction, while
truncated, garbage and absent-file inputs publish no partial handle. Both renderer-labelled direct
runs pass the same51 controls byte-for-byte with empty stderr; strict C99 and metadata compilation,
diff checks and validation self-tests pass. Staged metadata reaches2615/3205 and `cnb.h` reaches
104/272. No game, renderer, decoded asset or project-built execution is claimed. Evidence:
`checkpoint-607-c-cnb-document-focused-*-direct.*`.

`standalone/c_api.content.cnb_texture_data` adds20 texture-format and neutral texture-data routes in
26 controls. It fixes the wire values, exact/atomic diagnostic text, known/unknown classifications,
unit sizes, volume and block-rounded level sizes, and all27 two-way runtime SurfaceFormat mappings.
A copied2x2 Rgba8 value and a general8x4x2 three-mip volume cover versioned shape, mip dimensions,
representation order/format/count, copied level bytes, atomic short copies, bounds and stale
ownership. A second BC7 representation proves synchronous preference selection and ordinary no-
match behavior. Both renderer-labelled direct runs are byte-identical with empty stderr; strict
C99 and metadata compilation, diff checks and validation self-tests pass. Staged metadata reaches
2635/3205 and `cnb.h` reaches124/272. No document, graphics device, GPU texture, renderer, pixels or
project-built execution is claimed. Evidence:
`checkpoint-608-c-cnb-texture-data-focused-*-direct.*`.

`standalone/c_api.content.cnb_texture_codecs` adds the remaining eight texture codec routes in22
controls. Synthetic Rgba8 data round-trips through complete 2D, cube and volume documents with
three mips, six face-major two-mip faces and depth-aware mips respectively. Exact sizing, atomic
short output, asset/chunk identity, every payload byte, wrong-decoder refusal, cube shape rules and
schema-v1 rejection of BC7 all pass. A SpriteFont writer embeds the same canonical texture chunks;
the embedded reader restores its atlas while the standalone 2D decoder correctly refuses the non-
texture asset identity. Both renderer-labelled direct runs are byte-identical with empty stderr;
strict C99 and metadata compilation, diff checks and validation self-tests pass. Staged metadata
reaches2643/3205 and `cnb.h` reaches132/272. No graphics device, GPU texture, renderer, filesystem,
displayed pixels or project-built execution is claimed. Evidence:
`checkpoint-609-c-cnb-texture-codecs-focused-*-direct.*`.

`standalone/c_api.content.cnb_model_core` adds the first30 model-data routes in26 controls. One
neutral graph owns two indexed bones, a complete triangle part, copied vertex/index bytes and a
fully populated material. Exact atomic bone/part/effect strings, versioned numeric values and stale
ownership pass. All eight independently named material texture slots remain distinct from all
seven importer-state slots, whose coordinate sets, UV transforms and samplers round-trip exactly.
Canonical boolean and every tested nested bound refusal remain explicit. Both renderer-labelled
direct runs are byte-identical with empty stderr; strict C99 and metadata compilation, diff checks
and validation self-tests pass. Staged metadata reaches2673/3205 and `cnb.h` reaches162/272. No
document codec, ContentManager, graphics device, renderer or draw call is claimed. Evidence:
`checkpoint-610-c-cnb-model-core-focused-*-direct.*`.

`standalone/c_api.content.cnb_model_morph` adds all12 morph routes in20 controls. A two-vertex part
owns two morph targets with distinct position/normal delta streams, two default weights and one
cubic weight key carrying separate weight/in-tangent/out-tangent streams. Exact live derived
counts, atomic copies, caller-buffer independence, empty streams, selector refusals and flag
replacement without losing built streams all pass. Clearing the entire nested graph is idempotent
and subsequent stream access is refused. Both renderer-labelled direct runs are byte-identical
with empty stderr; strict C99 and metadata compilation, diff checks and validation self-tests pass.
Staged metadata reaches2685/3205 and `cnb.h` reaches174/272. No document codec, mesh deformation,
animation player, graphics device, renderer or draw call is claimed. Evidence:
`checkpoint-611-c-cnb-model-morph-focused-*-direct.*`.

`standalone/c_api.content.cnb_model_scene` adds the remaining20 neutral model/codec routes in28
controls. A model owns one child-attached mesh, a two-joint skeleton with three distinct matrix
sets, a two-key scene-node animation and a directional light. Exact atomic names, copied part and
hierarchy indices, keyframes and skeleton removal pass before and after a complete Model document
encode/parse/decode round-trip. Encoder sizing and short-output behavior are atomic, and decoded
skeleton clear is idempotent. Both renderer-labelled direct runs are byte-identical with empty
stderr; strict C99 and metadata compilation, diff checks and validation self-tests pass. Staged
metadata reaches2705/3205 and `cnb.h` reaches194/272, completing all62 neutral model-data and model-
codec routes. No CNJ file, ContentManager, graphics device, renderer, animation playback or draw
call is claimed. Evidence:`checkpoint-612-c-cnb-model-scene-focused-*-direct.*`.

`standalone/c_api.content.cnb_model_from_cnj` adds all nine filesystem CNJ compiler-result routes
in18 controls. A private Model document absorbs exact vertex/index sidecars, publishes two sorted
external asset names and transfers its compiled graph exactly once. Both dependency lists remain
readable after the transfer; atomic short copies, Count bounds, stale access, missing-file refusal
and the model's independent lifetime after result destruction all pass. Both renderer-labelled
direct runs are byte-identical with empty stderr; strict C99 and metadata compilation, diff checks
and validation self-tests pass. Staged metadata reaches2714/3205 and `cnb.h` reaches203/272. No
ContentManager, graphics device, renderer, draw call or project-built execution is claimed.
Evidence:`checkpoint-613-c-cnb-model-from-cnj-focused-*-direct.*`.

`standalone/c_api.content.cnb_sprite_font_data` adds all11 neutral SpriteFont routes in24 controls.
Two complete glyph rows, derived metrics and a copied8x4 Rgba8 atlas survive deterministic complete
document encode/parse/decode. The workflow also proves default-character presence, canonical-bool
refusal, rollback of a glyph with nonzero reserved data, Count bounds, independent repeated atlas
copies and stale ownership. A deliberately descending character map is refused until the same rows
are sorted with the setter; sizing and short-output behavior are atomic. Both renderer-labelled
direct runs are byte-identical with empty stderr; strict C99 and metadata compilation, diff checks
and validation self-tests pass. Staged metadata reaches2725/3205 and `cnb.h` reaches214/272. No
ContentManager, graphics device, renderer, text layout, draw call or project-built execution is
claimed. Evidence:`checkpoint-614-c-cnb-sprite-font-data-focused-*-direct.*`.

`standalone/c_api.content.cnb_sound_effect_data` adds all nine audio-format and neutral SoundEffect
routes in17 controls. Exact fixed-frame sizes and atomic diagnostic names cover every published
format plus an unknown identity. Copied stereo PCM8 bytes, metrics and an interior loop survive a
deterministic schema-two document encode/parse/decode after caller mutation and original-handle
release. Sample and document short buffers are atomic; stale ownership, an inconsistent byte count
and a reserved packet format are refused at their documented boundaries. Both renderer-labelled
direct runs are byte-identical with empty stderr; strict C99 and metadata compilation, diff checks
and validation self-tests pass. Staged metadata reaches2734/3205 and `cnb.h` reaches223/272. No
SoundEffect runtime object, audio device, playback, audible output or project-built execution is
claimed. Evidence:`checkpoint-615-c-cnb-sound-effect-data-focused-*-direct.*`.

`standalone/c_api.content.cnb_media_metadata` adds all ten Song and Video metadata routes in20
controls. A named195-second Song and a42-second1280x720 Video encode as deterministic complete
documents whose specialized stream-reference views exactly match their single XREF dependencies.
Atomic sizing/copying, a valid nameless/unknown-duration Song, exact video soundtrack/rate state,
invalid Song reference and three invalid video shapes all pass. Each decoder rejects the other's
asset type, and released-document access preserves output. Both renderer-labelled direct runs are
byte-identical with empty stderr; strict C99 and metadata compilation, diff checks and validation
self-tests pass. Staged metadata reaches2744/3205 and `cnb.h` reaches233/272. No media file,
ContentManager, decoder, audio/video device, playback, rendering or project-built execution is
claimed. Evidence:`checkpoint-616-c-cnb-media-metadata-focused-*-direct.*`.

`standalone/c_api.content.cnb_curve_animation` adds all11 Curve, AnimationClip and shared keyframe
routes in29 controls. A two-key runtime Curve round-trips as an ordinary independent curve handle;
a two-track scene-node clip copies borrowed key arrays, preserves target space and survives its
document. Exact track bounds and atomic keyframe copies pass. The shared byte writer emits the
published48-byte row, its reader restores it, and finite/unrepresentable seconds are separated as
success versus content failure without output corruption. Both renderer-labelled direct runs are
byte-identical with empty stderr; strict C99 and metadata compilation, diff checks and validation
self-tests pass. Staged metadata reaches2755/3205 and `cnb.h` reaches244/272. No model, animation
player, graphics device, renderer, drawing or project-built execution is claimed. Evidence:
`checkpoint-617-c-cnb-curve-animation-focused-*-direct.*`.

`standalone/c_api.content.cnb_loader_registry` adds all11 registry/loader routes in21 controls. A
real pair of distinct canonical names with the same31-bit FNV identity proves collision refusal;
the matching custom file resolves and invokes with live callback-scoped document/manager handles,
which expire immediately afterward. Numeric and document resolution return owned loader copies
that remain invocable after registration removal. Built-in registration is idempotent, and its
Curve loader reports `NOT_SUPPORTED` rather than exposing a C++ object to C. Both Xvfb direct runs
pass byte-identically with empty stderr; strict C99 and metadata compilation, diff checks and
validation self-tests pass. Staged metadata reaches2766/3205 and `cnb.h` reaches255/272. The
caller-owned graphics device exists only because the canonical loader signature requires a real
ContentManager; no pixels or drawing are claimed. No project-built execution is claimed. Evidence:
`checkpoint-618-c-cnb-loader-registry-focused-*-direct.*`.

`standalone/c_api.content.cnb_importers` adds all five image, DDS and WAV import/decode front-end
routes in23 controls. A private one-pixel BMP proves default RGBA8 decoding plus matching and
nonmatching color-key semantics. A complete six-face DXT1 cube is decoded from caller memory and
from a file into identical owned RGBA8 faces, while non-cube, truncated and missing sources fail
without partial handles. A looping mono PCM8 WAV preserves its exact samples, rate, shape and
inclusive RIFF `smpl` region through both front ends after caller memory changes; compressed,
truncated and missing inputs fail atomically. Both renderer-labelled direct runs are byte-identical
with empty stderr; strict C99, metadata compilation, diff checks and validation self-tests pass.
Staged metadata reaches2771/3205 and `cnb.h` reaches260/272. No ContentManager, graphics/audio
device, rendering, playback, audible/visual quality or project-built execution is claimed.
Evidence:`checkpoint-619-c-cnb-importers-focused-*-direct.*`.

`standalone/c_api.content.cnb_compile_cnj` adds the final12 generic CNJ-to-CNB result routes in22
controls. A private Model CNJ and two binary sidecars produce deterministic complete bytes, exact
Model identity/name, a CNJ-first authored absorbed-file list and sorted external effect/texture
names. All text and byte transfers are atomic, Count bounds preserve output, copied bytes outlive
the result and parse/decode into the expected quad model. Stale access plus missing and unsupported
documents fail without partial results. Both renderer-labelled direct runs are byte-identical with
empty stderr; strict C99, metadata compilation, diff checks and validation self-tests pass. Staged
metadata reaches2783/3205 and `cnb.h` reaches272/272, completing the public header's semantic route
audit. No ContentManager, graphics device, renderer, animation playback, draw call, visual result
or project-built execution is claimed. Evidence:
`checkpoint-620-c-cnb-compile-cnj-focused-*-direct.*`.

`standalone/c_api.graphics.model_bones` adds the first17 `models.h` routes in25 controls. Default
and named bones preserve empty/copied UTF-8 text, signed indices and complete local matrices. A
child view obtained before mutation tracks a two-level retained hierarchy in authored order;
indexed, named, membership and parent views identify the same nodes, enforce Count bounds and
survive release of their original handles. Self-parenting and ancestor cycles are atomic refusals,
as are malformed UTF-8 and stale handles. The first draft expected the malformed name to return
`INVALID_ARGUMENT`; the public string boundary correctly returns `ENCODING`, so this was a demo
bug rather than a CNA issue. Both renderer-labelled direct runs are byte-identical with empty
stderr; strict C99, metadata compilation, diff checks and validation self-tests pass. Staged
metadata reaches2800/3205 and `models.h` reaches17/216. No Model, mesh, ContentManager, graphics
device, renderer, animation player, draw call, visual result or project-built execution is
claimed. Evidence:`checkpoint-621-c-model-bones-focused-*-direct.*`.

`standalone/c_api.graphics.model_mesh_parts` adds21 more `models.h` routes in23 controls. Default
and explicit standalone parts preserve all four signed scalar fields across their complete ranges,
the opaque64-bit tag, every published primitive topology and all seven independent material and
specular sampler slots. Invalid topology, sampler version/slot and collection inputs leave prior
state and outputs intact. An ordered snapshot retains the exact live parts independently of its
caller array and original handles; aliases share mutation and survive collection release. Both
renderer-labelled direct runs are byte-identical with empty stderr; strict C99, metadata
compilation, diff checks and validation self-tests pass. Staged metadata reaches2821/3205 and
`models.h` reaches38/216. No effect, buffer, Model, mesh, graphics device, renderer, draw call,
visual result or project-built execution is claimed. Evidence:
`checkpoint-622-c-model-mesh-parts-focused-*-direct.*`.

`standalone/c_api.graphics.model_meshes` adds32 more `models.h` routes in33 controls. A real
drawable part retains its exact effect and vertex/index buffers and refuses premature destruction;
named and empty meshes preserve values, parent ownership and live views. Ordered mesh snapshots,
lookup, collection bounds, duplicate effect mutation and stable aliases all pass. Exact framebuffer
probes prove the indexed mesh draws its green center pixel and an empty mesh preserves the black
clear. The audit found CNA-DEMOS-ISSUE-081: native and C ModelMesh construction omit an effect
already attached to a part from `Effects`; detach/reattach through the public setter synchronizes
the collection. Both SOFTWARE and OPENGLES3 Xvfb direct runs pass33/33 with byte-identical stdout
and empty stderr; strict C99 and metadata compilation, diff checks and validation self-tests pass.
Staged metadata reaches2853/3205 and `models.h` reaches70/216. No ContentManager, imported asset,
top-level Model, screenshot, visual-quality assessment or project-built execution is claimed.
Evidence:`checkpoint-623-c-model-meshes-focused-*-direct.*`.

`standalone/c_api.graphics.models` adds27 more `models.h` routes in39 controls. All three
Model constructors, collections/root, tags, owned-resource callbacks and atomic local/absolute
transform transfers pass. An authored glTF report and warning retain all scalar and text channels
and derive the exact loss totals. Actual drawing composes the child bone's absolute transform with
the supplied world matrix: the green triangle center reaches(700,120), while(400,240) stays black.
The first probe expected world alone; source review confirmed the parent composition, so that was a
demo error rather than a CNA defect. Both SOFTWARE and OPENGLES3 Xvfb direct runs pass39/39 with
byte-identical stdout and empty stderr; strict C99 and metadata compilation, diff checks and
validation self-tests pass. Staged metadata reaches2880/3205 and `models.h` reaches97/216. No
ContentManager, imported glTF, animation player, screenshot, visual-quality assessment or project-
built execution is claimed. Evidence:`checkpoint-624-c-models-focused-*-direct.*`.

`standalone/c_api.graphics.morph_targets` adds all26 morph-related `models.h` routes in29
controls. Deep-copied base/delta/weight/track inputs, distinct linear/step/cubic evaluation, exact
normalized CPU blending, tangent streams and flat-normal triangle metadata all pass their atomic
contracts. ModelMeshPart retains the data beyond its original handle; weights(0.5,0.25) produce
position(0.5,0,0.5) in direct VertexBuffer readback. Both SOFTWARE and OPENGLES3 Xvfb direct runs
pass29/29 with byte-identical stdout and empty stderr; strict C99 and metadata compilation, diff
checks and validation self-tests pass. Staged metadata reaches2906/3205 and `models.h` reaches
123/216. No mesh, Model, ContentManager, animation player, draw call, visual result or project-built
execution is claimed. Evidence:`checkpoint-625-c-morph-targets-focused-*-direct.*`.

`standalone/c_api.graphics.skinned_models` adds26 `models.h` routes in28 controls. Copied
skeleton/bind arrays and clips, sorted lookup, transform evaluation, atomic hierarchy refusal and
move construction/assignment all pass. Matching-skeleton attachment transfers parts. A real
same-device part retains its buffers and optional texture; exact-name replacement releases the old
set and owns the donor set until removal. Both SOFTWARE and OPENGLES3 Xvfb direct runs pass28/28
with byte-identical stdout and empty stderr; strict C99 and metadata compilation, diff checks and
validation self-tests pass. Staged metadata reaches2932/3205 and `models.h` reaches149/216. No
skinning shader, texture sampling, draw call, visual result or project-built execution is claimed.
Evidence:`checkpoint-626-c-skinned-models-focused-*-direct.*`.

`standalone/c_api.graphics.skinning_data_player` adds32 `models.h` routes in28 controls.
Deep-copied hierarchy, bind, inverse-bind, root-prefix and sorted clip values pass exact and atomic
transfers. Clip target space and rig-root metadata remain explicit. AnimationPlayer distinguishes
empty/bind state, named selection, half-time local/world/skin composition, absolute clamp and loop,
invalid updates, and retains playback after the source SkinningData handle is released. Both
SOFTWARE and OPENGLES3 renderer-labelled direct runs pass28/28 with byte-identical stdout and empty
stderr; strict C99 and metadata compilation, diff checks and validation self-tests pass. Staged
metadata reaches2964/3205 and `models.h` reaches181/216. No Model, ContentManager, graphics device,
renderer, draw call, visual result or project-built execution is claimed. Evidence:
`checkpoint-627-c-skinning-data-player-focused-*-direct.*`.

`standalone/c_api.graphics.model_animations` adds the11 `ModelAnimationsEXT` and direct pose-helper
routes in29 controls. Two deep-copied clips expose exact sorted metadata and explicit target spaces.
A real two-bone Model proves joint-palette refusal, half-time scene-node posing, untouched untracked
bones, hierarchy composition, time clamping and non-finite atomicity. The bind-pose helper uploads a
separately authored two-bone palette to one real SkinnedEffect and reports exactly one posed effect.
Both SOFTWARE and OPENGLES3 Xvfb direct runs pass29/29 with byte-identical stdout and empty stderr;
strict C99 and metadata compilation, diff checks and validation self-tests pass. Staged metadata
reaches2975/3205 and `models.h` reaches192/216. No vertex/index buffer, texture, draw call, visual
result or project-built execution is claimed. Evidence:
`checkpoint-628-c-model-animations-focused-*-direct.*`.

`standalone/c_api.graphics.model_scene_metadata` adds16 `models.h` routes in28 controls. An authored
infinite perspective camera round-trips all flags, scalar values, matrices and atomic name copies.
A one-bone SkinningData maps to a real Model mesh by index, survives source release through a fresh
owned handle and distinguishes an unrigged skin. The aggregate sphere exactly places center
(1,2,3), radius2 through root translation x=10. The initial demo expected zero aspect ratio to be
rejected; source review showed the native XNA factory accepts it, so the check was corrected and was
not a CNA defect. Both SOFTWARE and OPENGLES3 Xvfb direct runs pass28/28 with byte-identical stdout
and empty stderr; strict C99 and metadata compilation, diff checks and validation self-tests pass.
Staged metadata reaches2991/3205 and `models.h` reaches208/216. No material, buffer, texture, draw
call, visual result or project-built execution is claimed. Evidence:
`checkpoint-629-c-model-scene-metadata-focused-*-direct.*`.

The external renderer-retirement blocker observed during focused526-630 is resolved. Complete634
configured and built the full current tree against clean CNA `53aba6ef15`, validating the actual
627 catalog IDs and643 declared/640 enabled CTests. All focused evidence from checkpoints526-633 is
now absorbed into a project-built runner checkpoint; retain those focused logs only as diagnostic
history. Future CNA changes may still occur externally, so record its state before each complete
build and never edit or revert it from this project.

Focused635 extends `graphics.vertex_declaration.stock_vertex_types` in place. Its eight controls
cover stable IVertexType/static declaration identity, all exact packed declaration elements and
strides, every authored public field, equality/inequality, typed and boxed Equals with wrong-type
rejection, equal hashes, exact field-labelled text, and the retained checker/tint draw. The project
build passes8/8 under SOFTWARE and OPENGLES3 with byte-identical stdout and empty stderr; the
SOFTWARE target CTest passes too. Temporary generated coverage projects1417->1437/2326 XNA members
without changing demo/type/C-route counts. Evidence:`/tmp/checkpoint-635-stock-*.{out,err,json}` and
`/tmp/checkpoint-635-coverage.md`. Pixel readback was programmatic; no screenshot or visual
inspection was performed. The four explicit CLR interface aliases remain intentionally uncredited
because the exact C++ property path is executed. Focused635 used clean sibling CNA `09c3108e5`.

Focused636 extends four existing SpriteFont IDs. `glyph_constructor` passes eight controls for exact
retained atlas/table inspection, asymmetric cropping, signed bearing measurement and programmatic
pixel placement. `default_character` passes six controls including atomic invalid-fallback refusal
and optional reset; `measure_string` passes seven including empty and CR/CRLF rules; `spacing`
passes four including copy independence and negative gap spacing. The whole seven-ID category
passes under SOFTWARE and OPENGLES3; all four changed SOFTWARE CTests pass. SOFTWARE batch stderr
is empty. The OPENGLES3 category batch emits six GTK initialization-order warnings while all seven
results pass; the isolated final glyph run passes too. No screenshot or visual inspection was
performed. XNA inventory remains1437/2326 because its five SpriteFont members were already credited;
the four now-direct CNAEXT inspection methods are outside that inventory. Evidence:
`/tmp/checkpoint-636-sprite-font-*`. Final build is diagnostic-free against clean CNA `09c3108e5`.

Focused637 extends `graphics.sprite_batch.begin_end_rules` from five to twelve controls. Explicit
Dispose/event/idempotence and three post-dispose refusals pass; a queued temporary Texture2D wrapper
leaves scope before End yet still yields exact orange/black pixels. The final control reproduces new
CNA-DEMOS-ISSUE-082: disposing an open deferred batch never releases GraphicsDevice's active-batch
counter, so another Immediate batch is refused. SOFTWARE and OPENGLES3 each report eleven passes
plus the same expected failure with byte-identical stdout and empty stderr; the known-issue CTest
passes. Temporary coverage projects1437->1438/2326 through SpriteBatch.Dispose. Evidence:
`/tmp/checkpoint-637-sprite-batch-*` and `/tmp/checkpoint-637-coverage.md`. No screenshot or visual
inspection was performed. Clean sibling CNA remains `09c3108e5`; no CNA source was edited.

Focused638 extends `input.gamepad.state_values` from11 to18 controls for typed/boxed equality,
hashes and exact text across GamePadState and all four nested values. Its renderer-free run passes
with empty stderr. `input.gamepad.capabilities` now reads both stick-button properties and no longer
declares a GamePad need: the existing disconnected path had been unreachable under the very
condition it tested. With no physical controller present, SOFTWARE and OPENGLES3 each pass eight
checks for four disconnected/no-control slots with byte-identical stdout and empty stderr. This is
no connected-hardware claim. Both focused CTests pass. Temporary coverage projects1438->1460/2326;
the next full smoke should convert one skip to pass per renderer. Evidence:
`/tmp/checkpoint-638-gamepad-*` and `/tmp/checkpoint-638-coverage.md`. No screenshot or visual
inspection was performed. Final build is diagnostic-free against clean CNA `09c3108e5`.

Focused639 extends `media.video_player.pause_and_loop` from six to12 controls without changing its
real decode, pause, resume or loop workflow. It disposes the player while the loop is active and
checks IsDisposed, Stopped state, null Video and zero PlayPosition; repeated Dispose is harmless;
and Play, Stop and GetTexture each refuse use with ObjectDisposedException. SOFTWARE and OPENGLES3
both pass12/12 with empty stderr, while their naturally different playback timings prevent a byte-
identical-output claim. The focused SOFTWARE CTest passes. Temporary coverage projects1460->
1462/2326 through VideoPlayer.Dispose and IsDisposed, with no catalog/type/C-route change.
Evidence:`/tmp/checkpoint-639-video-lifecycle-*` and `/tmp/checkpoint-639-coverage.md`. No screenshot,
frame-pixel inspection, visual-quality or audible-quality claim was made. Final build is diagnostic-
free against clean CNA `09c3108e5`; no CNA source was edited.

Focused640 extends `audio.sound_effect_instance.apply_3d` from seven to10 controls. It checks the
exact listener/emitter default records, round-trips a nonzero AudioListener.Velocity and nondefault
AudioEmitter.Up, and passes that authored state to Apply3D while the voice remains stopped before
restoring neutral values for the existing mode transitions and sweep. SOFTWARE and OPENGLES3 pass
10/10 with byte-identical stdout and empty stderr; the focused SOFTWARE CTest passes. Temporary
coverage projects1462->1464/2326 with no catalog/type/C-route change. This is public state and
accepted-input evidence under dummy audio, not mixed Doppler, panning, orientation or listening
evidence. Evidence:`/tmp/checkpoint-640-audio-spatial-*` and `/tmp/checkpoint-640-coverage.md`. No
screenshot or visual inspection was performed. Final build is diagnostic-free against clean CNA
`09c3108e5`; no CNA source was edited.

Focused641 extends `audio.xact.categories` from10 to12 controls and
`audio.xact.renderer_details` from three to six. AudioCategory now exercises typed/boxed equality,
wrong-type rejection, equal hashes and exact text, including equal SFX names from two different
engines whose controls remain engine-specific. RendererDetail copies CNA's actual SDL3_mixer
record, checks typed/boxed equality, both equal operators, hash and friendly text, rejects empty/
wrong boxes, and retains both strings after its discovery engine is disposed. No second device was
invented and no unequal-record or output-device-selection claim is made. Both IDs pass under
SOFTWARE and OPENGLES3 with byte-identical per-ID stdout and empty stderr; both SOFTWARE CTests
pass. Temporary coverage projects1464->1472/2326 with no catalog/type/C-route change. Evidence:
`/tmp/checkpoint-641-*` and `/tmp/checkpoint-641-coverage.md`. No listening, screenshot or visual
inspection was performed. Final build is diagnostic-free against clean CNA `09c3108e5`; no CNA
source was edited.

Focused642 extends `media.song.properties` from five to seven controls. Three caller-owned Songs
show that records for the same file compare equal and hash equally despite different display names,
while different files remain unequal despite sharing a display name. Typed/System::Object Equals,
both operators, null comparison and each exact ToString value are exercised; the original metadata
and Dispose/IsDisposed workflow remains. SOFTWARE and OPENGLES3 pass7/7 with byte-identical stdout
and empty stderr; the focused SOFTWARE CTest passes. Temporary coverage projects1472->1477/2326
with no catalog/type/C-route change. Neither file is played, so this is no decoded or audible-
identity claim. Evidence:`/tmp/checkpoint-642-song-values-*` and
`/tmp/checkpoint-642-coverage.md`. No listening, screenshot or visual inspection was performed.
Final build is diagnostic-free against clean CNA `09c3108e5`; no CNA source was edited.

Focused643 extends the standalone `media.media_library.music_index` from18 to23 child controls.
A second independent MediaLibrary indexes the same isolated files and yields distinct but equal
Album/Artist/Genre/Playlist objects. Album identity combines title and artist; Artist, Genre and
Playlist identity uses the name. Typed/System::Object Equals, both operators, equal hashes, exact
text and wrong-type rejection now run against those real records. The five collection Item members
were also credited because the original and mirror workflows directly index them to establish
order and shared identity. Both runner selections pass with byte-identical stdout and the same six
expected TitleContainer INFO stderr lines; the focused SOFTWARE CTest passes. Temporary coverage
projects1477->1501/2326 with no catalog/type/C-route change. Different hashes for unequal values,
reflection helpers, borrowed disposal and post-library pointer use are not claimed. Evidence:
`/tmp/checkpoint-643-media-values-*` and `/tmp/checkpoint-643-coverage.md`. No playback, listening,
screenshot or visual inspection was performed. Both changed targets build diagnostic-free against
clean CNA `09c3108e5`; no CNA source was edited.

Focused644 extends `media.media_player.queue_and_repeat` from six to nine controls. MediaPlayer.Play
creates two ordered queue-owned Song records distinct from the caller objects; both items and the
active pointer remain readable after the caller Songs are destroyed. MediaQueue.Item also rejects
negative and count indices with ArgumentOutOfRangeException while preserving both entries. The
existing MoveNext/MovePrevious/repeat/shuffle workflow remains intact, and the unsafe ISSUE-058
Queue.ActiveSong replay path is not invoked. SOFTWARE and OPENGLES3 pass9/9 with byte-identical
stdout and empty stderr; the focused SOFTWARE CTest passes. Temporary coverage projects1501->
1502/2326 with no catalog/type/C-route change. No playback quality or listening claim is made.
Evidence:`/tmp/checkpoint-644-media-queue-*` and `/tmp/checkpoint-644-coverage.md`. No screenshot or
visual inspection was performed. Final build is diagnostic-free against clean CNA `09c3108e5`;
no CNA source was edited.

Focused645 extends `media.media_library.collections` from one to three controls and fixes its
returned-record ownership. Every raw MediaSource pointer is adopted by unique_ptr; CNA's actual
Local Device record has matching nonempty Name/ToString. Constructing MediaLibrary from that record
publishes a distinct copied MediaSource with the same name/type/text, and the existing false
IsDisposed assertion now receives metadata credit. SOFTWARE and OPENGLES3 pass3/3 with byte-
identical stdout and empty stderr; the focused SOFTWARE CTest passes. All runs use fresh empty XDG
music/picture directories, report zero library items and leave those directories empty, so no real
user library was read. Temporary coverage projects1502->1505/2326 with no catalog/type/C-route
change. This is CNA's one returned desktop source, not wider device enumeration. Evidence:
`/tmp/checkpoint-645-media-source-*` and `/tmp/checkpoint-645-coverage.md`. No playback, listening,
screenshot or visual inspection was performed. Final build is diagnostic-free against clean CNA
`09c3108e5`; no CNA source was edited.

Focused646 extends the standalone `media.media_library.save_picture` from14 to20 child controls
while retaining exactly one expected ISSUE-041 failure. The immediate Picture Date falls within
the SavePicture call; its CNAEXT path token resolves the independent reopened Picture through
public GetPictureFromToken, while an unknown token returns null. Original/reopened Picture and
Saved Pictures album objects exercise typed/System::Object equality, operators, equal hashes,
exact text and wrong-type refusal; Picture/PictureAlbum collection Item exposes both files and
their shared album. SOFTWARE and OPENGLES3 each produce19 passes plus ISSUE-041 with byte-identical
stdout and the same two TitleContainer INFO stderr lines; the known-issue SOFTWARE CTest passes.
Temporary coverage projects1505->1519/2326 with no catalog/type/C-route change. Borrowed Dispose
flags and post-library pointer use remain intentionally unclaimed. Evidence:
`/tmp/checkpoint-646-picture-values-*` and `/tmp/checkpoint-646-coverage.md`. No screenshot or visual
inspection was performed. Both changed targets build diagnostic-free against clean CNA
`09c3108e5`; no CNA source was edited.

Complete647 absorbs focused635 through646 in the actual 627-demo runner. The documented outer-Xvfb
validation passes metadata627/0, all640 enabled CTests, and both full smokes with zero unexpected
passes: SOFTWARE555 pass/34 expected fail/35 skip/3 manual and OPENGLES3587/34/3/3. Every status
and knownIssue pair matches complete634 except the intended new
`graphics.sprite_batch.begin_end_rules` ISSUE-082 expected failure. Generated coverage is297/329
types,1519/2326 members and2999/3205 C routes. The build is diagnostic-free and the binary/library
hashes are recorded in section7. Evidence:`build/linux-full/reports/checkpoint-647-complete-*`.
No screenshot, listening test or manual visual inspection was performed. The run began with clean
CNA `09c3108e58`; final capture found the externally changed dirty CNA state documented above, so
this is immutable binary/runtime evidence rather than a claim about that later source tree.

Focused648 extends `content.xnb.custom_type_reader` from five to10 controls and its original
generated XNB from118 to250 bytes. One coherent Level state block supplies an exact float/double,
nontrivial RGBA color, all16 row-major matrix elements, quaternion XYZW and Vector2/3/4 values;
LevelReader calls all eight public ContentReader helpers directly and the demo checks every field.
SOFTWARE and OPENGLES3 pass10/10 with byte-identical stdout and empty stderr; the focused SOFTWARE
CTest passes. The updated asset regenerates deterministically, the build is diagnostic-free, and
temporary coverage projects1519->1527/2326 without a catalog/type/C-route change. Evidence:
`/tmp/checkpoint-648-content-reader-*` and `/tmp/checkpoint-648-coverage.md`. No screenshot or
visual inspection was performed. The exercised production libraries retain the complete647 hashes;
the runner hash is `f162068386348d2b43e93b6a512b589fe47b85759fc5180a6c3dd53ba2b33220`.
Sibling CNA remained at `af798b73a4` while its externally owned dirty paths grew to seven; no CNA
source was edited or reverted here.

Focused649 adds the coherent console ID `input.mouse.state_values` with eight controls. Default and
fully authored MouseState records expose every XNA field; typed/boxed equality, wrong/empty boxes,
both operators, equal hashes, a changed coordinate and exact stable text close its value contract.
The nine-argument CNA extension additionally proves horizontal-wheel state is stored but excluded
from XNA equality/hash/text. Existing `input.keyboard.state_values` grows from eight to nine checks
for equal hashes across reordered key insertion and the empty-set zero hash. Both IDs pass on
SOFTWARE and OPENGLES3 with byte-identical per-ID stdout and empty stderr; both SOFTWARE CTests
pass. After the automatic new-source CMake regeneration emitted one external Draco policy warning,
the final incremental build is diagnostic-free. The staged catalog is628 demos and temporary
coverage projects1527->1534/2326; basic Input is112/113 with only platform-specific Mouse.WindowHandle
intentionally uncredited. Evidence:`/tmp/checkpoint-649-*` and `/tmp/checkpoint-649-coverage.md`.
No physical input, screenshot or visual inspection was performed. Production library hashes remain
the complete647 values; the runner is
`df5c168ad6fa00ca142e3744559d7b118bf6ace04d4f868f7e8e8e1c8f9cdab4`. Sibling CNA stayed at
`af798b73a4` while its external dirty set reached eleven paths; no CNA source was edited here.

Focused650 extends `input.touch.touch_values` from eight to17 controls. Its moved record and a
pressure-bearing copy exercise typed/boxed equality, wrong/empty boxes, both operators, equal
hashes, pressure exclusion and exact position-only text; a changed previous position is unequal.
False TryGetPreviousLocation/FindById paths now require their assigned invalid sentinels.
TouchCollection.CopyTo inserts the ordered snapshot at a nonzero vector index, while a copied
collection demonstrates the documented FNA behavior that IsReadOnly is advisory: Add, Insert,
Remove, RemoveAt and Clear all mutate with exact checked order/results. SOFTWARE and OPENGLES3
pass17/17 with byte-identical stdout and empty stderr; the focused SOFTWARE CTest passes and the
build is diagnostic-free. Temporary coverage projects1534->1545/2326 and Touch reaches41/48.
Remaining CLR Enumerator/interface aliases are dispositioned as iteration plumbing; TouchPanel's
WindowHandle is platform-specific. Evidence:`/tmp/checkpoint-650-*` and
`/tmp/checkpoint-650-coverage.md`. No touch hardware, screenshot or visual inspection was used.
Production library hashes remain the complete647 values; the runner is
`8871f4e6840285aef8f543b8b1cb4df9906542ca3e168dbc185ddf64656bb783`. Sibling CNA stayed at
external dirty `af798b73a4` with eleven observed paths; no CNA source was edited here.

Focused651 extends three existing math IDs without adding a variant. The generated/manual
BoundingBox, actual CreateFromBoundingBox sphere and real +X casting Ray each gain typed/boxed
equality, wrong/empty-box rejection, both operators, equal hashes and one changed component that is
unequal. Their construction, merge and hit-distance controls remain intact. SOFTWARE and
OPENGLES3 pass box6/6, sphere9/9 and ray8/8 with byte-identical per-ID stdout and empty stderr; all
three SOFTWARE CTests pass and the build is diagnostic-free. Temporary coverage projects1545->
1557/2326 without a catalog/type/C-route change. BoundingSphere.Contains/CreateFromFrustum remain
separate behavior work. Evidence:`/tmp/checkpoint-651-*` and `/tmp/checkpoint-651-coverage.md`. No
rendering, screenshot or visual inspection was used. Production library hashes remain complete647;
the runner is `030d05c1b3ff63aacee15d4fd4ff2acd534102a6dd4a89f91b7ff063faf8afdb`.
Sibling CNA remained external dirty `af798b73a4` with eleven observed paths; no CNA source was
edited here.

Focused652 extends `math.bounding_sphere.create` from nine to12 controls. Point containment now
distinguishes interior/boundary/exterior; sphere containment distinguishes nested/overlap/disjoint;
and a box plus identity frustum are unambiguously contained. CreateFromFrustum produces a positive
sphere enclosing all eight actual corners within1e-5 without claiming a minimum sphere. The first
focused run failed because the demo demanded strict per-corner Contains; Ritter construction plus
float rounding can put a boundary corner just outside strict classification. The corrected
geometric-tolerance check matches the prior C audit, so no CNA issue was recorded. SOFTWARE and
OPENGLES3 pass12/12 with byte-identical stdout and empty stderr; the focused SOFTWARE CTest passes
and the build is diagnostic-free. Temporary coverage projects1557->1559/2326 with no catalog/type/
C-route change. Evidence:`/tmp/checkpoint-652-*` and `/tmp/checkpoint-652-coverage.md`. No rendering,
screenshot or visual inspection was used. Production library hashes remain complete647; the runner
is `fb21e5d9c8a24dd312aedeb7e301caa606777b3267d7f741f6aac53fb3128298`. Sibling CNA remained
external dirty `af798b73a4` with eleven observed paths; no CNA source was edited here.

Focused653 extends `math.bounding_frustum.culling` from six to11 controls and
`math.plane.classify_points` from seven to10. An identity frustum gives exact left/right/top/bottom
planes and canonical six-plane text; an independent copy, changed matrix and boxed/wrong/empty
values exercise typed/boxed equality, both operators and equal hashes. The authored floor plane
adds homogeneous Vector4 Dot plus the same complete identity contract. SOFTWARE and OPENGLES3 pass
11/11 and10/10 respectively with byte-identical per-ID stdout and empty stderr; both focused
SOFTWARE CTests pass and the build is diagnostic-free. Temporary coverage projects1559->
1573/2326 without a catalog/type/C-route change. Evidence:`/tmp/checkpoint-653-*` and
`/tmp/checkpoint-653-coverage.md`. No rendering, screenshot or visual inspection was used.
Production library hashes remain complete647; the runner is
`91f4946b17f20fdfbddb4a1cfa54b4a0de216e9cbf7bf329caa6a5eee1865bf3`. Sibling CNA remained
external dirty `af798b73a4` with twelve observed paths; no CNA source was edited here.

Focused654 extends `math.color.construct` from six to eight controls,
`math.color.packed_value` from three to four and `math.rectangle.edges_and_center` from five to
seven. Real authored values exercise typed/boxed equality, wrong/empty boxes, both operators, equal
hashes and changed fields. Color additionally packs through its actual non-generic IPackedVector
base, proving midpoint rounding and high/low saturation in the packed-value workflow. SOFTWARE and
OPENGLES3 pass8/8,4/4 and7/7 with byte-identical per-ID stdout and empty stderr; all three focused
SOFTWARE CTests pass and the build is diagnostic-free. Temporary coverage projects1573->
1582/2326 without a catalog/type/C-route change. Evidence:`/tmp/checkpoint-654-*` and
`/tmp/checkpoint-654-coverage.md`. No rendering, screenshot or visual inspection was used.
Production library hashes remain complete647; the runner is
`76646cd7957e0a3581b3920a3423acffd1303797fc44653f2c337573aab90a50`. Sibling CNA advanced
externally to dirty `73991b01f7` with six observed paths; no CNA source was edited here.

Focused655 extends `math.matrix.element_operations` from eight to11 controls and
`math.quaternion.double_cover` from12 to15 while crediting the already-checked zero Matrix
constructor in `math.matrix.identity`. Matrix subtraction/division operators now agree with their
named element-wise methods; a real scale matrix exercises typed/boxed equality, wrong/empty boxes,
both identity operators and equal hashes. Quaternion component construction and the same identity
contract operate on an actual rotation, while component-wise Add/Subtract operators are explicitly
distinguished from orientation composition and relative rotation. SOFTWARE and OPENGLES3 pass
11/11,4/4 and15/15 with byte-identical per-ID stdout and empty stderr; all three focused SOFTWARE
CTests pass and the build is diagnostic-free. Temporary coverage projects1582->1595/2326 without
a catalog/type/C-route change. Evidence:`/tmp/checkpoint-655-*` and
`/tmp/checkpoint-655-coverage.md`. No rendering, screenshot or visual inspection was used.
Production library hashes remain complete647; the runner is
`f95758d2d3a2aceaf44ed5cb22dff39236331407818fc2fd20aed1f1d58ec9b3`. Sibling CNA remained
external dirty `73991b01f7` with six observed paths; no CNA source was edited here.

Focused656 closes all33 remaining inventoried Vector2/3/4 members across six existing IDs.
Vector2 magnitude now fixes construction and canonical axes; toolbox arithmetic adds complete
operator parity and typed/boxed identity; its transform workflow proves TransformNormal ignores
pivot translation. Vector3 construction receives credit from its authored arithmetic inputs and
the normalize workflow adds exact squared magnitude. Vector4 color math adds zero/one/four axes,
five-unit normalization, typed/boxed identity and every component-wise operator against the named
helpers. SOFTWARE and OPENGLES3 pass4/4,12/12,2/2,7/7,5/5 and17/17 with byte-identical per-ID
stdout and empty stderr; all six focused SOFTWARE CTests pass and the build is diagnostic-free.
Temporary coverage projects1595->1628/2326 and leaves no Vector2/3/4 member gap, without a catalog/
type/C-route change. Evidence:`/tmp/checkpoint-656-*` and `/tmp/checkpoint-656-coverage.md`. No
rendering, screenshot or visual inspection was used. Production library hashes remain complete647;
the runner is `41b6b9bf58d86dc538e5502e0a8037c7e79a1661c5d19c4f480a27c550ba9378`.
Sibling CNA remained external dirty `73991b01f7` with six observed paths; no CNA source was edited.

Focused657 extends `math.curve.key_collection` from10 to16 controls. A directly constructed
collection establishes its empty mutable contract; complete-key Contains differs from position-only
CompareTo; CopyTo preserves sentinels around the ordered insertion window; and independent
collection/Curve clones survive source removals. A cloned real middle key exercises typed/boxed
equality, wrong/empty boxes, both operators, equal hashes and tangent-sensitive inequality.
SOFTWARE and OPENGLES3 pass16/16 with byte-identical stdout and empty stderr; the focused SOFTWARE
CTest passes and the build is diagnostic-free. Temporary coverage projects1628->1639/2326 and
leaves only CurveKeyCollection's CLR `GetEnumerator` and explicit `IEnumerable.GetEnumerator`
aliases uncredited by disposition; C++ iteration is exposed through begin/end. No catalog/type/C-
route count changes. Evidence:`/tmp/checkpoint-657-*` and `/tmp/checkpoint-657-coverage.md`. No
rendering, screenshot or visual inspection was used. Production library hashes remain complete647;
the runner is `464337875c4f14424ca1cb4b1a0e53a7f8665b9c59fa406d3b45529f281283d7`.
Sibling CNA remained external dirty `73991b01f7` with six observed paths; no CNA source was edited.

Focused658 extends four existing component IDs. `change_events` grows2->3 controls and proves the
protected update-order, draw-order and visibility callbacks beside their public events.
`collection_events` grows4->6 with direct empty-collection and explicit event-args construction.
`game_component` grows4->5 with idempotent Dispose and one Disposed event, while
`drawable_game_component` grows3->4 with one content unload through the public base dispose entry.
Metadata maps the same real derived calls to IGameComponent, IUpdateable and IDrawable. SOFTWARE
and OPENGLES3 Xvfb runs pass3/3,6/6,5/5 and4/4 with byte-identical stabilized per-ID stdout and
empty stderr; all four focused SOFTWARE CTests pass. The first build called a C++ name-hidden
Dispose overload directly and was corrected as a demo bug. The first successful GameComponent
pair printed renderer-dependent total game ticks (10/11); the unasserted diagnostic was replaced
with its stable asserted component count and both runs were repeated. Temporary coverage projects
1639->1660/2326 and297->299/329 types; only CLR GameComponent.Finalize remains in this family by
disposition. Evidence:`/tmp/checkpoint-658-*` and `/tmp/checkpoint-658-coverage.md`. No screenshot
or visual inspection was used. Production library hashes remain complete647; the runner is
`71db4fa0a80c545943805c137d79db38d4ed6b09d49f69507d8df02ed2cf5443`. Sibling CNA remained
external dirty `73991b01f7` with six observed paths; no CNA source was edited.

Focused659 extends two existing graphics-manager IDs. `preparing_device_settings` grows3->8
controls: direct GraphicsDeviceInformation defaults, all mutable properties, deep clone,
typed/boxed equality and hash are exercised alongside explicit event-args construction; a small
derived probe calls protected device selection, ranking and reset suitability against real manager
defaults. `device_events` grows4->6 controls, confirms the manager and Game expose the same device,
then disposes the manager twice and requires exactly one DeviceDisposing/Disposed pair plus a
released device reference. The actual creation/draw/reset/disposal lifecycle credits the protected
callbacks and public/explicit IGraphicsDeviceManager and IDisposable routes. SOFTWARE and
OPENGLES3 Xvfb runs pass8/8 and6/6 with byte-identical per-ID stdout and empty stderr; both focused
SOFTWARE CTests pass. The build is diagnostic-free. Temporary coverage projects1660->1687/2326;
GraphicsDeviceInformation, PreparingDeviceSettingsEventArgs and IGraphicsDeviceManager have no
remaining gap, while GraphicsDeviceManager retains only its two default-size constants and two
preference properties. Evidence:`/tmp/checkpoint-659-*` and `/tmp/checkpoint-659-coverage.md`. No
screenshot or visual inspection was used. Production library hashes remain complete647; the runner
is `e80a8122118ad5f5a99251aa8c6cacf3f5e4a098e1ecc88a174829fd609cdede`. Sibling CNA remained
external dirty `73991b01f7` with six observed paths; no CNA source was edited.

Focused660 extends `game.graphics_device_manager.back_buffer_size` from two to eight controls. It
proves the initial preferred size equals the public 800x480 constants, retains Bgr565/true
preferences, applies the adapter-selected Color/Depth24 format pair with real multisampling, then
uses ApplyChanges to reach an 800x600 Color/zero-sample device. This closes the last two manager
properties and names its last two fields: temporary coverage projects1687->1689/2326 members and
151->153/753 fields, leaving no GraphicsDeviceManager family gap. The comparison also exposed
CNA-DEMOS-ISSUE-083: QueryBackBufferFormat returns zero samples while the same device creation
applies4 on SOFTWARE and8 on OPENGLES3. Both marked runs execute8 controls, fail only that exact
query-agreement check, have byte-identical stdout and empty stderr; the known-issue SOFTWARE CTest
passes. The build is diagnostic-free. Evidence:`/tmp/checkpoint-660-*` and
`/tmp/checkpoint-660-coverage.md`; the initial exact-value probes are
`/tmp/checkpoint-660-diagnostic*`. No screenshot or visual inspection was used. Production library
hashes remain complete647; the runner is
`6ecdbb68ca0ffcbca50510de599b738675632800f28d7c56f44f088e79c8daf8`. Sibling CNA remained
external dirty `73991b01f7` with six observed paths; no CNA source was edited.

Focused661 extends seven existing Game/service/window IDs and adds13 member credits. Method order
now records two exact Update/BeginDraw/Draw/EndDraw cycles, proves the manager's device identity,
and observes one Game.Disposed event after EndRun/UnloadContent. Run-one-frame invokes Tick
directly after three RunOneFrame calls and observes Dispose; the service workflow directly
constructs an empty GameServiceContainer. Actual back-buffer resize, orientation and title paths
credit the corresponding GameWindow protected implementation callbacks. A detached window uses
both EndScreenDeviceChange overloads and observes one screen-name event with no repeat. Five
portable IDs have byte-identical SOFTWARE/OPENGLES3 stdout and empty stderr; back-buffer executes9
controls with only ISSUE-083's expected failure. Orientation6/6 and screen-device3/3 pass on
OPENGLES3; their SOFTWARE runs are declared `Need::Window` skips and emit only the runner's95-byte
skip diagnostic. All seven focused SOFTWARE CTests pass (two skipped). Temporary coverage projects
1689->1702/2326; only Game.Finalize, its unused platform missing-requirement hook, and GameWindow's
three empty/uncalled activation/paint hooks remain by explicit disposition. Evidence:
`/tmp/checkpoint-661-*` and `/tmp/checkpoint-661-coverage.md`. No screenshot, manual interaction or
visual inspection was used.

Sibling CNA advanced externally during the build from dirty `73991b01f7` to committed
`0562550c3` (Video/VideoPlayer lifetime fix), then a coherent rebuild scheduled only the normal
asset copies. Six unrelated FNA3D/renderer plan and patch paths remain dirty; no CNA source was
edited here. The focused runs use runner
`9beab2a88ee159291dade1ec40ca86fb21f92a35350e179f3db2c7dfd859b57c`, libcna.so
`c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and libcna_c_api.so
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`; these production-library
hashes supersede complete647 only for focused661 and are not yet a complete-smoke baseline.

Focused662 extends six existing resource/service IDs and closes12 members. GraphicsResource
lifetime grows7->10 controls with creating-device identity plus unnamed/type and named/debug text.
Readable VertexBuffer grows3->5 with BufferUsage and explicit disposal; the indexed cube grows1->2
with WriteOnly usage and disposal. DynamicIndexBuffer grows2->3 and RenderTarget2D grows1->3: each
uses the explicit CNA notification seam to observe one public ContentLost event/flag, then proves
the next upload or render-target bind clears the flag before real drawing. This does not claim a
physical/context device loss. Device events subscribe through IGraphicsDeviceService itself and
retain all six passing creation/reset/disposal controls. SOFTWARE and OPENGLES3 pass every ID with
byte-identical per-ID stdout and empty stderr; all six SOFTWARE CTests pass. Temporary coverage
projects1702->1714/2326. GraphicsResource retains only CLR Finalize by disposition; VertexBuffer,
IndexBuffer, DynamicIndexBuffer, RenderTarget2D and IGraphicsDeviceService have no member gap.
Evidence:`/tmp/checkpoint-662-*` and `/tmp/checkpoint-662-coverage.md`. No screenshot or visual
inspection was used. Sibling CNA remained external dirty `0562550c3` with six observed paths; no
CNA source was edited. Production library hashes remain focused661; the runner is
`fa6ea3e8a14c65cbf35383db39e93e2a81fb3230648729869b868c2c7b91d4a6`.

Focused663 extends five existing presentation/value IDs and closes7 members.
`presentation_parameters` grows5->7 controls with exact direct-construction defaults and live
GameWindow handle identity. MRT's five controls now prove caller and device bindings retain both
exact 2D targets; the cube workflow grows2->3 with a NegativeZ binding's target/face identity before
real six-face rendering. Display modes grows5->7 with an authored Bgr565 safe rectangle and exact
XNA-order text; viewport properties remains7 controls while its Bounds mutation now exercises a
nonzero Y and all four components. SOFTWARE and OPENGLES3 pass presentation7/7, display7/7,
cube3/3 and viewport7/7; MRT passes5/5 on OPENGLES3 and is a declared SOFTWARE custom-effects
skip. Presentation, display modes and viewport have byte-identical stdout and empty stderr. Cube's
pre-existing informational rasterized-pixel counts remain renderer-specific (431/0 versus447/10),
while all controlled outcomes pass. All five SOFTWARE CTests pass (one skipped). Temporary coverage
projects1714->1721/2326, leaving no gap in PresentationParameters, RenderTargetBinding,
DisplayMode or Viewport. Evidence:`/tmp/checkpoint-663-*` and `/tmp/checkpoint-663-coverage.md`. No
screenshot or visual inspection was used. Sibling CNA remained external dirty `0562550c3` with six
observed paths; no CNA source was edited. Production library hashes remain focused661; the runner
is `ff1e1186080c6d57b161eddc9c9c471ff6e56ba8390353f6d715b601544c62c7`.

Focused664 extends three existing graphics-state IDs. `bound_state_is_immutable` grows4->10
controls: SamplerState joins the live immutability proof, four independent states report disposed,
and each later device/collection bind is rejected with ObjectDisposedException. Blend functions
grows5->6 controls and applies separate alpha equations; exact pixels now also prove reverse-
subtract alpha reaches zero, while Min/Max use the XNA-required One/One factors. The anisotropic
workflow adds one exact AnisotropicClamp preset check before its established real floor draw.
SOFTWARE and OPENGLES3 pass10/10,6/6 and1/1 respectively with byte-identical stdout and empty
stderr; all three focused SOFTWARE CTests pass. Temporary coverage moves1721->1726/2326 and
fields153->154/753, leaving no SamplerState gap and closing Dispose on all four state types. The
initial Min/Max rejection and subtract-alpha mismatch were demo errors corrected from the profile
contract and matching renderer output, not CNA defects. Evidence:`/tmp/checkpoint-664-*` and
`/tmp/checkpoint-664-coverage.md`. No screenshot or visual inspection was used. The build retained
production library hashes `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da`
and `2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`; runner
`7cfb850fb681be30594d3f1ce7634e4e6b3c3c9aadf203d8df7b3d9d4025c69e`. Sibling CNA advanced
externally after runtime validation to `f00150424` with two observed dirty documentation paths; no
CNA source was edited. Finish the12 remaining graphics-state members next.

Focused665 adds `graphics.depth_stencil_state.stencil_outcomes` and extends the existing stencil
mask. The new five-control ID writes four distinct stencil references through actual clockwise and
counter-clockwise stencil-test-fail and depth-test-fail branches, then independently reveals the
four triangle regions as exact colors and keeps an outside pixel clear. The mask ID grows3->4:
stencil starts at binary10, two failure paths replace only bit0, an exact comparison proves stored3
preserved the high bit, and a later read mask of1 makes stored3 match reference1 for the established
checker reveal. Both IDs pass on SOFTWARE and OPENGLES3 with byte-identical stdout and empty
stderr; both focused SOFTWARE CTests pass. Temporary coverage moves1726->1732/2326 and fields154->
155/753 at629 demos, leaving no DepthStencilState gap. Evidence:`/tmp/checkpoint-665-*` and
`/tmp/checkpoint-665-coverage.md`. Pixel validation used programmatic readback; no screenshot or
visual inspection was used. The successful reconfigure/build used external `f00150424`; production
library hashes remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`, runner
`98d1abb28db323c133a2ee348c63050719c872560d375d4ac89eee3d0b0faa69`. The first compile rejected
a non-literal constexpr demo table; changing it to const fixed the demo bug. No CNA source was
edited. Finish the six BlendState/RasterizerState members next.

Focused666 extends multisampling and depth bias without a new ID. The multisample workflow grows
3->5 controls: all-sample rendering resolves to five grey levels, mask0 resolves to one clear
level on both renderers, and disabling RasterizerState multisampling gives SOFTWARE's two all-or-
nothing levels. OPENGLES3 remains at five because OpenGL ES cannot represent that per-state toggle;
the demo records the platform-specific behavior. The depth-bias workflow grows2->3 and proves a
negative SlopeScaleDepthBias brings a sloped white decal in front of its sloped green floor, beside
the established unbiased/fixed-bias controls. Both IDs pass both renderers with empty stderr; both
focused SOFTWARE CTests pass. Depth-bias stdout is identical; multisampling intentionally reports
rasterizer-off levels2 versus5. Temporary coverage moves1732->1735/2326, leaving RasterizerState
complete and only BlendState ColorWriteChannels1/2/3 in the graphics-state family. Evidence:
`/tmp/checkpoint-666-*` and `/tmp/checkpoint-666-coverage.md`. Pixel validation used programmatic
readback; no screenshot or visual inspection was used. Production library hashes remain
`c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`; runner
`e92e45644daa3b499a1b7b7428930f7d543bfd21ac97a5ce51cf30be545cedcb`. Sibling CNA stayed external
dirty `f00150424`; no CNA source was edited. Finish the three active MRT masks next.

Focused667 completes the graphics-state family in the existing MRT ID. Before its established
two-target albedo/normal pass, a four-output probe clears every target to RGBA(10,20,30,40), then
uses distinct masks on active slots1/2/3. Exact one-pixel target readback shows slot1
(128,20,30,40), slot2(10,102,30,40) and slot3(10,20,76,40): respectively only red, green and blue
changed, while every disabled channel retained the clear value. OPENGLES3 passes8/8 with empty
stderr; SOFTWARE retains the declared custom-effects skip and its focused CTest passes as skipped.
Temporary coverage moves1735->1738/2326, leaving BlendState, DepthStencilState, RasterizerState and
SamplerState without generated gaps. Evidence:`/tmp/checkpoint-667-*` and
`/tmp/checkpoint-667-coverage.md`. Pixel validation used programmatic texture/backbuffer readback;
no screenshot or visual inspection was used. Production library hashes remain
`c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`; runner
`acfd685dad3e65cba99c464ae08fcc52f4dea72114e984ec14f32cff4c12e4bc`. Sibling CNA stayed external
dirty `f00150424`; no CNA source was edited. Audit EffectParameter's17 remaining value/reflection
members next.

Focused668 extends `graphics.effect.parameter_values` from16 to25 controls. One grouped storage
workflow now round-trips Boolean scalar/array, signed Int32 and Single arrays, Vector2/Vector3 and
Quaternion scalar/arrays, a Vector4 array, string text and exact Texture3D/TextureCube identities.
An authored Material struct carries semantic MATERIAL0, exposes Tint by case-insensitive COLOR0
lookup and owns a string annotation `UiName=Surface tint`. This covers public CNA-constructed
storage/reflection without claiming compiler-populated FX metadata. Both renderers retain23 passing
controls and the same two documented ISSUE-034/035 failures, with byte-identical stdout and empty
stderr; the focused known-issue SOFTWARE CTest passes. Temporary coverage moves1738->1760/2326 and
299->301/329 types, closing EffectParameter and leaving only CLR enumerators on its collection.
Evidence:`/tmp/checkpoint-668-*` and `/tmp/checkpoint-668-coverage.md`. No drawing, screenshot or
visual inspection was used. Production library hashes remain
`c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`; runner
`cb61dd5ac090d3f139a78bb2bc7dfb2b97c93526699df6dec1ec68ac0be377ab`. Sibling CNA remained
external `f00150424` and gained a third dirty state-test path; no CNA source was edited. Audit the12
remaining non-enumerator EffectAnnotation members next.

Focused669 extends the same ID from25 to28 controls and closes all12 remaining EffectAnnotation
members. CNA-constructed annotations round trip exact Boolean, Int32 and Single scalars, distinct
Vector2/Vector3/Vector4 widths and a non-symmetric Matrix whose column-major fixture would expose a
transpose error. The same values report exact semantic, row/column, class and type metadata. Both
renderers retain26 passing controls plus the same two documented ISSUE-034/035 failures, with byte-
identical stdout and empty stderr; the focused known-issue SOFTWARE CTest passes. Temporary
coverage moves1760->1772/2326 at629 demos,301/329 types and155/753 fields. EffectAnnotation has no
generated gap; its collection retains only CLR enumerators. Evidence:`/tmp/checkpoint-669-*` and
`/tmp/checkpoint-669-coverage.md`. The runner SHA-256 is
`0902178b4d6b6abc343f40d808a1b938cf455ad949f2d5f1b5baabe259dcd3a8`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. No drawing, screenshot or
visual inspection was used. Audit adjacent EffectPass/EffectTechnique reflection next.

After the focused run, sibling CNA remained at `f00150424` while its externally owned dirty set
grew from three to seven paths (four documentation/plan and three FNA3D implementation/test paths).
No CNA source was edited or reverted here; recheck that moving external state before the next build.

Focused670 extends `graphics.effect.parameters_and_techniques` from4 to8 controls. The real
BasicEffect graph reports exact `BasicEffect` and `P0` names; index and exact-name lookup return the
same objects, while case-mismatched names and a negative pass index return null. Separate authored
string annotations remain scoped to the technique and pass. SOFTWARE and OPENGLES3 pass8/8 with
byte-identical stdout and empty stderr, and the focused SOFTWARE CTest passes. Temporary coverage
moves1772->1776/2326 at629 demos,301/329 types and155/753 fields. EffectPass and EffectTechnique
have no generated gap; their collections and the annotation/parameter collections retain only
dispositioned CLR enumerators. Evidence:`/tmp/checkpoint-670-*` and
`/tmp/checkpoint-670-coverage.md`. Runner SHA-256 is
`08cb15659e56a90aeec2e53d0b7f67919aa9e53aaa4a62883d6f3e9669629fa0`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. No drawing, screenshot or
visual inspection was used. Sibling CNA advanced externally to `1ad225ae28` with six dirty
documentation/FNA3D paths; no CNA source was edited here. Audit Effect construction, OnApply and
disposal next.

Focused671 adds `graphics.effect.custom_apply_lifecycle`, a six-control derived-effect workflow.
Direct construction yields the exact empty-parameter `Default`/`P0` graph. Direct Effect::Apply
and current-pass Apply each enter the overridden OnApply once. Dispose marks the object, raises one
Disposing event, makes both apply routes throw before the hook and remains idempotent. SOFTWARE and
OPENGLES3 pass6/6 with byte-identical stdout and empty stderr; the focused SOFTWARE CTest passes.
Temporary coverage moves1776->1779/2326 at630 demos,301/329 types and155/753 fields, leaving no
generated Effect member gap. Evidence:`/tmp/checkpoint-671-*` and
`/tmp/checkpoint-671-coverage.md`. Runner SHA-256 is
`ababcc3e7ad594ab57e984760ef38fd530a9e972142149f101aecf63a718206c`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. The demo does not draw,
take a screenshot or claim visual behavior. Sibling CNA remained external `1ad225ae28` with six
dirty documentation/FNA3D paths; no CNA source was edited. Audit GraphicsDevice next.

Focused672 extends three existing IDs and closes four GraphicsDevice members. Presentation grows
7->9 controls with exact windowed DisplayMode/adapter agreement and Normal device status.
Multisampling grows5->6: the standalone device mask0 produces one clear level on both renderers and
restoring -1 is observed, alongside the retained BlendState mask path. Stencil remains4 controls
but now binds reference1, overrides it to3 through GraphicsDevice in immediate mode and reveals
stored value3 with exact pixels. Presentation and stencil output are byte-identical; MSAA retains
only its documented rasterizer-off 2-versus-5-level difference. All stderr is empty and the three
focused SOFTWARE CTests pass. Temporary coverage moves1779->1783/2326 at630 demos,301/329 types
and155/753 fields. Evidence:`/tmp/checkpoint-672-*` and `/tmp/checkpoint-672-coverage.md`. Runner
SHA-256 is `49a765e90afc56b21fab5b63dbf9dd420baec9b8e535e812409058b1a4bb300f`; production library
hashes remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. Pixel validation used
programmatic readback; no screenshot or visual inspection was used. Sibling CNA remained external
`1ad225ae28` with six dirty documentation/FNA3D paths; no CNA source was edited. Audit vertex-stage
device collections next.

Focused673 adds `graphics.device.vertex_stage_bindings`, a four-control public-state workflow. A
Single Texture2D binds by exact identity in vertex slot1 while pixel slot1 remains null; PointClamp
changes only the vertex sampler while the pixel sampler remains LinearWrap. Color-format
replacement throws atomically, and disposing the floating texture clears its vertex slot.
SOFTWARE and OPENGLES3 pass4/4 with byte-identical stdout and empty stderr; the focused SOFTWARE
CTest passes. Temporary coverage moves1783->1785/2326 at631 demos,301/329 types and156/753 fields.
The demo does not claim compiled vertex-shader sampling. Evidence:`/tmp/checkpoint-673-*` and
`/tmp/checkpoint-673-coverage.md`. Runner SHA-256 is
`0ce5a5581704a1cfd493701be25d63aaf366f5fffc6b929dcc91cb0ec596d8ee`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. No drawing, screenshot or
visual inspection was used. Sibling CNA remained external `1ad225ae28` with six dirty
documentation/FNA3D paths; no CNA source was edited. Finish GraphicsDevice lifecycle next.

Focused674 adds `graphics.device.owned_lifecycle`, a seven-control real second-device workflow. A
HiDef device starts live at256x144, clears/presents, resets to320x180 with exact Resetting -> Reset
event order while retaining an owned texture, then clears/presents again. Dispose marks the device
before its one event, disposes the texture once, rejects later Present/Reset and is idempotent.
SOFTWARE and OPENGLES3 pass7/7 with byte-identical stdout and empty stderr; the focused SOFTWARE
CTest passes. Temporary coverage moves1785->1791/2326 at632 demos,301/329 types and156/753 fields.
GraphicsDevice retains only dispositioned DeviceLost (documented absent on desktop and observed
zero through reset/disposal) and CLR Finalize. Evidence:`/tmp/checkpoint-674-*` and
`/tmp/checkpoint-674-coverage.md`. Runner SHA-256 is
`1f711d0a146cc50cc975e7c84b97a2835217dcc2c607ee2c58004f929a53c8e3`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. Clear/Present executed, but
no pixel, screenshot or visual inspection was used. Sibling CNA remained external `1ad225ae28`
with six dirty documentation/FNA3D paths; no CNA source was edited. Audit stock effects next.

Focused675 extends two existing stock-effect workflows. `effect_interfaces` grows3->5 controls and
reads exact concrete AlphaTestEffect/EnvironmentMapEffect matrix/fog/light state after setting it
through public interfaces and before real fogged draws. Environment reflection grows2->4 and now
clones its fully configured first effect: exact diffuse/ambient, two texture identities and all
three enabled lights survive, while transform/Fresnel mutations remain independent and preserve
the established exact center pixels. SOFTWARE and OPENGLES3 outputs are byte-identical with empty
stderr; both focused SOFTWARE CTests pass. Temporary coverage moves1791->1810/2326 at632 demos,
301/329 types and156/753 fields. IEffectLights has no gap; protected per-effect OnApply entries are
dispositioned as real pass-dispatch hooks already proven by focused671. Evidence:
`/tmp/checkpoint-675-*` and `/tmp/checkpoint-675-coverage.md`. Runner SHA-256 is
`ab2bc040bee473cf2a8518bdb2def27925ede9ba5e410de978bfa25576859acc`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. Pixel validation was
programmatic; no screenshot or visual inspection was used. Sibling CNA remained external
`1ad225ae28` with six dirty documentation/FNA3D paths; no CNA source was edited. Finish
AlphaTestEffect next.

Focused676 extends `graphics.alpha_test_effect.cutout` from2 to5 controls. A fully configured
source is cloned; the clone retains texture, projection, threshold, Alpha=.75 and enabled vertex
colors, then remains independent when the source changes. The rendered colored-vertex path yields
exact RGBA(96,191,191,191) at the opaque center and RGBA(96,124,0,191) where the near corner is
discarded and exposes the far tinted orange circle. SOFTWARE and OPENGLES3 outputs are byte-
identical with empty stderr; the focused SOFTWARE CTest passes. Temporary coverage moves1810->
1813/2326 at632 demos,301/329 types and156/753 fields. AlphaTestEffect retains only dispositioned
protected OnApply. Evidence:`/tmp/checkpoint-676-*` and `/tmp/checkpoint-676-coverage.md`. Runner
SHA-256 is `7d1e918f125963ca6d3061e64d5500597f9f5a7817f1f3a0215f3f257d61a12f`; production library
hashes remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. Pixel validation was
programmatic; no screenshot or visual inspection was used. Sibling CNA remained external
`1ad225ae28` with six dirty documentation/FNA3D paths; no CNA source was edited. Audit
DualTextureEffect next.

Focused677 extends `graphics.dual_texture_effect.light_map` from2 to4 controls and closes all ten
remaining caller-visible DualTextureEffect members. Its clone retains both texture identities,
nonidentity World/View transforms that cancel spatially, material/alpha, vertex-color selection
and enabled black fog from0 to2, then remains independent from three source mutations. Packed cyan
vertex colors, diffuse(1,.5,1), Alpha=.8 and the half-fogged z=-1 quad produce exact brightest
RGBA(102,102,203,204); the dark light-map corner remains RGBA(16,16,32,204).

SOFTWARE and OPENGLES3 pass4/4 with byte-identical stdout and empty stderr; the focused SOFTWARE
CTest passes. The first draft used CNA's non-trivial Color class directly inside a raw vertex and
failed the demo's own layout assertion; replacing it with the documented packed uint32_t was a
demo correction. The first pixel estimate also omitted DualTextureEffect's `2 * base` term; source
review accounted for the measured value before the final check. Temporary coverage moves1813->
1823/2326 at632 demos,301/329 types and156/753 fields. DualTextureEffect retains only protected
OnApply, dispositioned through the real draw and base dispatch workflow. Evidence:
`/tmp/checkpoint-677-*` and `/tmp/checkpoint-677-coverage.md`. Runner SHA-256 is
`f89d962792fa89163ee6cf8462cefa5f9d1f5b5ea018140c087bd919adee950d`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. Pixel validation was
programmatic; no screenshot or visual inspection was used. Sibling CNA advanced externally to
clean `f698d0f548`; no CNA source was edited. Finish EnvironmentMapEffect's Alpha, EmissiveColor and
EnvironmentMapSpecular behavior next.

Focused678 extends `graphics.environment_map_effect.reflection` from4 to5 controls and closes its
last three caller-visible members. The cloned material retains Alpha=.75, red EmissiveColor=.25 and
blue EnvironmentMapSpecular=.2, and remains independent through temporary source mutations. Zero
diffuse isolates the terms in real pixels: the Fresnel0 centre is exact RGBA(0,191,38,191), showing
the alpha-scaled lime reflection plus blue specular, while the facing Fresnel1 centre is exact
RGBA(48,1,38,191), exposing the red emissive surface while retaining specular and alpha.

SOFTWARE and OPENGLES3 pass5/5 with byte-identical stdout and empty stderr; the focused SOFTWARE
CTest passes. Temporary coverage moves1823->1826/2326 at632 demos,301/329 types and156/753 fields.
EnvironmentMapEffect retains only protected OnApply, dispositioned through these real pass draws
and focused671's base dispatch workflow. Evidence:`/tmp/checkpoint-678-*` and
`/tmp/checkpoint-678-coverage.md`. Runner SHA-256 is
`09ca54ad6860d8f666e88b09f7cb9f901cf08429b30666f55174b4d5754c2a71`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. Pixel validation was
programmatic; no screenshot or visual inspection was used. Sibling CNA remained clean external
`f698d0f548`; no CNA source was edited. Audit RenderTargetCube's five remaining public behaviors
next.

Focused679 extends `graphics.render_target.render_target_cube` from3 to7 controls and closes all
five remaining RenderTargetCube members. Its live128x128 cube reports Depth24, zero applied MSAA
and PreserveContents. The explicit CNA notification seam raises one public ContentLost event and
sets IsContentLost; binding the first face clears the flag before rendering, without claiming a
physical device loss. After the first frame, the +Z face is rebound without clear/draw; exact
green centre RGBA(30,110,40,255) survives and is sampled through the sphere on frame2, proving
preservation rather than getter state alone.

SOFTWARE and OPENGLES3 pass7/7 with empty stderr and the focused SOFTWARE CTest passes. The two
new controlled pixel lines match; the retained orbiting-cube diagnostic remains renderer-specific
at431/0 versus445/12. An initial 4x-MSAA draft was stopped after the 30-frame SOFTWARE path exceeded
40 seconds; automation was reduced to the three frames the checks require and the coherent dynamic
cube remains single-sample. Existing dedicated multisampling workflows retain the behavioral MSAA
coverage. Temporary coverage moves1826->1831/2326 at632 demos,301/329 types and156/753 fields.
Evidence:`/tmp/checkpoint-679-*` and `/tmp/checkpoint-679-coverage.md`. Runner SHA-256 is
`4ba9059fab6c3103607581c6db5e7fff1610711cbfaad14e3b7f078b5b1d5a3e`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. Pixel validation was
programmatic; no screenshot or visual inspection was used. Sibling CNA remained clean external
`f698d0f548`; no CNA source was edited. Audit VertexBufferBinding's three value members next.

Focused680 extends two existing IDs and closes all three VertexBufferBinding members. Several
streams retains its three controls but prefixes both position and color buffers with decoys, binds
each at VertexOffset1, reads back both exact buffer identities/offsets/frequency0 and still produces
the exact white centre and clear exterior. Instancing grows1->2 controls: device readback preserves
the exact mesh frequency0 and matrix-stream frequency1 before one call draws100 cubes across both
screen halves.

SOFTWARE and OPENGLES3 pass both IDs with empty stderr; both focused SOFTWARE CTests pass.
Several-streams stdout is byte-identical. The animated instancing diagnostic differs by one covered
pixel (39638/34709 versus39639/34709), while both controlled outcomes pass. Temporary coverage
moves1831->1834/2326 at632 demos,301/329 types and156/753 fields. Evidence:
`/tmp/checkpoint-680-*` and `/tmp/checkpoint-680-coverage.md`. Runner SHA-256 is
`47d7c8ab84c26c333edc3826f115de0fe35fef09dc714c128ccac5bf97c5938a`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. Pixel validation was
programmatic; no screenshot or visual inspection was used. Sibling CNA remained clean external
`f698d0f548`; no CNA source was edited. Audit GraphicsAdapter's seven value/platform members next.

Focused681 extends `graphics.graphics_adapter.display_modes` from7 to14 controls and closes all
seven GraphicsAdapter gaps. Vendor/Device IDs form either a bounded16-bit pair or the documented
zero/unavailable pair; Revision/SubSystemId are exact zero. The default adapter's opaque monitor
identity agrees with Adapters[0] and nonzero identities are unique. A restored process-wide flag
sequence proves hardware/default, Reference->SOFTWARE, and Null->HEADLESS precedence while both
flags remain independently readable.

SOFTWARE and OPENGLES3 pass14/14 with empty stderr; the focused SOFTWARE CTest passes. Environment-
specific stdout is intentional: SOFTWARE observes fallback Default Display800x480/handle0, while
Xvfb OPENGLES3 observes screen1280x1024/handle1. Both report PCI1002:15BF in this environment. No
device was constructed under the alternate selection flags; their CNA renderer mapping is verified
through the public extension and the original flags are restored. Temporary coverage moves1834->
1841/2326 at632 demos,301/329 types and156/753 fields. Evidence:`/tmp/checkpoint-681-*` and
`/tmp/checkpoint-681-coverage.md`. Runner SHA-256 is
`0268447030705bb2bf88da7f52df0f738d82615bea0544562076e5f98c43a63a`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. No drawing, screenshot or
visual inspection was used. Sibling CNA remained clean external `f698d0f548`; no CNA source was
edited. Begin the packed-vector value-member audit next.

Focused682 extends four existing packed-vector conversion IDs and closes all40 value members on
HalfVector2, NormalizedByte2, NormalizedShort2, Rg32 and Short2. Each reconstructs itself through
PackedValue, agrees across ==/!=, typed and boxed Equals, rejects a boxed sibling type, preserves
equal hashes and exact XNA text, then packs through IPackedVector and returns canonical Z=0/W=1.
The retained conversion/clamping checks remain intact.

Half precision and signed normalized pass8/8; wide UNorm and integer formats pass6/6. SOFTWARE and
OPENGLES3 outputs are byte-identical with empty stderr; all four focused SOFTWARE CTests pass.
Temporary coverage moves1841->1881/2326 at632 demos,301/329 types and156/753 fields. Evidence:
`/tmp/checkpoint-682-*` and `/tmp/checkpoint-682-coverage.md`. Runner SHA-256 is
`92d27727f5633116213d80eada189c4f9f595153f3cf4d4f0c8d097190bbb71e`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. No rendering, screenshot or
visual inspection was used. Sibling CNA remained clean external `f698d0f548`; no CNA source was
edited. Continue with the four-component packed-vector value contracts next.

Focused683 extends the same four packed-vector conversion IDs and closes all28 remaining value
members on HalfVector4, NormalizedShort4, Rgba1010102 and Short4. Each reconstructs itself through
PackedValue, agrees across ==/!=, typed and boxed Equals, rejects a boxed different type, preserves
equal hashes and exact expanded or packed-hex text, then packs all four components through
IPackedVector. The retained precision, layout and clamping checks remain intact.

Half precision and signed normalized pass10/10; wide UNorm and integer formats pass8/8. SOFTWARE
and OPENGLES3 outputs are byte-identical with empty stderr; all four focused SOFTWARE CTests pass.
Temporary coverage moves1881->1909/2326 at632 demos,301/329 types and156/753 fields. Evidence:
`/tmp/checkpoint-683-*` and `/tmp/checkpoint-683-coverage.md`. Runner SHA-256 is
`a0c45ab94d7df4e53b200b2abb8cff3e2a5a71d1a89e3133eebac627447acc1c`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. No rendering, screenshot or
visual inspection was used. Sibling CNA remained clean external `f698d0f548`; no CNA source was
edited. Continue with the remaining compact color and scalar packed-vector value contracts next.

Focused684 extends five existing packed-vector conversion IDs and the existing interface ID to
close all52 remaining members in the PackedVector namespace. Alpha8, Bgr565, Bgra4444, Bgra5551,
Byte4, HalfSingle, NormalizedByte4 and Rgba64 now reconstruct exact PackedValue state, agree across
==/!=, typed and boxed Equals, reject a boxed different type, preserve equal hashes and exact text,
and pack through IPackedVector. Alpha8, Bgr565 and HalfSingle additionally prove their interface-only
Vector4 expansion conventions. The interface workflow addresses CNA's real IPackedVectorT<uint16_t>
typed PackedValue get/set, which represents the CLR generic IPackedVector<T>.PackedValue inventory
item. All retained precision, quantization and clamping checks remain intact.

Compact color and half precision pass12/12; integer formats and wide UNorm pass10/10; signed
normalized passes12/12; the interface workflow passes5/5. SOFTWARE and OPENGLES3 outputs are
byte-identical with empty stderr; all six focused SOFTWARE CTests pass. Temporary coverage moves
1909->1961/2326 at632 demos,301/329 types and156/753 fields; PackedVector reaches164/164 members.
Evidence:`/tmp/checkpoint-684-*` and `/tmp/checkpoint-684-coverage.md`. Runner SHA-256 is
`0199b37520950fa0ae976dbcf89fc036030badcda8912f997f30dbc5243e44dd`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. No rendering, screenshot or
visual inspection was used. Sibling CNA remained clean external `f698d0f548`; no CNA source was
edited. Continue with the five remaining PacketReader typed reads next.

Focused685 extends `net.packet.send_and_receive` to close PacketReader's final five member gaps:
ReadDouble, ReadVector2, ReadVector4, ReadQuaternion and ReadMatrix. A real local System Link packet
now carries scalar, Color, vector, quaternion and matrix state in one fixed order. The receiver
requires every value to round-trip exactly and consumes the full141-byte packet. Its established
sender identity, writer rewind and arrival controls remain intact.

SOFTWARE and OPENGLES3 pass6/6 with two automatic local profiles, byte-identical stdout and empty
stderr. The focused SOFTWARE CTest initially failed because the sandbox refused ENet host creation;
the authorized rerun with the same profiles passes. Temporary coverage moves1961->1966/2326 at632
demos,301/329 types and156/753 fields; PacketReader has no remaining member gap. Evidence:
`/tmp/checkpoint-685-*` and `/tmp/checkpoint-685-coverage.md`. Runner SHA-256 is
`70f3159fecf292f169fde38f2a9e97b4fde23492f02f25cafa9efecc9707edcc`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. No screenshot or visual
inspection was used. The passing transport also exposes stale CNA public documentation and a stale
bundled example for the already fixed Color round trip; this is recorded as CNA-DEMOS-ISSUE-084.
Sibling CNA remained clean external `f698d0f548`; no CNA source was edited. Continue with the Design
type-converter member audit next.

Focused686 extends three existing Design type-converter IDs and closes all32 remaining constructor,
ConvertFrom, ConvertTo and CreateInstance members across the13 concrete/base converter types.
Invariant text now round-trips every parser and asserts structural output for non-parsing types;
constructor descriptors rebuild BoundingSphere and allow non-parsing BoundingBox/Ray conversion;
property-grid maps recreate all scalar, vector, geometry and16-element Matrix values. The two
documented protected MathTypeConverter fields are intentionally not credited as direct data API:
parsing/non-parsing converter behavior and ordered properties exercise their results, and a demo
that subclasses only to expose protected storage would add no user-facing behavior.

String round trips pass9/9, properties pass10/10 and instance descriptors pass5/5. SOFTWARE and
OPENGLES3 outputs are byte-identical with empty stderr; all three focused SOFTWARE CTests pass.
Temporary coverage moves1966->1998/2326 at632 demos,301/329 types and156/753 fields; Design reaches
51/51 non-field members. Evidence:`/tmp/checkpoint-686-*` and `/tmp/checkpoint-686-coverage.md`.
Runner SHA-256 is `ffa1221d945b8c4d2236dd9fccb3cc7392bf18735dc98f6341a3039196bafffe`;
production library hashes remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da`
and `2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. No rendering, screenshot
or visual inspection was used. Sibling CNA remained clean external `f698d0f548`; no CNA source was
edited. Continue with ContentSerializerAttribute state and clone behavior next.

Focused687 extends the existing separate-process intermediate serializer and closes all seven
remaining ContentSerializerAttribute members. It first proves every XNA default, sets AllowNull,
CollectionItemName, ElementName, FlattenContent, Optional and SharedResource together, clones the
format and then resets the original's item name/optional state without changing the clone. Its real
format consumption still renames Health and the established descriptor workflow still validates
optional, required, ignored, null and collection serialization behavior.

The child process passes14/14 controls on both SOFTWARE and OPENGLES3 selections with byte-identical
stdout and empty stderr; the focused SOFTWARE CTest passes. Temporary coverage moves1998->2005/2326
at632 demos,301/329 types and156/753 fields; Content reaches38/48 members. Evidence:
`/tmp/checkpoint-687-*` and `/tmp/checkpoint-687-coverage.md`. Runner SHA-256 is
`c5e12075b16be16cdb36ad124949f72758956e955e1d6b26d7c7b076edd414a4`; child SHA-256 is
`921bc3cc247620203732459ca7042500454872304f96b9408fa0ec52f6a5bf64`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. No rendering, screenshot or
visual inspection was used. Sibling CNA remained clean external `f698d0f548`; no CNA source was
edited. Continue the remaining ordinary Content member audit next.

Focused688 extends the two existing OpenStream workflows and closes the last three ordinary Content
members. The custom manager retains the exact IServiceProvider identity, its override constructs a
ContentLoadException with the exact missing packed-asset message, and ResourceContentManager's real
Load/missing-resource paths both dispatch through its protected OpenStream. The first draft tied the
missing-asset control to three total opens, which made it depend on fixing CNA-DEMOS-ISSUE-012; the
corrected control requires one additional call relative to its own starting count.

Both workflows remain marked with CNA-DEMOS-ISSUE-012. Custom OpenStream executes four controls with
three passing and only the established Load bypass failing; resource content executes two controls
with the missing-resource path passing and only the established reader-registration failure. Both
renderer outputs are byte-identical with empty stderr, and the two known-issue SOFTWARE CTests pass
their expected-failure contract. Temporary coverage moves2005->2008/2326 at632 demos,301/329 types
and156/753 fields; Content reaches41/48. Evidence:`/tmp/checkpoint-688-*` and
`/tmp/checkpoint-688-coverage.md`. Runner SHA-256 is
`96594066b1de36e1ee994df00fcc33ac9589aa71abd9ecdba00811c819423135`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. No screenshot or visual
inspection was used. ContentSerializerCollectionItemNameAttribute, ContentSerializerIgnoreAttribute,
ContentSerializerRuntimeTypeAttribute and ContentSerializerTypeVersionAttribute are nominal .NET
attribute wrappers: existing descriptor workflows prove collection naming, ignore, runtime type and
version semantics through CNA's actual C++ registration API. Constructing wrappers that no C++
consumer reads would add count without behavior, so their seven members remain intentionally
uncredited. Sibling CNA remained clean external `f698d0f548`; no CNA source was edited. Continue
with Mouse.WindowHandle next.

Focused689 extends `platform.window.native_handle` and closes Input's final member gap. With a real
X11 window, Mouse.WindowHandle must equal GameWindow.Handle. The workflow clears the static target,
observes zero, restores the real token and observes the exact original again before continuing the
typed native-window checks. Restoring the real target preserves the process-wide input state for
the remainder of the game.

SOFTWARE is correctly skipped before checks because that renderer has no window; its expected skip
diagnostic is the only stderr. OPENGLES3/X11 passes6/6 with empty stderr. The focused SOFTWARE CTest
passes its skip contract. Temporary coverage moves2008->2009/2326 at632 demos,301/329 types and
156/753 fields; Input reaches113/113 members. Evidence:`/tmp/checkpoint-689-*` and
`/tmp/checkpoint-689-coverage.md`. Runner SHA-256 is
`0bbde05b264739003f9f0c1f06c6458244c318291ecb174507caf5a29ff3584b`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. No screenshot or visual
inspection was used. Sibling CNA remained clean external `f698d0f548`; no CNA source was edited.
Continue with Media object/collection lifetime behavior next.

Focused690 extends the existing private-XDG `media.media_library.music_index` workflow with the
three distinct CNA Media disposal layers. Album, Artist, Genre and Playlist begin live; repeated
Dispose marks only the selected borrowed view and retains its relationships. The five top-level
music collections remain independent, then repeated Dispose marks and clears each non-owning view
without deleting saved object pointers. Repeated MediaLibrary.Dispose marks a separately indexed
library without cascading into its live collection/object graph.

SOFTWARE and OPENGLES3 selections each pass29/29 authoritative child controls with byte-identical
stdout and the same six reviewed TitleContainer INFO lines on stderr. The focused SOFTWARE CTest
passes. Temporary coverage moves2009->2028/2326 at632 demos,301/329 types and156/753 fields; Media
moves150->169/192. Evidence:`/tmp/checkpoint-690-music-*` and
`/tmp/checkpoint-690-coverage.md`. Runner SHA-256 is
`41e51d8daa2a24f14a3f5022ceaa158e18d199ac33ece22cf10d021e1b1bf408`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. No playback, screenshot,
listening or visual result is claimed. Sibling CNA remained clean external `f698d0f548`; no CNA
source was edited. Continue picture-object and picture-collection lifetime in `save_picture` next.

Focused691 extends the existing private-XDG `media.media_library.save_picture` workflow. Picture and
PictureAlbum begin live; repeated Dispose marks only the selected borrowed views, preserves their
relationship and leaves sibling pictures live. The reopened top-level PictureCollection and root
PictureAlbumCollection begin live; repeated Dispose marks and empties those non-owning views while
their saved Picture and PictureAlbum objects remain readable.

SOFTWARE and OPENGLES3 selections are byte-identical:23/24 controls pass and only the established
CNA-DEMOS-ISSUE-041 short-read control fails after SavePicture consumes13/1666 bytes. Both stderr
logs contain the same two reviewed TitleContainer INFO lines. The focused SOFTWARE CTest passes its
expected-failure contract. Temporary coverage moves2028->2036/2326 at632 demos,301/329 types and
156/753 fields; Media moves169->177/192. Evidence:`/tmp/checkpoint-691-picture-*` and
`/tmp/checkpoint-691-coverage.md`. Runner SHA-256 is
`56a5b9f50ef63c9878f96695db21999c7df8a397aedbf49917b17c1a86424ea7`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. No screenshot or visual
result is claimed. Sibling CNA remained clean external `f698d0f548`; no CNA source was edited.
Continue with VisualizationData construction and collection-enumerator disposition next.

Focused692 extends `media.media_player.visualization` with the final ordinary Media member. Before
enabling playback, the workflow requires a newly constructed VisualizationData to expose exactly
two256-value arrays with every frequency and sample zero. It then retains the established positive
spectrum check after MediaPlayer fills the same object. The remaining14 Media inventory entries are
generic/non-generic CLR GetEnumerator pairs for seven collections; actual C++ range iteration is
already used, so they are explicitly dispositioned without misleading API metadata.

SOFTWARE and OPENGLES3/X11 pass2/2 with byte-identical stdout and empty stderr. The focused SOFTWARE
CTest passes. Temporary coverage moves2036->2037/2326 at632 demos,301/329 types and156/753 fields;
Media reaches178/192 ordinary credited members with only the14 dispositioned enumerator artifacts.
Evidence:`/tmp/checkpoint-692-visualization-*` and `/tmp/checkpoint-692-coverage.md`. Runner SHA-256
is `49c1c0a40b19a7d4ecaf63bfefe3f73a8488d1a32bc7de898f43fad0f25b1cb4`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. Spectrum values were checked
programmatically; no listening, screenshot or visual-quality result is claimed. Sibling CNA
remained clean external `f698d0f548`; no CNA source was edited. Continue with
TouchPanel.WindowHandle next.

Focused693 extends `platform.window.native_handle` with Input.Touch's final ordinary member. It
writes the live GameWindow token to TouchPanel.WindowHandle, observes exact identity, clears it to
zero and restores the previously observed process-wide value. Unlike Mouse, CNA does not
automatically attach TouchPanel to the presenting window; the demo therefore proves the public
opaque property contract without claiming touch delivery or changing the caller's prior state.

SOFTWARE is correctly skipped before controls because it exposes no window; its expected diagnostic
is the only stderr. OPENGLES3/X11 passes7/7 with empty stderr, and the focused SOFTWARE CTest passes
its skip contract. Temporary coverage moves2037->2038/2326 at632 demos,301/329 types and156/753
fields; Input.Touch reaches42/48 with only six dispositioned CLR enumerator artifacts. Evidence:
`/tmp/checkpoint-693-touch-window-*` and `/tmp/checkpoint-693-coverage.md`. Runner SHA-256 is
`4cb3144c6e67e1d3b26a076845e4637729805b6105a04e8eb9eb31e8701f5459`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. No physical touch,
screenshot or visual result is claimed. Sibling CNA remained clean external `f698d0f548`; no CNA
source was edited. Audit the remaining meaningful ordinary Graphics members next.

Focused694 closes Graphics' last three ordinary gaps across existing real workflows. A standalone
DirectionalLight must start disabled with Down direction, One diffuse and Zero specular values.
The custom20-byte vertex layout now also disposes a separate declaration twice and requires
ObjectDisposedException when a VertexBuffer tries to bind it. OcclusionQuery already ended and
disposed a real renderer query, verified IsDisposed/HasRenderer and refused later Begin/PixelCount;
its metadata now names the concrete Dispose member instead of crediting only GraphicsResource.

Default lighting passes5/5, occlusion query11/11 and custom vertex8/8 on both renderers; all six
stderr logs are empty. Default-lighting and custom-vertex stdout is byte-identical; occlusion output
retains the expected precise SOFTWARE count versus OPENGLES3 boolean result. All three focused
SOFTWARE CTests pass. Temporary coverage moves2038->2041/2326 at632 demos,301/329 types and156/753
fields; Graphics moves491->494/550. The remaining56 Graphics entries are explicitly dispositioned
CLR/override/interface/platform/toolchain artifacts. Evidence:`/tmp/checkpoint-694-*` and
`/tmp/checkpoint-694-coverage.md`. Runner SHA-256 is
`b404136297e90c458cfd35a8082d72a41dfe0bb2e6db389db38aa28b6c44c117`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. Pixel checks used
programmatic readback; no screenshot or visual inspection was performed. Sibling CNA remained clean
external `f698d0f548`; no CNA source was edited. Storage's three generated gaps are dispositioned by
source audit; continue AvailableNetworkSession and collection lifetime next.

Focused695 extends the two-process System Link find/join workflow with the last ordinary available-
session members. The host reserves one private slot, leaves one public slot and advertises two sparse
session values. The joiner observes both capacities and exact values, joins from a live result
collection, disposes that collection twice and proves its retained copied listing now refuses Join
with ObjectDisposedException while the joined session remains usable.

SOFTWARE and OPENGLES3/X11 both pass all12 host/joiner controls with empty stderr. Their only stdout
difference is nondeterministic ordering of concurrent host-left and joiner-left log lines. The
focused SOFTWARE CTest passes; metadata validation is632/0 and the five validator self-tests pass.
Temporary coverage moves2041->2044/2326 at632 demos,301/329 types and156/753 fields; Net moves
76->79/127. Evidence:`/tmp/checkpoint-695-find-*` and `/tmp/checkpoint-695-coverage.md`. Runner
SHA-256 is `3cc513ec1930448c93b86b23fc4e164131a4f7bef3b7da4c064d6835ac729561`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. This is Linux local UDP
discovery/session evidence; no second physical machine, screenshot or visual result is claimed.
Sibling CNA remained clean external `f698d0f548`; no CNA source was edited. Continue with remote
NetworkGamer departure/history/session/round-trip behavior next.

Focused696 extends the same real System Link leave with the remote-gamer lifecycle. On join, the
host requires the remote gamer to be live and bound to that exact NetworkSession. CNA removes it
from the live roster before GamerLeft; the handler then requires the same object to report
HasLeftSession, remain session-bound, appear as the sole PreviousGamers entry and expose a
non-negative retained transport RoundtripTime. Zero remains valid on a fast local path or before a
sample; no positive-latency claim is made.

SOFTWARE and OPENGLES3/X11 pass all16 host/joiner controls with byte-identical stdout and empty
stderr. The focused SOFTWARE CTest passes; metadata validation is632/0 and all five validator
self-tests pass. Temporary coverage moves2044->2048/2326 at632 demos,301/329 types and156/753
fields; Net moves79->83/127. Evidence:`/tmp/checkpoint-696-find-*` and
`/tmp/checkpoint-696-coverage.md`. Runner SHA-256 is
`a77d0fbcdac607990069e5495719e5ac1f155e5aad87fa96d369c038455b0259`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. This is Linux local UDP
evidence; no second physical machine, screenshot or visual result is claimed. Sibling CNA remained
clean external `f698d0f548`; no CNA source was edited. Continue with session bandwidth counters in
an existing packet workflow next.

Focused697 extends the existing two-process simulated-conditions workflow with the two session
traffic-rate properties. After every host Update it retains the maximum published sent and received
rates across the baseline, delayed and outage/recovery phases; both must become positive while real
packet bursts cross the ENet connection. No exact byte count or scheduler-dependent rate is assumed.

SOFTWARE and OPENGLES3/X11 both pass the new bandwidth control and the four existing transport
controls; only the established reliable-outage check fails for CNA-DEMOS-ISSUE-030. Both stderr
logs are empty and the focused SOFTWARE CTest passes its expected-failure contract. Metadata is
632/0, the validator self-tests pass and temporary coverage moves2048->2050/2326 at632 demos,
301/329 types and156/753 fields; Net moves83->85/127. Evidence:`/tmp/checkpoint-697-bandwidth-*`
and `/tmp/checkpoint-697-coverage.md`. Runner SHA-256 is
`714eb2bfa1766b84f8416fc8f21fe1d860b407529b72fadb22eea33259db8d2d`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. This is Linux loopback
traffic; no WAN throughput, screenshot or visual result is claimed. Sibling CNA remained clean
external `f698d0f548`; no CNA source was edited. Continue with ready-state reset next.

Focused698 extends `net.network_session.game_flow` with the remaining ordinary ready-state member.
After one local player becomes ready, the host calls ResetReady in Lobby and requires both players
and IsEveryoneReady to clear. Both players become ready again and start the round; a ResetReady call
in Playing must throw InvalidOperationException without changing either ready flag, after which the
existing EndGame transition clears them normally.

SOFTWARE and OPENGLES3/X11 pass8/8 with byte-identical stdout and empty stderr. The focused SOFTWARE
CTest passes; metadata is632/0 and all five validator self-tests pass. Temporary coverage moves
2050->2051/2326 at632 demos,301/329 types and156/753 fields; Net moves85->86/127. Evidence:
`/tmp/checkpoint-698-reset-ready-*` and `/tmp/checkpoint-698-coverage.md`. Runner SHA-256 is
`45b16dbe61b1c4ae4de6e46a211eddade60918490359077173c164996e6f9b96`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. State was checked
programmatically; no screenshot or visual result is claimed. Sibling CNA remained clean external
`f698d0f548`; no CNA source was edited. Classify the remaining Net inventory next.

Focused699 converts the successful discovery and connection in
`net.network_session.find_and_join` to the real BeginFind/EndFind and BeginJoin/EndJoin pairs. Each
Begin result is synchronously complete for CNA's System Link path, invokes its callback with the
exact supplied state and is consumed once by End; EndFind performs the LAN search and EndJoin
performs the blocking handshake. The disposed-listing control still calls synchronous Join and
retains its ObjectDisposedException evidence. Discovery also requires positive upstream and
downstream QoS estimates from the two probe trains.

The first focused run exposed a demo-only nondeterminism: BeginFind was checked on every retry
before discovery returned the host. The final workflow checks the token contract exactly once while
still retrying the search. SOFTWARE and OPENGLES3/X11 then pass all19 host/joiner controls with
byte-identical stdout and empty stderr. The focused SOFTWARE CTest passes; metadata is632/0 and all
five validator self-tests pass. Temporary coverage moves2051->2057/2326 at632 demos,301/329 types
and156/753 fields; Net moves86->92/127. Evidence:`/tmp/checkpoint-699-async-find-join-*` and
`/tmp/checkpoint-699-coverage.md`. Runner SHA-256 is
`1aebfef00e54d69be070398e927e0198b488be38eca5b0087d5b8b41dd8a3b7a`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. This is Linux local UDP
evidence; no service-directory, WAN, screenshot or visual result is claimed. Sibling CNA remained
clean external `f698d0f548`; no CNA source was edited. Continue NetworkSession disposal next.

Focused700 extends `net.network_session.local_session`, which already owns and disposes the real
two-player Local session. Creation now requires IsDisposed=false. After the existing gamer, event
and machine checks, Dispose is called twice while the object remains alive; it must publish
IsDisposed=true, clear Host and all four public gamer views, and make a later Update throw
ObjectDisposedException. Unload safely repeats disposal before deleting the caller-owned object.

SOFTWARE and OPENGLES3/X11 pass10/10 with byte-identical stdout and empty stderr. The focused
SOFTWARE CTest passes; metadata is632/0 and all five validator self-tests pass. Temporary coverage
moves2057->2058/2326 at632 demos,301/329 types and156/753 fields; Net moves92->93/127. Evidence:
`/tmp/checkpoint-700-session-dispose-*` and `/tmp/checkpoint-700-coverage.md`. Runner SHA-256 is
`a69aa5be48992b641eece5ccac3356a6daa16efee1e0ed1de541310f029def3d`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. No network, screenshot or
visual result is claimed. Sibling CNA remained clean external `f698d0f548`; no CNA source was
edited. Continue the NetworkSessionProperties fixed-shape collection contract next.

Focused701 extends `net.network_session.session_properties` without adding a variant ID. Its
authored two values and six empty slots now drive value/null Contains and IndexOf, exact eight-item
C++ range iteration, atomic short CopyTo refusal and successful offset CopyTo with preserved
sentinels. Add, Insert, Remove, RemoveAt and Clear must all throw NotSupportedException and leave the
fixed collection unchanged; IsReadOnly remains false because indexed values are writable even
though its shape is fixed. The original create/live-map/out-of-range behavior remains intact.

The first build found a demo-only local name collision between the structural-refusal helper and
the existing index-refusal boolean; renaming them fixed compilation without changing CNA. SOFTWARE
and OPENGLES3/X11 pass8/8 with byte-identical stdout and empty stderr. The focused SOFTWARE CTest,
metadata validation and five validator self-tests pass. Temporary coverage intentionally remains
2058/2326 at632 demos,301/329 types and156/753 fields; the XNA inventory's ten explicit-interface
and two GetEnumerator spellings have no distinct CNA C++ entry points, so they stay dispositioned
instead of receiving synthetic metadata. Evidence:`/tmp/checkpoint-701-session-properties-*` and
`/tmp/checkpoint-701-coverage.md`. Runner SHA-256 is
`acc273453f8c6ac1c5e290cefcf5e11b12793ebb32ff0927606728195a6b2364`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. No network, screenshot or
visual result is claimed. Sibling CNA remained clean external `f698d0f548`; no CNA source was
edited. Continue with LocalWithLeaderboards in the session-type workflow next.

Focused702 extends `net.network_session.online_session_types` without adding a variant ID. Alongside
the established offline Local success and PlayerMatch/Ranked service refusals, the local profile now
creates and disposes a LocalWithLeaderboards session. The returned session must retain that exact
type and its one local gamer, demonstrating the single-machine branch without implying online
matchmaking. The existing leaderboard demos separately exercise gameplay and score behavior for
this type.

SOFTWARE and OPENGLES3/X11 pass5/5 with byte-identical stdout and empty stderr. The focused SOFTWARE
CTest, metadata validation and five validator self-tests pass. Temporary coverage remains2058/2326
at632 demos and301/329 types while field coverage moves156->157/753 and Net fields7->8/22 through
the exact enum value. Evidence:`/tmp/checkpoint-702-session-types-*` and
`/tmp/checkpoint-702-coverage.md`. Runner SHA-256 is
`63d3a5ceafc6b93d30488ee85edf35f0a63c442b0aeab4e0a6c5d54e1a34175b`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. No service, network,
leaderboard write, screenshot or visual result is claimed. Sibling CNA remained clean external
`f698d0f548`; no CNA source was edited. Continue with the ordinary GamerServices member audit,
starting in the existing leaderboard workflows.

Focused703 extends `gamer_services.leaderboard.paging` without adding a variant ID. The initial
offline page now comes from BeginRead/EndRead; its token completes synchronously, calls back once
with exact caller state and retains the requested LeaderboardIdentity Key/GameMode. PageDown and
PageUp keep their original synchronous traversal and additionally execute both Begin/End pairs with
the same callback contract. The reader is disposed twice and reports disposed after its real
four-player, two-page workflow.

The first two direct SOFTWARE attempts failed before controls because only XDG_CONFIG_HOME had been
redirected while StorageDevice roots use XDG_DATA_HOME. Fresh isolated writable data roots fixed
the environment error; this was not a demo or CNA defect. SOFTWARE and OPENGLES3/X11 then pass13/13
with byte-identical stdout and empty stderr. The focused SOFTWARE CTest, metadata validation and five
validator self-tests pass. Temporary coverage moves2058->2069/2326 at632 demos,301/329 types and
157/753 fields; GamerServices moves116->127/244. Evidence:
`/tmp/checkpoint-703-leaderboard-reader-*` and `/tmp/checkpoint-703-coverage.md`. Runner SHA-256 is
`c452a7a94ccf99fa271b7ad0032370c1c7f91fdcae48118193745961af66f555`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. No service, network,
screenshot or visual result is claimed. Sibling CNA remained clean external `f698d0f548`; no CNA
source was edited. Continue with PropertyDictionary's meaningful typed and collection surface.

Focused704 extends the same paging workflow with eight real column types plus dictionary lookup.
Every writer row stores Outcome, TimeSpan, int32, int64, float, double, DateTime and string values;
the reader proves all values through their typed getters. Count, ContainsKey, TryGetValue, Item and
C++ range traversal are checked on the live writer/read dictionaries, including an absent lookup
that preserves its output.

The first run escaped `map::at`; investigation showed this was not the predicted persistence-only
failure. Offline LeaderboardWriter entries never enable PropertyDictionary's create-on-first-use
stream mode, so GetValueStream on a new `Replay` key throws `std::out_of_range`. The demo now catches
that exact refusal, continues every positive control and exposes one expected
CNA-DEMOS-ISSUE-085 assertion; source review also finds that the offline JSON serializer skips
Stream pointers even if creation were enabled. SOFTWARE and OPENGLES3/X11 each execute17 controls,
pass16 and fail only ISSUE-085 with byte-identical stdout and empty stderr. The focused known-issue
SOFTWARE CTest, metadata validation and five validator self-tests pass. Temporary coverage moves
2069->2080/2326 at632 demos,301/329 types and157/753 fields; GamerServices moves127->138/244.
Evidence:`/tmp/checkpoint-704-property-dictionary-*` and `/tmp/checkpoint-704-coverage.md`. Runner
SHA-256 is `96eed51f1a99650fae18c0c746b15165aec0a130f2a548a8aa103dec1b5e2285`; production library
hashes remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. No service, network,
screenshot or visual result is claimed. Sibling CNA remained clean external `f698d0f548`; no CNA
source was edited. Continue with the dictionary's C++ collection/mutation contract, leaving CLR
aliases uncredited.

Focused705 completes that dictionary audit on a copy of the eight actually persisted columns.
Keys and Values return matching sorted snapshots. CopyTo rejects an undersized shifted destination
without changing its sentinels, then copies all pairs at offset one while preserving both outer
sentinels. Pair Add/Contains/Remove distinguish value mismatches, duplicate Add refuses without
replacement, key Remove reports present/absent exactly and Clear empties the copy. IsReadOnly stays
true as CNA/FNA document even though those explicit mutators work.

SOFTWARE and OPENGLES3/X11 each execute20 controls, pass19 and fail only the established ISSUE-085
stream check with byte-identical stdout and empty stderr. The focused known-issue SOFTWARE CTest,
metadata validation and five validator self-tests pass. Temporary coverage intentionally stays
2080/2326 at632 demos,301/329 types and157/753 fields: the inventory's five ICollection operations,
IsReadOnly and two GetEnumerator spellings have no distinct CNA C++ entry points, so the demo names
actual public C++ members without fabricating CLR metadata. Evidence:
`/tmp/checkpoint-705-property-collection-*` and `/tmp/checkpoint-705-coverage.md`. Runner SHA-256 is
`3604ed12093f204414012e8a44aa98551a85be82b7419ed7c722e16a30363bf0`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. No service, network,
screenshot or visual result is claimed. Sibling CNA remained clean external `f698d0f548`; no CNA
source was edited. Continue the ordinary GamerServices audit with Gamer/GamerProfile identity and
lifetime.

Focused706 extends `gamer_services.gamer_profile.get_profile` without adding a variant ID. The live
signed-in Gamer starts undisposed and ToString equals both its display name and local gamertag. Two
independent profiles then support the existing sync/async value comparison; disposing the first
twice sets IsDisposed and makes GetGamerPicture throw ObjectDisposedException while the second
profile stays live and still returns its valid null-picture result.

SOFTWARE and OPENGLES3/X11 pass8/8 with byte-identical stdout and empty stderr. The focused SOFTWARE
CTest, metadata validation and five validator self-tests pass. Temporary coverage moves
2080->2084/2326 at632 demos,301/329 types and157/753 fields; GamerServices moves138->142/244.
Evidence:`/tmp/checkpoint-706-gamer-profile-*` and `/tmp/checkpoint-706-coverage.md`. Runner SHA-256
is `46592f1d95e2f6698ce0c885a51ae9144dc0f448396841f698927c8ec5ef3dd8`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. No service, network,
screenshot, profile picture or visual result is claimed. Sibling CNA remained clean external
`f698d0f548`; no CNA source was edited. Continue with AchievementCollection populated indexing,
copy and lifetime behavior.

Focused707 extends `gamer_services.achievement.award` without adding a variant ID. Its real
three-item catalog now exercises integer/key Item lookup and exact bounds exceptions, atomic short
CopyTo refusal plus successful offset copy, Contains and IndexOf. A copied collection proves
Insert/RemoveAt/Add/Remove/Clear ordering while IsReadOnly remains true. Disposing the live result
twice clears only that snapshot and leaves the earlier independent snapshot live.

SOFTWARE and OPENGLES3/X11 each execute10 controls, pass9 and fail only the established ISSUE-017
EarnedOnline control with byte-identical stdout and empty stderr. The focused known-issue SOFTWARE
CTest, metadata validation and five validator self-tests pass. Temporary coverage moves
2084->2087/2326 at632 demos,301/329 types and157/753 fields; GamerServices moves142->145/244. The
remaining AchievementCollection inventory names are nine explicit ICollection/IList operations and
two GetEnumerator forms; actual C++ behavior is covered without synthetic CLR metadata. Evidence:
`/tmp/checkpoint-707-achievement-collection-*` and `/tmp/checkpoint-707-coverage.md`. Runner SHA-256
is `3a2bfb46119a6eec82f9c4a50887c658ee29c7d0016a41712887b548c5ae74e4`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. No service, network,
screenshot or visual result is claimed. Sibling CNA remained clean external `f698d0f548`; no CNA
source was edited. Continue the same workflow with SignedInGamer's Begin/End achievement pairs.

Focused708 retains the synchronous first achievement award and later GetAchievements calls while
moving the second real award and the collection used for paging checks through
BeginAwardAchievement/EndAwardAchievement and BeginGetAchievements/EndGetAchievements. Both
offline tokens complete synchronously, call back exactly once and preserve their string/integer
caller state before End consumes the result.

SOFTWARE and OPENGLES3/X11 each execute12 controls, pass11 and fail only the established ISSUE-017
EarnedOnline control with byte-identical stdout and empty stderr. The focused known-issue SOFTWARE
CTest, metadata validation and five validator self-tests pass. Temporary coverage moves
2087->2091/2326 at632 demos,301/329 types and157/753 fields; GamerServices moves145->149/244.
Evidence:`/tmp/checkpoint-708-achievement-async-*` and `/tmp/checkpoint-708-coverage.md`. Runner
SHA-256 is `7b066659b012377c140a63555339a2da79daaaaf8eeee1bc247a820946d467c5`; production library hashes
remain `c7f0cf685edb229058ea24f121e227a3f530c8d4a591d2d2a932a4510e0c09da` and
`2165091725942665d606a36c27483b4f8cb84b6086b2bbd4fbfd4ebf2cfb8b2c`. No service, network,
screenshot or visual result is claimed. Sibling CNA remained clean external `f698d0f548`; no CNA
source was edited. Continue with AvatarAnimation/AvatarRenderer value and lifetime members.

Focused709 extends `gamer_services.avatar.animation_presets` without adding a variant ID. An
independent Celebrate clip sampled at one second exposes all71 local BoneTransforms, contains real
motion and returns the exact keyed laughing mouth/eyes plus raised eyebrows. The same object starts
live, tolerates repeated Dispose, publishes IsDisposed and rejects Update afterwards. The six
rendered preset instances remain independent and unchanged. CLR Finalize is intentionally
dispositioned as native C++ destruction rather than credited through synthetic metadata.

SOFTWARE and OPENGLES3/X11 pass9/9 with byte-identical stdout and empty stderr. The focused SOFTWARE
CTest, metadata validation and five validator self-tests pass. Temporary coverage moves
2091->2095/2326 at632 demos,301/329 types and157/753 fields; GamerServices moves149->153/244.
Evidence:`/tmp/checkpoint-709-avatar-animation-current-*` and
`/tmp/checkpoint-709-avatar-animation-current-coverage.md`. Runner SHA-256 is
`17da596e0c9e18dc93cb909f3b91638087b9de39b0ff0f5b3d079c98a5f89dd5`; production library hashes
are `3b45f580763e35ef708dfa18db757ef38144bd9715b05143f2c6cdb98734bd67` and
`aef70555a7a5b2f3b1b531d04d27e8f6847870a697fbb0c552719d44c666726c`. No service, network,
screenshot or manual visual result is claimed. CNA advanced externally to committed `6549c623e`
before the full documented build. Unrelated graphics/sampler edits then appeared after their CNA
objects had compiled; no CNA source was edited or reverted here. Continue with AvatarRenderer
lighting values and lifetime behavior.

Focused710 extends `gamer_services.avatar.render` without adding a variant ID. The live renderer
round-trips authored LightColor, LightDirection and AmbientLightColor values. Once Ready has been
observed during Update, the same Wave frame is captured with the authored lights, with both colors
zeroed and with the key direction reversed. Both color removal and direction reversal change
thousands of sampled avatar pixels on both renderers, with wide margins over the deterministic
thresholds. At EndRun the ready renderer starts live, tolerates repeated Dispose, publishes
IsDisposed and rejects both State and Draw afterwards. CLR Finalize remains intentionally
dispositioned as native C++ destruction.

The full documented build completed after CNA advanced externally to committed `67585f9de`.
SOFTWARE and OPENGLES3/X11 pass7/7 with empty stderr; stdout differs only in random-avatar height
and the resulting pixel counts. The focused SOFTWARE CTest, metadata validation and five validator
self-tests pass. Temporary coverage moves2095->2100/2326 at632 demos,301/329 types and157/753
fields; GamerServices moves153->158/244. Evidence:
`/tmp/checkpoint-710-avatar-renderer-final-*` and
`/tmp/checkpoint-710-avatar-renderer-final-coverage.md`. Runner SHA-256 is
`a465426a582a961e4c9615d4d36c677458037fe905ad53b31868bce4319bb5be`; production library hashes
are `54ef50bab694f57a11a88e232495dec3be4fd03ecdf2b63b11e95da5011d4913` and
`f17029e218669c79e51a7bec15cab6eb23bd8f0e4398e5ae2380eb5731f53fd6`. No screenshot or manual
visual-quality result is claimed. Unrelated external SDL-GPU/WebGPU edits appeared only after the
affected objects had compiled; no CNA source was edited or reverted here. Continue with
AvatarDescription BeginGetFromGamer/EndGetFromGamer and Changed behavior.

Focused711 extends `gamer_services.signed_in_gamer.automatic_sign_in` without adding a variant ID.
After the existing first-Update sign-in checks, the real local `SignedInGamer` is passed through
AvatarDescription::BeginGetFromGamer/EndGetFromGamer. The offline token completes synchronously,
invokes its callback exactly once, preserves string caller state and returns the profile's valid,
measured 1021-byte stored avatar. `AvatarDescription::Changed` is intentionally not credited: its
public production triggers are a service avatar revision or a player's manual save in the Guide
avatar editor; calling the event's generic public Raise or an internal profile setter would not
demonstrate that contract.

The full documented build completed after CNA advanced externally to committed `192c2e629`.
SOFTWARE and OPENGLES3/X11 pass8/8 with byte-identical stdout and empty stderr. The focused SOFTWARE
CTest, metadata validation and five validator self-tests pass. Temporary coverage moves
2100->2102/2326 at632 demos,301/329 types and157/753 fields; GamerServices moves158->160/244.
Evidence:`/tmp/checkpoint-711-avatar-description-*` and
`/tmp/checkpoint-711-avatar-description-coverage.md`. Runner SHA-256 is
`7a83f130295cc8fd3161c74e9de397f6c4ab87584306dc67075c21d0c25e6318`; production library hashes
are `e2c8c9dbd6ef8fb4e611a6f0818ebec6290c34d738930c3e9f1dbb4a55ba9aeb` and
`471f0a12b6b167262d18ae8fb763f4c10b07e7bc478f72f0bbfd5a2879b193ef`. No service avatar,
editor interaction, Changed event, screenshot or manual visual result is claimed. Unrelated external
renderer/docs edits appeared after the build; no CNA source was edited or reverted here. Continue
with Achievement::GetPicture using a real offline catalog picture.

Focused712 extends `gamer_services.achievement.award` without adding a variant ID. The real
`first_steps` catalog entry now names a valid 70-byte PNG. GetPicture returns it as a caller-owned
stream at position zero with the exact length and PNG signature; a second call produces a distinct
fresh stream. A copied `hidden_door` achievement with no picture rejects GetPicture with
GamerServicesNotAvailableException. The existing award, collection and async behavior remains.

SOFTWARE and OPENGLES3/X11 each execute14 controls, pass13 and fail only established ISSUE-017,
with byte-identical stdout and empty stderr. The focused known-issue SOFTWARE CTest, metadata
validation and five validator self-tests pass. Temporary coverage moves2102->2103/2326 at632
demos,301/329 types and157/753 fields; GamerServices moves160->161/244. Evidence:
`/tmp/checkpoint-712-achievement-picture-focused-*` and
`/tmp/checkpoint-712-achievement-picture-coverage.md`. Runner SHA-256 is
`84aa135a95784b1c2a87d794ff38bdf8514cdba9b019eb13da4a0a57ad04a88f`; production library hashes
remain `e2c8c9dbd6ef8fb4e611a6f0818ebec6290c34d738930c3e9f1dbb4a55ba9aeb` and
`471f0a12b6b167262d18ae8fb763f4c10b07e7bc478f72f0bbfd5a2879b193ef`. The exact demo object was
compiled and the runner relinked against the focused711 production libraries; a full project build
was deliberately deferred while sibling CNA carried unrelated uncommitted renderer/docs work. The
asset was inspected as bytes but not drawn; no screenshot or manual visual result is claimed. No
CNA source was edited or reverted. Continue with GamerServicesComponent construction,
Initialize/Update behavior and the related dispatcher state in the automatic-sign-in workflow.

Focused713 extends `gamer_services.signed_in_gamer.automatic_sign_in` without adding a variant ID.
Before initialization it finds exactly one game-owned GamerServicesComponent with the correct owner
while the dispatcher remains inactive. Initialization activates the dispatcher and copies the
game's exact legacy window handle. The demo disables the component for the first Update, proving
the pending local sign-in and SignedIn event remain queued, then enables it so the next Update
publishes the existing identity and avatar results. InstallingTitleUpdate remains intentionally
uncredited because CNA never raises it in production; generic EventHandler::Raise would be
synthetic.

SOFTWARE and OPENGLES3/X11 pass10/10 with byte-identical stdout and empty stderr. The focused
SOFTWARE CTest, metadata validation and five validator self-tests pass. Temporary coverage moves
2103->2108/2326 at632 demos,301/329 types and157/753 fields; GamerServices moves161->166/244.
Evidence:`/tmp/checkpoint-713-component-focused-*` and
`/tmp/checkpoint-713-component-coverage.md`. Runner SHA-256 is
`d1a96602d510a1503401ef6752405261bde9500824f899e012ff5f1b7d281fe7`; production library hashes
remain `e2c8c9dbd6ef8fb4e611a6f0818ebec6290c34d738930c3e9f1dbb4a55ba9aeb` and
`471f0a12b6b167262d18ae8fb763f4c10b07e7bc478f72f0bbfd5a2879b193ef`. The exact object was
compiled and the runner relinked against those libraries. A full build remains deferred because
sibling CNA carries more than100 unrelated uncommitted renderer/build/docs paths; none were edited
or reverted here. No service, screenshot or manual visual result is claimed. Continue with Guide's
feasible local DelayNotifications and IsScreenSaverEnabled behavior, then disposition service-only
social overlays.

Focused714 extends `gamer_services.guide.notification_position` without adding a variant ID. It
reads the SDL platform screen-saver state, sets and observes the opposite value, then restores and
verifies the original value. The existing whole-region framebuffer probe still finds the sign-in
toast at TopRight and no misplaced content at BottomCenter. DelayNotifications remains
intentionally uncredited: only an online game-invitation prompt makes its delay observable, so an
offline call alone would be synthetic coverage.

SOFTWARE and OPENGLES3/X11 pass5/5 with empty stderr. Their stdout differs only by the expected
renderer pixel counts,9882 versus9879. The focused SOFTWARE CTest, metadata validation and five
validator self-tests pass. Temporary coverage moves2108->2109/2326 at632 demos,301/329 types and
157/753 fields; GamerServices moves166->167/244. Evidence:
`/tmp/checkpoint-714-guide-state-focused-*` and
`/tmp/checkpoint-714-guide-state-coverage.md`. Runner SHA-256 is
`b8896b5162de8298e307a3463a9f99d0eccbd3d4e6020ca59495cf131a665698`; production library hashes
remain `e2c8c9dbd6ef8fb4e611a6f0818ebec6290c34d738930c3e9f1dbb4a55ba9aeb` and
`471f0a12b6b167262d18ae8fb763f4c10b07e7bc478f72f0bbfd5a2879b193ef`. The exact object was
compiled and the runner relinked against those libraries. A full build remains deferred while the
sibling CNA checkout carries more than100 unrelated external changes; none were edited or reverted
here. Pixel readback was programmatic; no screenshot or manual visual result is claimed. Continue
with Cue::IsCreated and Cue::IsPreparing in the existing XACT state workflow.

Focused715 extends `audio.xact.cue_states` without adding a variant ID. Its new-cue assertion now
proves that synchronous SoundBank::GetCue skips the public Created and Preparing transients and
returns a Prepared, idle object. The existing state machine still reaches playing, simultaneous
playing/paused, stopped, authored Stopping and disposal behavior.

SOFTWARE and OPENGLES3-labelled dummy-audio runs pass15/15 with empty stderr. Their stdout differs
only in the measured authored-release wait,506 versus508 ms, already labelled as control timing
rather than output-sample timing. The focused SOFTWARE CTest, metadata validation and five
validator self-tests pass. Temporary coverage moves2109->2111/2326 at632 demos,301/329 types and
157/753 fields; Audio moves109->111/121. Evidence:
`/tmp/checkpoint-715-cue-state-focused-*` and
`/tmp/checkpoint-715-cue-state-coverage.md`. Runner SHA-256 is
`fe663e76478439345a428b80cd5e4e8384ebd2f3be823fde0041a5dee56064c3`; production library hashes
remain `e2c8c9dbd6ef8fb4e611a6f0818ebec6290c34d738930c3e9f1dbb4a55ba9aeb` and
`471f0a12b6b167262d18ae8fb763f4c10b07e7bc478f72f0bbfd5a2879b193ef`. The exact object was
compiled and the runner relinked against those libraries. A full build remains deferred while the
sibling CNA checkout carries more than100 unrelated external changes; none were edited or reverted.
No listening test or audible-quality result is claimed. Continue with Microphone::All,
GetSampleDuration and IsHeadset in the existing manual input-level workflow.

Focused716 extends the manual `audio.microphone.input_level` workflow without adding a variant ID.
It enumerates Microphone::All, requires Default to be the first entry, uses IsHeadset in its visible
device classification and proves GetSampleDuration reverses the exact generated byte size of its
100 ms mono capture buffer. The original capture/BufferReady/input-level path remains available for
real hardware.

One-frame SOFTWARE and OPENGLES3 direct runs against SDL's dummy recording backend pass2/2 with
byte-identical stdout and empty stderr; both report `System audio recording device`,44100 Hz and
`not a headset`. The ordinary manual CTest remains declared disabled. Metadata validation and five
validator self-tests pass. Temporary coverage moves2111->2114/2326 at632 demos,301/329 types and
157/753 fields; Audio moves111->114/121. Evidence:
`/tmp/checkpoint-716-microphone-direct-*` and
`/tmp/checkpoint-716-microphone-coverage.md`. Runner SHA-256 is
`2730e04e0d2c0bdcf6b16659e674250a884cb3fbace14b6eb3dd2f25e64bd8e1`; production library hashes
remain `e2c8c9dbd6ef8fb4e611a6f0818ebec6290c34d738930c3e9f1dbb4a55ba9aeb` and
`471f0a12b6b167262d18ae8fb763f4c10b07e7bc478f72f0bbfd5a2879b193ef`. The exact object was
compiled and the runner relinked against those libraries. A full build remains deferred while the
sibling CNA checkout carries more than100 unrelated external changes; none were edited or reverted.
No physical microphone, live sample, BufferReady delivery, listening test or recording-quality
result is claimed. Consolidate the remaining Audio, Media, Net and platform-dependent member
dispositions before the next full checkpoint.

The first SOFTWARE complete-checkpoint attempt found one demo/environment defect rather than a CNA
defect: `c_api.input.keyboard_mouse` treated unavailable desktop-global pointer and cursor controls
as failed checks even though the metadata declares that requirement. The C demo now recognizes an
unobservable global warp only on a windowless native system, bypasses the dependent cursor checks
and returns the standard skip code after completing safe cleanup. A no-display SOFTWARE run skips;
the same binary passes all40 controls under X11/Xvfb, including the observable warp and restoration.
Metadata validation, the five validator self-tests and the focused X11/Xvfb CTest pass. The original
632-demo SOFTWARE run
otherwise contained exactly36 expected failures and no unexpected pass. Repeat the complete
SOFTWARE report with this fix, then run the complete OPENGLES3 checkpoint and publish the
consolidated remaining-member dispositions.

The repeated full SOFTWARE/X11 run is clean at558 pass,36 expected fail,35 skip and3 manual.
The first OPENGLES3 run then exposed two timing-sensitive demo assumptions. The lifecycle-order
demo required exactly one fixed-step Update before every draw even though a slower renderer may
legitimately run a catch-up Update; it now requires one or more Updates followed by the exact
BeginDraw/Draw/EndDraw sequence for each of its first two draws. The C runtime-control demo now
switches to variable-step timing before its two explicit Tick calls so SuppressDraw is measured
without fixed-step accumulator variance. Both corrected IDs pass three SOFTWARE repetitions and
ten OPENGLES3/X11 repetitions each. Re-run the complete OPENGLES3 report, then regenerate coverage
and publish the consolidated remaining-member dispositions.

## 15. Assumptions the next session must know

* Current workspace: `/rv/data/development/github.com/libcna/cna-demos`; CNA and the other
  dependencies are siblings (`../cna`, `../sharp-runtime`, `../easy-gl`, `../meta-gl`).
* Current verified toolchain: GCC 14.2 and CMake 3.31.6. Reuse `build/linux-full`; do not create
  temporary build trees. Set `CCACHE_DIR=/rv/cnaccache` and `CCACHE_BASEDIR=/rv` for every build.
* External work may change the sibling CNA checkout. Do not edit/revert it from this task;
  record the observed state and exercised binary hashes, and keep runtime checkpoints immutable.
* Running windowed renderers needs `xvfb-run`; audio needs a device or `SDL_AUDIO_DRIVER=dummy`.

Current complete baseline is647-member-audit-integration; exact results, hashes and evidence are
in section7. It absorbs every earlier focused batch through checkpoint646, including all102
previously staged standalone C IDs and the deeper member work through Picture/PictureAlbum
identity. Use its627-demo coverage and runtime reports as the comparison baseline.
All earlier drafts are APPLIED AND STALE. The gamepad, host-device, clipboard, text-input, joystick,
haptics, sensor-value, sensor-lifecycle, host-info, vibration, dialogs, system-tray, camera-frame,
clipboard-acceptance, display-value and adapter-snapshot batches are all included in that complete
checkpoint lineage. Adapter-capabilities and device-presentation are included in complete492. The later
render-target-bindings, device-capabilities, sprite-scaled, resource-lifecycle, index-buffer,
device-states and sprite-states batches are all included in complete497. Viewport, the later
device-values and resource-events extensions, texture slots and buffer bindings are included in
complete500. Device clears, owned-device lifecycle and device profile/policy state are included in
complete502. The complete504 checkpoint binary was built with clean CNA `f37f8d184`; CNA advanced
externally during smoke to clean `fbd8e64a3` through CI/CMake-test/docs/tooling-only changes, while
both production library hashes stayed unchanged. Complete504 and complete524 remain immutable
historical comparison evidence; complete719 is the current full-runtime baseline.
Preserve refusal evidence and source/fixture observations, including ISSUE-062. Retain
architecture/stable IDs.

## Focused623-C-ModelMesh-resources-collections-and-drawing

`c_api.graphics.model_meshes` adds32 `models.h` routes in33 controls. Exact retained effect and
buffer identities refuse premature destruction; named/empty mesh values, parent ownership, live
part/effect views, duplicate mutations and retained mesh snapshots establish collection identity,
lookup, bounds and alias lifetime. An indexed BasicEffect part produces the exact green center
pixel and an empty mesh preserves the black clear pixel. CNA-DEMOS-ISSUE-081 records that both
native and C construction omit a pre-attached part effect from `Effects`; detaching and reattaching
through the public setter synchronizes it. SOFTWARE and OPENGLES3 Xvfb direct runs pass33/33 with
byte-identical stdout and empty stderr; strict C99 and metadata compilation, diff checks and
validation self-tests pass. Staged metadata reaches2853/3205 and `models.h` reaches70/216.
Evidence:`checkpoint-623-c-model-meshes-focused-*-direct.*`. No ContentManager, imported asset,
top-level Model, screenshot, visual-quality assessment or project-built execution is claimed.
Top-level Model construction, transforms, resource ownership, glTF diagnostics and drawing are next.

## Focused624-C-Model-ownership-transforms-diagnostics-and-drawing

`c_api.graphics.models` adds27 `models.h` routes in39 controls. Default, simple and explicit-parent
Models establish retained collections/root, tag, resource-release callbacks and atomic local plus
absolute transform transfers. A copied glTF report and warning preserve every scalar/text channel
and derive exact loss totals. An actual triangle proves `Model.Draw` composes the child absolute
transform with world, moving green to(700,120) while(400,240) remains black. The first world-only
probe was a demo error corrected after source review. SOFTWARE and OPENGLES3 Xvfb direct runs pass
39/39 with byte-identical stdout and empty stderr; strict C99 and metadata compilation, diff checks
and validation self-tests pass. Staged metadata reaches2880/3205 and `models.h` reaches97/216.
Evidence:`checkpoint-624-c-models-focused-*-direct.*`. No ContentManager, imported glTF, animation
player, screenshot, visual-quality assessment or project-built execution is claimed. Morph-target
data, weight tracks, CPU blending and ModelMeshPart attachment are next.

## Focused625-C-morph-target-data-tracks-and-part-upload

`c_api.graphics.morph_targets` adds all26 morph-related `models.h` routes in29 controls. Owned base,
position/normal/tangent deltas, weights and complete tracks survive caller overwrite; linear, step
and cubic evaluation differ numerically. CPU blending combines positions, normalizes the result and
preserves UV bytes. Flat-normal triangle state and refusal paths are atomic. A retained part upload
is confirmed by direct VertexBuffer position(0.5,0,0.5) for weights(0.5,0.25). SOFTWARE and
OPENGLES3 Xvfb direct runs pass29/29 with byte-identical stdout and empty stderr; strict C99 and
metadata compilation, diff checks and validation self-tests pass. Staged metadata reaches2906/3205
and `models.h` reaches123/216. Evidence:`checkpoint-625-c-morph-targets-focused-*-direct.*`. No
mesh, Model, ContentManager, animation player, draw call, visual result or project-built execution
is claimed. SkinnedModelEXT skeleton/clip values, transforms, part ownership and moves are next.

## Focused626-C-SkinnedModel-skeleton-clips-moves-and-parts

`c_api.graphics.skinned_models` adds26 `models.h` routes in28 controls. Deep-copied two-bone
skeleton/clip state supports exact discovery, atomic track copies and distinct loop/clamp transform
evaluation. Invalid hierarchy replacement leaves state intact; move construction/assignment empties
sources. Matching-skeleton attachment transfers parts. A same-device part retains real buffers and
an optional texture; exact-name replacement releases the old set and removal releases the donor
set. SOFTWARE and OPENGLES3 Xvfb direct runs pass28/28 with byte-identical stdout and empty stderr;
strict C99 and metadata compilation, diff checks and validation self-tests pass. Staged metadata
reaches2932/3205 and `models.h` reaches149/216. Evidence:
`checkpoint-626-c-skinned-models-focused-*-direct.*`. No skinning shader, texture sampling, draw
call, visual result or project-built execution is claimed. SkinningData and AnimationPlayer are next.

## Focused627-C-SkinningData-and-AnimationPlayer

`c_api.graphics.skinning_data_player` adds32 `models.h` routes in28 controls. Deep-copied two-bone
hierarchy, bind/inverse-bind/root-prefix matrices and two sorted clips survive caller mutation and
obey exact, atomic copy contracts. Clip target space and declared rig-root index/name remain
explicit. The retained player exposes bind-pose local/world/skin transforms, selects the named
clip, composes the half-time pose, distinguishes absolute clamp from loop, refuses invalid updates
without moving, and continues after the original SkinningData handle is released. SOFTWARE and
OPENGLES3 renderer-labelled direct runs pass28/28 with byte-identical stdout and empty stderr;
strict C99 and metadata compilation, diff checks and validation self-tests pass. Staged metadata
reaches2964/3205 and `models.h` reaches181/216. Evidence:
`checkpoint-627-c-skinning-data-player-focused-*-direct.*`. No Model, ContentManager, graphics
device, renderer, draw call, screenshot, visual result or project-built execution is claimed. The
renderer-independent ModelAnimationsEXT collection and direct Model pose helpers are next.

## Focused628-C-ModelAnimations-and-direct-posing

`c_api.graphics.model_animations` adds11 `models.h` routes in29 controls. Two deep-copied named
clips preserve sorted exact values and refuse duplicate names, short copies, Count bounds and
undefined target spaces atomically. A real two-bone Model then rejects a joint-palette clip, applies
the same clip in scene-node space at half time, leaves its untracked child local transform intact,
composes their hierarchy and distinguishes duration clamp from non-finite refusal. A separate
two-bone SkinningData bind pose uploads to a real retained SkinnedEffect and reports exactly one
posed effect. SOFTWARE and OPENGLES3 Xvfb direct runs pass29/29 with byte-identical stdout and empty
stderr; strict C99 and metadata compilation, diff checks and validation self-tests pass. Staged
metadata reaches2975/3205 and `models.h` reaches192/216. Evidence:
`checkpoint-628-c-model-animations-focused-*-direct.*`. No vertex/index buffer, texture, draw call,
screenshot, visual result or project-built execution is claimed. Ancillary Model cameras, skins,
bounds, material variants and infinite projection are next.

## Focused629-C-Model-camera-skin-bounds-and-infinite-projection

`c_api.graphics.model_scene_metadata` adds16 `models.h` routes in28 controls. An authored camera
deep-copies and returns its exact scene-node index, flags, infinite projection, world transform,
scalars and atomic UTF-8 name. A one-bone skeleton maps to a real Model mesh index, remains reachable
through a fresh owned handle after source release, distinguishes an unrigged skin and survives list
clear through the issued handle. The Model's aggregate sphere exactly places a local center(1,2,3),
radius2 through root translation x=10. The first invalid-input check assumed zero aspect ratio was
rejected; source review showed XNA's native finite and infinite factories both accept it, so that was
a demo error rather than a CNA defect. SOFTWARE and OPENGLES3 Xvfb direct runs pass28/28 with byte-
identical stdout and empty stderr; strict C99 and metadata compilation, diff checks and validation
self-tests pass. Staged metadata reaches2991/3205 and `models.h` reaches208/216. Evidence:
`checkpoint-629-c-model-scene-metadata-focused-*-direct.*`. No material, buffer, texture, draw call,
screenshot, visual result or project-built execution is claimed. The real content-loaded material-
variant and Tag boundary is next.

## Focused630-C-content-loaded-Model-variants-and-tags

`c_api.content.model_loading` adds the last eight `models.h` routes in30 controls. A real glTF load
reports exact ordered Ocean/Gold names, preserves output on a short copy, applies valid and rejects
invalid selections atomically, and exact back-buffer probes distinguish its red default, blue
Ocean and gold body while the unmapped wheels remain black. Two private hand-written XNB fixtures
then load through the same public Model route: a dictionary tag retains its exact BoundingSphere
after Model destruction, while a shared reflective reader returns the caller's exact foreign
object and CNA neither frees nor mutates it. SOFTWARE and OPENGLES3 Xvfb direct runs pass30/30 with
byte-identical stdout and empty stderr; strict C99 and metadata compilation, diff checks and
validation self-tests pass. Staged metadata reaches2999/3205 and `models.h` reaches216/216,
completing its semantic route audit. Evidence:
`checkpoint-630-c-model-loading-focused-*-direct.*`. Pixel readback was programmatic; no screenshot
or visual inspection was performed. Project-built runner validation still awaits a coherent sibling
CNA checkout. Begin the deeper XNA public member/behavior gap audit next.

## Focused631-Model-pose-arrays-and-collection-contracts

The existing `graphics.model.bones` keeps its four original assertions and adds eight member-
behavior controls. Bone, child and mesh name lookup/membership agree with indexed identity and
distinguish missing/detached objects. `CopyBoneTransformsFrom` refuses a short source atomically,
applies a complete local pose while safely ignoring one extra source entry, composes a root +X
translation through the Forearm absolute transform and restores every authored local value. Short
local and absolute outputs retain all sentinels. The updated object compiles cleanly and an isolated
relink against the last complete object/library set passes12/12 on SOFTWARE and OPENGLES3 with
byte-identical stdout and empty stderr. A temporary coverage generation projects XNA members
1405->1408/2326; no demo ID or C-route count changes. Evidence:
`checkpoint-631-model-bones-members-focused-*`. No screenshot or visual inspection was performed,
and full project validation remains blocked by the external sibling state. Extend the existing
`graphics.model.mesh_parts` workflow with its still-unproved null-effect skip behavior next.

## Focused632-ModelMeshPart-omission-rules

The existing `graphics.model.mesh_parts` retains all10 prior assertions and adds two omission
controls. A null right-part Effect removes green from Effects, keeps the left triangle red and
leaves the right pixel clear. After restoring green, zero left primitives keep red in Effects but
omit only its geometry while the right triangle still draws. The exact updated object compiles
cleanly and a cumulative isolated relink passes12/12 on SOFTWARE and OPENGLES3 with byte-identical
stdout and empty stderr. Evidence:`checkpoint-632-model-mesh-parts-skip-focused-*`. Pixel readback
was programmatic; no screenshot or visual inspection was performed. No ID or coverage count
changes. Continue the top-level Model/ModelMesh behavior audit before leaving the model family.

## Focused633-VertexElement-layout-values-and-drawing

The generated Model-family remainder is now closed by disposition: its collection GetEnumerator
members are CLR/C# machinery already represented by C++ range iteration, and `ModelMesh::Tag` is
the same arbitrary carrier concept demonstrated on Model/ModelMeshPart. No extra model demo was
added. The existing `graphics.vertex_declaration.custom_vertex` instead retains its original
stride assertion and adds six coherent controls for the three elements' exact struct offsets,
formats, semantics and usage indices; complete value equality/inequality, typed Equals and hash;
four mutable properties and exact ToString; and actual layout consumption. First-frame pixels
confirm a white fan center and black exterior under both renderers. The exact object compiles
cleanly and a cumulative isolated relink passes7/7 on SOFTWARE and OPENGLES3 with byte-identical
stdout and empty stderr. Temporary coverage moves the post-checkpoint631 XNA projection
1408->1417/2326 without changing demo or C-route counts. Evidence:
`checkpoint-633-custom-vertex-focused-*` and `/tmp/checkpoint-633-coverage.md`. Pixels were read
programmatically; no screenshot or visual inspection was performed. The exercised C API library
remains SHA256 `730ac2631f65b402d5d016ae85649067441b52c973e49f786b49b3253d75100a`.
Sibling CNA is now clean at `cc547f5224`; inspect that revision, then attempt the deferred complete
reconfigure/build and validation before the next member batch if it is coherent.
