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
latter through Xvfb when available), and regenerates coverage from those reports. `JOBS` is
optional and defaults to `nproc`; `SKIP_GL=1` deliberately omits GL and must be noted in the
handoff. Build warnings, command failures, unexpected demo failures and unexpected passes stop
the checkpoint. Build and CTest output is retained in `build/<preset>/reports/build.log` and
`ctest.log`; smoke output is retained as `smoke-<renderer>.log` alongside its JSON report.
The two current JSON outputs are removed at the start; an interrupted process must not leave
old results looking like fresh evidence. Explicitly named baseline archives are preserved.
When the inherited desktop display is unstable, run the whole script under
`WAYLAND_DISPLAY= xvfb-run -a -s '-screen 0 1280x1024x24' tools/validate.sh`, so CTest and SOFTWARE
also have a private display. The script still creates its own nested Xvfb for OPENGLES3.
`python3 tools/test_validate.py` verifies the script's failure propagation with
command stubs, without compiling CNA.

Coverage distinguishes an explicit NOT RUN result from an ID absent in the supplied report.
Regenerating metadata counts with old reports does not validate demos added since those runs;
the generated table and missing-ID list expose that checkpoint debt.

`ctest` registers:

* `cna_demos.cli.*` -- command-line and registry tests (help, list, search, info, validate,
  catalog JSON well-formedness checked by CMake's own JSON parser, usage errors);
* `cna_demos.demo.<renderer>.<id>` -- one test per demo and renderer, **discovered at ctest time**
  from `cna-demos --list-ids` (`cmake/CnaDemosDiscoverTests.cmake`, the `gtest_discover_tests`
  pattern). New demos become tests without re-running CMake. Manual demos are registered DISABLED.

| Setting (CMake cache) | Default | Meaning |
|---|---|---|
| `CNA_DEMOS_TEST_RENDERERS` | `SOFTWARE` if compiled in, else the default renderer | renderers the per-demo tests run on |
| `CNA_DEMOS_TEST_TIMEOUT` | 120 | seconds per demo test |
| `CNA_DEMOS_TEST_AUDIO_DRIVER` | `dummy` | `SDL_AUDIO_DRIVER` for tests: exercises the audio API without hardware |

Labels: `cli`, `demo`, the category (`graphics`, `math`, ...), the automation class
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

## 5. Screenshots and the future visual-regression system

Implemented now:

* `cna-demos <id> --automated --screenshot out.png` saves the **last** frame of a Game demo as PNG.
  It reads the back buffer with `GraphicsDevice::GetBackBufferData` before `Present` and writes it
  with `Texture2D::SaveAsPng` -- both CNA APIs (HiDef profile, see ARCHITECTURE.md section 3).
* `deterministic = true` marks demos whose frame N depends only on N.

Intended extension (not built yet), designed so it needs no change to demos:

```text
for each deterministic demo D and renderer R:
    cna-demos D --automated --renderer R --screenshot actual/R/D.png
    compare actual/R/D.png with references/R/D.png (per-channel tolerance, max differing pixels)
    report: identical / within tolerance / different (+ diff image)
```

Design decisions already taken to keep this cheap:

* the capture frame is the demo's `frames` value, fixed in metadata;
* only demos marked `deterministic` (static scenes, seeded randomness) are compared; making
  GameTime-driven animation comparable would need a virtual clock -- e.g. a CNA
  `CNA::Platform::IPlatform` decorator passed to `Game(std::unique_ptr<IPlatform>)` whose
  performance counter advances exactly one step per Update. Not built: it depends on the
  internals of `Game::Tick`'s waiting loop and is worth doing only with CNA's maintainers;
* references are per renderer (renderers legitimately differ in rasterisation details);
* the comparison tool can be a small CNA-free program or script; references belong in a separate
  directory (`references/<renderer>/<id>.png`) with a README describing how they were approved.

## 6. Continuous integration

`.github/workflows/ci.yml` builds against the CNA release pinned in the workflow:

1. checks out CNA and its siblings next to cna-demos (the same layout as a developer machine);
2. configures the `windowless` preset (SOFTWARE/HEADLESS/STUB -- no GPU, no display, no GL siblings);
3. builds, runs `cna-demos --validate`, runs `ctest` (CLI tests + every demo on SOFTWARE with the
   dummy audio driver), and uploads the results report and catalog as artifacts.

Display-based renderers (`OPENGLES3` under Xvfb) and hardware/platform validation are not part of
the default job; they are listed in HANDOFF.md with their last manual result. CI time stays
bounded as the collection grows because demo tests run in parallel and can be sharded by label.
