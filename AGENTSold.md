# CLAUDE.md -- guide for coding agents working on cna-demos

cna-demos is a long-lived collection of small executable demos of **CNA** (C++ XNA 4.0
reimplementation, `../cna`). One executable `cna-demos` (terminal browser + CLI) holds every
in-process demo; each demo is one `.cpp` under `demos/` that self-registers with stable ID and
metadata. Read, in order: this file, **HANDOFF.md** (current state, next task), PLAN.md,
docs/ARCHITECTURE.md.

## Starting a session

1. Read HANDOFF.md. 2. `git status`, `git log --oneline | head`.
3. Make sure the siblings exist: `../cna` (+ submodules SDL, SDL_mixer, draco),
   `../sharp-runtime`, `../easy-gl`, `../meta-gl` (versions in HANDOFF.md).
4. Baseline: `cmake --preset linux-full && cmake --build --preset linux-full -j4`,
   `build/linux-full/bin/cna-demos --validate`, `ctest --test-dir build/linux-full -j4`.
5. Continue with HANDOFF.md's "Next task". 6. Before stopping (or every substantial batch),
   update HANDOFF.md and commit.

## Commands

```bash
cmake --preset linux-full                     # or windowless (no display/GL siblings), linux-multi
cmake --build --preset linux-full -j4
B=build/linux-full/bin
$B/cna-demos --validate                       # registry/metadata/assets checks
$B/cna-demos <id> --automated --renderer SOFTWARE --screenshot /tmp/x.png   # run + capture
SDL_AUDIO_DRIVER=dummy SDL_VIDEO_DRIVER=x11 env -u WAYLAND_DISPLAY \
  xvfb-run -a $B/cna-demos <id> --automated --renderer OPENGLES3 --screenshot /tmp/y.png
SDL_AUDIO_DRIVER=dummy $B/cna-demos --smoke --renderer SOFTWARE --report /tmp/r.json
ctest --test-dir build/linux-full -j4         # CLI tests + one test per demo (SOFTWARE)
python3 tools/coverage_report.py --exe $B/cna-demos --cna-root ../cna --results SOFTWARE=/tmp/r.json
```

For this shared workstation, use HANDOFF.md's existing build tree, compiler and shared ccache;
keep parallelism at four. For a complete headless checkpoint, use the explicit X11 invocation in
[AUTOMATION.md](docs/AUTOMATION.md). Xvfb's DISPLAY alone does not prevent SDL selecting an
inherited Wayland session. Set a private `XDG_RUNTIME_DIR` for headless runs.

## Rules (non-negotiable)

* **English only** (code, comments, docs, CLI, commits) except intentional localisation demos.
* **Real CNA API only.** Verify every call in `../cna/modules/*/include` before writing it.
  Properties are `getXProperty()/setXProperty()`. Never assume XNA/MonoGame/FNA APIs exist.
* **One concept per demo**; keep the taught CNA call visible; no wrappers around CNA.
* **ID = path**: `demos/a/b/c.cpp` → `a.b.c` (or `a.b.c.<name>` for siblings in one file).
  IDs are permanent, lower_snake segments, leaves only. Never sequential numbers.
* **Metadata** (`CNA_DEMOS_CONSOLE` / `CNA_DEMOS_GAME` / `CNA_DEMOS_STANDALONE`): title, description
  ending with '.', `.apis` as `Type::Member` (XNA names; ctor = `Type::Type`; CNA ext fully
  qualified), correct `automation` (`Automated`/`SmokeOnly`/`Manual`) and `needs`.
* **Checks**: `ctx.Check(...)` every verifiable fact. SKIPPED only for declared missing
  requirements. Never fake a pass; never report compile-only as run.
* **No inflation**: no per-enum-value demos, no copies, no placeholders/TODO demos.
* **CNA bugs**: minimise, record in docs/CNA_ISSUES.md; no silent workarounds. Do not edit `../cna`
  from this repository's tasks except an obvious low-risk fix done separately in CNA's repo.
* **No CMake edits to add a demo.** New category → one line in `demos/CMakeLists.txt`.
  Standalone only for process-level features (`standalone/<id>/`).
* **Assets**: original (add to `tools/generate_assets.py`) or licensed + THIRD_PARTY_NOTICES.md.
* Keep HANDOFF.md accurate after every batch; regenerate docs/COVERAGE.md; update ROADMAP status
  and CHANGELOG.

## Where things are

`src/CnaDemos/` framework headers (demos include `CnaDemos/Demo.hpp`) · `src/framework/`
implementation · `src/app/` CLI + browser · `demos/<category>/` demos · `standalone/<id>/` ·
`assets/` · `cmake/` · `tools/` · `docs/`. Details: docs/ARCHITECTURE.md; how to write a demo:
docs/DEMO_AUTHORING.md; automation/CTest/screenshots: docs/AUTOMATION.md.
