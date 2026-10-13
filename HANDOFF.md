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
* In progress: 14 (C API, 33 standalone C demos; `linux-full` builds CNA with `CNA_BUILD_C_API=ON`).
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

493 catalog demos (latest complete checkpoint492-C-device-presentation; one focused batch pending): math 78, game 33, graphics 162 (19 SpriteBatch, 8 device, 9 texture2d, 5 texture_cube, 2 texture3d, 7 sprite font,
4 blend state, 4 sampler state, 3 rasterizer state, 6 depth-stencil state, 4 primitives,
4 vertex buffer, 3 index buffer, 2 vertex declaration, 9 render target, 1 occlusion query, 8 BasicEffect, 30 other
effects and shaders, 2 viewport, 2 adapter, 1 resource, 6 packed vector, 23 model), audio 23 (10 XACT), core 3 (2 logger),
content 46 (incl. an XNA .contentproj), input 39 (5 keyboard, 6 mouse, 2 text input, 2 clipboard, 12 gamepad, 5 touch, 7 CNAEXT devices:
joystick, hot-plug, device lists, haptics, sensors, power), storage 7, media 13 (compiled video and song, tagged library import and album art), gamer services 19
(own process each; 5 avatar, 2 leaderboard), net 14 (own process each; 5 standalone: two processes), devices 5, phone 1, design 4,
diagnostics 2, cnaext 4, platform 2 (1 standalone), c_api 38 (standalone C programs). Every declared category has demos.
Coverage: see docs/COVERAGE.md (current catalog with complete492 runtime reports). Actual C ABI
coverage is678/3205: `input_gamepad.h`61/63,
`input_devices.h`31/37, `input_text.h`17/17, `input_joystick.h`27/30 and
`input_haptics.h`33/36, `sensors.h`116/144, `devices.h`59/62, `display.h`22/22 and
`render_target.h`12/12. Complete492 exercises the first669 routes in both full smoke suites; the
nine new render-target routes have focused evidence under both renderers.
Build-only CNA-DEMOS-ISSUE-057 is tracked separately; it adds no runtime knownIssue marker.
Known-issue demos: 30 (`c_api.devices.sensor_lifecycle` CNA-DEMOS-ISSUE-062,
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
Live numbers: `cna-demos --categories`, docs/COVERAGE.md.

## 7. Build and test status

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

See docs/CNA_ISSUES.md. Open: CNA-DEMOS-ISSUE-063 (synthetic-camera Closed comments contradict
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

HIGHEST PRIORITY: continue the semantic C-route audit with the ten uncovered graphics-device
introspection routes in `graphics.h` (renderer identity/info, capability/feature/limit/format
queries, capability report, shader dialect and backbuffer info). Keep the fixed SpriteBatch type
name helpers classified as trivial, and handle `cna_sprite_batch_submit_scaled_many` in the
existing sprite-batch workflow rather than inflating this device-query batch. Preserve the
renderer-specific answers instead of assuming both backends expose the same capabilities.
Preserve external state and do not claim a real dialog, URL launch, vibration or tray interaction
when a public test backend is the executed path. Do not credit the three joystick, three haptic or
six host sensor/touch populated-metadata routes until suitable hardware executes them.
Do not credit the separate live gamepad dead-zone overload on a disconnected neutral slot or
duplicate generated argument checks. Leave mouse-touch emulation manual-deferred, generic
touch/media reflection uncredited and `media_player_update_ext` deferred. Never edit/revert CNA;
preserve ISSUE-038/057/058/059/060/061/062/063.

## 15. Assumptions the next session must know

* Current workspace: `/rv/data/development/github.com/libcna/cna-demos`; CNA and the other
  dependencies are siblings (`../cna`, `../sharp-runtime`, `../easy-gl`, `../meta-gl`).
* Current verified toolchain: GCC 14.2 and CMake 3.31.6. Reuse `build/linux-full`; do not create
  temporary build trees. Set `CCACHE_DIR=/rv/cnaccache` and `CCACHE_BASEDIR=/rv` for every build.
* External work may change the sibling CNA checkout. Do not edit/revert it from this task;
  record the observed state and exercised binary hashes, and keep runtime checkpoints immutable.
* Running windowed renderers needs `xvfb-run`; audio needs a device or `SDL_AUDIO_DRIVER=dummy`.

Current complete baseline is492-C-device-presentation; source commit and exact results are in section7.
All earlier drafts are APPLIED AND STALE. The gamepad, host-device, clipboard, text-input, joystick,
haptics, sensor-value, sensor-lifecycle, host-info, vibration, dialogs, system-tray, camera-frame,
clipboard-acceptance, display-value and adapter-snapshot batches are all included in that complete
checkpoint. Adapter-capabilities and device-presentation are included in complete492. The focused
render-target-bindings batch is applied and awaits a later complete checkpoint.
Preserve refusal evidence and source/fixture observations, including ISSUE-062. Retain
architecture/stable IDs.
