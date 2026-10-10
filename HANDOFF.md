# HANDOFF

**Read this first in every new session.** It is the single place that says what state the
project is in and what to do next. Update it after every batch. Last updated: 2026-10-10.

## 1. Purpose (one paragraph)

cna-demos is a growing collection of small executable demonstrations of CNA (C++ XNA 4.0
reimplementation): one concept per demo, stable textual IDs, metadata, a terminal browser, a CLI,
unattended runs, CTest tests and a coverage report. Long-term goal: every meaningful CNA
capability has a demo. See docs/PROJECT.md.

## 2. Current phase

* Phase 0 (research) and Phase 1 (infrastructure + pilot) are **complete**.
* **Phase 2 -- math** is complete (72 math demos, 6 packed-vector demos).
* Next: **Phase 3 -- game lifecycle** (see "Next task").

## 3. Architecture in 10 lines

* One executable `cna-demos` (src/app: CLI `Cli.cpp`, browser `Browser.cpp`).
* Framework (src/framework, headers src/CnaDemos): `Registry`, `DemoInfo` metadata, `DemoContext`
  (output/checks/seed), `DemoGame : Microsoft::Xna::Framework::Game`, `Runner`, catalog JSON,
  validation, process helpers.
* Demos: `demos/<category>/.../<name>.cpp`, ID = path, registered by `CNA_DEMOS_CONSOLE`/
  `CNA_DEMOS_GAME`; each category is an OBJECT library found by `GLOB_RECURSE CONFIGURE_DEPENDS`.
* Standalone demos: `standalone/<id>/{metadata.cpp,main.cpp,CMakeLists.txt}` → `cna-demo-<id-with-dashes>`.
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

* No SpriteFont asset yet → no on-screen text in demos (decision pending, PLAN.md §2).
* Visual regression: screenshots exist, reference comparison not implemented (AUTOMATION.md §5).
* CI on GitHub (ubuntu-24.04, `windowless` preset): runs 1-2 configured, built, validated and
  smoke-ran every demo successfully; the CTest step failed because the discovery script used
  `if(IN_LIST)` (needs CMP0057, unset in ctest script mode on CMake 3.x). Fixed with `list(FIND)`;
  verified locally with ctest 3.28. Check the next run's result.
* `windowless` preset: see build status below.

## 6. Demo counts

86 demos: math 72, graphics 9 (6 packed vector), core 1, content 1, audio 1, input 1, platform 1 (standalone).
Coverage (docs/COVERAGE.md): 50/329 XNA types, 264/2326 XNA members named by a demo.
Known-issue demos: 1 (`math.curve.step_continuity`, CNA-DEMOS-ISSUE-002).
Categories declared but empty: game, media, storage, diagnostics, devices, phone, design, cnaext,
net, gamer_services. Live numbers: `cna-demos --categories`, docs/COVERAGE.md.

## 7. Build and test status (this session, Linux, Ubuntu 24.04, GCC 14.2, CMake 4.4.4 via pip)

| Check | Result |
|---|---|
| Configure + build `linux-full` (8 renderers, CNA as libcna.so) | ✅ |
| `cna-demos --validate` | ✅ 0 problems |
| `tools/validate.sh` (build, validate, ctest, smoke SOFTWARE + OPENGLES3/Xvfb, coverage) | ✅ |
| `ctest` (15 CLI tests + 86 demos on SOFTWARE, dummy audio) | ✅ 101/101 (1 expected failure via WILL_FAIL) |
| `--smoke` on SOFTWARE (no display), dummy audio | ✅ 85 passed, 1 expected failure (known issue) |
| `--smoke` on OPENGLES3 under Xvfb (Mesa llvmpipe, GLES 3.2), dummy audio | ✅ 85 passed, 1 expected failure (without the dummy driver the audio demo is SKIPPED: no ALSA device -- correct) |
| Screenshots inspected (SOFTWARE, OPENGLES3): draw_texture, render_target2d | ✅ correct, identical across both |
| CMake 3.28.3 (distro) with CNA_SHARED_LIBRARY=ON | ❌ CNA-DEMOS-ISSUE-001; cna-demos now defaults to static there |

## 8. Exact commands that work

```bash
# siblings: ../cna ../sharp-runtime ../easy-gl ../meta-gl (+ cna submodules)
cmake --preset linux-full -DCMAKE_CXX_COMPILER=g++-14 -DCMAKE_C_COMPILER=gcc-14
cmake --build --preset linux-full -j4
tools/validate.sh                       # the standard end-of-batch check (all of the below)
export XDG_RUNTIME_DIR=/tmp/xdg-runtime; mkdir -p -m 700 $XDG_RUNTIME_DIR
build/linux-full/bin/cna-demos --validate
ctest --test-dir build/linux-full -j4
SDL_AUDIO_DRIVER=dummy build/linux-full/bin/cna-demos --smoke --renderer SOFTWARE --report /tmp/r.json
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

See docs/CNA_ISSUES.md. Open: CNA-DEMOS-ISSUE-001 (libcna.so link with CMake 3.27/3.28),
CNA-DEMOS-ISSUE-002 (Curve Step keys evaluated against absolute position 1.0).

## 11. Dependencies (pinned for CI; same as cna-template's dependencies.lock)

| Repo | Version | Commit |
|---|---|---|
| cna | 0.1.0 | 10a4ef1cb4cd832f761536d77588000104947f8d |
| sharp-runtime | 0.1.0 | c20373bae5be1f02d1fe2fa33c4cbba8ad05e4a0 |
| easy-gl | 0.1.1 | f4d6afaf16cdc7e7e73130950bc45e6d865b41bd |
| meta-gl | 0.4.1 | dd3b30055d55fdc42240de7af248259338256992 |

System packages: README "Requirements". CMake >= 3.29 recommended (pip `cmake` 4.4.4 used here).

## 12. Important design decisions

PLAN.md §2 (table). The non-obvious ones: HiDef project profile; SOFTWARE as test renderer;
`SDL_AUDIO_DRIVER=dummy` in tests; SKIPPED only for declared needs; standalone only for
process-level features; static CNA on CMake < 3.29.

## 13. Unusual and generated files

* `docs/COVERAGE.md` -- generated (tools/coverage_report.py). Do not edit by hand.
* `assets/*` -- generated (tools/generate_assets.py), committed, deterministic.
* `build*/` and `*.log` at the root -- ignored.

## 14. Next task

**Phase 3 -- game lifecycle** (`demos/game/`): Game method order, fixed/variable time step,
TargetElapsedTime, IsRunningSlowly, Exit/Exiting, components (GameComponent,
DrawableGameComponent, UpdateOrder/DrawOrder), GameServiceContainer, GameWindow (title,
ClientBounds, resizing), GraphicsDeviceManager (back buffer, ApplyChanges, profile, events),
LaunchParameters, TitleContainer -- see docs/ROADMAP.md Phase 3. Then Phase 4 (2D graphics). Each: read the header in
`../cna/modules/math/include/Microsoft/Xna/Framework/`, write console demos with checks, build,
validate, ctest, regenerate COVERAGE.md, update this file, commit.

## 15. Assumptions the next session must know

* The CNA checkouts live next to this repository (`/home/user/cna` etc. in the cloud session).
* The distribution CMake (3.28) is too old for CNA's shared mode; `pip install cmake` provides 4.x.
* Running windowed renderers needs `xvfb-run`; audio needs a device or `SDL_AUDIO_DRIVER=dummy`.
