# Architecture

This document describes how cna-demos is built and how it runs, in enough detail to change it
safely. The rules for writing one demo are in [DEMO_AUTHORING.md](DEMO_AUTHORING.md); automation
is in [AUTOMATION.md](AUTOMATION.md).

## 1. Overview

```text
cna-demos (one executable)
├── src/app/          main(), command line (Cli.cpp), terminal browser (Browser.cpp)
├── src/framework/    registry, metadata, runner, DemoGame, catalog/JSON, validation, process helpers
├── demos/<category>/ one OBJECT library per category, one small .cpp per demo
└── standalone/<id>/  metadata.cpp compiled into cna-demos; main.cpp built as its own executable
                          │
                          ▼ links
                     CNA (`CNA` umbrella target from a CNA source checkout, add_subdirectory)
```

* **One user-facing executable**, `cna-demos`, contains every in-process demo.
* A demo is **one `.cpp` file** that registers itself with a macro. There is no central list of
  demos to edit and no generated registry.
* The **stable identity** of a demo is its textual ID (`graphics.sprite_batch.draw_texture`).
  Numbers shown by the browser are positions in a menu and mean nothing else.
* A few demos must own their process (renderer selection, process start-up, the C API, ...).
  They are **standalone executables**, the documented exception.

## 2. Repository layout

| Path | Contents |
|---|---|
| `CMakeLists.txt`, `CMakePresets.json` | top-level build, presets |
| `cmake/CnaDemosCna.cmake` | locating and adding CNA |
| `cmake/CnaDemosCategories.cmake` | `cna_demos_add_category()`, `cna_demos_add_standalone()`, warnings, packaging |
| `cmake/CnaDemosTests.cmake`, `CnaDemosDiscoverTests.cmake`, `CnaDemosCheckCatalog.cmake`, `CnaDemosCheckProcessOutput.cmake` | CTest integration |
| `src/CnaDemos/*.hpp` | the framework API demos include (`CnaDemos/Demo.hpp` is the only one a demo needs) |
| `src/framework/*.cpp` | framework implementation (static library `cna_demos_framework`) |
| `src/app/*.cpp` | the executable's command line and browser |
| `demos/CMakeLists.txt` | the list of top-level categories |
| `demos/<category>/<subcategory...>/<name>.cpp` | the demos |
| `standalone/<demo-id>/` | standalone demos (`metadata.cpp`, `main.cpp`, `CMakeLists.txt`) |
| `assets/` | shared demo assets, copied next to the executables at build time |
| `content/` | content-pipeline sources (e.g. the font sheet) compiled by CNA's `cna-content` into the same assets directory (CNB format) |
| `content_xnb/` | content-pipeline sources compiled into XNA's XNB format, into `assets/xnb/` |
| `content_media/` | sources whose import needs CNA's FFmpeg backend (videos, compressed audio), compiled into `assets/media/` only when CNA has it |
| `content_project/` | an XNA Game Studio content project (`Game.contentproj`) and its sources, built as it is into XNB under `assets/contentproj/` |
| `title/` | files CNA reads by a fixed path in the title directory (the offline achievement catalog), copied next to the executables |
| `tools/` | developer tools (Python): asset generator, coverage report |
| `docs/` | documentation; `docs/COVERAGE.md` is generated |

The project never needs `src/` or `include/` restrictions from CNA: since CNA 0.1.0 its module
validator is anchored on CNA's own root (the bug cna-template once worked around is fixed).

## 3. CNA integration

CNA exports no CMake package. Like cna-template and cna-examples, cna-demos adds a CNA **source
checkout** with `add_subdirectory()` and links the `CNA` INTERFACE umbrella, which carries every
framework module, the selected renderer(s), Sharp Runtime, include paths and definitions.

* Location: `CNA_ROOT_DIR` (cache variable or environment variable), default `../cna`. CNA in turn
  needs `../sharp-runtime` and, for the GL renderers, `../easy-gl` and `../meta-gl`, and its
  submodules `third_party/SDL`, `SDL_image`, `SDL_mixer`, `draco`. CNA reports each missing piece
  itself; cna-demos only checks that the CNA checkout exists.
* CNA is added `EXCLUDE_FROM_ALL` (only what cna-demos links is built) and, with CMake >= 3.25,
  `SYSTEM` (CNA headers do not trigger cna-demos' own warning flags).
* Forced CNA options: `CNA_BUILD_TESTS=OFF`, `CNA_BUILD_EXAMPLES=OFF` (and easy-gl's). Defaults
  changed by cna-demos: `CNA_CNAEXT=ON`, `CNA_DEVICES=ON` -- a demo collection wants the optional
  modules. Each can be turned off; dependent categories then disappear.
* Modules outside the umbrella (`CNA::Net`, `CNA::GamerServices`, `CNA::Phone`, `CNA::Design`)
  are linked by the categories that need them (`demos/CMakeLists.txt`).
* Standalone intermediate XML tools (`content.custom.intermediate_*`) and the XNA component
  adapter tool (`content.custom.xna_pipeline_components`) link
  `CNA::ContentPipelineBuild`. Each process owns its serializer registry and dependencies;
  these stay outside the browser's runtime link closure. Metadata remains discoverable when
  the tool target is unavailable, and the runner reports the omitted executable as NOT RUN.
  C++ standalones use the browser's shared-library-first link workaround (ISSUE-016), preventing
  optional module archives from introducing a second runtime and duplicate static destructors.
* `content.custom.incremental_compiler` links `CNA::ContentCompiler`, the same public coordinator
  used by `cna-content`. It creates a fresh game-owned registry per invocation and lets CNA own
  discovery, fingerprints, manifest skipping, publication and manifest-based clean. The tool's
  temporary score inputs are original runtime fixtures, not another native build tree. This
  build-time dependency remains isolated from the browser's runtime link closure.
* C ABI examples default to C99. The focused `c_api.core.thread_local_errors` and
  `c_api.game.foreign_thread_dispatch` targets use C11 threads/atomics after a compile/link
  availability check and link the standard Threads target. The former uses game-independent
  diagnostic routes; the latter explicitly pumps worker game-property calls on the owning thread.
  Neither claims automatic callback pumping or graphics execution on a worker.
* `CNA_SHARED_LIBRARY`: CNA links its runtime as `libcna.so` by default on Linux. With CMake older
  than 3.29 that link fails (see [CNA_ISSUES.md](CNA_ISSUES.md) CNA-DEMOS-ISSUE-001), so
  cna-demos defaults it to OFF there. With a shared CNA, `cna-demos` names the `libcna.so` file
  first on its link line: `CNA::GamerServices`/`CNA::Net` are static and link CNA's module
  archives, and ahead of libcna.so they would put a second CNA into the executable
  (CNA-DEMOS-ISSUE-016).

### Renderers

Renderer choice is CNA's own interface and is passed through untouched:

* `CNA_GRAPHICS_RENDERER` -- the default renderer of the build;
* `CNA_GRAPHICS_RENDERERS` -- optional list compiled into one binary (multi-renderer mode).

At run time `cna-demos --renderer NAME` calls `CNA::GraphicsRendererSelection::SetPreferred()`
before any device exists and exports `CNA_GRAPHICS_RENDERER=NAME` so standalone demos inherit it.
Without the option CNA's own precedence applies (environment variable, then build default).

CNA **latches** the renderer when the first `GraphicsDevice` of a process is created; one
process therefore uses one renderer. Running demos on several renderers means several processes
(the CTest per-demo tests do exactly that). Demos never branch on the renderer name; they ask
`GraphicsDevice::SupportsCapability()` or declare a `Need` (section 6).

### Graphics profile

`main()` calls `CNA::SetProjectGraphicsProfileEXT(GraphicsProfile::HiDef)`, CNA's counterpart of an
XNA project's "Graphics Profile" property. Demos therefore run under HiDef and the whole API is
available (`GetBackBufferData`, used by `--screenshot`, is refused under Reach). A demo about the
Reach profile selects it itself with `GraphicsDeviceManager::setGraphicsProfileProperty()`.

## 4. Process model and demo kinds

| Kind | Registered with | Runs | Use for |
|---|---|---|---|
| Console | `CNA_DEMOS_CONSOLE(Function, {...})` | `void Function(DemoContext&)` in-process, no Game, no window | math, content data, storage, logging, anything without a frame loop |
| Game | `CNA_DEMOS_GAME(Class, {...})` | a `DemoGame` subclass, constructed, `Run()`, destroyed, in-process | everything that needs a GraphicsDevice or a frame loop |
| Standalone | `CNA_DEMOS_STANDALONE({...})` in `standalone/<id>/metadata.cpp` | a separate executable started as a child process | process-wide state: renderer selection, start-up, custom entry points, the C API, several processes (System Link Find/Join) |

In-process Game demos rely on CNA supporting several complete `Game` lifecycles one after another
in one process (CNA tests this: `GameTest.UnloadContentWorksAcrossRepeatedGameInstancesInOneProcess`).
The browser and `--run-all` therefore run Game demos back to back in one process and return to
the menu afterwards. A crash (not an exception) still ends the process; CTest runs each demo in
its own process for isolation.

The exception is state CNA (like XNA) sets up once per process: `GamerServicesDispatcher` refuses
a second initialization, so a demo with a `GamerServicesComponent` cannot follow another one in
the same process. Such a demo sets `ownProcess` in its metadata; batch runs and the browser then
start `cna-demos <id> --batch-child --report <tmp>` and take the result from that report (status,
message, checks). `cna-demos <id>` itself is such a process and runs the demo directly. The demo
stays an ordinary in-process Game demo in every other respect (one source file, the shared
executable).

## 5. Registry and metadata

### Registration

`CnaDemos/Demo.hpp` defines the three macros. Each expands to a `const` registrar object in an
anonymous namespace whose constructor adds a `RegisteredDemo` to `Registry::Instance()` during
static initialisation. The registry is a function-local static, so initialisation order between
translation units does not matter; registration order is unspecified and every query returns
demos sorted by ID.

Demo sources are compiled into **OBJECT libraries** (one per category, plus
`cna_demos_standalone_metadata`). Object files are always linked, so the registrars cannot be
dropped by the linker the way unreferenced members of a static library would be.

### Metadata

`CnaDemos::DemoInfo` (in `src/CnaDemos/Metadata.hpp`) is written by the author with designated
initializers. Fields: `id`, `title`, `description`, `apis`, `tags`, `automation`, `needs`, `frames`,
`deterministic`, `assets`, `restrictions`, `knownIssue`, `knownIssueRenderers`, `ownProcess`. Nothing is written twice:

* **category** = first ID segment; **subcategory** = the segments between first and last;
* **kind** = which macro registered the demo;
* **source** = `__FILE__`, made repository-relative;
* **executable** (standalone) = `cna-demo-` + ID with `.` replaced by `-`, the same rule in C++
  (`StandaloneExecutableName`) and CMake (`cna_demos_add_standalone`).

### ID rules (enforced by `cna-demos --validate`)

* two or more `lower_snake_case` segments joined by `.`;
* unique;
* matches the source path: `demos/a/b/c.cpp` registers `a.b.c` or `a.b.c.<name>` (several closely
  related demos may share one file); standalone demos register exactly their directory name;
* a leaf: no demo ID is also the group of another demo (`a.b` and `a.b.c` cannot both be demos);
* title without trailing period, description of at least 20 characters ending with a period, at
  least one API, lower-case tags, existing assets.

## 6. Demo lifecycle

### Console demo

`RunDemo()` creates a `DemoContext`, installs it as `DemoContext::Current()` and calls the
function. The demo prints with `ctx.Print(...)`/`ctx.Out()` and records expectations with
`ctx.Check(condition, "...")`.

### Game demo

1. The runner installs the context and calls the registered factory, which constructs the demo
   class. `DemoGame`'s constructor reads `DemoContext::Current()`, creates the
   `GraphicsDeviceManager`, sets `Content.RootDirectory = "assets"` and the window title.
2. `Game::Run()` creates the device and calls `Initialize()`. `DemoGame::Initialize()` checks the
   demo's declared `needs` against `GraphicsDevice::SupportsCapability()` (and, for `Need::Window`,
   whether the renderer has a native window). An unmet need throws `SkipDemo`: the run ends as
   **SKIPPED** with the reason, before `LoadContent()`.
3. The demo's own `LoadContent()/Update()/Draw()` run as in any XNA game.
4. `DemoGame::EndDraw()` counts frames. With a frame limit (automated runs, or `--frames`) it
   saves the screenshot on the last frame if one was requested, presents, and calls `Exit()`.
   Interactive runs end with Escape, gamepad Back, or closing the window.
5. `Run()` returns; the runner calls `Dispose()` -- XNA's `using (game) game.Run();` pattern, which
   is what invokes `UnloadContent()` -- and then destroys the game. Disposal and destruction are
   part of every run, so cleanup defects show up as failures or crashes.

Exceptions from anywhere in this sequence end the run as **FAILED** with the message --
except `SkipDemo`, and `NoAudioHardwareException` from a demo that declared `Need::Audio`
(SKIPPED, because the environment truly lacks the documented requirement).

### Standalone demo

The runner starts `<exe-dir>/cna-demo-<id>` with `--automated`, `--frames N`,
`--screenshot PATH` and `--log-level LEVEL` as applicable (the renderer travels in `CNA_GRAPHICS_RENDERER`, which
`--renderer` exports) and maps its exit code: 0 passed, 77 skipped, anything else failed. A
missing executable (built with `CNA_DEMOS_BUILD_STANDALONE=OFF`) is NOT RUN, not passed. The C API
demos (`c_api.*`, C programs linking only `CNA::CApi`) declare `Need::CApi`: in a build without
CNA's C API they are SKIPPED without being started.

### Results and exit codes

`RunStatus` is Passed, Failed, Skipped or NotRun. A single-demo run exits 0 / 1 / 77 / 1; batch
runs exit 1 if anything failed. Usage errors exit 2.

## 7. Categories and the CMake architecture

```text
cna_demos_framework (STATIC)  ← src/framework/*.cpp, links CNA
cna_demos_cat_<category> (OBJECT, one per category) ← demos/<category>/**/*.cpp (GLOB_RECURSE CONFIGURE_DEPENDS)
cna_demos_standalone_metadata (OBJECT) ← standalone/*/metadata.cpp
cna-demos (EXECUTABLE) ← src/app/*.cpp + all of the above
cna-demo-<id> (EXECUTABLE, one per standalone demo; C_API ones are C programs linking CNA::CApi)
cna_demos_assets (custom target) copies assets/ to <build>/bin/assets
cna_demos_content (cna_add_content) compiles content/ into <build>/bin/assets (CNB files)
cna_demos_content_xnb (cna_add_content FORMAT xnb) compiles content_xnb/ into <build>/bin/assets/xnb
cna_demos_content_media (cna_add_content, only with CNA's FFmpeg backend) compiles content_media/ into <build>/bin/assets/media
cna_demos_content_project (cna_add_content CONTENT_PROJECT) builds content_project/Game.contentproj into <build>/bin/assets/contentproj
cna_demos_title_files (custom target) copies title/ to <build>/bin
```

* Categories are declared once each in `demos/CMakeLists.txt`, optionally with `REQUIRES <option>`
  (e.g. `CNA_ENABLE_NET`) and `LIBRARIES <CNA target>`. A category whose requirement is off is
  skipped with a status message and its demos are absent from that build's catalog.
* Sources are found with `file(GLOB_RECURSE ... CONFIGURE_DEPENDS)`: sorted, deterministic,
  re-checked on every build. Adding a demo never requires editing CMake.
* There is exactly one `add_executable()` for all in-process demos, however many there are.
* Everything is written to `<build>/bin/`, so the executable, standalone demos and `assets/` sit
  together (CNA resolves content relative to the executable's directory, via
  `TitleLocation`/SDL's base path).

Scaling notes: the per-demo cost is one small translation unit. Build parallelism comes from
Ninja; category OBJECT libraries keep per-target command lines short. Precompiled headers and
unity builds are deliberately not used (unity builds would merge the anonymous namespaces that
keep demos independent). Revisit with measurements when the collection is in the thousands.

## 8. Assets

* `assets/` holds shared, small, original assets organised by purpose (`textures/`, `sounds/`, ...).
  A demo lists what it loads in `.assets`; `--validate` checks the files exist.
* `content/` holds sources that must be compiled first, as in an XNA content project: CNA's
  `cna-content` builds them at build time (`cna_add_content`, per-asset processors in
  `content/.cna-content.json`) into the same `bin/assets` directory, so demos load them by name
  (`Content.Load<SpriteFont>("fonts/pixel_font")`). cna-content only deletes files its own manifest
  records, so it shares the directory safely with the copied loose assets. Cross builds name a host
  tool with `CNA_DEMOS_CONTENT_EXECUTABLE`.
* `content_xnb/` is the same for XNA's XNB format (`cna_add_content(... FORMAT xnb)`), compiled into
  `bin/assets/xnb/` and loaded as `xnb/<name>`. It exists for the XNB demos -- e.g. DXT textures,
  which CNB schema 1 cannot store (CNA documents and warns about that).
* Assets are generated by `tools/generate_assets.py` (deterministic output, committed) or authored
  as small text fixtures, including the custom CNJ enemy definitions in `assets/custom`.
  Third-party assets are only allowed with licence and attribution in `THIRD_PARTY_NOTICES.md`.
* Prefer procedural data inside the demo when the demo is not about loading; prefer a shared
  asset over duplicating one.
* `content_media/` holds sources whose import needs CNA's FFmpeg backend: videos (`.wmv`, as XNA's
  pipeline imported) and compressed audio. The directory is compiled only when CNA built that
  backend (`TARGET cna_video_ffmpeg`); without it the demos that load them skip (`Need::Video`)
  and `--validate` does not ask for their assets.
* `content_project/` is an XNA Game Studio content project as a ported game has one:
  `Game.contentproj` (written by hand) names each item's importer, processor, processor parameters
  and output name, and files it only copies. `cna_add_content(... CONTENT_PROJECT ...)` builds it
  as it is -- XNB, since the project says so -- into `bin/assets/contentproj/`; its sources are
  generated by `tools/generate_assets.py`.
* `title/` holds files CNA opens by a fixed title-relative path (`TitleContainer`), not through
  `Content`: `GamerServices/Achievements.json`, the offline achievement catalog. It is copied into
  `bin/` itself.

## 9. Command line, browser, catalog

* `src/app/Cli.cpp` parses options (no third-party parser) and dispatches. `--help` is the
  reference for the commands.
* `src/app/Browser.cpp` builds the category tree from the IDs on every screen (no separate tree
  data): Category → Subcategory → ... → Demo → information → run. It also offers `/search` and
  typing an ID directly. Display names come from the ID segments with a small table of spellings
  (`gltf` → `glTF`, `sprite_batch` → `SpriteBatch`).
* `--catalog-json` writes schema `cna-demos-catalog/1`: versions, compiled-in renderers, and per
  demo every metadata field plus derived category, subcategory, kind, source and executable.
  It is the input of the coverage report and of any future documentation generator.
* `--report FILE` writes schema `cna-demos-results/1` with one status per demo run.

## 10. Extension points

| To add | Do this |
|---|---|
| a demo | one `.cpp` under `demos/` ([DEMO_AUTHORING.md](DEMO_AUTHORING.md)) |
| a category | a directory under `demos/` and one `cna_demos_add_category()` line |
| a standalone demo | `standalone/<id>/` with `metadata.cpp`, `main.cpp`, `CMakeLists.txt` |
| a metadata field | `DemoInfo`, catalog writer, `--info`, validation, docs -- in one change |
| a `Need` | `Need` enum + `AllNeeds()` name; map it in `DemoGame::CheckNeeds()` if the device can answer |
| screenshot regression | see [AUTOMATION.md](AUTOMATION.md) section 5 |
| binding demos (C) | `standalone/c_api.<area>.<name>/` with `main.c`, `cna_demos_add_standalone(SOURCES main.c C_API)`, `Need::CApi`; see DEMO_AUTHORING.md §8 |

## 11. Generated files

Nothing generated is committed except:

* `docs/COVERAGE.md` -- regenerate with `tools/coverage_report.py` (see the file's header);
* `assets/*` -- regenerate with `tools/generate_assets.py`.

`tests/process_output.cpp` checks the real process launcher through a buffered pipe, compiling
Process.cpp directly without pulling in CNA. It prevents child output from overtaking pending
parent diagnostics; both C and C++ standard streams are flushed before launch.

Build-time generated files live only in the build tree (the CTest discovery wrapper
`cna_demos_discover_tests.cmake`).
