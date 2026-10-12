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
* Mostly complete: 6 (effects and shaders, 35).
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

467 catalog demos (complete checkpoint467): math 78, game 33, graphics 159 (19 SpriteBatch, 8 device, 9 texture2d, 5 texture_cube, 2 texture3d, 7 sprite font,
4 blend state, 4 sampler state, 3 rasterizer state, 6 depth-stencil state, 4 primitives,
4 vertex buffer, 3 index buffer, 2 vertex declaration, 9 render target, 1 occlusion query, 8 BasicEffect, 27 other
effects and shaders, 2 viewport, 2 adapter, 1 resource, 6 packed vector, 23 model), audio 21 (9 XACT), core 3 (2 logger),
content 46 (incl. an XNA .contentproj), input 39 (5 keyboard, 6 mouse, 2 text input, 2 clipboard, 12 gamepad, 5 touch, 7 CNAEXT devices:
joystick, hot-plug, device lists, haptics, sensors, power), storage 7, media 11 (compiled video and song), gamer services 19
(own process each; 5 avatar, 2 leaderboard), net 14 (own process each; 5 standalone: two processes), devices 5, phone 1, design 4,
diagnostics 2, cnaext 4, platform 2 (1 standalone), c_api 19 (standalone C programs). Every declared category has demos.
Coverage: see docs/COVERAGE.md (regenerated by tools/validate.sh).
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

Focused467-scene is also verified (see next task): palette6GLchecks and C38childchecks with
only original ISSUE-056, inspected images,24targeted CTests. Current metadata names174C routes;
full467-scene is pending. No new catalog ID.

**Complete 467 at source c45349f is verified.** tools/validate.sh and outer Xvfb both return 0,
independently journaled; session 41309 is CLOSED. Build: 7 scheduled operations, metadata 467/0,
all 480 enabled CTests in 217.01 s (448 passed incl27 WILL_FAIL,32 skipped,3 manual disabled).
SOFTWARE405/27/32/3, OPENGLES3 432/29/3/3. Generated coverage now uses these complete results:
293/329types, 1373/2326XNA members, 170/3210 C routes; comprehensive coverage is not claimed.

Exact originalcomplete466-transfer comparison retains all 37/41 prior failed assertions and
adds only ISSUE-056 (38/42 total). Only C save_game changes status/knownIssue; its23 old child
checks remain plus8 named additions (31 total, 30 pass/1 failure). All other C/custom lists/traces
and five network evaluation checks are exact. New kernel5GL/SWskip matches focused evidence.
Only other JSON count change is GL game.window.title10->11, its positive per-Update variation.
All35 selected binary/four asset hashes remain stable. Normalized stderr adds exactly one
SOFTWARE CustomEffects skip; GL is unchanged, including42known X protocol records and the
intentional shader compile error. Separate outerXvfb diagnostics retain14 amdgpu pairs; no
hardware GPU claim. CNA 714f9f76a observed clean at capture/final, not an atomic input snapshot.
All capture/archive/finish helpers pass; immutable evidence: reports/checkpoint-467-*.

Focused build36scheduled/33completed, metadata 467/0 and all24targeted CTests pass (5.55s,
16passed/8capability skips). The actual800x480kernel image was inspected and all27sample pixels
are exact. C EOF behavior is independently reproduced through direct FileStream and C API:
position-1after short/empty EOF reads despite valid data/guards/rewind. All35shipped binaries
remain unchanged through those probes. Proof:checkpoint-466-transfer-eof-*, ISSUE-056.
Previous complete466-transfer source5944070/closureaf64eff, its mixed-layout build correction
and explicitly reviewed GL diagnostic reduction remain in CHECKPOINT_HISTORY.md.

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

See docs/CNA_ISSUES.md. Open: CNA-DEMOS-ISSUE-056 (FileStream position after EOF),
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

HIGHEST PRIORITY: focused467-scene is verified; commit source/docs then run its complete
checkpoint against ORIGINAL COMPLETE467 (sourcec45349f, closure7e62f83). Both drafts are now
APPLIED AND STALE. Build12scheduled/11completed, metadata467/0, all24targeted CTests pass
(16passed/8capability skips,4.14s). GL palette4->6checks and both actually inspected images
retain original color controls plus the shared parent transform/tint; all17selected before/after
pixels are exact. Two rows became three, so whole-image identity is not claimed. C storage
31->38childchecks retains every old line/status plus precisely7positive permission controls,
with37passes and only original ISSUE-056. GL/C stderr empty; SW keeps the same declared skip.
Evidence/review:reports/checkpoint-467-scene-focused-*. Current inventory174C routes; generated
COVERAGE.md remains complete467's170-route snapshot until new full validation finishes.

Prepared /tmp/cna-run-467-scene-validation.sh performs full validation with inner/outer exit
journals and separate Xvfb diagnostics. After its build, run /tmp/cna-capture-467-scene.py
before smokes and never recapture after runtime. Do not edit sources/rebuild during smokes.
Archive/review complete smoke outputs using /tmp/cna-archive-467-scene.py SOFTWARE/OPENGLES3,
then /tmp/cna-467-scene-finish.py. Helpers prepared, not yet executed. Catalog is
/tmp/cna-catalog-467-scene.json. Expected480enabled CTests, totals SW405/27/32/3, GL432/29/3/3.
Preserve all38/42raw failures, every old outcome/knownIssue and C/custom/network evidence,
filtering only7named permission additions. Only palette4->6GLis an intended old JSON change;
positive title Update variance needs explicit review. No new normalized stderr is expected.

After full validation update coverage/closure docs and continue deeper public behaviors;
independent skinned diffuse-light terms and further resource/dispatch contracts remain useful
coherent extensions. No new demo ID is needed for the current batch.

## 15. Assumptions the next session must know

* Current workspace: `/rv/data/development/github.com/libcna/cna-demos`; CNA and the other
  dependencies are siblings (`../cna`, `../sharp-runtime`, `../easy-gl`, `../meta-gl`).
* Current verified toolchain: GCC 14.2 and CMake 3.31.6. Reuse `build/linux-full`; do not create
  temporary build trees. Set `CCACHE_DIR=/rv/cnaccache` and `CCACHE_BASEDIR=/rv` for every build.
* External work may change the sibling CNA checkout. Do not edit/revert it from this task;
  record the observed state and exercised binary hashes, and keep runtime checkpoints immutable.
* Running windowed renderers needs `xvfb-run`; audio needs a device or `SDL_AUDIO_DRIVER=dummy`.

Current complete helpers /tmp/cna-capture-467.py, /tmp/cna-archive-467.py
SOFTWARE/OPENGLES3 and /tmp/cna-467-finish.py all passed. Current diagnostic differences are
only one declared SOFTWARE skip; the previous finite GL reduction belongs to466-transfer.
All runtime hashes/exits/reports are immutable. The volume transfer and game-content-borrow
/tmp predecessors are APPLIED AND STALE. Do not reapply or restart old466-timing work.

APPLIED shader predecessor /tmp/cna-sample-kernel.cpp (STALE): one bright texel, original kernel, horizontal
1:2:1 spread and vertical1:6:1 spread using the same shader. Actual SetUniformVec2Array offsets,
SetUniformFloatArray weights and SetUniformVec2 texel size remain visible. Caller arrays are
overwritten before drawing; expected centers/neighbors/off-axis/outer pixels distinguish tap
contributions. Five intended checks. Native GL5checks and inspected image now pass; SW declares its capability skip.

APPLIED C predecessor /tmp/cna-storage-stream-lifetime.c (STALE): eight additions preserve the save format and
all23original checks. Device/container parent destruction is refused while children live.
A seek selects the last8bytes (nonzero name tail plus padding), a12byte request into14allocated
bytes proves short-read data/guards, a second EOF read preserves output, position must be24,
then rewind and original full-save read still work; a closed handle leaves query output intact.
Two routes plus knownIssue056 are applied in metadata. Native probe confirms exactly the position failure;
remaining30checks pass on both requested renderer environments, empty stderr. No Game is created.

EOF proof helpers /tmp/cna-build-storage-eof-probe.py and -canonical-eof-probe.py write only
/tmp objects/executables using existing compile/link configuration (no dependency-file edits,
no second build tree). /tmp/cna-run-eof-probes.py and /tmp/cna-review-eof-probes.py completed.
Direct canonical probe uses a unique /tmp file, reads4nonzero bytes with intact guards, gets
position-1 after short/empty reads, then successfully rewinds/rereads. Exit1 solely means the
position contract failed; fixture/other-control failures would be exit2. C probes each exit1
with30/31checks passing and SUCCESS/-1 instead of24. Exact original23child list plus8named
additions verified; all35shipped binary hashes unchanged. Source observations are clean CNA
59598bfac at first review and714f9f76a at later review, sharp-runtime c20373bae clean; none is
an atomic input snapshot. Evidence/sources/helpers: reports/checkpoint-466-transfer-eof-*.
Focused runner /tmp/cna-run-467-focused.py before/after and exact review completed.


APPLIED follow-up /tmp/cna-uniform-scene.cpp (predecessor now stale) extends existing uniform_palettes rather
than adding an ID. It retains four original checks in the first two resized viewports and
adds a third scene-wide parent transform/premultiplied tint, using actual scalar SetUniformMat4
and SetUniformVec4 alongside the existing arrays. Explicit16float matrix layout, caller-buffer
overwrite, expected shifted centers RGB(0,32,128)/(64,0,128), vacated old centers and retained
gap distinguish transform order and tint/alpha. Six intended checks. Before/after images will
change from two rows to three; preserve old assertions but do not claim pixel identity.
Source proposal; syntax passed, native/visual after evidence now verified (see section14). Full467is closed.

APPLIED follow-up /tmp/cna-storage-read-only.c (predecessor now stale) extends the now-applied31check C
save_game. It adds7coherent permission controls: Read+Truncate refusal before file changes,
valid explicit Read open, combined CanRead/CanWrite/CanSeek inspection, refused one-byte
write, unchanged position/length, exact original save bytes and successful close. Actual
storage.h routes and canonical exception mapping (NotSupported -> CNA_RESULT_NOT_SUPPORTED)
were reviewed. Existing31checks including ISSUE-056 remain; expected38total/one known failure.
Four new C routes would need metadata (open_file_access and three capability queries).
Syntax and native validation passed (37positive checks/only existing056). Preserve the same demo-owned file/cleanup, no new ID.

Next helpers are now running as described in section14: /tmp/cna-467-next-syntax.py checks both follow-up drafts
with existing C++/C flags; /tmp/cna-run-467-scene-focused.py captures current palette/storage
before application, then after. Full467is closed; before captures/syntax completed successfully. Palette4->6GLchecks/SWskip;
C31->38childchecks with exactly the same ISSUE-056 failure. Two GL images should be inspected;
layout changes from2rows to3, so pixel identity is not expected. Planned same-count checkpoint
label467-scene avoids overwriting current467archives. New C route total would174, no new ID.
