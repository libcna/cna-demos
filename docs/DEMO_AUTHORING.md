# Writing a demo

A demo shows **one CNA capability** in a program short enough to read in a minute. Adding one is
normally: create one `.cpp` file, build, run, validate. No CMake edits.

## 1. Before writing

1. **Is it real?** Find the API in CNA's headers (`../cna/modules/<module>/include/...`). Never
   write a demo from memory of XNA, MonoGame or FNA -- CNA's names and signatures are what count
   (properties are `getXProperty()`/`setXProperty()`).
2. **Is it new?** `cna-demos --search <words>` and `docs/COVERAGE.md`. A demo must teach something
   another demo does not. Different *usage* earns a demo; a different constant usually does not.
3. **Is it one concept?** "Rotate a sprite" is one demo. "Rotate, scale, tint and flip" is four,
   or one demo about the full `Draw` overload if that overload is the point.

## 2. Choose the ID and the file

The ID is the file path below `demos/` with `/` → `.` and without `.cpp`:

```text
demos/graphics/sprite_batch/rotation.cpp   →   graphics.sprite_batch.rotation
```

* Segments are `lower_snake_case`. The first segment is the category (one of the directories in
  `demos/CMakeLists.txt`, mirroring CNA's modules); the middle segments name the CNA type or
  area (`sprite_batch`, `texture2d`, `vector3`, `render_target`); the last one names the concept.
* Several tightly related demos may share one file; their IDs then extend the file's ID:
  `demos/graphics/sampler_state/filter.cpp` may register `graphics.sampler_state.filter.point` and
  `graphics.sampler_state.filter.linear`.
* IDs are permanent. Renaming one breaks scripts, reports and links -- avoid it.
* A demo ID cannot also be a group: if `graphics.sprite_batch.rotation` exists, there is no
  `graphics.sprite_batch.rotation.fast`.

## 3. Console demo (no window)

```cpp
// SPDX-License-Identifier: MS-PL
// Vector2::Length and Vector2::LengthSquared.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using Microsoft::Xna::Framework::Vector2;

namespace
{
    void Vector2Length(CnaDemos::DemoContext& ctx)
    {
        const Vector2 v(3.0f, 4.0f);
        ctx.Print("Vector2(3, 4).Length() = {}", v.Length());
        ctx.Check(v.Length() == 5.0f, "Length of (3, 4) is 5");
    }
}

CNA_DEMOS_CONSOLE(Vector2Length, {
    .id = "math.vector2.length",
    .title = "Vector2 length",
    .description = "Computes the length of a Vector2.",
    .apis = {"Vector2::Length"},
})
```

## 4. Game demo (a frame loop and a device)

```cpp
class ClearDemo final : public CnaDemos::DemoGame
{
protected:
    void Draw(const GameTime& gameTime) override
    {
        getGraphicsDeviceProperty().Clear(Color::CornflowerBlue);
        Game::Draw(gameTime);
    }
};

CNA_DEMOS_GAME(ClearDemo, {
    .id = "graphics.device.clear",
    .title = "Clear the screen",
    .description = "Clears the back buffer to Color::CornflowerBlue every frame.",
    .apis = {"GraphicsDevice::Clear"},
    .frames = 10,
    .deterministic = true,
})
```

`DemoGame` is a plain `Microsoft::Xna::Framework::Game`. Write `LoadContent`, `Update`, `Draw` and
`UnloadContent` exactly as in an XNA game. The base class provides the `GraphicsDeviceManager`
(`Graphics()`), `Content.RootDirectory = "assets"`, the title, need checks, the automated frame
limit, Escape-to-exit and screenshots. If you override `Initialize()` or `EndDraw()`, call
`DemoGame::Initialize()` / `DemoGame::EndDraw()`.

Do not present the frame yourself: `Game::EndDraw()` presents once per frame, as in XNA.

A demo that waits for something in real time (a song to end, a timer) may call `Exit()` itself once
its checks are done, when `Context().Automated()` is true -- an interactive run keeps the window
open. `--screenshot` still captures the last frame it drew.

## 5. Metadata reference

| Field | Required | Meaning |
|---|---|---|
| `id` | yes | see section 2 |
| `title` | yes | short, sentence case, no trailing period |
| `description` | yes | one or two sentences, what the demo shows; ends with a period |
| `apis` | yes | CNA members the demo is about, `Type::Member` with XNA member names (`SpriteBatch::Draw`, `Texture2D::Width`, a constructor as `RenderTarget2D::RenderTarget2D`, an ambiguous nested type as `TouchCollection.Enumerator::MoveNext`, CNA extensions fully qualified `CNA::GraphicsRendererSelection::SetPreferred`). These feed the coverage report: spell them exactly. |
| `tags` | no | extra lower-case search words |
| `automation` | no (`Automated`) | `Automated`: an unattended run checks the feature. `SmokeOnly`: an unattended run only proves start-up/shutdown (input demos). `Manual`: cannot run unattended (needs hardware, a service, a person from the start) |
| `needs` | no | `Need::ThreeD`, `Need::Audio`, `Need::Window`, ... combined with `|`. Unmet graphics needs and missing audio hardware end the run as SKIPPED with a reason. Input needs document that a person is required |
| `frames` | no (60) | frames an automated Game run draws |
| `deterministic` | no (false) | the captured frame is identical on every run and machine -- a screenshot-regression candidate. True for static scenes; **not** for GameTime-driven animation (catch-up Updates make game time at frame N machine-dependent). Never derive images from the wall clock or unseeded randomness |
| `assets` | no | files under `assets/` the demo loads (validated) |
| `restrictions` | no | renderer/platform restrictions and *why* |
| `knownIssue` | no | `CNA-DEMOS-ISSUE-NNN` when a recorded CNA defect makes the demo fail (section 11) |
| `knownIssueRenderers` | no | renderer names (`"OPENGLES3"`) when the defect is in those renderers only; empty = all |
| `ownProcess` | no (false) | isolate once-per-process services or deliberate global state changes that cannot faithfully restore unknown parent state. Batch/browser runs launch the shared executable in a new process; supported for Game and Console. Every `GamerServicesDemo` and the built-in reader-withdrawal example set it |

## 6. Style

* Start with `// SPDX-License-Identifier: MS-PL` and a one-line comment saying what is shown.
* Keep the CNA call that the demo teaches **visible in the demo**. Do not hide it behind helpers.
  Shared helpers are for boilerplate the demo is *not* about.
* Comments explain CNA behaviour and non-obvious choices, not C++.
* Use `ctx.Check()` for every fact the demo can verify; failed checks fail the run.
  `Context().Check(...)` inside a `DemoGame`.
* Use `ctx.Print()`/`Context().Print()` for output (`--quiet` silences it); avoid `std::cout`.
* Randomness: seed from `Context().Seed()`. Time: prefer `GameTime` or frame counts.
* Anonymous namespace for everything in the file; `final` on demo classes.
* Batch runs execute many demos in one process. Undo process-wide changes in `UnloadContent`:
  remove handlers from static events (`Mouse::ClickedEXT`, `TextInputEXT::TextInput`, ...) by
  token, and restore global input state (cursor shape, relative mouse mode, gamepad keyboard
  emulation, clipboard contents).
* Shared helpers in `src/CnaDemos/` cover boilerplate only: `GraphicsNames.hpp`,
  `InputNames.hpp` and `GamerServicesNames.hpp` (enum names for output), `PixelChecks.hpp`
  (back-buffer pixel checks), `MathChecks.hpp` (tolerances), `Geometry.hpp` (sphere and cube
  meshes), `GamerServicesDemo.hpp` (below).
* Gamer services (`gamer_services`, `net`): derive from `CnaDemos::GamerServicesDemo` instead of
  `DemoGame` and set `.ownProcess = true`. Its constructor adds the `GamerServicesComponent` (as
  an XNA game's constructor does) after selecting CNA's offline mode -- no service is ever
  contacted -- with the demos' own profile store (`profiles` in the cna-demos storage root, unless
  `CNA_GAMER_SERVICES_PROFILES_DIR` is set) and the local profile "Demo Player" signed in at start
  (unless `CNA_GAMER_SERVICES_AUTO_SIGN_IN` names others). It appears in `Gamer::SignedInGamers`
  in the first Update. Unattended runs answer the Guide with `Guide::SimulateMessageBoxClickEXT`
  / `SimulateKeyboardInputCancelEXT` and type with `TextInputEXT::INTERNAL_OnTextInput`. What
  CNA keeps offline (profiles, achievements, leaderboards) persists between runs: write checks
  that hold on a second run too.
* No placeholders, no TODO demos, no "coming soon". A demo exists when it works.
* English everywhere, except text that is itself the subject of a localisation/Unicode demo --
  mark those as intentional.

## 7. Assets

Prefer generating small data in the demo when loading is not the point. When it is, use or add a
shared asset under `assets/<purpose>/`: add it to `tools/generate_assets.py` (original, generated)
or document a third-party file in `THIRD_PARTY_NOTICES.md`. List it in `.assets`.

Content that XNA would compile first (SpriteFonts, and later effects or models) goes under
`content/` and is built by CNA's `cna-content` into the same assets directory; choose a processor
in `content/.cna-content.json` when the file extension does not imply it, and set processor
parameters there (for textures: `colorKey`, `premultiplyAlpha`, `generateMipmaps`,
`resizeToPowerOfTwo`; see the `content.texture_processor.*` demos). Sources that must be compiled to
XNA's XNB format instead go under `content_xnb/` and load as `xnb/<name>`; videos and
compressed audio go under `content_media/` and load as `media/<name>` (declare `Need::Video`: they
are compiled only when CNA has its FFmpeg backend). A demo about XNA content projects adds an
item to `content_project/Game.contentproj` and loads it as `contentproj/<folder>/<Name>`; CNJ
documents (`.cnj`: curves, sprite fonts with their atlas) go under `content/` like any source.
Load it by name like any
XNA game (`Content.Load<SpriteFont>("fonts/pixel_font")`) and list the *compiled* file in
`.assets` (`"fonts/pixel_font.cnb"`). The shared font for on-screen text is `fonts/pixel_font`
(printable ASCII, LineSpacing 18, crisp with `SamplerState::PointClamp`).

Files CNA reads by a fixed path relative to the title's directory rather than through `Content`
go under `title/`, which is copied next to the executables: `title/GamerServices/Achievements.json`
is the offline achievement catalog the `gamer_services.achievement.*` demos use.

## 8. Standalone demos

Only when the feature needs its own process (renderer selection, start-up, custom entry point,
the C API, process-global state). Create `standalone/<demo-id>/`:

* `metadata.cpp` -- `CNA_DEMOS_STANDALONE({ .id = "<demo-id>", ... , .restrictions = "Standalone: <why>" })`;
* `main.cpp` -- a complete program; accept `--automated` and `--frames N`; exit 0 on success,
  77 when a documented requirement is missing, 1 on failure;
* `CMakeLists.txt` -- `cna_demos_add_standalone(SOURCES main.cpp)`.

The runner also passes `--screenshot PATH` when one was requested and `--log-level LEVEL` (CNA's
log output); a program that cannot use one ignores it.

`CnaDemos/StandaloneProgram.hpp` (header-only) has what the C++ programs share: `ParseArguments`
(the runner's arguments, log level applied, the rest kept for the program), `Report` (log lines
with the process's role, counted checks), `SaveScreenshot` and `ThisProgram` (the program's own
path, to start it again in another role with `CnaDemos::RunProcess` -- link `cna_demos_framework`).
Programs with gamer services call `CnaDemos::UseOfflineDemoProfiles()` (`GamerServicesDemo.hpp`)
and set `CNA_GAMER_SERVICES_AUTO_SIGN_IN` themselves. The `net.*` standalone demos are two
processes of one program: the host starts the other side on a worker thread and checks its exit
code; demos that need the network run one at a time under CTest (`RESOURCE_LOCK`).

### C API demos (`c_api.*`)

C programs written against CNA's C ABI (`../cna/modules/c-api/include/CNA/C/*.h`, model program
`../cna/modules/c-api/examples/c/hello_cna.c`). They are standalone demos with three differences:

* `main.c` defaults to C99 without extensions, and `CMakeLists.txt` says
  `cna_demos_add_standalone(SOURCES main.c C_API)`: the program links `CNA::CApi` only. Without
  CNA's C API (`CNA_BUILD_C_API=OFF`; the `linux-full` preset turns it on) it is not built;
* the metadata declares `.needs = CnaDemos::Need::CApi` (plus any other need), so such a build
  reports the demo as SKIPPED instead of NOT RUN;
* `.apis` lists C names as they are: routes (`cna_game_create`) and structs (`CNA_SpriteCommand`).
  docs/COVERAGE.md counts the routes against every `CNA_C_API` declaration in the headers.

`c_api.core.thread_local_errors` and `c_api.game.foreign_thread_dispatch` are focused C11
exceptions: standard threads and atomics coordinate their checks. Each target requests C11 and
probes thread/atomic support before building; ordinary C targets retain C99. Without that support
the metadata stays visible and the executable is omitted. The diagnostics example uses only
game-independent routes; the dispatch example explicitly serves worker game-property calls on
the owner thread with bounded waits. Neither is a rendering test, and neither demonstrates
automatic callback pumping or queued-call cancellation merely by handling normal completion.

`#include "CnaDemos/CApiDemo.h"` gives the shared scaffolding -- argument parsing
(`cna_demo_init`), checks (`cna_demo_check`, `cna_demo_check_result`, which prints the ABI's
diagnostic for a failed call), the exit code (`cna_demo_finish`, `cna_demo_skip`) and
`cna_demo_capture` (back buffer to PNG, for `--screenshot`; it needs the HiDef profile, which
`cna_demo_use_hidef` selects through a graphics device manager the demo destroys before its game). It wraps no CNA feature: every
taught call stays in the demo. Follow the ABI's own rules in every demo: zero each versioned
struct and set `struct_size`/`struct_version`; borrow the graphics device only inside a
callback; check every `CNA_Result`; destroy owned children after `cna_game_run` returns and before
`cna_game_destroy` (the game refuses to be destroyed while one lives, and it raises
`unload_content` only from inside `cna_game_destroy`, so that callback cannot release them).

## 9. Validate

```bash
cmake --build --preset linux-full                         # or your build directory
build/linux-full/bin/cna-demos --validate                 # IDs, metadata, paths, assets
build/linux-full/bin/cna-demos <id>                       # run it and look at it
build/linux-full/bin/cna-demos <id> --automated --renderer SOFTWARE --screenshot /tmp/<id>.png
ctest --preset linux-full -R '<id>'                        # the discovered per-demo test
```

Look at the screenshot. A demo that runs but draws the wrong thing is not done.

## 10. Good and bad scope

| Good | Why |
|---|---|
| `graphics.sprite_batch.rotation` -- one sprite rotating about its centre | one concept, visible |
| `graphics.sampler_state.filter.point` and `.linear` side by side in one file | same concept, two settings compared |
| `audio.sound_effect_instance.pitch` | one property, audible |

| Bad | Why |
|---|---|
| `graphics.sprite_batch.everything` | ten concepts, teaches none |
| `input.keys.a`, `input.keys.b`, ... | one demo per enum value adds nothing |
| a demo that wraps `SpriteBatch` in a helper class | hides the API it should teach |
| a demo of `Texture2D::FromFile` | CNA has no such member -- check the headers |
| a demo that sleeps on the wall clock to animate | not deterministic, breaks screenshots |

## 11. When the demo exposes a CNA bug

Do not work around it silently. Minimise it, add an entry to [CNA_ISSUES.md](CNA_ISSUES.md), keep
the demo checking the **documented** behaviour, and set `.knownIssue = "CNA-DEMOS-ISSUE-NNN"` (plus
`.knownIssueRenderers` when only some renderers have the defect -- compare the renderers first). The
demo then still reports FAILED (with the issue named), while batch runs and CTest count it as an
expected failure; if it ever passes, CTest fails so the stale marker is noticed. Fix CNA only for an
obvious, low-risk, well-understood defect, in CNA's own repository, separately from demo work.

## 12. C++ ownership

Check who owns a returned pointer in the actual CNA implementation. `SoundBank::GetCue` returns
a newly allocated, caller-owned Cue: keep it in `std::unique_ptr<Cue>`. Calling `Dispose()`
releases playback resources but does not delete that C++ object. Declare owned cues after their
banks so scope destruction releases them first. In contrast, `VideoPlayer::GetTexture`,
`LeaderboardWriter::GetLeaderboard` and service lookups return borrowed pointers; do not put
those in an owning smart pointer. Preserve the CNA call in the demo source.
