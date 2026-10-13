# CNA Demos

**CNA Demo Browser** -- small, focused, executable demonstrations of
[CNA](https://github.com/libcna/cna), the C++ reimplementation of the XNA 4.0 programming model.

Each demo shows **one** CNA capability in a short program you can run, read and copy:
clearing the screen, drawing a texture with `SpriteBatch`, rendering to a `RenderTarget2D`,
loading a PNG through the `ContentManager`, playing a `SoundEffect`, reading the keyboard,
choosing the renderer at runtime. Every demo has a stable ID, searchable metadata, and an
unattended mode, so the same collection is also a smoke-test and regression suite for CNA and its
renderers.

```text
$ cna-demos --list
audio.sound_effect.play                    Play a sound effect
content.texture2d.load_png                 Load a PNG texture
graphics.render_target.render_target2d     Render to a texture
graphics.sprite_batch.draw_texture         Draw a texture
math.vector2.length                        Vector2 length
...
$ cna-demos graphics.sprite_batch.draw_texture
```

## Status

**Early, working foundation (version 0.1.0).** The infrastructure is complete and validated: one
executable with an interactive terminal browser and a scripting CLI, a self-registering demo
registry with metadata and validation, unattended runs with frame limits and screenshots, CTest
integration with one test per demo, a JSON catalog, and an API coverage report. The demo
collection itself is at its beginning; see [docs/COVERAGE.md](docs/COVERAGE.md) for the current
numbers and [docs/ROADMAP.md](docs/ROADMAP.md) for what comes next.

The long-term scope is every meaningful CNA capability -- potentially thousands of demos. It is a
multi-session project; [HANDOFF.md](HANDOFF.md) always describes the current state.

## Relationship to CNA

cna-demos is a separate project that **consumes** CNA from a source checkout; it contains no CNA
code. It demonstrates only what the checked-out CNA actually provides. Problems found in CNA while
writing demos are recorded in [docs/CNA_ISSUES.md](docs/CNA_ISSUES.md). How it differs from CNA's
tests, cna-samples and cna-examples is explained in [docs/PROJECT.md](docs/PROJECT.md).

## Requirements

* CMake **3.23** or newer (**3.29+ recommended**: with older CMake, CNA is linked statically --
  see [CNA_ISSUES.md](docs/CNA_ISSUES.md) CNA-DEMOS-ISSUE-001).
* A C++23 compiler: GCC 14+, Clang 18+ or MSVC 19.38+ (what CNA itself requires).
* CNA **0.1.0** and its sibling checkouts, side by side with this repository:

  ```text
  parent/
  ├── cna/             https://github.com/libcna/cna            (with submodules, see below)
  ├── sharp-runtime/   https://github.com/libcna/sharp-runtime  (always)
  ├── easy-gl/         https://github.com/libcna/easy-gl        (OPENGLES3/OPENGL33/WEBGL2 only)
  ├── meta-gl/         https://github.com/libcna/meta-gl        (OPENGLES3/OPENGL33/WEBGL2 only)
  └── cna-demos/       this repository
  ```

* CNA's own system dependencies. On Debian/Ubuntu:

  ```bash
  sudo apt-get install -y cmake ninja-build pkg-config ccache \
      libavcodec-dev libavformat-dev libavutil-dev libswresample-dev \
      libcurl4-openssl-dev nlohmann-json3-dev libzstd-dev libopus-dev \
      libx11-dev libxext-dev libxrandr-dev libxi-dev libxcursor-dev libxfixes-dev \
      libxss-dev libxtst-dev libwayland-dev wayland-protocols libxkbcommon-dev \
      libgl1-mesa-dev libegl-dev libasound2-dev libpulse-dev
  # optional: SDL_GPU renderer and its Vulkan runtime driver
  sudo apt-get install -y libvulkan-dev libshaderc-dev glslang-tools mesa-vulkan-drivers
  ```

## Build

```bash
git clone https://github.com/libcna/cna.git
git clone https://github.com/libcna/sharp-runtime.git
git clone https://github.com/libcna/easy-gl.git
git clone https://github.com/libcna/meta-gl.git
git -C cna submodule update --init third_party/SDL third_party/SDL_image third_party/SDL_mixer third_party/draco
git clone https://github.com/libcna/cna-demos.git
cd cna-demos

cmake --preset linux-multi          # or: windowless, default, linux-full (adds SDL_GPU + C API)
cmake --build --preset linux-multi
```

The first configure builds SDL3 once (CNA caches it); a full build compiles CNA and takes a few
minutes. `cmake --list-presets` shows all presets:

| Preset | Renderers | Needs |
|---|---|---|
| `default` | CNA's platform default (OPENGLES3 on Linux, SDL_RENDERER elsewhere) | a display for running |
| `windowless` | SOFTWARE (default), HEADLESS, STUB | nothing -- no display, no GL siblings; used by CI |
| `linux-multi` | OPENGLES3 (default), OPENGL33, SDL_RENDERER, SOFTWARE, HEADLESS, STUB | easy-gl/meta-gl |
| `linux-full` | linux-multi + SDL_GPU, C API | libshaderc; a supported SDL GPU runtime driver |

Without presets: `cmake -S . -B build -DCNA_ROOT_DIR=/path/to/cna -DCNA_GRAPHICS_RENDERER=...`.
`CNA_ROOT_DIR` may also be set in the environment. Renderer options are CNA's own
(`CNA_GRAPHICS_RENDERER`, `CNA_GRAPHICS_RENDERERS`); see CNA's README.

## Run

```bash
cd build/linux-multi/bin
./cna-demos                                   # interactive browser: Category → ... → Demo → Run
./cna-demos --list                            # all demos
./cna-demos --search sprite                   # search IDs, titles, descriptions, tags, APIs
./cna-demos --info graphics.render_target.render_target2d
./cna-demos graphics.render_target.render_target2d          # run one demo (Escape closes it)
./cna-demos graphics.device.clear --renderer OPENGL33       # pick a compiled-in renderer
./cna-demos --smoke --renderer SOFTWARE                     # every demo, unattended, no display
./cna-demos graphics.sprite_batch.draw_texture --automated --screenshot shot.png
./cna-demos --catalog-json > catalog.json                   # machine-readable catalog
./cna-demos --help                                          # everything else
```

In the browser, type a number to open a group or demo, `b` to go back, `/text` to search, an ID to
jump to a demo, `q` to quit. Demos run inside the browser process and return to it when closed
(gamer services demos, which CNA can set up only once per process, start a fresh `cna-demos`).

## How demos are organised

* IDs are hierarchical: `<category>.<area>.<concept>`, e.g. `graphics.sprite_batch.draw_texture`.
  The ID *is* the source path: `demos/graphics/sprite_batch/draw_texture.cpp`.
* Categories mirror CNA's modules: `core`, `math`, `game`, `graphics`, `content`, `audio`, `media`,
  `input`, `storage`, `platform`, `diagnostics`, `devices`, `phone`, `design`, `cnaext`, `net`,
  `gamer_services`.
* Most demos run inside `cna-demos`. The few that need their own process (for example selecting
  the renderer, which CNA latches per process) are separate executables under `standalone/`;
  the browser lists and launches them like any other demo.
* Gamer services demos run offline with local profiles kept apart from your own CNA profiles
  (the local profile "Demo Player" is signed in for them); they never contact a CNA service.

## Adding a demo

Create one `.cpp` file under `demos/`, register it with `CNA_DEMOS_CONSOLE(...)` or
`CNA_DEMOS_GAME(...)`, build, run `cna-demos --validate`. No CMake changes. The full guide, with
templates and the metadata reference, is [docs/DEMO_AUTHORING.md](docs/DEMO_AUTHORING.md).

## Testing

```bash
ctest --preset windowless                              # with the windowless preset
ctest --test-dir build/linux-multi -L headless         # everything that needs no display
xvfb-run -a ctest --test-dir build/linux-multi -L display
```

There are CLI/registry tests and one test per demo (discovered automatically). Unattended
classifications, SKIPPED semantics, screenshots and the planned visual-regression system are
described in [docs/AUTOMATION.md](docs/AUTOMATION.md). `python3 tools/coverage_report.py` regenerates
[docs/COVERAGE.md](docs/COVERAGE.md) (developer tool; not needed to build).

## Platform caveats

* Developed and tested on Linux (Ubuntu 24.04, GCC 14): SOFTWARE, HEADLESS, STUB without a display;
  OPENGLES3/OPENGL33/SDL_RENDERER under Xvfb with Mesa's software drivers. Historical checkpoints
  also exercised CNA's since-retired VULKAN renderer. Windows, macOS,
  web and Android builds follow CNA's support matrix but have **not** been tested for cna-demos
  yet -- see HANDOFF.md for exactly what was exercised.
* Input demos need a person (they are smoke-tested unattended only). Audio demos need an audio
  device or `SDL_AUDIO_DRIVER=dummy`; without either they report SKIPPED.

## Documentation

| Document | Contents |
|---|---|
| [docs/PROJECT.md](docs/PROJECT.md) | purpose and philosophy |
| [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) | how it is built and how it runs |
| [docs/DEMO_AUTHORING.md](docs/DEMO_AUTHORING.md) | how to add a demo |
| [docs/AUTOMATION.md](docs/AUTOMATION.md) | unattended runs, CTest, screenshots, CI |
| [docs/ROADMAP.md](docs/ROADMAP.md) | coverage areas still to do |
| [docs/COVERAGE.md](docs/COVERAGE.md) | generated coverage report |
| [docs/CNA_ISSUES.md](docs/CNA_ISSUES.md) | CNA problems found by demos |
| [PLAN.md](PLAN.md), [HANDOFF.md](HANDOFF.md) | implementation plan and current state |
| [CONTRIBUTING.md](CONTRIBUTING.md), [CHANGELOG.md](CHANGELOG.md) | contributing, history |

## License

Microsoft Public License (Ms-PL), the same licence as CNA -- see [LICENSE](LICENSE).
Third-party notices and asset provenance: [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
