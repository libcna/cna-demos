# Automation, smoke testing and future visual regression

## 1. Automation classes

Every demo declares what an unattended run of it proves (`DemoInfo::automation`):

| Class | Unattended run | Feature verified by the run? | Examples |
|---|---|---|---|
| `automated` | runs a bounded number of frames (Game) or to completion (console) | **yes** -- the demo's `Check`s decide pass/fail; Game demos also prove start-up, frames and cleanup | math, content loading, render targets, audio API with the dummy driver |
| `smoke-only` | runs a bounded number of frames | **no** -- only start-up, frame loop and shutdown; the feature needs a person | keyboard, mouse, gamepad and touch demos |
| `manual` | not started; reported as NOT RUN | no | hardware without an emulation, network services, credentials |

Kinds are orthogonal: console, Game and standalone demos can each be in any class
(see [ARCHITECTURE.md](ARCHITECTURE.md) section 4).

Status words used everywhere (CLI, reports, coverage):

* **PASSED** -- ran to the end, every check held, no exception.
* **FAILED** -- a check failed, an exception escaped, or a standalone demo exited non-zero.
* **SKIPPED** -- a documented requirement is missing *in this environment* (a renderer without 3D
  for a `Need::ThreeD` demo, no audio hardware for a `Need::Audio` demo). Exit code 77. Never used
  to hide a failure: an exception that is not a declared missing requirement is FAILED.
* **NOT RUN** -- manual demo in an unattended batch, or a standalone executable that was not built.

Asynchronous completion checks should observe the expected state within a finite steady-clock
budget. For natural audio completion, leave the voice running and assert `Stopped` after the
state transition or deadline. Nominal clip duration alone does not guarantee that the device
worker reports completion after a fixed sleep. Preserve failure evidence when investigating
such timing assumptions; report successful runs as state-completion evidence.

For short-lived overlays, inspect rendered frames as soon as presentation begins. A fixed late
frame number may already be after a real-time notification expires. Stop observation when either
the expected or incorrect placement becomes visible, and retain the original placement assertion;
never wait only for the desired pixels. Missing presentation must still fail at a finite deadline
or when the automated run ends. Preserve the failed run and inspect actual captures after correction.

### Known CNA issues (expected failures)

A demo whose `knownIssue` names an entry of `docs/CNA_ISSUES.md` checks the documented behaviour
and fails because of that recorded CNA defect. It is never turned into SKIPPED or PASSED:

* a single run prints FAILED and names the issue; the exit code is 1;
* batch runs list it as an expected failure and do not fail the batch for it -- but a known-issue
  demo that **passes** is reported as "UNEXPECTED PASS" and fails the batch;
* CTest registers it with `WILL_FAIL` and the label `known-issue`, so CI stays green while the
  defect exists and turns red when it disappears (time to remove the marker);
* `cna_demos.cli.catalog_json_parses` checks that every `knownIssue` has an entry in CNA_ISSUES.md;
* a defect that only some renderers have names them in `knownIssueRenderers` (for example
  `{"OPENGLES3", "OPENGL33"}`): on those the failure is expected, on every other renderer the demo
  must pass like any other. `--list-ids` prints such an issue as `<issue>@R1,R2`, and CTest sets
  `WILL_FAIL` only for the listed renderers.

The marker applies to the whole demo, not to each assertion. A passing CTest with WILL_FAIL
therefore does not establish that every unrelated check still passed. When extending a known-issue
demo, inspect all `CHECK FAILED` lines in its raw log and explain each against CNA_ISSUES.md.
For complete checkpoints, compare those lines (including their multiplicities) with the archived
baseline as well as comparing report IDs/statuses. Match the marker anywhere in a line,
including prefixed standalone output such as `host:   CHECK FAILED:`, not just at the start. A new assertion failure needs investigation even
when the demo's overall FAILED status was already expected.

`tools/compare_checkpoints.py` performs that evidence comparison once per renderer. It allows
new non-failing IDs, reports changed check counts, and exits 1 for changed existing statuses,
issue markers, missing IDs, or added/removed failed assertions (including multiplicities).
Exit 2 means the evidence could not be read or did not match the supplied catalog. Investigate
every reported change; the tool never rewrites or accepts a baseline automatically. Older merged
logs can contain stdout/stderr splices; unflushed parent stdout can also be split by child stdout. Preserve the raw file; any manually repaired comparison
copy must document its exact change, retain both hashes and corroborate the reconstructed
assertion with independent evidence such as the JSON failure message. Never broadly strip
errors or treat a changed assertion as accepted merely because its demo has a known-issue flag.

```bash
build/linux-full/bin/cna-demos --catalog-json > /tmp/cna-current-catalog.json
python3 tools/compare_checkpoints.py \
  build/linux-full/reports/checkpoint-409-SOFTWARE.json \
  build/linux-full/reports/results-SOFTWARE.json \
  --baseline-log build/linux-full/reports/checkpoint-409-SOFTWARE.log \
  --current-log build/linux-full/reports/smoke-SOFTWARE.log \
  --catalog /tmp/cna-current-catalog.json
python3 tools/test_compare_checkpoints.py
```

Use the corresponding OPENGLES3 files for the other smoke run. This complements `validate.sh`;
it does not prove a source revision, platform, renderer or visual outcome from filenames alone.

## 2. Running unattended

```bash
cna-demos <id> --automated                    # one demo; exit 0 / 1 / 77
cna-demos --smoke                             # every demo (= --run-all --automated)
cna-demos --run-category graphics --automated # one category or ID prefix
cna-demos --run-tag sprite --automated
cna-demos --smoke --report results.json       # machine-readable results (cna-demos-results/1)
cna-demos --smoke --renderer SOFTWARE         # windowless CPU rasteriser, no display needed
```

`--automated` sets the frame limit of each Game demo to its `frames` metadata (default 60) and
disables Escape handling; `--frames N` overrides the limit. Determinism inputs:

* `--seed N` (default 12345) is the only seed demos use (`Context().Seed()`).
* Game demos keep XNA's default fixed time step (60 Hz `TargetElapsedTime`), so every Update advances
  `GameTime` by the same step. The number of Updates per drawn frame is **not** fixed, though: when
  a frame is late the loop runs catch-up Updates, so the game time reached at frame N depends on
  machine speed. Animated demos are therefore not `deterministic` (verified: the same rotation
  demo captured at frame 20 shows different angles on SOFTWARE and OPENGLES3).
* No demo uses the network or user files unless that is the demonstrated feature (and then it is
  classified accordingly).
* Gamer services demos run offline -- `CnaDemos::GamerServicesDemo` overrides CNA's service
  configuration, so a CNA service configured for the user's games is never contacted -- with the
  demos' own profile store and the local profile "Demo Player" signed in automatically
  (`CNA_GAMER_SERVICES_PROFILES_DIR` and `CNA_GAMER_SERVICES_AUTO_SIGN_IN` override both). They
  run in a process of their own (`ownProcess` metadata, see ARCHITECTURE.md section 4), answer the
  Guide through CNA's `Simulate...EXT` calls and type through `TextInputEXT`. Achievements CNA keeps
  offline persist between runs; the demos' checks hold on the first run and on every later one.
* Net demos use Local sessions (no sockets), except `net.packet.send_and_receive`, which hosts a
  System Link session (`Need::Network`): it binds UDP sockets, including CNA's discovery port
  61190, and would answer another machine's session search; it sends nothing on its own. Without a
  usable network it should report SKIPPED, but CNA's socket errors are not yet XNA's
  `NetworkNotAvailableException` (CNA-DEMOS-ISSUE-018), so today it reports FAILED.

## 3. CTest

The standard full checkpoint is `JOBS=4 tools/validate.sh` (default preset: `linux-full`). It
builds, validates metadata/assets, runs CTest, runs SOFTWARE and OPENGLES3 smoke batches (the
latter through Xvfb with SDL's x11 driver and WAYLAND_DISPLAY removed when available), and regenerates coverage from those reports. `JOBS` is
optional and defaults to `nproc`; `SKIP_GL=1` deliberately omits GL and must be noted in the
handoff. Build warnings, command failures, unexpected demo failures and unexpected passes stop
the checkpoint. Build and CTest output is retained in `build/<preset>/reports/build.log` and
`ctest.log`; smoke stdout is retained as `smoke-<renderer>.log` and stderr separately as
`smoke-<renderer>.stderr.log`, alongside the JSON report. Keep both when archiving a checkpoint.
This prevents native logger stderr from being inserted inside a buffered stdout assertion line;
assertion comparison uses stdout, while stderr retains native/platform diagnostics for review.
The process launcher also flushes C++ and C standard streams before starting an inherited-output
child. Separate stderr alone cannot prevent pending parent stdout from being overtaken by child
stdout. The small `cna_demos.framework.process_output_order` test captures through a pipe with
C/C++ buffering deliberately enabled; the old launcher prints CHILD before both parent lines.
The current JSON and stderr outputs are removed at the start; an interrupted process must not leave
old results looking like fresh evidence. Explicitly named baseline archives are preserved.
When the inherited desktop display is unstable, run the whole script under
`SDL_VIDEO_DRIVER=x11 env -u WAYLAND_DISPLAY xvfb-run -a -s '-screen 0 1280x1024x24' tools/validate.sh`, so CTest and SOFTWARE
also have a private display. The script still creates its own nested Xvfb for OPENGLES3 and
forces x11 locally for that child, even if the caller selected Wayland. Xvfb setting DISPLAY alone
does not select SDL's backend: an unchanged native_handle probe printed Wayland with inherited
WAYLAND_DISPLAY, then X11 when explicitly isolated. Both passed; only the Wayland probe emitted
the observed GTK initialization warning. Original inherited-display checkpoints remain valid
runtime evidence but are not proof of private X11 execution. This is an invocation/validation
isolation correction, not a newly attributed CNA API defect.
`python3 tools/test_validate.py` verifies the script's failure propagation with
command stubs, without compiling CNA. It also injects a synthetic inherited Wayland driver and
checks that the GL child receives the private X11 display; that case fails on the old script.

Coverage distinguishes an explicit NOT RUN result from an ID absent in the supplied report.
Regenerating metadata counts with old reports does not validate demos added since those runs;
the generated table and missing-ID list expose that checkpoint debt.

`ctest` registers:

* `cna_demos.cli.*` -- command-line and registry tests (help, list, search, info, validate,
  catalog JSON well-formedness checked by CMake's own JSON parser, usage errors);
* `cna_demos.framework.process_output_order` -- parent/child output ordering, without linking CNA;
* `cna_demos.demo.<renderer>.<id>` -- one test per demo and renderer, **discovered at ctest time**
  from `cna-demos --list-ids` (`cmake/CnaDemosDiscoverTests.cmake`, the `gtest_discover_tests`
  pattern). New demos become tests without re-running CMake. Manual demos are registered DISABLED.

| Setting (CMake cache) | Default | Meaning |
|---|---|---|
| `CNA_DEMOS_TEST_RENDERERS` | `SOFTWARE` if compiled in, else the default renderer | renderers the per-demo tests run on |
| `CNA_DEMOS_TEST_TIMEOUT` | 120 | seconds per demo test |
| `CNA_DEMOS_TEST_AUDIO_DRIVER` | `dummy` | `SDL_AUDIO_DRIVER` for tests: exercises the audio API without hardware |

Labels: `cli`, `framework`, `demo`, the category (`graphics`, `math`, ...), the automation class
(`automated`, `smoke-only`, `manual`), the kind (`console-demo`, `game-demo`,
`standalone-demo`), the renderer, and `headless` or `display`.

```bash
ctest --test-dir build/linux-full -L headless          # no display server needed
xvfb-run -a ctest --test-dir build/linux-full -L display
ctest --test-dir build/linux-full -L graphics -j8       # one category, sharded by CTest
```

Demos that need the network (`Need::Network`: the System Link demos) share the CTest
`RESOURCE_LOCK` `cna_demos_system_link`, so `ctest -j` runs them one at a time: System Link
discovery finds every session on the network, and two of these demos running together would find
each other's sessions.

To exercise several renderers, list them: `-DCNA_DEMOS_TEST_RENDERERS="SOFTWARE;OPENGLES3"`.
Windowed renderers need a display (`xvfb-run`; Mesa llvmpipe provides GL/GLES and lavapipe Vulkan
without a GPU).

## 4. What a run does and does not establish

Record results with the level they actually reached -- never more:

| Level | Meaning | Evidence |
|---|---|---|
| compile-verified | the demo builds | it is in the catalog of a successful build |
| startup-tested | the demo starts on a renderer and exits cleanly | smoke-only PASSED |
| runtime-tested | the demo's checks passed on a renderer | automated PASSED in a `--report` |
| visually inspected | a person (or an approved reference image) confirmed the picture | screenshot reviewed |
| unavailable | the platform/renderer could not be exercised here | stated in HANDOFF.md |

`docs/COVERAGE.md` shows the "Run on <renderer>" columns from `--report` files; HANDOFF.md
lists which renderers and platforms were exercised in the last session.

## 5. Screenshots and visual regression

Implemented:

* `cna-demos <id> --automated --screenshot out.png` saves the **last** frame of a Game demo as PNG.
  It reads the back buffer with `GraphicsDevice::GetBackBufferData` before `Present` and writes it
  with `Texture2D::SaveAsPng` -- both CNA APIs (HiDef profile, see ARCHITECTURE.md section 3).
* `deterministic = true` marks demos whose frame N depends only on N.
* `tools/visual_regression.py` captures approved deterministic Game demos, compares RGBA pixels
  with `references/<renderer>/<id>.png`, and retains actual images, stdout/stderr and highlighted
  mismatch masks. It rejects catalog entries that are non-deterministic or have an active known
  issue. Exact comparison is the default; reviewed per-channel tolerance and differing-pixel
  budgets are available explicitly.
* The initial reviewed pilot covers clear colour, a SpriteBatch texture and compiled SpriteFont
  rendering on SOFTWARE and OPENGLES3. All six committed references are exact across repeated
  private-X11 runs. `references/README.md` records the approval/update rule.
* `VISUAL_REGRESSION=1 tools/validate.sh` runs the comparator after the ordinary checkpoint. It
  uses a private X11 server automatically when the OPENGLES3 smoke stage ran; `SKIP_GL=1` limits
  it to SOFTWARE.

Design decisions already taken to keep this cheap:

* the capture frame is the demo's `frames` value, fixed in metadata;
* only demos marked `deterministic` (static scenes, seeded randomness) are compared; making
  GameTime-driven animation comparable would need a virtual clock -- e.g. a CNA
  `CNA::Platform::IPlatform` decorator passed to `Game(std::unique_ptr<IPlatform>)` whose
  performance counter advances exactly one step per Update. Not built: it depends on the
  internals of `Game::Tick`'s waiting loop and is worth doing only with CNA's maintainers;
* references are per renderer (renderers legitimately differ in rasterisation details);
* expanding the pilot requires reviewing each actual image and adding only stable, meaningful
  representatives. The `deterministic` marker is eligibility, not a requirement to snapshot all
  158 eligible Game demos.

## 6. Continuous integration

`.github/workflows/ci.yml` builds against the exact CNA and sharp-runtime commits pinned in the
workflow (HANDOFF.md "Dependencies"):

1. checks out CNA and its siblings next to cna-demos (the same layout as a developer machine);
2. configures the `windowless` preset (SOFTWARE/HEADLESS/STUB -- no GPU or GL siblings) and installs
   Xvfb for platform integrations such as `Guide::IsScreenSaverEnabled`;
3. builds, runs `cna-demos --validate`, runs `ctest` (CLI tests + every demo on SOFTWARE with the
   dummy audio driver and private X11 display), and uploads the results report and catalog as
   artifacts.

Display-based renderers (`OPENGLES3` under Xvfb) and hardware/platform validation are not part of
the default job; they are listed in HANDOFF.md with their last manual result. CI time stays
bounded as the collection grows because demo tests run in parallel and can be sharded by label.
