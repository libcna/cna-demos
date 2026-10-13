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
* In progress: 7 (models/glTF, 23), 8 (content, 46), 9 (audio 23 incl. 10 XACT, media 12 incl. a
  compiled video and song), **12 (gamer services 19 incl. avatars and leaderboards; net 14; remainders in ROADMAP)**.
* In progress too: 13 (devices 5, phone 1, design 4, diagnostics 2 + logger 2, cnaext 4).
* In progress: 14 (C API, 21 standalone C demos; `linux-full` builds CNA with `CNA_BUILD_C_API=ON`).
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

476 catalog demos (complete checkpoint476-C-input-gamepad-values): math 78, game 33, graphics 162 (19 SpriteBatch, 8 device, 9 texture2d, 5 texture_cube, 2 texture3d, 7 sprite font,
4 blend state, 4 sampler state, 3 rasterizer state, 6 depth-stencil state, 4 primitives,
4 vertex buffer, 3 index buffer, 2 vertex declaration, 9 render target, 1 occlusion query, 8 BasicEffect, 30 other
effects and shaders, 2 viewport, 2 adapter, 1 resource, 6 packed vector, 23 model), audio 23 (10 XACT), core 3 (2 logger),
content 46 (incl. an XNA .contentproj), input 39 (5 keyboard, 6 mouse, 2 text input, 2 clipboard, 12 gamepad, 5 touch, 7 CNAEXT devices:
joystick, hot-plug, device lists, haptics, sensors, power), storage 7, media 13 (compiled video and song, tagged library import and album art), gamer services 19
(own process each; 5 avatar, 2 leaderboard), net 14 (own process each; 5 standalone: two processes), devices 5, phone 1, design 4,
diagnostics 2, cnaext 4, platform 2 (1 standalone), c_api 21 (standalone C programs). Every declared category has demos.
Coverage: see docs/COVERAGE.md (regenerated by tools/validate.sh). The focused disconnected-gamepad
batch raises actual C ABI coverage to364/3210 and `input_gamepad.h` to61/63; its complete checkpoint
is pending because the concurrently edited sibling CNA Vulkan source/header do not currently build.
Build-only CNA-DEMOS-ISSUE-057 is tracked separately; it adds no runtime knownIssue marker.
Known-issue demos: 29 (`c_api.storage.save_game` CNA-DEMOS-ISSUE-056,
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
Live numbers: `cna-demos --categories`, docs/COVERAGE.md.

## 7. Build and test status

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

Last complete baseline:

Complete checkpoint **476-C-input-gamepad-values**, source `6ac3d08`, extends the existing C
gamepad child28->47checks without another ID or fixture. Full tools/validate.sh passes metadata476/0
and all489enabled CTests in153.30s. SOFTWARE411/27/35/3 and OPENGLES3441/29/3/3 preserve
all38/42prior failures, every unrelated C/custom child list and protected trace, five network
controls,18music checks and8album-art checks. Only the C gamepad child grows to the exact focused47
controls; parentJSON remains0. All39 selected paths match preflight and stay fixed through both
smoke suites. Against complete476-C-input-keyboard, only the catalog and C gamepad executable
change; four assets and30fixtures are exact. Normalized stderr is identical. Outer Xvfb has6exact
amdgpu pairs; both exits are zero. Post-build capture sees clean CNA9edbf314; unrelated DirectX11
renderer/documentation retirement work appears during smoke at the same head without changing
relevant gamepad source hashes or selected Linux runtime artifacts. Coverage stays297/329types and
1405/2326XNA members; actual C ABI coverage is341/3210 and `input_gamepad.h`38/63. No physical
controller, device actuation/sensor, other-platform or new visual claim. Complete evidence:
checkpoint-476-c-input-gamepad-values-*.
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

See docs/CNA_ISSUES.md. Open: CNA-DEMOS-ISSUE-057 (concurrent tool staging diagnostic race),
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

HIGHEST PRIORITY: once the external CNA Vulkan source/header are mutually consistent again, run
two watched standard builds and the complete476 checkpoint for the focused C disconnected-gamepad
batch above. Do not treat the direct-object/manual-link focused evidence as a substitute. While
that external workstream is blocked, audit `input_devices.h` for the next coherent C extended-input
batch: distinguish deterministic disconnected enumeration/value behavior from hardware-success
branches, and reuse an existing C input demo where coherent rather than adding route-count noise.
Do not credit the separate live gamepad dead-zone overload on a disconnected neutral slot or
duplicate generated argument checks. Leave mouse-touch emulation manual-deferred, generic
touch/media reflection uncredited and `media_player_update_ext` deferred. Never edit/revert CNA;
preserve ISSUE-038/057/058/059/060/061.

## 15. Assumptions the next session must know

* Current workspace: `/rv/data/development/github.com/libcna/cna-demos`; CNA and the other
  dependencies are siblings (`../cna`, `../sharp-runtime`, `../easy-gl`, `../meta-gl`).
* Current verified toolchain: GCC 14.2 and CMake 3.31.6. Reuse `build/linux-full`; do not create
  temporary build trees. Set `CCACHE_DIR=/rv/cnaccache` and `CCACHE_BASEDIR=/rv` for every build.
* External work may change the sibling CNA checkout. Do not edit/revert it from this task;
  record the observed state and exercised binary hashes, and keep runtime checkpoints immutable.
* Running windowed renderers needs `xvfb-run`; audio needs a device or `SDL_AUDIO_DRIVER=dummy`.

Current complete baseline is476-C-input-gamepad-values; source commit is in section7. All earlier
drafts are APPLIED AND STALE. The focused live/disconnected `input_gamepad.h` batch is applied; its
standard/full build is externally blocked as described in sections7 and14.
Preserve refusal evidence and source/fixture observations. Retain architecture/stable IDs.
