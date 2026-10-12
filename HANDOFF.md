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
* Mostly complete: 6 (effects and shaders, 37).
* In progress: 7 (models/glTF, 23), 8 (content, 46), 9 (audio 22 incl. 9 XACT, media 11 incl. a
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

470 catalog demos (focused470-cascades; last complete checkpoint469-pcf): math 78, game 33, graphics 161 (19 SpriteBatch, 8 device, 9 texture2d, 5 texture_cube, 2 texture3d, 7 sprite font,
4 blend state, 4 sampler state, 3 rasterizer state, 6 depth-stencil state, 4 primitives,
4 vertex buffer, 3 index buffer, 2 vertex declaration, 9 render target, 1 occlusion query, 8 BasicEffect, 29 other
effects and shaders, 2 viewport, 2 adapter, 1 resource, 6 packed vector, 23 model), audio 22 (9 XACT), core 3 (2 logger),
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

HIGHEST PRIORITY: complete full470-cascades validation. Cascade source is APPLIED AND STALE.
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
1. Commit source/docs and export /tmp/cna-catalog-470-cascades.json.
2. Run /tmp/cna-run-470-cascades-validation.sh, then /tmp/cna-capture-470-cascades.py once
   after its build during CTest. Freeze source/assets/runtime during both smoke phases.
3. Archive both renderers with /tmp/cna-archive-470-cascades.py into matching review logs;
   run /tmp/cna-470-cascades-finish.py into /tmp/cna-470-cascades-finish-review.log.
   Baseline complete469-pcf: sourceb8f3d0d, closure15576a7. Expected483enabled CTests,
   SW406/27/34/3, GL435/29/3/3. Preserve all38/42failures, exact child/network controls,
   normalized stderr except the new declared SOFTWARE cascade skip. Investigate differences.
4. Close full docs/history/handoff and commit. Immediately continue470-query: run focused
   helper BEFORE applying /tmp/cna-occlusion-lifecycle.cpp, inspect before/after captures,
   then review before committing, because that reviewer compares original code against HEAD.

## 15. Assumptions the next session must know

* Current workspace: `/rv/data/development/github.com/libcna/cna-demos`; CNA and the other
  dependencies are siblings (`../cna`, `../sharp-runtime`, `../easy-gl`, `../meta-gl`).
* Current verified toolchain: GCC 14.2 and CMake 3.31.6. Reuse `build/linux-full`; do not create
  temporary build trees. Set `CCACHE_DIR=/rv/cnaccache` and `CCACHE_BASEDIR=/rv` for every build.
* External work may change the sibling CNA checkout. Do not edit/revert it from this task;
  record the observed state and exercised binary hashes, and keep runtime checkpoints immutable.
* Running windowed renderers needs `xvfb-run`; audio needs a device or `SDL_AUDIO_DRIVER=dummy`.

Current complete baseline is469-pcf, source b8f3d0d, closure15576a7.
All earlier receiver/PCF, audio, XACT, reader, move, field, lighting, palette and EOF proposals
are APPLIED AND STALE. Preserve previous checkpoint evidence, including failures and probes.
See docs/CHECKPOINT_HISTORY.md. Never overwrite current source with an older /tmp draft.

Applied cascade source: /tmp/cna-skinned-shadow-cascades.cpp; syntax41794 CLOSED0, empty log.
Its six proposed controls use two authored atlas slices, an inclusive split, one-unit blend
band, independent fitting camera and Count0 single-map fallback. All six GL controls and the explicit SOFTWARE skip now pass; actual image inspected.
Focused evidence: checkpoint-470-cascades-focused-*. Original source-only recovery:
checkpoint-469-shadow-next-*, especially shadow-cascades-notes.
Prepared focused/full helper family: /tmp/cna-*470-cascades*; recovery under
checkpoint-469-shadow-next-validation-drafts-*. The cascade watched build, focused/targeted runner and reviewer have passed; full helpers are pending.

Later source-only audit: /tmp/cna-occlusion-query-audit.md, recovered as
checkpoint-469-shadow-next-occlusion-query-audit.md. Existing visibility/count/polling behavior
is distinct from query-ordering and disposed-state gaps. Extend the existing coherent demo,
retaining original three visibility checks; no per-error IDs or premature GPU timing claims.

Later unapplied OcclusionQuery draft: /tmp/cna-occlusion-lifecycle.cpp, syntax74173 CLOSED0,
empty log. A scratch query proposes eight ordering/disposal controls before the unchanged
original three visibility controls (11expected). No native/image evidence. Recovery source,
syntax and audit: checkpoint-469-pcf-next-*. Finish PCF and cascades first.

Prepared later query helper family: /tmp/cna-*470-query*, recovered under
checkpoint-469-pcf-next-helpers-*. Run focused helper with before/after argument; capture
before applying the source and inspect all four actual images. Run reviewer before committing
so HEAD remains the original query source. Full expected483CTests and unchanged470totals.
These are unrun scripts, not evidence. PCF closure helper is /tmp/cna-close-469-pcf.py.

External CNA advanced during the frozen checkpoint from e842e7473 to clean123c36c34b:
LINK-041 changes declared CNB loader installation/removal locking, LINK-042 refines static
linker-flag matching. Selected runtime hashes remained unchanged; these newer source changes
are not credited as exercised by469-pcf. The470-cascades initial and consistency builds have now incorporated them.
Evidence: checkpoint-469-pcf-external-source-delta.json. No sibling files were edited.

Cascade focused reviewer/document helper now account for an initial changed-input build and
require a final unchanged-input consistency build. Prepared /tmp/cna-document-470-cascades-focused.py
must run only after actual image inspection and successful focused reviewer. Recovery:
checkpoint-470-cascades-pre-native-*. Both helpers have now passed after actual image inspection.
Initial33371 CLOSED0 (584scheduled/581numbered), six changed inputs; consistency90087 CLOSED0
(32scheduled/29numbered), all3,909watched inputs unchanged. Focused/targeted16253 CLOSED0;
GL6checks with empty stderr, SWdeclaredskip0/exit77,26CTests4.36s. CNA123c36c34b has external
uncommitted CMake/probe/document changes, recorded in the journals; no sibling edits by this task.
