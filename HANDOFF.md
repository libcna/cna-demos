# HANDOFF

**Read this first in every new session.** It is the single place that says what state the
project is in and what to do next. Update it after every batch. Last updated: 2026-10-12.

## 1. Purpose (one paragraph)

cna-demos is a growing collection of small executable demonstrations of CNA (C++ XNA 4.0
reimplementation): one concept per demo, stable textual IDs, metadata, a terminal browser, a CLI,
unattended runs, CTest tests and a coverage report. Long-term goal: every meaningful CNA
capability has a demo. See docs/PROJECT.md.

## 2. Current phase

Systematic demo batches, one CNA area at a time (status per phase in docs/ROADMAP.md):

* Complete: Phase 0-1 (research, infrastructure), 2 (math, 78 + 6 packed vectors), 3 (game lifecycle, 33), 4 (2D
  graphics), 5 (render states, buffers, primitives, render targets), 10 (input, 39), 11 (storage, 7).
* Mostly complete: 6 (effects and shaders, 30).
* In progress: 7 (models/glTF, 23), 8 (content, 46), 9 (audio 21 incl. 9 XACT, media 11 incl. a
  compiled video and song), **12 (gamer services 19 incl. avatars and leaderboards; net 14; remainders in ROADMAP)**.
* In progress too: 13 (devices 5, phone 1, design 4, diagnostics 2 + logger 2, cnaext 4).
* In progress: 14 (C API, 19 standalone C demos; `linux-full` builds CNA with `CNA_BUILD_C_API=ON`).
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
* `ownProcess` demos (gamer services: once-per-process dispatcher) are in-process Game demos that
  batch runs and the browser start as `cna-demos <id> --batch-child --report <tmp>`.
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

456 demos (455 last fully validated; launch_parameters pending): math 78, game 33, graphics 148 (19 SpriteBatch, 8 device, 9 texture2d, 1 texture3d, 7 sprite font,
4 blend state, 4 sampler state, 3 rasterizer state, 6 depth-stencil state, 4 primitives,
4 vertex buffer, 3 index buffer, 2 vertex declaration, 9 render target, 1 occlusion query, 8 BasicEffect, 22 other
effects and shaders, 2 viewport, 2 adapter, 1 resource, 6 packed vector, 23 model), audio 21 (9 XACT), core 3 (2 logger),
content 46 (incl. an XNA .contentproj), input 39 (5 keyboard, 6 mouse, 2 text input, 2 clipboard, 12 gamepad, 5 touch, 7 CNAEXT devices:
joystick, hot-plug, device lists, haptics, sensors, power), storage 7, media 11 (compiled video and song), gamer services 19
(own process each; 5 avatar, 2 leaderboard), net 14 (own process each; 5 standalone: two processes), devices 5, phone 1, design 4,
diagnostics 2, cnaext 4, platform 2 (1 standalone), c_api 19 (standalone C programs). Every declared category has demos.
Coverage: see docs/COVERAGE.md (regenerated by tools/validate.sh).
Known-issue demos: 26 (`c_api.game.host_failure` CNA-DEMOS-ISSUE-051,
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

**456 is IN PROGRESS / focused validation passed, full checkpoint pending.** Its catalog/metadata validate
456 demos with zero problems; generated coverage is source-only (293/329 types, 1370/2326 XNA
members, 166/3210 C routes), with no 456 full-runtime columns. The first build succeeded, but a
later external FrameworkDispatcher.cpp change initially prevented the sentinel-enhanced C source
rebuild (ISSUE-053). The repaired retry passed, and both focused launch runs pass fifteen child
checks with empty stderr. All 35 related CTests pass (20.53 s); complete validation session 87099 is running. See the active-batch notes below. The complete 455 evidence remains archived.

Current environment: Linux, GCC 14.2, CMake 3.31.6. The full **455 checkpoint passed** at
9c023d8: tools/validate.sh exit 0, all 468 enabled CTests (199.23 s), both complete isolated-X11
smoke runs and coverage regeneration. Exact ORIGINAL 453 comparisons add only material_colors
and specular_lights (four checks each), retain every old status/known issue/check count and all
35/36 failed assertions, and preserve every C/custom-content child list/trace. All 34 selected
binary/four asset hashes match runtime capture; normalized stderr is identical (no GTK warnings,
same 84 GL X protocol lines). Evidence/catalog/scripts: reports/checkpoint-455-*. Four focused
800x480 captures were actually inspected: material differs only 41,472 ambient-panel pixels by
one green level; specular differs only two pixels by one red level, within original tolerance 1.
External CNA cgltf/GamerServices work changed during preparation (resolved ISSUE-052); clean
ced719909 was observed during runtime and at completion. No atomic input/dependency snapshot.

| Check | Result |
|---|---|
| Configure + build linux-full (8 renderers, shared CNA + C API) | Passed; incremental checkpoint build clean |
| Metadata/assets --validate | 455 demos, 0 problems |
| tools/validate.sh | Passed, both complete smoke runs and coverage regeneration |
| CTest | 443 passed (including 25 WILL_FAIL), 25 skipped, 3 manual disabled; 0 failures |
| SOFTWARE smoke | 402 passed, 25 expected failures, 25 skipped, 3 manual not run |
| OPENGLES3 smoke (explicit x11, private Xvfb) | 423 passed, 26 expected failures, 3 skipped, 3 manual not run |
| Changed assertions | All 35/36 older failures retained; both new material/specular IDs have four passes |
| stderr | Unchanged normalized diagnostics; no GTK warnings; GL X protocol diagnostics unchanged |
| Screenshots | Four material/specular captures inspected; differences fit original tolerance 1 |
| Content build | Passed (CNB, XNB, media, content project) |

Reports/logs: `build/linux-full/reports/`. An initial validation was interrupted during CTest;
a subsequent run exposed a catalog-check timeout. That demo-tooling defect was fixed by
extracting each JSON demo object before checking its fields (2.66 seconds in isolation,
3.89 under load instead of a 30-second timeout); invalid fixture checks still reject bad data.
Only the final full run above counts as the completed checkpoint.

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
| SOFTWARE | runtime-tested (all demos), screenshots inspected |
| OPENGLES3 | runtime-tested under Xvfb + Mesa llvmpipe, screenshots inspected |
| HEADLESS, STUB, OPENGL33, SDL_RENDERER, VULKAN, SDL_GPU | compiled in; not yet run in a recorded session |
| WEBGL2, WEBGPU, DIRECTX9, DIRECTX11, METAL, FNA3D | not built (other platforms / extra deps) |

Platforms: only Linux x86_64 was available. Windows, macOS, web, Android: untested.

## 10. Known CNA issues

See docs/CNA_ISSUES.md. Open: CNA-DEMOS-ISSUE-051 (host failure C exit-hook contract mismatch),
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
CNA-DEMOS-ISSUE-040 (documentation only: Picture thumbnail comments contradict resizing),
CNA-DEMOS-ISSUE-041 (SavePicture ignores short reads and writes zero-padded image data),
CNA-DEMOS-ISSUE-042 (documentation only: root-prefix comments describe older mesh cancellation).

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

**Immediate next action:** commit the validated public ShaderDialectEXT compatibility fix,
update /tmp/cna-archive-456.py and /tmp/cna-456-finish.py sourceCommit, then restart COMPLETE 456
validation. Compatibility build 49553 passed (644 scheduled targets, no compiler diagnostics).
Focused session 12553 passed: four SOFTWARE skips with exactly the known custom-effects logger
line; OPENGLES3 shader/MRT checks 5/4/2/2; launch parameters 15 child passes on both selections;
metadata 456/0. Evidence: checkpoint-456-dialect-focused-*. No new images, but existing pixel
assertions executed. Full 455 remains the last complete baseline; later drafts are UNAPPLIED.

Full 455 at 9c023d8: tools/validate.sh exit 0, all 468 enabled CTests (199.23 s), SW 402/25/25/3,
GL 423/26/3/3. Exact ORIGINAL 453 comparisons add only the two four-check material/specular IDs;
every old status, check count, known issue, 35/36 failed assertions and C/custom-content child
list/trace remains. All 34 binaries/four assets and normalized stderr match. Evidence/catalog/
helpers: reports/checkpoint-455-*. Both scenes pass on both selections; all focused stderr empty.
Four 800x480 captures actually inspected: material differs only 41,472 ambient-card pixels by
one green level (26 SW /25 GL), specular only two pixels by one red level; original tolerance 1.

Initial build FAILED in external uncommitted CNA cgltf implementation splitting: lost private
helper declarations and transitive C string/memory includes. Both new demo objects compiled,
but the executable did not link; all 34 old runtime binaries remained unchanged. Evidence:
reports/checkpoint-455-initial-*. Recorded as ISSUE-052. The external author added string.h and
CgltfInternals wrappers; retry succeeded (111 scheduled targets, known Draco CMP0148 developer
warning only, no compiler diagnostics). ISSUE-052 is resolved for the exercised build; no CNA
file edited/reverted and no warning suppressed by cna-demos. CNA/GamerServices refactoring
was uncommitted over 3ee6ae3 during build; later clean ced719909 was observed during smoke.
No atomic build-input snapshot is claimed.

All 47 targeted C/graphics/CLI CTests passed (18.80 s: 46 passes incl. three WILL_FAIL, one PBR
skip). Evidence, four images, repaired-source observations and retry log:
reports/checkpoint-455-focused-*. /tmp/cna-review-455-focused.py verifies the focused results and
exact image differences. The specular design JSON is analytic design only, not native mutation
validation. /tmp/cna-catalog-455.json has the current catalog.

Full archives include the actual catalog, comparison/finish scripts and final CNA state.
Clean ced719909 observed at completion; source was changing during preparation, so no atomic
source/input or complete transitive binary snapshot is claimed. No source/assets were changed
by cna-demos during runtime.

Previous COMPLETE checkpoint: 453 at 91fc3f9 (closure 9a95a11), tools/validate.sh exit 0, all 466
enabled CTests (197.82 s), SW 400/25/25/3, GL 421/26/3/3. Exact ORIGINAL 451-prefix comparison
adds only automatic_dispatch/vertex_colors, preserves every old 35/36 failure, status and count,
and accepts exactly the TLS nine-check extension after its eight original controls. Other C/
custom-content lists/traces, all 34 selected hashes/four assets and normalized stderr match.
Evidence/catalog/helpers: reports/checkpoint-453-*. Its inspected vertex-color captures differ
only 864 blue pixels by one level. All older C/vertex-color /tmp source drafts are stale.

Current applied C candidate: standalone/c_api.game.launch_parameters/main.c (113 lines), with
its metadata and CMakeLists. Fifteen intended child checks cover explicit bounded StringViews,
owned UTF-8 bytes after caller overwrite, guarded exact/short buffer copies, ordinal enumeration
(Zeta before lowercase names), documented first-wins Add, malformed replacement preserving prior
state, null/zero-length empty values and explicit empty-list clearing. Output lengths are seeded
with UINT64_MAX before short/exact/empty copies so unchanged outputs cannot pass accidentally.
The current C source passes GCC 14 C99 -Wall/-Wextra/-Werror syntax-only validation; runtime is
pending ISSUE-053 repair. /tmp/cna-launch-parameters* source drafts are older, APPLIED AND STALE.
Prepared /tmp/cna-run-456-focused.py, /tmp/cna-archive-456.py, /tmp/cna-456-finish.py and
/tmp/cna-capture-456.py expect 15 child checks, unchanged ORIGINAL 455 statuses/failures and
old C/custom-content lists/traces, plus 35 selected binaries/four assets. The helpers now name source commit 8f2b04c; update that if source changes, and capture runtime
hashes after the checkpoint build succeeds. /tmp/cna-catalog-456.json and source-only coverage are current metadata evidence.
Then continue the semantic member audit. Remaining skinned gaps include independent diffuse contributions from lights 1/2, vertex/texture-
alpha interactions, vertex-lighting specular and arbitrary joint scales. C gaps include automatic Draw dispatch, other callback failures and destruction
cancellation; this batch demonstrates automatic Update specifically. Canonical TLS now covers
nonempty owner details versus empty worker details, not all families or two distinct concurrent
nonempty parameters. Invalid Matrix FOV, CurveKeyCollection index and pure SoundEffect sample
helpers were rejected as ParamName fixtures by source inspection (std exceptions or local guards/
zero-return paths); do not infer canonical detail coverage from those routes. Do not restart
completed phases or add trivial getter/enum variants.

Earlier checkpoint narratives and subsystem source observations are preserved in
[docs/CHECKPOINT_HISTORY.md](docs/CHECKPOINT_HISTORY.md). Consult the relevant history when
returning to an older area; its historical next-task instructions do not replace this section.

## 15. Assumptions the next session must know

* Current workspace: `/rv/data/development/github.com/libcna/cna-demos`; CNA and the other
  dependencies are siblings (`../cna`, `../sharp-runtime`, `../easy-gl`, `../meta-gl`).
* Current verified toolchain: GCC 14.2 and CMake 3.31.6. Reuse `build/linux-full`; do not create
  temporary build trees. Set `CCACHE_DIR=/rv/cnaccache` and `CCACHE_BASEDIR=/rv` for every build.
* External work may change the sibling CNA checkout. Do not edit/revert it from this task;
  record the observed state and exercised binary hashes, and keep runtime checkpoints immutable.
* Running windowed renderers needs `xvfb-run`; audio needs a device or `SDL_AUDIO_DRIVER=dummy`.

## Active 456 batch

Source 8f2b04c adds c_api.game.launch_parameters (113-line C source, 15 child checks). Both
focused SOFTWARE/OPENGLES3 runs pass with empty stderr; all 35 related C/CLI/launch CTests pass
in 20.53 s, including two existing WILL_FAIL cases. Evidence: reports/checkpoint-456-focused-*.
Catalog/metadata: 456 demos, zero problems, 293/329 types, 1370/2326 XNA members, 166/3210 C routes.
No loop/image is claimed for this map example. Its old /tmp source drafts are APPLIED AND STALE.

Previous COMPLETE validation session 87099 terminated exit 1 during build; /tmp/cna-demos-456-validation.log. Its build regenerated
after external configuration changes and scheduled 567 targets. Do not edit demo sources/assets
or rebuild separately while it runs. After the repaired full checkpoint build reaches CTest, run /tmp/cna-capture-456.py
for 35 selected runtime binaries/four assets. Then archive each completed smoke using
/tmp/cna-archive-456.py SOFTWARE|OPENGLES3, and finish with /tmp/cna-456-finish.py. Helpers name
source 8f2b04c; /tmp/cna-catalog-456.json is current metadata. Copies are in checkpoint-456-cna-*.
Full 455 remains the last complete checkpoint until all these gates pass.

ISSUE-053 is resolved for the exercised build: initial final-build attempt failed in external
FrameworkDispatcher namespace lookup; the corrected retry passed (200 scheduled targets).
Preserve the genuine failed compiler log. IMPORTANT capture correction: the files named
checkpoint-456-failed-FrameworkDispatcher.cpp and -failed-state.json were captured AFTER the
external array had moved to a global anonymous namespace. They already contain the repair.
Earlier hash-equality blocker reports were wrong and are superseded by
checkpoint-456-evidence-correction.json. Repair is in external 7d8eba687; later f15d59446 and
ongoing ContentManager/GamerServices edits were observed. This is not an atomic input snapshot.
No CNA source edited/reverted by cna-demos. Original first-build evidence remains separately
archived; it does not validate the later output-sentinel refinement.

Next graphics batch, AFTER full 456 closes: two existing animation extensions and one new cube
transfer scene, catalog 457. All three drafts are UNAPPLIED and GCC 14 syntax-checked only:
- /tmp/cna-palette-player-spaces.cpp: palette_sampling 14->19 checks. Reuses the three-bone
  authored hierarchy through SkinningData/AnimationPlayer to distinguish local/world/skin points
  and compare both public evaluators. Skeleton/clip outlive the player. No new image.
- /tmp/cna-node-animation-contracts.cpp: node_animation 4->9 checks. Nonidentity untracked ancestry,
  target-space refusal before writes, and time clamping after deliberately disturbing the pose.
  Restore authored placement before drawing. Refresh its baseline capture from COMPLETE 456 first.
- /tmp/cna-cube-face-transfers.cpp: 155 lines, eight intended checks, destination
  demos/graphics/texture_cube/face_transfers.cpp. Independent Color faces, source/destination array
  windows, 2x2/1x1 mips, refused writes with deliberately different texels and untouched output
  guards. Atlas pixels are public read-back data rendered as Texture2D; no direct GPU-readback or
  cube-shader-sampling claim. Metadata order corrected; final syntax diagnostics empty.

Drafts and syntax helpers/logs are preserved under reports/checkpoint-456-next-*. Next runtime/
comparison helpers are /tmp/cna-{run-457-focused,archive-457,457-finish,capture-457}.py (syntax
checked, not run). They require ORIGINAL 456, exactly the two count extensions/new cube ID,
unchanged other outcomes/C/custom-content child lists, 35 binaries/four assets, and PENDING_457
replacement after the actual source commit. Inspect all four new node/cube captures; do not
skip focused validation or the complete 457 checkpoint merely because the drafts compile.

Earlier baseline animation runs used the first successful 456 build: palette 14/node 4 pass on
both renderers, four empty stderr streams. Both 800x480 node first-frame captures were inspected;
only two pixels differ across renderers (maximum channel difference 73), not RGBA-identical.
Three selected hashes remained unchanged. Evidence: checkpoint-456-before-* and
-intermediate-binaries.json. Refresh this baseline after current full 456 because CNA changed.

Further source-only TextureCube member audit is in COVERAGE_AUDIT.md / ROADMAP phase 5.
Future DDS diagnostic preparation: /tmp/cna-cube-dds-fixture.py generates the original 506-byte
/tmp/cna-cube_levels.dds with six RGB24 faces and three independent mip colors. JSON records
exact offsets/colors/hash; Python checks structure only. Archived as checkpoint-456-next-*.
Current TextureCube::DDSFromStreamEXT reads once using total Length; unlike the exercised 2D
route, short reads and nonzero starting positions need a bounded runtime probe. Compare normal,
offset-16, seven-byte short-read and truncated streams (with/without prefix); the final payload
byte is deliberately nonzero. No DDS native decoding/upload/visual evidence or new CNA issue yet.

456 full attempt session 87099 TERMINATED exit 1 during build (310/567 progress), before CTest:
external CNA promoted ShaderDialectEXT into public CNA/ShaderDialectEXT.hpp / namespace CNA.
Four shader/MRT demos still imported the former internal name. This is fixed locally by using
the actual public header/namespace in all four sources; no shader text or assertions changed.
Evidence: checkpoint-456-public-dialect-{build,validation}.log and post-failure observed state.
This is demo compatibility maintenance, not a new runtime CNA issue. Rebuild/focus these demos,
commit the source changes, update 456 helper sourceCommit, then restart full validation. The
previous focused launch checks remain valid for their recorded binaries; full 456 is NOT complete.

Independent shader member audit now records ShaderEffect constructors/packages, success versus
failure diagnostics, uniform families/arrays, sampler kinds, block layout and clone/lifetime
boundaries. Existing GLSL success does not establish the error-log branch merely because metadata
names GetCompileErrorEXT. ShaderEffect::Clone currently rebuilds source and copies labels/language;
matrix/uniform/texture-state expectations need a bounded runtime probe before any CNA bug claim.
No additional shader demo, metadata route or runtime credit was added by this source-only table.

Compatibility build 49553 and focused session 12553 both completed successfully. The public
ShaderDialectEXT import replaces all four former internal imports; shader text/assertions are
unchanged. SOFTWARE skip stderr is the exact old custom-effects diagnostic (present in original
455), not empty; the focused helper was corrected after inspecting that original evidence.
Both GL shader results and C launch checks match expected counts. No runtime logs suppressed.
